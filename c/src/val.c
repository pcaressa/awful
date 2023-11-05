/** \file val.c */

#include <stdio.h>
#include "../header/stack.h"
#include "../header/val.h"

void val_delete(val_t v)
{
    if (v.type == STACK || v.type == CLOSURE)
        // A closure is a stack of stacks [params, body, env]
        stack_delete(v.val.s);
}

static void val_list_printf(FILE *f, stack_t s)
{
    while (s != NULL) {
        val_printf(f, s->val);
        if (s->next == NULL) break;
        fputc(',', f);
        s = s->next;
    }
}

void val_printf(FILE *f, val_t v)
{
    switch (v.type) {
    case NUMBER: fprintf(f, "%g", v.val.n); break;
    case STRING: fprintf(f, "'%s'", v.val.t); break;
    case ATOM: fputs(v.val.t, f); break;
    case KEYWORD: fprintf(f, "<function %p>", v.val.p); break;
    case STACK: {
        fputc('[', f);
        val_list_printf(f, v.val.s);
        fputc(']', f);
        break;
    }
    case CLOSURE: {
        fputc('{', f);
        stack_t s = v.val.s;
        val_list_printf(f, s->val.val.s);
        fputc(':', f);
        s = s->next;
        val_list_printf(f, s->val.val.s);
        fputc('|', f);
        s = s->next;
        val_list_printf(f, s->val.val.s);
        fputc('}', f);
        break;
    }
    default:
        if (v.type > 32 && v.type < 128) {
            fprintf(f, "'%c'", v.type);
        } else {
            fputc('?', f);
        }
    }
}
