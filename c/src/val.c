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

void val_printf(FILE *f, val_t v)
{
    switch (v.type) {
    case NUMBER: fprintf(f, "%g", v.val.n); break;
    case STRING: fprintf(f, "'%s'", v.val.t); break;
    case ATOM: fputs(v.val.t, f); break;
    case KEYWORD: fprintf(f, "<function %p>", v.val.p); break;
    case STACK: {
        fputc('[', f);
        stack_t s = v.val.s;
        while (s != NULL) {
            val_printf(f, s->val);
            if (s->next == NULL) break;
            fputc(',', f);
            s = s->next;
        }
        fputc(']', f);
        break;
    }
    case CLOSURE: {
        fputs("CLOSURE: ", f);
        val_t v1 = {.type = STACK, .val.s = v.val.s};
        val_printf(f, v1);
        break;
    }
    //~ case CLOSURE: {
        //~ // {type:CLOSURE, val:[[x1,...,xn], body, env]}
        //~ fputc('{', file);
        //~ val_printf(file, STACK, v);
        //~ fputc(':', file);
        //~ val_printf(file, STACK, v.s->next.val.s);
        //~ fputs(" where ", file);
        //~ val_printf(file, STACK, v->next->next.val.s);
        //~ fputc('}', file);
        //~ break;
    //~ }
    default:
        if (v.type > 32 && v.type < 128) {
            fprintf(f, "'%c'", v.val.d);
        } else {
            fputc('?', f);
        }
    }
}
