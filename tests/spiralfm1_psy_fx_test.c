/* SPDX-License-Identifier: GPL-3.0-only */
#define SPIRALFM1_PREVIEW_ONLY 1
#include "ui_pages_test.c"
int main(void)
{
    unsigned i;
    outdir="build/host";panel=PANEL_DEFAULT;layers_init();host_tracks_init();bank_resolve();palette_set(0);
    for(i=0;i<NPART;i++)host_preset(&trk[i],TRK_DEF[i][0],TRK_DEF[i][1]);
    go_home();tap(B_FX);
    assert(cur_page()==&PAGES[page_first(FAM_FX)] && cur_page()->scope==SC_GLOBAL);
    song.g[G_ROUTE]=0;song.g[G_DMIX]=0;song.g[G_DFDBK]=60;song.g[G_ENGGO]=0;
    for(i=0;i<NPART;i++){trk[i].p[P_DLY]=0;trk[i].p[P_REV]=0;}
    encs[panel.enc[EN_K1]]=2;frame();assert(song.g[G_ROUTE]>0 && song.g[G_FILT]==0);
    encs[panel.enc[EN_K2]]=2;frame();assert(song.g[G_DMIX]>0);
    assert(!trk[0].p[P_DLY] && trk[1].p[P_DLY]>0 && trk[2].p[P_DLY]>0);
    assert(trk[1].p[P_REV]>0 && trk[2].p[P_REV]>0);
    encs[panel.enc[EN_K3]]=100;frame();assert(song.g[G_DFDBK]==100);
    assert(song.g[G_ENGGO]==127);
    song.g[G_NAME]=0;encs[panel.enc[EN_K4]]=2;frame();assert(song.g[G_NAME]>0 && song.g[G_ENGGO]==127 && song.g[G_DFDBK]==100);
    encs[panel.enc[EN_K3]]=-100;frame();assert(song.g[G_DFDBK]==0 && song.g[G_ENGGO]==0);
    assert(!trk[0].p[P_REV]);
    edit_param(1,-100);assert(song.g[G_DMIX]==0 && trk[1].p[P_DLY]==0 && trk[2].p[P_REV]==0);
    ui.force=1;frame();ppm("spiralfm1-psy-fx");
    open_family(FAM_ENV);tap(B_FX);assert(cur_page()==&PAGES[page_first(FAM_FX)]);
    song.sel=TRK_DRUM;go_home();tap(B_FX);assert(page_for_drum(cur_page()));
    puts("PASS: FX tap opens PSY FX; knob 3 couples feedback/drive at both limits; knob 4 ALIEN independent; lead/FX sends and drum selection supported");
    return 0;
}
