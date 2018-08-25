/*
 * Copyright 2018 Thomas de Grivel <thoxdg@gmail.com> +33614550127
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include "data.h"

s_data_type g_data_alloc_type = {
  sizeof(s_data_alloc) * 8,
  DATA_TYPE_BITS
};

s_data_alloc g_data_alloc;

void data_init ()
{
  data_alloc_init(&g_data_alloc, &g_data_alloc_type,
                  DATA_ALLOC_MAX, 0, (f_data_clean*) data_alloc_clean);
}

void data_alloc_init (s_data_alloc *da, s_data_type *t,
                      unsigned int max, f_data_init *init,
                      f_data_clean *clean)
{
  assert(da);
  assert(t);
  da->t = t;
  da->max = max;
  da->mem = calloc(max, (t->bits + 7) / 8);
  da->n = 0;
  da->free = calloc(max, sizeof(unsigned int));
  da->free_n = 0;
  da->init = init;
  da->clean = clean;
}

void data_alloc_clean (s_data_alloc *da)
{
  assert(da);
  free(da->mem);
  free(da->free);
}

static inline
void * data_new_at (s_data_alloc *da, unsigned int i)
{
  unsigned int octets = (da->t->bits + 7) / 8;
  unsigned int offset = i * octets;
  void *m = da->mem + offset;
  bzero(m, octets);
  if (da->init)
    da->init(m);
  return m;
}

void * data_new (s_data_alloc *da)
{
  assert(da);
  if (da->free_n) {
    unsigned int i = da->free[--da->free_n];
    da->free[da->free_n] = 0;
    return data_new_at(da, i);
  }
  if (da->n < da->max) {
    unsigned int i = da->n++;
    return data_new_at(da, i);
  }
  return 0;
}

void data_delete (s_data_alloc *da, void *data)
{
  unsigned int octets;
  unsigned int i;
  assert(da);
  assert(da->t);
  octets = ((da->t->bits + 7) / 8);
  assert(da->mem <= data);
  assert(data < da->mem + da->max * octets);
  if (da->clean)
    da->clean(data);
  i = (data - da->mem) / octets;
  da->free[da->free_n++] = i;
}
