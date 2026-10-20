/* SPDX-License-Identifier: GPL-3.0-only */
/* Host test of the actual DIGITAL engine. Preview uses a simple host amplitude
 * envelope: no firmware mixer, LFO, preset ADSR, effects or hardware emulation. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "felucca_tables.h"
#include "../firmware/src/core.h"
#include "../firmware/src/dsp.c"
#include "../firmware/src/eng_digital.c"
static uint32_t inc_note(int note) {
    return (uint32_t)(440.0 * pow(2.0, (note - 69) / 12.0) * 4294967296.0 / FS);
}
static void setup(track_t *t, voice_t *v, int pi) {
    memset(t, 0, sizeof *t); memset(v, 0, sizeof *v);
    for (int j=0;j<8;j++) t->p[P_E0+j]=DIGITAL_PRESETS[pi].e[j];
    v->vel=96; digital_note_on(t,v);
}
int main(int argc,char **argv) {
    const int count=sizeof DIGITAL_PRESETS/sizeof DIGITAL_PRESETS[0];
    if(count!=29) return 1;
    for(int pi=13;pi<count;pi++) {
        const preset_t *pr=&DIGITAL_PRESETS[pi];
        for(int j=0;j<8;j++)
            if(pr->e[j]<ENG_DIGITAL.edit[j].min || pr->e[j]>ENG_DIGITAL.edit[j].max) return 2;
        int peak=0;
        for(int note=24;note<=96;note+=12) for(int sweep=0;sweep<=127;sweep++) {
            track_t t; voice_t v; setup(&t,&v,pi);
            vmod_t m={0}; m.inc=inc_note(note); m.amp0=m.amp1=16384;
            m.cutoff=sweep<<8;
            for(int b=0;b<30;b++) {
                int32_t out[CTL]={0}; digital_render(&t,&v,out,CTL,&m);
                for(int j=0;j<CTL;j++) {
                    int a=abs(out[j]); if(a>12001) return 3;
                    if(a>peak) peak=a;
                }
            }
        }
        if(peak<64) return 4;
        printf("PASS %s: pitch / index sweep, peak %d\n",pr->name,peak);
    }
    if(argc<2) return 0;
    FILE *f=fopen(argv[1],"wb"); if(!f) return 5;
    uint32_t frames=FS*2*8,bytes=frames*2,v32=36+bytes;
    uint16_t v16;
    fwrite("RIFF",1,4,f); fwrite(&v32,4,1,f); fwrite("WAVEfmt ",1,8,f);
    v32=16; fwrite(&v32,4,1,f); v16=1; fwrite(&v16,2,1,f); fwrite(&v16,2,1,f);
    v32=FS; fwrite(&v32,4,1,f); v32=FS*2; fwrite(&v32,4,1,f);
    v16=2; fwrite(&v16,2,1,f); v16=16; fwrite(&v16,2,1,f);
    fwrite("data",1,4,f); fwrite(&bytes,4,1,f);
    for(int pi=13;pi<21;pi++) {
        track_t t; voice_t voice; setup(&t,&voice,pi);
        for(uint32_t pos=0;pos<FS*2;pos+=CTL) {
            uint32_t step=(uint32_t)(pos/(FS*60.0/148/4));
            uint32_t in_step=(uint32_t)fmod(pos,FS*60.0/148/4);
            if(in_step<CTL) digital_note_on(&t,&voice);
            vmod_t m={0}; m.inc=inc_note((pi==13?36:60)+(step%4==3?7:0));
            m.cutoff=(int32_t)((64+20*sin(pos*6.283185307/FS)) *256);
            double amp=(pi==17||pi==20)?0.25:0.4*exp(-in_step/(FS*0.045));
            m.amp0=m.amp1=(int32_t)(amp*32767);
            int32_t block[CTL]={0}; digital_render(&t,&voice,block,CTL,&m);
            for(uint32_t j=0;j<CTL && pos+j<FS*2;j++) {
                int16_t s=(int16_t)block[j]; fwrite(&s,2,1,f);
            }
        }
    }
    fclose(f); return 0;
}
