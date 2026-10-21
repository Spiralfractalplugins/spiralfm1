/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <assert.h>
static double tone(int amount,double hz)
{
    unsigned b,i; double energy=0;
    sf_kbf.cut=(127-amount*117/127)<<8;sf_kbf.l1=sf_kbf.l2=sf_kbf.r1=sf_kbf.r2=0;song.g[G_ROUTE]=amount;
    for(b=0;b<FS/CTL;b++) {
        for(i=0;i<CTL;i++)sf_kb_l[i]=sf_kb_r[i]=(int32_t)(8000*sin(6.283185307179586*hz*(b*CTL+i)/FS));
        sf_kb_process(CTL);
        if(b>FS/CTL/2)for(i=0;i<CTL;i++)energy+=(double)sf_kb_l[i]*sf_kb_l[i];
    }
    return energy;
}
int main(void)
{
    int32_t l[CTL],r[CTL],rev[CTL];unsigned b,i;long energy=0,dry=0;
    host_tracks_init();assert(SF_KB_RESO==25);
    assert(tone(90,8000)<tone(0,8000)*.02);
    assert(tone(90,65)>tone(0,65)*.50);
    memset(drums.v,0,sizeof(drums.v));TDRUM->p[P_E0]=SF_DRUM_FOREST;drum_on(36,100);sf_kb_active=1;
    for(b=0;b<50;b++) {
        memset(l,0,sizeof(l));memset(r,0,sizeof(r));memset(rev,0,sizeof(rev));memset(sf_kb_l,0,sizeof(sf_kb_l));memset(sf_kb_r,0,sizeof(sf_kb_r));
        drums_render(l,r,rev,CTL);
        for(i=0;i<CTL;i++){energy+=abs(sf_kb_l[i]);dry+=abs(l[i]);}
    }
    assert(energy>10000 && dry==0);sf_kb_active=0;
    puts("PASS: kick routed to shared low-end bus; lowpass rejects highs and retains bass; fixed resonance 25/127 (20% rounded)");
    return 0;
}
