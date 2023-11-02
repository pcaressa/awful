/** \file val.h */

#ifndef val_INC
#define val_INC

#include <stdio.h>
#include "stack.h"

/** Type containing a single Awful value or token. */
typedef union {
    char d;             // Used for delimiters
    double n;           // Number
    char *t;            // string
    struct stack_s *s;  // stack or closure
    void *p;            // Used for keywords
} val_t;

/** Constants used to encode values or tokens types. */
enum {
    NUMBER = -7,
    STRING,
    ATOM,
    DELIMITER,
    KEYWORD,
    STACK,
    CLOSURE
};

/** Print a value on a file. */
extern void val_printf(FILE *file, int type, val_t v);

#endif
