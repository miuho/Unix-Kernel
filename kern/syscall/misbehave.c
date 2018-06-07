/** @file kern/misbehave.c
 *
 *  @brief misbehave syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "misbehave";

void misbehave_handler() {
    report_progress(tag, "entry");
    report_progress(tag, "exit");
    return;
}
