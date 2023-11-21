/** \file afwul_key.c */

/** This submodule of the awful module manages built-in
    functions, aka keywords. */

#include <math.h>
#include <string.h>
#include "../header/awful.h"
#include "../header/awful_key.h"
#include "../header/except.h"
#include "../header/stack.h"
#include "../header/val.h"

/** Parses two expressions and check their values are numbers. */
#define GETXY() \
    val_t x = awful_eval(tokens, env);    \
    except_on(x.type != NUMBER, "Number expected");    \
    val_t y = awful_eval(tokens, env);    \
    except_on(y.type != NUMBER, "Number expected");

static val_t ADD(stack_t *tokens, stack_t env)
{
    GETXY();
    x.val.n += y.val.n;
    return x;
}

static val_t BOS(stack_t *tokens, stack_t env)
{
    val_t x = awful_eval(tokens, env);
    except_on(x.type != STACK, "BOS x needs x to be a stack");
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
    val_t x = awful_eval(tokens, env);
    val_t y = awful_eval(tokens, env);
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

static val_t GE(stack_t *tokens, stack_t env)
{
    GETXY();
    x.val.n = x.val.n >= y.val.n;
    return x;
}

static val_t GT(stack_t *tokens, stack_t env)
{
    GETXY();
    x.val.n = x.val.n > y.val.n;
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

void *awful_key_find(char *t)
{
    return
        (strcmp(t, "ADD") == 0) ? ADD:
        (strcmp(t, "BOS") == 0) ? BOS:
        (strcmp(t, "COND") == 0) ? COND:
        (strcmp(t, "DIV") == 0) ? DIV:
        (strcmp(t, "EQ") == 0) ? EQ:
        (strcmp(t, "GE") == 0) ? GE:
        (strcmp(t, "GT") == 0) ? GT:
        (strcmp(t, "LE") == 0) ? LE:
        (strcmp(t, "LT") == 0) ? LT:
        (strcmp(t, "MAX") == 0) ? MAX:
        (strcmp(t, "MIN") == 0) ? MIN:
        (strcmp(t, "MUL") == 0) ? MUL:
        (strcmp(t, "NE") == 0) ? NE:
        (strcmp(t, "NIL") == 0) ? NIL:
        (strcmp(t, "POW") == 0) ? POW:
        (strcmp(t, "PUSH") == 0) ? PUSH:
        (strcmp(t, "SUB") == 0) ? SUB:
        (strcmp(t, "TOS") == 0) ? TOS: NULL;
}
