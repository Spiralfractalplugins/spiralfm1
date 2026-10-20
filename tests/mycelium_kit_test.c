/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <assert.h>
int main(void)
{
    unsigned a,b,k,l,i,block,peak;int32_t left[CTL],right[CTL],rev[CTL];
    static const uint8_t gm[16]={36,38,39,42,46,43,48,49,51,70,63,37,56,75,35,40};
    for(a=32;a<35;a++)for(b=a+1;b<35;b++) {
        assert(memcmp(&DS_KITS[a].s[3],&DS_KITS[b].s[3],sizeof(dsnd_t)));
        assert(memcmp(&DS_KITS[a].s[1],&DS_KITS[b].s[1],sizeof(dsnd_t)));
        assert(memcmp(&DS_KITS[a].s[2],&DS_KITS[b].s[2],sizeof(dsnd_t)));
    }
    host_tracks_init();song.g[G_DRLVL]=100;
    for(k=MY_DRUM_PSY;k<=MY_DRUM_HITECH;k++)for(l=0;l<16;l++) {
        memset(&drums,0,sizeof drums);drums.set=-2;TDRUM->p[P_E0]=k;drum_on(gm[l],110);peak=0;
        for(block=0;block<FS*2/CTL;block++) {
            memset(left,0,sizeof left);memset(right,0,sizeof right);memset(rev,0,sizeof rev);drums_render(left,right,rev,CTL);
            for(i=0;i<CTL;i++)if((unsigned)abs(left[i])>peak)peak=(unsigned)abs(left[i]);
        }
        assert(peak>10 && peak<32768);
    }
    puts("PASS: distinct closed hats, snares and claps in all kit pairs; 48 one-shots nonzero and below full scale");
    return 0;
}
