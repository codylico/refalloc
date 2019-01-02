
#define REFALLOC_WIN32_DLL_INTERNAL

#include "refalloc.h"
#include <limits.h>
#include <stdlib.h>

/* BEGIN definition of reference counter */

/*
static refalloc_counter refalloc_counter_acquire
    (refalloc_counter* dest);
static refalloc_counter refalloc_counter_release
    (refalloc_counter* dest);
*/

#if (defined __STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) \
    && (!defined __STDC_NO_ATOMICS__)
#  include <stdatomic.h>
  typedef _Atomic int volatile refalloc_counter;
#  define REFALLOC_COUNTER_MAX (INT_MAX/2)
  static refalloc_counter refalloc_counter_acquire
      (refalloc_counter* dest)
  {
    return atomic_fetch_add_explicit(dest, +1, memory_order_acquire);
  }
  static refalloc_counter refalloc_counter_release
      (refalloc_counter* dest)
  {
    return atomic_fetch_add_explicit(dest, -1, memory_order_release);
  }
#elif ((defined __clang__) || (defined __GNUC__))
  typedef int volatile refalloc_counter;
#  define REFALLOC_COUNTER_MAX (INT_MAX/2)
  static refalloc_counter refalloc_counter_acquire
      (refalloc_counter* dest)
  {
    return __sync_fetch_and_add(dest, 1);
  }
  static refalloc_counter refalloc_counter_release
      (refalloc_counter* dest)
  {
    return __sync_fetch_and_sub(dest, 1);
  }
#elif _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
  typedef LONG volatile refalloc_counter;
#  define REFALLOC_COUNTER_MAX (LONG_MAX/2)
  static refalloc_counter refalloc_counter_acquire
      (refalloc_counter* dest)
  {
    return InterlockedExchangeAddAcquire(dest, +1);
  }
  static refalloc_counter refalloc_counter_release
      (refalloc_counter* dest)
  {
    return InterlockedExchangeAddRelease(dest, -1);
  }
#else
  typedef int volatile refalloc_counter;
#  define REFALLOC_COUNTER_MAX (INT_MAX/2)
  static refalloc_counter refalloc_counter_acquire
      (refalloc_counter* dest)
  {
    return (*dest+=1)-1;
  }
  static refalloc_counter refalloc_counter_release
      (refalloc_counter* dest)
  {
    return (*dest-=1)+1;
  }
#endif /*REFALLOC_OS_...*/

/* END   definition of reference counter */

/* BEGIN definition of reference base structure */

struct refalloc_base {
  refalloc_counter n;
  refalloc_dtor dtor;
  void* ptr;
};


/* END definition of reference base structure */

/* BEGIN implementation of reference counting */

void* refalloc_malloc(size_t sz, refalloc_dtor dtor){
  size_t const static refalloc_offset = offsetof(struct refalloc_base,ptr);
  size_t const static max_size =
    (~((size_t)0U)) - offsetof(struct refalloc_base,ptr);
  if (sz >= max_size){
    /* reject */
    return NULL;
  } else {
    void *out = malloc(sz+refalloc_offset);
    if (out != NULL){
      struct refalloc_base* const outref = (struct refalloc_base*)out;
      outref->n = 1;
      outref->dtor = dtor;
      return &(outref->ptr);
    } else return NULL;
  }
}

void* refalloc_acquire(void* ptr){
  size_t const static refalloc_offset = offsetof(struct refalloc_base,ptr);
  struct refalloc_base* const ptrref =
    (struct refalloc_base*)(void*)(((char*)ptr)-refalloc_offset);
  /* try to acquire a reference */{
    refalloc_counter next = refalloc_counter_acquire(&ptrref->n);
    if (next < REFALLOC_COUNTER_MAX){
      return ptr;
    } else /* artifically refuse the reference */{
      refalloc_counter prev = refalloc_counter_release(&ptrref->n);
      if (prev == 1){
        if (ptrref->dtor != NULL)
          (*ptrref->dtor)(ptr);
        free(ptrref);
      }
      return NULL;
    }
  }
}

void refalloc_release(void* ptr){
  size_t const static refalloc_offset = offsetof(struct refalloc_base,ptr);
  if (ptr != NULL){
    struct refalloc_base* const ptrref =
      (struct refalloc_base*)(void*)(((char*)ptr)-refalloc_offset);
    /* release the reference */{
      refalloc_counter prev = refalloc_counter_release(&ptrref->n);
      if (prev == 1){
        if (ptrref->dtor != NULL)
          (*ptrref->dtor)(ptr);
        free(ptrref);
      }
    }
  }
  return;
}

/* BEGIN implementation of reference counting */
