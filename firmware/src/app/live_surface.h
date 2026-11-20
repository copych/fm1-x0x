/* SPDX-License-Identifier: GPL-3.0-only */
/* Complete LIVE surface; two 120-row transfers reuse the existing canvas.
 * Only dedicated 20/24/36 px fonts are used. Never shrink to fit. */
static const char *const LIVE_SECTIONS[SECTIONS_COUNT]={"INTRO","GROOVE","BUILD","BREAK","BRIDGE","DROP","OUTRO"};
static const char *const LIVE_SHAPES[AUTO_SHAPES_COUNT]={"MOD","GROW","FALL"};
static const char *const LIVE_MIX_TITLE[LIVE_MIX_CONTROLS_COUNT]={"LEVEL","PAN","REVERB","DELAY"};
static uint16_t live_bpm_background(void)
{
    static uint32_t last_beat=0xffffffffu,pulse_until;
    static uint8_t downbeat,pulse_active;
    uint32_t now=plat_ms(),step=eng_step[TRK_DRUM]&15u;
    uint32_t position=((uint32_t)seq.t[TRK_DRUM].bars<<4)|step;
    if(!seq.playing || !engine_live_clock_ready()) {last_beat=0xffffffffu;pulse_active=0;return C_BLACK;}
    if(!(step&3u) && position!=last_beat) {
        last_beat=position;pulse_until=now+90u;downbeat=(uint8_t)(step==0);pulse_active=1;
    }
    if(pulse_active && (int32_t)(pulse_until-now)<=0)pulse_active=0;
    return pulse_active ? (downbeat?C_HI:C_AMB) : C_BLACK;
}
static void live_text(int x,int y,int width,const felucca_font_t *font,const char *s,uint16_t color,int offset)
{
    char b[28]; int n=0;
    while(s[n] && n<27) { b[n]=s[n]; ++n; } b[n]=0;
    while(n && tw(font,b)>width) b[--n]=0;
    cv_text(x,y-offset,font,b,color);
}
static void live_value_right(int x,int y,int width,const felucca_font_t *font,const char *s,uint16_t color,int offset)
{
    char b[28];int n=0;
    while(s[n] && n<27){b[n]=s[n];++n;}b[n]=0;
    while(n && tw(font,b)>width)b[--n]=0;
    cv_text(x+width-tw(font,b),y-offset,font,b,color);
}
static void live_pan_slider(int x,int y,int width,int value,uint16_t color,int offset)
{
    int center=x+width/2,pos;
    if(value<0)value=0;
    if(value>127)value=127;
    /* Keep the handle inside the rail, with value 64 exactly at center. */
    pos=value<=64 ? x+3+value*(center-x-3)/64 :
        center+(value-64)*(x+width-4-center)/63;
    cv_rect(x,y-offset,width,2,C_LINE);
    cv_rect(center,y+5-offset,1,3,C_GRAY);
    cv_rect(pos-3,y-3-offset,7,8,color);
    cv_rect(pos,y-1-offset,1,4,C_BLACK);
}
static void draw_live_mix(int offset)
{
    static const char *const names[LIVE_TARGETS]={"909","808","A","B"};
    int k;
    for(k=0;k<LIVE_TARGETS;++k) {
        int x=k*60,value=pref_get(live_ref(ui.part,k)),y;
        char num[12];uint16_t color=part_muted(k)?C_DIM:PART_COL[k];
        if(value<0)value=0;
        if(value>127)value=127;
        y=107+(127-value)*63/127;
        live_text(x+3,77,38,&FONT_P20,names[k],color,offset);
        if(part_muted(k))live_text(x+42,77,15,&FONT_P20,"M",C_AMB,offset);
        if(live.mix_control==LIVE_MIX_PAN) {
            live_pan_slider(x+6,137,48,value,color,offset);
            put_i(num,value-64);
        } else {
            cv_rect(x+27,107-offset,6,64,C_LINE);
            cv_rect(x+27,y-offset,6,171-y,color);
            cv_rect(x+18,y-2-offset,24,4,color);
            put_i(num,value);
        }
        live_value_right(x+3,174,54,&FONT_P36,num,color,offset);
    }
}
static const char *const LIVE_FX_NAME[LIVE_EFFECTS_COUNT]={"LOWPASS","HIGHPASS","PUMP","DRIVE","TAPE STOP","CHOPPER","REPEAT 1/8","REPEAT 1/16","REPEAT 1/32"};
static void draw_live_fx(int offset)
{
    static const char *const gesture[4]={"FILT","PUMP","FDBK","REV"};
    static const char *const delay[4]={"TIME","FDBK","TONE","LEVEL"};
    static const char *const reverb[4]={"DECAY","TONE","LOCUT","LEVEL"};
    static const char *const comp[4]={"COMP","ATK","REL","MIX"};
    static const char *const detail[4]={"THR","RATIO","GAIN","PUMP"};
    const char *const *labels=live.fx_view==LIVE_FX_DELAY?delay:live.fx_view==LIVE_FX_REVERB?reverb:
        live.fx_view==LIVE_FX_COMP?comp:live.fx_view==LIVE_FX_COMP_DETAIL?detail:gesture;
    int k;
    for(k=0;k<4;++k) {
        pref_t r=live_ref(ui.part,k);
        const x0x_param_t *d=pref_desc(r);
        int x=k*60,value=live.fx_view==LIVE_FX_GESTURES&&k<2?
            engine_master_value(k==0?MST_DJF:MST_PUMP):pref_get(r);
        int maximum=d?d->max:127,y;char num[28],unit[28];
        live_text(x+3,77,54,&FONT_P20,labels[k],C_GRAY,offset);
        if(d && d->names){
            pref_value(r,num,unit);
            live_value_right(x+3,174,54,&FONT_P20,num,C_WHITE,offset);
            continue;
        }
        if(value<0)value=0;
        if(value>maximum)value=maximum;
        y=107+(maximum?(maximum-value)*63/maximum:63);
        cv_rect(x+27,107-offset,6,64,C_LINE);
        if(live.fx_view==LIVE_FX_GESTURES && k==0) {
            const int center=107+(127-64)*63/127;
            cv_rect(x+15,center-offset,30,1,C_GRAY);
            if(y<center)cv_rect(x+27,y-offset,6,center-y,C_HI);
            else cv_rect(x+27,center-offset,6,y-center+1,C_HI);
        } else cv_rect(x+27,y-offset,6,171-y,C_HI);
        cv_rect(x+18,y-2-offset,24,4,C_WHITE);
        put_i(num,value);
        live_value_right(x+3,174,54,&FONT_P36,num,C_WHITE,offset);
    }
}
static void draw_live_device(int offset)
{
    static const char *const bass_sound[4]={"CUT","RES","ENV","DEC"};
    static const char *const bass_extra[4]={"ACC","WAVE","DRIVE","TRANS"};
    static const char *const drum_sound[4]={"TUNE","CHAR","DEC","DRIVE"};
    static const char *const drum_extra[4]={"LEV","PAN","REV","DLY"};
    const char *const *labels=is_drum()?(live_layer()==LIVE_EXTRA?drum_extra:drum_sound):
        (live_layer()==LIVE_EXTRA?bass_extra:bass_sound);
    int k;
    for(k=0;k<4;++k){
        int x=k*60,value,applied,y;char num[28],unit[28];
        pref_t r=live_ref(ui.part,k);
        const x0x_param_t *d=pref_desc(r);
        if(!is_drum() && live_layer()==LIVE_EXTRA && k==2){
            int position=live_drive_value(ui.part);
            live_text(x+3,77,54,&FONT_P20,"DRIVE",C_GRAY,offset);
            live_text(x+3,105,54,&FONT_P20,!position?"OFF":position<64?"SOFT":"RAT",part_col(),offset);
            y=132+(127-position)*38/127;
            cv_rect(x+27,132-offset,6,39,C_LINE);
            cv_rect(x+27,y-offset,6,171-y,part_col());
            cv_rect(x+18,y-2-offset,24,4,C_WHITE);
            put_i(num,position);live_value_right(x+3,174,54,&FONT_P36,num,C_WHITE,offset);
            continue;
        }
        live_text(x+3,77,54,&FONT_P20,labels[k],C_GRAY,offset);
        if(!d){live_value_right(x+3,174,54,&FONT_P20,"--",C_DIM,offset);continue;}
        value=pref_get(r);
        if(r.kind==R_ENG && (applied=engine_auto_value(r.a,r.b,r.c))>=0)value=applied;
        if(value<0)value=0;
        if(value>d->max)value=d->max;
        if(d->names){
            int n;pref_value(r,num,unit);
            for(n=0;num[n];++n)if(num[n]>='a'&&num[n]<='z')num[n]-=32;
            live_value_right(x+3,174,54,&FONT_P20,num,C_WHITE,offset);
        }else{
            if(is_drum() && live_layer()==LIVE_EXTRA && k==1){
                live_pan_slider(x+6,137,48,value,part_col(),offset);put_i(num,value-64);
            }else{
                y=107+(d->max?((d->max-value)*63/d->max):63);
                cv_rect(x+27,107-offset,6,64,C_LINE);
                if(r.kind==R_BTRANS){
                    const int center=107+63/2;
                    cv_rect(x+15,center-offset,30,1,C_GRAY);
                    if(y<center)cv_rect(x+27,y-offset,6,center-y,part_col());
                    else cv_rect(x+27,center-offset,6,y-center+1,part_col());
                }else cv_rect(x+27,y-offset,6,171-y,part_col());
                cv_rect(x+18,y-2-offset,24,4,C_WHITE);
                if(r.kind==R_BTRANS)pref_value(r,num,unit);
                else put_i(num,value);
            }
            live_value_right(x+3,174,54,&FONT_P36,num,C_WHITE,offset);
        }
    }
}
static void draw_live_auto(int offset)
{
    static const char *const labels[4]={"DEST","DEPTH","BARS","SHAPE"};
    static const char *const bass[4]={"CUT","RES","ENV","DEC"};
    static const char *const drum[4]={"TUNE","CHAR","DEC","DRIVE"};
    int k;
    for(k=0;k<4;++k){
        int x=k*60;char num[12];
        live_text(x+3,77,54,&FONT_P20,labels[k],C_GRAY,offset);
        if(k==0)live_value_right(x+3,174,54,&FONT_P20,
            (is_drum()?drum:bass)[live.auto_parameter&3],C_WHITE,offset);
        else if(k==1){
            int depth=live.auto_depth>127?127:live.auto_depth;
            int y=107+(127-depth)*63/127;
            cv_rect(x+27,107-offset,6,64,C_LINE);
            cv_rect(x+27,y-offset,6,171-y,part_col());
            cv_rect(x+18,y-2-offset,24,4,C_WHITE);
            put_i(num,depth);
            live_value_right(x+3,174,54,&FONT_P36,num,C_WHITE,offset);
        }else if(k==2){
            put_i(num,1<<(live.auto_period&3));
            live_value_right(x+3,174,54,&FONT_P36,num,C_WHITE,offset);
        }else{
            int n,shape=live.auto_shape%AUTO_SHAPES_COUNT;
            /* Small contour above the name; fixed work and no extra control. */
            cv_rect(x+6,162-offset,48,1,C_LINE);
            for(n=0;n<24;++n){
                int h=shape==AUTO_MODULATE? (n<12?n*2:(23-n)*2):
                    shape==AUTO_GROW?n:23-n;
                cv_rect(x+6+n*2,155-h-offset,2,2,part_col());
            }
            live_value_right(x+3,174,54,&FONT_P20,LIVE_SHAPES[shape],C_WHITE,offset);
        }
    }
}
static void draw_live_surface(void)
{
    int band,k,section=engine_section_value(0),pending=engine_section_value(1),bar=engine_section_value(2);
    int length=engine_section_value(3);
    uint16_t bpm_background=live_bpm_background();
    char b[28],num[28],unit[28],instrument_title[28];
    const char *title=live.mode==LIVE_MIXER?LIVE_MIX_TITLE[live.mix_control]:live.mode==LIVE_EFFECTS?
        (live.fx_view==LIVE_FX_DELAY?"DELAY":live.fx_view==LIVE_FX_REVERB?"REVERB":
        live.fx_view==LIVE_FX_COMP?"COMPRESSOR":live.fx_view==LIVE_FX_COMP_DETAIL?"COMP DETAIL":"MOMENT FX"):
        live.mode==LIVE_AUTOMATION?(ui.part==PART_303A?"AUTO A":ui.part==PART_303B?"AUTO B":ui.part?"AUTO 808":"AUTO 909"):
        live.mode==LIVE_DIRECTOR?"DIRECTOR":PART_N[ui.part];
    if(live.mode==LIVE_EFFECTS && live.fx_view==LIVE_FX_GESTURES && live.fx_key>=0)title=LIVE_FX_NAME[live.fx_key];
    if(live.mode==LIVE_INSTRUMENT && is_drum()) {
        const char *voice=engine_voice_name(ui.part?T_808:T_909,ui.sel[ui.part]);
        char *q=put_s(instrument_title,ui.part?"808 ":"909 ");
        int n=0;
        while(voice[n] && q<instrument_title+27) {
            *q++=voice[n]>='a'&&voice[n]<='z'?voice[n]-32:voice[n];++n;
        }
        *q=0;title=instrument_title;
    }
    for(band=0;band<2;++band) {
        int offset=band*120;
        cv_begin(240,120,C_BLACK);
        cv_rect(0,-offset,60,24,bpm_background);
        put_i(b,(int)seq_tempo(&seq));
        live_value_right(6,0,48,&FONT_P20,b,bpm_background==C_BLACK?C_WHITE:C_BLACK,offset);
        live_text(70,0,99,&FONT_P20,LIVE_SECTIONS[section%SECTIONS_COUNT],C_HI,offset);
        put_i(put_s(put_i(b,bar),"/"),length); live_value_right(184,0,53,&FONT_P20,b,C_WHITE,offset);
        for(k=0;k<length && k<32;++k) {
            int w=228/length; if(w<2)w=2;
            cv_rect(6+k*w,27-offset,w-2,3,k==bar-1?C_WHITE:k<bar-1?C_HI:C_LINE);
        }
        live_text(6,39,156,&FONT_P24,title,part_col(),offset);
        if(live.mode==LIVE_INSTRUMENT && live.extra) live_text(160,43,76,&FONT_P20,"EXTRA",C_GRAY,offset);
        if(live.mode==LIVE_MIXER) {
            live_text(180,43,56,&FONT_P20,live.mix_kit?"808":"909",C_GRAY,offset);
            draw_live_mix(offset);
        } else if(live.mode==LIVE_EFFECTS) {
            live_text(164,43,72,&FONT_P20,live.roll_key>=0?(live.mix_kit?"808 SD":"909 SD"):
                "GLOBAL",C_GRAY,offset);
            draw_live_fx(offset);
        } else if(live.mode==LIVE_INSTRUMENT) {
            draw_live_device(offset);
        } else if(live.mode==LIVE_AUTOMATION) {
            draw_live_auto(offset);
        } else for(k=0;k<4;++k) {
            int x=3+k*60,y=77,value;
            pref_t r=live_ref(ui.part,k);
            const x0x_param_t *d=pref_desc(r);
            const char *label=d?d->name:"--";
            if(live.mode==LIVE_DIRECTOR && k>=2) {
                static const char *const short_section[SECTIONS_COUNT]={"INTRO","GRVE","BUILD","BREAK","BRDG","DROP","OUTRO"};
                label=k==2?"BARS":"NEXT";
                if(k==2)put_i(num,length); else put_s(num,(unsigned)pending<SECTIONS_COUNT?short_section[pending]:"--");
            } else {
                if(live.mode==LIVE_MIXER && live_layer()==LIVE_MIX) label=PART_N[k];
                pref_value(r,num,unit);
                if(r.kind==R_ENG && (value=engine_auto_value(r.a,r.b,r.c))>=0)put_i(num,value);
                { int n; for(n=0;num[n];++n)if(num[n]>='a'&&num[n]<='z')num[n]-=32; }
            }
            { char l[28]; int n=0; while(label[n] && n<27) { l[n]=label[n]>='a'&&label[n]<='z'?label[n]-32:label[n]; ++n; } l[n]=0;
                live_text(x,y,54,&FONT_P20,l,C_GRAY,offset); }
            if(live.mode==LIVE_DIRECTOR && k<2 && d){
                int raw=pref_get(r),maximum=d->max,sy;
                if(raw<0)raw=0;
                if(raw>maximum)raw=maximum;
                sy=107+(maximum?(maximum-raw)*63/maximum:63);
                cv_rect(x+24,107-offset,6,64,C_LINE);
                cv_rect(x+24,sy-offset,6,171-sy,part_col());
                cv_rect(x+15,sy-2-offset,24,4,C_WHITE);
            }
            live_value_right(x,174,54,k==3?&FONT_P20:&FONT_P36,num,C_WHITE,offset);
        }
        cv_rect(6,212-offset,228,1,C_LINE);
        if((int32_t)(ui.msg_until-plat_ms())>0) live_text(6,216,228,&FONT_P20,ui.msg,C_AMB,offset);
        else if(live.mode==LIVE_EFFECTS) {
            static const char *const mode[4]={"BYPASS","LOWPASS","BANDPASS","HIGHPASS"};
            int filter_mode=engine_master_value(MST_MODE);
            live_text(6,216,140,&FONT_P20,mode[(unsigned)filter_mode<4u?filter_mode:0],C_HI,offset);
            if(live.roll_key>=LIVE_EFFECTS_COUNT){
                static const char *const roll[SNARE_ROLL_RATES_COUNT]={"R 1/8","R 1/16","R 1/32"};
                live_text(164,216,72,&FONT_P20,roll[live.roll_key-LIVE_EFFECTS_COUNT],C_AMB,offset);
            }else live_text(164,216,72,&FONT_P20,engine_momentary_state(1)>=0?"WAIT":live.fx_key>=0?"HELD":"LIVE",C_GRAY,offset);
        }
        else if(live.mode==LIVE_AUTOMATION) {
            pref_t r=live_ref(ui.part,live.auto_parameter);
            int value=r.kind==R_ENG?engine_auto_value(r.a,r.b,r.c):-1;
            char *q=put_s(b,value>=0?"RUN ":"OFF ");
            if(value>=0)q=put_s(put_i(q,value),"  ");
            put_i(put_s(put_i(q,engine_section_value(4)%(1<<live.auto_period)+1),"/"),1<<live.auto_period);
            live_text(6,216,228,&FONT_P20,b,C_AMB,offset);
        }
        else {
            int p; static const char *const shortname[4]={"9","8","A","B"};
            for(p=0;p<LIVE_TARGETS;++p)live_text(6+p*23,216,21,&FONT_P20,shortname[p],part_muted(p)?C_DIM:PART_COL[p],offset);
            live_text(116,216,120,&FONT_P20,pending>=0?LIVE_SECTIONS[pending]:seq.playing?"PLAY":"STOP",C_HI,offset);
        }
        cv_commit(band,0,(uint32_t)offset);
    }
}
