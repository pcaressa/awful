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

typedef struct str_list_s {
    struct str_list_s *next;
    char *s;    // immutable string
} *str_list_t;

/** The string table is an array indexed by string hashes:
    each item points to a str_list_t with the list of all
    strings with that same hash. */
static str_list_t str_table[TABSIZE] = {0};

/** Simple minded hash function. **/
static unsigned str_hash(const char *s, size_t n)
{
    unsigned h = 0;
    while (n > 0) {
        h += ((unsigned)*s++) * 257;
        h &= TABSIZE - 1;
        -- n;
    }
    return h;
}

char *str_cat(char *s1, const char *s2)
{
    if (s1 == NULL) return s2;
    if (s2 == NULL) return s1;
    /* Create the concatenated string and next invoke
        str_new on it: we cannot extend s1 since
        it would change its hash! */
    char *s3 = malloc(strlen(s1) + strlen(s2) + 1);
    except_on(s3 == NULL, "Cannot allocate string");
    return str_new(strcat(strcpy(s3, s1), s2), 0);
}

char *str_new(const char *s, size_t n)
{
    /*  Insert a string into the table and return its address:
        if the string already is in, retrieves its address.
        If n = 0 don't allocate the string but use s. */
    str_list_t item;
    unsigned h;
    if (n == 0) {
        n = strlen(s);
        h = str_hash(s, n);
        for (str_list_t p = str_table[h]; p != NULL; p = p->next)
            // Compare character-wise including the final '\0'.
            if (strcmp(p->s, s) == 0)
                return p->s;
        // The string is not in the stack: create and insert it
        item = malloc(sizeof(struct str_list_s));
        except_on(item == NULL, "Cannot allocate string");
        item->s = s;
    } else {
        h = str_hash(s, n);
        for (str_list_t p = str_table[h]; p != NULL; p = p->next)
            // Compare character-wise including the final '\0'.
            if (memcmp(p->s, s, n) == 0 && p->s[n] == '\0')
                return p->s;
        // The string is not in the stack: create and insert it
        item = malloc(sizeof(struct str_list_s));
        char *t = malloc(n + 1);
        except_on(item == NULL || t == NULL, "Cannot allocate string");
        memcpy(t, s, n);
        t[n] = 0;
        item->s = t;
    }
    // Push item in front of the list str_table[h]
    item->next = str_table[h];
    str_table[h] = item;
    return item->s;
}

void str_reset(void)
{
    // Free all strings in the table
    for (unsigned h = 0; h < TABSIZE; ++ h) {
        str_list_t p = str_table[h];
        while (p != NULL) {
            str_list_t next = p->next;
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
        for (str_list_t p = str_table[i]; p != NULL; p = p->next) {
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
