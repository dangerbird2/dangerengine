/**
 * @brief
 *
 * Copyright (c) 2015, Steven Shea
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
*this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the dist.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
*FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
*those
 * of the authors and should not be interpreted as representing official
*policies,
 * either expressed or implied, of Steven Shea.
**/

#include "array.h"
#include "slsutils.h"
#include <stdlib.h>
#include <string.h>

struct slsArray_p {
  size_t length;
  size_t alloc_size;
  size_t element_size;
  char* array;
};

static const slsArray sls_array_proto = {.init = sls_array_init,
                                         .dtor = sls_array_dtor,
                                         .priv = NULL };

slsArray const* sls_array_class()
{
  return &sls_array_proto;
}

slsArray* sls_array_new(void const* array,
                        size_t element_size,
                        size_t n_elements)
{
  slsArray* self = NULL;
  self = sls_objalloc(sls_array_class(), sizeof(slsArray));
  if (self) {
    self = sls_msg(self, init, array, element_size, n_elements);
  }

  return self;
}

slsArray* sls_array_init(slsArray* self,
                         void const* data,
                         size_t element_size,
                         size_t n_elements)
{
  if (!self || (element_size == 0)) {
    return NULL;
  }

  self->priv = calloc(1, sizeof(slsArray_p));
  sls_checkmem(self->priv);

  size_t const alloc_size = element_size * n_elements;

  size_t const element_bytes = element_size * n_elements;

  *(self->priv) = (slsArray_p){.length = n_elements,
                               .element_size = element_size,
                               .alloc_size = alloc_size,
                               .array = calloc(alloc_size, sizeof(char)) };

  sls_checkmem(self->priv->array);

  // copy array to buffer
  if (data) {
    sls_check(memcpy(self->priv->array, data, element_bytes),
              "slsArray::init-> array failed to copy to buffer!");
  }

  return self;

error:
  if (self && self->dtor) {
    return sls_msg(self, dtor);
  } else {
    return sls_array_dtor(self);
  }
}

size_t sls_array_length(slsArray const* self)
{
  sls_check(self && self->priv, "null pointer");
  return self->priv->length;

error:
  return 0;
}

slsArray* sls_array_dtor(slsArray* self)
{
  if (!self) {
    return NULL;
  }
  if (self->priv) {
    if (self->priv->array) {
      free(self->priv->array);
    }
    free(self->priv);
  }

  return self;
}

size_t sls_array_element_size(slsArray const* self)
{
  sls_check(self && self->priv, "null pointer");

  return self->priv->element_size;
error:
  return 0;
}

void* sls_array_get(slsArray* self, size_t i)
{
  if (!self) {
    return NULL;
  }
  sls_check(i <= self->priv->length, "out of index error!");

  char* ptr = self->priv->array + (i * self->priv->element_size);

  return ptr;

error:
  return NULL;
}


void *sls_array_get_or_fail(slsArray *self, size_t i)
{
  void *res = sls_array_get(self, i);
  if (!res){
    raise(SIGABRT);
  }
  return res;
}


void const* sls_array_cget(slsArray const* self, size_t i)
{
  return sls_array_get((slsArray*)self, i);
}

slsArray* sls_array_copy(slsArray const* self)
{
  return sls_array_new(
    self->priv->array, self->priv->element_size, self->priv->length);
}

void sls_array_set(slsArray* self, size_t i, void* value)
{

  void* ptr = (void*)sls_array_get(self, i);
  if (!ptr || !value) {
    return;
  }

  memcpy(ptr, value, self->priv->element_size);
}

void sls_array_insert(slsArray* self, size_t i, void* value)
{
  sls_array_insert_array(self, i, value, 1);
}

void sls_array_insert_array(slsArray *self, size_t i, const void *values, size_t data_n_elements)
{
  if (!self || !self->priv) {
    return;
  }
  slsArray_p* p = self->priv;
  const size_t len = p->length;
  const size_t newlen = len + data_n_elements;
  const size_t newsize = newlen * p->element_size;
  const size_t insert_size = data_n_elements * p->element_size;
  sls_check(i <= len, "index %lu cannot be appended above %lu", i, len);

  // resize array if necessary
  size_t new_alloc_size = newsize;
  if (newsize >= p->alloc_size) {
    new_alloc_size = p->alloc_size > 2 ? p->alloc_size * 2 : 8;
    sls_array_reserve(self, new_alloc_size);
  }

  // now shift bytes determined by offset size
  size_t items_offset = len - i;

  // location to insert new array
  // if necessary, move existing items
  char* insert_start_point =
      p->array +
      i * p->element_size; // location which inserted values is offseting
  if (items_offset > 0) {

    char* offset = insert_start_point + p->element_size;
    memmove(offset, insert_start_point, p->element_size * items_offset);
  }

  // set values and increment length
  memcpy(insert_start_point, values, insert_size);
  p->length+= data_n_elements;

  return;
  error:
  return;
}

void sls_array_remove(slsArray *self, size_t index)
{
  slsArray_p *p = self->priv;
  sls_check(index < p->length, "index %lu out of bounds for array length %lu", index, p->length);
  if(index == p->length-1){
    --p->length; // just decrement the length counter
  } else {
    char *movdest = p->array + (index *p->element_size);
    char *movsrc = movdest + p->element_size;
    size_t bytes_moved = (p->length - 1 - index) * p->element_size;

    memmove(movdest, movsrc, bytes_moved);
    --p->length;
  }
  return;
  error:
  return;
}



void sls_array_append(slsArray* self, void* value)
{
  sls_array_insert(self, self->priv->length, value);
}

size_t sls_array_alloc_size(slsArray* self)
{
  return self->priv->alloc_size;
}

void sls_array_reserve(slsArray* self, size_t count)
{
  assert(self && self->priv);

  if (count < 1) {
    // ensure count is a natural number
    count = 8;
  }
  slsArray_p* p = self->priv;

  if (p->alloc_size < count) {
    size_t new_size = count;
    p->alloc_size = new_size;

    p->array = realloc(p->array, new_size * p->element_size);
    sls_checkmem(p->array);
  }

  return;
error:
  assert(0);
  return;
}

slsArrayItor* sls_arrayitor_begin(slsArray* self, slsArrayItor* itor)
{
  *itor =
    (slsArrayItor){.array = self, .index = 0, .elt = sls_array_get(self, 0) };
  return itor;
}

slsArrayItor* sls_arrayitor_next(slsArrayItor* self)
{
  size_t next = self->index + 1;
  sls_checkmem(self->array && self->array->priv);
  if (next >= self->array->priv->length) {
    return NULL;
  }
  self->index = next;
  self->elt =
    self->array->priv->array + (self->index * self->array->priv->element_size);
  return self;

error:
  return NULL;
}

