/* SPDX-License-Identifier: GPL-3.0-only */
/* Included by engine.c: main composes; render validates/publishes fixed snapshots.
 * A one-level musical undo excludes sound, mix, automation and manual mutes. */
typedef struct {
    bpart_t bass[NBASS];
    dpart_t drum[NKIT];
    drumgen_cfg_t cfg[NKIT];
    uint8_t pat[LIVE_TARGETS], mask;
} perform_scene_t;
static perform_scene_t perform_before X0X_POOL, perform_after X0X_POOL, perform_stage X0X_POOL, perform_source X0X_POOL;
static uint32_t perform_hash[LIVE_TARGETS] X0X_POOL;
static uint8_t perform_valid, perform_applied, perform_action;

static void perform_capture(perform_scene_t *p, unsigned mask)
{
    int t;
    p->mask = (uint8_t)mask;
    for (t = 0; t < LIVE_TARGETS; ++t) {
        p->pat[t] = seq.ppat[t];
        if (!(mask & (1u << t))) continue;
        if (t < NKIT) { p->drum[t] = seq.pat[p->pat[t]].drum[t]; p->cfg[t] = drum_memory.cfg[t]; }
        else motif_copy(&p->bass[t - NKIT], &seq.pat[p->pat[t]].bass[t - NKIT]);
    }
}
static uint32_t perform_source_hash(int t, int p)
{
    return t < NKIT ? director_hash(&seq.pat[p].drum[t], sizeof(dpart_t)) :
        director_hash(&seq.pat[p].bass[t - NKIT], sizeof(bpart_t));
}
int engine_director_pending(void) { return director_pending != DIRECTOR_IDLE; }
int engine_perform_request(enum ePerformAction action, unsigned mask, uint32_t entropy)
{
    int t;
    if ((unsigned)action >= PERFORM_ACTIONS_COUNT || motif_request || motif_result || director_pending || director_result)
        return 0;
    if (action == PERFORM_UNDO || action == PERFORM_REDO) {
        if (!perform_valid || (action == PERFORM_UNDO ? !perform_applied : perform_applied)) return 0;
        perform_stage = action == PERFORM_UNDO ? perform_before : perform_after;
        mask = perform_stage.mask;
        for (t = 0; t < LIVE_TARGETS; ++t)
            if ((mask & (1u << t)) && seq.ppat[t] != perform_stage.pat[t]) return 0;
    } else {
        mask &= (1u << LIVE_TARGETS) - 1u;
        if (!mask) return 0;
        for (t = NKIT; t < LIVE_TARGETS; ++t)
            if ((mask & (1u << t)) && seq.pat[seq.ppat[t]].bass[t - NKIT].gen.mutate_bars) return 0;
        if (action == PERFORM_RETURN) for (t = 0; t < LIVE_TARGETS; ++t) if (mask & (1u << t)) {
            if (t < NKIT ? !drum_memory.valid[t] : !motifs.valid[t-NKIT]) return 0;
        }
        perform_capture(&perform_source, mask);
        perform_stage = perform_source;
        for (t = 0; t < LIVE_TARGETS; ++t) if (mask & (1u << t)) {
            uint32_t seed = tb3po_new_seed(entropy ^ ((uint32_t)t * 0x9e3779b9u));
            if (action == PERFORM_RETURN) {
                if (t < NKIT) { perform_stage.drum[t] = drum_memory.phrase[t]; perform_stage.cfg[t] = drum_memory.liked_cfg[t]; }
                else motif_copy(&perform_stage.bass[t-NKIT], &motifs.phrase[t-NKIT]);
            } else if (t < NKIT) {
                perform_stage.cfg[t].seed = seed;
                drumgen_generate(&perform_stage.drum[t], t, &perform_stage.cfg[t]);
                drum_initial_mix(t);
            } else {
                bpart_t *b = &perform_stage.bass[t - NKIT];
                if (action == PERFORM_NEW) { b->gen.seed = seed; tb3po_generate(b); }
                else tb3po_develop(b, &seed);
            }
        }
    }
    for (t = 0; t < LIVE_TARGETS; ++t) if (mask & (1u << t))
        perform_hash[t] = perform_source_hash(t, seq.ppat[t]);
    perform_action = (uint8_t)action;
    __asm__ volatile("" ::: "memory");
    motif_request = MOTIF_PERFORM + 1u;
    return 1;
}
static int perform_publish(void)
{
    int t;
    for (t = 0; t < LIVE_TARGETS; ++t) if (perform_stage.mask & (1u << t))
        if (seq.ppat[t] != perform_stage.pat[t] ||
            perform_hash[t] != perform_source_hash(t, seq.ppat[t])) return -2;
    if (perform_action < PERFORM_UNDO) perform_before = perform_source;
    for (t = 0; t < LIVE_TARGETS; ++t) if (perform_stage.mask & (1u << t)) {
        if (t < NKIT) {
            seq.pat[perform_stage.pat[t]].drum[t] = perform_stage.drum[t];
            drum_memory.cfg[t] = perform_stage.cfg[t];
        } else motif_copy(&seq.pat[perform_stage.pat[t]].bass[t - NKIT], &perform_stage.bass[t - NKIT]);
    }
    if (perform_action < PERFORM_UNDO) perform_after = perform_stage;
    perform_valid = 1;
    perform_applied = perform_action != PERFORM_UNDO;
    return MOTIF_PERFORM + 1;
}
