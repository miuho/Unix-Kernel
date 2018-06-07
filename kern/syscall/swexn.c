/** @file kern/swexn.c
 *
 *  @brief swexn syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <common_include.h>

#include <syscall.h>

static char *tag = "swexn";


/**
 * @brief check if the input eflags.
 * 
 * @param eflags the input eflags.
 * @return 1 if succesful, 0 otherwise.
 *
 */
int check_eflags(unsigned long eflags) {
    if ((eflags & EFL_RESV1) == 0) {
        report_error(tag, "check_eflags: RESV1");
        return 0;
    }
    
    if ((eflags & EFL_AC) != 0) {
        report_error(tag, "check_eflags: AC");
        return 0;
    }

    if ((eflags & EFL_IOPL_RING3) != 0) {
        report_error(tag, "check_eflags: RING0");
        return 0;
    }
    
    if ((eflags & EFL_IF) == 0) {
        report_error(tag, "check_eflags: IF");
        return 0;
    }
    
    return 1;
}

int check_newureg(ureg_t *ureg) {
    unsigned long cs = ureg->cs;
    unsigned long ss = ureg->ss;
    unsigned long eflags = ureg->eflags;

    if (cs != SEGSEL_USER_CS) {
        report_error(tag, "check_newureg: cs wrong");
        return 0;
    }
    
    if (ss != SEGSEL_USER_DS) {
        report_error(tag, "check_newureg: ss wrong");
        return 0;
    }
    
    if (!check_eflags(eflags)) {
        report_error(tag, "check_newureg: eflags wrong");
        return 0;
    }

    return 1;
}

int swexn_handler(void *args) {

    report_progress(tag, "entry");

    ktcb_t *ktcb = running_ktcb;
    tcb_t *tcb = ktcb->tcb;
    pcb_t *pcb = ktcb->tcb->pcb;
    void *pgd = (void *)pcb->pgd;

    /* check args */
    if (vm_mem_region_check(pcb, pgd, args, 16) < 0) {
        report_error(tag, "swexn handler: arguments not accessible, exit");
        return -1;
    }

    void *esp3 = *(void **)args;
    swexn_handler_t eip = *(swexn_handler_t *)(args + 4);
    void *arg = *(void **)(args + 8);
    ureg_t *newureg = *(ureg_t **)(args + 12);

    report_progress(tag, "swexn_handler: esp3 = %p", esp3);
    report_progress(tag, "swexn_handler: eip = %p", eip);
    report_progress(tag, "swexn_handler: arg = %p", arg);
    report_progress(tag, "swexn_handler: newureg = %p", newureg);

    if (esp3 != NULL && 
        ((unsigned long)esp3 < USER_MEM_START || 
        (unsigned long)esp3 > USER_STACK_BASE)) {
        report_error(tag, "swexn handler: esp3 clearly wrong, exit");
        return -1;
    }

    if (eip != NULL &&
        ((unsigned long)eip < USER_MEM_START ||
        (unsigned long)eip > USER_STACK_BASE)) {
        report_error(tag, "swexn handler: eip clearly wrong, exit");
        return -1;
    }

    if (newureg != NULL &&
        vm_mem_region_check(pcb, pgd, newureg, 4) < 0) {
        report_error(tag, "swexn handler: newureg not accessible, exit");
        return -1;
    }

    swexn_handler_t old_eip = tcb->swexn_eip;
    void *old_esp3 = tcb->swexn_esp3;
    void *old_arg = tcb->swexn_arg;

    /* de-register an exception handler if one's registered */
    if (esp3 == NULL || eip == NULL) {
        tcb->swexn_eip = NULL;
        tcb->swexn_esp3 = NULL;
        tcb->swexn_arg = NULL;
    }

    /* register a software exception handler */
    else {
        tcb->swexn_eip = eip;
        tcb->swexn_esp3 = esp3 - ((unsigned long)esp3 % 4);
        tcb->swexn_arg = arg;
    }
    
    /* adopt the register values (including eip) before returning */
    /* to user space */
    if (newureg != NULL) {
        /* validate newureg */
        report_progress(tag, "cause=%d, cr2=0x%x", 
                        newureg->cause, newureg->cr2);
        if (!check_newureg(newureg)) {
            tcb->swexn_eip = old_eip;
            tcb->swexn_esp3 = old_esp3;
            tcb->swexn_arg = old_arg;

            return -1;
        }

        
        /* enable the IF flag of eflags register */
        newureg->eflags |= EFL_IF;

        report_progress(tag, "going to mode switch, exit");

        disable_interrupts();
        set_esp0(ktcb->regs->esp0);
        move_regs_and_mode_switch(newureg->edi, newureg->esi, newureg->ebp,
                                newureg->ebx, newureg->edx, newureg->ecx,
                                newureg->eax,
                                newureg->ds, newureg->fs, newureg->gs,
                                newureg->es, newureg->eip, newureg->cs, 
                                newureg->eflags, newureg->esp, newureg->ss);
    }

    report_progress(tag, "exit");

    return 0;
}

