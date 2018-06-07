/** @file kern/print.c
 *
 *  @brief print syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <common_include.h>
#include <syscall.h>

mutex_t print_mp;

static char *tag = "print";

int print_init() {
    if (mutex_init(&print_mp) != 0) {
        report_error(tag, "print_init: mp init failed");
        return -1;
    }
    
    return 0;
}

int print_handler(void *args) {
    report_progress(tag, "entry");

    pcb_t *pcb = running_ktcb->tcb->pcb;

    if (vm_mem_region_check(pcb, (void *)pcb->pgd, args, 8) < 0) {
        report_error(tag, "print_handler: arguments not accessible, exit");
        return -1;
    }

    int len = *(int *)args;
    if (len < 0) {
        report_error(tag, "invalid len, exit");
        return -1;
    }

    char *buf = *(char **)(args + 4);
    if (vm_mem_region_check(pcb, (void *)pcb->pgd, buf, len) < 0) {
        report_error(tag, "print_handler: can't read from buf, exit");
        return -1;
    }

    char c;

    mutex_lock(&print_mp);
    while (len != 0) {
        c = *(buf);
        putbyte(c);
        len--;
        buf++;
    }
    mutex_unlock(&print_mp);

    report_progress(tag, "exit");

    return 0;
}
