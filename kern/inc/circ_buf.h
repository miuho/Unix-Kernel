/** @file kern/inc/circ_buf.h
 *  @brief header file for circular buffer
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_CIRC_BUF_H_
#define _KERN_INC_CIRC_BUF_H_

#include <mutex.h>

/* circular buffer struct definition */
typedef struct circ_buf {
    /* the buffer pointer */
    char *buf;

    /* space and position info */
    int len;
    int start;
    int end;
    int full;

    /* lock */
    mutex_t mp;
} circ_buf_t;

/** @brief init a circular buffer
 *
 *  @param cb the pointer to the cb struct
 *  @param len the length of the buffer
 *  @return 0 on success, negative on error 
 */
int cb_init(circ_buf_t *cb, int len); 

/** @brief destroy a circular buffer
 *
 *  @param cb the pointer to the cb struct
 *  @return Void
 */
void cb_destroy(circ_buf_t *cb); 

/** @brief put a char into the circular buffer
 *
 *  @param cb the poitner to the cb struct
 *  @param c the char we want to put
 *  @return 0 on success, negative on error
 */
int cb_put(circ_buf_t *cb, char c);

/** @brief get a char from the circular buffer
 *
 *  @param cb the pointer to the cb struct
 *  @return the char on success, -1 on error (or buffer empty)
 */
char cb_get(circ_buf_t *cb); 

/** @brief delete the end of circular buffer 
 *         (same as cb_get from end of buffer)
 *
 *  @param cb the pointer to the cb struct
 *  @return the char on success, -1 on error (or buffer empty)
 */
char cb_delete_end(circ_buf_t *cb);

/** @brief find a char in the circular buffer
 *
 *  @param cb the pointer to the cb struct
 *  @param c the char we want to find
 *  @return 1 on found, 0 on not found, -1 on error
 */
int cb_find(circ_buf_t *cb, char c);

#endif
