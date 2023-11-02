/** \file repl.c */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../header/repl.h"

int repl(int (*interpret)(char*, FILE*), char *prompt)
{
    static char buf[BUFSIZ];
    static char tib[128];
    
    *buf = '\0';
    fprintf(stderr, "\n%s: ", prompt);
    
    FILE *in = fopen("../in.awf", "r");
    assert(in != NULL);
    
    while (fgets(tib, sizeof(tib), in) != NULL) {
fputs(tib, stderr);
        char *pn = strchr(tib, '\n');
        if (pn != NULL && pn[-1] == '\\') {
            // Join the line to buf.
            strcat(buf, tib);
            fprintf(stderr, "\n%s| ", prompt);
            continue;
        }
        if (*buf == '\0') strcpy(buf, tib);
        // Now buf contains the text to interpret
        interpret(buf, stderr);
        fprintf(stderr, "\n%s: ", prompt);
        *buf = '\0';
    }
    return 0;
}
