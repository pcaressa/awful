/** scan.c */

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "../header/awful_key.h"
#include "../header/except.h"
#include "../header/scan.h"
#include "../header/stack.h"
#include "../header/str.h"
#include "../header/val.h"

stack_t scan(char *text, char *delims, void *key_find(char*,unsigned))
{
    val_t v;
    stack_t tokens = NULL;
    while (*text != '\0') {
        text += strspn(text, " \t\n\r");    // skip spaces
        if (*text == '\0') break;
        if (*text == '\\') {    // Skip until the end of the line
            if ((text = strchr(text + 1, '\n')) == NULL) break;
            continue;
        }
        if (strchr(delims, *text) != NULL) {
            v.type = *text;
            tokens = stack_push(tokens, v);
            ++ text;
        } else
        if (*text == '\'' || *text == '"') {
            char q = *text;
            char *p = strchr(text + 1, q);
            except_on(p == NULL, "End of text inside string");
            v.type = STRING;
            v.val.t = str_new(text + 1, p - text - 1);
            tokens = stack_push(tokens, v);
            text = p + 1;
        } else {
            // Scans up to the following space or delimiter.
            char *p = text++;
            while (*text != '\0' && !isspace(*text)
            && strchr(delims, *text) == NULL)
                ++ text;
            // The atom starts at p and its length is text - p.
            // Check against a number.
            char *q;
            v.val.n = strtod(p, &q);
            if (q == text) {
                v.type = NUMBER;
            } else {
                void *k = key_find(p, text - p);
                if (k != NULL) {
                    v.type = KEYWORD;
                    v.val.p = k;
                } else {
                    v.type = ATOM;
                    v.val.t = str_new(p, text - p);
                }
            }
            tokens = stack_push(tokens, v);
        }
    }
    return stack_reverse(tokens);
}
