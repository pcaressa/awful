/** \file awful.c */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header/except.h"
#include "../header/repl.h"
#include "../header/scan.h"
#include "../header/stack.h"
#include "../header/val.h"

// Referenced before being defined:
static val_t awful_eval(stack_t *r_tokens, stack_t env);

/*
    Keywords routines.
*/

/** Parses two expressions and check their values are numbers. */
#define GETXY() \
    val_t y = awful_eval(tokens, env);    \
    except_on(y.type != NUMBER, "Number expected");    \
    val_t x = awful_eval(tokens, env);    \
    except_on(x.type != NUMBER, "Number expected");

static val_t ADD(stack_t *tokens, stack_t env)
{
    GETXY();
    x.val.n += y.val.n;
    return x;
}

static val_t BOS(stack_t *tokens, stack_t env)
{
    val_t x = awful_eval(tokens, env);
    except_on(x.type != STACK, "TOS x needs x to be a stack");
    x.val.s = x.val.s->next;
    return x;
}

static val_t COND(stack_t *tokens, stack_t env)
{
    val_t x = awful_eval(tokens, env);
    val_t y = awful_eval(tokens, env);
    val_t z = awful_eval(tokens, env);
    except_on(x.type != NUMBER, "Number expected in COND");
    return (x.val.n) ? y : z;
}

static val_t DIV(stack_t *tokens, stack_t env)
{
    GETXY();
    x.val.n /= y.val.n;
    return x;
}

static val_t EQ(stack_t *tokens, stack_t env)
{
    val_t y = awful_eval(tokens, env);
    val_t x = awful_eval(tokens, env);
    double flag = 0.0;
    int type = x.type;
    if (type == y.type) {
        if (type == NUMBER) {
            flag = x.val.n == y.val.n;
        } else
        if (type == STRING || type == ATOM) {
            flag = strcmp(x.val.t, y.val.t) == 0;
        } else {
            except_on(1, "EQ applies only to atoms");
        }
    }
    x.val.n = flag;
    return x;
}

static val_t LE(stack_t *tokens, stack_t env)
{
    GETXY();
    x.val.n = x.val.n <= y.val.n;
    return x;
}

static val_t LT(stack_t *tokens, stack_t env)
{
    GETXY();
    x.val.n = x.val.n < y.val.n;
    return x;
}

static val_t MAX(stack_t *tokens, stack_t env)
{
    GETXY();
    return x.val.n > y.val.n ? x : y;
}

static val_t MIN(stack_t *tokens, stack_t env)
{
    GETXY();
    return x.val.n < y.val.n ? x : y;
}

static val_t MUL(stack_t *tokens, stack_t env)
{
    GETXY();
    x.val.n *= y.val.n;
    return x;
}

static val_t NE(stack_t *tokens, stack_t env)
{
    val_t retval = EQ(tokens, env);
    retval.val.n = !retval.val.n;
    return retval;
}

static val_t NIL(stack_t *tokens, stack_t env)
{
    val_t v = {.type = STACK, .val.s = NULL};
    return v;
}

static val_t POW(stack_t *tokens, stack_t env)
{
    GETXY();
    x.val.n = pow(x.val.n, y.val.n);
    return x;
}

static val_t PUSH(stack_t *tokens, stack_t env)
{
    val_t x = awful_eval(tokens, env);
    val_t y = awful_eval(tokens, env);
    except_on(y.type != STACK, "PUSH x y needs y to be a stack");
    y.val.s = stack_push(y.val.s, x);
    return y;
}

static val_t SUB(stack_t *tokens, stack_t env)
{
    GETXY();
    x.val.n -= y.val.n;
    return x;
}

static val_t TOS(stack_t *tokens, stack_t env)
{
    val_t x = awful_eval(tokens, env);
    except_on(x.type != STACK, "TOS x needs x to be a stack");
    return x.val.s->val;
}

typedef val_t (*keyword_t)(stack_t*, stack_t);

static struct { char *s; val_t k;} KEYWORDS[] = {
#   define R(x) {#x, {.type = KEYWORD, .val.p = (void*)x}},
#   include "keywords.h"
#   undef R
};

/** Look for an atom inside an stack of environments:
    if found then return a clone of the value of the
    variable. */
