/** @file kern/if_flag.c
 *
 *  @brief advanced disable/enable interrupts implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <x86/eflags.h>
#include <x86/asm.h>

int if_enable(void)
{
    unsigned long eflags = (unsigned long)get_eflags();
    int if_was_set =  eflags & EFL_IF;
    
    enable_interrupts();

    return if_was_set;
}

int if_disable(void)
{
    unsigned long eflags = (unsigned long)get_eflags();
    int if_was_set =  eflags & EFL_IF;
    
    disable_interrupts();

    return if_was_set;
}

void if_recover(int if_was_set)
{
    if (if_was_set) {
        enable_interrupts();
        return;
    }
    else {
        disable_interrupts();
        return;
    }
}
