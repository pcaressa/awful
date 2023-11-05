/** \file except.h */

#ifndef except_INC
#define except_INC

/** Allows a syntax of the form

        TRY { some code } CATCH { remedy code }

    To raise an exception use except_on(cond, fmt, ...)
    whose parameter list from fmt on is like the one
    of printf.
*/

#include <setjmp.h>

/** Exception handler. */
extern jmp_buf except_buf;

/** If cond is not 0 then raises an exception. */
extern void except_on(int cond, const char *fmt, ...);

#define TRY if(setjmp(except_buf)==0)
#define CATCH else

#endif
