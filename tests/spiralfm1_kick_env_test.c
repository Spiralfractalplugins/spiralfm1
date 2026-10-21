/* SPDX-License-Identifier: GPL-3.0-only */
#define SPIRALFM1_PREVIEW_ONLY 1
#include "ui_pages_test.c"
int main(void)
{
    unsigned i;voice_t *v=0;
    outdir="build/host";panel=PANEL_DEFAULT;layers_init();host_tracks_init();bank_resolve();palette_set(0);
    for(i=0;i<NPART;i++)host_preset(&trk[i],TRK_DEF[i][0],TRK_DEF[i][1]);
    TDRUM->p[P_E0]=SF_DRUM_FOREST;spiralfm1_factory_groove();song.sel=TRK_DRUM;go_home();tap(B_ENV);
    assert(cur_page()->graph==GR_ADSR && page_for_drum(cur_page()));
    TDRUM->p[P_ED_FX]=0;encs[panel.enc[EN_K1]]=2;frame();assert(TDRUM->p[P_ED_FX]==1 && TDRUM->p[P_ATK]>0);
    memset(drums.v,0,sizeof drums.v);TDRUM->p[P_ATK]=60;drum_on(36,100);
    for(i=0;i<NDRUM;i++)if(drums.v[i].active && drums.v[i].note==36)v=&drums.v[i];
    assert(v && v->env==0 && sf_kick_env_tick(v,CTL)>0 && v->stage==1);
    input_off(TDRUM,36);assert(v->stage==3 && !v->ph[2]);
    drum_on(42,100);for(i=0;i<NDRUM;i++)if(drums.v[i].active && drums.v[i].note==42)assert(sf_kick_env_tick(&drums.v[i],CTL)==-1);
    TDRUM->p[P_ATK]=0;TDRUM->p[P_DEC]=0;TDRUM->p[P_SUS]=0;TDRUM->p[P_REL]=0;drum_on(36,100);
    for(i=0;i<NDRUM;i++)if(drums.v[i].active && drums.v[i].note==36){unsigned j;v=&drums.v[i];sf_kick_env_tick(v,CTL);assert(v->env<(1<<23));v->ph[2]=0;for(j=0;j<128 && v->active;j++)sf_kick_env_tick(v,CTL);assert(v->stage==0 && !v->active);}
    TDRUM->p[P_ATK]=0;TDRUM->p[P_DEC]=70;TDRUM->p[P_SUS]=127;TDRUM->p[P_REL]=70;
    ui.msg_t=0;ui.force=1;frame();ppm("spiralfm1-kick-env");
    song.sel=1;go_home();tap(B_FX);song.g[G_NAME]=0;
    encs[panel.enc[EN_K4]]=1000;frame();assert(song.g[G_NAME]==127);
    for(i=1;i<NPART;i++)assert(trk[i].p[P_SLCR]==SL_STUT && trk[i].p[P_SLDEPTH]==127 && trk[i].p[P_SLRATE]==2);
    assert(trk[0].p[P_SLCR]==SL_OFF && TDRUM->p[P_SLCR]==SL_OFF);
    ui.force=1;frame();ppm("spiralfm1-alien-stutter");
    encs[panel.enc[EN_K4]]=-1000;frame();for(i=1;i<NPART;i++)assert(trk[i].p[P_SLCR]==SL_OFF && !trk[i].p[P_SLDEPTH]);
    puts("PASS: kick ENV page, attack/decay/sustain/release and note-off; hats bypass; FX 4 couples synth-only stutter and ring, 0 switches stutter off");
    return 0;
}
