/** \file str.c */

#include <stdlib.h>
#include <string.h>
#include "../header/except.h"
#include "../header/stack.h"
#include "../header/str.h"

/**
    Strings are stored into a hash table whose elements
    are stack items: each item of the stack contains
    a string whose hash encoding is the same.
    
    When a string is created, it is inserted or retrieved
    from the table.
*/

#define TABSIZE (1024)  /* Need to be a power of 2 */

typedef struct str_table_s {
    struct str_table_s *next;
    char *s;    // immutable string
    unsigned l; // its length
} *str_table_t;

/** The string table is an array indexed by string hashes:
    each item points to a str_table_t with the list of all
    strings with that same hash. */
static str_table_t str_table[TABSIZE] = {0};

/** Allocate a new item in the h-th element of the string
    table and return its address. */
static str_table_t str_table_new(unsigned h)
{
    str_table_t item = malloc(sizeof(struct str_table_s));
    except_on(item == NULL, "Cannot allocate string %s:%i",
        __FILE__, __LINE__);
    // Push item in front of the list str_table[h]
    item->next = str_table[h];
    return str_table[h] = item;
}

/** Simple minded hash function. The h parameter is used to
    compute the hash of a concatenation of strings s1 + s2:
    call str_hash(str_hash(0,s1,strlen(s1)),s2, strlen(s2). */
static unsigned str_hash(unsigned h, const char *s, size_t n)
{
    while (n > 0) {
        h += ((unsigned)*s++) * 257;
        h &= TABSIZE - 1;
        -- n;
    }
    return h;
}

char *str_cat(const char *s1, const char *s2)
{
    size_t l1 = (s1 == NULL) ? -1 : strlen(s1);
    size_t l2 = (s2 == NULL) ? -1 : strlen(s2);
    except_on(l1 == -1 && l2 == -1, "BUG: %s:%n", __FILE__, __LINE__);
    if (l1 == -1) return str_new(s2, l2);
    if (l2 == -1) return str_new(s1, l1);
    size_t l3 = l1 + l2;
    // Looks for s1 + s2 inside the table
    unsigned h = str_hash(str_hash(0, s1, l1), s2, l2);
    for (str_table_t p = str_table[h]; p != NULL; p = p->next)
        // Compare character-wise including the final '\0'.
        if (p->l == l3 && memcmp(p->s, s1, l1) == 0
        && memcmp(p->s + l1, s2, l2) == 0)
            return p->s;
    // The string is new: allocate it.
    char *s3 = malloc(l3 + 1);
    except_on(s3 == NULL, "Cannot allocate string %s:%i",
        __FILE__, __LINE__);
    str_table_t item = str_table_new(h);
    item->l = l3;
    return item->s = strcat(strcpy(s3, s1), s2);
}

char *str_new(const char *s, size_t n)
{
    /*  Insert a string into the table and return its address:
        if the string already is in, retrieves its address.
        If n = 0 don't allocate the string but use s. */
    unsigned h = str_hash(0, s, n);
    for (str_table_t p = str_table[h]; p != NULL; p = p->next)
        // Compare character-wise including the final '\0'.
        if (p->l == n && memcmp(p->s, s, n) == 0)
            return p->s;
    str_table_t item = str_table_new(h);
    item->s = malloc(n + 1);
    item->l = n;
    except_on(item->s == NULL, "Cannot allocate string %s:%i",
        __FILE__, __LINE__);
    item->s[n] = '\0';
    return memcpy(item->s, s, n);
}

void str_reset(void)
{
    // Free all strings in the table
    for (unsigned h = 0; h < TABSIZE; ++ h) {
        str_table_t p = str_table[h];
        while (p != NULL) {
            str_table_t next = p->next;
            free(p->s);
            free(p);
            p = next;
        }
        str_table[h] = NULL;
    }
}

void str_status(FILE *dump)
{
    int n = 0;
    unsigned size = 0;
    for (int i = 0; i < TABSIZE; ++ i)
        for (str_table_t p = str_table[i]; p != NULL; p = p->next) {
            ++ n;
            size += strlen(p->s);
        }
    fprintf(dump, "%i strings (%u Kbytes)\n", n, size / 1024);
}

/** Return the address of the substring of s which is stripped
    by spaces, both on the left and on the right: on the left
    the effect is achieved returing a pointer on the first non
    space character, on the right by setting to '\0' the leftmost
    space from the end of the string. */
char *str_strip(char *s)
{
    s += strspn(s, " \t\n\r");    // skip spaces
    size_t len = strlen(s);
    while (--len > 0 && strchr(" \t\n\r", s[len]))
        ;
    s[len + 1] = '\0';
    return s;
}
