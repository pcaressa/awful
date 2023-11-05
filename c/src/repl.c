/** \file repl.c */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../header/repl.h"

int rep(FILE *in, int (*interpret)(char*, FILE*), char *prompt)
{
    static char buf[BUFSIZ];
    static char tib[128];
    
    *buf = '\0';
    fprintf(stderr, "\n%s: ", prompt);
Read:
    if (fgets(tib, sizeof(tib), in) == NULL) return EOF;
    if (in != stdin) fputs(tib, stderr);
    if (*tib != '\n') {
        char *pn = strchr(tib, '\n');
        if (pn != NULL && pn[-1] == '\\') {
            // Join the line to buf.
            strcat(buf, tib);
            fprintf(stderr, "\n%s| ", prompt);
            goto Read;
        }
        if (*buf == '\0') strcpy(buf, tib);
        // Now buf contains the text to interpret
        interpret(buf, stderr);
        *buf = '\0';
        fprintf(stderr, "\n%s: ", prompt);
stack_status();
    }
    return 0;
}


int repl(FILE *in, int (*interpret)(char*, FILE*), char *prompt)
{
    static char buf[BUFSIZ];
    static char tib[128];
    
    *buf = '\0';
    fprintf(stderr, "\n%s: ", prompt);
    for (;;) {
        if (fgets(tib, sizeof(tib), in) == NULL) break;
        if (in != stdin) fputs(tib, stderr);
        if (*tib != '\n') {
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
            *buf = '\0';
            fprintf(stderr, "\n%s: ", prompt);
stack_status();
    }}
    return 0;
}
//~ int repl(int (*interpret)(char*, FILE*), char *prompt)
//~ {
    //~ static char buf[BUFSIZ];
    //~ static char tib[128];
    
    //~ *buf = '\0';
    //~ fprintf(stderr, "\n%s: ", prompt);
    
    //~ FILE *in = fopen("../in.awf", "r");
    //~ assert(in != NULL);
    
    //~ while (fgets(tib, sizeof(tib), in) != NULL) {
//~ fputs(tib, stderr);
        //~ char *pn = strchr(tib, '\n');
        //~ if (pn != NULL && pn[-1] == '\\') {
            //~ // Join the line to buf.
            //~ strcat(buf, tib);
            //~ fprintf(stderr, "\n%s| ", prompt);
            //~ continue;
        //~ }
        //~ if (*buf == '\0') strcpy(buf, tib);
        //~ // Now buf contains the text to interpret

        //~ interpret(buf, stderr);
        //~ fprintf(stderr, "\n%s: ", prompt);
        //~ *buf = '\0';
//~ stack_status();
    //~ }
    //~ return 0;
//~ }
