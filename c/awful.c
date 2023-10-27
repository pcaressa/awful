/** \file awful.c */

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
    void *p;            // generic pointer: used for functions
} val_t;

/** Stack item. */
typedef struct stack_s {
    enum {NUMBER = -6, STRING, ATOM, KEYWORD,
        STACK, CLOSURE} type;
    val_t val;
    struct stack_s *next;
} *stack_t;

/** Free stack. */
static stack_t stack_freelist = NULL;

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
    stack_t tos;
    if (stack_freelist == NULL) {
        tos = malloc(sizeof(struct stack_s));
        assert(tos != NULL);
    } else {
        tos = stack_freelist;
        stack_freelist = stack_freelist->next;
    }
    tos->type = t;
    tos->val = v;
    tos->next = s;
    return tos;
}

/** Discard the top of a stack passed by reference;
    the new stack pointer is written into s but also
    returned as value. */
stack_t stack_drop(stack_t *s)
{
    assert(s != NULL);
    stack_t tos = *s;
    *s = (*s)->next;
    tos->next = stack_freelist;
    stack_freelist = tos;
    return *s;
}

/** Reverse the order of elements in a stack. */
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
        stack_drop(&s);
        /*stack_t next = s->next;
        // free(s);
        s->next = stack_freelist;
        stack_freelist = s;
        s = next;*/
    }
}

/* *****  Lexical analyzer  ***** */

/** Scan a list of tokens from text, using any character
    in delimiters as delimiter; the stack keywords is a
    stack of consecutive pairs name->value->... where
    values are void * pointers.
    A stack is returned, containing the list of tokens
    in reverse order, so that the last one is its top. */
stack_t scan(char *text, char *delimiters, stack_t keywords)
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
            v.t = str_new(text + 1, p - text - 1);
            tokens = stack_push(tokens, STRING, v);
            text = p + 1;
        } else {
            // Scans up to the following space or delimiter.
            char *p = text++;
            while (*text != '\0' && !isspace(*text)
            && strchr(delimiters, *text) == NULL)
                ++ text;
            v.t = str_new(p, text - p);

            // Check against a keyword.
            for (stack_t p = keywords; p != NULL; p = p->next->next) {
                // p is a stack with two elements: name->pointer
                assert(p->type == ATOM);
                if (strcmp(v.t, p->val.t) == 0) {
                    v.p = p->next->val.p;
                    tokens = stack_push(tokens, KEYWORD, v);
                    break;
                }
            }
            if (p == NULL) {
                // Check against a number.
                char *q;
                double n = strtod(p, &q);
                if (q == text) {
                    v.n = n;
                    tokens = stack_push(tokens, NUMBER, v);
                } else {
                    tokens = stack_push(tokens, ATOM, v);
                }
            }
        }
    }
    return tokens;
}

/* *****  Interpreter  ***** */

// Referenced before being defined:
stack_t awful_eval(stack_t *r_tokens, stack_t env);

/*
    Keywords routines.
*/

#define GETXY() \
    stack_t y = awful_eval(tokens, env);    \
    except_on(y->type != NUMBER, "Number expected");    \
    stack_t x = awful_eval(tokens, env);    \
    except_on(x->type != NUMBER, "Number expected");

stack_t ADD(stack_t *tokens, stack_t env)
{
    GETXY();
    stack_t retval = stack_push(NULL, NUMBER, x->val);
    retval->val.n += y->val.n;
    return retval;
}

stack_t SUB(stack_t *tokens, stack_t env)
{
    GETXY();
    stack_t retval = stack_push(NULL, NUMBER, x->val);
    retval->val.n -= y->val.n;
    return retval;
}

typedef stack_t (*keyword_t)(stack_t*, stack_t);

static struct { char *s; keyword_t k;} KEYWORDS[] = {
    {"ADD", ADD},
    {"SUB", SUB},
    {NULL, NULL}
};

/** Looks for an atom inside an stack of environments:
    if found then the stack item containing it is
    returned, else NULL. */
stack_t awful_find(char *t, stack_t e)
{
    for (; e != NULL; e = e->next) {
        /* We expect e to be a stack of pairs
            name->value -> name->value -> ... */
        assert(e->type == STACK);
        for (stack_t p = e->val.s; p != NULL; p = p->next->next) {
            assert(p->type == STRING);
            if (strcmp(t, p->val.t) == 0) {
                return p->next;
            }
        }
    }
    return NULL;
}

/** \note

    The interpreter expects in input a stack whose
    elements are "tokens", thus each item is a value
    representing a value or a symbol.
*/

/** Interpret a token list, w.r.t. an environment, both
    passed by reference, and return the stack with the
    result of the evaluation (tos = value of the expression).
    On error, return NULL. */
stack_t awful_eval(stack_t *r_tokens, stack_t env)
{
    stack_t tokens = *r_tokens;
    stack_t retval;
    except_on(tokens == NULL, "Unexpected end of text");
    switch (tokens->type) {
    case NUMBER: case STRING:
        retval = tokens;
        *r_tokens = tokens->next;
        break;
    case ATOM:
        retval = awful_find(tokens->val.t, env);
        except_on(retval == NULL, "Undefined variable %s", tokens->val.t);
        break;
    case KEYWORD:
        // A keyword has the address of its routine as value
        retval = ((keyword_t)(tokens->val.p))(stack_drop(r_tokens), env);
        break;
    case '{':
        assert(!"TODO");
    case '(':
        assert(!"TODO");
    default:
        except_on(1, "Bad expression");
    }
    return retval;
}

/** Interpret a string and return the corresponding
    value as a stack with a single item: if an error
    occurs, NULL is returned. */
stack_t awful(char *text)
{
    if (setjmp(except_buf) != 0) {
        return NULL;
    }
    // Creates the stack containing the consecutive
    // pairs name->address->... for keywords.
    stack_t keywords = NULL;
    for (int i = 0; KEYWORDS[i].s != NULL; ++ i) {
        val_t v;
        v.p = KEYWORDS[i].k;
        keywords = stack_push(keywords, KEYWORD, v);
        v.t = KEYWORDS[i].s;
        keywords = stack_push(keywords, ATOM, v);
    }

    stack_t tokens = scan(text, "(){},:", keywords);
    tokens = stack_reverse(tokens);

    for (stack_t s = tokens; s != NULL; s = s->next) {
        val_printf(stdout, s->type, s->val);
        putchar(' ');
    }

    // Creates the default environment: a stack containing
    // a stack of consecutive pairs name->address of keywords.
    val_t v; v.s = keywords;
    stack_t env = stack_push(env, STACK, v);
    stack_t retval = awful_eval(&tokens, &env);
    stack_delete(env);
    stack_delete(tokens);

    return retval;
}

int main(int argc, char **argv)
{
    awful("{x: ADD x 2}");
    return 0;
}
