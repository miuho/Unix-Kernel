/** @file kern/context/cs.c 
 *
 *  @brief context switch to another kernel thread
 *
 *  @author An Wu (anwu)
 *  @author Hingon Miu (hmiu)
 */

#include <context_switch.h>
#include <reg.h>
#include <cr.h>
#include <pgtable.h>
#include <loader.h>
#include <reporter.h>
#include <asm.h>
#include <kthread_pool.h>

static char *tag = "cs";


void cs_switch(unsigned long edi, unsigned long esi, unsigned long ebp, 
                unsigned long esp, unsigned long ebx, unsigned long edx, 
                unsigned long ecx, unsigned long eax, 
                unsigned long eip, ktcb_t *from, ktcb_t *to) {

    /* disable interrupts before entry */
    report_misc(tag, "entry");

    if (to == NULL) {
        report_error(tag, "cs_entry: context switch get null pointers, exit");
        return;
    }

    if (from == to) {
        report_misc(tag, "cs_entry: from == to, exit");
        return;
    }

    /* save from information */
    if (from == NULL) {
        report_warning(tag, "WARNING: cs_entry: from is NULL");
    }
    else {
        reg_t *from_regs = from->regs;
        from_regs->ebp = get_ebp();
    }
    
    running_ktcb = to;

    /* context switch */
    report_misc(tag, 
"going to switch, from=%p, current ebp=%x,  to=%p, to->ebp=0x%x, to->pgd=0x%x",
            from, get_ebp(), to, to->regs->ebp, to->tcb->pcb->pgd);
    if (from != NULL) 
        report_misc(tag, "saved from->ebp=0x%x", from->regs->ebp);

    report_misc(tag, "going to set esp0");
    set_esp0(to->regs->esp0);

    report_misc(tag, "going to set cr3");
    set_cr3(to->tcb->pcb->pgd);

    report_misc(tag, "going to switch, exit");

    set_ebp_and_switch(to->regs->ebp);
}

