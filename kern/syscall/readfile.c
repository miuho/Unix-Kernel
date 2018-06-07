/** @file kern/readfile.c
 *
 *  @brief readfile syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>
#include <exec2obj.h>

static char *tag = "readfile";

int readfile_handler(void *args) {
    report_progress(tag, "entry");

    pcb_t *pcb = running_ktcb->tcb->pcb;
    void *pgd = (void *)pcb->pgd;

    if (vm_mem_region_check(pcb, pgd, args, 16) < 0) {
        report_error(tag, "args not accessible, exit");
        return -1;
    }

    char *filename = *(char **)args;
    char *buf = *(char **)(args + 4);
    int count = *(int *)(args + 8);
    int offset = *(int *)(args + 12);

    if (count < 0 || offset < 0) {
        report_error(tag, "count or offset is negative, exit");
        return -1;
    }

    if (vm_mem_region_check(pcb, pgd, buf, count) != 1) {
        report_error(tag, "buf not writable, exit");
        return -1;
    }

    if (vm_mem_array_check(pcb, pgd, 1, filename) < 0) {
        report_error(tag, "filename not accessible, exit");
        return -1;
    }

    /* didn't use loader here, because we need to check offset and file 
     * length
     */
    int i;
    for (i = 0; i < exec2obj_userapp_count; i++) {
        if (strcmp(filename, exec2obj_userapp_TOC[i].execname) == 0)
            break;
    }

    if (i == exec2obj_userapp_count) {
        report_error(tag, "can't find filename in table of contents, exit");
        return -1;
    }

    int execlen = exec2obj_userapp_TOC[i].execlen;

    if (offset >= execlen) {
        report_error(tag, "offset is larger than execlen, exit");
        return -1;
    }

    int length = execlen - offset;
    char *ret = memcpy(buf, exec2obj_userapp_TOC[i].execbytes + offset,
                       length > count ? count : length);

    if (ret != buf) {
        report_error(tag, "memcpy failed, exit");
        return -1;
    }

    report_progress(tag, "exit");
    return length > count ? count : length;
}

