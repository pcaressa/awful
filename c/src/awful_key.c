/** \file afwul_key.c */

/** This submodule of the awful module manages built-in
    functions, aka keywords. */

#include <math.h>
#include <stdlib.h>
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
    x.val.s = (x.val.s == NULL) ? NULL : x.val.s->next;
    return x;
}

static val_t COND(stack_t *tokens, stack_t env)
{
    val_t x = awful_eval(tokens, env);
    except_on(x.type != NUMBER, "Number expected in COND");
    val_t y = awful_eval(tokens, env);
    val_t z = awful_eval(tokens, env);
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
            free(x.val.t);
            free(y.val.t);
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

static val_t ISNIL(stack_t *tokens, stack_t env)
{
    val_t x = awful_eval(tokens, env);
    x.val.n = x.type == STACK && x.val.s == NULL;
    x.type = NUMBER;
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
    except_on(x.val.s == NULL, "TOS applied to an empty stack");
    return x.val.s->val;
}

void *awful_key_find(char *t, unsigned n)
{
    // Silly and baroque statement replacing a for loop
    // through a table of pairs (string, addr) or its
    // unfolding as a sequence of if (memcmp...)
    return
        (n == 2) ? (
            (t[0] == 'E' && t[1] == 'Q') ? EQ:
            (t[0] == 'G' && t[1] == 'E') ? GE:
            (t[0] == 'G' && t[1] == 'T') ? GT:
            (t[0] == 'L' && t[1] == 'E') ? LE:
            (t[0] == 'L' && t[1] == 'T') ? LT:
            (t[0] == 'N' && t[1] == 'E') ? NE: NULL) :
        (n == 3) ? (
            (t[0] == 'A' && t[1] == 'D' && t[2] == 'D') ? ADD:
            (t[0] == 'B' && t[1] == 'O' && t[2] == 'S') ? BOS:
            (t[0] == 'D' && t[1] == 'I' && t[2] == 'V') ? DIV:
            (t[0] == 'M' && t[1] == 'A' && t[2] == 'X') ? MAX:
            (t[0] == 'M' && t[1] == 'I' && t[2] == 'N') ? MIN:
            (t[0] == 'M' && t[1] == 'U' && t[2] == 'L') ? MUL:
            (t[0] == 'N' && t[1] == 'I' && t[2] == 'L') ? NIL:
            (t[0] == 'P' && t[1] == 'O' && t[2] == 'W') ? POW:
            (t[0] == 'S' && t[1] == 'U' && t[2] == 'B') ? SUB:
            (t[0] == 'T' && t[1] == 'O' && t[2] == 'S') ? TOS: NULL) :
        (n == 4) ? (
            (t[0] == 'C' && t[1] == 'O' && t[2] == 'N' && t[3] == 'D') ? COND:
            (t[0] == 'P' && t[1] == 'U' && t[2] == 'S' && t[3] == 'H') ? PUSH: NULL) :
        (n == 5) ? (
            (t[0] == 'I' && t[1] == 'S' && t[2] == 'N' && t[3] == 'I' && t[4] == 'L') ? ISNIL: NULL)
        : NULL;
}
