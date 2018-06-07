/** @file kern/set_status.c
 *
 *  @brief set_status syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "set_status";

void set_status_handler(int status) {

    report_progress(tag, "entry");

    running_ktcb->tcb->pcb->exit_status = status;

    report_progress(tag, "exit");
    return;
}
