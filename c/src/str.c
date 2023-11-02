/** \file str.c */

#include <assert.h>
#include <stdlib.h>
#include "../header/stack.h"
#include "../header/str.h"

/**
    Strings are stored into a hash table whose elements
    are stacks: each item of the stack contains a string
    whose hash encoding is the same.
    
    When a string is created it is inserted or retrieved
    from the table.
*/

#define TABSIZE (1024)

static stack_t str_table[TABSIZE] = {0};

/** Simple minded hash function. **/
static unsigned hash(char *s, int n)
{
    unsigned h = *s;
    while (--n > 0) {
        h += (*++s) * 257;
        h &= TABSIZE - 1;
    }
    return h;
}

char *str_new(char *s, int n)
{
    /*  Insert a string into the table and return its
        address: if the string already is in, retrieves
        its address. */
    unsigned h = hash(s, n);
    if (str_table[h] != NULL)
        for (stack_t p = str_table[h]; p != NULL; p = p->next)
            // Compare character-wise including the final '\0'.
            if (memcmp(p->val.t, s, n) == 0 && p->val.t[n] == '\0')
                return p->val.t;
    // The string is not in the stack: create and insert it
    char *t = malloc(n + 1);
    assert(t != NULL);
    memcpy(t, s, n);
    t[n] = '\0';
    str_table[h] = stack_push(str_table[h], STRING, t);
    return t;
}

void str_status(void)
{
    int n = 0;
    for (int i = 0; i < TABSIZE; ++ i)
        for (stack_t s = str_table[i]; s != NULL; s = s->next)
            ++ n;
    fprintf(stderr, "%i strings allocated\n", n);
}

