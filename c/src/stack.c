/** \file stack.c */

#include <stdio.h>
#include <stdlib.h>
#include "../header/except.h"
#include "../header/stack.h"
#include "../header/str.h"
#include "../header/val.h"

/**
    A chunk is an array of CHUNKSIZ stack items:
    each stack in the program takes its items
    from a chunk. The function stack_reset() is
    used to free all chunks for future reuse.
*/

#define CHUNKSIZ (1024)

typedef struct stack_chunk_s {
    struct stack_chunk_s *next;
    unsigned here;  ///< Index of 1ft free item in chunk
    struct stack_s chunk[CHUNKSIZ];
} *stack_chunk_t;

/** First chunk of stack items. */
static stack_chunk_t stack_chunks = NULL;

stack_t stack_dup(stack_t s1, stack_t s2)
{
    except_on(s1 == NULL, "Cannot pop from empty stack");
    stack_t s = stack_new();
    s->val = s1->val;
    s->next = s2;
    return s;
}

void stack_fprint(FILE *f, stack_t s)
{
    val_t v = {.type = STACK, .val.s = s};
    val_fprint(f, v);
}

stack_t stack_new(void)
{
    stack_t s = NULL;
    for (stack_chunk_t c = stack_chunks; c != NULL; c = c->next)
        if (c->here < CHUNKSIZ) {
            s = c->chunk + c->here++;
            break;
        }
    // If s == NULL no free chunk was found.
    if (s == NULL) {
        stack_chunk_t new_chunk = malloc(sizeof(struct stack_chunk_s));
        except_on(new_chunk == NULL, "Fatal allocation error"
            " @%s:%i", __FILE__, __LINE__);
        new_chunk->next = stack_chunks;
        s = new_chunk->chunk;
        new_chunk->here = 1;
        stack_chunks = new_chunk;
    }
    return s;
}

stack_t stack_next(stack_t s)
{
    return s == NULL ? NULL : s->next;
}

stack_t stack_push(stack_t s, val_t v)
{
    except_on(v.type < 0 || v.type > 127, "BUG!"
        " @%s:%i [t = %i", __FILE__, __LINE__, v.type);
    stack_t tos = stack_new();
    tos->val = v;
    tos->next = s;
    return tos;
}

stack_t stack_push_s(stack_t s, stack_t s1)
{
    val_t v = {.type = STACK, .val.s = s1};
    return stack_push(s, v);
}

void stack_reset(void)
{
    for (stack_chunk_t c = stack_chunks; c != NULL; c = c->next) {
        c->here = 0;
    }
    str_reset();
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
            s->next = prev;
        }
    }
    return s;
}

void stack_status(FILE *dump)
{
    unsigned mem = 0;
    unsigned n = 0;
    for (stack_chunk_t c = stack_chunks; c != NULL; c = c->next) {
        mem += sizeof(struct stack_chunk_s);
        n += c->here;
    }
    fprintf(dump, "\n%u stack items (%u Kbytes)\n", n, mem / 1024);
}
