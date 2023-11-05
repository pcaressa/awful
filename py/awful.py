# awful.py module - Awful interpreter
# (c) 2023 by Paolo Caressa

"""
    Syntax accepted:

    term
        number
        string
        atom
        keyword expression
        "{" atom-list ":" expression "}"
        "(" term expression-list ")"
    
    expression
        term
        term expression
    
    expression-list
        expression
        expression "," expression-list

    atom-list
        atom
        "!" atom
        atom parameters
        "!" atom parameters

    Keywords denote builtin functions that are not values
    of the language but that always parse and evaluate
    the following expression as long as values for all the
    parameters they need are lacking.

    The "{x1 ... xn:e}" construct denotes a function with
    formal parameter x1,...,xn and body e. If free variables
    occur in e, their values are taken accordingo to the
    static scoping rule. The quote symbol prevent the
    corresponding actual parameter to be evaluated but
    evaluation is delayed after all formal and actual
    parameters are paired.

    The "(t e1, ..., en)" construct evaluates e that ougth
    to be a function object F; next parameters of F are
    paired with expressions e1,..., en following t:items 
    of the expression list neeed to be separated by commas
    since they are sequences of terms and they are bounded
    by a delimiter, such as ",", ")" or "}". Notice that
    e1, ..., en are not evaluated yet. This is done
    after the pairing, and finally a new environment is
    created containing those pairs and the body of the
    function F is evaluated: the resulting value is the
    one delivered by the "(t e1,...,en)" construct. 
    
    Keywords instead evaluates their actual parameters
    immediately.

    For example "ADD 1 2" returns "3", as "({x: ADD 1 x} 2)"
    does, but "{x:ADD 1 x} 2" returns the function and raises
    an error since the "2" is superfluous, and an expression
    can only have one value.
        
    Another example: consider

        ((COND (EQ x 0) {: DIV 1 x} {y: 0}))
    
    The COND function takes three parameters: the first is the
    result of (EQ x 0), the second and the third are two functions.
    The double parentheses are needed since we want to evaluate
    the result of the COND function, which is a parameterless
    function.

"""

from scan import expect, look_ahead, parse, scan

# Implementation of the builtin functions: each function
# takes two parameters (e,c) in input, the current environment
# and the text still to be parsed, and returns a triple
# (v, e, c) being v the result of the function application
# and e, c the updated environment and text. Actually
# functions do not alter the environment, but they perform
# calls to eval_term that can do it.

# Builtin functions

# The following function is auxiliary to numerical ones
def get_xy(e, c):
    """Evaluates two expressions expecting two numbers:
    if not, an error is raised."""
    x, e, c = eval_term(e, c)
    y, e, c = eval_term(e, c)
    if x[0] != "NUMBER" or y[0] != "NUMBER":
        raise BaseException("Number expected" + str(x))
    return x[1], y[1], e, c

def ADD(e, c):
    x, y, e, c = get_xy(e, c)
    return ["NUMBER", x + y], e, c

def SUB(e, c):
    x, y, e, c = get_xy(e, c)
    return ["NUMBER", x - y], e, c

def MUL(e, c):
    x, y, e, c = get_xy(e, c)
    return ["NUMBER", x * y], e, c

def DIV(e, c):
    x, y, e, c = get_xy(e, c)
    return ["NUMBER", x / y], e, c

def POW(e, c):
    x, y, e, c = get_xy(e, c)
    return ["NUMBER", x ** y], e, c

def MAX(e, c):
    x, y, e, c = get_xy(e, c)
    return ["NUMBER", max(x, y)], e, c

def MIN(e, c):
    x, y, e, c = get_xy(e, c)
    return ["NUMBER", min(x, y)], e, c

def EQ(e, c):
    y, e, c = eval_term(e, c)
    x, e, c = eval_term(e, c)
    return ["NUMBER", float(x == y)], e, c

def NE(e, c):
    y, e, c = eval_term(e, c)
    x, e, c = eval_term(e, c)
    return ["NUMBER", float(x != y)], e, c

def LT(e, c):
    x, y, e, c = get_xy(e, c)
    return ["NUMBER", float(x < y)], e, c

