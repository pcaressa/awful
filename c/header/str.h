/** \file str.h */

#ifndef str_INC
#define str_INC

#include <stdio.h>

/** Concatenates two strings: s1 is expanded to make space
    for a copy of s2 that is appended to s1. The address of
    the new string (possibly still s1) is returned.
    WARNING: s1 should not be a literal string, but it
    can be NULL!!! */
extern char *str_cat(char *s1, const char *s2);

/** Create a new string from a text starting at s and
    with length n and returns its address. */
extern char *str_new(const char *s, size_t n);

/** Resets all strings: don't free them explicitly, that
    is done by stack_reset() that destroys data inside
    stack elements. */
extern void str_reset(void);

/** Print on a file the current string table status. */
extern void str_status(FILE *dump);

/** Return the address of the substring of s which is stripped
    by spaces, both on the left and on the right: on the left
    the effect is achieved returing a pointer on the first non
    space character, on the right by setting to '\0' the leftmost
    space from the end of the string. */
extern char *str_strip(char *s);

#endif
