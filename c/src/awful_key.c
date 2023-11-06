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

void *awful_key_find(char *text)
{
    /*  Associative table names:subroutines. */
    static struct { char *name; awful_key_t subr; } keys[] = {
        {"ADD", ADD},
        {"BOS", BOS},
        {"COND", COND},
        {"DIV", DIV},
        {"EQ", EQ},
        {"LE", LE},
        {"LT", LT},
        {"MAX", MAX},
        {"MIN", MIN},
        {"MUL", MUL},
        {"NE", NE},
        {"NIL", NIL},
        {"POW", POW},
        {"PUSH", PUSH},
        {"SUB", SUB},
        {"TOS", TOS},
    };
    for (int i = 0; i < sizeof(keys)/sizeof(*keys); ++ i) {
        if (strcmp(keys[i].name, text) == 0)
            return (void*)keys[i].subr;
    }
    return NULL;
}
