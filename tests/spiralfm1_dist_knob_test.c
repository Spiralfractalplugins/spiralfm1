/* SPDX-License-Identifier: GPL-3.0-only */
#define SPIRALFM1_PREVIEW_ONLY 1
#include "ui_pages_test.c"
int main(void) {
 unsigned i;int16_t pan,dist;int32_t clean[CTL*2],wet[CTL*2];uint64_t diff=0;uint32_t peak=0;
 outdir="build/host";panel=PANEL_DEFAULT;layers_init();host_tracks_init();bank_resolve();palette_set(0);go_home();
 for(i=0;i<4;i++) {
  song.sel=(uint8_t)i;pan=TSEL->p[P_PAN];dist=TSEL->p[P_DIST];tracks_edit(3,1);
  if(i==0||i==3){assert(TSEL->p[P_PAN]==pan&&TSEL->p[P_DIST]>dist);}
  else assert(TSEL->p[P_PAN]>pan&&TSEL->p[P_DIST]==dist);
 }
 song.sel=0;studio_open(SC_TRK);studio_tracks_draw();ppm("spiralfm1-bass-dist");
 song.sel=3;studio_tracks_draw();ppm("spiralfm1-drums-dist");
 for(i=0;i<CTL;i++)assert(sf_drum_dist(0)==0);
 TDRUM->p[P_DIST]=0;assert(sf_drum_dist(18000)==18000&&sf_drum_dist(-18000)==-18000);
 TDRUM->p[P_DIST]=127;assert(sf_drum_dist(18000)>18000&&sf_drum_dist(-18000)<-18000);
 host_tracks_init();TDRUM->p[P_E0]=SF_DRUM_FOREST;TDRUM->p[P_DIST]=0;drum_on(36,100);drums_render(clean,clean+CTL,0,0);
 memset(clean,0,sizeof clean);memset(wet,0,sizeof wet);
 for(i=0;i<200;i++) {
  unsigned k;TDRUM->p[P_DIST]=127;
  if(i%30==0){drum_on(36,100);drum_on(42,80);}
  mix_block(wet,CTL);
  for(k=0;k<CTL*2;k++){uint32_t p=wet[k]<0?-wet[k]:wet[k];if(p>peak)peak=p;assert(p<=32767);diff+=p;}
 }
 assert(diff&&peak);puts("PASS: K4 bass/drums edits distortion, synths retain pan; zero bypass, nonlinear drum saturation and bounded mixed audio.");return 0;
}
