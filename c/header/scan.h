/** \file scan.h */

#ifndef scan_INC
#define scan_INC

#include "stack.h"

/** Scan a list of tokens from text, using any character
    in delimiters as delimiter; the stack keywords is a
    stack of consecutive pairs name->value->... where
    values are void * pointers.
    A stack is returned, containing the list of tokens
    in reverse order, so that the last one is its top;
    each item is an Awful value as follows:
    
    - {type:NUMBER, val:n}
    - {type:STRING, val:t}
    - {type:ATOM, val:t}
    - {type:STACK, val:s}
    - {type:CLOSURE, val:s}
    - {type:KEYWORD, val:p}
    - {type:d} if d is a delimiter (val is ignored)
    */
stack_t scan(char *text, char *delimiters, stack_t keywords);

#endif
