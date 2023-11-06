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
    stack_t body = NULL;
    int type;
    while (tokens != NULL && ((type = tokens->val.type) != ')' && type != ',' || parentheses > 0)) {
        parentheses = (type == '(') - (type == ')');
        body = stack_dup(tokens, body);
        tokens = tokens->next;
    }
    except_on(tokens == NULL, "Unexpected end of text in actual parameter");
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

val_t awful_eval(stack_t *r_tokens, stack_t env)
{
    stack_t tokens = *r_tokens;
    except_on(tokens == NULL, "Unexpected end of text");
    
    val_t v;
    switch (tokens->val.type) {
    case NUMBER:
    case STRING:
        v = tokens->val;
        tokens = tokens->next;
        break;
    case ATOM:
        v = awful_find(tokens->val.val.t, env);
        except_on(v.type == NONE, "Undefined variable %s", tokens->val.val.t);
        tokens = tokens->next;
        break;
    case KEYWORD: {
        // A keyword has the address of its routine as value
        awful_key_t k = (awful_key_t)tokens->val.val.p;
        tokens = tokens->next;
        v = (*k)(&tokens, env);
        break;
    }
    case '{':
        tokens = tokens->next;
        v = awful_closure(&tokens, env);
        break;
    case '(':
        tokens = tokens->next;
        v = awful_application(&tokens, env);
        break;
    default:
        except_on(1, "Unknown token");
    }
    *r_tokens = tokens;
    return v;
}

int awful(char *text, FILE *file)
{    
    if (setjmp(except_buf) != 0) return -1;

    // Scans the text into the tokens stack.
    stack_t tokens = scan(text, "(){},:!", awful_key_find);
    
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
