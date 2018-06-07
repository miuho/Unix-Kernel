/** @file kern/data_structure/circ_buf.c
 *
 *  @brief circular buffer implementation
 *  @author An Wu (anwu)
 *  @author Hingon Miu (hmiu)
 *
 */

#include <circ_buf.h>
#include <stddef.h>
#include <malloc.h>
#include <reporter.h>
#include <simics.h>

static const char *tag = "circ_buf";

int cb_init(circ_buf_t *cb, int len) {
    /* check args */
    if (cb == NULL || len <= 0) {
        report_error(tag, "wrong input to init");
        return -1;
    }
     
    /* allocate internal buf */
    if ((cb->buf = calloc(len, sizeof(char))) == NULL) {
        report_error(tag, "cannot calloc");
        return -1;
    }

    cb->start = 0;
    cb->end = 0;
    cb->full = 0;
    cb->len = len;

    if (mutex_init(&(cb->mp)) < 0) {
        report_error(tag, "fail to init mutex");
        return -1;
    }


    return 0;
}

void cb_destroy(circ_buf_t *cb) {
    /* check args */
    if (cb == NULL) {
        report_error(tag, "input is NULL");
        return;
    }

    /* destroy mem */
    mutex_destroy(&(cb->mp));
    free(cb->buf);
}

int cb_put(circ_buf_t *cb, char c) {
    /* check args */
    if (cb == NULL) {
        report_error(tag, "cb_put: input is NULL");
        return -1;
    }

    /* circ_buf already full */
    if (cb->full) {
        report_warning(tag, "cb_put: circ_buf is full");
        return -1;
    }

    cb->buf[cb->end] = c;
    cb->end = (cb->end + 1) % cb->len;

    /* full */
    if (cb->end == cb->start) {
        cb->full = 1;
    }

    return 0;
}

char cb_get(circ_buf_t *cb) {
    /* check args */
    if (cb == NULL) {
        report_error(tag, "cb_get: input is NULL");
        return -1;
    }

    /* empty */
    if (!cb->full && (cb->start == cb->end)) {
        report_progress(tag, "cb_get: circ_buf is empty");
        return -1;
    }

    char tmp = cb->buf[cb->start];

    cb->start = (cb->start + 1) % cb->len;
    cb->full = 0;

    return tmp;
}

char cb_delete_end(circ_buf_t *cb) {
    /* check args */
    if (cb == NULL) {
        report_error(tag, "cb_delete_end: input is NULL");
        return -1;
    }

    /* empty */
    if (!cb->full && (cb->start == cb->end)) {
        report_progress(tag, "cb_delete_end: circ_buf is empty");
        return -1;
    }

    char tmp = cb->buf[cb->end];

    cb->end = (cb->end - 1 + cb->len) % cb->len;
    cb->full = 0;

    return tmp;
}

int cb_find(circ_buf_t *cb, char c) {

    if (cb == NULL) {
        report_error(tag, "cb_find: input is NULL");
        return -1;
    }

    /* empty */
    if (!cb->full && (cb->start == cb->end)) {
        report_progress(tag, "cb_find: circ_buf is empty");
        return 0;
    }

    int idx = cb->start;

    do {
        if (cb->buf[idx] == c) {
            /* found */
            return 1;
        }
        idx = (idx + 1) % cb->len;
    } while (idx != cb->end);
    
    return 0;
}
