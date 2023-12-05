/** \file repl.c */

#define VERSION "0.2312"

#undef NDEBUG
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header/awful.h"
#include "../header/nice.h"
#include "../header/str.h"

#define repl_BUFSIZ (65536)

/** Buffer used to join lines ending with '\\' */
static char repl_buf[repl_BUFSIZ];

/** File where output is printed. */
static FILE *repl_out = NULL;

/** Current file line counter. */
static int repl_line = 0;

/** Current evaluation function: awful or niceful. */
static int (*repl_eval)(char*, FILE*) = nice;

/** Gets a line from a file. If prompt != NULL then it is
    printed on repl_out: the scanned line is stored at repl_buf
    if cat == 0, else it is appended to the string already
    in repl_buf. In any case, the address of the last inserted
    line is returned; if an error, or the end of the file,
    occurs, NULL is returned. */
static char *repl_get(FILE *in, const char *prompt, int cat)
{
    char *p;
    char c = ':';       // becomes '|' in multiple lines
    if (cat == 0) repl_buf[0] = '\0';
    // Set r to the address where to store the line to scan
    char *r = repl_buf + strlen(repl_buf);
    for (;;) {
        if (prompt)
            printf("%s %i%c ", prompt, repl_line, c);
        r = fgets(r, repl_BUFSIZ - (r - repl_buf), in);
        if (r == NULL) return r;
        if (r - repl_buf >= repl_BUFSIZ - 2) {
            fputs("Line too long!\n", stderr);
            return r;
        }
        if ((p = strrchr(r, '\\')) == NULL) return r;
        // Strip spaces on the right
        while (p > r && isspace(p[-1]))
            -- p;
        *p = ' ';   // transforms the backslash into a space
        ++ repl_line;
        r = p + 1;
        c = '|';
    }
}

// Forward declaration
static void repl(FILE *in, char *prompt);

/** Apply the eval evaluator to the lines of a text file
    whose name is at s. */
static void repl_batch(char *s)
{
    s = str_strip(s);
    char *name = malloc(strlen(s) + 1);
    assert(name || !fputs("Malloc error (this is weird)", stderr));
    strcpy(name, s);
    
    FILE *f = fopen(name, "r");
    if (f == NULL) perror(name);
    else {
        int saved = repl_line;
        repl_line = 0;
        repl(f, name);
        fclose(f);
        repl_line = saved;
    }
    free(name);
}

/** Prints a help message. */
static void repl_help(void)
{
    fputs(
    "Interactive mode: type the expression to evaluate on a single\n"
    "   line: to continue the expression on another line end it by\n"
    "   a backslash. Anything after the backslash will be ignored\n"
    "   (so you can use them as comments).\n\n"
    "REPL commmands: type them instead of an expression, they are:\n"
    "   'awful': switch to Awful interpreter.\n"
    "   'batch FILENAME': the FILENAME text file is opened for\n"
    "      reading and each line of it is evaluated as a single\n"
    "      line typed in the interactive mode.\n"
    "   'bye' ends the session and closes the interpreter.\n"
    "   'help' prints this message.\n"
    "   'niceful': switch to Niceful interpreter.\n"
    "   'output': redirect output to terminal screen.\n"
    "   'output FILENAME': redirect output to file FILENAME (in"
    "      append mode).\n"
    "   'prelude FILENAME ...' the FILENAME text file is opened for\n"
    "      reading and its lines are joined in a single line to\n"
    "      which the next input line is appended: the resulting\n"
    "      string is evaluated.\n"
    "Warning: a preluded file cannot exceed 64Kbytes.\n" 
    , repl_out);
}

/** Read from s a file name which is opened for appending and
    assigned to the repl_out variable. If s is the empty string
    (after being stripped) then repl_out is set to stdout. */
static void repl_output(char *s)
{
    s = str_strip(s);
    if (*s == '\0') {
        if (repl_out != stdout) fclose(repl_out);
        repl_out = stdout;
    } else {
        FILE *f = fopen(s, "a");
        if (f == NULL) perror(s);
        else {
            if (repl_out != stdout) fclose(repl_out);
            repl_out = f;
        }
    }
}

/** Read the file whose name is at filename in repl_buf and
    append to it a line from the in file: next evaluate
    the result and print the result on repl_out. */
static void repl_prelude(char *filename, FILE *in, char *prompt)
{
    filename = str_strip(filename);
    char *name = malloc(strlen(filename) + 1);
    assert(name || !"Malloc error (this is weird)");
    strcpy(name, filename);
    
    FILE *f = fopen(name, "r");
    if (f == NULL) perror(name);
    else {
        unsigned len = fread(repl_buf, 1, repl_BUFSIZ, f);
        fclose(f);
        if (len >= repl_BUFSIZ - 2) {
            fprintf(stderr, "Prelude %s too long (max %u bytes)",
                name, repl_BUFSIZ);
        } else {
            repl_buf[len] = ' ';
            repl_buf[len + 1] = '\0';
            if (repl_get(in, prompt, 1) && *repl_buf != '\0'
            && repl_eval(repl_buf, repl_out))
                printf(": line %i\n", repl_line);
        }
    }
    free(name);
}

/** Scan from file in a line of text (possibly asking for more
    if the line ends with a backslash), apply to the result the
    interpret function and print on the out file the result.
    If prompt is not NULL it is printed on out before reading
    a line from in. */
static void repl(FILE *in, char *prompt)
{
    int n;
    char *p;
    for (repl_line = 1; repl_get(in, prompt, 0); ++ repl_line) {
        char *text = str_strip(repl_buf);
        if (strcmp(text, "awful") == 0) {
            fputs("Awful interpreter\n", stderr);
            prompt = "awful";
            repl_eval = awful;
        } else if (memcmp(text, "batch ", 6) == 0) {
            repl_batch(text + 6);
        } else if (strcmp(text, "bye") == 0) {
            break;
        } else if (strcmp(text, "help") == 0) {
            repl_help();
        } else if (strcmp(text, "niceful") == 0) {
            fputs("Niceful interpreter\n", stderr);
            prompt = "niceful";
            repl_eval = nice;
        } else if (memcmp(text, "output", 6) == 0) {
            repl_output(text + 6);
        } else if (memcmp(text, "prelude ", 8) == 0) {
            repl_prelude(text + 8, in, prompt);
        } else {
            if (*text != '\0' && repl_eval(text, repl_out))
               printf(": line %i\n", repl_line);
        }
    }
}

int main(int argc, char **argv)
{
    puts(
        "AWFUL - A Weird FUnctional Language\n"
        "(c) 2023 by Paolo Caressa <github.com/pcaressa/awful>\n"
        "[v." VERSION ". Type 'help' for... guess what?]\n"
    );
    repl_out = stdout;  // cannot initialize at global scope
    repl(stdin, "niceful");
    puts("Goodbye");
    return 0;
}
