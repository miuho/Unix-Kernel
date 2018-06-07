/* user/libsyscall/set_term_color.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global set_term_color
set_term_color:
    PUSH    %esi
    MOV     8(%esp), %esi       /* prepare arg */
    INT     $SET_TERM_COLOR_INT /* make system call */
    POP     %esi
    RET                         /* return */
