# AWFUL - A Weird FUnctional Language

### Paolo Caressa

#### (c) 2023

## Introduction

AWFUL is a simple functional language designed and implemented for didactic purposed.

The ideas presented in the language stem essentially from PJ Landin papers in the '60s.

The language provides the bare minimum equipment for a functional one:

- atomic data types: numbers and strings
- lists.
- function data types with static scoping.

The language comes with a representation syntax and a description syntax: the latter is the inner form in which language expressions are handled and evaluated.

## Expressions description

Awful can essentially do only two things: to represent expressions in an inner form and to evaluate them.

An expression is the application of one or more functions to a value: possible values are

- *numbers*, decimal numbers, possibly signed, possibly with decimal part, possibly with exponent; examples: `-1 +.001 -1e+1 +100.001e-1`.
- *strings*, sequences of characters enclosed between quotes or double quotes.
- *lambdas*, thus functions not applied to any value but taken per se: the syntax is `label : expression`, the terms `label` and `expression` will be defined below.

From the syntactic point of view, expressions are just sequences of tokens, separated by spaces or delimiters (which are tokens, too) and spanning thru the end of the line.

Numbers and strings aside, the other kinds of tokens are:

- *delimiters*, thus the single character tokens `( ) [ ] { } , :` that are always parsed as single tokens, regardless of surrounding characters.
- *labels*, sequences of contiguous characters delimited either by spaces or by delimiters.

For example `1+2` is a label, while `1 +2` is a number followed by a label and `1 + 2` is a number followed by a label followed by a number. `1,2` are a number followed by a delimiter followed by another number.

Available operators are:

- The comma `,` that is a binary operator, associating on the left, which takes two arguments a, b and returns the ordered pair (a,b). Pairs can be used to build lists such as (a,(b,(c,d))), or more generally binary trees as ((a,b),(c,(e,f))). The builtin label `NIL` denotes the empty list: notice that the notation `()` won't work.
- The operators `or`, `and` with the same priority (use parentheses to alter it) are actually numerical functions: `a or b` returns the maximum between `a` and `b`, `a and b` the minimum. When applied to 0,1 they behave like Boolean operators but they are not lazy.
- The builtin function `not` accepts a number x and provides 1-x. On 0,1 it behaves like Boolean complement.
- Relations can be applied only to numbers and have their obvious meaning.
- Also the arithmetical operations `+ - * / mod ~ ^` should be clear, but: `~ a` is the negation and `a ^ b` is the raising of a to the power b.
- Strings can be concatenated by the infix `a $ b` operator.

An expression can now formally be defined as:

    expression  = proposition
                | proposition "," expression
    
    proposition = relation
                | relation "and" proposition
                | relation "or" proposition
    
    relation    = sum
                | sum relop sum
                | "not" relation

    relop       = "=" | "<" | ">" | "<>" | ">=" | "<="

    sum         = product
                | product "+" sum
                | product "/" sum
                | product "$" sum
    
    product     = negation
                | negation "+" product
                | negation "/" product
                | negation "mod" product

    negation    = power
                | "~" power

    power       = term
                | term "^" term

    term        = value
                | function value
                
    value       = number
                | string
                | function
                | "(" expression ")"
    
    function    = lambda
                | label

    lambda      = label ":" expression

This syntax is written having a top-down recursive parser in mind, and it easier than the syntax of most programming languages expressions.

When an expression is parsed, it is actually translated into an inner form, described in the next section, before being evaluated.

## Expression evaluation

To evaluate an expression means to assign to it a uniquely determined value, which depends both on the constant parts of the expression (according to the algorithm given below) and on the labels, whose values are retrieved from an *evaluation environment* wrt which che evaluation is performed.

An environment is a list of pairs (label, value): whenever a label is encountered, the corresponding value in the environment is retrieved and substituted to the label.

Consider for example

    (x + 1)

This cannot be evalued unless an environment defining `x` is provided: thus the expression will have possibly different values in different environments.

An expression which can be evaluated with no reference to any environment is called *closed expression*. On providing an environment an expression may be closed, and we call the pair (environment, expression) a *closure* of the expression.

Now let us describe the evaluation algorithm `eval` which takes an environment A, an expression E and returns the value V = eval(A, E) of the expression according to the environment.

We define the value of eval following the syntax we given above.

- To evaluate a pair `P1,P2` evaluate first P1, next P2, take their values V1 and V2 and return the pair (V1,V2) as value.
- To evaluate `R1 and R2` they are both evalued, they both should be numbers and the minimum of the two is returned as value.
- To evaluate `R1 or R2` they are both evalued, they both should be numbers and the maximum of the two is returned as value.
- To evaluate `not R` R is evalued, it should be a number N, then 1-N is returned as value.
- To evaluate `S1 relop S2`, both S1 and S2 are evaled, they should be numbers, then if the relation holds true 1 is returned, else 0 is returned as value.
- To evaluate `S1 + S2`, `S1 - S2`, `S1 * S2`, `S1 / S2`, `S1 mod S2`, `S1 ^ S2`, `~ S1`, both S1 and S2 are evaled, they should be numbers, then the operation is performed and the value returned as value.
- To evaluate `S1 $ S2`, both S1 and s2 are evauled, they should be strings, then the concatenation of these two strings is returned as value.
- To evaluate a label, the label is searched in the environment A: if not found the evaluation fails, else the value associated to the label by the environment is returned as value.
- A number, string or function have themselves as values.
- When a function is followed by a value, the function is evalued on that value.

