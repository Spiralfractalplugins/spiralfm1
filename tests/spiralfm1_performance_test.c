/* SPDX-License-Identifier: GPL-3.0-only */
#define SPIRALFM1_PREVIEW_ONLY 1
#include "ui_pages_test.c"
int main(void)
{
    unsigned i;step_t bass[NSTEP];dstep_t drums_saved[NSTEP];uint32_t before;
    outdir="build/host";panel=PANEL_DEFAULT;layers_init();host_tracks_init();bank_resolve();palette_set(0);
    for(i=0;i<NPART;i++)host_preset(&trk[i],TRK_DEF[i][0],TRK_DEF[i][1]);
    spiralfm1_factory_groove();go_home();
    memcpy(bass,trk[0].step,sizeof bass);memcpy(drums_saved,TDRUM->dstep,sizeof drums_saved);
    tap(B_ENV);assert(cur_fam()==FAM_ENV && !sf_fill);
    tap(B_LFO);assert(cur_fam()==FAM_LFO && sf_lead_style==0);
    go_home();song.playing=1;press(B_ENV);for(i=0;i<47;i++)frame();assert(sf_fill);
    release(B_ENV);assert(!sf_fill && !memcmp(drums_saved,TDRUM->dstep,sizeof drums_saved));
    press(B_LFO);for(i=0;i<47;i++)frame();release(B_LFO);assert(sf_lead_style==1);
    assert(!memcmp(bass,trk[0].step,sizeof bass) && !memcmp(drums_saved,TDRUM->dstep,sizeof drums_saved));
    assert(song.g[G_BPM]==156 && song.playing);
    before=sf_lead_style;song.rec=1;press(B_LFO);for(i=0;i<47;i++)frame();release(B_LFO);assert(sf_lead_style==before);song.rec=0;
    song.playing=0;press(B_ENV);for(i=0;i<47;i++)frame();release(B_ENV);assert(!sf_fill);
    ui.msg_t=0;go_home();tap(B_FX);settings.zoom=0;
    for(i=0;i<4;i++) {encs[panel.enc[EN_K1+i]]=12;frame();ui.force=1;frame();
        ppm(i==0?"spiralfm1-fx-lowpass":i==1?"spiralfm1-fx-space":i==2?"spiralfm1-fx-drive":"spiralfm1-fx-alien");
    }
    ui.menu=1;ui.menu_sel=MI_LOGO;menu_input(BT(B_OCTUP));assert(ui.menu==5);
    ui.force=1;frame();ppm("spiralfm1-logo-menu");
    encs[panel.enc[EN_PRESET]]=1;menu_input(BT(B_OCTUP));assert(ui.menu==5);
    menu_input(BT(B_OCTDN));assert(ui.menu==1);
    puts("PASS: LOGO menu opens original splash, ignores groove commands, returns via OCT-");
    puts("PASS: ENV/LFO taps preserved; ENV fill releases without drum edits; LFO hold changes only lead/FX; recording/stopped guards; four FX focus frames rendered with ZOOM off");
    return 0;
}
