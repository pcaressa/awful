# AWFUL - A Weird FUnctional Language

#### (c) 2023 by Paolo Caressa

## Introduction

Awful is a simple functional language designed and implemented for didactic purposes: its main motivation was to understand and to replicate ideas by Peter J Landin borrowed from his papers from early '60s.

Awful provides the bare minimum equipment for a functional environment:

- atomic data types: numbers, strings and atoms; atoms are used to refer to other objects
- structured data types: stacks and closures

The language comes in two layers:

- Awful, the *representation language*;
- Niceful, the *publication language*.

The latter is addressed to humans wanting to use the language, the former is addressed to machines to execute it.

One may thinks to representation language as the "inner form" and to publication language as "syntactic sugar" built on top of the inner form.

Currently the status of the project is the following:

- Documentation: this md file.
- Awful:
    - Python implementation: [py/](py/) folder containing a series of Python scripts with a complete implementation: see below for the usae.
    - C implementation: [c/](c/) folder containing C headers and sources to be compiled into a single executable.
- Niceful:
    - Python implementation: [py/](py/) folder containing the niceful.py script.

TODO:

- Niceful implementation in Niceful, used to self-implement Niceful using the Awful interpreter.

## Awful: A Weird FUnctional Language

Awful provides a Lisp-like (but parentheses-less) formalism to encode values and expressions.

### Using the interpreter

Currently two interpreters are available: a Python version, written in a non idiomatic Python aimed at portability, and a C version, that should be considered the reference implementation.

#### Python version

The Python prototype is simple to use: you'll need the Python environment set up, version 3; no packages other than the built-in ones are used.

Just store the folder [py] somewhere in the PATH and launch

    python3 awful

In the first place, you can launch this command from inside the folder [py/] itself.

#### C version

The C interpreter ought to be compiled before being used: you'll need a C compiler such as `clang` or the one coming with `MS Visual Studio` etc. Some compilers, such as `clang`, needs to explicit load math libraries when compiling.

The [c/] folder contains all source files needed to compile the interpreter: see the [c/README.md] for instructions.

For example you could type, inside the [c/src/]:

    clang -lm *.c -o awful

Of course, add all compiler options you like. The executable awful can now be launched to execute the interpreter.

### Interacting with the interpreter

After launching the interpreter, a prompt will appear:

    AWFUL - AWful FUnctional Language
    (c) 2023 by Paolo Caressa <github.com/pcaressa/awful>

    Type: 'bye' to leave, 'batch FILENAME' to process a file
    Lines starting with backslash or empty are skipped
    A line ending with backslash is joined to the following one

    awful:

Now you can type an Awful expression and get the result computed: if the expression needs to span over multiple lines then add a final backslash to the line and the interpreter will ask for more, as in

    awful: ADD 1       \
    awful|     MUL 2 3
    7.0
    awful: 

To leave the interpreter type `bye`, to evaluate a file whose lines contain single Awful expressions use `batch FILENAME`; a line starting by a backslash will be ignored, so that one can insert comments in a batch file in this way.

For example suppose the `sample.awf` text file contains

    \ Example of function application
    ({x:ADD x 1} 10)

    ADD\
    19\
    21

Then, we could write

    awful:batch sample.awf
    11.0
    40.0
    awful:

Inside a script one can use the `bye` and the `batch` directives, too.

### Awful syntax

An expression is just a sequence of token: a token can be

- a number: a decimal/exponential notation representing a floating point number.
- a string: an immutable character sequence enclosed between double quotes and not containing double quotes or enclosed between quotes and not containing quotes.
- a delimiter: parentheses, braces, comma and colon.
- a keyword: one of the symbols `ADD BOS COND DIV EQ LE LT MAX MIN MUL NE POW PUSH SUB TOS`.
- an atom: a contiguous sequence of non space characters and non delimiter characters which is neither a number nor a keyword.

An expression is a sequence of token matching one of the following rules:

- A single number, string or atom.
- A keyword followed by a sequence of tokens encoding expressions needed as actual parameters of the keyword.
- `{` followed by a possible empty sequence of atoms, each one possibly preceded by `!`, followed by `:`, followed by an expression, followed by `}`.
- `(` followed by an expression, possibly followed by a sequence of comma-separated expressions`)`

The number of expressions that need to follow a keyword is:

- 1 for `BOS TOS`.
- 2 for `ADD DIV EQ LE LT MAX MIN MUL NE POW PUSH SUB`.
- 3 for `COND`.

### Awful semantics

The behavior of keywords when they are executed is the following: by *e* we mean any expression, by *n* any expression whose value is a number, by *s* any expression whose value is a stack. It is understood that if *n* or *s* are used, the language interpreter check against the type of the value and raises an error if the type is not the one expected.

- the value of `ADD` *n1* *n2* is *n1* + *n2*;
- the value of `BOS` *s1* is *s1* deprived of its first element;
- the value of `COND` *n e1 e2* is *e1 if *n* is not zero, else *e2*;
- the value of `DIV` *n1 n2* is *e1 / e2*;
- the value of `EQ` *e1 e2* is 1 if *e1 = e2*, else 0;
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

## Niceful: an NICE FUnctional Language

Niceful provides a ML-like formalism to encode values and expressions.

### Niceful syntax

