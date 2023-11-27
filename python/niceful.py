"""
    Niceful macro-processor.
    
    This translator converts an Niceful expression into an
    Awful expression and then invokes the Awful interpreter 
    to evaluate with it.
    
    Syntax accepted:
    
    clause
        expression
        "let" assignment-list "in" clause
        "letrec" assignment-list "in" clause

    assignment-list
        variable "=" expression
        variable "=" expression "," assignment-list

    expression
        conditional
        "fun" variable-list ":" clause

    variable-list
        variable
        variable variable-list

    conditional
        proposition
        "if" proposition "then" clause "else" clause

    proposition
        relation
        relation "and" expression
        relation "or" expression

    relation
        sum
        sum "=" sum
        sum "<" sum
        sum "<=" sum
        sum "<>" sum
        sum ">" sum
        sum ">=" sum
        "not" relation

    sum
        product
        product "+" sum
        product "-" sum
        product ":" sum     // a:b is cons(a,b) = a.extend(b)

    product
        term
        term "*" product
        term "/" product

    term
        number
        string
        variable
        "[" clause-list "]"
        "-" term
        "1st" term
        "rest" term
        "empty" term
        "(" clause ")"
        term "(" ")"
        term "(" clause-list ")"

    expression-list
        expression
        expression-list "," expression

    Operators are parsed according to the following priorities:
    
        Lowest: "and" "or"
                "=" | "<" | ">" | "<>" | ">=" | "<="
                "+" | "-" | ":"
                "*" | "/"
"""
from awful import awful, awful_to_str
from scan import expect, look_ahead, parse, scan

DELIMITERS = "()[],:"

KEYWORDS = ("1st", "and", "else", "empty", "fun", "if", "in", "let",
            "letrec", "nil", "not", "or", "rest", "then",
            "=", "<", ">", "<>", "<=", ">=", "+", "-", "*", "/", "^")

# Classic top-down parser: verbose but easy. Each
# function parses from c and translates into a string
# s a syntactic production in a recursive way.

def parse_list(c):
    """Parse a list from c and translate it into a string s:
    the pair (s, c) is returned, with c updated and the first
    unparsed token is the final ']'.
    A list [e1,..,en] turns into the Awful expression
    PUSH e1 PUSH e2 ... PUSH en NIL."""
    if look_ahead(c) == "]":
        c = c[1:]
        retval = "NIL"
    else:
        s, c = parse_let(c)
        retval = "PUSH " + s + " "
        npar = 1    # number of opened parentheses in retval
        while look_ahead(c) == ",":
            s, c = parse_let(c[1:])
            retval += "PUSH " + s + " "
            npar += 1
        c = expect(c, "]")
        retval += " NIL"
    return retval, c

def parse_funappl(retval, c):
    "retval contains the term just parsed."
    while len(c) > 0 and c[0][0] == "(":
        aparams = []
        c = c[1:]   # skip the "("
        while look_ahead(c) != ")":
            s1, c = parse_let(c)
            aparams.append(s1)
            tok = look_ahead(c)
            c = c[1:]
            if tok == ")":
                break
            if tok != ",":
                raise BaseException("',' or ')' expected in actual parameter list")
        retval = "(" + retval  + " " + ",".join(aparams) + ")"
    return retval, c

def parse_term(c):
    tok, val, c = parse(c)
    if tok == "NUMBER": retval = str(val)
    elif tok == "STRING": retval = '"' + val + '"'
    elif tok == "ATOM": retval = val
    elif tok == "nil": retval = "NIL"
    elif tok == "-":
        s, c = parse_term(c)
        retval = "SUB 0 " + s
    elif tok == "(":
        s, c = parse_let(c)
        c = expect(c, ")")
        retval = " " + s + " "
    elif tok == "1st":
        s, c = parse_term(c)
        retval = "TOS " + s
    elif tok == "rest":
        s, c = parse_term(c)
        retval = "BOS " + s
    elif tok == "empty":
        s, c = parse_term(c)
        retval = "ISNIL " + s
    elif tok == "[":
        retval, c = parse_list(c)
    else:
        raise BaseException(f"Syntax error: '{val}'")

    # After a term, a list of actual parameters, enclosed
    # between parentheses and separated by commas, may follows:
    # if it is the case, translate it.
    return  parse_funappl(retval, c)

def parse_pow(c):
    retval, c = parse_term(c)
    if len(c) > 0 and c[0][0] == "^":
        s, c = parse_term(c[1:])
        retval = "POW " + retval + " " + s
    return retval, c

