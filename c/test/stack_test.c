#include "../header/stack.h"

int main(void)
{
    stack_t s1 = NULL;
    for (int i = 0; i < 10; ++ i) {
        val_t n = {.type = NUMBER, .val.n = i};
        s1 = stack_push(s1, n);
    }
    s1 = stack_reverse(s1);
    fputs("s1 = ", stderr);
    stack_printf(stderr, s1);
    stack_status();
    
    stack_t s2 = NULL;
    val_t v = {.type = STACK, .val.s = s1};
    s2 = stack_push(s2, v);
    s2 = stack_push(s2, v);
    s2 = stack_push(s2, v);
    s2 = stack_reverse(s2);
    fputs("s2 = ", stderr);
    stack_printf(stderr, s2);
    stack_status();
    stack_delete(s2);
    stack_status();
    
    v.val.s = s2;
    s2 = stack_push(s2, v);
    stack_printf(stderr, s2);
    s2 = stack_reverse(s2);

    stack_status();
    stack_delete(s2);
    stack_status();
}
