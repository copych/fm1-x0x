/* SPDX-License-Identifier: GPL-3.0-only */
/* Complete LIVE surface; two 120-row transfers reuse the existing canvas.
 * Only dedicated 20/24/36 px fonts are used. Never shrink to fit. */
static const char *const LIVE_SECTIONS[SECTIONS_COUNT]={"INTRO","GROOVE","BUILD","BREAK","BRIDGE","DROP","OUTRO"};
static const char *const LIVE_SHAPES[AUTO_SHAPES_COUNT]={"MOD","GROW","FALL"};
static void live_text(int x,int y,int width,const felucca_font_t *font,const char *s,uint16_t color,int offset)
{
    char b[28]; int n=0;
    while(s[n] && n<27) { b[n]=s[n]; ++n; } b[n]=0;
    while(n && tw(font,b)>width) b[--n]=0;
    cv_text(x,y-offset,font,b,color);
}
static void draw_live_surface(void)
{
    int band,k,section=engine_section_value(0),pending=engine_section_value(1),bar=engine_section_value(2);
    int length=engine_section_value(3);
    char b[28],num[28],unit[28];
    const char *title=live.mode==LIVE_MIXER?(live.mix_kit?"MIXER 808":"MIXER 909"):live.mode==LIVE_EFFECTS?"MOMENT FX":
        live.mode==LIVE_AUTOMATION?(ui.part==PART_303A?"AUTO A":ui.part==PART_303B?"AUTO B":ui.part?"AUTO 808":"AUTO 909"):
        live.mode==LIVE_DIRECTOR?"DIRECTOR":PART_N[ui.part];
    for(band=0;band<2;++band) {
        int offset=band*120;
        cv_begin(240,120,C_BLACK);
        put_i(b,(int)seq_tempo(&seq)); live_text(6,0,48,&FONT_P20,b,C_WHITE,offset);
        live_text(70,0,99,&FONT_P20,LIVE_SECTIONS[section%SECTIONS_COUNT],C_HI,offset);
        put_i(put_s(put_i(b,bar),"/"),length); live_text(184,0,53,&FONT_P20,b,C_WHITE,offset);
        for(k=0;k<length && k<32;++k) {
            int w=228/length; if(w<2)w=2;
            cv_rect(6+k*w,27-offset,w-2,3,k==bar-1?C_WHITE:k<bar-1?C_HI:C_LINE);
        }
        live_text(6,39,156,&FONT_P24,title,part_col(),offset);
        if(live.mode==LIVE_INSTRUMENT && live.extra) live_text(160,43,76,&FONT_P20,"EXTRA",C_GRAY,offset);
        if(live.mode==LIVE_EFFECTS && live.fx_key>=0) {
            static const char *const fx[LIVE_EFFECTS_COUNT]={"LOWPASS","HIGHPASS","PUMP","DRIVE","TAPE STOP","CHOPPER","REPEAT 1/8","REPEAT 1/16","REPEAT 1/32"};
            live_text(6,73,220,&FONT_P24,fx[live.fx_key],C_AMB,offset);
            live_text(6,118,220,&FONT_P20,"HELD / RELEASE",C_WHITE,offset);
            live_text(6,159,220,&FONT_P20,"MASTER",C_GRAY,offset);
        } else for(k=0;k<4;++k) {
            int x=6+(k&1)*120,y=77+(k/2)*65,value;
            pref_t r=live_ref(ui.part,k);
            const x0x_param_t *d=pref_desc(r);
            const char *label=d?d->name:"--";
            if(live.mode==LIVE_AUTOMATION) {
                static const char *const labels[4]={"TARGET","DEPTH","BARS","SHAPE"};
                static const char *const target[4]={"CUT","RES","ENV","DEC"};
                static const char *const drum_target[4]={"TUNE","CHAR","DEC","DRIVE"};
                label=labels[k];
                if(k==0)put_s(num,(is_drum()?drum_target:target)[live.auto_parameter&3]);
                else if(k==1)put_i(num,live.auto_depth);
                else if(k==2)put_i(num,1<<live.auto_period);
                else put_s(num,LIVE_SHAPES[live.auto_shape%AUTO_SHAPES_COUNT]);
            } else if(live.mode==LIVE_DIRECTOR && k>=2) {
                label=k==2?"BARS":"NEXT";
                if(k==2)put_i(num,length); else put_s(num,pending>=0?LIVE_SECTIONS[pending] : "--");
            } else {
                if(live.mode==LIVE_MIXER && live_layer()==LIVE_MIX) label=PART_N[k];
                pref_value(r,num,unit);
                if(r.kind==R_ENG && (value=engine_auto_value(r.a,r.b,r.c))>=0)put_i(num,value);
                { int n; for(n=0;num[n];++n)if(num[n]>='a'&&num[n]<='z')num[n]-=32; }
            }
            { char l[28]; int n=0; while(label[n] && n<27) { l[n]=label[n]>='a'&&label[n]<='z'?label[n]-32:label[n]; ++n; } l[n]=0;
                live_text(x,y,110,&FONT_P20,l,C_GRAY,offset); }
            live_text(x,y+23,110,&FONT_P36,num,C_WHITE,offset);
        }
        cv_rect(6,212-offset,228,1,C_LINE);
        if((int32_t)(ui.msg_until-plat_ms())>0) live_text(6,216,228,&FONT_P20,ui.msg,C_AMB,offset);
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
