/** \file nice.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header/awful.h"
#include "../header/except.h"
#include "../header/nice.h"
#include "../header/scan.h"
#include "../header/stack.h"
#include "../header/str.h"
#include "../header/val.h"

/** Constants used to denote Niceful keywords: since scan()
    assumes a keyword is represented by a void* we cast these
    enumerated values to void*. Since NULL is (void*)0 we
    start enumerating from 1. */
#define LET (void*)1
#define LETREC (void*)2
#define IN (void*)3
#define FUN (void*)4
#define IF (void*)5
#define THEN (void*)6
#define ELSE (void*)7
#define AND (void*)8
#define OR (void*)9
#define NOT (void*)10
#define EQ (void*)11
#define NE (void*)12
#define LT (void*)13
#define LE (void*)14
#define GT (void*)15
#define GE (void*)16
#define PLUS (void*)17
#define MINUS (void*)18
#define TIMES (void*)19
#define SLASH (void*)20
#define HAT (void*)21
#define FIRST (void*)22
#define REST (void*)23

/** This function is neede by scan() to retrieve a unique code
    associated to each keyword of the language. */
static void *nice_key_find(char *t)
{
    return
        (strcmp(t, "let") == 0) ? LET:
        (strcmp(t, "letrec") == 0) ? LETREC:
        (strcmp(t, "in") == 0) ? IN:
        (strcmp(t, "fun") == 0) ? FUN:
        (strcmp(t, "if") == 0) ? IF:
        (strcmp(t, "then") == 0) ? THEN:
        (strcmp(t, "else") == 0) ? ELSE:
        (strcmp(t, "and") == 0) ? AND:
        (strcmp(t, "or") == 0) ? OR:
        (strcmp(t, "not") == 0) ? NOT:
        //(strcmp(t, "=") == 0) ? EQ:
        (strcmp(t, "==") == 0) ? EQ:
        (strcmp(t, "<>") == 0) ? NE:
        (strcmp(t, "<") == 0) ? LT:
        (strcmp(t, "<=") == 0) ? LE:
        (strcmp(t, ">") == 0) ? GT:
        (strcmp(t, ">=") == 0) ? GE:
        (strcmp(t, "+") == 0) ? PLUS:
        (strcmp(t, "-") == 0) ? MINUS:
        (strcmp(t, "*") == 0) ? TIMES:
        (strcmp(t, "/") == 0) ? SLASH:
        (strcmp(t, "^") == 0) ? HAT:
        (strcmp(t, "1st") == 0) ? FIRST:
        (strcmp(t, "rest") == 0) ? REST: NULL;
}

/** Expands a string by the size number. */
static char *str_exp(char *s, unsigned size)
{
    char *r = (s == NULL) ? malloc(size + 1)
                          : realloc(s, strlen(s) + size + 1);
    except_on(r == NULL, "String cannot be expanded anymore");
    return r;
}

/** Concatenates two strings: the s1 is expanded to make space
    for a copy of s2 that is appended to s1. The address of the
    new string (possibly still s1) is returned. */
static char *str_cat(char *s1, const char *s2)
{
    return strcat(str_exp(s1, strlen(s2)), s2);
}

/** Same as str_cat, but s2 is deallocated after
    being appended to s1. */
static char *str_catd(char *s1, char *s2)
{
    s1 = strcat(str_exp(s1, strlen(s2)), s2);
    str_del(s2);
    return s1;
}

/** Raise an error if s is empty, else parse its top and
    check that its type matches t. The stack with the top
    dropped is returned. */
static stack_t nice_expect(stack_t s, char t)
{
    if (s != NULL && (s->val.type == ATOM && s->val.val.t[0] == t && s->val.val.t[1] == '\0' || s->val.type == t))
        return stack_drop(s);
    fputc(t, stderr);
    except_on(1, " expected");
}

/** Raise an error if s is empty, else parse its top and
    check that it is keyword k. The stack with the top
    dropped is returned. */
