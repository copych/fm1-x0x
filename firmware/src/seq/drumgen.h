/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "pattern.h"

typedef struct { uint32_t seed; uint8_t weird, full; } drumgen_cfg_t;
/* Recognizable straight landmarks; seeded placement transforms the same groove.
 * Canonical voice order lets both kits share one composition vocabulary. */
enum eDrumRole { DG_KICK, DG_SNARE, DG_LOW, DG_MID, DG_HIGH, DG_RIM, DG_CLAP,
    DG_CLOSED, DG_OPEN, DG_CRASH, DG_METAL, DG_ROLES_COUNT };
static inline int drumgen_voice(int kit, int role)
{
    static const uint8_t map[NKIT][DG_ROLES_COUNT] = {
        {0,1,2,3,4,5,6,7,8,9,10}, {0,1,2,3,4,5,6,10,9,8,7}
    };
    return map[kit][role];
}
static inline uint32_t drumgen_initial_mutes(int kit)
{
    return ((1u << NDRUM) - 1u) & ~((1u << drumgen_voice(kit, DG_KICK)) |
        (1u << drumgen_voice(kit, DG_CLOSED)) | (1u << drumgen_voice(kit, DG_OPEN)));
}
static inline uint32_t drumgen_hash(uint32_t x)
{
    x ^= x >> 16; x *= 0x7feb352du; x ^= x >> 15; x *= 0x846ca68bu; return x ^ (x >> 16);
}
static inline int drumgen_place(int step, int role, uint32_t seed, unsigned weird)
{
    /* Open/closed hats share a permutation, preserving their musical separation. */
    unsigned group = role == DG_OPEN ? DG_CLOSED : (unsigned)role;
    uint32_t h = drumgen_hash(seed ^ (group * 0x9e3779b9u));
    int anchor = role == DG_KICK || role == DG_CRASH;
    int target = step;
    if (!(anchor && step < 2) && weird > drumgen_hash(h ^ (uint32_t)(step >> 1)) % 100u)
        target ^= 1;
    if (weird > 40u && weird - 40u > (h >> 8) % 60u) {
        unsigned shift = 1u + (h >> 16) % (anchor ? 14u : 15u);
        if (!anchor) target = (int)(((unsigned)target + shift) & 15u);
        else if (target) target = 1 + (int)(((unsigned)target - 1u + shift) % 15u);
    }
    return target;
}
static inline void drumgen_generate(dpart_t *p, int kit, const drumgen_cfg_t *g)
{
    /* 120 means absent. <=75 are exact, seed-independent straight standards. */
    static const uint8_t priority[DG_ROLES_COUNT][16] = {
        {0,120,120,120,25,120,120,120,25,120,120,120,25,120,120,120},
        {120,120,120,120,40,120,120,92,120,120,120,120,40,120,95,88},
        {120,120,120,120,120,120,120,120,120,120,120,120,92,120,80,120},
        {120,120,120,120,120,120,120,90,120,120,120,120,120,120,120,85},
        {120,120,120,120,120,120,90,120,120,120,120,120,120,95,120,120},
        {120,120,120,120,120,120,70,120,120,120,120,120,120,120,70,95},
        {120,120,120,120,55,120,120,120,120,120,120,120,55,120,120,95},
        {50,75,25,75,50,75,25,75,50,75,25,75,50,75,25,75},
        {120,120,120,120,120,120,90,120,120,120,120,120,120,120,80,120},
        {0,120,120,120,120,120,120,120,95,120,120,120,120,120,120,120},
        {120,120,85,120,120,120,85,120,120,120,85,120,120,120,85,95}
    };
    int role, s;
    unsigned full = g->full > 100 ? 100 : g->full, weird = g->weird > 100 ? 100 : g->weird;
    for (role = 0; role < NDRUM; ++role)
        for (s = 0; s < NSW; ++s) p->hit[role][s] = 0;
    for (s = 0; s < NSW; ++s) p->accent[s] = 0;
    p->len = 16; p->rate = RATE_16;
    for (role = 0; role < DG_ROLES_COUNT; ++role) {
        int voice = drumgen_voice(kit, role);
        for (s = 0; s < 16; ++s) {
            unsigned threshold = priority[role][s];
            int target;
            if (threshold > 75u && threshold <= 100u) {
                threshold += drumgen_hash(g->seed ^ ((uint32_t)role << 8) ^ (uint32_t)s) % 4u;
                if (threshold > 100u) threshold = 100u;
            }
            if (full < threshold) continue;
            target = drumgen_place(s, role, g->seed, weird);
            sm_set(p->hit[voice], target);
            if (!(s & 3)) sm_set(p->accent, target);
        }
    }
    /* One physical hat gesture per step; opening it replaces a closed hit. */
    p->hit[drumgen_voice(kit, DG_CLOSED)][0] &= ~p->hit[drumgen_voice(kit, DG_OPEN)][0];
}
