# An Awful Introduction to Functional Programming

## Paolo Caressa

### v.2023-12

## Foreword

Awful (A Weird FUnctional Language) is a simple (perhaps the simplest) functional language designed and implemented for didactic purposes: I described a draft version of it and shown its prototypal implementation in Python at the Codemotion Milan 2023 conference.

This document is a gentle introduction to some basic concepts of functional programming through Awful (which is the purpose I designed the language for). Awful, at least in its current version, by no means represents a valid choice to develop serious functional programs, even if it is possible to do that, but it shows some basic devices and how they can be naively implemented.

The implementation consists in a prototype written in Python and in a more robust implementation written in C: both are stored in the repository [https://github.com/pcaressa/awful](https://github.com/pcaressa/awful).

To set up the interpreter and use it please see the [../README.md](../README.md) file in the interpreter distribution.

Enjoy,   
Paolo

## What is a function?

>*A function of a variable quantity is an analytic expression composed in any way whatsoever of the variable quantity and numbers or constant quantities.*
>
>Leonhard Euler
>

Functional languages, as the word suggests, are programming languages in which algorithms are expressed by functions: instead of method execution of most languages, the purpose of a program in a functional language is to evaluate a function, or a set of functions, combining their input parameters and output results to get, in the end, a final result of the computation.

This is the approach to the concept of algorithm as developed by Alonzo Church in the 30s, while the other one was Alan Turing approach via abstract machines (there are more of course: Markov systems, Post systems, register machines, while-programs etc.).

So we need to ask: what is a function? In this notes I will deal with loosely typed functional languages, not with strictly typed ones, since I want to focus on the basic devices of function evaluation.

### Functions as intensional objects

The mathematical concept of a function is quite old: in the 18th century, the great mathematicians Leonhard Euler and Joseph Louis Lagrange defined a function essentially as a symbolic expression depending on some parameters (which we call *formal parameters*) such that, on assigning a value to each such parameter one could compute a uniquely defined value for the function.

Today we will say: a function is a syntactic device depending on a list of formal parameters such that, upon assigning a value to each parameter, the syntactic device can run and produce a value in output.

If the value depends only on the parameters then the function is *deterministic*, thus a same arrangement of values for the parameters determines always the same result.

In other terms, a function is a black box that we feed with an ordered list of input data and that returns in output a single value such that the same input, the same output.

The simplest way to define a function in mathematics is via a formula: for example (symbol `^` means "raise to the power")

    f(x,y) = x^2 + y^2

This formula defines a function `f`, accepting two formal parameters `x` and `y`, that returns the sum of their squares `x^2` and `y^2`. On plugging numbers for `x` and `y` and doing the computations, we get a number:

    f(1,2) = 1^2 +2^2
           = 1 + 4
           = 5

Obviously the value of `f(1,2)` is always the same each time we perform the computation, and we call it the *return value* of the function.

Now consider

    s(x) = the number y such that x^2 is y

Thus `s(x)` is the squared root of `x`: if `y = s(x)` then `y^2 = x`. But both `y = 1` and `y = -1` satisty the equation `s(1) = y`, so that `s(1)` is not uniquely defined and `s` is not to be considered as a function at all!

The golden rule is


> *A function is a correspondence between input parameters and output value such that a same input corresponds at most to one and only one output.*

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

>*Progress is possible only if we train ourselves to think about programs without thinking of them as pieces of executable code.*
>
>EW Dijkstra
>

Awful is essentially a computer-friendly notation for intensional functions. Its approach to express intensional function stems directly from Church's ideas:

- to provide a set of "primitive" built-in functions;
- to provide a notation to build new functions from primitive or already defined ones.

**Notice.** Awful comes with almost no syntax: the interpreter breaks the source text into *tokens* that can be:

- A single character among `( ) { } :` called *delimiter*: this is always parsed as a single character, thus `({:})` are five tokens.
- A string, which is any sequence of characters enclosed between quotes or double quotes: in the former case double quotes can appear inside the string, in the latter quotes can appear. No escape characters are provided.
- A number, which is the usual integer/decimal/exponential notation of all programming languages from C on, e.g. `-.001`, `-.001e23`, `123.0` etc.
- An atom that is any consecutive sequence of non space characters that is neither a delimiter, nor a string, nor a number. For example: `atom0`, `0000a0000`, `0x01`, `++` etc.

Atoms and numbers are separated by spaces, delimiters or strings.

**Keep in mind that** `1+2` is a single atom while `1 + 2` is a number, followed by the atom `+` followed by another number.

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

### Number built-in functions

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

Of course, a modern language would allow a lot of built-in mathematical functions, say exponential, logarithm, trygonometric functions and so on, but I want to keep the language small and easy. It'll be simple to extend it to include whatever built-in function we want.

Two functions are provided that will be used to simulate Boolean logic by numbers:

- `MAX x y` that takes two numbers and returns the greatest one, e.g: `MAX -1 0` is `0` and `MAX 100 100` is `100`.
- `MIN x y` that takes two numbers and returns the lowest one, e.g: `MIN 1 0` is `0` and `MIN 100 100` is `100`.

Moreover some comparison functions are provided:

- `EQ x y` (equal) that returns 1 if `x = y` and 0 otherwise.
- `NE x y` (not equal) that returns 0 if `x  = y` and 1 otherwise.
- `LT x y` (less than) that returns 1 if `x < y` and 0 otherwise.
- `LE x y` (less or equal) that returns 1 if `x <= y` and 0 otherwise.
- `GT x y` (greater than) that returns 1 if `x > y` and 0 otherwise.
- `GE x y` (greater or equal) that returns 1 if `x >= y` and 0 otherwise.

### (Lack of) string built-in functions

The current version of Awful does not provide any operation on strings, but the `=` and `<>` comparisons will work also for them (not the other ordering relations).

### Stack built-in functions

The only constant of stack type is `NIL`, the empty stack. To build stacks one uses the `PUSH` built-in function that accepts any value and a stack and pushes the former on top of the latter, as in

    PUSH 1 NIL

that creates a stack with just one element, 1, pushing it on the empty stack. Also

    PUSH 1 PUSH 2 PUSH 3 NIL

creates a stack whose topmost element is 1, next 2 and, at the bottom, 3. Indeed, the leftmost `PUSH` expects two values: the first one is `1`, while the second one starts with another `PUSH`, that expects two values, too: the first one is `2`, while the second one starts with another `PUSH`, that expects two values in turn, the first one being `3` and the second one `NIL`; this latter `PUSH` returns the stack with just `3` inside, where the second `PUSH` pushes `2` on top, and the first `PUSH` pushes `1` on top.

We can represent stacks as lists [x1,...,xn] where x1 is the topmost element. Thus the result of the previous nested `PUSH` is `[1,2,3]`.

Notice that in `PUSH x y` the second parameter `y` ought to be a stack, otherwise an error is raised and the computation ends.

We also have:

- `TOS x` (top of stack) that, given a stack `x`, retrieves its topmost element. For example, if `x = [1,2,3]` then `TOS x` is `1` (a number, not a stack). Notice that `TOS NIL` raises an error.
- `BOS x` (bottom of stack) that, given a stack `x`, returns the stack resulting from `x` by dropping its topmost element. For example, if `x = [1,2,3]` then `BOS x` is `[2,3]`. Notice that `BOS NIL` is `NIL`.

Of course

    PUSH TOS x BOS x

is the same stack as `x`.

Since `TOS` raises an error when applied to the empty stack there is a built-in function that checks against a stack being empty:

- `ISNIL x` returns 1 if `x` is `NIL` else 0.

### Conditional function

The built-in function

    COND x y z

that accepts three values as parameters returns `z` if `x = 0` else returns `y`. For example

    COND EQ 1 2 "False" "True"

returns the string `"True"`.

With this function one can simulate all Boolean functions: for example

- `COND x 0 1` is equivalent to `NOT x`;
- `COND x y 0` is equivalent to `x AND y`;
- `COND x 1 y` is equivalent to `x OR y`,

One could also use arithmetical functions:

- `EQ 0 x` is equivalent to `NOT x`;
- `MIN x y` is equivalent to `x AND y`;
- `MAX x y` is equivalent to `x OR y`,

### Functions

So far, we described an infix notation to deal with standard data types: numbers and stacks.

But the very aim of Awful is to express functions as we described in the first section of this tutorial.

The general syntax is:

    {x1 ... xn : e}

where `x1` ... `xn` are atoms with no special meaning (thus neither constants nor built-in functions) and `e` is a sequence of tokens resulting in a single value.

When Afwul parses such a syntax, it creates a data structure, called *closure*, with the following data:

- The list of formal parameters `[x1,...,xn]` of the function;
- The *body* of the function, thus the sequence of tokens `e`;
- The current environment.

The last item has not been defined yet: let us remedy.

### Environments

A function `{x1 ... xn : e}` is considered to be a value of the language, just like a stack; the only operation we can do on functions is to *evaluate* them. To do that, we need to provide *actual parameters* to be substituted for the formal parameters inside the function body when evaluating it.

One way to do that would be to perform a physical substitution of each formal parameter appearing inside the body with the value of the corresponding actual parameter and next evaluating the resulting expression.

But in practice, a way to do that is to define an *associations list* `[[x1:v1],...,[xn:vn]]` being `x1,...,xn` the formal parameters (that are atoms) and `v1,...,vn` the values of the corresponding actual parameters.

Such an associations list is pushed in a stack, called the *environment*, which is initially `NIL`.

When evaluating the function's body, if an atom is parsed that does not correspond to any constant or built-in function, this atom is searched in the topmost associations list of the stack and, if it is found as first element of a pair `[x,v]`, then `v` is returned as value of the atom; if the atom is not in the topmost associations list is is searched in the following ones, below it, in the stack.

If the atom is not the `x` in any pair `[x,v]` in any associations list of the environment then a "Undefined variable" error is raised.

The environment is a stack of associations lists and not just an associations list itself since functions can be nested.

Let us see an example: consider the function

    {x: ADD x 1}

that increases its argument: we would write it as `f(x) = x+1` in the usual mathematical notation.

Let us suppose the environment to be

    [[x:1,y:0],[x:2]]

To evaluate the function we evaluate the body `ADD x 1`: on parsing `x` we look for it inside the environment, considering the topmost associations list, which is `[x:1,y:0]` in this case. Notice that the value `2` for `x` in the second element of the stack is "shadowed" by the value in the associations list on top of the stack.

### Evaluations

Now the question is: how do we create environments? Awful does not provide any explicit device for that; rather, it allows to *evaluate* a function on a list of actual parameters, according to the syntax

    (f e1, ..., en)

where `f` is the function and `e1, ..., en` are the expressions whose resulting values are the actual parameters to be paired to the formal parameters of `f`.

Suppose the current environment is `E`: when the syntax `(f e1, ..., en)` is parsed, Awful does the following:

- Evaluates `f` that should return a function: if not an error is raised.
- Set the stack `A` to `NIL`.
- Take the formal parameters `x1,...,xn` of `f`.
- For `i = 1` to `n`:
    - evaluate `ei` and pushes the pair `[xi,vi]` (being `vi` the value of `ei`) on the stack `A`.
- Evaluate the body of `f` within the environment `PUSH A E`

If there's a mismatch between the number of formal parameters and the number of actual parameters an error is raised. Notice that the formal parameter list may be empty, in which case to evaluate the function we just write `(f)`.

Example: let us suppose the current environment to be `E` and let us evaluate

    ({x: ADD x 1} 10)

In this case `f = {x: ADD x 1}` and `e1 = 10`; the associations list `A` is just `[[x,10]]` and we push it on `E` getting a stack `E' = [[[x,10]], ...]`.

Within this environment we evaluate the function's body: `ADD x 1`.

When we parse the atom `x` we look for it inside the top of `E'`, which is `A = [[x,10]]` and we find the corresponding value `10` that we return as value of the evaluation of `x`; therefore the result of `ADD x 1` is `11`.

The same applies to functions with more than one parameter, as

    ({x y: ADD POW x 2 POW y 2} 3, 4)

In this case the associations list is `A = [[y,4],[x,3]]` so that when evaluating the function's body `ADD POW x 2 POW y 2` we get `9 + 16 = 25`.

### Functions returning functions

Actually, Awful is quite generous in allowing several formal parameter at once for a single function. Single parameter functions would suffice.

Indeed, consider again the function

    {x y: ADD POW x 2 POW y 2}

that takes two parameters `x` and `y`: now look at

    {x: {y: ADD POW x 2 POW y 2}}

This function has one parameter `x` but its body consists in a function, too, namely `{y: ADD POW x 2 y 2}`.

Thus, if we evaluate

    ({x: {y: ADD POW x 2 POW y 2}} 3)

the result is a function, not a number, namely a function which is equivalent to

    {y: ADD POW 3 2 POW y 2}

i.e. `{y: ADD 9 POW y 2}`.

Therefore, we can evaluate this resulting function, as in

    (({x: {y: ADD POW x 2 POW y 2}} 3) 4)

that returns 25.


### [Mathematical digression]

That a function of several variables

    {x1 ... xn : e}

is equivalent several nested functions of one variable

    {x1: {... {xn: e}... }}

should not be a surprise for people used to the extensional (thus set-theoretical) concept of a function.

Indeed, suppose `f:A -> B` to be a function from a set `A` to a set `B`. The set of all such functions is denoted by `B^A`.

For example, if `A = {1,...,n}` is the set of whole numbers between `1` and `n`, then a function `f:A->B` is just a vector, since it is determined by its values `f(1)=b1`, ..., `f(n)=bn`. Usually we write `B^A=B^n` in this case.

Now consider `A = A1*A2` (cartesian product of two sets), then `f:A->B` is an element of `B^(A1*A2)`; but this set is equivalent (bijective) to `(B^A2)^A1`, that's why the exponential notation is appropriate. Therefore a function `f:A1*A2->B` is equivalent to a function `f:A1->B^A2` thus a function that for each `x in A1` returns a function `f(x):A2->B`; the latter can be evaluated on any element `y in A2` getting `f(x)(y) in B`.

Thus the set of functions `f:A1*A2->B` is equivalent to the set of functions `f:A1->B^A2` which are sometimes written as `f:A1->(A2->B)`. Of course, such an equivalence holds also for several variables: `f:A1*A2*...*An->B` is equivalent to `f:A1->(A2->(...->(An->B)))`.

Applying this equivalence is called `curryfication` in Computer Science, from the name of the great logician Haskell Curry that invented a logical system equivalent to Church's lambda-calculus, expressed in terms of some operators called *combinators*.

We could say anything I've said until now (and anything I'll say later) in the pure language of combinators: see any good lambda-calculus book for that (such as HP Barendregt, *The lambda-calculus, its syntax and semantics*, College Publications, 2012) or also some classic functional programming book (such as MJC Gordon, *Programming language theory and its implementation*, Prentice Hall, 1988).

