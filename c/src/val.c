/** \file val.c */

#include <stdio.h>
#include "../header/stack.h"
#include "../header/val.h"

<<<<<<< HEAD
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
=======
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
            val_printf(f, b->val);
        }
        fputc('|', f);
        s = s->next;
        val_list_printf(f, s->val.val.s);
        fputc('}', f);
        break;
    }
    default:
        fputc((v.type > 32 && v.type < 128) ? v.type : '?', f);
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7
    }
}
