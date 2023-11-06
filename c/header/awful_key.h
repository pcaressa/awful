/** \file awful_key.h */

#ifndef awful_key_INC
#define awful_key_INC

#include "stack.h"
#include "val.h"

/** Data type of a pointer to a keyword routine, that is
    invoked when the keyword is parsed in a tokens list. */
typedef val_t (*awful_key_t)(stack_t*, stack_t);

/** Check whether the string text is a keyword and if it is
    then return the pointer of the corresponding routine,
    casted to void*, else NULL. */
extern void *awful_key_find(char *text);

#endif
