/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "seq/drumgen.h"
static int count(uint32_t x) { int n = 0; while (x) { n += (int)(x & 1u); x >>= 1; } return n; }
int main(void)
{
    int kit, seed, full, weird, moved = 0;
    for (kit = 0; kit < NKIT; ++kit) for (seed = 1; seed <= 64; ++seed) {
        for (weird = 0; weird <= 100; weird += 10) {
            dpart_t prev = {0};
            for (full = 0; full <= 100; ++full) {
                drumgen_cfg_t g = {(uint32_t)seed, (uint8_t)weird, (uint8_t)full};
                dpart_t p = {0}, same = {0};
                int v;
                drumgen_generate(&p, kit, &g); drumgen_generate(&same, kit, &g);
                assert(!memcmp(&p, &same, sizeof p));
                assert(p.len == 16 && p.rate == RATE_16);
                for (v = 0; v < NDRUM; ++v) {
                    assert(!(p.hit[v][0] & 0xffff0000u) && !p.hit[v][1]);
                    assert((prev.hit[v][0] & p.hit[v][0]) == prev.hit[v][0]);
                    if (!full) assert(!p.hit[v][0]);
                    if (v != 0 && v != 1 && v != (kit ? 10 : 7)) assert(!p.hit[v][0]);
                }
                prev = p;
            }
        }
        for (full = 1; full <= 100; ++full) {
            drumgen_cfg_t g = {(uint32_t)seed, 0, (uint8_t)full};
            dpart_t straight = {0};
            drumgen_generate(&straight, kit, &g);
            for (weird = 1; weird <= 100; ++weird) {
                dpart_t p = {0}; int v;
                g.weird = (uint8_t)weird; drumgen_generate(&p, kit, &g);
                for (v = 0; v < NDRUM; ++v) assert(count(p.hit[v][0]) == count(straight.hit[v][0]));
                assert(p.hit[0][0] & 1u); /* kick anchor */
                if (memcmp(p.hit, straight.hit, sizeof p.hit)) ++moved;
            }
        }
    }
    assert(moved > 1000);
    puts("drumgen: deterministic, nested density, independent placement, silence, bounds, kit routing OK");
    return 0;
}
