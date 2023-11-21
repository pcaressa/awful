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

/** The string table is an array indexed by string hashes:
    each item contains a stack with the list of all strings
    with that same hash. */
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

void str_del(char *s)
{
    // SHOULD IMPLEMENT A FREELIST FOR STRINGS:
    // each item contains the size of the allocated string
    // and the actual string: when the latter is freed,
    // the area is put into the free list ordered by length
    // so that str_new, before mallocating, could check if
    // there's an already allocated available string.
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
            if (memcmp(p->val.val.t, s, n) == 0 && p->val.val.t[n] == '\0')
                return p->val.val.t;
    // The string is not in the stack: create and insert it
    char *t = malloc(n + 1);
    assert(t != NULL);
    memcpy(t, s, n);
    t[n] = '\0';
    val_t v = {.type = STRING, .val.t = t};
    str_table[h] = stack_push(str_table[h], v);
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
