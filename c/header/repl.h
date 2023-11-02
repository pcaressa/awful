/** \file repl.h */

#ifndef repl_INC
#define repl_INC

/** Activates a REPL by applying the function interpreter
    to each string introduced by the user. The function
    has signature int interpret(char*,FILE*): it takes a
    string in input, evaluates it and prints the result
    on file f. */
extern int repl(int (*interpret)(char*, FILE*), char *prompt);

#endif
