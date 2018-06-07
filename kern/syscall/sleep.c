/** @file kern/sleep.c
 *
 *  @brief sleep syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "sleep";

int sleep_handler(int ticks) {

    if (ticks == 0) {
        report_progress(tag, "get 0 ticks, exit");
        return 0;
    }

    if (ticks < 0) {
        report_error(tag, "get negative ticks, exit");
        return -1;
    }
    
    int if_was_set = if_disable();
    
    report_progress(tag, "putting running_ktcb %p to sleep for %d ticks",
                    running_ktcb, ticks + ticks_global);
    sched_running_to_sleep(running_ktcb, ticks + ticks_global);
   
    report_progress(tag, "going to switch, exit"); 
    cs_save_and_switch(running_ktcb, sched_next());

    if_recover(if_was_set);

    report_progress(tag, "exit");
    return 0;
}
