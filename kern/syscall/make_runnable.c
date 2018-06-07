/** @file kern/make_runnable.c
 *
 *  @brief make_runnable syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "make_runnable";

int make_runnable_handler(int tid) {

    report_progress(tag, "entry");
    
    int if_was_set = if_disable();
    ktcb_t *to_run;
    if ((to_run = sched_waiting_to_running(tid)) == NULL) {
        /* this tid was not waiting */
        if_recover(if_was_set);
        report_warning(tag, "cannot find waiting thread tid=%d, exit", tid);
        return -1;
    }

    /* calling thread is suspended and put into the runnable queue */
    if (sched_running_to_runnable(running_ktcb) == -1) {
        if_recover(if_was_set);
        /* running ktcb is in runnable queue */
        report_warning(tag, 
                "cannot enqueue running_ktcb into runnable queue, exit");
        return -1;
    }

    report_progress(tag, "make_runnable going to switch, exit");

    cs_save_and_switch(running_ktcb, to_run);
    
    if_recover(if_was_set);
    report_progress(tag, "make_runnable returns, exit");
    return 0;
}
