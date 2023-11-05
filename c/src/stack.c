/** \file stack.c */

#include <assert.h>
<<<<<<< HEAD
#include <stdarg.h>
=======
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7
#include <stdio.h>
#include <stdlib.h>
#include "../header/stack.h"
#include "../header/str.h"
#include "../header/val.h"

<<<<<<< HEAD
/** Free stack. */
static stack_t stack_freelist = NULL;
=======
/** Free stack: this contains a list of free stack items
    that can be used to construct other stacks. An item
    in stack_free has type NONE so that it is rcognized
    by delete and drop routines that leave it untouched. */
static stack_t stack_free = NULL;
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7

static int stack_obj_count = 0;

void stack_delete(stack_t s)
{
<<<<<<< HEAD
    while (s != NULL) {
        if (s->type == STACK || s->type == CLOSURE) {
            stack_delete(s->val.s);
        }
        stack_t next = s->next;
        stack_drop(s);
        s = next;
    }
=======
//~ fprintf(stderr, "> stack_delete "); stack_printf(stderr, s); fputc('\n', stderr);
    while (s != NULL) {
        s = stack_drop(s);
//~ stack_printf(stderr, s); fputc('\n', stderr);
    }
//~ fprintf(stderr, "< stack_delete ");
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7
}

stack_t stack_drop(stack_t s)
{
<<<<<<< HEAD
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
=======
//~ fprintf(stderr, "> stack_drop "); stack_printf(stderr, s); fputc('\n', stderr);
    assert(s != NULL);
    stack_t next = s->next;
    s->next = stack_free;
    stack_free = s;
    if (s->val.type == STACK || s->val.type == CLOSURE) {
    //if (tos->val.type == STACK) {
//~ fprintf(stderr, "> stack_delete "); stack_printf(stderr, s->val.val.s); fputc('\n', stderr);
        s->val.type = NONE;
        stack_delete(s->val.val.s);
    }
    return next;
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7
}

stack_t stack_dup(stack_t s1, stack_t s2)
{
    assert(s1 != NULL);
    stack_t s = stack_new();
    memcpy(s, s1, sizeof(struct stack_s));
    s->next = s2;
<<<<<<< HEAD
    if (s2 != NULL) ++ s2->refs;
=======
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7
    return s;
}

stack_t stack_new(void)
{
<<<<<<< HEAD
    stack_t s;
    if (stack_freelist == NULL) {
=======
//~ fprintf(stderr, "> stack_new\n");
    stack_t s;
    if (stack_free == NULL) {
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7
        s = malloc(sizeof(struct stack_s));
        assert(s != NULL);
        ++ stack_obj_count;
    } else {
<<<<<<< HEAD
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
=======
        s = stack_free;
        stack_free = stack_free->next;
    }
//~ fprintf(stderr, "< stack_new "); stack_printf(stderr, s); fputc('\n', stderr);
    return s;
}

stack_t stack_push(stack_t s, val_t v)
{
//~ fprintf(stderr, "> stack_push \n"); stack_printf(stderr, s); fputc(',', stderr); val_printf(stderr, v); fputc('\n', stderr);
    stack_t tos = stack_new();
    tos->val.type = v.type;
    if (v.type >= 0 && v.type < 32) {
        tos->val.val = v.val;
    } else
    if (v.type < 0 || v.type > 127) {
        fprintf(stderr, "t = %i", v.type);
        assert(!"BUG!");
    }
    tos->next = s;
    return tos;
//~ fprintf(stderr, "< stack_push \n"); stack_printf(stderr, s); fputc('\n', stderr);
}

stack_t stack_push_s(stack_t s, stack_t s1)
{
    val_t v = {.type = STACK, .val.s = s1};
    return stack_push(s, v);
}

stack_t stack_reverse(stack_t s)
{
    if (s != NULL) {
        stack_t prev = NULL;
        stack_t next = s->next;
        s->next = prev;
        while (next != NULL) {
            prev = s;
            s = next;
            next = next->next;
            s->next = prev; }}
    return s;
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7
}

void stack_printf(FILE *f, stack_t s)
{
<<<<<<< HEAD
    val_t v = {.s = s};
    val_printf(f, STACK, v);
=======
    val_t v = {.type = STACK, .val.s = s};
    val_printf(f, v);
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7
}

void stack_status(void)
{
    int n = 0;
<<<<<<< HEAD
    for (stack_t p = stack_freelist; p != NULL; p = p->next)
        ++ n;
    fprintf(stderr, "\nStack: #objects = %i, #freelist = %i\n",
=======
    for (stack_t p = stack_free; p != NULL; p = p->next)
        ++ n;
    fprintf(stderr, "\nStack: #objects = %i, #free = %i\n",
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7
        stack_obj_count, n);
}
