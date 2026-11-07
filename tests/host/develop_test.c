/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "seq/tb3po.h"
int main(void)
{
    unsigned seed, scale, changed = 0;
    for (scale = 0; scale < TB3PO_NSCALES; ++scale)
        for (seed = 1; seed < 200; ++seed) {
            bpart_t before = {0}, a, b;
            uint32_t ra = seed, rb = seed;
            int i;
            tb3po_defaults(&before.gen, seed);
            before.gen.scale = (uint8_t)scale;
            before.len = 17; /* odd lengths and hidden cells */
            tb3po_generate(&before);
            before.step[3].flags = G_TIE;
            a = b = before;
            tb3po_develop(&a, &ra);
            tb3po_develop(&b, &rb);
            assert(!memcmp(&a, &b, sizeof a) && ra == rb);
            assert(!memcmp(&a.gen, &before.gen, sizeof a.gen));
            assert(a.len == before.len && a.rate == before.rate && a.dir == before.dir && a.transpose == before.transpose);
            for (i = 0; i < NSTEPS; ++i) {
                assert(a.step[i].flags == before.step[i].flags);
                assert(a.step[i].note <= 127);
                if (i >= before.len || i % 4 == 0 || bstep_gate(&before.step[i]) != G_NOTE)
                    assert(a.step[i].note == before.step[i].note);
                if (a.step[i].note != before.step[i].note) {
                    int d = (int)a.step[i].note - before.step[i].note;
                    assert(d >= -5 && d <= 5); /* adjacent degrees of supplied scales */
                    ++changed;
                }
            }
        }
    assert(changed > 100);
    puts("develop: deterministic; rhythm/articulation/anchors/hidden cells preserved; nearby movement verified");
    return 0;
}
