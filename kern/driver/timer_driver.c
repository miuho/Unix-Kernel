/** @file kern/timer_driver.c 
 *
 *  @brief implementation of device-driver library for timer.
 *
 *  @author An Wu (anwu)
 *  @author Hingon Miu (hmiu)
 */

#include <driver.h>
#include <x86/asm.h>
#include <interrupt_defines.h>
#include <timer_defines.h>
#include <simics.h>
#include <stdio.h>
#include <timer_wrapper.h>
#include <install_desc.h>
#include <loader.h>
#include <sched.h>
#include <context_switch.h>
#include <reporter.h>
#include <frame.h>

/* 5 ms period */
#define TIMER_FREQUENCY 500
#define CYCLES_BETWEEN_INTERRUPTS ((TIMER_RATE) / (TIMER_FREQUENCY))

static void (*tickback_globl)(unsigned int);    /* timer tickback function */
static int ticks = 0;                           /* # of ticks so far */

static const char *tag = "timer_driver";

/** @brief handle a timer interrupt
 *
 *  @return Void
 */
void timer_handler() {
    report_misc(tag, "TIMER INTERRUPT");

    tickback_globl(++ticks); 

    outb(INT_CTL_PORT, INT_ACK_CURRENT);  
    
    /* haven't set up loader yet */
    if (running_ktcb == NULL) {
        report_error(tag, "timer_handler: running_ktcb is NULL");
        return;
    }
    
    /* sched_ktcb keeps running */
    if (running_ktcb == sched_ktcb) {
        return;
    }

    /* put running_ktcb in runnable */
    if (sched_running_to_runnable(running_ktcb) != 0) {
        report_error(tag, "can't put running to runnable");
    }

    ktcb_t *ktcb = sched_next();
    
    report_misc(tag, "switching to %p", ktcb);

    cs_save_and_switch(running_ktcb, ktcb);
}


void timer_init(void *idt_base_p, void (*tickback)(unsigned int)) {
    outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
    outb(TIMER_PERIOD_IO_PORT, CYCLES_BETWEEN_INTERRUPTS & 0xFF);
    outb(TIMER_PERIOD_IO_PORT, (CYCLES_BETWEEN_INTERRUPTS >> 8) & 0xFF);

    tickback_globl = tickback;
    install_desc(idt_base_p, TIMER_IDT_ENTRY, timer_wrapper,
                    interrupt_gate, 0);
}