static stack_t nice_expect_key(stack_t s, char *k)
{
    if (s != NULL && s->val.type == KEYWORD && s->val.val.p == nice_key_find(k))
        return stack_drop(s);
    fputs(k, stderr);
    except_on(1, " expected");
}

/** Raise an error if s is empty, else returns the value on
    the top item without popping it. */
static val_t nice_more(stack_t s)
{
    except_on(s == NULL, "Unexpected end of text");
    return s->val;
}

/** If s is NULL return NONE else the type of the item on top
    of the stack. */
static int nice_next(stack_t s)
{
    return (s != NULL) ? s->val.type : NONE;
}

/*
    The Niceful -> Awful translator is implemented by a brutal,
    cut/paste bag of recursive functions according to the following
    BNF grammar, just as Wirth taught to all us (symbols are enclosed
    between backslashes, that are not symbols of the language).

    expression = conditional
        | \let\ assignments \in\ expression
        | \letrec\ assignments \in\ expression

    assignments = [ atom \=\ conditional {\,\ atom \=\ conditional} ]

    conditional = proposition
        | \if\ proposition \then\ expression \else\ expression

    proposition = relation {bool-op proposition}

    relation = sum [ rel-op sum ] | \not\ relation
    
    sum = product { add-op product }
    
    product = term { mul-op term }
    
    power = term [\^\ term]
    
    term = number | atom | string | list
        | \-\ term | \1st\ term | \rest\ term 
        | \(\ expression \)\ | term { \(\ [expr-list] \)\ }
        | \fun\ {atom} \:\ expression

    expr-list = expression {\,\ expression}

    bool-op = \and\ | \or\
    rel-op = \=\ | \<>\ | \<\ | \>\ | \<=\ | \>=\
    add-op = \+\ | \-\ | \:\
    mul-op = \*\ | \/\

    string = \"\{character}\"\ | \'\{character}\'\
    list = \[\ [expr-list] \]\

*/

// Forward reference
static char *nice_expression(stack_t *r_nice);

#ifdef DEBUG
// Debug stuff
static int __indent_ = 0;
#define RESET __indent_ = 0;
#define ENTER for (int i=0;i<__indent_;++i)putchar(' ');printf("> %s: ", __func__); stack_printf(stdout, *r_nice); putchar('\n'); fflush(stdout); ++ __indent_;
#define EXIT -- __indent_; for (int i=0;i<__indent_;++i)putchar(' ');printf("< %s: %s, ", __func__, awful);stack_printf(stdout, *r_nice); putchar('\n'); fflush(stdout);
#else
#define RESET
#define ENTER
#define EXIT
#endif

/** Parse a list from *r_nice into a string which is returned;
    the value pointer by r_nice is updated. */
