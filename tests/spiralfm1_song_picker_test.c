/* SPDX-License-Identifier: GPL-3.0-only */
#define SPIRALFM1_PREVIEW_ONLY 1
#include "ui_pages_test.c"
int main(void) {
 unsigned i;arr_config_t user;
 outdir="build/host";panel=PANEL_DEFAULT;layers_init();host_tracks_init();bank_resolve();palette_set(0);arr_defaults(&arrangement);user=arrangement;
 go_home();song.playing=0;transport_req=0;tap(B_SAVE);assert(ui.menu==6);
 ui.menu=0;studio_open(SC_SONG);
 tap(B_SAVE);assert(ui.menu==6);ui.menu=0;studio_open(SC_SONG);
 song_screen_draw();ppm("spiralfm1-user-song-picker");
 encs[panel.enc[EN_PRESET]]=1;frame();assert(ui.menu==6&&!sf_song_active());
 assert(!memcmp(&user,&arrangement,sizeof user));
 for(i=0;i<4;i++) {
  sf_song_sel=(uint8_t)i;tap(B_OCTUP);assert(ui.menu==7);
  assert(!sf_song_active());tap(B_OCTUP);assert(sf_song_active()&&arrangement.reserved[1]==i+1&&on_song_page());
  assert(arr_valid(&arrangement,arrangement_ready()));
  if(i==0){song_screen_draw();ppm("spiralfm1-factory-song-loaded");}
  sf_song_exit();assert(!memcmp(&user,&arrangement,sizeof user));
  encs[panel.enc[EN_PRESET]]=1;frame();assert(ui.menu==6);
 }
 ui.menu=0;song.playing=1;encs[panel.enc[EN_PRESET]]=1;frame();assert(!ui.menu);
 puts("PASS: SONG PRESETS opens four factory songs, selection needs confirmation, user order preserved; blocked while playing.");return 0;
}