### Free variables and closures

Let us come back to the nested function evaluation

    (({x: {y: ADD POW x 2 POW y 2}} 3) 4)

If we consider the inner function, `{y: ADD POW x 2 POW y 2}`, we see that in it two variables appear: `x` and `y`. The latter is the formal parameter, while the former is a *free variable*.

When an expression has free variables, the values for these variables should be retrieved from the current environment, otherwise an error occurs.

So we expect to evaluate the inner function within an environment that provides both a value for `x` and a value for `y`.

The solution to this problem comes from the definition of a function we gave some section ago: recall that the syntax

    {x1 ... xn: e}

returns an object that stores the following data:

- the formal parameter list `[x1,...,xn]`;
- the body `e`;
- the current environment.

The latter is just the environment in which the function definition occurs. This is somwhat "frozen" inside the function and provides all possible definitions of free variables within it.

Since an expression with free variables is called *opened* and an expression without free variables is called *closed*, we call *closure* the data structure that adds to the expression the environment containing the definitions for all its free variables.

Now let us see how the Awful expression

    (({x: {y: ADD POW x 2 POW y 2}} 3) 4)

is evaluated. Let us suppose the current environment is `E`.

We match the outer evaluation against `(f e1, ..., en)` and we get:

- `n = 1`
- `f = ({x: {y: ADD POW x 2 POW y 2}} 3)`
- `e1 = 4`

