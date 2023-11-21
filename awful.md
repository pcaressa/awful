# Awful: A Weird FUnctional Language

## Paolo Caressa

### v.2023-11

## Foreword

Awful is a simple (perhaps the simplest) functional language designed and implemented for didactic purposes: I described it at the Codemotion Milan 2023 conference.

This document is a gentle introduction to it and to the concepts of functional programming it conveys.

The implementation consists in a prototype written in Python and in a more robust implementation written in C: both are found in the repository (https://github.com/pcaressa/awful)[https://github.com/pcaressa/awful].

Enjoy,
Paolo

## What is a function?

### Functions as intensional objects

The mathematical concept of a function is quite old: in the 18th century, the great mathematicians Leonhard Euler and Joseph Louis Lagrange defined a function essentially as a symbolic expression depending on some parameters (which we call *formal parameters*) such that, on assigning a value to each such parameter one could compute a uniquely defined value for the function.

Today we will say: a function is a syntactic device depending on a list of formal parameters such that, upon assigning a value to each parameter, the syntactic device can run and produce a value in output.

If the value depends only on the parameters then the function is *deterministic*, thus a same arrangement of values for the parameters determines always the same result.

In other terms, a function is a black box that we feed with an ordered list of input data and that returns in output a single value such that the same data the same output.

The simplest way to define a function in mathematics is via a formula: for example

    f(x,y) = x^2 + y^2

This formula defines a function `f` with two formal parameters `x` and `y` that returns the sum of their squares `x^2` and `y^2`. On plugging numbers for `x` and `y` we get a number, as in

    f(1,2) = 1^2 +2^2
           = 1 + 4
           = 5

Obviously the value of `f(1,2)` is always the same, and we call it the *return value* of the function.

Now consider

    s(x) = the number y such that x^2 is y

Thus `s(x)` is the squared root of `x`: if `y = s(x)` then `y^2 = x`. But both `y = 1` and `y = -1` satisty the equation `s(1) = y`, so that `s(1)` is not uniquely defined and `s` is not to be considered as a function at all!

The golden rule is

    A function is a correspondence between input parameters and output value such that a same input corresponds at most to one and only one output.

For us, functions are always computed by algorithms, thus a function is a black box accepting inputs and returning outputs such that the black box contains an algorithm, for example a computer program.

This is the "intensional" concept of a function, advocated by Alonzo Church, the logician that in the 30s created lambda-calculus, the theoretical basis for functional programming.

### Functions as extensional objects

Let us consider the infamous factorial function which can be implemented as

    def fact(n):
        f = 1
        while n > 1:
            f = f * n
            n = n - 1
        return f

Let us suppose our equipment allows only 32 bit numbers, therefore the maximum unsigned integer is `2^32-1` = `4294967295`. This number is greater than `fact(12)` = `479001600` but less than `fact(13)` = `6227020800`, so that we can actually represent only factorials of `n` for `n = 0,1,...,12`. Our function could be simplified(?) to

    def fact(n):
        if n <= 1: f = 1
        elif n == 2: f = 2
        elif n == 3: f = 6
        elif n == 4: f = 24
        elif n == 5: f = 120
        elif n == 6: f = 720
        elif n == 7: f = 5040
        elif n == 8: f = 40320
        elif n == 9: f = 362880
        elif n == 10: f = 3628800
        elif n == 11: f = 39916800
        elif n == 12: f = 479001600
        else:
            print("Overflow!")
            f = -1
        return f

That is ugly and there's a better solution:

    fact = {1:1, 2:2, 3:6, 4:24, 5:120, 6:720,
        7:5040, 8:40320, 9:362880, 10:3628800,
        11:39916800, 12:479001600}

This is not a function but a data structure, a dictionary thus an associative table. In Python (as in any other language) dictionaries follow the same golden rule of functions: you cannot assign different values to a same key.

For example if you write

    d = {1:1, 1:2, 2:2}

then the resulting dictionary will be `{1:2,2:2}` since the `1:2` pair `key:value` overwrites the previous `1:1`, because two different valued for a same key are not allowed.

Therefore a dictionary is just a function: indeed, given a dictionary `d` we could wrap it into an intensional function

    def f(x):
        return d[x]

while a function `f(x)` whose values are taken from a well defined list or set `S` can be transformed into a dictionary as

    d = {x:f(x) for x in S}

which is Python shortcut notation for

    d = {}
    for x in S:
        d[s] = f(x)

A function expressed via a dictionary is called an "extensional" function: for example, in calculus functions are often considered as extensional objects, thus graphs of functions.

### Functions in functional languages

So, a function can be considered either an algorithm or a data structure: in functional languages it is both (indeed most modern languages embrace this identification and provide functions as "first class" objects).

From a purely theoretical point of view, one could equip a language only with functions and define everything else (numbers, strings, lists, etc.) in terms of them. This is a lot of theoretical fun and lambda-calculus books explain that in details. But here I am concerned with a more practical developer-oriented viewpoint.

Now the time has come to define our Afwul language.

## The Awful notation for functions

Awful is essentially a computer-friendly notation for intensional functions. Its approach to express intensional function stems directly from Church's ideas:

- to provide a set of "primitive" built-in functions;
- to provide a notation to build new functions from primitive or already defined ones.

### Awful native data types

Awful native data types are:

- numbers
- strings
- stacks
- functions

Numbers are usual decimal numbers of any programming language from Fortran on: they are represented as floating point numbers inside the computer. Notice that there are no integer numbers, use numbers instead.

Examples: `-.0001e10`, `123.`, `0.01`, `123`, `+2e-2`. The following are not numbers: `--2`, `0x10`, `zero`.

Strings are as usual: depending on the implementation they allow Unicode or just ASCII characters. You can enclose a string between quotes (in which case no quote should appear inside the string) or between double quotes (in which case no double quote should appear inside the string). No escape characters are provided.

Examples: `"A string"`, `'She said: "A string!"'`, `"My math's books"`. Wrong examples `"You cannot escape \""`.

Stacks are the most fundamental data structures: with them one could do anything. So Awful provides them natively.

The only constant of stack type is `NIL`, that represents the empty stack. A stack is a sequence of items that accepts items "pushed" on it and allows to retrieve the last pushed item.

### Number functions

Each data type comes with a set of built-in functions. For numbers we have:

- `ADD x y` that takes two numbers and returns their sum `x + y`.
- `SUB x y` that takes two numbers and returns their sum `x + y`.
- `MUL x y` that takes two numbers and returns their sum `x + y`.
- `DIV x y` that takes two numbers and returns their sum `x + y`.
- `POW x y` that takes two numbers and returns their sum `x + y`.

The argument of a function can be anything returning a value: for example we can nest arithmetical operators such as

    POW ADD POW 3 2 POW 4 2 0.5

How do we parse amd evaluate such an expression?

- First we parse `POW`: it needs two parameters, so:
    - we parse the first one, getting `ADD`: this, too, requires two parameters so:
        - we parse the first one, getting `POW`: this, too, requires two parameters so:
            - we parse `3` which is a number;
            - we parse `2` which is a number;
            - we compute `POW 3 2` that returns `9`.
        - we parse the second one, getting `POW`: this, too, requires two parameters, so:
            - we parse `4` which is a number;
            - we parse `2` which is a number;
            - we compute `POW 4 2` that returns `16`
        - Now we have the parameters of `ADD` so e compute `ADD 9 16` getting `25`.
    - we parse the second one, getting `0.5`
    - we compute `POW 25 0.5` getting `5`

So, numbers are values in themselves, while built-in functions need parameters to be computed.

To help in reading long and nested numerical expressions, paarentheses may be used but not as `f(x, y)` but rather as `(f x, y)`

There are also some transcendental functions:

- `EXP x` that takes a number and returns its exponential.
- `EXP x` that takes a number and returns its exponential.
- `EXP x` that takes a number and returns its exponential.
- `EXP x` that takes a number and returns its exponential.
- `EXP x` that takes a number and returns its exponential.
- `EXP x` that takes a number and returns its exponential.
- `EXP x` that takes a number and returns its exponential.