/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "pattern.h"

/* Runtime memory only. No sound parameters, automation or project format changes. */
typedef struct { bpart_t phrase[NBASS]; uint8_t valid[NBASS]; } motif_bank_t;
static inline void motif_copy(bpart_t *to, const bpart_t *from)
{
    int i;
    for (i = 0; i < NSTEPS; ++i) {
        to->step[i].note = from->step[i].note;
        to->step[i].flags = from->step[i].flags;
    }
    to->len = from->len;
    to->rate = from->rate;
    to->dir = from->dir;
    to->transpose = from->transpose;
    to->gen = from->gen;
}
static inline void motif_like(motif_bank_t *m, int b, const bpart_t *p)
{
    motif_copy(&m->phrase[b], p);
    m->valid[b] = 1;
}
static inline int motif_return(const motif_bank_t *m, int b, bpart_t *p)
{
    if (!m->valid[b]) return 0;
    motif_copy(p, &m->phrase[b]);
    return 1;
}