The last point deserves some remarks. First of all, functions have only one parameter. Moreover, a function which is followed by something else is considered to be applied to that something else, which is evaluated and whose value is given to the function as actual parameter.

The algorithm is

- When a function is parsed and the expression in which it is contained is not ended, then evaluate the rest of the expression and the resulting value is given to the function as actual parameter.

So on writing

    sqrt abs x

first is evalued `x`, whose value in the current environment is retrieved, next `abs x` is evalued and finally `sqrt abs x`.

Notice that

    compose (f, g)

contains three functions, `compose`, `f` and `g`: the former is evaluated on the pair `(f, g)`, while both `f` and `g` are not followed by any value (the comma and the parenthesis being delimiters, not values) so that their are not evalued on any actual parameter but considered values *per se*.

The priorities of operators are, from the less prioritaire to the more prioritaire:

- ,
- and or
- not
- = < > <> >= <=
- \+ \- $
- \* /
- ~
- ^


## Expression representation

The inner representation of expressions relies on stacks: a stack is just a list of items with only two operations allowed:

- to push an element onto the stack;
- to pop the last pushed element from the stack.

We will assume that stacks can contain the following kinds of items:

- numbers;
- strings;
- labels;
- other stacks.

We will denote a stack as a list whose last item is the top, for example (3 2 1) or also (... 3 2 1).

The algorithm to translate from an expression into a stack is shown below, for the moment let us remark how values are represented:

- Numbers, strings and labels are represented by themselves.
- 



## Evaluation



## Blocks

A block is a piece of code, written in an imperative language, which can be used to perform non functional operations, such as to change the value of a label in an environment, get a value from a file, write a value on the terminal, etc.

A block is enclosed between braces and the token in it follows an Algol-like grammar:

    block       = "[" statements "]"
    statements  = statement | statements ";" statement
    statement   = assignment | conditional | loop | io

    assignment  = "new" label "=" expression ";"
                | "let" label "=" expression ";"
                | "del" label
    
    conditional = "if" expression "then" statements "else" statements "fi"

    loop        = "while" expression "do" statements "od"

    io          = "scan" label
                | "print" expression ";"
                | "input" label
                | "yield" expression ";"


## Inner form

The inner form of an Awful expression is given by a list of values/labels. Lists are, as usual, ordered sequences of values: the first value of the list is called its *head* while the list of remaining elements is called its *tail*. An empty list has neither head nor tail; a list with one element has head but not tail.

We write `L:` for the head of a list and `:L` for its tail. Lists are represented by their items enclosed between parentheses. For example if `L` is `(1 2 3)` then `L: = 1` and `:L = (2 3)`.


Each possible value is represented by a list as follows:

- A number x is `(NUMBER x)`.
- A string s is `(STRING s)`.
- A list (x1, ..., xn) is `(LIST x1 ... xn)`.
- A function {x: x1 ... xn} is `(FUNCTION x x1 ... xn)`.
- A label x is `(LABEL x)`.
- A builtin function f is `(BUILTIN f)`.

For example the expression

    ~ abs f (x + y, x - y)

is translated into

    ((BUILTIN ~) (BUILTIN abs) (LABEL f) (LIST (+ (LABEL x) (LABEL y)) (- (LABEL x) (LABEL y))))

An environment is a list of pairs (label list), such as

    ((x (NUMBER 10)) (y (NUMBER 5)) (f /))

When an expression is evaluated, the first item of the list is parsed and, if it is a number, string or list its value is the value of the expression: if there are more symbol in the expression an error occurs.






Within this environment, the previous expression is evaluated as follows:

1. The builtin `~` is parsed and it is invoked on the tail of the list: `((BUILTIN abs) (LABEL f) (LIST (+ (LABEL x) (LABEL y)) (- (LABEL x) (LABEL y))))`.
2. The invoked function evaluates its parameter and next uses it: thus a new evaluation starts.
   -

    


## Syntax

```
script  = command ";"
        | command script

command = "let" name "=" expression ["in" name]
        | "env" name
        | "with" name "eval" expression

expr    = value
        | function value

value   = number
        | string
        | lambda
        | sequence

lambda  = "{" [kv-list ","] name ":" expr "}"

kv-list = value ":" expr
        | value ":" expr "," kv-list

```

## Environments

An *environment* is a stack of pairs `label:value` being `label` a name and `value` an object of a language, thus a number, a string, a list or a function.

Numbers and strings are much like in other languages. A list is a sequence of arbitrary values, while a function is an object which contains associations between keys and values and which can also contain a general rule to associate a value to another value. More on that later.

The purpose of an environment is to provide values for labels, needed when evaluating an expression.


## Expressions

An expression is a sequence of symbols which may be evaluated, provided that all variables in it are bind to some value.


## Sequence

A sequence is a sequence of symbols enclosed between parentheses, where nested sequences may appear as symbols, too.

The structure of a sequence is as follows:

    ( expression operator ... )

where value is any expression

## Functions

A function is specified by a set of pairs (key, value) and by a lambda expression, both optional.

The representation syntax is

    {
        key : expression, ... ,
        sequence : expression,
        variable : expression
    }

where key is a constant of any type (number, string or function)

