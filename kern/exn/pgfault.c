/** @file kern/pgfault.c
 *
 *  @brief page fault handler implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <common_include.h>

static char *tag = "pgfault";

void pgfault_handler(unsigned long edi, unsigned long esi, unsigned long ebp,
                    unsigned long esp, unsigned long ebx, unsigned long edx,
                    unsigned long ecx, unsigned long eax, 
                    unsigned long error_code, unsigned long fault_eip, 
                    unsigned long fault_cs, unsigned long fault_eflags, 
                    unsigned long fault_esp, unsigned long fault_ss) {
    unsigned long fault_addr = (unsigned long)get_cr2();

    if (fault_cs == SEGSEL_KERNEL_CS) {
        report_error(tag, "pgfault at 0x%x from kernel", fault_addr);
    }
    else {
        report_warning(tag, "pgfault at 0x%x from user", fault_addr);
    }

    tcb_t *tcb = running_ktcb->tcb;
    pcb_t *pcb = tcb->pcb;
    void *pgd = (void *)pcb->pgd;
    
    /* check fault addr and report different messages */
    if (fault_addr < USER_MEM_START || fault_addr >= USER_STACK_BASE) {
        report_error(tag, "user tries to use kernel memory");

        ureg_t ureg;
        ureg_create(&ureg, SWEXN_CAUSE_PAGEFAULT, fault_addr,
                    fault_ss, fault_ss, fault_ss, fault_ss,
                    edi, esi, ebp, ebx, edx, ecx, eax, error_code,
                    fault_eip, fault_cs, fault_eflags, fault_esp,
                    fault_ss); 
        fault_handler(&ureg); 
    }

    if ((fault_addr >= (unsigned long)pcb->txt_base &&
        fault_addr < (unsigned long)(pcb->txt_base + pcb->txt_len)) ||
        (fault_addr >= (unsigned long)pcb->rodat_base &&
        fault_addr < (unsigned long)(pcb->rodat_base + pcb->rodat_len))) {
        report_error(tag, "user tries to write on read only memory");

        ureg_t ureg;
        ureg_create(&ureg, SWEXN_CAUSE_PAGEFAULT, fault_addr,
                    fault_ss, fault_ss, fault_ss, fault_ss,
                    edi, esi, ebp, ebx, edx, ecx, eax, error_code,
                    fault_eip, fault_cs, fault_eflags, fault_esp,
                    fault_ss); 
        fault_handler(&ureg); 
    }

    if (pgd_get_frm(pgd, (void *)fault_addr) == NULL) {
        report_warning(tag, "user tries to use unallocated memory");

        ureg_t ureg;
        ureg_create(&ureg, SWEXN_CAUSE_PAGEFAULT, fault_addr,
                    fault_ss, fault_ss, fault_ss, fault_ss,
                    edi, esi, ebp, ebx, edx, ecx, eax, error_code,
                    fault_eip, fault_cs, fault_eflags, fault_esp,
                    fault_ss); 
        fault_handler(&ureg); 
    }
    
    if (vm_frm_copy(pgd, (void *)fault_addr, 1) != 0) {
        report_error(tag, "fail to COW");

        ureg_t ureg;
        ureg_create(&ureg, SWEXN_CAUSE_PAGEFAULT, fault_addr,
                    fault_ss, fault_ss, fault_ss, fault_ss,
                    edi, esi, ebp, ebx, edx, ecx, eax, error_code,
                    fault_eip, fault_cs, fault_eflags, fault_esp,
                    fault_ss); 
        fault_handler(&ureg);
    }

    report_progress(tag, "pgfault handler resolved 0x%x which stores %d", 
                    (int)fault_addr, *(int *)fault_addr);
   
    report_progress(tag, "exit");
}

void pgfault_init(void *idt_base_p) {
    install_desc(idt_base_p, IDT_PF, pgfault_wrapper, trap_gate, 3);
}

void pgfault_handler_install() {
    void *idt_base_p = idt_base();
    pgfault_init(idt_base_p);
}

