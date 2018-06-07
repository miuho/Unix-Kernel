/** @file frame.h
 *
 *  @brief physical frame manipulation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_FRAME_H_
#define _KERN_INC_FRAME_H_

#include <mutex.h>

/* the queue of free frames */
struct queue *free_frame_queue;

/* the lock for free_frame_queue */
mutex_t frm_mp;

/* the lock for frame reference count (COW) */
mutex_t frm_ref_mp;

/** @brief init the frame
 *
 *  @return Void
 */
int frame_init(void);

/** @brief init the frames for the kernel (direct mapping)
 *         automatically set the reference count to 1
 *
 *  @return Void
 */
void *frame_kern_init(void);

/** @brief allocate a frame from free_frame_queue
 *         automatically untrack the reference count
 *
 *  @return Void
 */
void *frame_alloc(void);

/** @brief free a frame and put it inside free_frame_queue
 *
 *  @param frame the frame we want to free
 *  @return Void
 */
void frame_free(void *frame);

/** @brief get the reference count (COW) of a frame
 *
 *  @param frame the frame we want to check
 *  @return Void
 */
int get_frame_refs(void *frame);

/** @brief set the reference count (COW) of a frame
 *
 *  @param frame the frame we want to set
 *  @param refs the reference count we want to set
 *  @return 0 on success, -1 on error
 */
int set_frame_refs(void *frame, int refs);

/** @brief get the number of frames free
 *
 *  @return the number of frames free
 */
int frame_get_count();

#endif
