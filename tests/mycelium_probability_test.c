/* SPDX-License-Identifier: GPL-3.0-only */
#define MYCELIUM_PREVIEW_ONLY 1
#include "ui_pages_test.c"
int main(void)
{
    unsigned part,i,count;step_t s={0};int16_t *vp;const param_desc_t *d;
    outdir="build/host";panel=PANEL_DEFAULT;layers_init();host_tracks_init();bank_resolve();palette_set(0);
    for(i=0;i<NPART;i++)host_preset(&trk[i],TRK_DEF[i][0],TRK_DEF[i][1]);
    TDRUM->p[P_E0]=MY_DRUM_FOREST;mycelium_factory_groove();
    s.time=ST_NOTE;s.n=1;s.note[0]=63;s.vel=90;s.rat=3;
    for(part=1;part<3;part++) {
        track_t *t=&trk[part];song.sel=part;go_home();
        d=home_param(0,&vp);assert(vp==&t->p[P_APROB] && d==&TP[P_APROB]);
        song.g[G_SWING]=19;t->p[P_APROB]=127;
        encs[panel.enc[EN_K1]]=-4;frame();assert(t->p[P_APROB]<127 && song.g[G_SWING]==19);
        t->p[P_APROB]=0;t->step[0]=s;t->seq_idx=0;t->rat_done[0]=0;
        seq_step(t,&s,1000,0);assert(!t->seq_n && my_prob_skip[part]);
        seq_ratchets(t,750,1000);assert(t->rat_done[0]==0);
        t->p[P_APROB]=127;seq_step(t,&s,1000,0);assert(t->seq_n==1 && !my_prob_skip[part]);
        seq_ratchets(t,750,1000);assert(t->rat_done[0]==3);
        t->p[P_APROB]=64;count=0;
        for(i=0;i<1024;i++) {seq_step(t,&s,1000,0);count+=t->seq_n!=0;}
        assert(count>400 && count<625);
        t->p[P_APROB]=0;seq_release(t);input_on(t,69,100);
        {unsigned live=0;for(i=0;i<NVOICE;i++)if(t->v[i].active && t->v[i].note==69 && t->v[i].gate)live++;
        assert(live>0);}input_off(t,69);
        t->p[P_APROB]=90;ui.force=1;frame();ppm(part==1?"mycelium-prob-lead":"mycelium-prob-fx");
        printf("PASS: track %u probability 0/100 endpoints, ratchet suppression, ~50%% (%u/1024), live notes and HOME knob\n",part+1,count);
    }
    trk[0].p[P_APROB]=0;seq_step(&trk[0],&s,1000,0);assert(trk[0].seq_n==1);
    song.sel=0;go_home();song.g[G_SWING]=0;tracks_edit(0,2);assert(song.g[G_SWING]>0);
    song.sel=TRK_DRUM;tracks_edit(0,2);assert(song.g[G_SWING]>0);
    puts("PASS: bass unaffected by probability; swing control retained on bass/drum overview");
    return 0;
}
