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
    assert(text != NULL);
    stack_t tokens = NULL;
    while (*text != '\0') {
        text += strspn(text, " \t\n\r");
        if (*text == '\0') break;
        if (strchr(delimiters, *text) != NULL) {
            //tokens = stack_push(tokens, DELIMITER, *text);
            tokens = stack_push(tokens, *text, *text);
            ++ text;
        } else
        if (*text == '\\') {    // Comment
            break;
        } else
        if (*text == '\'' || *text == '"') {
            char q = *text;
            char *p = strchr(text + 1, q);
            except_on(p == NULL, "End of text inside string");
            char *t = str_new(text + 1, p - text - 1);
            tokens = stack_push(tokens, STRING, t);
            text = p + 1;
        } else {
            // Scans up to the following space or delimiter.
            char *p = text++;
            while (*text != '\0' && !isspace(*text)
            && strchr(delimiters, *text) == NULL)
                ++ text;
            // The atom starts at p and its length is text - p.
            // Check against a number.
            char *q;
            double n = strtod(p, &q);
            if (q == text) {
                tokens = stack_push(tokens, NUMBER, n);
            } else {
                char *t = str_new(p, text - p);

                // Check against a keyword.
                stack_t k = keywords;
                while (k != NULL) {
                    // k is a stack of consecutive pairs name->pointer
                    assert(k->type == ATOM);
                    if (strcmp(t, k->val.t) == 0) {
                        tokens = stack_push(tokens, KEYWORD, k->next->val.p);
                        break;
                    }
                    k = k->next->next;
                }
                if (k == NULL)  // Atom (variable!)
                    tokens = stack_push(tokens, ATOM, t);
            }
        }
    }
    return tokens;
}
