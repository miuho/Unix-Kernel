/** @file kern/gettid.c
 *
 *  @brief gettid syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "gettid";

int gettid_handler() {
    report_progress(tag, "entry");

    ktcb_t *ktcb = running_ktcb;
    if (ktcb == NULL) {
        report_error(tag, "gettid_handler: no process is running, exit");
        return -1;
    }

    report_progress(tag, "exit");
    return ktcb->tcb->tid;
}
