/** @file malloc.c
 *
 *  @brief thread-safe versions of malloc package
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include "malloc_internal.h"
#include <stdlib.h>
#include <types.h>
#include <stddef.h>

void *malloc(size_t __size)
{
  malloc_mutex_lock();
  void *tmp = _malloc(__size);
  malloc_mutex_unlock();

  return tmp;
}

void *calloc(size_t __nelt, size_t __eltsize)
{
  malloc_mutex_lock();
  void *tmp = _calloc(__nelt, __eltsize);
  malloc_mutex_unlock();

  return tmp;
}

void *realloc(void *__buf, size_t __new_size)
{
  malloc_mutex_lock();
  void *tmp = _realloc(__buf, __new_size);
  malloc_mutex_unlock();

  return tmp;
}

void free(void *__buf)
{
  malloc_mutex_lock();
  _free(__buf);
  malloc_mutex_unlock();

  return;
}