Now we need to compute the value of `f` and `e1` (the latter is trivially `4`): `f` is a function evaluation, too, and we match it against `(f' e'1,...,e'm)`, getting

- `m = 1`
- `f' = {x: {y: ADD POW x 2 POW y 2}}`
- `e'1 = 3`

Now we evaluate `f'`: it is a function, so we take note of:

- its formal parameters list `[x]`;
- its body `{y: ADD POW x 2 POW y 2}`;
- the current environment `E`.

We form the associations list `[[x,3]]` and push it on `E` getting an environment `E'`.

Next we evaluate the body of the function within `E'` and we get a function in turn, determined by the data

- formal parameters list `[y]`;
- body `ADD POW x 2 POW y 2`;
- current environment `E'`.

This closure is the result of the evaluation of `(f' e'1)`, thus the outer function `f` we need to apply to `4`. The latter application occurs by pushing the associations list `[[y,4]]` on the environment of the closure, thus `E'`.

Therefore, we reduce to compute the function

    ADD POW x 2 POW y 2

within the environment `[[[y,4]], [[x,3]], ...]` which results in `ADD POW 3 2 POW 4 2 = 25` as expected.

If all that seems to be complicated, well: it is. However the main point is that when Awful evaluates a function is does it by taking the environment stored in the closure and pushing to it the associations list formed during the evaluation process.

