/** \file val.h */

#ifndef val_INC
#define val_INC

#include <stdio.h>
#include "stack.h"

/** Constants used to encode values or tokens types. */
enum { NONE, NUMBER, STRING, ATOM, KEYWORD, STACK, CLOSURE };

/** Type containing a single Awful value or token. */
typedef struct val_s {
    int type;   /* A constant denoting the type of data: it
                    can be a NUMBER, etc. constant or a
                    number between 33 and 127 representing
                    a single delimiter. */
    union {
        char d;             // Used for delimiters
        double n;           // Number
        char *t;            // string
        struct stack_s *s;  // stack or closure
        void *p;            // Used for keywords
    } val;
} val_t;

/** Delete the contents of a val_t: this only does anything
    if v contains a stack or a closure. */
extern void val_delete(val_t v);

/** Print a value on a file. */
extern void val_printf(FILE *f, val_t v);

#endif
