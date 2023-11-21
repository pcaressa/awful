/** \file stack.c */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../header/stack.h"
#include "../header/str.h"
#include "../header/val.h"

/** Free stack: this contains a list of free stack items
    that can be used to construct other stacks. An item
    in stack_free has type NONE. */
static stack_t stack_free = NULL;

static int stack_obj_count = 0;

void stack_delete(stack_t s)
{
    while (s != NULL) {
        s = stack_drop(s);
    }
}

stack_t stack_drop(stack_t s)
{
    assert(s != NULL);
    stack_t next = s->next;
    s->next = stack_free;
    stack_free = s;
    if (s->val.type == STACK || s->val.type == CLOSURE) {
        stack_delete(s->val.val.s);
    }
    s->val.type = NONE;
    return next;
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
    if (stack_free == NULL) {
        s = malloc(sizeof(struct stack_s));
        assert(s != NULL);
        ++ stack_obj_count;
    } else {
        s = stack_free;
        stack_free = stack_free->next;
    }
    return s;
}

stack_t stack_push(stack_t s, val_t v)
{
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
}

void stack_printf(FILE *f, stack_t s)
{
    val_t v = {.type = STACK, .val.s = s};
    val_printf(f, v);
}

void stack_status(void)
{
    int n = 0;
    for (stack_t p = stack_free; p != NULL; p = p->next)
        ++ n;
    fprintf(stderr, "\nStack: #objects = %i, #free = %i\n",
        stack_obj_count, n);
}
