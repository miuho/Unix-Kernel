/** @file kern/get_ticks.c
 *
 *  @brief get_ticks syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "get_ticks";

int get_ticks_handler() {
    report_progress(tag, "entry");
    report_progress(tag, "exit");
    return ticks_global;
}
