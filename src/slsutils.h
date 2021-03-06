/**
 * //
 * // Created by Steven on 4/25/15.
 * //
 *
 * @file
 * @brief utilities specific to Dangerengine environment
 */

#ifndef DANGERENGINE_SLSUTILS_H
#define DANGERENGINE_SLSUTILS_H



#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define sls_debug_glerrors() sls_debug_glerrors_impl_(__FILE__, __LINE__)

#include "slsmacros.h"

/**
 * @brief Error checker with same format as sls_check, but submits
 * a given error code on failure
 */

// redefine
#undef sls_checkmem
#define sls_checkmem(pointer)                                                  \
  do {                                                                         \
    sls_check((pointer), "memory error! %s", #pointer);                        \
  } while (0)

SLS_BEGIN_CDECLS

/**
 * @brief allocates 'size' bytes on the heap, and copies contents to
 * pointer
 * @details essentially a heap allocated memcpy, this facilitates
 * struct object construction
 *
 * @param prototype [description]
 * @param size [description]
 *
 * @return [description]
 */
void*
sls_objalloc(void const* prototype, size_t size);

/**
 * @brief cross-platform chdir function.
 * @detail Calls either win32 or possix chdir function
 * and returns value as specified by given function
 */


void
sls_drain_glerrors();

uint32_t
sls_debug_glerrors_impl_(char const* file, size_t line);



SLS_END_CDECLS
#endif // DANGERENGINE_SLSUTILS_H
