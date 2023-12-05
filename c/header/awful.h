/** \file awful.h */

#ifndef awful_INC
#define awful_INC

#include <stdio.h>
#include "../header/stack.h"
#include "../header/val.h"

/// Max depth of recursion for the awful_eval function
#define MAX_EVAL (1024)

/** Interpret a token list, thus a stack whose elements are
    items representing an Awful text, w.r.t. an environment,
    both passed by reference, and return the value with the
    result of the evaluation. On error, the returned value
    is NONE. */
extern val_t awful_eval(stack_t *r_tokens, stack_t env);

/** Interpret the string *text as an Awful expression and
    print the resulting value on the file.
    If an error occurs, a non zero error code is returned. */
extern int awful(char *text, FILE *file);

#endif
