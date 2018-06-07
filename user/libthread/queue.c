/** @file queue.c
 *
 *  @brief The implementation of a queue data structure.
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 *  @bug No known bug.
 */

#include <queue.h>
#include <simics.h>

#include <malloc.h>

/** @brief the queue structure that contains the head and tail of the queue
 */
struct queue {
    struct node *head;
    struct node *tail;
};

/** @brief The node stores data and the next/prev node's address.
 */
struct node {
    void *data;
    struct node *next;
    struct node *prev;
};

queue queue_new() {
    return calloc(1, sizeof(struct queue));
}

void queue_destroy(queue q) {
    free(q);
}

int enqueue(void *data, queue q) {
    if (q == NULL) {
        return -1;
    }
   
    struct node *n = calloc(1, sizeof(struct node));
    /* calloc failed */
    if (n == NULL) {
        return -1;
    }
    else {
	n->data = data;

        /* build relations */
        n->next = q->head;
        n->prev = NULL;
        if (q->head != NULL) {
            q->head->prev = n;
        }
        q->head = n;
	if (q->tail == NULL) {   /* queue was empty */
	    q->tail = n;
        }
        return 0;
    }
}

void *dequeue(queue q) {
    if (q == NULL || q->tail == NULL) {
        return NULL;
    }
    else {
        struct node *tail = q->tail;
        struct node *tmp = q->tail->prev;

        /* build relations */
        if (tmp == NULL) {      // queue only had 1 node
            q->head = NULL;
            q->tail = NULL;
        }
        else {
            tail->prev->next = NULL;
            q->tail = tmp;
        }

        void *data = tail->data;
	free(tail);
	return data;
    }
}

void *peek(queue q) {
    if (q == NULL || q->tail == NULL) {
        return NULL;
    }
    else {
	/* fetch the tail node data */
	return q->tail->data;
    }
}

int queue_empty(queue q) {
    if (q == NULL) {
        return -1;
    }
    if (q->head == NULL) {
        return 1;
    }
    else {
        return 0;
    }
}
