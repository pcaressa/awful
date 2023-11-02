/** \file stack.c */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../header/stack.h"
#include "../header/str.h"
#include "../header/val.h"

/** Free stack. */
static stack_t stack_freelist = NULL;

static int stack_obj_count = 0;

void stack_delete(stack_t s)
{
    while (s != NULL) {
        if (s->val.type == STACK || s->val.type == CLOSURE) {
            stack_delete(s->val.val.s);
        }
        stack_t next = s->next;
        stack_drop(s);
        s = next;
    }
}

stack_t stack_drop(stack_t s)
{
    assert(s != NULL);
    stack_t tos = s;
    s = tos->next;
    tos->next = stack_freelist;
    stack_freelist = tos;
    if (tos->val.type == STACK || tos->val.type == CLOSURE)
        stack_delete(tos->val.val.s);
    return s;
}

stack_t stack_dup(stack_t s1, stack_t s2)
{
    assert(s1 != NULL);
    stack_t s = stack_new();
    memcpy(s, s1, sizeof(struct stack_s));
    s->next = s2;
    return s;
}

stack_t stack_new(void)
{
    stack_t s;
    if (stack_freelist == NULL) {
        s = malloc(sizeof(struct stack_s));
        assert(s != NULL);
        ++ stack_obj_count;
    } else {
        s = stack_freelist;
        stack_freelist = stack_freelist->next;
    }
    return s;
}

stack_t stack_push(stack_t s, val_t v)
{
    stack_t tos = stack_new();
    tos->val.type = v.type;
    if (v.type >= 0 && v.type < 128) {
        tos->val.val = v.val;
    } else {
        fprintf(stderr, "t = %i", v.type);
        assert(!"BUG!");
    }
    tos->next = s;
    return tos;
}

stack_t stack_reverse(stack_t s)
{
    if (s == NULL || s->next == NULL) {
        return s;
    }
    /* for each triple of consecutive elements a->b->c
        inverts the links as a<-b<-c. */
    stack_t prev = NULL;
    stack_t here = s;
    stack_t next = s->next;

    while (next->next != NULL) {
        here->next = prev;
        stack_t next_next = next->next;
        next->next = here;
        prev = here;
        here = next;
        next = next_next;
    }
    next->next = here;
    return next;
}

void stack_printf(FILE *f, stack_t s)
{
    val_t v = {.type = STACK, .val.s = s};
    val_printf(f, v);
}

void stack_status(void)
{
    int n = 0;
    for (stack_t p = stack_freelist; p != NULL; p = p->next)
        ++ n;
    fprintf(stderr, "\nStack: #objects = %i, #freelist = %i\n",
        stack_obj_count, n);
}