The Awful interpreter takes care of all these stack creations and also takes care to use the correct environment in the correct evaluation.

### What more?

We have almost done: now we have a language in which to express numbers, strings, stacks, functions and operations on them.

It remains just one concept to be considered, but before that, let us switch to a nicer language.

## A nice functional language

>*Syntactic sugar causes cancer of the semicolons.*
>
>Alan J. Perlis
>

The Awful syntax is ugly and cumbersome: let us express the same concepts in a nicer form, by means of Niceful (NICE FUnctional Language).

I've could introduced this language directly, rather than describing Awful first, but the reader will discover why I did it: I do not spoil it now.

Awful let us express functions and expressions but it does not seems to be a programming language at all: it lacks variable to say the least.

Niceful is a pure functional language which is used to write expressions that can be evaluated to a value.

Again, we have numbers, strings, stacks and functions as primitive data types and expressions to deal with them and, again, there is no grammar, but the source text is a sequence of tokens: delimiters, strings, numbers or atoms, the latter separated by spaces or delimiters.

### Numerical expressions

Niceful provides the usual algebraic infix notation for arithmetical operations and relations in contrast to Awful prefix notation.

While numerical constants are the same, we have operators `+ - * / ^` instead of `ADD SUB MUL DIV POW`, relations `= < > <> >= <=` instead of `EQ LT GT NE GE LE` (one can also use `==` instead of `=`). Moreover we have the prefix unary `-` negation operator and the Boolean `not and or` operators.

Operator priorities are as expected, and they can be altered by means of parentheses.

Examples:

- `1 + 2 * 3 ^ 4`
- `(1 + 2) * (3 ^ 4)`
- `x * y <= 0 and y <> 1`
- `not(x == 1 or x = 2)`

Beware that:

- *operators are atoms*, thus they need to be surrounded by spaces or delimiters, the latter being `( ) [ ] , :`. Thus `1+2` is a single atom with unknown value, not the operation `1 + 2`. However, since parentheses and commas are delimiters, `(1 + 2)` is correct as is `(f x + 1,x - 1)`.
- *Boolean operators are not short-circuit ones*, rather they always evaluate both their operands: thus `x = 0 or 1 / x > 0` will result in an error "float division by zero" if `x = 0` since, even if the first operand of `or` would suffice to infer that the value of the Boolean expression is `1` (true), still the second operand `1 / x` is evaluated, too.

### Stack expressions

Niceful provides the familiar notation

    [x1, ..., xn]

for the stack whose topmost element is `x1`, with `x2` below it, ... until the bottomest one which is `xn`. Also the notation `[]` works but one can use the keyword `nil` instead.

There are also the following operators:

- `empty x` that returns 1 if `x` is the empty stack, else 0.
- `1st x` that returns the top of stack `x` (or raises an error if `x` is empty).
- `rest x` that returns the stack `x` after its topmost element has been removed.

To push an element on a stack use the operator

    x : y

where `y` needs to be a stack and `x` can be any value. Of course

    1st x : rest x = x

### Functions

A function is denoted in Niceful with the following syntax:

    fun x1 ... xn : e

being of course `x1,...,xn` its formal parameters and `e` its body: no commas are needed to separate the formal parameters, just spaces.

The only operation on functions is the evaluation, which is denoted as

    f(e1, ..., en)

being `f` the function and `e1,...,en` the actual parameters to match with the formal ones.

For example

    (fun x: x + 1)(10)

result in `11`. Notice that parentheses around the function definition are needed since the body of the function expands as much as possible on the right.

We can nest functions as in Awful, for example

    (fun x: fun y: x ^ 2 + y ^ 2)(3)(4)

Notice that we need to put two arguments `(3)(4)`: the first is for the outer function, the second for the inner. With a single function of two variables we would write:

    (fun x y: x ^ 2 + y ^ 2)(3,4)

Of course, an actual parameter can be any expression.

### Variables

Niceful allows to evaluate an expression in an environment whose variables have been defined as in most programming languages:

    let x1 = e1, ..., xn = en in e

This syntax defines variables `x1,...,xn` initializing them with the values of the corresponding expressions `e1,...,en`: next, expression `e` is evaluated in the environment containing those variable definitions.

For example:

    let x = 3, y = 4 in x ^ 2 + y ^ 2

Notice that such "let-clauses" are genuine expressions, so they can be nested, as in

    let x = 3 in
        let y = x + 1 in
            x ^ 2 + y ^ 2

Also, notice that the following will raise an "Unbounded variable" error:

    let x = 3, y = x + 1 in x ^ 2 + y ^ 2

Indeed variables defined in a same let-clause are assigned "simultaneously", not in sequence as when let-clauses are nested.

Of course, we can assign to a variable any kind of object, as in

    let L = [1,2,3], 2nd = fun x: 1st rest x in 2nd(L)

that returns `2` as result.

### Conditionals

Conditional expressions are provided by Niceful by means of the classic "if-then-else" form:

    if e1 then e2 else e3

For example

    if x < 0 then "Negative"
    else if x = 0 then "Zero"
                    else "Positive"

The nice fact, which does not occur in Awful, is that only one among `e2` and `e3` is evaluated. For example

    if x = 0 then "Error" else 1 / x

won't raise any error if `x = 0` but will return the string `"Error"`. Thus, `if` only evaluates one of the expressions after `then` and `else`, according to the value of the expression after `if`. Namely:

- if `e1` evaluates to zero then `e1` is ignored and `e3` is evaluated and its value returned as value of the all conditional expression;
- if `e1` evaluates to a value different from zero then `e3` is ignored and `e2` is evaluated and its value returned as value of the all conditional expression.

### Recursion

Recursion is the theoretical device, indeed possible by theorems due to Stephen Kleene (and some decades earlier by Richard Dedekind), by which a function is defined in terms of itself.

