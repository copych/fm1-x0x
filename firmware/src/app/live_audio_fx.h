/* SPDX-License-Identifier: GPL-3.0-only */
/* One stereo capture ring, fixed allocation. ISR owns all runtime. The source is
 * the real post-compressor master. 96-frame wet and seam glides avoid hard cuts. */
#define LIVE_CAPTURE 16384u
static int16_t live_capture[LIVE_CAPTURE*2u] X0X_POOL;
static struct {
    uint32_t write,filled,start,length,read;
    int8_t effect;
    uint8_t held;
    float wet,speed,tape,previous[2];
} audio_gesture;
void engine_audio_gesture(int effect)
{ if(effect>=-1 && effect<GESTURES_COUNT)cq_put(C_LIVE_FX,(uint32_t)(effect+1),0,0); }
static void live_audio_reset(void)
{
    audio_gesture.write=audio_gesture.filled=audio_gesture.read=0;
    audio_gesture.effect=-1;audio_gesture.held=0;audio_gesture.wet=0.0f;
    audio_gesture.previous[0]=audio_gesture.previous[1]=0.0f;
}
static void live_audio_command(int effect)
{
    unsigned frames;
    if(effect<0){audio_gesture.held=0;return;}
    if(effect>=GESTURES_COUNT || audio_gesture.filled<64u)return;
    frames=(unsigned)(44100.0f*30.0f/seq_tempo(&seq));
    if(effect>=GESTURE_REPEAT8)frames>>=(unsigned)(effect-GESTURE_REPEAT8);
    else frames=audio_gesture.filled;
    if(frames>audio_gesture.filled)frames=audio_gesture.filled;
    if(frames<64u)frames=64u;
    audio_gesture.length=frames;
    audio_gesture.start=(audio_gesture.write-frames)&(LIVE_CAPTURE-1u);
    audio_gesture.read=0;audio_gesture.tape=0.0f;audio_gesture.speed=1.0f;
    audio_gesture.effect=(int8_t)effect;audio_gesture.held=1;
    if(effect==GESTURE_TAPE){
        audio_gesture.start=0;
        audio_gesture.tape=(float)((audio_gesture.write-32u)&(LIVE_CAPTURE-1u));
    }
}
static void live_audio_process(float *l,float *r,uint32_t n)
{
    uint32_t j;
    for(j=0;j<n;++j){
        float in[2]={l[j],r[j]},out[2];int c;
        if(audio_gesture.effect>=0){
            unsigned pos=audio_gesture.read;
            if(audio_gesture.effect==GESTURE_TAPE) {
                pos=(unsigned)audio_gesture.tape;
                audio_gesture.tape+=audio_gesture.speed;
                if(audio_gesture.tape>=(float)LIVE_CAPTURE)audio_gesture.tape-=(float)LIVE_CAPTURE;
                audio_gesture.speed*=0.9998f;
                if(audio_gesture.speed<0.0001f)audio_gesture.speed=0.0f;
            }
            for(c=0;c<2;++c){
                float wet;
                if(audio_gesture.effect==GESTURE_CHOP)wet=(form.step&1u)?0.0f:in[c];
                else wet=(float)live_capture[(((audio_gesture.start+pos)&(LIVE_CAPTURE-1u))*2u)+(unsigned)c]*(1.0f/16384.0f);
                if(audio_gesture.effect==GESTURE_TAPE)wet*=audio_gesture.speed;
                /* Slew the processed stream across retriggers and loop seams. */
                if(audio_gesture.read<96u || audio_gesture.effect==GESTURE_CHOP)
                    wet=audio_gesture.previous[c]+(wet-audio_gesture.previous[c])*(1.0f/96.0f);
                audio_gesture.previous[c]=wet;
                out[c]=in[c]+(wet-in[c])*audio_gesture.wet;
            }
            if(audio_gesture.effect!=GESTURE_TAPE || audio_gesture.read<LIVE_CAPTURE)++audio_gesture.read;
            if(audio_gesture.effect!=GESTURE_TAPE && audio_gesture.read>=audio_gesture.length)audio_gesture.read=0;
        }else{out[0]=in[0];out[1]=in[1];}
        if(audio_gesture.held){if(audio_gesture.wet<1.0f)audio_gesture.wet+=1.0f/96.0f;if(audio_gesture.wet>1.0f)audio_gesture.wet=1.0f;}
        else if(audio_gesture.wet>0.0f){audio_gesture.wet-=1.0f/96.0f;if(audio_gesture.wet<0.0f)audio_gesture.wet=0.0f;}
        if(!audio_gesture.held && audio_gesture.wet<=0.0f)audio_gesture.effect=-1;
        if(audio_gesture.effect<0 || (audio_gesture.effect==GESTURE_TAPE && audio_gesture.read<LIVE_CAPTURE-64u)){
            for(c=0;c<2;++c){float v=in[c];if(v>1.999f)v=1.999f;if(v< -1.999f)v= -1.999f;
                live_capture[audio_gesture.write*2u+(unsigned)c]=(int16_t)(v*16384.0f);}
            audio_gesture.write=(audio_gesture.write+1u)&(LIVE_CAPTURE-1u);
            if(audio_gesture.filled<LIVE_CAPTURE)++audio_gesture.filled;
        }
        l[j]=out[0];r[j]=out[1];
    }
}
