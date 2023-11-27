/** \file awful.c */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header/awful.h"
#include "../header/awful_key.h"
#include "../header/except.h"
#include "../header/repl.h"
#include "../header/scan.h"
#include "../header/stack.h"
#include "../header/val.h"

/** Look for an atom inside a stack of environments: if found,
    then return a clone of the value of the variable. */
static val_t awful_find(char *t, stack_t e)
{
    // Expect e to be [name,value,...,name,value]
    for (; e != NULL; e = e->next)
        for (stack_t p = e->val.val.s; p != NULL; p = p->next->next)
            if (strcmp(t, p->val.val.t) == 0)
                return p->next->val;
    val_t none = {.type = NONE};
    return none;
}

/** Parses a sequence of tokens from r_tokens until the
    next "," or ")" is found: the ending ')' or ',' is
    parsed too but not included in the returned parsed text.
    Inside the sequence braces and parentheses can be nested. */
static stack_t awful_parse(stack_t *r_tokens)
{
    stack_t tokens = *r_tokens;
    except_on(tokens == NULL, "Unexpected end of text in actual parameter");
    int parentheses = 0;    // '('-')' match counter
    int braces = 0;         // '{'-'}' match counter
    stack_t body = NULL;
    int type;
    while (tokens != NULL && ((type = tokens->val.type) != ')' && type != ',' || parentheses > 0 /*|| braces > 0*/)) {
        parentheses += (type == '(') - (type == ')');
        braces += (type == '{') - (type == '}');
        body = stack_dup(tokens, body);
        tokens = tokens->next;
    }
    except_on(tokens == NULL, "Unexpected end of text in actual parameter");
    tokens = tokens->next;          // skip ')' or ','
    *r_tokens = tokens;
    return stack_reverse(body);
}

#ifdef DEBUG
// Debug stuff
static int __indent_ = 0;
#define RESET __indent_ = 0;
#define ENTER for (int i=0;i<__indent_;++i)putchar(' ');printf("> %s: tokens = ", __func__); stack_printf(stdout, *r_tokens); fputs(", env = ", stdout);stack_printf(stdout, env); putchar('\n'); fflush(stdout); ++ __indent_;
#define EXIT --__indent_; for (int i=0;i<__indent_;++i)putchar(' ');printf("< %s: ", __func__);val_printf(stdout, retval); printf("\ttokens = "); stack_printf(stdout, tokens); printf("\tenv = "); stack_printf(stdout, env); putchar('\n'); fflush(stdout);
#else
#define RESET
#define ENTER
#define EXIT
#endif

/** Parse the application of a closure to a list of actual
    parameters and return its value: *r_tokens is the
    "control stack" containing the next symbol to parse,
    a variable or ":", while env contains the current
    environment.*/
static val_t awful_application(stack_t *r_tokens, stack_t env)
{
ENTER
    // tokens = f [e1 "," ... "," en] ")"
    val_t f = awful_eval(r_tokens, env);
    stack_t tokens = *r_tokens;
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
        the result is pushed in assoc.
        Remember that fparams is a list t1->a1->t2->a2->...
        where ti is NONE or '!' and ai is the formal
        parameter name. */
    stack_t assoc = NULL;
    for (stack_t p = fparams; p != NULL; p = p->next) {
        if (p->val.type == NONE) {
            val_t v = awful_eval(&tokens, env);
            except_on(tokens->val.type != ','
                    && tokens->val.type != ')',
                "')' or ',' expected after actual parameters");
            tokens = tokens->next;  // skip ')' or ','
            assoc = stack_push(assoc, v);
        } else {
            stack_t actual = awful_parse(r_tokens);
            assoc = stack_push_s(assoc, actual);
        }
        p = p->next;    // skip the NONE/'!' item
        assoc = stack_push(assoc, p->val);
    }
    /*  Evaluates all expressions, corresponding to formal
        parameters marked by '!', in the environment env
        with assoc pushed in front of it. */
    stack_t new_env = (assoc == NULL) ? env : stack_push_s(env, assoc);
//~ printf("\naparams = "); stack_printf(stdout, assoc);
    for (stack_t ap = assoc, fp = fparams; ap != NULL;
    ap = ap->next->next, fp = fp->next->next) {
        if (fp->val.type == '!') {
            stack_t to_eval = ap->next->val.val.s;
            /*  Evaluate to_eval and substitute it with the
                resulting value. */
//~ printf("\nnew_env: "); stack_printf(stdout, new_env);
//~ printf("\nto_eval = "); stack_printf(stdout, to_eval);
            stack_t saved = to_eval;
            val_t retval = awful_eval(&to_eval, new_env);
            stack_delete(saved);
            ap->next->val = retval;
//~ printf("\nfparams = "); stack_printf(stdout, retval.val.s->val.val.s);
//~ printf("\nfbody = "); stack_printf(stdout, retval.val.s->next->val.val.s);
        }
    }
    // The environment in which to evaluate the
    // closure is [assoc] + fenv.
    new_env = (assoc == NULL) ? fenv : stack_push_s(fenv, assoc);

//~ printf("\nf: "); val_printf(stdout, f);
//~ printf("\nenv: "); stack_printf(stdout, env);
//~ printf("\nassoc: "); stack_printf(stdout, assoc);
//~ printf("\nbody"); stack_printf(stdout, body);
//~ printf("\nnew_env: "); stack_printf(stdout, new_env);
//~ printf("\nEVALUATE BODY "); stack_printf(stdout, body); fputc('\n', stdout);
    val_t retval = awful_eval(&body, new_env);
    // val_delete(f);
    // stack_delete(assoc);
//~ puts("OK!");
    
    *r_tokens = tokens;
EXIT
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
ENTER
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
        // Push NONE or '!' depending on the parameter being
        // evaluated immediately or delayed.
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
EXIT
    return retval;    
}

val_t awful_eval(stack_t *r_tokens, stack_t env)
{
ENTER
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
        except_on(retval.type == NONE,
            "Undefined variable %s", tokens->val.val.t);
        tokens = tokens->next;
        break;
    case KEYWORD: {
        // A keyword has the address of its routine as value
        awful_key_t k = (awful_key_t)tokens->val.val.p;
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
        val_printf(stderr, tokens->val);
        except_on(1, " unexpected here");
    }
    *r_tokens = tokens;
EXIT
    return retval;
}

int awful(char *text, FILE *file)
{
    val_t v = {.type = NONE};
    stack_t tokens = scan(text, "(){},:!", awful_key_find);
    if (tokens != NULL && setjmp(except_buf) == 0) {
        stack_t tokens_saved = tokens;
        v = awful_eval(&tokens_saved, NULL);
        if (v.type != NONE) {
            val_printf(file, v);
            val_delete(v);
        }
        fputc('\n', file);
    }
    stack_delete(tokens);   // also in case of exception
stack_status();
    return v.type == NONE;
}