Actually, recursion works as induction in elementary arithmetics: the latter relies on the fact that natural numbers are generated by 0 just adding 1: from 0 we get 1; from 1 we get 2; and so on. Recursion can be applied to any situation in which we want to perform a computation on some data, we know how to do that in some simple and particular cases and we can break the general case in subcases which at last will reduce to the simple ones.

So, a recursive function should settle a trivial case in which its value is stated explicitly for certain values, and a general case in which the value is computed in terms of simpler values applied to the same function.

As an example, let us write a recursive function to compute the length of a stack `x`: the trivial case is when the stack is empty, `x = nil`, since then its length is `0`. If `x` is not `nil` it will be formed by a topmost element and a stack of remaining elements: the latter is intuitively shorter than `x`, so that we suppose to know its length by a recursive call and increase it by one to take into account the first element of `x`.

    fun x: if empty x then 0
           else 1 + "recursive call"(rest x)

The string "recursive call" denotes a call to the same function we are defining, but on a different parameter, `rest x` which is "simpler" than `x`.

To write this definition in Niceful we need to give a name to our function: the temptation is to write

    let len = fun x: if empty x then 0
                     else 1 + len(rest x)
    in len([1,2,3])

But if you try to type this in the Niceful interpreter you'll get an `Unbounded variable` error: in fact, `len` is not yet defined when the function is under definition!

Recall that `let x1 = e1, ..., xn = en in e` defines all variables simultaneously: thus the expressions `e1,...,en` are evaluated before any of the variables `x1,...,xn` are assigned.

Thus, it is impossible to write a recursive definition by means of a let-clause.

But Niceful is nice enough to provide an alternative: `letrec`. Try to type (beware that the interpreter expects an expression on a single line, or in multiple lines terminated by backslashes, I omit the latter here for readability)

    letrec len = fun x: if empty x then 0
                        else 1 + len(rest x)
    in len([1,2,3])

and you'll get 3!

Indeed, the expression `len([1,2,3])` is converted into the following:

1. `1 + len(rest [2,3])`
2. `1 + (1 + len(rest [3]))`
3. `1 + (1 + (1 + len(rest [])))`
4. `1 + (1 + (1 + 0))`
5. `1 + (1 + 1)`
6. `1 + 2`
7. `3`

Notice that at each recursive call, a `1` summand is added plus `len` applied to a shorter list; when `len` is applied to the empty list the `then` branch of the function is evaluated returning `0`.

To prevent to smash the runtime stack by an infinite loop of recursive calls, Awful has an inner counter that raises an error `Evaluation too nested` when nested evaluations inside an expressions exceed a given amount, by default 1024 (you can change this in the code): this is a feature of the C interpreter only.

### Surprise ending

It seems that Niceful is not only a true programming language, with a familiar and easy syntax, but that it is actually more powerful than Awful. But this is false.

Indeed, a Niceful expression can always be translated into an Awful one: in fact, the Niceful interpreter perform this translation and then invokes the Awful interpreter to evaluate the resulting Awful expression.

To describe such a translation is the purpose of the next section.

## Niceful translations

>*To iterate is human, to recurse divine.*
>
>LP Deutsch
>

Let us define a function `T:S->S` from the set of strings to the set of strings that, when applied to a legal Niceful expression, returns the corresponding Afwul expression. We have not defined formally what an Awful or Niceful expression is, however we can easily list all possible forms of Niceful expression.

### Patterns of Niceful expressions

The bare list of all possible Niceful expressions follows: notice that this is a recursive definition (we write `e,e1,...` to denote any expression, `x,x1,...` to denote any atom, `n` any number and `s` any string).

- `n`
- `s`
- `nil`
- `[e1,...,en]`
- `fun x1 ... xn: e`
- `e1 + e2`
- `e1 - e2`
- `e1 * e2`
- `e1 / e2`
- `e1 ^ e2`
- `e1 : e2`
- `e1 = e2`
- `e1 <> e2`
- `e1 < e2`
- `e1 >= e2`
- `e1 > e2`
- `e1 <= e2`
- `e1 and e2`
- `e1 or e2`
- `not e`
- `- e`
- `empty e`
- `1st e`
- `rest e`
- `e(e1,...,en)`, the evaluation of the function resulting from `e` on `e1,...,en`.
- `(e)`, this has the same value of `e` but is needed to force operator priorities.
- `if e1 then e2 else e3`
- `let x1 = e1, ..., xn = en in e`
- `letrec x1 = e1, ..., xn = en in e`

### The Niceful transform

Now let us define `T(s)` for each string of one of the previous forms.

- `T(n) = n`
- `T(s) = s`
- `T(nil) = NIL`
- `T([e1,...,en]) = PUSH T(e1) PUSH ... PUSH T(en) NIL`
- `T(fun x1 ... xn: e) = {x1 ... xn: T(e)}`
- `T(e1 + e2) = ADD T(e1) T(e2)`
- `T(e1 - e2) = SUB T(e1) T(e2)`
- `T(e1 * e2) = MUL T(e1) T(e2)`
- `T(e1 / e2) = DIV T(e1) T(e2)`
- `T(e1 ^ e2) = POW T(e1) T(e2)`
- `T(e1 : e2) = PUSH T(e1) T(e2)`
- `T(e1 = e2) = EQ T(e1) T(e2)`
- `T(e1 <> e2) = NE T(e1) T(e2)`
- `T(e1 < e2) = LT T(e1) T(e2)`
- `T(e1 >= e2) = GE T(e1) T(e2)`
- `T(e1 > e2) = GT T(e1) T(e2)`
- `T(e1 <= e2) = LE T(e1) T(e2)`
- `T(e1 and e2) = MIN T(e1) T(e2)`
- `T(e1 or e2) = MAX T(e1) T(e2)`
- `T(not e) = SUB 1 T(e)`
- `T(- e) = SUB 0 T(e)`
- `T(empty e) = ISNIL T(e)`
- `T(1st e) = TOS T(e)`
- `T(rest e) = BOS T(e)`
- `T((e))) = e` (indeed parentheses are useless in Awful since all operators are prefixed)
- `T(e(e1,...,en)) = (T(e) T(e1), ..., T(en))`.
- `if e1 then e2 else e3 = (COND T(e1) {:T(e2)} {:T(e3)})` (we wrap `e2` and `e3` inside functions and evaluate the result of `COND`, so that only the appropriated function will be evaluated)
- `T(let x1 = e1, ..., xn = en in e) = ({x1 ... xn: T(e)} T(e1), ..., T(en))`

