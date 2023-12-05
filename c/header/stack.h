/** \file stack.h */

#ifndef stack_INC
#define stack_INC

#include "val.h"

/** Stack item. */
typedef struct stack_s {
    struct stack_s *next;   //< item below this one
    val_t val;              //< value of data stored in this item
} *stack_t;

/** Returns 1 if stack s is empty, else 0. */
#define stack_empty(s) ((s) == NULL)

/** Returns a pointer to the stack resulting from s by
    dropping its topmost element. If s is NULL no error
    is raised but NULL is returned. */
extern stack_t stack_next(stack_t s);

/** Duplicate the top of stack s1 and pushes the result on
    s2, which is returned as value */
extern stack_t stack_dup(stack_t s1, stack_t s2);

/** Creates a new stack with a single item not initialized. */
extern stack_t stack_new(void);

/** Push a value v on the stack s. Return the updated value
    of s. */
extern stack_t stack_push(stack_t s, val_t v);

/** Push a value v = {STACK, s1} on the stack s.
    Return the updated value of s. */
extern stack_t stack_push_s(stack_t s, stack_t s1);

/** Delete all stack items allocated so far. */
extern void stack_reset(void);

/** Reverse the order of elements in a stack s:
    the new stack pointer is returned. */
extern stack_t stack_reverse(stack_t s);

/** Logs on a file the current stack status. */
extern void stack_status(FILE *dump);

#include <stdio.h>

/** Prints the contents of a stack on a file. */
extern void stack_fprint(FILE *f, stack_t s);

#endif
