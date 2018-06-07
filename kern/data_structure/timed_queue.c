/** @file kern/data_structure/timed_queue.c 
 *
 *  @brief a timed queue (for sleep system call)
 *  @author An Wu (anwu)
 *  @author Hingon Miu (hmiu)
 *  
 */

#include <timed_queue.h>
#include <loader.h>
#include <reporter.h>

timed_queue_t tq;

static const char *tag = "timed_queue";

int tq_init() {
    tq.head = NULL;
    return 0;
}

void tq_insert(node_t *n, void *data, unsigned int ticks) {
    
    report_progress(tag, "tq_insert: entry");

    if (n == NULL) {
        report_error(tag, "tq_insert: input gets NULL, exit");
        return;
    }
    
    n->ticks = ticks;
    n->data = data;
    n->next = NULL;

    /* insert into queue */
    node_t *curr = tq.head;
    node_t *prev = NULL;

    while (curr != NULL) {
        if (ticks < curr->ticks) {
            /* found place to insert */
            if (prev == NULL) {
                /* at head */
                n->next = curr;
                tq.head = n;
            }
            else {
                /* at middle */
                prev->next = n;
                n->next = curr;
            }
            return;
        }
        prev = curr;
        curr = curr->next;
    }

    /* insert at last */
    if (prev == NULL) {
        /* at head */
        tq.head = n;
    }
    else {
        /* at end */
        prev->next = n;
    }

    report_progress(tag, "tq_insert: exit");
}

void *tq_get() {
    if (tq.head == NULL) {
        return NULL;
    }

    if (ticks_global > tq.head->ticks) {
        node_t *tmp = tq.head;
        void *data = tmp->data;
        tq.head = tmp->next;
        
        return data;
    }
    else {
        return NULL;
    }
}

int tq_find(void *data) {
    node_t *n = tq.head;

    while (n != NULL) {
        if (data == n->data) {
            /* found */
            return 1;
        }

        n = n->next;
    }

    /* not found */
    return 0;
}       

unsigned long tq_delete(void *data) {
    node_t *n = tq.head;
    node_t *prev = NULL;
    unsigned long temp;

    while (n != NULL) {
        if (data == n->data) {
            /* found */
            temp = n->ticks;

            if (prev == NULL) {
                tq.head = n->next;
            }
            else {
                prev->next = n->next;
            }
            
            return temp;
        }

        prev = n;
        n = n->next;
    }

    /* not found */
    return 0;
}
