/** @file kern/malloc_wrappers.c
 *
 *  @brief wrappers for malloc package
 *  @author Hingon Miu (hmiu)
 *  @author An Wu (anwu)
 */

#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <mutex.h>
#include <reporter.h>
#include <if_flag.h>

mutex_t malloc_mp;
/* keep track whether the mutex lock is initialized or not */

static int initialized  = 0;

static char *tag = "malloc";

int malloc_init(void)
{
    if (mutex_init(&malloc_mp) != 0) {
        report_error(tag, "malloc_init: can't init malloc mp");
        return -1;
    }

    initialized = 1;

    return 0;
}

/* safe versions of malloc functions */

void *malloc(size_t size)
{
    if (initialized) mutex_lock(&malloc_mp);

    void *tmp = _malloc(size);

    if (initialized) mutex_unlock(&malloc_mp);

    return tmp;
}

void *memalign(size_t alignment, size_t size)
{
    if (initialized) mutex_lock(&malloc_mp);
  
    void *tmp = _memalign(alignment, size);
  
    if (initialized) mutex_unlock(&malloc_mp);
  
    return tmp;
}

void *calloc(size_t nelt, size_t eltsize)
{
    if (initialized) mutex_lock(&malloc_mp);
  
    void *tmp = _calloc(nelt, eltsize);
  
    if (initialized) mutex_unlock(&malloc_mp);
  
    return tmp;
}

void *realloc(void *buf, size_t new_size)
{
    if (initialized) mutex_lock(&malloc_mp);

    void *tmp = _realloc(buf, new_size);
  
    if (initialized) mutex_unlock(&malloc_mp);

    return tmp;
}

void free(void *buf)
{
    if (initialized) mutex_lock(&malloc_mp);

    _free(buf);
  
    if (initialized) mutex_unlock(&malloc_mp);

    return;
}

void *smalloc(size_t size)
{
    if (initialized) mutex_lock(&malloc_mp);

    void *tmp = _smalloc(size);
  
    if (initialized) mutex_unlock(&malloc_mp);

    return tmp;
}

void *smemalign(size_t alignment, size_t size)
{
    if (initialized) mutex_lock(&malloc_mp);

    void *tmp = _smemalign(alignment, size);
  
    if (initialized) mutex_unlock(&malloc_mp);
  
    return tmp;
}

void sfree(void *buf, size_t size)
{
    if (initialized) mutex_lock(&malloc_mp);

    _sfree(buf, size);
  
    if (initialized) mutex_unlock(&malloc_mp);

    return;
}


