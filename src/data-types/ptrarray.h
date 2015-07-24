//
// Created by Steven on 7/22/15.
//

#ifndef DANGERENGINE_PTRARRAY_H
#define DANGERENGINE_PTRARRAY_H

#include <stdlib.h>

#include "callbacks.h"

typedef struct slsPtrArray slsPtrArray;

/**
 * @brief A dynamic array specifically for storing an array of pointers
 */
struct slsPtrArray {
  void **data;
  size_t n_elements;
  size_t n_alloced;

  slsFreeFn free_fn;

};

slsPtrArray *sls_ptrarray_init(slsPtrArray *self,
                               void **data, size_t
                               n_elements,
                               slsFreeFn free_fn);

slsPtrArray *sls_ptrarray_dtor(slsPtrArray *self);

void sls_ptrarray_expand(slsPtrArray *self);

void *sls_ptrarray_remove(slsPtrArray *self, size_t idx);


#endif //DANGERENGINE_PTRARRAY_H

