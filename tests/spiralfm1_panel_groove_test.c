/* SPDX-License-Identifier: GPL-3.0-only */
#define SPIRALFM1_PREVIEW_ONLY 1
#include "ui_pages_test.c"
int main(void)
{
    unsigned i;
    panel=PANEL_DEFAULT; layers_init(); host_tracks_init(); bank_resolve();
    settings.palette=0; palette_set(0); go_home();
    press(B_HOME); frames(50); release(B_HOME);
    assert(ui.menu==1 && ui.menu_sel==MI_COLOR);
    /* The physical PRESETS encoder moves rows; K1 edits colors. */
    encs[panel.enc[EN_K1]]=1; frame();
    assert(ui.menu_sel==MI_COLOR && settings.palette==1);
    for(i=0;i<MI_JAM;i++) { encs[panel.enc[EN_PRESET]]=1; frame(); }
    assert(ui.menu_sel==MI_JAM);
    tap(B_OCTUP); assert(ui.menu==3);
    encs[panel.enc[EN_PRESET]]=-1; frame(); assert(sf_groove_sel==0);
    tap(B_OCTUP); assert(ui.menu==4);
    tap(B_OCTUP); assert(!ui.menu && spiralfm1_start_ready);
    input_on(&trk[0],36,100); assert(song.playing);
    puts("PASS: HOME hold/release, physical PRESETS row navigation, OCT+ confirmation and key start");
    return 0;
}
