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
#include <reporter.h>
#include <malloc.h>

static const char *tag = "queue";

/** @brief The node stores data and the next/prev node's address.
 */
struct node {
    void *data;
    struct node *next;
    struct node *prev;
};

queue queue_new() {

    queue q = calloc(1, sizeof(struct queue));
    if (q == NULL) {
        report_error(tag, "queue_new: calloc failed");
        return NULL;
    }

    if (mutex_init(&(q->mp)) < 0) {
        free(q);
        report_error(tag, "queue_new: mutex_init failed");
        return NULL;
    }
    
    return q;
}

int queue_traverse(queue q, isTrueElem fn) {
    /* check arg */
    if (q == NULL) {
        report_error(tag, "queue_traverse: input queue is NULL");
        return -1;
    }   

    struct node *n = q->head;
    if (n == NULL) {
        return 1;
    }

    /* check head to tail */
    while (n != NULL) {
        if (fn(n->data) == 0) {
            return 0;
        }
        n = n->next;
    }

    return 1;
}

int queue_traverse_all(queue q, isTrueElem fn) {
    if (q == NULL) {
        report_error(tag, "queue_traverse_all: input queue is NULL");
        return -1;
    }   
    
    struct node *n = q->head;
    if (n == NULL) {
        return 1;
    }
    
    int result = 1;
    /* check head to tail */
    while (n != NULL) {

        result &= (fn(n->data));
        n = n->next;
    }

    return result;
}

void *queue_find_true(queue q, isTrueElem fn) {
    if (q == NULL) {
        report_error(tag, "queue_find_true: input queue is NULL");
        return NULL;
    }   

    struct node *n = q->head;
    if (n == NULL) {
        return NULL;
    }

    /* check head to tail */
    void *data;
    while (n != NULL) {
        if (fn(n->data)) {
            data = n->data;
            return data;
        }
        n = n->next;
    }

    return NULL;
}

void queue_destroy(queue q) {
    mutex_destroy(&(q->mp));
    free(q);
}

int enqueue(void *data, queue q) {
    if (q == NULL) {
        report_error(tag, "enqueue: input queue is NULL");
        return -1;
    }
    

    struct node *n = calloc(1, sizeof(struct node));
    /* calloc failed */
    if (n == NULL) {
        report_error(tag, "enqueue: cannot calloc");
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
    if (q == NULL) {
        report_error(tag, "dequeue: input queue is NULL");
        return NULL;
    }
    

    if (q->tail == NULL) {
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
    /* check arg */
    if (q == NULL) {
        report_error(tag, "peek: input queue is NULL");
        return NULL;
    }
    

    if (q->tail == NULL) {
        return NULL;
    }

    else {
	/* fetch the tail node data */
	void *data = q->tail->data;
        return data;
    }
}

int queue_empty(queue q) {
    /* check arg */
    if (q == NULL) {
        report_error(tag, "queue_empty: input queue is NULL");
        return -1;
    }

    if (q->head == NULL) {
        return 1;
    }

    else {
        return 0;
    }
}

void *queue_find(int (*isEqual)(void *, void *), void *arg, queue q) {
    /* check arg */
    if (q == NULL) {
        report_error(tag, "queue_find: input queue is NULL");
        return NULL;
    }   
    

    struct node *n = q->head;
    if (n == NULL) {
        return NULL;
    }

    /* check head to tail */
    while (n != NULL) {
        if (isEqual(arg, n->data)) {
            void *data = n->data;
            return data;
        }
        n = n->next;
    }

    return 0;
}

void *queue_delete(int (*isEqual)(void *, void *), void *arg, queue q) {
    if (q == NULL) {
        report_error(tag, "queue_delete: input queue is NULL");
        return NULL;
    }   
    

    struct node *n = q->head;
    if (n == NULL) {
        return NULL;
    }

    /* check head to tail */
    while (n != NULL) {
        if (isEqual(arg, n->data)) {
            break;
        }
        n = n->next;
    }

    if (n == NULL) {
        return NULL;
    }

    /* found */
    void *data = n->data;
    
    if (n == q->head) {
        /* n at head */
        /* only elem */
        if (n == q->tail) {
            q->head = NULL;
            q->tail = NULL;
        }
        else {
            /* more exist */
            q->head = n->next;
            q->head->prev = NULL;
        }
    } 
    else {
        /* n not at head */
        if (n == q->tail) {
            /* n at tail */
            q->tail = n->prev;
            q->tail->next = NULL;
        }
        else {
            /* n in the middle */
            n->prev->next = n->next;
            n->next->prev = n->prev;
        }
    }

    free(n);

    return data;
}

int queue_size(queue q) {

    if (q == NULL) {
        report_error(tag, "queue_size: arg is NULL");
        return -1;
    }
    
    struct node *n = q->head;
    int count = 0;

    while (n != NULL) {
        count++;
        n = n->next;
    }
    

    return count;
}
