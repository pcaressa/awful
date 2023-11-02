/** \file val.c */

#include <stdio.h>
#include "../header/stack.h"
#include "../header/val.h"

/** Print a value on a file. */
void val_printf(FILE *file, int type, val_t v)
{
    switch (type) {
    case NUMBER: fprintf(file, "%g", v.n); break;
    case STRING: fprintf(file, "'%s'", v.t); break;
    case ATOM: fputs(v.t, file); break;
    case DELIMITER: fputc(v.d, file); break;
    case KEYWORD: fprintf(file, "<function %p>", v.p); break;
    case STACK: {
        fputc('[', file);
        stack_t s = v.s;
        while (s != NULL) {
            val_printf(file, s->type, s->val);
            if (s->next == NULL) break;
            fputc(',', file);
            s = s->next;
        }
        fputc(']', file);
        break;
    }
    case CLOSURE: {
        fputs("CLOSURE: ", file);
        val_printf(file, STACK, v);
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
        if (type > 32 && type < 128) {
            fprintf(file, "'%c'", v.d);
        } else {
            fputc('?', file);
        }
    }
}
