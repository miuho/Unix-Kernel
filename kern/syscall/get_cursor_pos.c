/** @file kern/get_cursor_pos.c
 *
 *  @brief get_cursor_pos syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "get_cursor_pos";

int get_cursor_pos_handler(void *args) {
    report_progress(tag, "entry");

    pcb_t *pcb = running_ktcb->tcb->pcb;

    if (vm_mem_region_check(pcb, (void *)pcb->pgd, args, 8) < 0) {
        report_error(tag, "get_cursor_pos: arguments not accessible, exit");
        return -1;
    }

    int *row = *(int **)args;
    int *col = *(int **)(args + 4);

    if (vm_mem_region_check(pcb, (void *)pcb->pgd, (void *)row, 4) != 1) {
        report_error(tag, "get_cursor_pos: row not accessible, exit");
        return -1;
    }

    if (vm_mem_region_check(pcb, (void *)pcb->pgd, (void *)col, 4) != 1) {
        report_error(tag, "get_cursor_pos: col not accessible, exit");
        return -1;
    }

    get_cursor(row, col);

    report_progress(tag, "exit");

    return 0;
}
