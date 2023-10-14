"""
    SECD implementation

    This is a loose interpretation of the SECD machine
    written in Python. This is a brutal and rough
    implementation, written for didactic purposes.

    Each item of the code list is a pair (opcode, value)
    where type is a Python function which is invoked when the
    item is executes, and value is a pair of one of the
    following forms:

        (NIL, 0)
        (NUM, n) being n the number
        (STR, s) being s the string
        (VAR, s) being v the string containing the variable name
        (LIS, [v1, ..., vn]) being [v1,...,vn] the list
        (FUN, [e, [x1,...,xn], v]) being e the environment in which
            to evaluate the function, [x1,...,xn] its parameters
            and v its body. This actually encodes a closure.
    
    In a more low level representation, such values would be
    memory words containing a number or a pointer to structures
    (string, list, closures).
    
    At runtime the virtual machine uses the following data
    structures, that are all stacks:

        S, the data stack
        E, the environment stack, containing the current bindings
            for variables: it is a stack whose elements are
            stacks in turn, their elements being pairs (variable,value).
            In Python is implemented as a list of dictionaries.
        C, the code list that contains the sequence of instructions
            to execute.
        D, the dump, a stack used to contain temporary data related
            to the application of a function to an argument: each item
            in this stack is a triple (S,E,C) denoting a state of the
            machine that has been saved for some purpose (nested
            evaluation).

    At start, S, E and D are empty while C contains the object code
    to be executed.

    Then, a pair (opcode, value) is popped from the head of C and
    the opcode function is called with value as parameter.

    This is repeated until C is empty: the result of the expression
    evaluation is the only element of S or.

    Opcodes and their runtime behavior are described here:
    
        (PUSH,v)        ->  push v on the stack
        (PUSHV,v)       ->  lookup v in the current environment and
                            pushes its value on the stack
        (SUM,nil)       ->  pop x1, x2, push x1+x2
        (SUB,nil)       ->  pop x1, x2, push x1-x2
        (MUL,nil)       ->  pop x1, x2, push x1*x2
        (DIV,nil)       ->  pop x1, x2, push x1/x2
        (NEG,nil)       ->  pop x1, push -x1
        (EQ,nil)        ->  pop x1, x2, push 1 if x1==x2, else 0
        (NEQ,nil)       ->  pop x1, x2, push 1 if x1==x2, else 0
        (GT,nil)        ->  pop x1, x2, push 1 if x1>x2, else 0
        (GE,nil)        ->  pop x1, x2, push 1 if x1>=x2, else 0
        (AND,nil)       ->  pop x1, x2, push 0 unless both x1 and x2 are !=0 and != nil
        (OR,nil)        ->  pop x1, x2, push 1 unless both x1 and x2 are 0 or nil
        (NOT,nil)       ->  pop x, push 1 if x==0 or x==nil, else 1
        (APPLY,nil)       ->  pop x, f and evaluates f on x, push the result on s
                            
"""

# Opcodes implementation

# Recall that values are stored in the s stack as pairs
# (type, v), where type is a constant denoting the kind
# of value and v is the value (number, string, list)

# These codes are used to label data in the stack
ANY = -1.0
NIL = 0.0
NUM = 1.0
STR = 2.0
VAR = 3.0
LIS = 4.0
FUN = 5.0

def POP(s):
    t1, v = s.pop()
    if t != ANY and t != t1:
        raise "Type mismatch"
    return t, v

def POPN(s):
    t, v = s.pop()
    if t != NUM:
        raise "Number expected"
    return v

def PUSH(v, s, e, c, d):
    s.append(v)
    return s, e, c, d

def PUSHV(v, s, e, c, d):
    # recall: e = [e0, e1, ...] where ei = {x:v,...}
    # the topmost element of e is the 0-th one.
    for stack in e:
        if v in stack:
            return PUSH(stack[v], s, e, c, d)
    raise BaseException("Unbound variable " + str(v))

def ADD(v, s, e, c, d):
    x1 = POPN(s)
    x2 = POPN(s)
    return PUSH((NUM, x2 + x1), s, e, c, d)

def SUB(v, s, e, c, d):
    x1 = POPN(s)
    x2 = POPN(s)
    return PUSH((NUM, x2 - x1), s, e, c, d)

def MUL(v, s, e, c, d):
    x1 = POPN(s)
    x2 = POPN(s)
    return PUSH((NUM, x2 * x1), s, e, c, d)

def DIV(v, s, e, c, d):
    x1 = POPN(s)
    x2 = POPN(s)
    return PUSH((NUM, x2 / x1), s, e, c, d)

def NEG(v, s, e, c, d):
    x1 = POPN(s)
    return PUSH((NUM, -x1), s, e, c, d)

def EQ(v, s, e, c, d):
    t1, x1 = s.pop()
    t2, x2 = s.pop()
    return PUSH((NUM, x2 == x1), s, e, c, d)

def LT(v, s, e, c, d):
    x1 = POPN(s)
    x2 = POPN(s)
    return PUSH((NUM, x2 < x1), s, e, c, d)

def LEQ(v, s, e, c, d):
    x1 = POPN(s)
    x2 = POPN(s)
    return PUSH((NUM, x2 <= x1), s, e, c, d)

def AND(v, s, e, c, d):
    x1 = POPN(s)
    x2 = POPN(s)
    return PUSH((NUM, x1 and x2), s, e, c, d)

def OR(v, s, e, c, d):
    x1 = POPN(s)
    x2 = POPN(s)
    return PUSH((NUM, x1 or x2), s, e, c, d)

def NOT(v, s, e, c, d):
    x1 = POPN(s)
    return PUSH((NUM, not x1), s, e, c, d)

def APPLY(v, s, e, c, d):
    # Expect on the stack [..., e1, ..., en, f] being e1,...,en
    # the actual parameters and f the function to evaluate
    t, f = s.pop()
    if t != FUN:
        raise "Function expected"
    # f is a closure: [env, [x1,...,xn], body]
    # Associate formal and actual parameters
    params = {}
    for i in range(len(f[1])-1,-1,-1):
        params[f[1][i]] = s.pop()
    # Save on the dump the status
    s = []
    d.append((s,e,c))
    e = f[0] + [params]
    c = f[2]
    return s, e, c, d

def next(s, e, c, d):
    """Perform a step in the execution of the SECD and
    returns the updated tuple (s,e,c,d)."""
    while len(c) == 0 and len(d) > 0:
        s1, e, c = d.pop()
        s1.extend(s)
        s = s1
    if len(c) > 0:
        code, value = c[0]
        c = c[1:]
        # code is a Python function, value its parameter
        #try:
        s, e, c, d = code(value, s, e, c, d)
        #except:
        #    print("Evaluation stopped here:")
        #    print(c)
    return s, e, c, d

# Test
s = []
e = []
c = [
    (PUSH, (NUM, 1)),
    (PUSH, (NUM, 2)),
    (PUSH, (FUN, [[], ["x1", "x2"], [(PUSHV, "x1"),(PUSHV,"x2"),(SUB, 0)]])),
    (APPLY, 0)
]
d = []

while len(c) > 0 or len(d) > 0:
    s,e,c,d = next(s,e,c,d)

print("s=",s, "e=", e, "c=", c, "d=", d)
