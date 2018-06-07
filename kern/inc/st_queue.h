/** @file st_queue.h
 *
 *  @brief The implementation of a static queue data structure.
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 */

#ifndef _KERN_INC_ST_QUEUE_H_
#define _KERN_INC_ST_QUEUE_H_

/* static queue structure def */
struct st_node {
    void *data;
    struct st_node *next;
    struct st_node *prev;
};

/** @brief the queue structure that contains the head and tail of the queue
 */
typedef struct st_queue {
    struct st_node *head;
    struct st_node *tail;
} *st_queue;


/** @brief construct a new queue
 *
 *  @return pointer to the new queue. NULL if failed
 */
st_queue st_queue_new();

/** @brief destroy (and free memory) a queue
 *
 *  if queue if not valid, then the operation is undefined
 *
 *  @param q the queue pointer intended to destroy
 *  @return Void
 */
void st_queue_destroy(st_queue q);

/** @brief Insert the pointer value to the queue.
 *
 *  @param n the pre-allocated node
 *  @param q The queue 
 *  @param data The void * type data to be inserted.
 *  @return 0 on success, -1 on failure
 */
int st_enqueue(struct st_node *n, void *data, st_queue q);

/** @brief dequeue a value from the queue
 *
 *  @param q The queue
 *  @return the dequeued value. NULL if q is invalid or queue is empty
 */
void *st_dequeue(st_queue q);

/** @brief Returns the value to be dequeued
 *
 *  @param q The queue
 *  @return the value to be dequeued. NULL if q is invalid or queue is empty
 */
void *st_peek(st_queue q);

/** @brief check if a queue is empty
 *
 *  @param q the queue
 *  @return 1 if empty, 0 if not empty, -1 if queue invalid
 */
int st_queue_empty(st_queue q);

/** @brief find an element which satisfy isEqual in the queue
 *
 *  @param isEqual the check function
 *  @param arg first argument to isEqual
 *  @param q the queue we're using
 *  @return the data if found, NULL if not found
 */
void *st_queue_find(int (*isEqual)(void *, void *), void *arg, st_queue q);

/** @brief find an element which satisfy isEqual in the queue and delete it
 *
 *  @param isEqual the check function
 *  @param arg first argument to isEqual
 *  @param q the queue we're using
 *  @return the data if found, NULL if not found
 */
void *st_queue_delete(int (*isEqual)(void *, void *), void *arg, st_queue q);

/** @brief traverse a queue to check on the return values of fn are all 1.
 *         Can be short-circuited
 *
 *  @param q the queue
 *  @param fn the function we want to apply
 *  @return 1 if all return values are 1, 0 if any is 0
 */
int st_queue_traverse(st_queue q, int (*fn)(void *));

/** @brief traverse a queue to apply fn on every node
 *
 *  @param q the queue
 *  @param fn the fn we want to apply
 *  @return 1 if all return values are 1, 0 if any is 0
 */
int st_queue_traverse_all(st_queue q, int(*fn)(void *));

/** @brief find an element in queue if fn(element) returns true
 *
 *  @param q the queue
 *  @param fn the function we want to apply
 *  @return the element pointer if found, NULL if not found or error
 */
void *st_queue_find_true(st_queue q, int(*fn)(void *));

/** @brief return the size of a queue
 *
 *  @param q the queue
 *  @return the size of the queue
 */
int st_queue_size(st_queue q);

#endif /* _ST_QUEUE_H */
