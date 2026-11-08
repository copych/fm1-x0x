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
                    if (v != drumgen_voice(kit, DG_CLOSED))
                        assert((prev.hit[v][0] & p.hit[v][0]) == prev.hit[v][0]);
                    if (!full) assert(p.hit[v][0] == ((v == 0 || v == drumgen_voice(kit, DG_CRASH)) ? 1u : 0u));
                }
                {
                    int ch = drumgen_voice(kit, DG_CLOSED), oh = drumgen_voice(kit, DG_OPEN);
                    uint32_t before = prev.hit[ch][0] | prev.hit[oh][0], now = p.hit[ch][0] | p.hit[oh][0];
                    assert((before & now) == before);
                    assert(!(p.hit[ch][0] & p.hit[oh][0]));
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
    for (kit = 0; kit < NKIT; ++kit) for (seed = 1; seed <= 64; ++seed) {
        static const uint8_t levels[] = {0,25,50,75};
        static const uint32_t hats[] = {0,0x4444u,0x5555u,0xFFFFu};
        int i;
        for (i = 0; i < 4; ++i) {
            drumgen_cfg_t g = {(uint32_t)seed, 0, levels[i]};
            dpart_t p = {0};
            drumgen_generate(&p, kit, &g);
            assert(p.hit[0][0] == (i ? 0x1111u : 1u));
            assert(p.hit[drumgen_voice(kit, DG_CLOSED)][0] == hats[i]);
            assert(p.hit[drumgen_voice(kit, DG_CRASH)][0] == 1u);
        }
        {
            drumgen_cfg_t g = {(uint32_t)seed, 0, 100};
            dpart_t p = {0}; int v;
            drumgen_generate(&p, kit, &g);
            for (v = 0; v < NDRUM; ++v) assert(p.hit[v][0]);
            assert(!(drumgen_initial_mutes(kit) & ((1u << 0) |
                (1u << drumgen_voice(kit, DG_CLOSED)) | (1u << drumgen_voice(kit, DG_OPEN)))));
        }
    }
    assert(moved > 1000);
    puts("drumgen: deterministic, nested density, independent placement, exact standards, all voices, bounds, kit routing OK");
    return 0;
}
