/* SPDX-License-Identifier: GPL-3.0-only */
/* Render-owned snare roll scheduler; included after s_drum. */
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
