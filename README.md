# AWFUL - A Weird FUnctional Language

#### (c) 2023 by Paolo Caressa

## Introduction

Awful is a simple functional language designed and implemented for didactic purposes: its main motivation was to understand and to replicate ideas by Peter J Landin borrowed from his papers from early '60s.

Awful provides the bare minimum equipment for a functional environment:

- atomic data types: numbers, strings and atoms; atoms are used to refer to other objects;
- structured data types: stacks and closures.

The language comes in two layers:

- Awful, the *representation language*;
- Niceful, the *publication language*.

The latter is addressed to humans wanting to use the language, the former is addressed to machines to execute it.

One may thinks to representation language as the "inner form" and to publication language as "syntactic sugar" built on top of the inner form.

Currently the status of the project is the following:

- Documentation: this reference file plus a tutorial in [doc/](doc/).
- Python implementation: [python/](python/) folder containing a series of Python scripts with a complete implementation: see below for the usage (see also [python/README.md](python/README.md)).
- C implementation: [c/](c/) folder folontaining C headers and sources to be compiled into a single executable; see [c/README.md](c/README.md/) for usage.

TODO:

- Niceful implementation in Niceful, used to self-implement Niceful using the Awful interpreter.
- Afwul to C translator.

Enjoy!
Paolo

## Awful: A Weird FUnctional Language

Awful provides a Lisp-like (but parentheses-less) formalism to encode values and expressions.

### Using the interpreter

Currently two interpreters are available: a Python version, written in a non idiomatic Python aimed at portability, which served for prototyping purposes and a C version, that should be considered *the* implementation.

#### Python version

The Python prototype is simple to use: you'll need the Python environment set up, version 3; no packages other than the built-in ones are used.

Just store the folder [py] somewhere in the PATH and launch

    python3 awful

In the first place, you can launch this command from inside the folder [py/] itself.

#### C version

The C interpreter ought to be compiled before being used: you'll need a C compiler such as `clang` or the one coming with `MS Visual Studio` etc. Some compilers, such as `clang`, explicitly need to load math libraries when compiling.

The [c/] folder contains all source files needed to compile the interpreter: see the [c/README.md] for instructions.

For example you could type, inside the [c/src/]:

    clang -lm -DAWFUL *.c -o awful

to create an executable for the Awful language (of course, add all compiler options you like). The executable awful can now be launched to execute the interpreter:

    ./awful

### Interacting with the interpreter

After launching the interpreter, a prompt will appear:

    AWFUL - AWful FUnctional Language
    (c) 2023 by Paolo Caressa <github.com/pcaressa/awful>

    Type: 'bye' to leave, 'batch FILENAME' to process a file
    Lines starting with backslash or empty are skipped
    A line ending with backslash is joined to the following one

    awful 1:

Now you can type an Awful expression and get the result computed: if the expression needs to span over multiple lines then add a final backslash to the line and the interpreter will ask for more, as in

    awful 1: ADD 1       \
    awful 2|     MUL 2 3
    7
    awful 3: 

To leave the interpreter type `bye`, to evaluate a file whose lines contain single Awful expressions use `batch FILENAME`; text after a backslash will be ignored, so that one can insert comments in a batch file in this way.

For example suppose the `sample.awf` text file contains

    \ Example of function application
    ({x:ADD x 1} 10)

    ADD\
    19\
    21

Then, we could write

    awful 1: batch sample.awf
    11
    40
    awful 2:

Inside a script one can use the `bye` and the `batch` directives, too.

### Awful syntax

An expression is just a sequence of token: a token can be

- a number: a decimal/exponential notation representing a floating point number.
- a string: an immutable character sequence enclosed between double quotes and not containing double quotes or enclosed between quotes and not containing quotes.
- a delimiter: parentheses, braces, comma and colon.
- a keyword: one of the symbols `ADD BOS COND DIV EQ GE GT LE LT MAX MIN MUL NE POW PUSH SUB TOS`.
- an atom: a contiguous sequence of non space characters and non delimiter characters which is neither a number nor a keyword.

An expression is a sequence of token matching one of the following rules:

