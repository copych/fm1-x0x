/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "pattern.h"

typedef struct { uint32_t seed; uint8_t weird, full; } drumgen_cfg_t;
/* A seeded priority field, not a fresh random draw on each knob movement.
 * Adjacent-pair swaps are bijections: Weird changes placement, never hit count.
 * Full admits nested subsets: silence is explicit at zero. */
static inline uint32_t drumgen_hash(uint32_t x)
{
    x ^= x >> 16; x *= 0x7feb352du; x ^= x >> 15; x *= 0x846ca68bu; return x ^ (x >> 16);
}
static inline void drumgen_generate(dpart_t *p, int kit, const drumgen_cfg_t *g)
{
    static const uint8_t priority[3][16] = {
        {1, 120, 85, 120, 40, 120, 75, 95, 20, 120, 65, 120, 50, 120, 90, 100},
        {120, 120, 95, 120, 15, 120, 90, 120, 120, 120, 80, 120, 30, 120, 100, 120},
        {60, 90, 1, 80, 55, 85, 10, 75, 65, 95, 20, 85, 50, 80, 30, 70}
    };
    int v, s;
    unsigned full = g->full > 100 ? 100 : g->full, weird = g->weird > 100 ? 100 : g->weird;
    for (v = 0; v < NDRUM; ++v)
        for (s = 0; s < NSW; ++s) p->hit[v][s] = 0;
    for (s = 0; s < NSW; ++s) p->accent[s] = 0;
    p->len = 16; p->rate = RATE_16;
    for (v = 0; v < 3; ++v) {
        int voice = v == 2 ? (kit ? 10 : 7) : v;
        for (s = 0; s < 16; ++s) {
            uint32_t h = drumgen_hash(g->seed ^ ((uint32_t)v * 0x9e3779b9u) ^ (uint32_t)(s >> 1));
            unsigned threshold = priority[v][s];
            int target = s;
            if (threshold > 1 && threshold <= 100) {
                unsigned jitter = drumgen_hash(h ^ (uint32_t)s) % 6u;
                threshold = threshold + jitter > 100 ? 100 : threshold + jitter;
            }
            if (!full || full < threshold) continue;
            if (!(v == 0 && s < 2) && weird > h % 100u) target ^= 1;
            sm_set(p->hit[voice], target);
            if (!(s & 3)) sm_set(p->accent, target);
        }
    }
}
