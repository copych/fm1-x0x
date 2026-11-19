/* SPDX-License-Identifier: GPL-3.0-only */
/* Main-loop performance gestures, included after the existing mute helpers. */
static void live_fx_release(void)
{
    int key = live.fx_key;
    if (key < 0) return;
    if(key>=LIVE_FX_TAPE) { engine_audio_gesture(-1); live.fx_key=LIVE_FX_NONE; return; }
    if (key < 2) {
        engine_set(T_MST,0,MST_DJF,proj.sound.v[T_MST][0][MST_DJF]);
        engine_set(T_MST,0,MST_MODE,proj.sound.v[T_MST][0][MST_MODE]);
        engine_set(T_MST,0,MST_CUTOFF,proj.sound.v[T_MST][0][MST_CUTOFF]);
        engine_set(T_MST,0,MST_RESO,proj.sound.v[T_MST][0][MST_RESO]);
    } else if (key == LIVE_FX_PUMP) engine_set(T_MST,0,MST_PUMP,proj.sound.v[T_MST][0][MST_PUMP]);
    else engine_set(T_FX,0,FX_DRIVE,proj.sound.v[T_FX][0][FX_DRIVE]);
    live.fx_key = LIVE_FX_NONE;
}
static void live_note_release(void)
{
    if (live.held_note >= 0) engine_bass_off(live.note_part);
    live.held_note = -1;
}
static void live_solo_clear(void)
{
    if (!live.solo_kind) return;
    seq.mute = live.solo_mute; seq.vmute = live.solo_vmute;
    live.solo_kind = LIVE_SOLO_NONE;
}
static void live_leave(void) { live_fx_release(); live_note_release(); live_solo_clear(); }
static void live_select(int part, int extend)
{
    int p;
    live_fx_release(); live_note_release();
    if (extend) {
        unsigned mask = live.selected ^ (1u << part);
        if (!mask) return;
        live.selected = (uint8_t)mask;
        if (!(mask & (1u << part))) {
            for (p = 0; p < LIVE_TARGETS; ++p) if (mask & (1u << p)) { part = p; break; }
        }
    } else live.selected = (uint8_t)(1u << part);
    ui.part = (uint8_t)part; ui.touched = -1; ui.help = 0;
    live.mode=LIVE_INSTRUMENT; live.extra=0;
    if(part<NKIT) live.mix_kit=(uint8_t)part;
    build_pages();
}
static void live_remember_mix(void)
{
    live_solo_clear();
    if (!live.restore_valid) {
        live.restore_mute = seq.mute; live.restore_vmute = seq.vmute; live.restore_valid = 1;
    }
}
static void live_solo(enum eLiveSolo kind, int target)
{
    if (live.solo_kind == kind && live.solo_target == target &&
        (kind == LIVE_SOLO_PART || live.solo_part == ui.part)) { live_solo_clear(); return; }
    live_solo_clear();
    live.solo_mute = seq.mute; live.solo_vmute = seq.vmute;
    live.solo_kind = (uint8_t)kind; live.solo_target = (uint8_t)target; live.solo_part = ui.part;
    if (kind == LIVE_SOLO_PART) {
        int p;
        for (p = 0; p < LIVE_TARGETS; ++p) if (p != target) seq.mute |= seq_part_mask(p);
    } else {
        int p;
        for (p = 0; p < LIVE_TARGETS; ++p) if (p != ui.part) seq.mute |= seq_part_mask(p);
        seq.vmute |= (((1u << NDRUM) - 1u) & ~(1u << target)) << (ui.part * NDRUM);
    }
}
static int live_key(int black, int white, int key, int down)
{
    if (black >= 0) {
        if (!down || black >= LIVE_TARGETS) return 1;
        if (live.mode==LIVE_MIXER && !(ui.btn & ((1u<<B_SEL)|(1u<<B_HOME)|(1u<<B_REC)))) {
            live_remember_mix(); mute_part(black); return 1;
        }
        if (ui.btn & (1u << B_HOME)) {
            ui.btn_used |= 1u << B_HOME; live_remember_mix(); mute_part(black);
        } else if (ui.btn & (1u << B_REC)) {
            ui.btn_used |= 1u << B_REC; live_solo(LIVE_SOLO_PART,black);
        } else {
            if (ui.btn & (1u << B_SEL)) ui.btn_used |= 1u << B_SEL;
            live_select(black,(ui.btn & (1u << B_SEL)) != 0);
        }
        return 1;
    }
    if (white < 0) return 1;
    if (!down && live.fx_key == white) { live_fx_release(); return 1; }
    if(live.mode==LIVE_MIXER) {
        if(down && white<NDRUM) {
            if(ui.btn&(1u<<B_REC)) {
                int focus=ui.part;ui.btn_used|=1u<<B_REC;ui.part=live.mix_kit;
                live_solo(LIVE_SOLO_INSTRUMENT,white);ui.part=(uint8_t)focus;
            } else {
                ui.btn_used|=ui.btn&(1u<<B_HOME);live_remember_mix();mute_track(live.mix_kit,white);
            }
        }
        return 1;
    }
    if (live.mode==LIVE_AUTOMATION && down) {
        if(!engine_live_clock_ready()){warn("USE 909 16 STEPS",0);return 1;}
        if (white<9 || white==10) {
            pref_t r=live_ref(ui.part,live.auto_parameter);
            if(white<9) {
                live.auto_shape=(uint8_t)(white/LIVE_DEPTH_PRESETS_COUNT);
                live.auto_depth=LIVE_DEPTH_PRESET[white%LIVE_DEPTH_PRESETS_COUNT];
            }
            if (r.kind==R_ENG) engine_auto_start(r.a,r.b,r.c,live.auto_shape,live.auto_depth,1<<live.auto_period);
            say("AUTOMATION",0);
        } else engine_auto_stop();
        return 1;
    }
    if (live.mode==LIVE_DIRECTOR && down) {
        if(!engine_live_clock_ready()){warn("USE 909 16 STEPS",0);return 1;}
        if (white<SECTIONS_COUNT) engine_section_queue(white);
        else if (white==7 || white==8) engine_section_resize(white==7?-1:1);
        else if (white==9) {
            if (!engine_director_request(is_drum()?ui.part:0)) warn("DIRECTOR NOT READY",0);
        }
        return 1;
    }
    if (down && white < NDRUM && is_drum() && (ui.btn & (1u << B_HOME))) {
        ui.btn_used |= 1u << B_HOME; live_remember_mix(); mute_track(ui.part,white); return 1;
    }
    if (down && white < NDRUM && is_drum() && (ui.btn & (1u << B_REC))) {
        ui.btn_used |= 1u << B_REC; live_solo(LIVE_SOLO_INSTRUMENT,white); return 1;
    }
    if (live.mode==LIVE_EFFECTS || (ui.btn & (1u << B_FX))) {
        if (!down || white >= LIVE_EFFECTS_COUNT) return 1;
        ui.btn_used |= 1u << B_FX;
        live_fx_release(); live.fx_key = (int8_t)white;
        if(white>=LIVE_FX_TAPE) engine_audio_gesture(white-LIVE_FX_TAPE);
        else if (white < 2) engine_set(T_MST,0,MST_DJF,white ? 103 : 25);
        else if (white == 2) engine_set(T_MST,0,MST_PUMP,100);
        else engine_set(T_FX,0,FX_DRIVE,100);
        return 1;
    }
    if (is_drum()) {
        if (!down || white >= NDRUM) return 1;
        if (ui.btn & (1u << B_HOME)) {
            ui.btn_used |= 1u << B_HOME; live_remember_mix(); mute_track(ui.part,white);
        } else if (ui.btn & (1u << B_REC)) {
            ui.btn_used |= 1u << B_REC; live_solo(LIVE_SOLO_INSTRUMENT,white);
        } else {
            ui.sel[ui.part] = (uint8_t)white;
            if (!seq.playing) engine_drum(ui.part,white,0.75f);
            build_pages();
        }
    } else {
        if (down && white<6) {
            static const int8_t delta[6][4]={{-8,-12,-8,0},{4,16,16,0},{10,8,14,-8},{-4,-4,0,16},{-16,-8,-12,-8},{16,0,4,8}};
            int p,i;
            for(p=NKIT;p<LIVE_TARGETS;++p) if(live.selected&(1u<<p)) for(i=0;i<4;++i) {
                pref_t r=PR(R_ENG,T_303,p-NKIT,i);int value=engine_auto_value(r.a,r.b,r.c);
                pref_set(r,(value>=0?value:pref_get(r))+delta[white][i]);
            }
            for(p=NKIT;p<LIVE_TARGETS;++p) if(live.selected&(1u<<p)) {
                pref_t r=PR(R_ENG,T_MIX,p,white==5?MX_PAN:MX_DLY);
                if(white==0 || white==3 || white==5)
                    pref_set(r,pref_get(r)+(white==0?-16:white==3?16:p==PART_303A?-8:8));
            }
            say("SOUND MACRO",0); return 1;
        }
        return 1; /* note entry remains in the dedicated editor */
        int note = 36 + key;
        if (down) {
            live.note_part = (uint8_t)bidx();
            engine_bass_on(bidx(),note,0,live.held_note >= 0);
            live.held_note = (int8_t)note;
        } else if (live.held_note == note) live_note_release();
    }
    return 1;
}
static void live_mix_cycle(int delta)
{
    int next=(int)live.mix_control+delta%LIVE_MIX_CONTROLS_COUNT;
    if(next<0)next+=LIVE_MIX_CONTROLS_COUNT;
    if(next>=LIVE_MIX_CONTROLS_COUNT)next-=LIVE_MIX_CONTROLS_COUNT;
    live.mix_control=(uint8_t)next;
}
static int live_button(int b)
{
    if (b==B_EDIT || b==B_LFO || b==B_FX || b==B_ENV) {
        live_fx_release(); live_note_release();
        if(b==B_EDIT) { if(live.mode==LIVE_INSTRUMENT)live.extra^=1; else live.extra=0; live.mode=LIVE_INSTRUMENT; }
        else if(b==B_LFO) {
            if(live.mode==LIVE_MIXER)live_mix_cycle(1);
            else {live.mode=LIVE_MIXER;live.mix_control=LIVE_MIX_LEVEL;}
        } else live.mode=b==B_FX?LIVE_EFFECTS:LIVE_AUTOMATION;
        build_pages(); return 1;
    }
    if (b == B_HOME) {
        if(ui.btn & (1u<<B_SEL)) {
            ui.btn_used|=1u<<B_SEL; set_view(V_HOME);
        } else {
            live_fx_release(); live_note_release();
            live.mode=LIVE_INSTRUMENT; live.extra=0; build_pages();
        }
        return 1;
    }
    if (b==B_SEQ || b==B_GLO) {
        if(ui.btn & (1u<<B_SEL)) {
            ui.btn_used|=1u<<B_SEL; return 0; /* deliberate entry into the classic UI */
        }
        say(b==B_SEQ?"SEL+SEQ: EDITOR":"SEL+GLO: SETUP",0);
        return 1;
    }
    if (b == B_PLAY && (ui.btn & (1u << B_HOME))) {
        ui.btn_used |= 1u << B_HOME;
        if (!engine_perform_request(PERFORM_REDO,0,0)) warn("NO MUSICAL REDO",0);
        return 1;
    }
    if (b == B_SAVE || b == B_PLAY) return 0;
    if (b == B_SEL) {
        if (ui.btn & (1u << B_HOME)) {
            ui.btn_used |= 1u << B_HOME; live_solo_clear();
            seq.mute = live.restore_valid ? live.restore_mute : 0;
            seq.vmute = live.restore_valid ? live.restore_vmute : 0;
            live.restore_valid = 0; say("MIX RESTORED",0);
        } else live.selected = (uint8_t)(1u << ui.part);
    } else if (b == B_ARP) {
        if(live.mode!=LIVE_DIRECTOR) { live_fx_release(); live.mode=LIVE_DIRECTOR; build_pages(); return 1; }
        if (engine_director_pending()) { engine_director_cancel(); say("DIRECTOR CANCEL",0); }
        else if (engine_director_request(is_drum() ? ui.part : 0)) say("DIRECTOR QUEUED",0);
        else warn("DIRECTOR NOT READY","GENERATE A / 16 STEP");
    } else if (b == B_REC) {
        if (ui.btn & (1u << B_HOME)) ui.btn_used |= 1u << B_HOME;
        if (engine_director_pending()) { engine_director_cancel(); say("DIRECTOR CANCEL",0); }
        else if (live.solo_kind) { live_solo_clear(); say("SOLO CLEARED",0); }
        else if (!engine_perform_request(PERFORM_UNDO,0,0)) warn("NO MUSICAL UNDO",0);
        else say("MUSICAL UNDO",0);
    } else if (b == B_OCTUP || b == B_OCTDN) {
        int up = b == B_OCTUP;
        if (up && (ui.btn & (1u << B_REC))) {
            ui.btn_used |= 1u << B_REC;
            if (engine_perform_request(PERFORM_REDO,0,0)) say("MUSICAL REDO",0);
            else warn("NO MUSICAL REDO",0);
        } else if ((up && (ui.btn & (1u << B_ENV))) || (!up && (ui.btn & (1u << B_LFO)))) {
            ui.btn_used |= 1u << (up ? B_ENV : B_LFO);
            if (!up) {
                if (!engine_perform_request(PERFORM_RETURN,live.selected,0)) warn("CAPTURE NOT READY",0);
                else say("RETURNING",0);
            } else if (!live.capture_mask) {
                int p;
                live.capture_mask = live.selected; live.capture_action = up ? MOTIF_LIKE : MOTIF_RETURN;
                for (p = 0; p < LIVE_TARGETS; ++p) live.capture_pat[p] = seq.ppat[p];
            } else warn("CAPTURE BUSY",0);
        } else if (engine_perform_request(up ? PERFORM_NEW : PERFORM_DEVELOP,live.selected,plat_ms() ^ ui.frame))
            say(up ? "NEW MATERIAL" : "DEVELOPING",0);
        else warn("COMPOSITION BUSY","OR AUTO MUTATE ON");
    }
    return 1;
}
static void live_capture_tick(void)
{
    int p;
    if (!live.capture_mask) return;
    for (p = 0; p < LIVE_TARGETS; ++p) if (live.capture_mask & (1u << p)) {
        int target = p < NKIT ? NBASS + p : p - NKIT;
        if (engine_motif_request((enum eMotifAction)live.capture_action,target,live.capture_pat[p]))
            live.capture_mask &= (uint8_t)~(1u << p);
        break;
    }
}
static void live_knob(int knob, int delta)
{
    int focus = ui.part, p, step = delta;
    enum eLiveLayer layer = live_layer();
    const x0x_param_t *d = pref_desc(live_ref(focus,knob));
    live_fx_release();
    if(live.mode==LIVE_AUTOMATION) {
        int value;
        uint8_t *field=knob==0?&live.auto_parameter:knob==1?&live.auto_depth:knob==2?&live.auto_period:&live.auto_shape;
        int max=knob==1?128:knob==3?AUTO_SHAPES_COUNT:4;
        value=*field+(knob==1?accel(EN_K1+knob,delta,127):delta);
        if(value<0)value=0;
        if(value>=max)value=max-1;
        *field=(uint8_t)value;
        if(knob==1) {
            pref_t r=live_ref(focus,live.auto_parameter);
            if(r.kind==R_ENG)engine_auto_depth(r.a,r.b,r.c,live.auto_depth);
        }
        return;
    }
    if(live.mode==LIVE_DIRECTOR && knob>=2) {
        if(knob==2)engine_section_resize(delta);
        else { int s=engine_section_value(1); if(s<0)s=engine_section_value(0); s+=delta>0?1:-1;
            if(s<0)s=0;
            if(s>=SECTIONS_COUNT)s=SECTIONS_COUNT-1;
            engine_section_queue(s); }
        return;
    }
    if(live.mode==LIVE_MIXER && layer==LIVE_MIX) {
        pref_t r=live_ref(focus,knob); pref_set(r,pref_get(r)+delta); return;
    }
    if(live.mode==LIVE_DIRECTOR) {
        pref_t r=PR(R_DGEN,live.mix_kit,0,knob); pref_set(r,pref_get(r)+delta); return;
    }
    if (d && !d->names && d->max >= 24) step = accel(EN_K1+knob,delta,d->max);
    else step = delta > 0 ? 1 : -1;
    for (p = 0; p < LIVE_TARGETS; ++p) {
        pref_t r;
        if (!(live.selected & (1u << p))) continue;
        if (layer == LIVE_MASTER && p != focus) continue;
        if (layer == LIVE_COMPOSE && is_drum() && p != focus) continue;
        if ((layer == LIVE_SOUND || layer == LIVE_EXTRA || layer == LIVE_COMPOSE) && (p < NKIT) != (focus < NKIT)) continue;
        ui.part = (uint8_t)p;
        r = live_ref(p,knob); cc_hand = 1;
        { int current=r.kind==R_ENG?engine_auto_value(r.a,r.b,r.c):-1;
            pref_set(r,(current>=0?current:pref_get(r))+step); }
        cc_hand = 0; motion_touch(r);
    }
    ui.part = (uint8_t)focus;
    ui.btn_used |= ui.btn & ((1u << B_EDIT)|(1u << B_ENV)|(1u << B_LFO)|(1u << B_FX)|(1u << B_SEL));
    ui.touched = (int8_t)knob; ui.touch_until = plat_ms()+900u;
}
