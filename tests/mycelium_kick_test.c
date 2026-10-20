/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <assert.h>
int main(void)
{
    unsigned kit,i,n;
    for(kit=MY_DRUM_PSY;kit<=MY_DRUM_HITECH;kit++) {
        host_tracks_init(); memset(drums.v,0,sizeof(drums.v)); TDRUM->p[P_E0]=kit; drum_on(36,100);
        n=0;
        for(i=0;i<NDRUM;i++) if(drums.v[i].active) {
            const smp_zone_t *z=&SMP_ZONES[drums.v[i].s[4]];
            assert(!drums.synth[i]);
            assert((drums.v[i].s[4]==MYCELIUM_KICK_ZONE)==(kit==MY_DRUM_FOREST));
            assert(drums.v[i].s[5]==(int32_t)((pow2_q16(-3*16)>>8)*(z->rate>>8))); n++;
        }
        assert(n==1);
        {
            voice_t decoded=drums.v[0]; int peak=0,s;
            const smp_zone_t *z=&SMP_ZONES[decoded.s[4]];
            for(i=1;i<z->n;i++) {s=sample_next(z,&decoded,0);if(s<0)s=-s;if(s>peak)peak=s;}
            assert(peak>20000 && peak<30000);
        }
        drum_on(51,100);
        n=0;for(i=0;i<NDRUM;i++) if(drums.v[i].active && drums.v[i].note==51) {assert(drums.synth[i]);n++;}
        assert(n==1);
    }
    host_tracks_init();memset(drums.v,0,sizeof(drums.v));TDRUM->p[P_E0]=5;drum_on(36,100);
    n=0;for(i=0;i<NDRUM;i++) if(drums.v[i].active) {assert(drums.synth[i]);n++;}assert(n==1);
    puts("PASS: Original Mandalars sample kicks in all three Mycelium kits at -3 semitones; synth percussion and legacy kicks retained");
    return 0;
}
