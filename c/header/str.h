/** \file str.h */

#ifndef str_INC
#define str_INC

#include <string.h>

/** Free a string allocated with str_new. */
extern void str_del(char *s);

/** Create a new string from a text starting at s and
    with length n and returns its address. */
extern char *str_new(char *s, int n);

#endif
