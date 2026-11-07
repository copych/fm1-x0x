/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "seq/director.h"
int main(void)
{
    director_plan_t p = {0}, again = {0};
    bpart_t a = {0}, b = {0}, original;
    dpart_t d = {0};
    drumgen_cfg_t cfg = {123, 30, 80};
    int s, k;
    a.len = b.len = d.len = 16;
    a.transpose = b.transpose = 24;
    a.gen.root = 9; a.gen.scale = 3;
    for (s = 0; s < NSTEPS; ++s) { a.step[s].note = (uint8_t)(33 + s % 4); a.step[s].flags = G_REST; }
    a.step[0].flags = G_NOTE | BS_ACCENT;
    a.step[4].flags = G_NOTE | BS_SLIDE;
    a.step[9].flags = G_NOTE;
    a.step[10].flags = G_NOTE;
    sm_set(d.hit[3], 5); sm_set(d.accent, 7);
    original = a;
    assert(director_prepare(&p, &a, &b, &d, 0, &cfg));
    assert(!memcmp(&a, &original, sizeof a));
    assert(director_prepare(&again, &a, &b, &d, 0, &cfg));
    assert(!memcmp(&p.response, &again.response, sizeof p.response));
    assert(!memcmp(p.support, again.support, sizeof p.support));
    assert(p.response.gen.root == a.gen.root && p.response.gen.scale == a.gen.scale);
    assert(p.context.role[0] == DIRECTOR_LEAD && p.context.role[1] == DIRECTOR_RESPONSE &&
           p.context.role[2] == DIRECTOR_SUPPORT && p.context.root == a.gen.root);
    assert(p.response.step[2].note == 45 && bstep_gate(&p.response.step[2]) == G_NOTE);
    assert(!(p.support[2] & ((1u << 9) | (1u << 10) | (1u << 11))));
    assert(bstep_gate(&p.response.step[10]) == G_REST); /* lead's own attack wins */
    assert(!(p.response.step[6].flags & BS_SLIDE)); /* no slide into silence */
    assert(sm_get(d.hit[3], 5) && sm_get(d.accent, 7));
    assert(director_unchanged(&p, &a, &b, &d));
    a.step[0].note++;
    assert(!director_unchanged(&p, &a, &b, &d)); a = original;
    b.transpose++;
    assert(!director_unchanged(&p, &a, &b, &d)); b.transpose--;
    sm_flip(d.hit[3], 5);
    assert(!director_unchanged(&p, &a, &b, &d)); sm_flip(d.hit[3], 5);
    b.len = 11; assert(!director_prepare(&p, &a, &b, &d, 0, &cfg)); b.len = 16;
    a.gen.mutate_bars = 4; assert(!director_prepare(&p, &a, &b, &d, 0, &cfg)); a.gen.mutate_bars = 0;
    assert(!director_prepare(&p, &a, &b, &d, NKIT, &cfg));
    for (s = 0; s < NSTEPS; ++s) a.step[s].flags = G_REST;
    assert(!director_prepare(&p, &a, &b, &d, 0, &cfg));
    /* Dense and extreme-pitch leads remain bounded; full zero is real silence. */
    for (s = 0; s < 16; ++s) { a.step[s].flags = G_NOTE | BS_SLIDE; a.step[s].note = 127; }
    cfg.full = 0;
    assert(director_prepare(&p, &a, &b, &d, 1, &cfg));
    for (s = 0; s < 16; ++s) assert(p.response.step[s].note <= 127);
    for (k = 0; k < 3; ++k) assert(!p.support[k]);
    puts("director: lead identity, response, shared vocabulary, deterministic support, manual-edit invalidation, bounds OK");
    return 0;
}
