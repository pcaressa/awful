/** \file repl.h */

#ifndef repl_INC
#define repl_INC

<<<<<<< HEAD
/** Activates a REPL by applying the function interpreter
    to each string introduced by the user. The function
    has signature int interpret(char*,FILE*): it takes a
    string in input, evaluates it and prints the result
    on file f. */
extern int repl(int (*interpret)(char*, FILE*), char *prompt);
=======
/** Activates a REPL by applying the interpret function to each
    string introduced by the user. Input lines are read from
    the in file, and results and error messages are printed on
    the out file. If not NULL, the prompt is printed each time
    a line is read from in.
    The interpret function should have signature
        int interpret(char*,FILE*)
    it takes a string in input, evaluates it and prints the
    result (or an error message) on the file. */
extern int repl(FILE *in, FILE *out, int (*interpret)(char*, FILE*), char *prompt);
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7

#endif
