
#include "bitmap.h"

void bitmap_init(bitmap_t *bitmap, uint32_t num_bits)
{
    bitmap->num_bits = num_bits;

    // if num_bits == 29, 29/32 = 0
    bitmap->num_words = num_bits / 32;
    if ((bitmap->num_words * 32) < num_bits)
        bitmap->num_words++;

    bitmap->map = (uint32_t *) kmalloc(bitmap->num_words * sizeof(uint32_t));

}

void bitmap_set(bitmap_t *bitmap, uint32_t num)
{
    if (num > bitmap->num_bits) return;

    uint32_t word = num / 32;
    uint32_t bit = num % 32;

    bitmap->map[word] = bitmap->map[word] | (1 << bit);
}


void bitmap_clr(bitmap_t *bitmap, uint32_t num)
{
    if (num > bitmap->num_bits) return;

    uint32_t word = num / 32;
    uint32_t bit = num % 32;

    bitmap->map[word] = bitmap->map[word] & ~(1 << bit);
}


void bitmap_clear_all(bitmap_t *bitmap)
{
    kmemset(bitmap->map, 0, bitmap->num_words * sizeof(uint32_t));
}

void bitmap_set_all(bitmap_t *bitmap)
{
        kmemset(bitmap->map, 0xff, bitmap->num_words * sizeof(uint32_t));
}

int bitmap_isset(const bitmap_t * const bitmap, uint32_t bit)
{
    if (bit > bitmap->num_bits) return 0;

    int x = bit / 32;
    int y = bit % 32;
    uint32_t word = bitmap->map[x];
    return (word & (1 << y)) ? 1 : 0;
}

int bitmap_isclr(const bitmap_t * const bitmap, uint32_t bit)
{
    if (bit > bitmap->num_bits) return 0;
    
    int x = bit / 32;
    int y = bit % 32;
    uint32_t word = bitmap->map[x];
    return (word & (1 << y)) ? 0 : 1;
}

int bitmap_first_clear(const bitmap_t * const bitmap)
{
    uint32_t pos = 0;
    int i;

    while (bitmap->map[pos] == 0xffffffff) pos++;
    for (i = 0; i < 32; i++) {
        // found a clear bit
        if ((bitmap->map[pos] & (1 << i)) == 0)
            break;
    }
    return (pos * 32) + i;
}

