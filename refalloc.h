/*
 * file: refalloc.h
 * brief: Reference-counted memory blocks
 * author: Cody Licorish (svgmovement@gmail.com)
 */
#ifndef __RefAlloc_refAlloc_H__
#define __RefAlloc_refAlloc_H__

#include <stddef.h>

#ifdef REFALLOC_WIN32_DLL
#  ifdef REFALLOC_WIN32_DLL_INTERNAL
#    define REFALLOC_API __declspec(dllexport)
#  else
#    define REFALLOC_API __declspec(dllimport)
#  endif /*REFALLOC_DLL_INTERNAL*/
#else
#  define REFALLOC_API
#endif /*REFALLOC_WIN32_DLL*/

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/**
 * Destructor callback.
 * \param ptr Pointer to space to destroy and free
 */
typedef void (*refalloc_dtor)(void* ptr);

/**
 * \brief Allocate some reference counted space
 * \param sz Size of space in bytes
 * \param dtor Destructor to use in case of free
 * \return the space on sucess, NULL otherwise
 */
REFALLOC_API void* refalloc_malloc(size_t sz, refalloc_dtor dtor);

/**
 * \brief Acquire a reference to some space
 * \param ptr pointer, previously allocated with `refalloc_malloc`
 * \return the same pointer on sucess, NULL otherwise
 */
REFALLOC_API void* refalloc_acquire(void* ptr);

/**
 * \brief Release a reference to some space
 * \param ptr pointer, previously allocated with `refalloc_malloc`
 * \note If the reference count drops to zero, the space is freed.
 */
REFALLOC_API void refalloc_release(void* ptr);

#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /*__RefAlloc_refAlloc_H__*/
