/** \file str.h */

#ifndef str_INC
#define str_INC

#include <stdio.h>

/** Concatenates two strings: a new string is created to
    contain the concatenation and its address is returned.
    If s1 == NULL then s2 is returned; if s2 == NULL then
    s1 is returned. s1 and s2 cannot be both NULL. */
extern char *str_cat(const char *s1, const char *s2);

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
