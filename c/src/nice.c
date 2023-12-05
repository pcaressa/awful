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

/** Delimiters of the Niceful language: they are token in
    themselves, not ATOMs. */
#define DELIMITERS "()[]{},:"

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
#define NNIL (void*)22
#define EMPTY (void*)23
#define FIRST (void*)24
#define REST (void*)25

/** This function is needed by scan() to retrieve a unique code
    associated to each keyword of the language. */
static void *nice_key_find(char *t, unsigned n)
{
    // Silly and baroque statement replacing a for loop
    // through a table of pairs (string, addr) or its
    // unfolding as a sequence of if (memcmp...)
    return
        (n == 1) ? (
            (t[0] == '+') ? PLUS:
            (t[0] == '-') ? MINUS:
            (t[0] == '*') ? TIMES:
            (t[0] == '/') ? SLASH:
            (t[0] == '<') ? LT:
            (t[0] == '>') ? GT:
            (t[0] == '^') ? HAT: NULL) :
        (n == 2) ? (
            (t[0] == '<' && t[1] == '=') ? LE:
            (t[0] == '<' && t[1] == '>') ? NE:
            (t[0] == '=' && t[1] == '=') ? EQ:
            (t[0] == '>' && t[1] == '=') ? GE:
            (t[0] == 'i' && t[1] == 'f') ? IF:
            (t[0] == 'i' && t[1] == 'n') ? IN:
            (t[0] == 'o' && t[1] == 'r') ? OR: NULL) :
        (n == 3) ? (
            (t[0] == '1' && t[1] == 's' && t[2] == 't') ? FIRST:
            (t[0] == 'a' && t[1] == 'n' && t[2] == 'd') ? AND:
            (t[0] == 'f' && t[1] == 'u' && t[2] == 'n') ? FUN:
            (t[0] == 'l' && t[1] == 'e' && t[2] == 't') ? LET:
            (t[0] == 'n' && t[1] == 'i' && t[2] == 'l') ? NNIL:
            (t[0] == 'n' && t[1] == 'o' && t[2] == 't') ? NOT: NULL) :
        (n == 4) ? (
            (t[0] == 'e' && t[1] == 'l' && t[2] == 's' && t[3] == 'e') ? THEN:
            (t[0] == 't' && t[1] == 'h' && t[2] == 'e' && t[3] == 'n') ? THEN:
            (t[0] == 'r' && t[1] == 'e' && t[2] == 's' && t[3] == 't') ? REST: NULL) :
        (n == 5) ? (
            (t[0] == 'e' && t[1] == 'm' && t[2] == 'p' && t[3] == 't' && t[4] == 'y') ? EMPTY: NULL) :
        (n == 6) ? (
            (t[0] == 'l' && t[1] == 'e' && t[2] == 't' && t[3] == 'r' && t[4] == 'e' && t[5] == 'c') ? LETREC: NULL)
        : NULL;
}

/** Raise an error if s is empty, else pop its top and
    check that its type matches t if t is a delimiter,
    else that its type is ATOM and its value the string
    with the single character t. The stack with the top
    dropped is returned. */
static stack_t nice_expect(stack_t s, char t)
{
    except_on(s == NULL || (strchr(DELIMITERS, t) != NULL
            ? s->val.type != t : s->val.type != ATOM
            || s->val.val.t[0] != t || s->val.val.t[1] != '\0'),
        "%c expected", t);
    return stack_next(s);
}

/** Raise an error if s is empty, else parse its top and
    check that it is keyword k. The stack with the top
    dropped is returned. */
