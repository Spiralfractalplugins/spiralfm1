/* SPDX-License-Identifier: GPL-3.0-only */
#define SPIRALFM1_PREVIEW_ONLY 1
#include "ui_pages_test.c"
int main(void)
{
    step_t lead[NSTEP],fxnotes[NSTEP],before[NSTEP];dstep_t drums_before[NSTEP];unsigned style,i,count;
    panel=PANEL_DEFAULT;layers_init();host_tracks_init();spiralfm1_factory_groove();go_home();
    memcpy(lead,trk[1].step,sizeof lead);memcpy(fxnotes,trk[2].step,sizeof fxnotes);memcpy(drums_before,TDRUM->dstep,sizeof drums_before);
    song.g[G_ROUTE]=73;song.playing=1;
    for(style=0;style<3;style++) {
        tap(B_ARP);assert(sf_bass_style==style);count=0;
        for(i=0;i<32;i++) {
            int on=style==0?i%4>=2:style==1?i%4==2:i%4!=0;
            assert((trk[0].step[i].time==ST_NOTE)==on);
            if(on){count++;assert(trk[0].step[i].note[0]==36);}
        }
        assert(count==(style==0?16:style==1?8:24));
        assert(!memcmp(lead,trk[1].step,sizeof lead));assert(!memcmp(fxnotes,trk[2].step,sizeof fxnotes));
        assert(!memcmp(drums_before,TDRUM->dstep,sizeof drums_before));assert(song.playing && song.g[G_BPM]==156 && song.g[G_ROUTE]==73);
    }
    song.playing=0;song.rec=1;memcpy(before,trk[0].step,sizeof before);tap(B_ARP);assert(!memcmp(before,trk[0].step,sizeof before));
    puts("PASS: physical ARP tap cycles gallop/offbeat/full-on live; other patterns, tempo and lowpass retained; recording protected");
    return 0;
}
