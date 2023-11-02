/** \file stack.c */

#include <assert.h>
#include <stdarg.h>
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
        if (s->type == STACK || s->type == CLOSURE) {
            stack_delete(s->val.s);
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
    -- tos->refs;
    if (tos->refs == 0) {
        tos->next = stack_freelist;
        stack_freelist = tos;
        if (tos->type == STACK || tos->type == CLOSURE)
            stack_delete(tos->val.s);
    }
    return s;
}

stack_t stack_dup(stack_t s1, stack_t s2)
{
    assert(s1 != NULL);
    stack_t s = stack_new();
    memcpy(s, s1, sizeof(struct stack_s));
    s->next = s2;
    if (s2 != NULL) ++ s2->refs;
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
    s->refs = 0;
    return s;
}

stack_t stack_push(stack_t s, int t, ...)
{
    stack_t tos = stack_new();
    tos->type = t;
    va_list args;
    va_start(args, t);
    switch (t) {
    case NUMBER: tos->val.n = va_arg(args, double); break;
    case STRING:
    case ATOM: tos->val.t = va_arg(args, char*); break;
    case STACK:
    case CLOSURE: tos->val.s = va_arg(args, stack_t); break;
    case KEYWORD: tos->val.p = va_arg(args, void*); break;
    default:
        if (t > 32 && t < 128) {
            /* ASCII codes are considered delimiters. */
            tos->val.d = t;
        } else {
            fprintf(stderr, "t = %i", t);
            assert(!"BUG!");
        }
    }
    va_end(args);
    tos->next = s;
    if (s != NULL) ++ s->refs;
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
    val_t v = {.s = s};
    val_printf(f, STACK, v);
}

void stack_status(void)
{
    int n = 0;
    for (stack_t p = stack_freelist; p != NULL; p = p->next)
        ++ n;
    fprintf(stderr, "\nStack: #objects = %i, #freelist = %i\n",
        stack_obj_count, n);
}