static stack_t nice_expect_key(stack_t s, char *k)
{
    if (s == NULL || s->val.type != KEYWORD
    || s->val.val.p != nice_key_find(k, strlen(k))) {
        fputs(k, stderr);
        except_on(1, " expected");
    }
    return stack_next(s);
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
static inline int nice_next(stack_t s)
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
#define ENTER for (int i=0;i<__indent_;++i)putchar(' ');printf("> %s: ", __func__); stack_fprint(stdout, *r_nice); putchar('\n'); fflush(stdout); ++ __indent_;
#define EXIT -- __indent_; for (int i=0;i<__indent_;++i)putchar(' ');printf("< %s: %s, ", __func__, awful);stack_fprint(stdout, *r_nice); putchar('\n'); fflush(stdout);
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
    stack_t nice = stack_next(*r_nice);
    char *awful = NULL;
    if (nice_next(nice) != ']') {
        // [e1,...,en] -> PUSH e1 PUSH e2 ... PUSH en NIL
        for (;;) {
            awful = str_cat(awful, " PUSH "); // 1st space important!
            awful = str_cat(awful, nice_expression(&nice));
            if (nice_next(nice) == ']')
                break;
            nice = nice_expect(nice, ',');
        }
    }
    awful = str_cat(awful, " NIL");
    nice = stack_next(nice);    // skip ']'
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a function from *r_nice into a string which is returned;
    the value pointer by r_nice is updated. */
static char *nice_fun(stack_t *r_nice)
{
    // Transform "x1 ... xn: e" into {x1 ... xn:e}
    stack_t nice = *r_nice;
ENTER
    char *awful = str_new("{", 1);
    for (;;) {
        except_on(nice_next(nice) != ATOM,
            "Atom expected as function formal parameter");
        awful = str_cat(awful, nice->val.val.t);    // xi
        nice = stack_next(nice);
        if (nice_next(nice) == ':')
            break;
        awful = str_cat(awful, " ");
    }
    nice = stack_next(nice);    // skip ':'
    awful = str_cat(awful, ":");
    awful = str_cat(awful, nice_expression(&nice)); // e
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
        nice = stack_next(nice);    // skip '('
        // Inserts a "(" on the left of awful
        char *body = str_new("(", 1);
        body = str_cat(body, awful);
        body = str_cat(body, " ");
        for (;;) {
            body = str_cat(body, nice_expression(&nice));
            if (nice_next(nice) == ')') {
                nice = stack_next(nice);    // skip ')'
                body = str_cat(body, ")");
                break;
            } else {
                nice = nice_expect(nice, ',');
                body = str_cat(body, ",");
            }
        }
        awful = body;
    }
    *r_nice = nice;
EXIT
    return awful;
}

/** Parse a term from *r_nice into a string which is returned;
    the value pointer by r_nice is updated. */
