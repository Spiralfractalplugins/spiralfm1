/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include <assert.h>
int main(int argc, char **argv)
{
    unsigned i, f, k, peak=0, clips=0;
    int32_t o[CTL*2];
    const unsigned frames = (FS*8u/CTL)*CTL;
    FILE *w;
    unsigned kind=argc>2 ? (unsigned)atoi(argv[2]) : 0;
    assert(argc>=2);
    host_tracks_init();
    host_preset(&trk[0],0,16); host_preset(&trk[1],1,21); host_preset(&trk[2],1,23);
    TDRUM->p[P_E0]=DRUM_DEFAULT_KIT; song.master_q12=2048;
    mycelium_factory_groove(); TDRUM->p[P_E0]=MY_DRUM_FOREST; song.g[G_DUCK]=28; mycelium_start_ready=1;
    if (kind) {
        host_preset(&trk[2],1,23);
        if(kind==3) host_preset(&trk[1],0,18);
        if(kind==4) {host_preset(&trk[0],0,17);host_preset(&trk[1],1,25);}
        TDRUM->p[P_E0]=kind==4 ? MY_DRUM_HITECH : MY_DRUM_PSY;
        mycelium_psy_groove(kind);
        trk[1].p[P_LEVEL]=92; trk[2].p[P_LEVEL]=88;
    }
    if(argc>3) song.solo=1u<<(unsigned)atoi(argv[3]);
    if(getenv("ALIEN"))song.g[G_NAME]=(int16_t)atoi(getenv("ALIEN"));
    if(getenv("FILL"))my_fill=(uint8_t)atoi(getenv("FILL"));
    if(getenv("KICKENV")) {
        TDRUM->p[P_ED_FX]=1;TDRUM->p[P_ATK]=atoi(getenv("KICKENV"))==2?50:0;
        TDRUM->p[P_DEC]=62;TDRUM->p[P_SUS]=0;TDRUM->p[P_REL]=50;
    }
    if(song.g[G_NAME])for(i=1;i<NPART;i++) {
        trk[i].p[P_SLCR]=SL_STUT;trk[i].p[P_SLPAT]=0;trk[i].p[P_SLRATE]=2;trk[i].p[P_SLDEPTH]=song.g[G_NAME];
    }
    if(argc>4 && !strcmp(argv[4],"fx")) {
        song.solo=0;song.g[G_DMIX]=96;song.g[G_DFDBK]=100;song.g[G_RSIZE]=100;song.g[G_ENGGO]=127;
        trk[1].p[P_DLY]=trk[2].p[P_DLY]=74;trk[1].p[P_REV]=trk[2].p[P_REV]=60;
    }
    if(argc>6){song.solo=0;mycelium_bass_pattern((unsigned)atoi(argv[6]));}
    if(argc>7){TDRUM->p[P_E0]=(int16_t)atoi(argv[7]);song.solo=1u<<TRK_DRUM;dstep_set(&TDRUM->dstep[12],3,LV_SOFT,0);dstep_set(&TDRUM->dstep[28],2,LV_SOFT,0);}
    input_on(&trk[0],36,100);
    if(getenv("MAXDIST")) {trk[0].p[P_DIST]=127;TDRUM->p[P_DIST]=127;}
    assert(song.playing);
    w=fopen(argv[1],"wb"); assert(w); wav_hdr(w,frames);
    for (f=0; f<frames; f+=CTL) {
        if(argc>5 && !strcmp(argv[5],"lp")) song.g[G_ROUTE]=(int16_t)((uint64_t)f*127/frames);
        if (f==CTL*8u) input_off(&trk[0],36);
        mix_block(o,CTL); fm1_ms=(uint32_t)((uint64_t)(f+CTL)*1000/FS);
        for (i=0;i<CTL*2u;i++) {
            k=o[i]<0 ? (unsigned)(-(int64_t)o[i]) : (unsigned)o[i];
            if(k>peak) peak=k;
            if(o[i]>32767 || o[i]<-32768) clips++;
        }
        for (i=0;i<CTL;i++) wav_put(w,o[2*i],o[2*i+1]);
    }
    fclose(w);
    printf("Forest groove: peak %u, clipped samples %u; actual host engine, not device recording\n",peak,clips);
    assert(peak>100 && clips==0);
    return 0;
}