static char *nice_list(stack_t *r_nice)
{
ENTER
    stack_t nice = stack_drop(*r_nice);
    char *awful = NULL;
    if (nice_next(nice) != ']') {
        // [e1,...,en] -> PUSH e1 PUSH e2 ... PUSH en NIL
        for (;;) {
            awful = str_cat(awful, " PUSH "); // 1st space important!
            awful = str_catd(awful, nice_expression(&nice));
            if (nice_next(nice) == ']')
                break;
            nice = nice_expect(nice, ',');
        }
    }
    awful = str_cat(awful, " NIL");
    nice = stack_drop(nice);    // skip ']'
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a function from *r_nice into a string which is returned;
    the value pointer by r_nice is updated. */
static char *nice_fun(stack_t *r_nice)
{
    // Transform "fun x1 ... xn: e" into {x1 ... xn:e}
ENTER
    stack_t nice = stack_drop(*r_nice);
    char *awful = str_cat(NULL, "{");
    for (;;) {
        except_on(nice_next(nice) != ATOM,
            "Atom expected as function formal parameter");
        awful = str_cat(awful, nice->val.val.t);    // xi
        nice = stack_drop(nice);
        if (nice_next(nice) == ':')
            break;
        awful = str_cat(awful, " ");
    }
    nice = stack_drop(nice);    // skip ':'
    awful = str_cat(awful, ":");
    char *e = nice_expression(&nice);   // e
    awful = str_catd(awful, e);
    awful = str_cat(awful, "}");
    
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a possible actual parameter list from *r_nice into a
    string which is returned appended to the left parameter,
    that should contain the possible function just parsed;
    the value pointer by r_nice is updated. */
static char *nice_aparams(stack_t *r_nice, char *awful)
{
ENTER
    stack_t nice = *r_nice;
    while (nice_next(nice) == '(') {
        nice = stack_drop(nice);    // skip '('
        // Inserts a "(" on the left of awful
        awful = str_catd(str_cat(NULL, "("), awful);
        awful = str_cat(awful, " ");
        for (;;) {
            awful = str_catd(awful, nice_expression(&nice));
            if (nice_next(nice) == ')') {
                nice = stack_drop(nice);    // skip ')'
                awful = str_cat(awful, ")");
                break;
            } else {
                nice = nice_expect(nice, ',');
                awful = str_cat(awful, ",");
            }
        }
    }
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a term from *r_nice into a string which is returned;
    the value pointer by r_nice is updated. */
static char *nice_term(stack_t *r_nice)
{
    static char buf[BUFSIZ];
    char *awful = NULL;
ENTER
    stack_t nice = *r_nice;
    except_on(nice == NULL, "Unexpected end of text");
    switch (nice->val.type) {
        case NUMBER: {
            sprintf(buf, "%g", nice->val.val.n);
            awful = str_cat(NULL, buf);
            nice = stack_drop(nice);
            break;
        }
        case STRING: {
            char q[2];
            q[0] = (strchr(nice->val.val.t, '"') == NULL) ? '"' : '\'';
            q[1] = '\0';
            awful = str_cat(NULL, q);
            awful = str_cat(awful, nice->val.val.t);
            awful = str_cat(awful, q);
            nice = stack_drop(nice);
            break;
        }
        case ATOM: {
            awful = str_cat(NULL, nice->val.val.t);
            nice = stack_drop(nice);
            break;
        }
        case '[': {
            awful = nice_list(&nice);
            break;
        case '(': {
            nice = stack_drop(nice);
            awful = str_catd(NULL, nice_expression(&nice));
            nice = nice_expect(nice, ')');
            break;
        }
        default:
            except_on(nice->val.type != KEYWORD, "Syntax error");
            void *p = nice->val.val.p;
            nice = stack_drop(nice);
            if (p == MINUS) {
                awful = str_cat(NULL, "SUB 0 ");
                awful = str_catd(awful, nice_term(&nice));
            } else
            if (p == FIRST) {
                awful = str_cat(NULL, "TOS ");
                awful = str_catd(awful, nice_term(&nice));
            } else
            if (p == REST) {
                awful = str_cat(NULL, "BOS ");
                awful = str_catd(awful, nice_term(&nice));
            } else
            if (p == FUN) {
                awful = nice_fun(&nice);
            } else
                except_on(1, "Syntax error");
        }
    }
    // A term may be followed by a list of actual parameters
    // enclosed between parentheses.
    awful = nice_aparams(&nice, awful);
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a Niceful power from *r_nice and return a string
    containing the corresponding awful expression. Parsed token
    stack elements are deleted. */
static char *nice_power(stack_t *r_nice)
{
ENTER
    stack_t nice = *r_nice;
    char *awful = NULL;
    char *e = nice_term(&nice);
    if (nice == NULL || nice->val.val.p != HAT) awful = e;
    else {
        nice = stack_drop(nice);    // skip the operator
        awful = str_cat(NULL, "POW ");
        awful = str_catd(awful, e);
        awful = str_cat(awful, " ");
        awful = str_catd(awful, nice_term(&nice));
    }
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a Niceful product from *r_nice and return a string
    containing the corresponding awful expression. Parsed token
    stack elements are deleted. */
static char *nice_product(stack_t *r_nice)
{
ENTER
    stack_t nice = *r_nice;
    char *awful = NULL;
    char *e = nice_power(&nice);
    if (nice_next(nice) == ':') {
        nice = stack_drop(nice);    // skip the operator
        awful = str_cat(NULL, "PUSH ");
        awful = str_catd(awful, e);
        awful = str_cat(awful, " PUSH ");
        awful = str_catd(awful, nice_product(&nice));
        awful = str_cat(awful, " NIL");
    } else {
        int is_key = nice_next(nice) == KEYWORD;
        char *opt =
            (is_key && nice->val.val.p == TIMES) ? "MUL " :
            (is_key && nice->val.val.p == SLASH) ? "DIV " : NULL;
        if (opt == NULL) awful = e;
        else {
            nice = stack_drop(nice);    // skip the operator
            awful = str_cat(NULL, opt);
            awful = str_catd(awful, e);
            awful = str_cat(awful, " ");
            awful = str_catd(awful, nice_product(&nice));
        }
    }
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a Niceful sum from *r_nice and return a string
    containing the corresponding awful expression. Parsed token
    stack elements are deleted. */
static char *nice_sum(stack_t *r_nice)
{
ENTER
    stack_t nice = *r_nice;
    char *awful = NULL;
    char *e = nice_product(&nice);
    int is_key = nice_next(nice) == KEYWORD;
    char *opt =
        (is_key && nice->val.val.p == PLUS) ? "ADD " :
        (is_key && nice->val.val.p == MINUS) ? "SUB " : NULL;
    if (opt == NULL) awful = e;
    else {
        nice = stack_drop(nice);    // skip the operator
        awful = str_cat(NULL, opt);
        awful = str_catd(awful, e);
        awful = str_cat(awful, " ");
        awful = str_catd(awful, nice_sum(&nice));
    }
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a Niceful relation from *r_nice and return a string
    containing the corresponding awful expression. Parsed token
    stack elements are deleted. */
static char *nice_relation(stack_t *r_nice)
{
ENTER
    stack_t nice = *r_nice;
    char *awful = NULL;
    if (nice_more(nice).val.p == NOT) {
        awful = str_cat(NULL, "EQ 0 ");
        awful = str_catd(awful, nice_relation(&nice));
    } else {
        char *e = nice_sum(&nice);
        int is_key = nice_next(nice) == KEYWORD;
        char *opt =
            (nice_next(nice) == ATOM && nice->val.val.t[0] == '=' && nice->val.val.t[1] == '\0'
                || is_key && nice->val.val.p == EQ) ? "EQ ":
            (is_key && nice->val.val.p == NE) ? "NE ":
            (is_key && nice->val.val.p == LT) ? "LT ":
            (is_key && nice->val.val.p == LE) ? "LE ":
            (is_key && nice->val.val.p == GT) ? "GT ":
            (is_key && nice->val.val.p == GE) ? "GE ": NULL;
        if (opt == NULL) awful = e;
        else {
            nice = stack_drop(nice);    // skip the operator
            awful = str_cat(NULL, opt);
            awful = str_catd(awful, e);
            awful = str_cat(awful, " ");
            awful = str_catd(awful, nice_sum(&nice));
        }
    }
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a Niceful proposition from *r_nice and return a string
    containing the corresponding awful expression. Parsed token
    stack elements are deleted. */
static char *nice_proposition(stack_t *r_nice)
{
ENTER
    stack_t nice = *r_nice;
    char *awful = NULL;
    char *e = nice_relation(&nice);
    int is_key = nice_next(nice) == KEYWORD;
    char *opt =
        (is_key && nice->val.val.p == OR) ? "MAX " :
        (is_key && nice->val.val.p == AND) ? "MIN " : NULL;
    if (opt == NULL) awful = e;
    else {
        nice = stack_drop(nice);    // skip the operator
        awful = str_cat(NULL, opt);
        awful = str_catd(awful, e);
        awful = str_cat(awful, " ");
        awful = str_catd(awful, nice_proposition(&nice));
    }
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a Niceful conditional from *r_nice and returns a string
    containing the corresponding awful expression. Parsed token
    stack elements are deleted. */
static char *nice_conditional(stack_t *r_nice)
{
ENTER
    stack_t nice = *r_nice;
    char *awful = NULL;
    if (nice_more(nice).type != KEYWORD || nice->val.val.p != IF)
        awful = nice_proposition(&nice);
    else {
        // Transform "IF e1 e2 e3" into (COND e1{:e2}{:e3})
        nice = stack_drop(nice);    // skip IF
        awful = str_cat(awful, "(COND ");
        awful = str_catd(awful, nice_proposition(&nice));  // e1
        nice = nice_expect_key(nice, "then");
        awful = str_cat(awful, "{:");
        awful = str_catd(awful, nice_expression(&nice));  // e2
        awful = str_cat(awful, "}{:");
        nice = nice_expect_key(nice, "else");
        awful = str_catd(awful, nice_expression(&nice));  // e3
        awful = str_cat(awful, "})");
    }
    *r_nice = nice;
EXIT
    return awful;
}

/** Transform "let x1=v1, ..., xn=vn in e" into
    ({x1 ... xn:e} v1, ..., vn). The "let" or "letrec"
    keyword has been parsed, (rec == 1 in case of "letrec").
*/
static char *nice_let(stack_t *r_nice, int rec)
{
ENTER
    stack_t nice = stack_drop(*r_nice); // skip let/letrec
    char *awful = str_cat(NULL, "({");
    char *values = NULL;
    for (;;) {
        except_on(nice_next(nice) != ATOM,
            "Variables in let/letrec should be atoms");
        if (rec) awful = str_cat(awful, "!");
        awful = str_cat(awful, nice->val.val.t);   // variable xi
        nice = nice_expect(stack_drop(nice), '=');
        values = str_cat(values, nice_conditional(&nice));
        if (nice_more(nice).val.p == IN) {
            nice = stack_drop(nice);
            break;
        }
        nice = nice_expect(nice, ',');
        values = str_cat(values, ",");
        awful = str_cat(awful, " ");
    }
    // Here all pairs xi = vi are parsed, the list of
    // x1 ... xn dumped on awful and value contains the
    // list of all vn, ..., v1: we close it by ')'
    values = str_cat(values, ")");
    awful = str_cat(awful, ":");
    awful = str_catd(awful, nice_expression(&nice));    // body
    awful = str_cat(awful, "}");
    awful = str_cat(awful, values);
    awful = str_cat(awful, ")");

    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a Niceful expression from *r_nice and return a string
    containing the corresponding awful expression. Parsed token
    stack elements are deleted. */
static char *nice_expression(stack_t *r_nice)
{
ENTER
    stack_t nice = *r_nice;
    int is_key = nice_next(nice) == KEYWORD;
    char *awful =
        (is_key && nice->val.val.p == LET) ? nice_let(&nice, 0) :
        (is_key && nice->val.val.p == LETREC) ? nice_let(&nice, 1) :
            nice_conditional(&nice);
    *r_nice = nice;
EXIT
    return awful;
}

int nice(char *text, FILE *file)
{
    stack_t tokens = scan(text, "()[]{},:", nice_key_find);
    int err = tokens == NULL;
    if (!err && setjmp(except_buf) == 0) {
RESET   char *t = nice_expression(&tokens);
        // notice that nice_expression deletes the stack
        // tokens but allocates the string t.
        if (tokens != NULL) {
            fprintf(stderr, "Warning: text after expression shall be ignored:");
            while (tokens != NULL) {
                fputc(' ', stderr);
                val_printf(stderr, tokens->val);
                tokens = stack_drop(tokens);
            }
            fputc('\n', stderr);
        }
        err = t == NULL;
        if (!err) {
fprintf(stderr, "\nAwful: %s\n", t);
            err = awful(t, file);
            str_del(t);
        }
    }
    stack_delete(tokens);   // also in case of exception
    return err;
}