- A single number, string or atom.
- A keyword followed by a sequence of tokens encoding expressions needed as actual parameters of the keyword.
- `{` followed by a possible empty sequence of atoms, each one possibly preceded by `!`, followed by `:`, followed by an expression, followed by `}`.
- `(` followed by an expression, possibly followed by a sequence of comma-separated expressions`)`

The number of expressions that need to follow a keyword is:

- 1 for `BOS ISNIL TOS`.
- 2 for `ADD DIV EQ GE GT LE LT MAX MIN MUL NE POW PUSH SUB`.
- 3 for `COND`.

### Awful semantics

The behavior of keywords when they are executed is the following: by *e* we mean any expression, by *n* any expression whose value is a number, by *s* any expression whose value is a stack. It is understood that if *n* or *s* are used, the language interpreter check against the type of the value and raises an error if the type is not the one expected.

- the value of `ADD` *n1* *n2* is *n1* + *n2*;
- the value of `BOS` *s1* is *s1* deprived of its first element;
- the value of `COND` *n e1 e2* is *e1 if *n* is not zero, else *e2*;
- the value of `DIV` *n1 n2* is *e1 / e2*;
- the value of `EQ` *e1 e2* is 1 if *e1 = e2*, else 0;
- the value of `GE` *n1 n2* is 1 if *n1 >= n2*, else 0;
- the value of `GT` *n1 n2* is 1 if *n1 > n2*, else 0;
- the value of `ISNIL` *n1* is 1 if *n1 = NIL*, else 0;
- the value of `LE` *n1 n2* is 1 if *n1 <= n2*, else 0;
- the value of `LT` *n1 n2* is 1 if *n1 < n2*, else 0;
- the value of `MAX` *n1 n2* is *n1* if *n1 > n2*, else *n2*;
- the value of `MIN` *n1 n2* is *n1* if *n1 < n2*, else *n2*
- the value of `MUL` *n1 n2* is *n1 / n2*;
- the value of `NE` *e1 e2* is 0 if *e1 = e2*, else 1;
- the value of `POW` *n1 n2* is *n1* raised to *n2*;
- the value of `PUSH` *e s* is the stack obtained by *s* by pushing *e* on top of it;
- the value of `TOS` *s* is the top of the stack *s*.

A function object `{` *x1 ... xn* `:` *e* `}` is a value in itself but it can also be applied to a sequence of expression, matching in number the number of *formal parameters x1 ... xn* of the function. The expression *e* is called the *body* of the function. When a function definition is evaluated, its value is a triple (*pars, body, fenv*) where *pars* is the list of the formal parameters (each one with a flag true if the variable was marked by a `!`), *body* is an expression and *fenv* is an environment (see below) that is the current one at the moment of the definition.

### Environments and evaluations

An environment is a stack of associative lists: each such list contains a sequence of pairs (*x,v*) where *x* is an atom (variable) and *v* any value. When an atom is parsed inside an expression, its value is searhed inside those associative lists, starting from the one on top of the environment stack.

At start, the environment is the empty stack. Each time a function evaluation

`(`*f e1, ..., en*`)`

is encountered, the following algorithm is performed: keep in mind that a function can be parameterless, in which case its evaluation is simply `(`*f*`)` so *n* = 0 in what it follows.

- *f* is evaluated:
- if its value is not a function an error is raised;
- else, let *x1 ... xn* be its parameters, *f1 ... fn* flags such that *fi* is true if *xi* was marked by a `!` in the function definition, *body* the function body and *fenv* the environment enclosed with the function at the moment of its definition.
- Set *e1* to the empty list
- For each *i* = 1 ... n:
    - if *fi* is false, then an expression is parsed and evaluated to a value *vi*, next the pair (*xi*, (1, *vi*)) is pushed on *e1*.
    - else, a sequence *s* of tokens is parsed until the next `,` or `)` (nested parentheses and braces are taken into account) and the pair (*xi*,(0,*s*)) is pushed on *e1*.
- Push *e1* on *e*.
- For each element (*xi*,(*flagi*,*yi*)) in *e1*:
    - if *flagi* is 0 then *yi* is evaluated to a value *vi* and (*xi*,(0, *yi*)) is changed to (*xi*, *vi*);
    - else, (*xi*, (1, *vi*)) is changed to (*xi*,*vi*)
