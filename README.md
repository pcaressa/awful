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

## Expressions

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


## Expression representation




## Evaluation

An expression can be evaluated, thus it can be processed to produce a single value: however, all labels freely appearing in it should possess a value to be used during the evaluation.

Such a value is provided by an *environment*, thus a list of pairs (label, value): whenever a label is encountered, the corresponding value in the environment is retrieved and substituted to the label.

Consider for example

    (x + 1)

This cannot be evalued unless an environment defining `x` is provided: thus the expression will have possibly different values in different environments.

An expression which can be evaluated with no reference to any environment is called *closed expression*. On providing an environment an expression may be closed, and we call the pair (environment, expression) a *closure* of the expression.

Now let us describe the evaluation algorithm `eval` which takes an environment A, an expression E and returns the value V = eval(A, E) of the expression according to the environment.

1. Parse a value V' from the expression:
    - if the value is a number or a string then V' is that number or string;
    - if the value is an infix expression then evaluate it (see below) and let V' be the resulting value;
    - if the value is a function then let V' that function;
    - if the value is a label then retrieve its value from A and set V' to that value.
2. If V' is not a function then set V = V' and stop (with an error if there are more tokens in the expression).
3. Else if V' is a function then evaluate the remaining tokens in the expressions to get a value V'', next apply function V' to the actual value V'' to get the value V.

To evaluate an infix expression one uses an operator precedence evaluation process, for example a stack-based bottom up process.

The priorities of operators are, from the less prioritaire to the more prioritaire:

- ,
- and or
- = < > <> >= <=
- \+ \- $
- \* /
- ^

Infix may be nested to alter priorities.


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

