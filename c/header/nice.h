/** \file nice.h */

#ifndef nice_INC
#define nice_INC

#include <stdio.h>

/** Interpret the string *text as a Niceful expression and
    print the resulting value on the file.
    If an error occurs, a non zero error code is returned. */
extern int nice(char *text, FILE *file);

#endif
