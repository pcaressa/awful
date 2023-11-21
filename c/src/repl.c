/** \file repl.c */

//#define AWFUL

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../header/awful.h"
#include "../header/nice.h"
#include "../header/repl.h"

/** Scan from file in a line of text (possibly asking for more
    if the line ends with a backslash), apply to the result the
    interpret function and print on the out file the result.
    If prompt is not NULL it is printed on out before reading
    a line from in.
    On error NULL is returned, else a hidden text with the line
    just interpreted. */
static char *rep(FILE *in, FILE *out, int (*eval)(char*, FILE*), char *prompt)
{
    static char buf[BUFSIZ];
    static int line_count = 0;

    *buf = '\0';
    ++ line_count;
    if (prompt) fprintf(out, "\n%s %i: ", prompt, line_count);
    if (fgets(buf, sizeof(buf), in) == NULL) return NULL;

    char *p;
    while ((p = strchr(buf, '\\')) != NULL) {
        if (prompt) fprintf(out, "\n%s %i| ", prompt, line_count);
        if ((p = fgets(p, sizeof(buf) - (p - buf), in)) == NULL)
            return NULL;
    }
    // Strip initial spaces and ending newline
    char *text = buf + strspn(buf, " \t\n\r");
    if ((p = strrchr(text, '\n')) != NULL) *p = '\0';

    if (strcmp(text, "bye") == 0) return NULL;
    if (memcmp(text, "batch ", 6) == 0) {
        text += 6;  // skip "batch "
        text += strspn(text, " \t\n\r");        // skip spaces
        (p = strrchr(text, '\n')) && (*p = '\0'); // strip ending '\n'
        
        FILE *f = fopen(text, "r");
        if (f == NULL) perror(text);
        else {
            int saved = line_count;
            line_count = 0;
            while (rep(f, out, eval, NULL))
                ;
            fclose(f);
            line_count = saved;
        }
    } else {
        if (*text != '\0' && eval(text, out))
           fprintf(out, " @ line %i\n", line_count);
    }
//stack_status();
    return buf;
}

int main(int argc, char **argv)
{
    puts(
#   ifdef AWFUL
        "AWFUL - A Weird FUnctional Language\n"
#   else
        "NICEFUL - a NICE FUnctional Language\n"
#   endif
        "(c) 2023 by Paolo Caressa\n\n"
        "Type: 'bye' to leave, 'batch FILENAME' to process a file\n"
        "Lines starting with backslash or empty are skipped\n"
        "A line ending with backslash is joined to the following one\n"
    );
#   ifdef AWFUL
    while (rep(stdin, stdout, awful, "awful"))
#   else
    while (rep(stdin, stderr, nice, "niceful"))
#   endif
        ;
    fputs("Goodbye\n", stdout);
    return 0;
}
