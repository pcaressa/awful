/** \file val.h */

#ifndef val_INC
#define val_INC

#include <stdio.h>
#include "stack.h"

<<<<<<< HEAD
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
=======
/** Constants used to encode values or tokens types. */
enum {
    NONE,       // No value at all, different from NIL
    NUMBER,     // Number type
    STRING,     // String type
    ATOM,       // Atom type (used for variables)
    KEYWORD,    // Built-in function type
    STACK,      // Stack type
    CLOSURE     // Closure type
};

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
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7

#endif
