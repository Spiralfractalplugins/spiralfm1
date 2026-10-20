/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <assert.h>
int main(void)
{
    unsigned kind,i,notes,kicks;
    host_tracks_init();
    for(kind=1;kind<=4;kind++) {
        mycelium_psy_groove(kind); notes=kicks=0;
        for(i=0;i<32;i++) {
            unsigned want=kind==1 ? i%4!=0 : (kind==2 || kind==4) ? i%4>=2 : i%2!=0;
            assert((trk[0].step[i].time==ST_NOTE)==want);
            notes+=trk[0].step[i].time==ST_NOTE;
            kicks+=!!(dstep_mask(&TDRUM->dstep[i])&1u);
        }
        assert(kicks==8 && notes==(kind==1 ? 24 : 16));
        for(i=0;i<NTRK;i++) assert(trk[i].p[P_SLEN]==32 && trk[i].p[P_SDIV]==2);
        assert(song.g[G_BPM]==(kind==4 ? 196 : kind==1 ? 148 : kind==2 ? 146 : 144));
        assert(trk[2].step[15].n==1 && trk[2].step[31].n==1);
    }
    trk[0].step[3].note[0]=41;
    mycelium_psy_groove(6); assert(trk[0].step[3].note[0]==41);
    puts("Mycelium: three distinct two-bar Psytrance grooves, common grid and bounds PASS");
    return 0;
}
