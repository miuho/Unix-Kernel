/** @file queue.h
 *
 *  @brief The implementation of a queue data structure.
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 */

#ifndef _QUEUE_H
#define _QUEUE_H

/** @brief the queue structure
 */
typedef struct queue *queue;

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
 *  @return 1 if not empty, 0 if empty, -1 if queue invalid
 */
int queue_empty(queue q);

#endif /* _QUEUE_H */
