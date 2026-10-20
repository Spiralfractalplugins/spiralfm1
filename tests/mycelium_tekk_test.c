/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <assert.h>
int main(void)
{
    unsigned i,kicks=0,bass=0,claps=0;
    host_tracks_init();mycelium_psy_groove(5);
    assert(song.g[G_BPM]==170 && trk[0].p[P_DIST]==22);
    for(i=0;i<32;i++) {
        kicks+=!!(dstep_mask(&TDRUM->dstep[i])&1);claps+=!!(dstep_mask(&TDRUM->dstep[i])&(1<<3));
        bass+=trk[0].step[i].time==ST_NOTE;
        assert((trk[0].step[i].time==ST_NOTE)==(i%4==2));
    }
    assert(kicks==9 && bass==8 && claps==2);
    puts("PASS: TEKK 170 BPM, offbeat bass, four-on-floor kick with end fill, sparse clap and metallic figures");
    return 0;
}
