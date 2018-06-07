/** @file kern/readline.c
 *
 *  @brief readline syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "readline";

void fill_cons() {
    
    report_progress(tag, "fill_cons: entry");

    char c;

    while ((c = (char)readchar()) != -1) {
        report_progress(tag, "fill_cons: get %c", c);

        /* handle backspace */
        if (c == '\b') {
            if (cb_delete_end(&cons_buf) != -1) {
                /* line available. echo to console */
                putbyte(c);
            }
        }
        else {
            /* only thread accessing cons_buf. no lock */
            cb_put(&cons_buf, c);

            /* echo to console */
            putbyte(c);

            /* if see a next line, signal a waiting readline */
            if (c == '\n') {
                cond_t *cv_p;
                mutex_lock(&(cons_cond_queue->mp));
                if ((cv_p = peek(cons_cond_queue)) != NULL) {
                    mutex_unlock(&(cons_cond_queue->mp));
                    cond_signal(cv_p);
                }
                else {
                    mutex_unlock(&(cons_cond_queue->mp));
                }
            }
        }
    }
}


int readline_handler(void *args) {
    report_progress(tag, "readline_handler: entry");

    pcb_t *pcb = running_ktcb->tcb->pcb;

    if (vm_mem_region_check(pcb, (void *)pcb->pgd, args, 8) < 0) {
        report_error(tag, "readline_handler: arguments not accessible, exit");
        return -1;
    }

    int len = *(int *)args;
    if (len < 0) {
        report_error(tag, "readline_handler: invalid len, exit");
        return -1;
    }

    char *buf = *(char **)(args + 4);
    if (vm_mem_region_check(pcb, (void *)pcb->pgd, buf, len) != 1) {
        report_error(tag, "readline_handler: can't write to buf, exit");
        return -1;
    }

    /* allocate locks for this call */
    cond_t cv;
    mutex_t mp;

    if (cond_init(&cv) != 0 || mutex_init(&mp) != 0) {
        report_error(tag, "readline_handler: can't initialize cond or mp");
    }

    /* put its cond in the global queue */
    mutex_lock(&(cons_cond_queue->mp));
    enqueue(&cv, cons_cond_queue);
    mutex_unlock(&(cons_cond_queue->mp));

    mutex_lock(&mp);

    while (1) {

        /* wait on signal from keyboard or another readline */
        cond_wait(&cv, &mp);

        /* check newline */
        if (cb_find(&cons_buf, '\n') == 1) {
            break;
        }
    }

    mutex_unlock(&mp);

    /* fill buf */
    int i;
    char c = -1;
    int read_len = 0;
    for (i = 0; i < len; i++) {
        c = cb_get(&cons_buf);

        if (c == -1) {
            report_error(tag, "readline_handler: see -1 when copy to buf");
            break;
        }

        buf[i] = c;
        read_len++;

        if (c == '\n')
            break;
    }

    /* dequeue itself */
    mutex_lock(&(cons_cond_queue->mp));
    if (dequeue(cons_cond_queue) != &cv) {
        report_error(tag, "dequeue cons_cond_queue gets different cv");
    }
    mutex_unlock(&(cons_cond_queue->mp));

    mutex_destroy(&mp);
    cond_destroy(&cv);

    /* there might be things left to read */
    if (read_len == len && c != '\n') {
        report_progress(tag, "going to signal next cond, exit");
        
        cond_t *cv_p;
         
        mutex_lock(&(cons_cond_queue->mp));
        if ((cv_p = peek(cons_cond_queue)) != NULL) {
            mutex_unlock(&(cons_cond_queue->mp));
            cond_signal(cv_p);
        }
        else {
            mutex_unlock(&(cons_cond_queue->mp));
        }
    }

    report_progress(tag, "exit");
    return read_len;
}