Notice the latter: a let-clause is just another way to write the evaluation of a function!

It remains to deal with letrec-clauses: for that we need to introduce a final Awful notion I purposely avoided when describing the Awful notation.

### Delayed evaluation in Awful

Let us come back to Awful function evaluations:

    ({x1 ... xn: e} e1, ..., en)

Recall the algorithm of evaluation:

- Parse the list `[x1, ..., xn]`.
- Set the stack `A` to `NIL`.
- For `i = 1` to `n`:
    - Evaluate `ei` in the current environment to get a value `vi` and push the pair `[xi,vi]` into `A`.
- Push `A` to the current environment to form a new environment `N`.
- Evaluate `e` w.r.t. `N` getting the result `v` which is the value of the function on the given actual parameters.

Therefore, when evaluating `e1, ..., en` the variables `x1, ..., xn` are still not defined (unless some variables with the same name are already in the current environment).

Now let us extend the notation for an Awful function to allow the insertion of a `!` in front of a formal parameter: for example

    {x !y: ADD x y}

When a parameter is prefixed by the exclamation mark `!` then, when the function is evaluated, its corresponding actual parameter is not evaluated but just parsed, and evaluated only after all others parameters have been parsed.

So the evaluation algorithm changes to:

- Parse the list `[x1, ..., xn]`.
- Set the stack `A` to `NIL`.
- For `i = 1` to `n`:
    - If `xi` was marked by a `!` in the function definition then parse `ei` without evaluating it getting a token list (just a stack of symbols) `ti`, next push the pair `[xi,ti]` on `A`.
    - Else evaluate `ei` in the current environment to get a value `vi`, next push the pair `[xi,vi]` on `A`.
- Push `A` to the current environment to form a new environment `N`.
- For `i = 1` to `n`:
    - If `xi` was marked by a `!` in the function definition then evaluate `ti` w.r.t. environment `N` getting a value `vi` which is overwritten to `ti` in the pair `[xi,ti]` in environment `A`.
- Evaluate `e` w.r.t. `N` getting the result `v` which is the value of the function on the given actual parameters.

For example let us evaluate

    ({x !y: ADD x y} 10, ADD x 5)

in an empty current environment `E = NIL`.

- Parse the formal parameters: `[x,!y]`.
- Set `A` to `NIL`.
- Evaluate the first actual parameter `10` and push `[x,10]` on `A`, getting `A = [[x,10]]`.
- Parse but not evaluate (since `y` is marked!) the second actual parameter `ADD x 5` and push `[y,"ADD x 5"]` on `A`, getting `A = [[y,"ADD x 5"],[x,10]]`.
- Add `A` to the current environment `E = NIL` to form a new environment `N = [A]`.
- Evaluate `"ADD x 5"` within `N` getting the value `15` and overwrite the latter as second element of the pair `[y,"ADD x 5"]` in `A`, getting `N = [[y,15],[x,10]]`.
- Evaluate `"ADD x y"` in the environment `N`, getting `25` as result.

In other terms, marking a variable in a function definition forces the interpreter to delay the evaluation of the corresponding actual parameter until all parameters have been parsed and paired to the formal parameters.

In this way, we can include one of the formal parameters in an actual parameter whose corresponding formal parameter has been marked in the function definition.

### Translation of `letrec`

Now let us translate the "letrec"-clause

    letrec x1 = e1, ..., xn = en in e

as

    ({!x1 ... !xn:e} e1, ..., en)

In this way we can quote any of the `x1, ..., xn` inside the `e1, ..., en`.

In particular this allows recursive definition, since for example

    letrec len = fun x: if empty x then 0
                        else 1 + len(rest x)
    in len([1,2,3])

would be translated as

    ({!len:(len PUSH 1 PUSH 2 PUSH 3 NIL)} {x:(COND ISNIL x{:0}{:ADD 1 (len BOS x)})})

Since the formal parameter `len` is marked, it can appear inside the actual parameter, which is a function, precisely `len` itself.

## Examples

>*Programs must be written for people to read, and only incidentally for machines to execute.*
>
>H Abelson, JH Sussman
>

To conclude this introduction let me show some examples of Niceful programs. Most of these examples are taken from classic papers and books on functional programming.

### Using the interpreter

To try the examples and build more you'll need to use the interpreter: you may choose either the Python or the C one, the previous easier to set up (see folders [../python/](../python/) and [../c/](../c/)).

Once an interpreter has been set up and launched, you'll see somthing as

    AWFUL - A Weird FUnctional Language
    (c) 2023 by Paolo Caressa <github.com/pcaressa/awful>
    [v.0.2312. Type 'help' for... guess what?]

    niceful 1: 

Then you can type the expression to be evaluated on on line and the interpreter will translate it into Awful and call the Awful interpreter to evaluate it, printing the result (you could use directly the Awful interpreter in the same way, here I show example of Niceful programs).

For example

    niceful 1: let x = 10, f = fun x: x + 1 in f(x)
    11

If you want to see the translation from Niceful into Awful just start the expression with `awful` word:

    niceful 2: awful let x = 10, f = fun x: x + 1 in f(x)
    ({x f:(f x)}10,{x:ADD x 1})
    niceful 3:

If you want to write directly in Awful, type just

    niceful 3: awful
    Awful interpreter
    awful 4: 

From now on only Awful expressions will be interpreted: to switch back to Niceful, type `niceful` at the prompt.

