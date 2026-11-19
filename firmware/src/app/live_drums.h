/* SPDX-License-Identifier: GPL-3.0-only */
/* Render-owned drum filter and roll scheduler; included after s_drum. */
static struct {
    float g,ic1[4],ic2[4],weight[3];
} drum_filter;
static struct {
    uint32_t remaining;
    uint8_t active,kit,rate,armed,mid_pending;
} snare_roll;
#if defined(X0X_WEB)
static uint32_t snare_test_hits;
#endif

void engine_snare_roll(int kit,int rate)
{
    if(rate<0)cq_put(C_LIVE_FX,LIVE_FX_CMD_ROLL,0,0);
    else if((unsigned)kit<NKIT && (unsigned)rate<SNARE_ROLL_RATES_COUNT)
        cq_put(C_LIVE_FX,LIVE_FX_CMD_ROLL,(uint32_t)kit,(uint32_t)rate+1u);
}
static void live_drums_reset(void)
{
    int k;
    drum_filter.g=0.0f;
    for(k=0;k<4;++k)drum_filter.ic1[k]=drum_filter.ic2[k]=0.0f;
    for(k=0;k<3;++k)drum_filter.weight[k]=0.0f;
    snare_roll.remaining=0;snare_roll.active=snare_roll.kit=snare_roll.rate=snare_roll.armed=snare_roll.mid_pending=0;
}
static void snare_command(unsigned kit,unsigned encoded)
{
    snare_roll.active=snare_roll.armed=snare_roll.mid_pending=0;snare_roll.remaining=0;
    if(!encoded || kit>=NKIT || encoded>SNARE_ROLL_RATES_COUNT ||
       !seq.playing || seq.ext || !engine_live_clock_ready())return;
    snare_roll.kit=(uint8_t)kit;snare_roll.rate=(uint8_t)(encoded-1u);
    snare_roll.armed=1;
}
static void snare_hit(void)
{
    if(!(seq.mute&seq_part_mask(snare_roll.kit))){
        s_drum(0,snare_roll.kit,1,0.8f);
#if defined(X0X_WEB)
        ++snare_test_hits;
#endif
    }
}
static void snare_front(int step)
{
    if(!snare_roll.armed)return;
    if(snare_roll.rate==SNARE_ROLL_8 && (step&1))return;
    snare_roll.active=1;snare_hit();
    snare_roll.mid_pending=(uint8_t)(snare_roll.rate==SNARE_ROLL_32);
    snare_roll.remaining=0;
}
static void snare_schedule_midpoint(void)
{
    if(snare_roll.mid_pending){
        snare_roll.mid_pending=0;
        /* fire_step schedules to_next after its callback: follow the actual
         * upcoming (possibly swung) interval, not a free-running timer. */
        snare_roll.remaining=seq.t[TRK_DRUM].to_next/2u;
    }
}
static void drum_filter_process(float *l,float *r,float *rv,float *dl,uint32_t n)
{
    float *bus[4]={l,r,rv,dl};
    int mode=master_get(&mst,MST_MODE),c,w,active=mode;
    uint32_t j;
    float g=drum_filter.g,k=mst.k,a1;
    for(w=0;w<3;++w)if(drum_filter.weight[w]>0.0f)active=1;
    if(!active){
        for(c=0;c<4;++c)drum_filter.ic1[c]=drum_filter.ic2[c]=0.0f;
        drum_filter.g=mst.g_t;return;
    }
    a1=1.0f/(1.0f+g*(g+k));
    for(j=0;j<n;++j){
        if(g!=mst.g_t){
            g+=(mst.g_t-g)*0.0625f;
            if(fm_fabsf(mst.g_t-g)<1e-6f*mst.g_t)g=mst.g_t;
            a1=1.0f/(1.0f+g*(g+k));
        }
        for(w=0;w<3;++w){
            float target=mode==w+1?1.0f:0.0f;
            float v=drum_filter.weight[w];
            if(v<target){v+=1.0f/96.0f;if(v>target)v=target;}
            else if(v>target){v-=1.0f/96.0f;if(v<target)v=target;}
            drum_filter.weight[w]=v;
        }
        for(c=0;c<4;++c){
            float s=bus[c][j],v3=s-drum_filter.ic2[c];
            float v1=a1*drum_filter.ic1[c]+g*a1*v3;
            float v2=drum_filter.ic2[c]+g*v1,hp=s-k*v1-v2;
            drum_filter.ic1[c]=fm_flush(2.0f*v1-drum_filter.ic1[c]);
            drum_filter.ic2[c]=fm_flush(2.0f*v2-drum_filter.ic2[c]);
            bus[c][j]=s+(v2-s)*drum_filter.weight[0]+(v1-s)*drum_filter.weight[1]+
                (hp-s)*drum_filter.weight[2];
        }
    }
    drum_filter.g=g;
}
