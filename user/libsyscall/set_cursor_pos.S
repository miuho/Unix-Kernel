/* user/libsyscall/set_cursor_pos.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global set_cursor_pos
set_cursor_pos:
    PUSH    %esi
    LEA     8(%esp), %esi       /* prepare arg */
    INT     $SET_CURSOR_POS_INT /* make system call */
    POP     %esi
    RET                         /* return */