static val_t awful_find(char *t, stack_t e)
{
    // We expect e to [name,value,...,name,value]
    for (; e != NULL; e = e->next)
        for (stack_t p = e->val.val.s; p != NULL; p = p->next->next)            if (strcmp(t, p->val.val.t) == 0) {
                return p->next->val;
            }
    val_t none = {.type = NONE};
    return none;
}

/** \note

    The interpreter expects in input a stack whose
    elements are "tokens", thus each item is a value
    representing a value or a symbol.
*/

/** Parses a sequence of tokens from r_tokens until the
    next "," or ")" is found: inside the sequence braces
    and parentheses can be nested. */
static stack_t awful_parse(stack_t *r_tokens)
{
    stack_t tokens = *r_tokens;
    except_on(tokens == NULL, "Unexpected end of text");
    
    int parentheses = 0;    // '('-')' matches counter
    stack_t body = NULL;
    int type;
    while (tokens != NULL && ((type = tokens->val.type) != ')' && type != ',' || parentheses > 0)) {
        parentheses = (type == '(') - (type == ')');
        body = stack_dup(tokens, body);
        tokens = tokens->next;
    }
    except_on(tokens == NULL, "Unexpected end of text");
    tokens = tokens->next;          // skip ')' or ','
    
    *r_tokens = tokens;
    return stack_reverse(body);
}

/** Parse the application of a closure to a list of actual
    parameters and return its value: *r_tokens is the
    "control stack" containing the next symbol to parse,
    a variable or ":", while env contains the current
    environment.*/
static val_t awful_application(stack_t *r_tokens, stack_t env)
{
//fprintf(stderr, "> awful_application\n");

    stack_t tokens = *r_tokens;
    except_on(tokens == NULL, "Unexpected end of text");

    // tokens = f [e1 "," ... "," en] ")"
    
    val_t f = awful_eval(&tokens, env);
    except_on(f.type != CLOSURE, "Function expected");
    // f.val.s = [[x1...xn] [body] [fenv]]
    // f.val.s.val.s = [x1, ...,xn]
    // f.next.val.s = [body]
    // f.next.next.val.s = [fenv]
    stack_t fparams = f.val.s->val.val.s;
    stack_t body = f.val.s->next->val.val.s;
    stack_t fenv = f.val.s->next->next->val.val.s;

    /* For each formal parameter parse an expression and
        associate it to the parameter inside assoc. */
    stack_t assoc = NULL;
    for (stack_t p = fparams; p != NULL; p = p->next) {
        stack_t pair = awful_parse(&tokens);
        //pair = stack_push(pair, STACK, p);
        assoc = stack_push(pair, p->val);
        //assoc = stack_push(assoc, STACK, pair);
    }

    // Evaluates all expressions in the environment assoc
    for (stack_t p = assoc; p != NULL; p = p->next->next) {
        stack_t pn = p->next;
        // Evaluate and substitute the value
        stack_t pn_saved = pn;
        val_t retval = awful_eval(&pn, env);
        p->next->val = retval;
        stack_drop(pn_saved);
    }
    val_t v = {.type = STACK, .val.s = assoc};
    stack_t new_env = stack_push(fenv, v);

//fputs("assoc=", stderr); stack_printf(stderr, assoc); fputc('\n', stderr);
//fputs("fenv=", stderr); stack_printf(stderr, fenv); fputc('\n', stderr);
//fputs("env=", stderr); stack_printf(stderr, env); fputc('\n', stderr);
//fputs("body=", stderr); stack_printf(stderr, body); fputc('\n', stderr);

    val_t retval = awful_eval(&body, new_env);
    stack_delete(assoc);
    stack_delete(new_env);
    val_delete(f);
    
//fprintf(stderr, "< awful_application\n");

    *r_tokens = tokens;
    return retval;
}

/** Parse a closure and return it: *r_tokens is the
    "control stack" containing the next symbol to parse,
    a variable or ":", while env contains the current
    environment. A closure is a stack of the form
    [[x1,...,xn],[body],[fenv]], so the returned value
    is this stack (or NONE in case of error). */
