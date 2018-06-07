/** @file kern/key_driver.c 
 *
 *  @brief implementation of device-driver library.
 *
 *  @author An Wu (anwu)
 *  @author Hingon Miu (hmiu)
 */

#include <x86/asm.h>
#include <keyhelp.h>
#include <interrupt_defines.h>
#include <simics.h>
#include <stdio.h>
#include <key_wrapper.h>
#include <install_desc.h>
#include <loader.h>
#include <sched.h>
#include <context_switch.h>
#include <console.h>
#include <circ_buf.h>
#include <mutex.h>
#include <reporter.h>
#include <syscall_handler.h>

#define KEYBOARD_BUFFER_SIZE 128

/* keyboard buffer */
static circ_buf_t k_buf;

static char *tag = "key_driver";

/** @brief handle a keyboard interrupt
 *
 *  @return Void
 */
void keyboard_handler() {
    report_misc(tag, "KEYBOARD INTERRUPT");
     
    char c = inb(KEYBOARD_PORT);
    cb_put(&k_buf, c);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);  // tell PIC 'done'
    
    /* enable interrupts before trying to fill console */ 
    enable_interrupts();

    /* signal the first waiting readline/getchar */
    cond_t *cv_p;
    mutex_lock(&(cons_cond_queue->mp));
    if ((cv_p = peek(cons_cond_queue)) != NULL) {
        mutex_unlock(&(cons_cond_queue->mp));
        fill_cons();
    }
    else {
        mutex_unlock(&(cons_cond_queue->mp));
    }

}


/** @brief init the keyboard handler
 *
 *  @param idt_base_p the idt base
 *  @return Void
 */
int key_init(void *idt_base_p) {
    if (cb_init(&k_buf, KEYBOARD_BUFFER_SIZE) != 0) {
        report_error(tag, "cb_init failed");
        return -1;
    }
    report_progress(tag, "circ_buf init done!");

    install_desc(idt_base_p, KEY_IDT_ENTRY, keyboard_wrapper, 
                    interrupt_gate, 0);

    return 0;
}


int key_readchar(void)
{
    char c = cb_get(&k_buf);

    if (c == -1) {    // buffer empty
        return -1;
    }

    else {
        kh_type augchar = process_scancode(c);  

        if (KH_HASDATA(augchar) && KH_ISMAKE(augchar)) { // char available
            return KH_GETCHAR(augchar);
        }
        else {
            return key_readchar();
        }
    }
}