def LE(e, c):
    x, y, e, c = get_xy(e, c)
    return ["NUMBER", float(x <= y)], e, c

def COND(e, c):
    "COND x1 x2 x3 chooses x2 if x1 != 0, else it chooses x3"
    x1, e, c = eval_term(e, c)
    x2, e, c = eval_term(e, c)
    x3, e, c = eval_term(e, c)
    if x1[0] != "NUMBER":
        raise BaseException("Number expected")
    if x1[1] != 0.0:
        return x2, e, c
    return x3, e, c

def NIL(e, c):
    return ["LIST", []], e, c

def PUSH(e, c):
    x1, e, c = eval_term(e, c)
    x2, e, c = eval_term(e, c)
    if x2[0] != "LIST":
        raise BaseException("Second parameter of PUSH must be a list")
    return ["LIST", [x1] + x2[1]], e, c

def TOS(e, c):
    x, e, c = eval_term(e, c)
    if x[0] != "LIST":
        raise BaseException("Parameter of TOS must be a list")
    return ["LIST", []] if len(x[1]) == 0 else x[1][0], e, c

def BOS(e, c):
    x, e, c = eval_term(e, c)
    if x[0] != "LIST":
        raise BaseException("Parameter of BOS must be a list")
    return ["LIST", [] if len(x) < 2 else x[1][1:]], e, c

KEYWORDS = {
    "ADD": ADD, "SUB": SUB, "MUL": MUL, "DIV": DIV, "POW": POW,
    "EQ": EQ, "NE": NE, "LT": LT, "LE": LE, "MAX": MAX,
    "MIN": MIN, "COND": COND, "NIL": NIL, "PUSH": PUSH,
    "TOS": TOS, "BOS": BOS
}

DELIMITERS = "(){}:,!"

# Parsing and evaluation functions

def find(v, e):
    "Looks for variable v inside a list of environments"
    for d in e:
        if v in d:
            return d[v]
    return None

def closure(e, c):
    """Parse a function {x1, ..., xn: expr} from c, create a
    closure ["CLOSURE", params, body, e] from it and return
    it along with the pair e, c."""
    # parses a list of variables
    params = []
    while len(c) > 0:
        tok, val, c = parse(c)
        if tok == ":":
            break
        if tok == '!':
            # the parameter is not to be evalued immediately
            delay_evaluation = True
            tok, val, c = parse(c)  # parse the parameter name
        else:
            delay_evaluation = False
        if tok != "ATOM":
            raise BaseException("Atom expected in function parameter list")
        params.append((delay_evaluation, val))
    
    # Any token up to a non nested "}" is taken to be
    # part of the function body.
    body = []
    npar = 0    # number of opened parentheses
    while len(c) > 0 and (look_ahead(c) != "}" or npar > 0):
        tok, val, c = parse(c)
        body.append((tok, val))
        npar += (tok == "{") - (tok == "}")
    # The current environment is added to the closure.
    c = expect(c, "}")
    if npar > 0:
        raise BaseException("'{' without matching '}'")
    return ["CLOSURE", params, body, e], e, c

def parse_expression(c):
    """Parse an element in an expression-list, thus parse
    anything before the next "," or ")": the returned value
    is the parsed expression in the same form as c is, thus
    list of pairs; the first token in c is ')' or ','."""
    tok, val, c = parse(c)
    if tok == ")":
        raise BaseException("Too few actual parameters")
    expr = [[tok,val]]
    npar = int(tok == '(')
    nbrace = int(tok == '{')
    while len(c) > 0 and (look_ahead(c) != "," or nbrace > 0) and (look_ahead(c) != ")" or npar > 0):
        tok, val, c = parse(c)
        expr.append([tok,val])
        npar += (tok == "(") - (tok == ")")
        nbrace += (tok == "{") - (tok == "}")
    if nbrace != 0:
        raise BaseException("Unmatching braces")
    tok = look_ahead(c)
    if tok != "," and tok != ")":
        raise BaseException("',' or ')' expected in function parameter list")
    return expr, c