Notice that a progressive integer is shown by the prompt: this is useful in batch files. Indeed, instead of typing lines you could put them into a text file `foo.nfl` (or whatever you want to name it) and type

    batch foo.nfl

Each line of the file will be evaluated and its result printed on the terminal. For example suppose `foo.nfl` to contain

    1
    4 / 2
    4 - 1
    4 + 0.0000000000000001

and that the file is in the same folder where you are running Niceful; then 

    niceful 5: batch foo.nfl
    foo.nfl 1: 1
    foo.nfl 2: 2
    foo.nfl 3: 3
    foo.nfl 4: 4

If you want the output of the interpreter to be redirected on a file, just use

    niceful 6: output foo.log

After that, file `foo.log` is opened in the append mode (if the file does not exist then it is created) and outputs are printed on it. For example consider

    niceful 6: output foo.log
    niceful 7: batch foo.nfl
    foo.nfl 1: foo.nfl 2: foo.nfl 3: foo.nfl 4: foo.nfl 5: niceful 8: output
    niceful 9: 

After the `output foo.log` command has been issued, no prompt is printed anymore: however, the interpreter still asks for a text, and we insert `batch foo.nfl`. Next the interpreter prints the prompts of the lines evaluated in the batch file, but not theirs results, that are dumped on the file `foo.nfl`. Finally, we just state `output` with no parameter which switch back the output to the terminal.

Another way to use script files is by means of *preludes*: a prelude is a script whose lines are considered to be a single expression which is usually not completed; after the processing of the prelude, the interpreter asks for a text to complete the expression.

Consider for example the file `foo.pre` whose content is

    let inc = fun x: x + 1,
        dec = fun x: x - 1,
        twice = fun x: x * 2,
        half = fun x: x / 2
    in

`foo.pre` doesn't contain a legal Niceful expression since the interpreter expects something after `in`. Still, you can use this file as in

    niceful 6: prelude foo.nfl
    niceful 6: inc(twice(dec(half(10))))
    9
    niceful 7:

Notice that the line of the prelude and the line that follows it are considered a single line (same progressive number).

To use examples hereafter put them in preludes.

To leave the interpreter use `bye`.

### Numeric examples

Some simple arithmetical functions:

    abs = fun x: if x < 0 then - x else x,
    sgn = fun x: if x < 0 then -1 else
                 if x = 0 then 0 else 1

To keep the language as simple as possible (but not simpler) I didn't include many usual arithmetical and numerical functions which it would be easy to include. Let us program some of them.

Consider the `floor` function which should take a number `n` and return the largest integer `<= n`. This is provided by hardware, but we can crazily program it as follows: 

    int-near = fun n x:
        if n > x then int-near(n - 1, x)
            else
        if n + 1 < x then int-near(n + 1, x)
            else
        n

Thus, `int-near(n,x)` does the job if we call it with `n` any integer and we can define

    floor = fun x: int-near(0,x),
    round = fun x: int-near(0, x + 0.5)

This technique of using more parameters than needed to provide a sort of local variables for a function is common in functional languages.

Let us implement, by an inefficient but well-known method, the exponential function: we will use the series

    exp(x) = 1 + x + (x^2)/2 + (x^3)/6 + (x^4)/24 + (x^5)/120 + ...

To evaluate the polynomial `1 + x + (x^2)/2 + (x^3)/6 + (x^4)/24 + (x^5)/120` we use the Horner scheme, thus we rearrange terms by a cleverly using the distributive property:

    1 + x + (x^2)/2 + (x^3)/6 + (x^4)/24 + (x^5)/120 =
    1 + x*(1 + x/2 + (x^2)/6 + (x^3)/24 + (x^4)/120) =
    1 + x*(1 + x/2)*(1 + x/3 + (x^2)/12 + (x^3)/60)) =
    1 + x*(1 + (x/2)*(1 + (x/3)*(1 + x/4 + (x^2)/20))) =
    1 + x*(1 + (x/2)*(1 + (x/3)*(1 + (x/4)*(1 + x/5))))

We could define

    exp-bad x = fun x: 1 + x * (1 + (x / 2)*(1 + (x / 3)*(1 + (x / 4)*(1 + x / 5))))

but, as the function name suggests, for numbers not near to 1 this function won't work properly: for example

    niceful 10: exp-bad(10)
    1477.67

while we should get something as `22026.4657948`. The problem is that we should consider a longer segment of the Taylor series.

Let's write a recursive function to compute the approximation up to the `n`-th power for a given `n > 0`:

    exp-n = fun x n N:
        if n = N then 1 + x / n
        else 1 + (x / n) * exp-n(x,n + 1,N),
        
    exp = fun x : exp-n(x,1,50)

Notice that function `exp-n` recurs from the last term of the Taylor series: since the higher term in Horner scheme is the more nested one, we start by computing it and going downward until the first term. 

To do that, the simple case of the recursion correspond to the maximum value for the power `x^n` which is for `n = N`. For example evaluate `exp-n(10,1,3)`:

    exp-n(10,1,3) -> 1 + (10 / 1) * exp-n(10,2,3)
    exp-n(10,2,3) -> 1 + (10 / 2) * exp-n(10,3,3)
    exp-n(10,3,3) -> 1 + (10 / 3)

Therefore

    exp-n(10,1,3) = 1 + (10 / 1) * exp-n(10,2,3)
        = 1 + (10 / 1) * (1 + (10 / 2) * exp-n(10,3,3))
        = 1 + (10 / 1) * (1 + (10 / 2) * 1 + (10 / 3))

thus exactly the computation of the Horner scheme.

With these definitions we get the value `22026.5` for `exp(10)`. Not bad.

### Stack examples

Let me shown some simple function definitions providing useful list tools: they are collected in the `list.pre` prelude (check the [../preludes/](../preludes/) folder). So, in each definition I'll show, imagine a `letrec` preceding it and a `in ...` following it.

The following computes the length of a list.

    len = fun x:
        if empty x then 0
        else 1 + len(rest x)

