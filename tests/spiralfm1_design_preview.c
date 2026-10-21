/* SPDX-License-Identifier: GPL-3.0-only */
#define SPIRALFM1_PREVIEW_ONLY 1
#include "ui_pages_test.c"
int main(int argc, char **argv)
{
    unsigned i;
    outdir = argc > 1 ? argv[1] : "build/host";
    panel = PANEL_DEFAULT;
    layers_init(); host_tracks_init(); bank_resolve();
    settings.palette = 0; palette_set(0);
    host_preset(&trk[0],0,16); host_preset(&trk[1],1,21); host_preset(&trk[2],1,23);
    TDRUM->p[P_E0] = SF_DRUM_FOREST;
    spiralfm1_factory_groove();
    sloop_splash(); ppm("spiralfm1-boot");
    go_home(); ui.force = 1; frame(); ppm("spiralfm1-tracks");
    ui.menu=1; ui.force=1; frame(); ppm("spiralfm1-menu");
    ui.menu=2; ui.force=1; frame(); ppm("spiralfm1-about");
    ui.menu=1; ui.menu_sel=MI_JAM;
    trk[0].step[3].note[0]=41;
    menu_input(BT(B_OCTUP));
    assert(trk[0].step[3].note[0]==41 && ui.menu==3);
    ui.force=1; frame(); ppm("spiralfm1-grooves");
    menu_input(BT(B_OCTUP));
    assert(trk[0].step[3].note[0]==41 && ui.menu==4);
    ui.force=1; frame(); ppm("spiralfm1-confirm");
    menu_input(BT(B_OCTDN));
    assert(trk[0].step[3].note[0]==41 && ui.menu==3);
    encs[panel.enc[EN_PRESET]]=1; menu_input(0);
    assert(sf_groove_sel==2);
    song.playing=1; menu_input(BT(B_OCTUP));
    assert(ui.menu==3 && trk[0].step[3].note[0]==41);
    song.playing=0; song.solo=2; song.g[G_DRLVL]=0;
    menu_input(BT(B_OCTUP)); menu_input(BT(B_OCTUP));
    assert(!ui.menu && spiralfm1_start_ready && !project_empty());
    assert(trk[0].step[3].note[0]==36);
    assert(song.g[G_BPM]==146 && trk[0].p[P_SLEN]==32);
    assert(song.solo==0 && song.g[G_DRLVL]==100 && TDRUM->p[P_E0]==SF_DRUM_PSY);
    assert(trk[0].eng_req==0 && trk[0].preset==16);
    assert(NBANK==21);
    puts("SPIRALFM1: groove selection, replacement confirmation, cancel and playback guard PASS");
    return 0;
}