def apply(f, e, c):
    """Apply f = ["CLOSURE", params, body, env] to comma
    separated actual parameters to be parsed from c."""
    
    # First, create a dictionary with formal/actual parameter
    # associations.
    d = {}
    npars = len(f[1])   # number of formal parameters
    if npars == 0:      # parameterless function!
        c = expect(c, ")")
    else:
        for i in range(npars):
            # A formal parameter is (flag,name) where the
            # flag is true if the parameter need first to
            # be parsed and then to be evaluated
            if f[1][i][0]:
                # The parameter is marked with "!": parse it
                y, c = parse_expression(c)
            else:
                # Evaluate immediately the parameter
                y, e, c = eval_term(e, c)
            tok, val, c = parse(c)  # get the delimiter "," or ")"
            d[f[1][i][1]] = y
            if tok == ")" and i + 1 != npars:
                raise BaseException("Too few actual parameters")
            if tok not in ",)":
                raise BaseException("Bad actual parameter list")
    
    # Now evaluate all parameters with delayed evaluation in
    # an environment that contains all formal parameters.
    for v in f[1]:
        if v[0]:    # v[0] = flag, v[1] = parameter name
            y, e_dummy, c_dummy = eval_term([d] + e, d[v[1]])
            d[v[1]] = y
    
    # Finally, evaluate the body of the function in the
    # environment of the closure augmented by d.
    y, e_dummy, c_dummy = eval_term([d] + f[3], f[2])

    if len(c_dummy) > 0:
        raise BaseException("Too much stuff in function definition")

    return y, e, c

def evaluation(e, c):
    tok_next = look_ahead(c)
    if tok_next in KEYWORDS:
        retval, e, c = KEYWORDS[tok_next](e, c[1:])
        c = expect(c, ")")
    else:
        f, e, c = eval_term(e, c)
        if f[0] != "CLOSURE":
            raise BaseException(f"Not a function: {f}")
        retval, e, c = apply(f, e, c)
        # no need to skip the ending ")": apply do it for us.
    return retval, e, c

def eval_term(e, c):
    """Evaluate the term in c using environment e to retrieve
    variables' values: return the result y and the updated
    pair (e, c). Items in e are dictionaries {var:value}:
    the list is parsed from the 0-th item on."""

    tok, val, c = parse(c)
    if tok == "NUMBER" or tok == "STRING":
        retval = [tok, val]
    elif tok == "ATOM":
        retval = find(val, e)
        if retval == None:
            raise BaseException("Unbounded variable " + val)
    elif tok in KEYWORDS:
        retval, e, c = KEYWORDS[tok](e, c)
    elif tok == "{":
        retval, e, c = closure(e, c)
    elif tok == "(":
        retval, e, c = evaluation(e, c)
    else:
        raise BaseException(f"Syntax error: {tok, val}")
    return retval, e, c

def awful_to_str(v):
    """Given an awful value return the string representing it."""
    # v = [type, value]
    t = v[0]
    val = v[1]
    if t in DELIMITERS or t in KEYWORDS: s = val
    elif t == "NUMBER": s = str(val)
    elif t == "STRING": s = '"' + val + '"'
    elif t == "LIST":
        s = "["
        for x in val:
            s += awful_to_str(x) + ","
        s = s[:-1] + "]"
    elif t == "CLOSURE":
        s = "{ " + " ".join(val) + ":"
        for x in v[2]:  # body
            s += awful_to_str(x)
        s += "}"
    else:
        s = "?"
    return s

def awful(p):
    """Parses an expression from the string p and evaluates it."""
    c = scan(p, keywords = KEYWORDS, delimiters = DELIMITERS)
    try:
        y, e, c = eval_term([], c)
        if len(c) > 0:
            print("Warning: more stuff on the expression after evaluation!")
        y = awful_to_str(y)
    except BaseException as ex:
        print("Error:", ex)
        y = None
    return y

if __name__ == "__main__":
    from repl import repl
    print("AWFUL - AWful FUnctional Language")
    print("(c) 2023 by Paolo Caressa <github.com/pcaressa/awful>")
    repl(awful, prompt = "awful")
