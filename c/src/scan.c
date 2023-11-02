/** scan.c */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "../header/except.h"
#include "../header/scan.h"
#include "../header/stack.h"
#include "../header/str.h"
#include "../header/val.h"

stack_t scan(char *text, char *delimiters, stack_t keywords)
{
    val_t v;
    assert(text != NULL);
    stack_t tokens = NULL;
    while (*text != '\0') {
        text += strspn(text, " \t\n\r");
        if (*text == '\0') break;
        if (strchr(delimiters, *text) != NULL) {
            //tokens = stack_push(tokens, DELIMITER, *text);
            v.type = *text;
            tokens = stack_push(tokens, v);
            ++ text; }
        else
        if (*text == '\\') break; // Comment: skip it!
        else
        if (*text == '\'' || *text == '"') {
            char q = *text;
            char *p = strchr(text + 1, q);
            except_on(p == NULL, "End of text inside string");
            v.type = STRING;
            v.val.t = str_new(text + 1, p - text - 1);
            tokens = stack_push(tokens, v);
            text = p + 1; }
        else {
            // Scans up to the following space or delimiter.
            char *p = text++;
            while (*text != '\0' && !isspace(*text)
            && strchr(delimiters, *text) == NULL)
                ++ text;
            // The atom starts at p and its length is text - p.
            // Check against a number.
            char *q;
            v.val.n = strtod(p, &q);
            if (q == text) {
                v.type = NUMBER;
                tokens = stack_push(tokens, v); }
            else {
                char *t = str_new(p, text - p);

                // Check against a keyword.
                stack_t k = keywords;
                while (k != NULL) {
                    // k is a stack of consecutive pairs name->pointer
                    assert(k->val.type == ATOM);
                    if (strcmp(t, k->val.val.t) == 0) {
                        v.type = KEYWORD;
                        v.val.p = k->next->val.val.p;
                        tokens = stack_push(tokens, v);
                        break; }
                    k = k->next->next; }
                if (k == NULL) {
                    // Atom (variable!)
                    v.type = ATOM;
                    v.val.t = t;
                    tokens = stack_push(tokens, v); }}}}
    return tokens;
}
