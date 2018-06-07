/** @file kern/set_cursor_pos.c
 *
 *  @brief set_cursor_pos syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "set_cursor_pos";

int set_cursor_pos_handler(void *args) {
    
    report_progress(tag, "entry");

    pcb_t *pcb = running_ktcb->tcb->pcb;

    if (vm_mem_region_check(pcb, (void *)pcb->pgd, args, 8) < 0) {
        report_error(tag, "set_cursor_pos: arguments not accessible, exit");
        return -1;
    }

    int row = *(int *)args;
    int col = *(int *)(args + 4);
    int temp = set_cursor(row, col);

    report_progress(tag, "exit");
    return temp;
}
