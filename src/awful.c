/** awful.c */

#include <assert.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* *****  Exception handling  ***** */

jmp_buf except_buf;     ///< exception handler

/** If cond is not 0 then raises an exception. */
void except_on(int cond, const char *fmt, ...)
{
    if (cond) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        longjmp(except_buf, -1);
    }
}

/* *****  String utilities  ***** */

/** Creates a new string from a text starting at s and
    with length n. */
char *str_new(char *s, int n)
{
    char *t = malloc(n + 1);
    assert(t != NULL);
    t[n] = '\0';
    return memcpy(t, s, n);
}

/* *****  Values and stacks  ***** */

/** Value. */
typedef union {
    char d;             // Used for delimiters
    double n;           // Number
    char *t;            // string
    struct stack_s *s;  // stack or closure
} val_t;

typedef struct stack_s {
    enum {NUMBER = -5, STRING, ATOM, STACK, CLOSURE} type;
    val_t val;
    struct stack_s *next;
} *stack_t;

/** Print a value on a file. */
void val_printf(FILE *file, int type, val_t v)
{
    switch (type) {
    case NUMBER: fprintf(file, "%g", v.n); break;
    case STRING: fprintf(file, "'%s'", v.t); break;
    case ATOM: fputs(v.t, file); break;
    case STACK:
        fputc('[', file);
        stack_t s = v.s;
        while (s != NULL) {
            val_printf(file, s->type, s->val);
            fputc(s->next == NULL ? ']' : ',', file);
            s = s->next;
        }
        break;
    case CLOSURE:
        // {type:CLOSURE, val:[[x1,...,xn], body, env]}
        fputc('{', file);
        val_printf(file, STACK, v);
        fputc(':', file);
        stack_t body = s->next;
        val_printf(file, STACK, body->val);
        fputc(';', file);
        stack_t env = body->next;
        val_printf(file, STACK, env->val);
        fputc('}', file);
    default:
        if (type > 32 && type < 128) {
            fprintf(file, "'%c'", v.d);
        } else {
            fputc('?', file);
        }
    }
}

/** Push a value (t,v) on the stack s. */
stack_t stack_push(stack_t s, int t, val_t v)
{
    stack_t tos = malloc(sizeof(struct stack_s));
    assert(tos != NULL);
    tos->type = t;
    tos->val = v;
    tos->next = s;
    return tos;
}

/** Reverse the order of elements in a stack. */
stack_t stack_reverse(stack_t s)
{
    if (s == NULL || s->next == NULL) {
        return s;
    }
    stack_t prev = NULL;
    stack_t here = s;
    stack_t next = here->next;

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

/** Dispose a stack. */
void stack_delete(stack_t s)
{
    while (s != NULL) {
        if (s->type == ATOM || s->type == STRING) {
            free(s->val.t);
        } else
        if (s->type == STACK) {
            stack_delete(s->val.s);
        } else
        if (s->type == CLOSURE) {
            // {type:CLOSURE, val:[[x1,...,xn], body, env]}
            stack_t params = s->val.s;
            stack_t body = params->next;
            stack_t env = body->next;
            stack_delete(params);
            stack_delete(body);
            stack_delete(env);
        }
        stack_t next = s->next;
        free(s);
        s = next;
    }
}

/** Scan a list of tokens from text, using any character
    in delimiters as delimiter.
    A stack is returned, containing the list of tokens
    in reverse order, so that the last one is its top.
*/
stack_t scan(char *text, char *delimiters)
{
    assert(text != NULL);
    stack_t tokens = NULL;
    val_t v;
    while (*text != '\0') {
        while (isspace(*text))
            ++ text;
        if (strchr(delimiters, *text) != NULL) {
            v.d = *text;
            tokens = stack_push(tokens, *text, v);
            ++ text;
        } else
        if (*text == '\'' || *text == '"') {
            char q = *text;
            char *p = strchr(text + 1, q);
            except_on(p == NULL, "End of text inside string");
            v.t = str_new(text + 1, p - text - 2);
            tokens = stack_push(tokens, STRING, v);
            text = p + 1;
        } else {
            // Scans up to the following space or delimiter.
            char *p = text++;
            while (*text != '\0' && !isspace(*text)
            && strchr(delimiters, *text) == NULL)
                ++ text;
            v.t = str_new(p, text - p);
            tokens = stack_push(tokens, ATOM, v);
        }
    }
    return tokens;
}

stack_t awful(char *text)
{
    stack_t tokens = NULL;
    if (setjmp(except_buf) == 0) {
        tokens = scan(text, "(){},:");
        tokens = stack_reverse(tokens);
        for (stack_t s = tokens; s != NULL; s = s->next) {
            val_printf(stdout, s->type, s->val);
            putchar(' ');
        }
    }
    stack_delete(tokens);
    return tokens;
}

int main(int argc, char **argv)
{
    awful("{x: ADD x 2}");
    return 0;
}