- The body of the function is evaluated: if a variable is to be evaluated inside this body then:
    - if it is one of *xi* then its value is retrieved by *e1*;
    - if not, its value is searched first in *fenv* and next in *e*;
    - if no value can be associated to the variable, an error is raised.
- *e1* is popped from *e*, restoring it as it was before the evaluation took place. 

Notice that `(f e1, ..., en)` always require `f` to result in a function: for example

    (ADD 1, 2)

is wrong, since it matches `f=ADD`, `e1=1` and `e2=2`: but `f` should be a complete expression, while `ADD` is not complete if not followed by two expressions in turn. But also writing 

    (ADD 1 2)

will result in an error, since the interpreter evaluates `ADD 1 2` getting 3 and tries to interpret is as a function, which is not.

The moral is: use parentheses only to evaluate functions, not to improve readibility of code. Awful code is awful.

## Niceful: an NICE FUnctional Language

Niceful provides a ML-like formalism to encode values and expressions.

### Using the interpreter

Currently two interpreters are available: a Python version, written in a non idiomatic Python aimed at portability, and a C version, that should be considered the reference implementation.

#### Python version

The Python prototype is simple to use: you'll need the Python environment set up, version 3; no packages other than the built-in ones are used.

Just store the folder [python/](python/) somewhere in the PATH and launch

    python3 niceful

In the first place, you can launch this command from inside the folder [python/](python/) itself.

#### C version

The C interpreter ought to be compiled before being used: you'll need a C compiler such as `clang` or the one coming with `MS Visual Studio` etc. Some compilers, such as `clang`, explicitly need to load math libraries when compiling.

The [c/] folder contains all source files needed to compile the interpreter: see the [c/README.md] for instructions.

For example you could type, inside the [c/src/]:

    clang -lm *.c -o niceful

to create an executable for the Niceful language (of course, add all compiler options you like). The executable awful can now be launched to execute the interpreter:

    ./niceful

### Interacting with the interpreter

After launching the interpreter, a prompt will appear:

    NICEFUL - NICE FUnctional Language
    (c) 2023 by Paolo Caressa <github.com/pcaressa/awful>

    Type: 'bye' to leave, 'batch FILENAME' to process a file
    Lines starting with backslash or empty are skipped
    A line ending with backslash is joined to the following one

    niceful 1: 

Now you can type an Awful expression and get the result computed: if the expression needs to span over multiple lines then add a final backslash to the line and the interpreter will ask for more, as in

    niceful 1: 1 +\
    niceful 2|     2 * 3
    7
    niceful 3: 

To leave the interpreter type `bye`, to evaluate a file whose lines contain single Awful expressions use `batch FILENAME`; text after a backslash will be ignored, so that one can insert comments in a batch file in this way.

For example suppose the `sample.nfl` text file contains

    \ Example of function application
    let inc = fun x: x + 1 in inc(10)

    19\
    +\
    21

Then, we could write

    niceful 1: batch sample.nfl
    11
    40
    niceful 2:

Inside a script one can use the `bye` and the `batch` directives, too.

### Niceful syntax

We can express Niceful syntax by means of the following grammar written in the old good BNF style symbols of the language are enclosed between backslashes, that are not part of the language alphabet:

    expression =
        conditional |
        \let\ assignments \in\ expression |
        \letrec\ assignments \in\ expression

    assignments = atom \=\ fun-expr {\,\ atom \=\ fun-expr}

    conditional =
        proposition |
        \if\ proposition \then\ expression \else\ expression

    proposition = relation {bool-op proposition}
    
    relation =
        sum [ rel-op sum ] |
        \not\ relation
    
    sum = product { add-op product }
    
    product = term { mul-op term }
    
    power = term [\^\ term]
    
    term = atom | string | list | \-\ term | \1st\ term | \rest\ term | \empty\ term
        \(\ expression \)\ | term { \(\ [expr-list] \)\ } |
        \fun\ {\!\ atom} \:\ expression
    
    expr-list = expression {\,\ expression}

    bool-op = \and\ | \or\
    rel-op = \=\ | \<>\ | \<\ | \>\ | \<=\ | \>=\
    add-op = \+\ | \-\ | \:\
    mul-op = \*\ | \/\

    string = \"\{character}\"\ | \'\{character}\'\
    list = \[\ \[\ | \[\ [expr-list] \]\

