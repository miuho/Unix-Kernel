/* user/libsyscall/new_pages.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global new_pages
new_pages:
    PUSH    %esi
    LEA     8(%esp), %esi       /* prepare arg */
    INT     $NEW_PAGES_INT      /* make system call */
    POP    %esi
    RET                         /* return */
