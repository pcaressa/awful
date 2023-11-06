/** \file scan.h */

#ifndef scan_INC
#define scan_INC

#include "stack.h"

/** Scan a list of tokens from text, using any character
    in delimiters as delimiter; the key_find function should
    return the pointer to the subroutine corresponding to a
    keyword if the text matches with the keyword name, else
    NULL. It is void* but when needed it is casted to a
    function pointer of the form val_t (*)(stack_t*, stack_t).

    Scan returns a tokens list, thus a stack containing the
    list of tokens where the top of the stack contains the
    first token, etc.
    
    Each item is an Awful value as follows:
    
    - {type:NUMBER, val:n}
    - {type:STRING, val:t}
    - {type:ATOM, val:t}
    - {type:STACK, val:s}
    - {type:CLOSURE, val:s}
    - {type:KEYWORD, val:p}
    - {type:d} if d is a delimiter (val is ignored)
*/
stack_t scan(char *text, char *delims, void *key_find(char *text));

#endif
