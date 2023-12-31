/** \file val.c */

#include <stdio.h>
#include "../header/stack.h"
#include "../header/val.h"

static void val_list_fprint(FILE *f, stack_t s)
{
    while (s != NULL) {
        val_fprint(f, s->val);
        if (s->next == NULL) break;
        fputc(',', f);
        s = s->next;
    }
}

void val_fprint(FILE *f, val_t v)
{
    switch (v.type) {
    case NONE: fputs("NONE", f); break;
    case NUMBER: fprintf(f, "%g", v.val.n); break;
    case STRING: fprintf(f, "'%s'", v.val.t); break;
    case ATOM: fputs(v.val.t, f); break;
    case KEYWORD: fprintf(f, "<keyword %p>", v.val.p); break;
    case STACK: {
        fputc('[', f);
        val_list_fprint(f, v.val.s);
        fputc(']', f);
        break;
    }
    case CLOSURE: {
        fputc('{', f);
        stack_t s = v.val.s;
        // Formal parameters are preceded by their marker ('!' or NONE)
        for (stack_t fp = s->val.val.s; fp != NULL; fp = fp->next) {
            fputc((fp->val.type == '!') ? '!' : ' ', f);
            fp = fp->next;
            fputs(fp->val.val.t, f);
        }
        fputc(':', f);
        s = s->next;
        // The body is printed with no commas to separate items
        for (stack_t b = s->val.val.s; b != NULL; b = b->next) {
            fputc(' ', f);
            val_fprint(f, b->val);
        }
        fputc('|', f);
        s = s->next;
        //val_list_fprint(f, s->val.val.s);
        fputc('}', f);
        break;
    }
    default:
        fputc((v.type > 32 && v.type < 128) ? v.type : '?', f);
    }
}
