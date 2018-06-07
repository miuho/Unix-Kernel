/** @file driver.c 
 *
 *  @brief implementation of device-driver library.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <driver.h>
#include <x86/asm.h>
#include <key_driver.h>
#include <timer_driver.h>
#include <reporter.h>

static char *tag = "driver";

int handler_install(void (*tickback)(unsigned int)) {

    void *idt_base_p = idt_base();

    /* init the handlers */
    timer_init(idt_base_p, tickback);
    report_progress(tag, "timer init done!");

    key_init(idt_base_p);
    report_progress(tag, "key init done!");

    return 0;
}


int readchar(void)
{
    return key_readchar();
}

