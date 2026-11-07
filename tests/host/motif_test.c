/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "seq/motif.h"
int main(void)
{
    motif_bank_t m = {0};
    bpart_t a = {0}, b = {0}, original, dest;
    int i;
    a.len = 12; a.rate = RATE_32; a.dir = DIR_REV; a.transpose = 28;
    a.gen.seed = 123; a.gen.scale = 2;
    for (i = 0; i < NSTEPS; ++i) {
        a.step[i].note = (uint8_t)(24 + i);
        a.step[i].flags = (uint8_t)((i % 3) | BS_ACCENT | BS_SLIDE);
    }
    original = a;
    dest = a;
    assert(!motif_return(&m, 0, &dest));
    assert(!memcmp(&dest, &a, sizeof a));
    motif_like(&m, 0, &a);
    a.step[1].note = 110; a.len = 64;
    b.step[0].note = 99; b.len = 7;
    motif_like(&m, 1, &b);
    assert(motif_return(&m, 0, &a));
    assert(!memcmp(&a, &original, sizeof a));
    dest = original;
    assert(motif_return(&m, 1, &dest));
    assert(!memcmp(&dest, &b, sizeof b));
    assert(motif_return(&m, 0, &dest)); /* repeated RETURN does not consume capture */
    assert(!memcmp(&dest, &original, sizeof original));
    motif_like(&m, 0, &b); /* explicit LIKE replaces only A */
    assert(motif_return(&m, 0, &dest) && dest.len == 7);
    puts("motif: empty, independent captures, complete phrase, replace and repeated return OK");
    return 0;
}
