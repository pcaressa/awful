/** \file repl.c */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../header/repl.h"

<<<<<<< HEAD
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
=======
/** Safe version of strcat: if the concatenated text from
    buf and line exceeds size an error is raised on file out
    and 1 is returned, else 0 is returned. */
static int strcat_safe(char *buf, unsigned size, char *line, FILE *out)
{
    if (strlen(buf) + strlen(line) >= size) {
        fprintf(out,
            "Text line too long: max %i characters allowed!\n",
            size);
        return 1;
    }
    strcat(buf, line);
    return 0;
}

/** Scan from file in a line of text (possibly asking for more
    if the line ends with a backslash), apply to the result the
    interpret function and print on the out file the result.
    If prompt is not NULL it is printed on out before reading
    a line from in.
    On error NULL is returned, else a hidden text with the line
    just interpreted. */
static char *rep(FILE *in, FILE *out, int (*interpret)(char*, FILE*), char *prompt)
{
    static char buf[BUFSIZ];
    static char line[128];
    static int line_count = 1;
    
    *buf = '\0';
    if (prompt != NULL)
        fprintf(out, "\n%s %i: ", prompt, line_count);
    if (fgets(line, sizeof(line), in) == NULL)
        return NULL;
    ++ line_count;
    // Manage multiple lines
    char *p;
    while ((p = strchr(line, '\\')) != NULL) {
        p[0] = ' ';
        p[1] = '\0';
        if (strcat_safe(buf, sizeof buf, line, out))
            return NULL;
        if (prompt != NULL)
            fprintf(out, "%s %i| ", prompt, line_count);
        fgets(line, sizeof(line), in);
        ++ line_count;
    }
    if (strcat_safe(buf, sizeof buf, line, out))
        return NULL;
    char *text = buf + strspn(buf, " \t\n\r");  // skip spaces
    if ((p = strrchr(text, '\n')) != NULL)
        *p = '\0';  // strip ending newline
    if (strcmp(text, "bye") == 0)
        return NULL;
    if (memcmp(text, "batch ", 6) == 0) {
        text += 6;  // skip "batch "
        text += strspn(text, " \t\n\r");    // skip spaces
        if ((p = strrchr(text, '\n')) != NULL)
            *p = '\0';  // strip '\n'
        FILE *f = fopen(text, "r");
        if (f == NULL) perror(text);
        else {
            int saved = line_count;
            line_count = 0;
            while (rep(f, out, interpret, NULL))
                ;
            fclose(f);
            line_count = saved;
        }
    } else {
        if (*text != '\0' && interpret(text, out))
            fprintf(out, " line %i\n", line_count);
    }
    return buf;
}

int repl(FILE *in, FILE *out, int (*interpret)(char*, FILE*), char *prompt)
{
    fputs("\nType: 'bye' to leave, 'batch FILENAME' to process a file"
        "\nLines starting with backslash or empty are skipped"
        "\nA line ending with backslash is joined to the following one\n",
        out);
    while (rep(in, out, interpret, prompt))
        ;
    fputs("Goodbye\n", out);
>>>>>>> 8a8839ebbf159d15191d30c8f61f114de2d17dd7
    return 0;
}