We can express Niceful syntax by means of the following grammar written in the old good BNF style symbols of the language are enclosed between backslashes, that are not part of the language alphabet:

    expression = fun-expr
        | \let\ assignments \in\ expression
        | \letrec\ assignments \in\ expression

    assignments = [ atom \=\ fun-expr {\,\ atom \=\ fun-expr} ]

    fun-expr = conditional | \fun\ {\!\ atom} \:\ expression

    conditional = proposition
        | \if\ proposition \then\ expression \else\ expression

    proposition = relation {bool-op proposition}
    
    relation = sum [ rel-op sum ]
    sum = product { add-op product }
    product = term { mul-op term }
    power = term [\^\ term]
    term = atom | string | list | \-\ term | \1st\ term | \rest\ term 
        | \(\ expression \)\ | term { \(\ [expr-list] \)\ }
    
    expr-list = expression {\,\ expression}

    bool-op = \and\ | \or\
    rel-op = \=\ | \<>\ | \<\ | \>\ | \<=\ | \>=\
    add-op = \+\ | \-\ | \:\
    mul-op = \*\ | \/\

    string = \"\{character}\"\ | \'\{character}\'\
    list = \[\ [expr-list] \]\

Each Niceful syntactic construction can be translated into a corresponding Awful expression or part of expression: thus Niceful is just a different form in which to express Awful expressions.

### Niceful translation function

For each kind of Niceful construct we will now specify the corresponding Awful expression. More precisely, let us define a function *y* = T(*x*) that translates a Niceful expression *x* into an Awful expression *y*.

We will define T on all possible expressions as defined by the previous grammar:

- If *x* is a number, string or atom then T(*x*) = *x*.
- T(`[]`) = `NIL`
- T(`[`*e1* `,` ... `,` *en* `]`) = `PUSH` T(*e1*) `PUSH` T(*e2*) ... `PUSH` T(*en*) `NIL`
- T(`-` *e*) = `SUB 0 ` T(*e*)
- T(`1st` *e*) = `TOS` *e*
- T(`rest` *e*) = `BOS` *e*
- T(`(`*e*`)`) = T(*e*)
- T(*e1* `^` *e2*) = `POW` T(*e1*) T(*e2*)
- T(*e1* `*` *e2*) = `MUL` T(*e1*) T(*e2*)
- T(*e1* `/` *e2*) = `DIV` T(*e1*) T(*e2*)
- T(*e1* `+` *e2*) = `ADD` T(*e1*) T(*e2*)
- T(*e1* `-` *e2*) = `SUB` T(*e1*) T(*e2*)
- T(*e1* `:` *e2*) = `PUSH` T(*e1*) T(*e2*)
- T(`NOT` *e*) = `SUB 1` T(*e*)
- T(*e1* `=` *e2*) = `EQ` T(*e1*) T(*e2*)
- T(*e1* `<>` *e2*) = `NE` T(*e1*) T(*e2*)
- T(*e1* `<` *e2*) = `LT` T(*e1*) T(*e2*)
- T(*e1* `<=` *e2*) = `LE` T(*e1*) T(*e2*)
- T(*e1* `>` *e2*) = `LT` T(*e2*) T(*e1*)
- T(*e1* `>=` *e2*) = `LE` T(*e2*) T(*e1*)
- T(*e1* `and` *e2*) = `MIN` T(*e1*) T(*e2*)
- T(*e1* `or` *e2*) = `MAX` T(*e1*) T(*e2*)
- T(`if` *e1* `then` *e2* `else` *e3*) = `((COND` T(*e1*) `{:`T(*e2*)`}` `{:`T(*e3*)`}` `))`
- T(`let` *x1* `=` *e1* `,` ... `,` *xn* `=` *en* `in` *e*) = `({` *x1* ... *xn* `:` T(*e*) `}` T(*e1*) `,` ... `,` T(*en*) `)`
- T(`letrec` *x1* `=` *e1* `,` ... `,` *xn* `=` *en* `in` *e*) = `({` `!`*x1* ... `!`*xn* `:` T(*e*) `}` T(*e1*) `,` ... `,` T(*en*) `)`

### Grammar priorities

The only care one has to take when implementing the translator function T from Niceful expressions to Awful expressions are priorities of Niceful binary operators, expressed in the BNF grammar.

## Implementation

Let us describe how to implement the Awful interpreter. We will assume to use a language that can represent the following data types:

- floating point numbers.
- strings.
- stacks whose items can be numbers, strings and stacks.

We'll write stacks as: [*tos,2os,3os, ...*] being *tos* the topmost elements etc.

The interpreter will be described by words but using only the values available in the language itself: we will use the Json notation [x1,...,xn] for stacks, as already stated, and we will write {k1:v1,...,kn:vn} as an abbreviation for the stack [[k1,v1],...,[kn:vn]].

### Afwul interpreter

The interpreter exposes a function/method *s = awful(t)* that accepts a string and returns a stack of one of the following forms:

- {"NUMBER": *n*}
- {"STRING", *t*}
- {"STACK", *s*}
- {"CLOSURE", *p, b, e*} where:
    - *p* is the stack [*x1,...,xn*] of formal parameter names;
    - *b* is a stack of tokens containing the body of the funciton;
    - *e* is stack of stacks, the environment in which the function has been defined.

To do that, *awul(t)* does the following:

- scans the string *t* transforming it in a list *c* of tokens [*t1,...,tn*]: each token is a pair [*type, value*] where:
    - aaa
- instantiates an empty *e* environment stack.
- calls the function *eval(c,e)* and, after discarding both *c* and *e*, returns its result. 

### Eval function

TODO

### Closures definitions

TODO

### Closures evaluations

TODO


### Niceful interpreter

TODO