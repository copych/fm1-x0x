/* SPDX-License-Identifier: GPL-3.0-only */
/* Audio-owned performance clock and eight bounded automation lanes. Main sends
 * complete commands through the existing SPSC queue; no shared staging/waits. */
#define LIVE_AUTO_LANES 8
static volatile struct {
    uint8_t active, t, v, i, shape, depth, bars, base, value, max, amount;
} live_auto[LIVE_AUTO_LANES] X0X_POOL;
static struct {
    volatile uint8_t section, bar, length;
    volatile int8_t pending;
    uint8_t started, was_playing;
    volatile uint32_t step;
    uint32_t elapsed;
} form;
int engine_live_clock_ready(void)
{
    return seq.pat && !seq.song_on && seq.chain_a==seq.chain_b &&
        seq.pat[seq.ppat[TRK_DRUM]].drum[0].len==16 &&
        seq.pat[seq.ppat[TRK_DRUM]].drum[0].rate==RATE_16;
}
void engine_live_reset(void) { cq_put(C_LIVE_FORM,2,0,0); cq_put(C_LIVE_FX,LIVE_FX_CMD_RESET,0,0); }

void engine_auto_start(int t, int v, int i, int shape, int depth, int bars)
{
    if (!engine_live_clock_ready() || (unsigned)t >= NTARGETS || (unsigned)v >= NVOICES_MAX || (unsigned)i >= NPARAMS_MAX ||
        (unsigned)shape >= AUTO_SHAPES_COUNT || (unsigned)depth > 127u ||
        (bars != 1 && bars != 2 && bars != 4 && bars != 8)) return;
    cq_put(C_LIVE_AUTO,(unsigned)t | ((unsigned)shape<<3),
           (unsigned)v | ((unsigned)i<<4),(unsigned)bars | ((unsigned)depth<<4));
}
void engine_auto_depth(int t, int v, int i, int depth)
{
    if((unsigned)t>=NTARGETS || (unsigned)v>=NVOICES_MAX || (unsigned)i>=NPARAMS_MAX || (unsigned)depth>127u)return;
    cq_put(C_LIVE_AUTO,128u|(unsigned)t,(unsigned)v|((unsigned)i<<4),(unsigned)depth);
}
void engine_auto_stop(void) { cq_put(C_LIVE_AUTO,255,0,0); }
void engine_section_queue(int section)
{ if (engine_live_clock_ready() && (unsigned)section < SECTIONS_COUNT) cq_put(C_LIVE_FORM,0,(unsigned)section,0); }
void engine_section_resize(int delta) { cq_put(C_LIVE_FORM,1,delta > 0 ? 1 : 0,0); }
int engine_section_value(int field)
{
    if (field == 0) return form.section;
    if (field == 1) return form.pending;
    if (field == 2) return form.bar+1;
    if (field == 4) return (int)(form.step/16u);
    return form.length;
}
int engine_auto_value(int t, int v, int i)
{
    int k;
    for (k=0;k<LIVE_AUTO_LANES;++k)
        if (live_auto[k].active && live_auto[k].t==t && live_auto[k].v==v && live_auto[k].i==i)
            return live_auto[k].value;
    return -1;
}
static void live_auto_cancel(int t, int v, int i, int restore)
{
    int k;
    for (k=0;k<LIVE_AUTO_LANES;++k) if (live_auto[k].active &&
        (t<0 || (live_auto[k].t==t && live_auto[k].v==v && live_auto[k].i==i))) {
        if (restore && mot_base_snd) apply_param(live_auto[k].t,live_auto[k].v,live_auto[k].i,
            mot_base_snd->v[live_auto[k].t][live_auto[k].v][live_auto[k].i]);
        live_auto[k].active=0;
    }
}
static void live_runtime_command(unsigned a, unsigned b, unsigned c, int form_command)
{
    int k, slot=-1;
    if (form_command) {
        if(a==2) {
            live_auto_cancel(-1,0,0,1);
            form.section=SECTION_GROOVE;form.pending=-1;form.length=4;
            form.bar=form.started=form.was_playing=0;form.step=form.elapsed=0;return;
        }
        if (!a) form.pending = form.pending==(int)b ? -1 : (int8_t)b;
        else {
            int len=form.length+(b?1:-1);
            if (len<=form.bar) len=form.bar+1;
            if (len>32) len=32;
            form.length=(uint8_t)len;
        }
        return;
    }
    if (a==255) { live_auto_cancel(-1,0,0,1); return; }
    {
        int t=a&7, v=b&15, i=b>>4, shape=(a>>3)&3, depth=(c>>4)&127;
        if(a&128u) {
            for(k=0;k<LIVE_AUTO_LANES;++k)
                if(live_auto[k].active && live_auto[k].t==t && live_auto[k].v==v && live_auto[k].i==i) {
                    live_auto[k].depth=(uint8_t)c;
                    live_auto[k].amount=(uint8_t)((live_auto[k].max*c+63u)/127u);
                }
            return;
        }
        const x0x_param_t *d=engine_param(t,v,i);
        if (!d || d->names || !mot_base_snd || shape>=AUTO_SHAPES_COUNT) return;
        for (k=0;k<LIVE_AUTO_LANES;++k) {
            if (live_auto[k].active && live_auto[k].t==t && live_auto[k].v==v && live_auto[k].i==i) { slot=k; break; }
            if (!live_auto[k].active && slot<0) slot=k;
        }
        if (slot<0) return; /* bounded: leave existing gestures intact when full */
        { int base=live_auto[slot].active?live_auto[slot].value:mot_base_snd->v[t][v][i];
          live_auto[slot].base=(uint8_t)base; }
        live_auto[slot].active=0;
        live_auto[slot].t=(uint8_t)t; live_auto[slot].v=(uint8_t)v; live_auto[slot].i=(uint8_t)i;
        live_auto[slot].shape=(uint8_t)shape; live_auto[slot].depth=(uint8_t)depth;
        live_auto[slot].bars=(uint8_t)(c&15u); live_auto[slot].max=d->max;
        live_auto[slot].amount=(uint8_t)((d->max*depth+63)/127);
        live_auto[slot].value=live_auto[slot].base; live_auto[slot].active=1;
        apply_param(t,v,i,live_auto[slot].base);
    }
}
static int live_runtime_step(int p)
{
    int boundary=0;
    if(!engine_live_clock_ready())return 0;
    if (!form.started) { form.started=1; form.step=0; form.bar=0; boundary=1; }
    else {
        ++form.step;
        if (!p) {
            if (form.bar+1>=form.length) {
                form.bar=0; boundary=1;
            } else ++form.bar;
        }
    }
    if(boundary && form.pending>=0) {
        int k;
        form.section=(uint8_t)form.pending;form.pending=-1;form.step=0;form.length=4;
        for(k=0;k<NBASS;++k) {
            int suppress=(form.section==SECTION_BREAK || form.section==SECTION_BRIDGE)?k==0:
                (form.section==SECTION_INTRO || form.section==SECTION_OUTRO)?k==1:0;
            if(suppress)bass303_note_off(&b303[k]);
        }
    }
    form.elapsed=0;
    return boundary;
}
static void live_runtime_tick(uint32_t frames)
{
    int k;
    uint32_t len=seq.t[TRK_DRUM].step_len>>8, frac;
    if(!engine_live_clock_ready()){live_auto_cancel(-1,0,0,1);form.pending=-1;return;}
    if (!seq.playing) {
        if (form.was_playing) live_auto_cancel(-1,0,0,1);
        form.was_playing=0; form.started=0; return;
    }
    form.was_playing=1;
    frac=len ? form.elapsed*256u/len : 0; if(frac>255u)frac=255u;
    for(k=0;k<LIVE_AUTO_LANES;++k) if(live_auto[k].active) {
        unsigned n=live_auto[k].bars*16u, phase=((form.step&(n-1u))*256u+frac)*256u/n;
        int amount=live_auto[k].amount, value;
        /* phase Q16; triangle starts at the manual value and returns there. */
        int wave=live_auto[k].shape==AUTO_MODULATE ?
            (phase<16384u ? (int)phase*4 : phase<49152u ? 131072-(int)phase*4 : (int)phase*4-262144) :
            live_auto[k].shape==AUTO_GROW ? (int)phase : -(int)phase;
        value=live_auto[k].base+(wave*amount)/65536;
        if(value<0)value=0;
        if(value>live_auto[k].max)value=live_auto[k].max;
        if(value!=live_auto[k].value) {
            live_auto[k].value=(uint8_t)value;
            apply_param(live_auto[k].t,live_auto[k].v,live_auto[k].i,value);
        }
    }
    if(form.elapsed<len) {
        form.elapsed+=frames;
        if(form.elapsed>len)form.elapsed=len;
    }
}
static int live_section_allows(int part, int voice)
{
    int s=form.section;
    if (s==SECTION_INTRO || s==SECTION_OUTRO)
        return part==PART_303A || (part<NKIT && (voice==0 || (part ? voice==9||voice==10 : voice==7||voice==8)));
    if (s==SECTION_BREAK) return part==PART_303B || (part<NKIT && voice!=0);
    if (s==SECTION_BRIDGE) return part!=PART_303A;
    return 1; /* manual mute/solo still applies in the sequencer before these gates */
}