static char *nice_term(stack_t *r_nice)
{
    static char buf[128];
    char *awful = NULL;
ENTER
    stack_t nice = *r_nice;
    except_on(nice == NULL, "Term expected");
    switch (nice->val.type) {
        case NUMBER: {
            sprintf(buf, "%g", nice->val.val.n);
            awful = str_new(buf, strlen(buf));
            nice = stack_next(nice);
            break;
        }
        case STRING: {
            buf[0] = (strchr(nice->val.val.t, '"') == NULL) ? '"' : '\'';
            buf[1] = '\0';
            awful = str_new(buf, 1);
            awful = str_cat(awful, nice->val.val.t);
            awful = str_cat(awful, buf);
            nice = stack_next(nice);
            break;
        }
        case ATOM: {
            awful = nice->val.val.t;
            nice = stack_next(nice);
            break;
        }
        case '[': {
            awful = nice_list(&nice);
            break;
        case '(': {
            nice = stack_next(nice);
            awful = nice_expression(&nice);
            nice = nice_expect(nice, ')');
            break;
        }
        default:
            except_on(nice->val.type != KEYWORD, "Syntax error");
            void *p = nice->val.val.p;
            nice = stack_next(nice);
            if (p == MINUS) {
                awful = str_new("SUB 0 ", 6);
                awful = str_cat(awful, nice_term(&nice));
            } else
            if (p == FIRST) {
                awful = str_new("TOS ", 4);
                awful = str_cat(awful, nice_term(&nice));
            } else
            if (p == REST) {
                awful = str_new("BOS ", 4);
                awful = str_cat(awful, nice_term(&nice));
            } else
            if (p == EMPTY) {
                awful = str_new("ISNIL ", 6);
                awful = str_cat(awful, nice_term(&nice));
            } else
            if (p == NNIL) {
                awful = str_new("NIL", 3);
            } else
            if (p == FUN) {
                awful = nice_fun(&nice);
            } else
                except_on(1, "Unary operator required");
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
    char *awful;
    char *e = nice_term(&nice);
    if (nice == NULL || nice->val.val.p != HAT) awful = e;
    else {
        nice = stack_next(nice);    // skip the operator
        awful = str_new("POW ", 4);
        awful = str_cat(awful, e);
        awful = str_cat(awful, " ");
        awful = str_cat(awful, nice_term(&nice));
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
    char *awful;
    char *e = nice_power(&nice);
    if (nice_next(nice) == ':') {
        nice = stack_next(nice);    // skip the operator
        awful = str_new("PUSH ", 5);
        awful = str_cat(awful, e);
        awful = str_cat(awful, " ");
        awful = str_cat(awful, nice_product(&nice));
    } else {
        int is_key = nice_next(nice) == KEYWORD;
        char *opt =
            (is_key && nice->val.val.p == TIMES) ? "MUL " :
            (is_key && nice->val.val.p == SLASH) ? "DIV " : NULL;
        if (opt == NULL) awful = e;
        else {
            nice = stack_next(nice);    // skip the operator
            awful = str_new(opt, strlen(opt));
            awful = str_cat(awful, e);
            awful = str_cat(awful, " ");
            awful = str_cat(awful, nice_product(&nice));
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
    char *awful;
    char *e = nice_product(&nice);
    int is_key = nice_next(nice) == KEYWORD;
    char *opt =
        (is_key && nice->val.val.p == PLUS) ? "ADD " :
        (is_key && nice->val.val.p == MINUS) ? "SUB " : NULL;
    if (opt == NULL) awful = e;
    else {
        nice = stack_next(nice);    // skip the operator
        awful = str_new(opt, strlen(opt));
        awful = str_cat(awful, e);
        awful = str_cat(awful, " ");
        awful = str_cat(awful, nice_sum(&nice));
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
    char *awful;
    if (nice_more(nice).val.p == NOT) {
        awful = str_new("EQ 0 ", 5);
        awful = str_cat(awful, nice_relation(&nice));
    } else {
        char *e = nice_sum(&nice);
        int is_key = nice_next(nice) == KEYWORD;
        char *opt =
            (nice_next(nice) == ATOM && strcmp(nice->val.val.t, "=") == 0
                || is_key && nice->val.val.p == EQ) ? "EQ ":
            (is_key && nice->val.val.p == NE) ? "NE ":
            (is_key && nice->val.val.p == LT) ? "LT ":
            (is_key && nice->val.val.p == LE) ? "LE ":
            (is_key && nice->val.val.p == GT) ? "GT ":
            (is_key && nice->val.val.p == GE) ? "GE ": NULL;
        if (opt == NULL) awful = e;
        else {
            nice = stack_next(nice);    // skip the operator
            awful = str_new(opt, strlen(opt));
            awful = str_cat(awful, e);
            awful = str_cat(awful, " ");
            awful = str_cat(awful, nice_sum(&nice));
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
    char *awful;
    char *e = nice_relation(&nice);
    int is_key = nice_next(nice) == KEYWORD;
    char *opt =
        (is_key && nice->val.val.p == OR) ? "MAX " :
        (is_key && nice->val.val.p == AND) ? "MIN " : NULL;
    if (opt == NULL) awful = e;
    else {
        nice = stack_next(nice);    // skip the operator
        awful = str_new(opt, strlen(opt));
        awful = str_cat(awful, e);
        awful = str_cat(awful, " ");
        awful = str_cat(awful, nice_proposition(&nice));
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
    char *awful;
    if (nice_more(nice).type != KEYWORD || nice->val.val.p != IF)
        awful = nice_proposition(&nice);
    else {
        // Transform "IF e1 e2 e3" into (COND e1{:e2}{:e3})
        nice = stack_next(nice);    // skip IF
        awful = str_new("(COND ", 6);
        awful = str_cat(awful, nice_proposition(&nice));  // e1
        nice = nice_expect_key(nice, "then");
        awful = str_cat(awful, "{:");
        awful = str_cat(awful, nice_expression(&nice));  // e2
        awful = str_cat(awful, "}{:");
        nice = nice_expect_key(nice, "else");
        awful = str_cat(awful, nice_expression(&nice));  // e3
        awful = str_cat(awful, "})");
    }
    *r_nice = nice;
EXIT
    return awful;
}

/** Transform "let x1 = v1, ..., xn = vn in e" into
        ({x1 ... xn:e} v1, ..., vn)
    and "letrec x1=v1, ..., xn = vn" into
        ({!x1 ... !xn:e} v1, ..., vn)
    Assume that the "let" or "letrec" keyword has NOT
    been parsed, (rec == 1 in case of "letrec").
*/
static char *nice_let(stack_t *r_nice, int rec)
{
ENTER
    stack_t nice = stack_next(*r_nice); // skip let/letrec
    
    /*  POSSIBILE OTTIMIZZAZIONE: DEFINISCI UN TIPO DI DATO
        buf CHE CONTIENE STRINGHE TEMPORANEE: LO USI PER LE
        STRINGHE awful COSI' NON HAI BISOGNO DI ALLOCARE
        SPAZIO NELLA TAVOLA DELLE STRINGHE.
    
        ALLOCHI UN BUFFER CON buf_new(s) DOVE s E' UNA
        STRINGA C. CON LA FUNZIONE
    
            s = buf_add(s, t)
        
        SI CONCATENA t IN FONDO A s.
    
        QUANDO SI È FINITO, SI USA buf_del(s) PER
        CANCELLARE LA STRINGA.
    */
    
    char *awful = str_new("({", 2);
    char *values = NULL;    // actual parameters list
    for (;;) {
        except_on(nice_next(nice) != ATOM,
            "Variables in let/letrec should be atoms");
        if (rec) awful = str_cat(awful, "!");
        awful = str_cat(awful, nice->val.val.t);   // variable xi
        nice = nice_expect(stack_next(nice), '=');
        values = str_cat(values, nice_conditional(&nice));
        if (nice_more(nice).val.p == IN) {
            nice = stack_next(nice);
            break;
        }
        nice = nice_expect(nice, ',');
        values = str_cat(values, ",");
        awful = str_cat(awful, " ");
    }
    /*  Here all pairs xi = vi are parsed, the list of formal
        parameters x1 ... xn dumped on awful and the list of
        actual parameters dumped on value: we close it by ')' */
    //values = str_cat(values, ")");
    awful = str_cat(awful, ":");
    awful = str_cat(awful, nice_expression(&nice));    // body
    awful = str_cat(awful, "}");
    awful = str_cat(awful, values);         // actual parameters
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
    int is_key = (nice_next(nice) == KEYWORD);
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
RESET
    int err = 1;
    if (setjmp(except_buf) == 0) {
        stack_t tokens = scan(text, DELIMITERS, nice_key_find);
        /* If the text starts with "awful" then the user is asking
            not to evaluate it but to translate it into awful. */
        int translate = tokens != NULL && tokens->val.type == ATOM
        && strcmp(tokens->val.val.t, "awful") == 0;
        if (translate) tokens = tokens->next;   // skip "awful"
        char *t = nice_expression(&tokens);
        if (tokens != NULL) {
            fprintf(stderr, "Warning: text after expression shall be ignored:");
            while (tokens != NULL) {
                fputc(' ', stderr);
                val_fprint(stderr, tokens->val);
                tokens = stack_next(tokens); }
            fputc('\n', stderr);
        }
        err = (t == NULL);
        if (!err) {
            if (translate) fprintf(file, "%s\n", t);
            else err = awful(t, file);
        }
    }
    return err;
}