To get the n-th element of a list (with indexing from `0` to `n-1` being `n` the length of the list), use

    nth = fun x n:
        if empty x or n < 0 then "Index out of range"
            else
        if n = 0 then 1st x
            else
        nth(rest x, n - 1)

Notice that an error message is returned instead of the actual value in case of error.

An important operation among lists is *append*: remember that our lists are actually stacks, so that we'll need to get to the last element of the first list.

    append = fun x1 x2:
        if empty x1 then x2
            else
        if empty rest x1 then 1st x1 : x2
            else
        1st x1 : append(rest x1, x2)

Here we deal "by hands" with the case `x1 = []` and `x1 = [a]`; if `x1 = [a,b,...]` then we reduce to the simpler case `x1 = [b,...]` and push `a` in front of the result.

**Notice.** Most functional languages provide pattern matching to express these `if-else` implicitly: often one could write

    append = fun [] x2:  x2
           |     [a] x2: a:x2
           |     a:x1 x2: a:append(x1, x2)

This would be a nice improvement to `Awful` :-).

By means of `append` let us write a function to invert the order of elements in a list:

    reverse = fun x:
        if empty x then nil
        else append(reverse(rest x), [1st x])

### Miscellaneous examples

Let us consider an example of *mutual recursion*: consider the following pair of functions implementing a test for even and odd numbers:

    even = fun n:
        if n < 0 then even(- n)
            else
        if n = 0 then 1
            else
        odd(n - 1),

    odd = fun n:
        if n < 0 then odd(- n)
            else
        if n = 0 then 0
            else
        even(n - 1)

These will work only for non negative numbers: for example, the evaluation of `odd(3)` proceeds as follows:

    odd(3) -> even(2) -> odd(1) -=> even(0) -> 1

Of course one could directly write in this case

    even = fun n:
        if n = 0 then 1
            else
        if n = 1 then 0
            else
        even(n - 2)

But I wanted to make a simple example of mutual recursion.

Let me finally consider some functions that may improve somehow the language itself.

First we have the function `map(f,x)` that applies a function `f` to each element of a list `x` returning the resulting list:

    map = fun f x:
        if empty x then nil
        else f(1st x) : map(f, rest x)

For example consider

    letrec
    map = fun f x:
        if empty x then nil
        else f(1st x) : map(f, rest x)
    in
        map(fun x: x * x, [1,2,3,4,5])

whose result is `[1,4,9,16,25]`.

We can also select elements from a stack according to a condition, encoded in function returning 0 if we want to discard an element, 1 (or also non zero) if we want to keep it:

    filter = fun f x:
        if empty x then nil
            else
        if f(1st x) then 1st x : filter(f, rest x)
            else
        filter(f, rest x)

For example the evaluation of

    letrec
    append = fun x1 x2:
        if empty x1 then x2
            else
        if empty rest x1 then 1st x1 : x2
            else
        1st x1 : append(rest x1, x2),
    filter = fun f x:
        if empty x then nil
            else
        if f(1st x) then 1st x : filter(f, rest x)
            else
        filter(f, rest x),
    even = fun n:
        if n < 0 then even(- n)
            else
        if n = 0 then 1
            else
        odd(n - 1),
    odd = fun n:
        if n < 0 then odd(- n)
            else
        if n = 0 then 0
            else
        even(n - 1)
    L = [1,2,3,4,5,6,7,8,9,10]
    in
        append(filter(odd,L), filter(even,L))

will result in

    [1,3,5,7,9,2,4,6,8,10]

Just for curiosity, the Awful expression corresponding to this Niceful one is:

    ({!append !filter !even !odd !L:(append (filter odd,L),(filter even,L))}{x1 x2:(COND ISNIL x1{:x2}{:(COND ISNIL BOS x1{:PUSH TOS x1 x2}{:PUSH TOS x1 (append BOS x1,x2)})})},{f x:(COND ISNIL x{:NIL}{:(COND (f TOS x){:PUSH TOS x (filter f,BOS x)}{:(filter f,BOS x)})})},{n:(COND LT n 0{:(even SUB 0 n)}{:(COND EQ n 0{:1}{:(odd SUB n 1)})})},{n:(COND LT n 0{:(odd SUB 0 n)}{:(COND EQ n 0{:0}{:(even SUB n 1)})})}, PUSH 1 PUSH 2 PUSH 3 PUSH 4 PUSH 5 PUSH 6 PUSH 7 PUSH 8 PUSH 9 PUSH 10 NIL)

As a final example, let us see how the `filter` function is powerful, in letting us to express very easily the classic Quicksort algorithm:

    letrec
    @ = fun x1 x2:              \ append
        if empty x1 then x2
            else
        if empty rest x1 then 1st x1 : x2
            else
        1st x1 : @(rest x1, x2),

    ? = fun f x:                \ filter
        if empty x then nil
            else
        if f(1st x) then 1st x : ?(f, rest x)
            else
        ?(f, rest x),

    quicksort = fun x:
        if empty x then nil
            else
        let a = 1st x, r = rest x
            in @(quicksort(?(fun y: y < a, r)),
                  @([a], quicksort(?(fun y: a <= y, r))))

    in quicksort([3,2,1,4])

Notica that, by a nested `let`, we defined two "local variables" `a` and `r` to avoid repeated evaluation of `1st x` anf `rest x` inside the quicksort recursive definition.

## Odds and ends (mostly ends)

The pure functional language I defined in these notes, Niceful, relies upon a formalism to express stacks and closures, Awful, which is very simple although quite obfuscated.

To let the language somewhat useful one should add at least input/output, string manipulation and a device to catch exceptions that extends inner errors: actually the language should be completed so that an interpreter for it could be programmed in itself.


Next, one could explore other techniques in functional programming such as tail-recursion, lazy evaluation, continuations, more environment implementations etc.

But for the moment I stop here.

(c) 2023 by Paolo Caressa [<github.com/pcaressa/awful>]([<github.com/pcaressa/awful>)
