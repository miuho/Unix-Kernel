/** @file kern/inc/timed_queue.h
 *
 *  @brief timed queue data structure for sleep system call
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_TIMED_QUEUE_H_
#define _KERN_INC_TIMED_QUEUE_H_

/* the node structure for the queue */
typedef struct node {
    unsigned int ticks;
    void *data;

    struct node *next;
} node_t;

/* the timed_queue structure */
typedef struct timed_queue {
    node_t *head;
} timed_queue_t;

/** @brief init the timed_queue
 *
 *  @return 0 on success, -1 on error
 */
int tq_init();

/** @brief insert a data (which is "getable" from ticks timer ticks)
 *
 *  @param data the data we want to insert
 *  @param ticks the ticks from when it's "getable"
 */
void tq_insert(node_t *n, void *data, unsigned int ticks);

/** @brief get an available data from the timed queue
 *
 *  @return data if available, NULL if not available or error
 */
void *tq_get();

/** @brief check if the data is in tq
 *
 *  @param data the data we want to check
 *  @return 1 if exists, 0 otherwise
 */
int tq_find(void *data);

/** @brief delete data from tq regardless of ticks
 *
 *  @param data the data we want to delete
 *  @return the ticks that this data will be available at
 */
unsigned long tq_delete(void *data);

#endif
