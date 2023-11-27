/** \file repl.c */

//#define AWFUL

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "../header/awful.h"
#include "../header/nice.h"
#include "../header/repl.h"

/** Current file line counter. */
static int repl_line = 0;

// Forward declaration
static char *repl_eval(FILE *in, FILE *out,
    int (*eval)(char*, FILE*), char *prompt);

/** Apply the eval evaluator to the lines of a text file
    whose name is filename. File out is needed by eval. */
static void repl_batch(char *filename, int (*eval)(char*, FILE*), FILE *out)
{
    filename += strspn(filename, " \t\n\r");    // skip spacesù
    char *p = strrchr(filename, '\n');
    if (p != NULL) *p = '\0'; // strip ending '\n'
    
    FILE *f = fopen(filename, "r");
    if (f == NULL) perror(filename);
    else {
        int saved = repl_line;
        repl_line = 0;
        while (repl_eval(f, out, eval, NULL))
            ;
        fclose(f);
        repl_line = saved;
    }
}

/** Scan from file in a line of text (possibly asking for more
    if the line ends with a backslash), apply to the result the
    interpret function and print on the out file the result.
    If prompt is not NULL it is printed on out before reading
    a line from in.
    On error NULL is returned, else a hidden text with the line
    just interpreted. */
static char *repl_eval(FILE *in, FILE *out,
                       int (*eval)(char*, FILE*), char *prompt)
{
    static char buf[BUFSIZ];

    *buf = '\0';
    ++ repl_line;
    if (prompt) fprintf(out, "\n%s %i: ", prompt, repl_line);
    if (fgets(buf, sizeof(buf), in) == NULL) return NULL;

    char *p;
    while ((p = strchr(buf, '\\')) != NULL) {
        // Strip spaces on the right
        while (p > buf && isspace(p[-1]))
            -- p;
        *p++ = ' ';   // transforms the backslash into a space
        ++ repl_line;
        if (prompt) fprintf(out, "\n%s %i| ", prompt, repl_line);
        if ((p = fgets(p, sizeof(buf) - (p - buf), in)) == NULL)
            return NULL;
    }
    // Strip initial spaces and ending newline
    char *text = buf + strspn(buf, " \t\n\r");
    if ((p = strrchr(text, '\n')) != NULL) *p = '\0';

    if (strcmp(text, "bye") == 0) return NULL;
    if (memcmp(text, "batch ", 6) == 0) {
        repl_batch(text + 6, eval, out);
    } else {
        if (*text != '\0' && eval(text, out))
           fprintf(out, ": line %i\n", repl_line);
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
    while (repl_eval(stdin, stdout, awful, "awful"))
#   else
    while (repl_eval(stdin, stdout, nice, "niceful"))
#   endif
        ;
    puts("Goodbye");
    return 0;
}
