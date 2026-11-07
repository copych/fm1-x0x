/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "motif.h"
#include "drumgen.h"
#include "tb3po.h"

enum eDirectorRole { DIRECTOR_LEAD, DIRECTOR_RESPONSE, DIRECTOR_SUPPORT, DIRECTOR_ROLES_COUNT };
enum eDirectorResult { DIRECTOR_NONE, DIRECTOR_APPLIED, DIRECTOR_CANCELLED, DIRECTOR_RESULTS_COUNT };
enum eDirectorState { DIRECTOR_IDLE, DIRECTOR_QUEUED, DIRECTOR_CANCEL_REQUESTED, DIRECTOR_STATES_COUNT };
typedef struct {
    uint8_t root, scale;
    uint8_t role[NBASS + 1]; /* 303 A, 303 B, selected drum kit */
} director_context_t;
/* Symbolic composition only: no synthesis, mixer, transport or storage ownership.
 * First vocabulary: 16-step forward phrases; A leads, B answers two 16ths later.
 * Explicit requests, never autonomous replacement of a performer's material. */
typedef struct {
    director_context_t context;
    bpart_t response;
    uint32_t source_hash[NBASS + 1], support[3];
    uint8_t pattern[NBASS + 1], kit;
} director_plan_t;

/* Bounded change fingerprint; no retained copies of the source patterns. */
static inline uint32_t director_hash(const void *data, unsigned size)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t h = 2166136261u;
    unsigned i;
    for (i = 0; i < size; ++i) h = (h ^ p[i]) * 16777619u;
    return h;
}
static inline int director_supported(const bpart_t *a, const bpart_t *b, const dpart_t *d)
{
    return a->len == 16 && b->len == 16 && d->len == 16 &&
        a->rate == RATE_16 && b->rate == RATE_16 && d->rate == RATE_16 &&
        a->dir == DIR_FWD && b->dir == DIR_FWD && !a->gen.mutate_bars && !b->gen.mutate_bars;
}
static inline int director_prepare(director_plan_t *p, const bpart_t *a,
                                   const bpart_t *b, const dpart_t *d,
                                   int kit, const drumgen_cfg_t *cfg)
{
    int s, any = 0, hat = kit ? 10 : 7;
    dpart_t generated;
    if ((unsigned)kit >= NKIT || !director_supported(a, b, d)) return 0;
    for (s = 0; s < 16; ++s) any |= bstep_gate(&a->step[s]) == G_NOTE;
    if (!any) return 0;
    p->context.root = a->gen.root; p->context.scale = a->gen.scale;
    p->context.role[0] = DIRECTOR_LEAD; p->context.role[1] = DIRECTOR_RESPONSE;
    p->context.role[2] = DIRECTOR_SUPPORT;
    p->source_hash[0] = director_hash(a, sizeof *a);
    p->source_hash[1] = director_hash(b, sizeof *b);
    p->source_hash[2] = director_hash(d, sizeof *d);
    p->kit = (uint8_t)kit;
    tb3po_response(&p->response, a);
    drumgen_generate(&generated, kit, cfg);
    p->support[0] = generated.hit[0][0];
    p->support[1] = generated.hit[1][0];
    p->support[2] = generated.hit[hat][0];
    for (s = 0; s < 16; ++s)
        if ((s & 3) && (bstep_gate(&a->step[s]) == G_NOTE ||
             bstep_gate(&p->response.step[s]) == G_NOTE))
            p->support[2] &= ~(1u << (s & 31));
    return 1;
}
/* Cancel stale plans when the performer changes their source material. */
static inline int director_unchanged(const director_plan_t *p, const bpart_t *a,
                                    const bpart_t *b, const dpart_t *d)
{
    return p->source_hash[0] == director_hash(a, sizeof *a) &&
        p->source_hash[1] == director_hash(b, sizeof *b) &&
        p->source_hash[2] == director_hash(d, sizeof *d);
}
