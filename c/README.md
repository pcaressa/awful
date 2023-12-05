# C interpreter for AWFUL

#### (c) 2023 by Paolo Caressa

The C interpreter ought to be compiled before being used: you'll need a C compiler such as `clang`, `gcc` or the one coming with `MS Visual Studio` etc. Some compilers, such as `clang`, explicitly need to load math libraries when compiling.

The current folder contains all source files needed to compile the interpreter.

For example you could type, inside the [src/]:

    clang -lm *.c -o awful

to create an executable for the Awful language (of course, add all compiler options you like). The executable awful can now be launched to execute the interpreter:

    ./awful

### Interacting with the interpreter

After launching the interpreter, a prompt will appear:

    AWFUL - A Weird FUnctional Language
    (c) 2023 by Paolo Caressa <github.com/pcaressa/awful>
    [v.0.2312. Type 'help' for... guess what?]

    niceful 1: 

The prompt shows that the interpreter expects to receive a Niceful expression: type it and press ENTER. If the expression needs to span over multiple lines then add a final backslash to the line and the interpreter will ask for more, as in

    niceful 1: let x = 10   \ This is a comment
    niceful 2|      in x ^ 2
    100
    niceful 3:

Text after a backslash will be ignored, so that one can use it also to insert comments.

The interpreter prints a progressive integer, after the prompt: it is useful when using batch files. To evaluate a file whose lines contain single expressions use `batch FILENAME`.

For example suppose the `sample.nfl` text file contains

    \ Example of function application
    (fun x: x + 1)(10)

    19\
    +\
    21

Then, we could type

    niceful 1: batch sample.nfl
    11
    40
    niceful 2:

Inside a script one can use the `bye` and the `batch` directives, too.

The C interpreter is a single program, while Python provides two interpreters, one for Awful and one for Python. To switch to the Awful interpreter, use the command `afwul` as in

    niceful 1: awful
    Awful interpreter
    awful 2:

To get back to the Niceful interpreter type `niceful`.

There are some more commands that can be typed instead of an expression: you can get them enumerated by the command `help` that prints the following short explanation:

    Interactive mode: type the expression to evaluate on a single
    line: to continue the expression on another line end it by
    a backslash. Anything after the backslash will be ignored
    (so you can use them as comments).

    REPL commmands: type them instead of an expression, they are:
    'awful': switch to Awful interpreter.
    'batch FILENAME': the FILENAME text file is opened for
        reading and each line of it is evaluated as a single
        line typed in the interactive mode.
    'bye' ends the session and closes the interpreter.
    'help' prints this message.
    'niceful': switch to Niceful interpreter.
    'output': redirect output to terminal screen.
    'output FILENAME': redirect output to file FILENAME (in      append mode).
    'prelude FILENAME ...' the FILENAME text file is opened for
        reading and its lines are joined in a single line to
        which the next input line is appended: the resulting
        string is evaluated.
    Warning: a preluded file cannot exceed 64Kbytes.

To leave the interpreter type `bye`.

These commands are explained also in the tutorial and in the language reference.

See the file [../doc/awful_intro_fl.md](../doc/awful_intro_fl.md) for a gentle introduction to the language and to the basic concepts in functional programming.

A short description of the language is in the [../README.md](../README.md) file in the root folder of the repository.
