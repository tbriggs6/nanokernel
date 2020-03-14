#ifndef _BITMAP_H
#define _BITMAP_H
#include <stdint.h>
#include "kstdlib.h"

typedef struct {
    uint32_t *map;
    uint32_t num_bits;
    uint32_t num_words;
} bitmap_t;

void bitmap_init(bitmap_t *bitmap, uint32_t num_bits);
void bitmap_clear_all(bitmap_t *bitmap);
void bitmap_set_all(bitmap_t *bitmap);
int bitmap_isset(const bitmap_t * const bitmap, int bit);
int bitmap_isclr(const bitmap_t * const bitmap, int bit);
int bitmap_first_clear(const bitmap_t * const bitmap);

#endif