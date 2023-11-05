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

/** The list of keywords will be stored inside this stack. */
static stack_t awful_keywords = NULL;

/** Look for an atom inside an stack of environments:
    if found then return a clone of the value of the
    variable. */
static val_t awful_find(char *t, stack_t e)
{
    // We expect e to be [name,value,...,name,value]
    for (; e != NULL; e = e->next)
        for (stack_t p = e->val.val.s; p != NULL; p = p->next->next)
            if (strcmp(t, p->val.val.t) == 0)
                return p->next->val;
    val_t none = {.type = NONE};
    return none;
}

/** \note

    The interpreter expects in input a stack whose
    elements are "tokens", thus each item is a value
    representing a value or a symbol.
*/

/** Parses a sequence of tokens from r_tokens until the
    next "," or ")" is found: the ending ')' or ',' is
    parsed too but not included in the returned parsed text.
    Inside the sequence braces and parentheses can be nested. */
static stack_t awful_parse(stack_t *r_tokens)
{
    stack_t tokens = *r_tokens;
    except_on(tokens == NULL,
        "Unexpected end of text in actual parameter");
    int parentheses = 0;    // '('-')' match counter
    stack_t body = NULL;
    int type;
    while (tokens != NULL && ((type = tokens->val.type) != ')' && type != ',' || parentheses > 0)) {
        parentheses = (type == '(') - (type == ')');
        body = stack_dup(tokens, body);
        tokens = tokens->next;
    }
    except_on(tokens == NULL,
        "Unexpected end of text in actual parameter");

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
    stack_t tokens = *r_tokens;
    except_on(tokens == NULL, "Unexpected end of text");

    // tokens = f [e1 "," ... "," en] ")"
    
    val_t f = awful_eval(&tokens, env);

    except_on(f.type != CLOSURE, "Function expected");
    /*  Notice that closure f is represented as:
            f.val.s = [[x1...xn] [body] [fenv]]
            f.val.s.val.s = [x1, ...,xn]
            f.next.val.s = [body]
            f.next.next.val.s = [fenv]  */
    stack_t fparams = f.val.s->val.val.s;
    stack_t body = f.val.s->next->val.val.s;
    stack_t fenv = f.val.s->next->next->val.val.s;

    /*  For each formal parameter parse an expression which
        is its actual parameters: if the formal parameter
        is marked as NONE, the actual parameter is not
        parsed but evaluated, else it'll be evaluated after
        all actual parameters have been parsed. In any case,
        the result is pushed in assoc. */
    stack_t assoc = NULL;
    for (stack_t p = fparams; p != NULL; p = p->next) {
        if (p->val.type == NONE) {
            val_t v = awful_eval(&tokens, env);
            except_on(tokens->val.type != ','
                    && tokens->val.type != ')',
                "')' or ',' expected after actual parameters");
            tokens = tokens->next;  // skip ')' or ','
            assoc = stack_push(assoc, v);
            p = p->next;    // skip the NONE item
            assoc = stack_push(assoc, p->val);
        } else {
            stack_t actual = awful_parse(r_tokens);
            assoc = stack_push_s(assoc, actual);
            p = p->next;
            assoc = stack_push(assoc, p->val);
        }
    }
    /*  Evaluates all expressions in the environment assoc
        corresponding to formal parameters marked by '!'. */
    for (stack_t ap = assoc, fp = fparams;
            ap != NULL;
                ap = ap->next->next, fp = fp->next->next) {
        if (fp->val.type == '!') {
            stack_t to_eval = ap->next->val.val.s;
            /*  Evaluate to_eval and substitute it with the
                resulting value. */
            stack_t saved = to_eval;
            val_t retval = awful_eval(&to_eval, env);
            ap->next->val = retval;
            stack_delete(saved);
        }
    }
    stack_t new_env = stack_push_s(fenv, assoc);

    val_t retval = awful_eval(&body, new_env);
    val_delete(f);
    
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
    stack_t tokens = *r_tokens;
    except_on(tokens == NULL, "Unexpected end of text");

    // tokens = [a1 ... an] ":" ... "}"
    /*  A formal parameter is actually a couple of consecutive
        values t,a where t has type NONE or '!' and a is an
        atom. If t is NONE the parameter shall be evaluated
        when parsed, else it shall be evaluated after all
        actual parameters are parsed, thus allowing for
        recursive definitions. */
    stack_t params = NULL;
    while (tokens->val.type != ':') {
        // A formal parameter can be an atom or !atom
        val_t v = {.type = NONE};
        if (tokens->val.type == '!') {
            v.type = '!';
            tokens = tokens->next;
        }
        params = stack_push(params, v);
        except_on(tokens->val.type != ATOM,
                  "Atom expected as closure formal parameter");
        params = stack_push(params, tokens->val);
        tokens = tokens->next;
    }
    params = stack_reverse(params);
    tokens = tokens->next;  // skip ':'

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

    // Creates the closure as a stack [params, body, env]
    stack_t s = NULL;
    s = stack_push_s(s, env);
    s = stack_push_s(s, body);
    s = stack_push_s(s, params);
    val_t retval = {.type = CLOSURE, .val.s = s};
    
    *r_tokens = tokens;
    return retval;    
}

/** Interpret a token list, w.r.t. an environment, both
    passed by reference, and return the value with the
    result of the evaluation. On error, the returned value
    is NONE. */
static val_t awful_eval(stack_t *r_tokens, stack_t env)
{
    stack_t tokens = *r_tokens;
    except_on(tokens == NULL, "Unexpected end of text");
    
    val_t retval;
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
        except_on(1, "Unknown token");
    }
    *r_tokens = tokens;
    return retval;
}

/** Interpret a string and print the resulting value.
    If an error occurs, a non zero error code is returned. */
int awful(char *text, FILE *file)
{    
    if (setjmp(except_buf) != 0) return -1;

    // Scans the text into the tokens stack.
    stack_t tokens = scan(text, "(){},:!", awful_keywords);
    
    tokens = stack_reverse(tokens);
    stack_t tokens_saved = tokens;
    val_t v = awful_eval(&tokens, NULL);
    stack_delete(tokens_saved);
    if (v.type != NONE) {
        val_printf(file, v);
        val_delete(v);
    }
    fputc('\n', file);
    return v.type == NONE;
}

int main(int argc, char **argv)
{
    /*  Creates the stack containing the consecutive pairs
        name->address->... for keywords that it is needed
        by scan. We use some macro black magic including
        the file "keywords.h" that contains the list of
        all keyword names as R(name). */
    val_t v;
#   define R(x)                                         \
        v.type = KEYWORD; v.val.p = x;                  \
        awful_keywords = stack_push(awful_keywords, v); \
        v.type = ATOM; v.val.t = #x;                    \
        awful_keywords = stack_push(awful_keywords, v);
#   include "keywords.h"
#   undef R

    fputs("AWFUL - A Weird FUnctional Language\n", stderr);
    fputs("(c) 2023 by Paolo Caressa\n", stderr);
    repl(stdin, stderr, awful, "awful");

    stack_delete(awful_keywords);
    return 0;
}