def parse_prod(c):
    retval, c = parse_pow(c)
    if len(c) > 0 and (tok := c[0][0]) in ["*", "/"]:
        s, c = parse_pow(c[1:])
        retval = ("MUL " if tok == "*" else "DIV ") + retval + " " + s
    return retval, c

def parse_sum(c):
    """Parse a Niceful sum and return it as an Awful string,
    coupled with the updated value of c."""
    retval, c = parse_prod(c)
    if len(c) > 0 and (tok := c[0][0]) in ["+", "-", ":"]:
        s, c = parse_prod(c[1:])
        retval = ("ADD " if tok == "+" else "SUB" \
            if tok == "-" else "PUSH ") + retval + " " + s \
            + (" NIL" if tok == "PUSH" else "")
    return retval, c

def parse_relation(c):
    if look_ahead(c) == "not":
        s, c = parse_relation(c[1:]) # c[1:] to skip "not"
        return "SUB 1 " + s, c
    retval, c = parse_sum(c)
    if len(c) > 0 and (tok := c[0][0]) in ["=", "<", "<>", ">=", ">", "<="]:
        s, c = parse_sum(c[1:])
        if tok == "=" or tok == "==": pre = "EQ "
        elif tok == "<": pre = "LT "
        elif tok == "<=": pre = "LE "
        elif tok == "<>": pre = "NE "
        elif tok == ">=": pre = "GE "
        elif tok == ">": pre = "GT "
        retval = pre + retval + " " + s
    return retval, c

def parse_proposition(c):
    retval, c = parse_relation(c)
    if len(c) > 0 and (tok := c[0][0]) in ["and", "or"]:
        s2, c = parse_proposition(c[1:]) # c[1:] to skip "and"/"or"
        retval = ("MAX " if tok == "and" else "MIN ") + retval + " " + s2
    return retval, c

def parse_conditional(c):
    if look_ahead(c) != "if":
        return parse_proposition(c)
    # if E1 then E2 else E3 becomes
    # (COND E1 {:E2} {:E3})
    e1, c = parse_proposition(c[1:])   # c[1:] to skip "if"
    c = expect(c, "then")
    e2, c = parse_let(c)
    c = expect(c, "else")
    e3, c = parse_let(c)
    return "(COND " + e1 + "{:" + e2 + "}{:" + e3 + "})", c

def parse_fun(c):
    if look_ahead(c) != "fun":
        return parse_conditional(c)
    # with X1 .... Xn : C
    fpars = []
    tok, val, c = parse(c[1:])  # c[1:] to skip "fun"
    while tok == "ATOM":
        fpars.append(val)
        tok, val, c = parse(c)
    if tok != ":":
        raise BaseException("Missing ':' in 'fun'")
    retval, c = parse_let(c)
    return "{" + " ".join(fpars) + ":" + retval + "}", c

def parse_let(c):
    """Parse from the Niceful token list c a clause that
    translate into an Awful string returned as value, paired
    with the updated value of c."""
    tok = look_ahead(c)
    if tok != "let" and tok != "letrec":
        return parse_fun(c)

    # "let X1 = E1, ..., Xn = En in C" is converted to
    #   "({X1, ..., Xn: C} E1, ..., En)"
    # "let X1 = E1, ..., Xn = En in C" is converted to
    #   "({!X1, ..., !Xn: C} E1, ..., En)"
    # The only difference between "let" and "letrec"
    # is that the latter delayes the evaluation of
    # its parameters.
    delayed = "!" if tok == "letrec" else ""

    # Parse the assignments list into a list vars and vals
    fpars = []  # list of formal parameters Xi
    apars = []  # list of actual parameters Ei
    tok, val, c = parse(c[1:])  # c[1:] to skip "let"
    while tok == "ATOM":
        fpars.append(delayed + val)
        c = expect(c, "=")
        e, c = parse_let(c)
        apars.append(e)
        tok, val, c = parse(c)
        if tok == "in":
            break
        if tok != ",":
            raise BaseException(f"',' or 'in' expected, not {val}")
        tok, val, c = parse(c)
    retval, c = parse_let(c)
    return "({" + " ".join(fpars) + ":" + retval + "}" + ",".join(apars) + ")", c
    
def niceful(t):
    """Evaluates the text t."""
    c = scan(t, keywords = KEYWORDS, delimiters = DELIMITERS)
    try:
        s, c = parse_let(c)
        print("Awful:", s)
        y = awful(s)
        if y != None:
            y = awful_to_str(y)
    except BaseException as ex:
        print("Error:", ex)
        y = None

    return y

if __name__ == "__main__":
    from repl import repl
    print("NICEFUL - NICE FUnctional Language")
    print("(c) 2023 by Paolo Caressa <github.com/pcaressa/awful>")
    repl(niceful, prompt = "niceful")
