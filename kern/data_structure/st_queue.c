/** @file st_queue.c
 *
 *  @brief The implementation of a st_queue data structure.
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 */

#include <st_queue.h>
#include <simics.h>
#include <reporter.h>
#include <malloc.h>
#include <string.h>

static const char *tag = "st_queue";

st_queue st_queue_new() {

    st_queue q = calloc(1, sizeof(struct st_queue));
    if (q == NULL) {
        report_error(tag, "st_queue_new: calloc failed");
        return NULL;
    }

    return q;
}

int st_queue_traverse(st_queue q, int (*fn)(void *)) {
    /* check arg */
    if (q == NULL) {
        report_error(tag, "st_queue_traverse: input st_queue is NULL");
        return -1;
    }   

    struct st_node *n = q->head;
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

int st_queue_traverse_all(st_queue q, int (*fn)(void *)) {
    if (q == NULL) {
        report_error(tag, "st_queue_traverse_all: input st_queue is NULL");
        return -1;
    }   

    struct st_node *n = q->head;
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

void *st_queue_find_true(st_queue q, int (*fn)(void *)) {
    if (q == NULL) {
        report_error(tag, "st_queue_find_true: input st_queue is NULL");
        return NULL;
    }   

    struct st_node *n = q->head;
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

void st_queue_destroy(st_queue q) {
    free(q);
}

int st_enqueue(struct st_node *n, void *data, st_queue q) {
    if (q == NULL) {
        report_error(tag, "st_enqueue: input st_queue is NULL");
        return -1;
    }
   
    if (n == NULL) {
        report_error(tag, "st_enqueue: st_node ptr is NULL"); 
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

void *st_dequeue(st_queue q) {
    if (q == NULL) {
        report_error(tag, "st_dequeue: input st_queue is NULL");
        return NULL;
    }
    

    if (q->tail == NULL) {
        return NULL;
    }

    else {
        struct st_node *tail = q->tail;
        struct st_node *tmp = q->tail->prev;

        /* build relations */
        if (tmp == NULL) {      // queue only had 1 st_node
            q->head = NULL;
            q->tail = NULL;
        }
        else {
            tail->prev->next = NULL;
            q->tail = tmp;
        }

        void *data = tail->data;
	return data;
    }
}

void *st_peek(st_queue q) {
    /* check arg */
    if (q == NULL) {
        report_error(tag, "st_peek: input st_queue is NULL");
        return NULL;
    }
    

    if (q->tail == NULL) {
        return NULL;
    }

    else {
	/* fetch the tail st_node data */
	void *data = q->tail->data;
        return data;
    }
}

int st_queue_empty(st_queue q) {
    /* check arg */
    if (q == NULL) {
        report_error(tag, "st_queue_empty: input st_queue is NULL");
        return -1;
    }

    if (q->head == NULL) {
        return 1;
    }

    else {
        return 0;
    }
}

void *st_queue_find(int (*isEqual)(void *, void *), void *arg, st_queue q) {
    /* check arg */
    if (q == NULL) {
        report_error(tag, "st_queue_find: input st_queue is NULL");
        return NULL;
    }   
    

    struct st_node *n = q->head;
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

void *st_queue_delete(int (*isEqual)(void *, void *), void *arg, st_queue q) {
    if (q == NULL) {
        report_error(tag, "st_queue_delete: input st_queue is NULL");
        return NULL;
    }   
    

    struct st_node *n = q->head;
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

    return data;
}

int st_queue_size(st_queue q) {

    if (q == NULL) {
        report_error(tag, "st_queue_size: arg is NULL");
        return -1;
    }
    
    struct st_node *n = q->head;
    int count = 0;

    while (n != NULL) {
        count++;
        n = n->next;
    }
    
    return count;
}
