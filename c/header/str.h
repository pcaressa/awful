/** \file str.h */

#ifndef str_INC
#define str_INC

#include <string.h>

/** Create a new string from a text starting at s and
    with length n and returns its address. */
extern char *str_new(char *s, int n);

#endif