Each Niceful syntactic construction can be translated into a corresponding Awful expression or part of expression: thus Niceful is just a different form in which to express Awful expressions.

### Niceful translation function

For each kind of Niceful construct we will now specify the corresponding Awful expression. More precisely, let us define a function *y* = T(*x*) that translates a Niceful expression *x* into an Awful expression *y*.

We will define T on all possible expressions as defined by the previous grammar:

- If *x* is a number, string or atom then T(*x*) = *x*.
- T(`[]`) = `NIL`
- T(`[`*e1* `,` ... `,` *en* `]`) = `PUSH` T(*e1*) `PUSH` T(*e2*) ... `PUSH` T(*en*) `NIL`
- T(`fun` *x1 ... xn* `:` *e*) = `{` *x1 ... xn* `:` *e* `}` 
- T(`-` *e*) = `SUB 0 ` T(*e*)
- T(`1st` *e*) = `TOS` *e*
- T(`rest` *e*) = `BOS` *e*
- T(`empty` *e*) = `ISNIL` *e*
- T(`(`*e*`)`) = T(*e*)
- T(*e1* `^` *e2*) = `POW` T(*e1*) T(*e2*)
- T(*e1* `*` *e2*) = `MUL` T(*e1*) T(*e2*)
- T(*e1* `/` *e2*) = `DIV` T(*e1*) T(*e2*)
- T(*e1* `+` *e2*) = `ADD` T(*e1*) T(*e2*)
- T(*e1* `-` *e2*) = `SUB` T(*e1*) T(*e2*)
- T(*e1* `:` *e2*) = `PUSH` T(*e1*) `PUSH` T(*e2*) `NIL`
- T(`NOT` *e*) = `SUB 1` T(*e*)
- T(*e1* `=` *e2*) = `EQ` T(*e1*) T(*e2*)
- T(*e1* `<>` *e2*) = `NE` T(*e1*) T(*e2*)
- T(*e1* `<` *e2*) = `LT` T(*e1*) T(*e2*)
- T(*e1* `<=` *e2*) = `LE` T(*e1*) T(*e2*)
- T(*e1* `>` *e2*) = `LT` T(*e2*) T(*e1*)
- T(*e1* `>=` *e2*) = `LE` T(*e2*) T(*e1*)
- T(*e1* `and` *e2*) = `MIN` T(*e1*) T(*e2*)
- T(*e1* `or` *e2*) = `MAX` T(*e1*) T(*e2*)
- T(`if` *e1* `then` *e2* `else` *e3*) = `(COND` T(*e1*) `{:`T(*e2*)`}` `{:`T(*e3*)`}` `)`
- T(`let` *x1* `=` *e1* `,` ... `,` *xn* `=` *en* `in` *e*) = `({` *x1* ... *xn* `:` T(*e*) `}` T(*e1*) `,` ... `,` T(*en*) `)`
- T(`letrec` *x1* `=` *e1* `,` ... `,` *xn* `=` *en* `in` *e*) = `({` `!`*x1* ... `!`*xn* `:` T(*e*) `}` T(*e1*) `,` ... `,` T(*en*) `)`

### Grammar priorities

The only care one has to take when implementing the translator function T from Niceful expressions to Awful expressions are priorities of Niceful binary operators, expressed in the BNF grammar.

## Examples

Some examples of programs follows: I will show Niceful examples and their translations in Awful.

### Some list functions

(I omit the backslashes at the end of each line: a future version of the language will allow to continue a line with no backslash and use a end-of-line token such as `;`)

```
    letrec
    / len(L) returns the length of list L
    len = fun L:
        if L = NIL then 0
        else 1 + len(rest L)
    ,
    / nth(L,n) returns the n-th element of L
    nth = fun L n:
        if L = NIL or n < 0 then "ERROR"
        else
        if n = 0 then 1st L
        else nth(rest L, n - 1)

```


