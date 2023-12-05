# Python interpreter for AWFUL

#### (c) 2023 by Paolo Caressa

The Python prototype is simple to use: you'll need the Python environment set up, version 3; no packages other than the built-in ones are used.

Just store the folder containing the Python source (the one containing this document) somewhere in the PATH and launch

    python3 awful

In the first place, you can launch this command from inside the folder itself.

To use the Niceful interpreter launch

    python3 niceful

After launching the interpreter, a prompt will appear:

    AWFUL - AWful FUnctional Language
    (c) 2023 by Paolo Caressa <github.com/pcaressa/awful>

    Type: 'bye' to leave, 'batch FILENAME' to process a file
    Lines starting with backslash or empty are skipped
    A line ending with backslash is joined to the following one

    awful:

Now you can type an Awful expression and get the result computed: if the expression needs to span over multiple lines then add a final backslash to the line and the interpreter will ask for more, as in

    awful 1: ADD 1       \
    awful 2|     MUL 2 3
    7.0
    awful 3: 

To leave the interpreter type `bye`, to evaluate a file whose lines contain single Awful expressions use `batch FILENAME`; text after a backslash will be ignored, so that one can insert comments in a batch file in this way.

For example suppose the `sample.awf` text file contains

    \ Example of function application
    ({x:ADD x 1} 10)

    ADD\
    19\
    21

Then, we could write

    awful: batch sample.awf
    11.0
    40.0
    awful:

Inside a script one can use the `bye` and the `batch` directives, too.

See the file [../doc/awful_intro_fl.md](../doc/awful_intro_fl.md) for a gentle introduction to the language and to the basic concepts in functional programming.

A short description of the language is in the [../README.md](../README.md) file in the root folder of the repository.
