/** @file queue.h
 *
 *  @brief The implementation of a queue data structure.
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 */

#ifndef _KERN_INC_QUEUE_H_
#define _KERN_INC_QUEUE_H_

#include <mutex.h>

/* the queue compare function */
typedef int (*isTrueElem)(void *);

/* the queue node declaration */
struct node;

/** @brief the queue structure that contains the head and tail of the queue
 */
typedef struct queue {
    /* lock */
    mutex_t mp;

    /* the head and tail of a queue */
    struct node *head;
    struct node *tail;
} *queue;


/** @brief construct a new queue
 *
 *  @return pointer to the new queue. NULL if failed
 */
queue queue_new();

/** @brief destroy (and free memory) a queue
 *
 *  if queue if not valid, then the operation is undefined
 *
 *  @param q the queue pointer intended to destroy
 *  @return Void
 */
void queue_destroy(queue q);

/** @brief Insert the pointer value to the queue.
 *
 *  @param q The queue 
 *  @param data The void * type data to be inserted.
 *  @return 0 on success, -1 on failure
 */
int enqueue(void *data, queue q);

/** @brief dequeue a value from the queue
 *
 *  @param q The queue
 *  @return the dequeued value. NULL if q is invalid or queue is empty
 */
void *dequeue(queue q);

/** @brief Returns the value to be dequeued
 *
 *  @param q The queue
 *  @return the value to be dequeued. NULL if q is invalid or queue is empty
 */
void *peek(queue q);

/** @brief check if a queue is empty
 *
 *  @param q the queue
 *  @return 1 if empty, 0 if not empty, -1 if queue invalid
 */
int queue_empty(queue q);

/** @brief find an element which satisfy isEqual in the queue
 *
 *  @param isEqual the check function
 *  @param arg first argument to isEqual
 *  @param q the queue we're using
 *  @return the data if found, NULL if not found
 */
void *queue_find(int (*isEqual)(void *, void *), void *arg, queue q);

/** @brief find an element which satisfy isEqual in the queue and delete it
 *
 *  @param isEqual the check function
 *  @param arg first argument to isEqual
 *  @param q the queue we're using
 *  @return the data if found, NULL if not found
 */
void *queue_delete(int (*isEqual)(void *, void *), void *arg, queue q);

/** @brief traverse a queue to check on the return values of fn are all 1.
 *         Can be short-circuited
 *
 *  @param q the queue
 *  @param fn the function we want to apply
 *  @return 1 if all return values are 1, 0 if any is 0
 */
int queue_traverse(queue q, isTrueElem fn);

/** @brief traverse a queue to apply fn on every node
 *
 *  @param q the queue
 *  @param fn the fn we want to apply
 *  @return 1 if all return values are 1, 0 if any is 0
 */
int queue_traverse_all(queue q, isTrueElem fn);

/** @brief find an element in queue if fn(element) returns true
 *
 *  @param q the queue
 *  @param fn the function we want to apply
 *  @return the element pointer if found, NULL if not found or error
 */
void *queue_find_true(queue q, isTrueElem fn);

/** @brief return the size of a queue
 *
 *  @param q the queue
 *  @return the size of the queue
 */
int queue_size(queue q);

#endif /* _QUEUE_H */