static val_t awful_closure(stack_t *r_tokens, stack_t env)
{
//fprintf(stderr, "> awful_closure\n");
    stack_t tokens = *r_tokens;
    except_on(tokens == NULL, "Unexpected end of text");

    // tokens = [a1 ... an] ":" e "}"
    stack_t params = NULL;
    while (tokens->val.type != ':') {
        except_on(tokens->val.type != ATOM, "Atom expected");
        params = stack_push(params, tokens->val);
        tokens = tokens->next;
    }
    tokens = tokens->next;  // skip ':'

//fputs("params=", stderr); stack_printf(stderr, params); fputc('\n', stderr);
    
//stack_printf(stderr, tokens); fputc('\n', stderr);
    stack_t body = NULL;
    int braces = 0;
    int type;
    while (tokens != NULL && ((type = tokens->val.type) != '}' || braces > 0)) {
        braces += (type == '{') - (type == '}');
        body = stack_dup(tokens, body);
        tokens = tokens->next;
    }
    except_on(tokens == NULL, "Unexpected end of text");
    tokens = tokens->next;  // skip the '}'
    body = stack_reverse(body);

//fputs("body=", stderr); stack_printf(stderr, body); fputc('\n', stderr);
    
    // Creates the closure [params, body, env]
    val_t retval = {.type = STACK, .val.s = NULL};

    val_t v = {.type = STACK, .val.s = env};
    retval.val.s = stack_push(retval.val.s, v);

    v.val.s = body;
    retval.val.s = stack_push(retval.val.s, v);

    v.type = CLOSURE;
    v.val.s = params;
    retval.val.s = stack_push(retval.val.s, v);
    
//fputs("fenv=", stderr); stack_printf(stderr, env); fputc('\n', stderr);
    *r_tokens = tokens;
    
//fprintf(stderr, "< awful_closure\n");
    return retval;    
}

/** Interpret a token list, w.r.t. an environment, both
    passed by reference, and return the value with the
    result of the evaluation. On error, the returned value
    is NONE. */
static val_t awful_eval(stack_t *r_tokens, stack_t env)
{
//fprintf(stderr, "> awful_eval(");stack_printf(stderr, *r_tokens);fprintf(stderr, ", ");stack_printf(stderr, env);fprintf(stderr, ")\n");
    
    stack_t tokens = *r_tokens;
    val_t retval;
    except_on(tokens == NULL, "Unexpected end of text");
    
    switch (tokens->val.type) {
    case NUMBER:
    case STRING:
        retval = tokens->val;
        tokens = tokens->next;
        break;
    case ATOM:
        retval = awful_find(tokens->val.val.t, env);
        except_on(retval.type == NONE, "Undefined variable %s", tokens->val.val.t);
        tokens = tokens->next;
        break;
    case KEYWORD: {
        // A keyword has the address of its routine as value
        keyword_t k = (keyword_t)tokens->val.val.p;
        tokens = tokens->next;
        retval = (*k)(&tokens, env);
        break;
    }
    case '{':
        tokens = tokens->next;
        retval = awful_closure(&tokens, env);
        break;
    case '(':
        tokens = tokens->next;
        retval = awful_application(&tokens, env);
        break;
    default:
//fprintf(stderr, "\n%i[%c]\n", tokens->type, tokens->type);
        except_on(1, "Bad expression");
    }
    *r_tokens = tokens;
    return retval;
}

/** Interpret a string and return the corresponding a string
    expressing the resulting value: this string is inside a
    hidden buffer. If an error occurs, a non zero error core
    is returned. */
int awful(char *text, FILE *file)
{
    if (setjmp(except_buf) != 0) return -1;
    
    // Creates the stack containing the consecutive
    // pairs name->address->... for keywords.
    stack_t keywords = NULL;
    for (int i = 0; KEYWORDS[i].k.val.t != NULL; ++ i) {
        keywords = stack_push(keywords, KEYWORDS[i].k);
        val_t v = {.type = ATOM, .val.t = KEYWORDS[i].s};
        keywords = stack_push(keywords, v);
    }
    // Scans the text into the tokens stack.
    stack_t tokens = scan(text, "(){},:", keywords);
    tokens = stack_reverse(tokens);

    stack_t tokens0 = tokens;
    val_t v = awful_eval(&tokens, NULL);
    stack_delete(tokens0);
    
    if (v.type == NONE) return -1;

    val_printf(file, v);
    val_delete(v);

    return 0;
}

int main(int argc, char **argv)
{
    fputs("AWFUL - A Weird FUnctional Language\n", stderr);
    fputs("(c) 2023 by Paolo Caressa\n", stderr);
    return repl(awful, "awful");
}
