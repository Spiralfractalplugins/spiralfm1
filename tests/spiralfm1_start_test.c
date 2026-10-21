/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <assert.h>
int main(void)
{
    unsigned i;
    host_tracks_init();
    host_preset(&trk[0], 1, 13);
    host_preset(&trk[1], 1, 14);
    host_preset(&trk[2], 1, 17);
    spiralfm1_factory_groove();
    assert(song.g[G_BPM] == 156 && !project_empty());
    for (i=0; i<32; i++) {
        assert((trk[0].step[i].time == ST_NOTE) == (i%4 != 0));
        if(i%4) assert(trk[0].step[i].note[0]==36);
        assert((dstep_mask(&TDRUM->dstep[i]) & ~((1u<<4)|1u)) == 0);
        assert(!!(dstep_mask(&TDRUM->dstep[i]) & 1u) == (i%4 == 0));
        assert(!!(dstep_mask(&TDRUM->dstep[i]) & (1u<<4)) ==
            (i%4==2));
    }
    spiralfm1_start_ready = 1;
    input_on(&trk[0], 60, 100);
    assert(song.playing && !spiralfm1_start_ready);
    input_off(&trk[0], 60);
    seq_stop();
    input_on(&trk[0], 62, 100);
    assert(!song.playing); /* deliberate STOP stays stopped */
    input_off(&trk[0], 62);
    spiralfm1_start_ready = 1; rec_wait = 1;
    spiralfm1_play_start();
    assert(!song.playing && spiralfm1_start_ready);
    rec_wait = 0;
    mclk.alive = 1; mclk.last_ms = fm1_ms; song.g[G_SYNC] = 1;
    spiralfm1_play_start();
    assert(!song.playing && spiralfm1_start_ready);
    mclk.alive = 0; song.g[G_SYNC] = 0;
    drum_input(4, LV_NORM, 0, 1);
    assert(song.playing && !spiralfm1_start_ready);
    seq_stop();
    for (i=0; i<NTRK; i++) steps_clear(&trk[i]);
    spiralfm1_start_ready = 1;
    input_on(&trk[0], 60, 100);
    assert(!song.playing); /* empty saved projects stay empty */
    puts("SPIRALFM1: pattern map, note/drum start, stop, record, external clock and empty project PASS");
    return 0;
}
