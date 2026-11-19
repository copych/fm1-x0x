/* SPDX-License-Identifier: GPL-3.0-only */
/* Audio-owned pending front, independent of the snare roll. */
static struct {
    volatile int8_t pending,active;
    uint8_t filter[4],pump,drive;
} momentary;
int engine_momentary_state(int pending){return pending?momentary.pending:momentary.active;}
void engine_momentary(int effect)
{if(effect>=-1 && effect<LIVE_EFFECTS_COUNT)cq_put(C_LIVE_FX,LIVE_FX_CMD_MOMENTARY,0,(uint32_t)(effect+1));}
static void momentary_release(void)
{
    if(momentary.active==LIVE_FX_LOWPASS || momentary.active==LIVE_FX_HIGHPASS){
        apply_param(T_MST,0,MST_DJF,momentary.filter[0]);
        apply_param(T_MST,0,MST_MODE,momentary.filter[1]);
        apply_param(T_MST,0,MST_CUTOFF,momentary.filter[2]);
        apply_param(T_MST,0,MST_RESO,momentary.filter[3]);
    }else if(momentary.active==LIVE_FX_PUMP)apply_param(T_MST,0,MST_PUMP,momentary.pump);
    else if(momentary.active==LIVE_FX_DRIVE)apply_param(T_FX,0,FX_DRIVE,momentary.drive);
    else if(momentary.active>=LIVE_FX_TAPE)live_audio_command(-1);
    momentary.pending=momentary.active=LIVE_FX_NONE;
}
static void momentary_command(int effect)
{
    momentary_release();
    if(effect>=0 && seq.playing && engine_live_clock_ready())momentary.pending=(int8_t)effect;
}
static void momentary_front(void)
{
    int effect=momentary.pending;
    if(effect>=0){
        momentary.pending=LIVE_FX_NONE;momentary.active=(int8_t)effect;
        if(effect==LIVE_FX_LOWPASS || effect==LIVE_FX_HIGHPASS){
            momentary.filter[0]=(uint8_t)master_get(&mst,MST_DJF);
            momentary.filter[1]=(uint8_t)master_get(&mst,MST_MODE);
            momentary.filter[2]=(uint8_t)master_get(&mst,MST_CUTOFF);
            momentary.filter[3]=(uint8_t)master_get(&mst,MST_RESO);
            apply_param(T_MST,0,MST_DJF,effect==LIVE_FX_LOWPASS?25:103);
        }else if(effect==LIVE_FX_PUMP){
            momentary.pump=(uint8_t)master_get(&mst,MST_PUMP);apply_param(T_MST,0,MST_PUMP,100);
        }else if(effect==LIVE_FX_DRIVE){
            momentary.drive=(uint8_t)fxbus_get(&fx,FX_DRIVE);apply_param(T_FX,0,FX_DRIVE,100);
        }else live_audio_command(effect-LIVE_FX_TAPE);
    }
    if(audio_gesture.held && audio_gesture.effect==GESTURE_CHOP){
        audio_gesture.chop_phase=0.0f;audio_gesture.chop_gain=1.0f;
    }
}
