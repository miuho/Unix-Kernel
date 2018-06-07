/** @file kern/syscall_handler.c
 *
 *  @brief init the syscall handlers
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

int syscall_init() {
    if (print_init() != 0) {
        report_error("syscall", "syscall_init: print_init failed");
        return -1;
    }
    return 0;
}

