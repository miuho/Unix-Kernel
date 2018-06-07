/** @file kern/halt.c
 *
 *  @brief halt syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "halt";

void halt_handler() {
    report_progress(tag, "entry");
    disable_interrupts();

    sim_call(SIM_HALT);

    halt();
}
