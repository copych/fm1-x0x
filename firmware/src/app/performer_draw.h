/* SPDX-License-Identifier: GPL-3.0-only */
static void draw_live(int band)
{
    int p, s, step = eng_step[TRK_DRUM] & 15;
    char text[28], *q;
    if (!band) {
        for (p = 0; p < LIVE_TARGETS; ++p) {
            int x = p * 60 + 2, selected = (live.selected & (1u << p)) != 0;
            uint16_t color = PART_COL[p];
            box(x,2,56,25,selected ? dim(color,3) : C_LINE);
            if (selected) frame(x,2,56,25,color);
            text_c(x+28,5,&FONT_S,PART_N[p],part_muted(p) ? C_DIM : color);
            { unsigned width = eng_peak[p]*48u/32768u; cv_rect(x+4,23,width > 48u ? 48 : (int)width,2,color); }
        }
        q = put_i(put_s(text,"BAR "),(seq.t[TRK_DRUM].bars & 3u)+1);
        put_s(put_s(q," / 4   "),seq.playing ? "PLAY" : "STOP");
        cv_text(4,32,&FONT_S,text,C_WHITE);
        text_r(236,34,&FONT_XS,engine_director_pending() ? "DIR QUEUED" : live.solo_kind ? "SOLO" : "ARP DIRECT",C_AMB);
        for (s = 0; s < 16; ++s)
            box(4+s*14+(s/4),55,11,7,seq.playing && s == step ? C_WHITE : !(s&3) ? part_col() : C_LINE);
    } else {
        const char *name = is_drum() ? engine_voice_name(ui.part ? T_808 : T_909,ui.sel[ui.part]) :
            ui.part == PART_303A ? "LEAD" : "RESPONSE";
        cv_text(4,vc(&FONT_M,0,28),&FONT_M,name,part_col());
        if (live.fx_key >= 0) {
            static const char *const fx[4] = {"MASTER LOWPASS","MASTER HIGHPASS","MASTER PUMP","MASTER DRIVE"};
            cv_text(4,32,&FONT_XS,fx[live.fx_key],C_AMB);
        } else if (is_drum()) {
            cv_text(4,31,&FONT_XS,"WHITE: INSTRUMENT   HOME: MUTE",C_GRAY);
        } else {
            const bpart_t *b = &cur_pat()->bass[bidx()];
            put_note(text,b->step[eng_step[ui.part] % (b->len ? b->len : 1)].note);
            text_r(235,8,&FONT_M,text,C_WHITE);
            cv_text(4,32,&FONT_XS,"OCT NEW/DEVELOP   ARP DIRECTOR",C_GRAY);
        }
        if (is_drum()) for (s = 0; s < NDRUM; ++s) {
            int x = 3+s*21, muted = ((seq.mute|seq.vmute) >> (ui.part*NDRUM+s)) & 1u;
            box(x,47,19,17,ui.sel[ui.part] == s ? dim(part_col(),3) : C_LINE);
            put_i(text,s+1);
            text_c(x+9,48,&FONT_XS,text,muted ? C_DIM : part_col());
            if (ui.sel[ui.part] == s) frame(x,47,19,17,part_col());
            if (eng_hit[ui.part][s] > 32) cv_rect(x+4,62,10,2,C_WHITE);
        } else {
            const char *scale = TB3PO_SCALE_NAMES[cur_pat()->bass[bidx()].gen.scale % TB3PO_NSCALES];
            q = put_s(put_s(put_s(text,"KEY "),NOTE_N[cur_pat()->bass[bidx()].gen.root % 12])," ");
            put_s(q,scale);
            cv_text(4,49,&FONT_S,text,C_HI);
            text_r(236,51,&FONT_XS,live.selected == ((1u<<PART_303A)|(1u<<PART_303B)) ? "A+B LINKED" : "LIVE",part_col());
        }
    }
}
