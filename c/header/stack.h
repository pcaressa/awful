/** \file stack.h */

#ifndef stack_INC
#define stack_INC

#include "val.h"

/** Stack item. */
typedef struct stack_s {
    unsigned refs;  //< Number of items pointing to this one
    int type;       //< type of data stored in this item
    val_t val;      //< value of data stored in this item
    struct stack_s *next;   // item below this one
} *stack_t;

/** Dispose a stack s and delete also all its substacks. */
extern void stack_delete(stack_t s);

/** Discard the top of a stack passed by reference;
    the new stack pointer is returned as value. */
extern stack_t stack_drop(stack_t s);

/** Duplicate the top of stack s1 and pushes the result on
    s2, which is returned as value */
extern stack_t stack_dup(stack_t s1, stack_t s2);

/** Creates a new stack with a single item not initialized. */
extern stack_t stack_new(void);

/** Push a value (t,v) on the stack s: v can be a NUMBER,
    STRING, STACK or CLOSURE according to t. The updated
    value of s is returned. */
extern stack_t stack_push(stack_t s, int t, ...);

/** Reverse the order of elements in a stack s:
    the new stack pointer is returned. */
extern stack_t stack_reverse(stack_t s);

#include <stdio.h>

/** Prints the contents of a stack on a file. */
extern void stack_printf(FILE *f, stack_t s);

#endif
