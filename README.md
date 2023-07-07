# AWFUL - A Weird FUnctional Language

### Paolo Caressa

#### (c) 2023

## Introduction

AWFUL is a simple functional language designed and implemented for didactic purposed.

The ideas presented in the language stem essentially from PJ Landin papers in the '60s.

The language provides:

- atomic data types: numbers and strings
- function data types.

The language comes with a representation syntax and a description syntax: the latter is the inner form in which language expressions are handled and evaluated.


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

