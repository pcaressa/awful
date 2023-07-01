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

