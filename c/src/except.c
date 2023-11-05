/** \file except.c */

#include <stdarg.h>
#include <stdio.h>
#include "../header/except.h"

jmp_buf except_buf;

void except_on(int cond, const char *fmt, ...)
{
    if (cond) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        longjmp(except_buf, -1);
    }
}
