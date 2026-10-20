/* SPDX-License-Identifier: GPL-3.0-only */
#define MYCELIUM_PREVIEW_ONLY 1
#include "ui_pages_test.c"
int main(int argc,char **argv) {
    uint32_t kind,k,i,scene,notes,bars,seen,peak,clips,prev,transitions;
    uint64_t frames,f,energy;
    int32_t out[CTL*2];FILE *w;char path[512];arr_config_t user;
    outdir="build/host";panel=PANEL_DEFAULT;layers_init();host_tracks_init();bank_resolve();palette_set(0);arr_defaults(&arrangement);user=arrangement;
    go_home();my_bass_style=2;
    press(B_ARP);for(i=0;i<50;i++)frame();assert(ly_lock==LY_ROLL&&my_arp_logo);ppm("mycelium-arp-logo");
    release(B_ARP);assert(ly_lock==LY_ROLL&&my_arp_logo&&my_bass_style==2);
    frame();tap(B_ARP);assert(ly_lock==LY_PLAY&&!my_arp_logo&&my_bass_style==2);
    tap(B_ARP);assert(my_bass_style==0);
    puts("PASS ARP: long hold locks logo/roll, release retains it, second tap exits without bass change.");
    for(kind=0;kind<4;kind++) {
        song.playing=0;transport_req=0;mycelium_song_load(kind);assert(my_song_active());
        assert(arr_valid(&arrangement,arrangement_ready()));bars=0;
        for(i=0;i<16;i++)bars+=arrangement.entry[i].bars;assert(bars==240&&!arrangement.loop);
        for(scene=0;scene<4;scene++) {
            arrangement_clock.index=scene*3;mycelium_song_scene(scene);
            for(k=0;k<4;k++)assert(trk[k].p[P_SLEN]==64);
            notes=0;for(i=0;i<64;i++)notes+=trk[0].step[i].time==ST_NOTE;
            assert(scene==0||scene==3?notes==0:notes>=16);
        }
        song.playing=0;transport_req=0;mycelium_song_load(kind);studio_open(SC_SONG);tap(B_REC);assert(!saves);
        if(kind==0){ppm("mycelium-epic-song");ui.menu=6;my_song_sel=0;ui.force=1;draw_menu();ppm("mycelium-epic-menu");ui.menu=0;}
        frames=((uint64_t)FS*bars*240/song.g[G_BPM]/CTL)*CTL+FS;
        snprintf(path,sizeof path,"%s/Mycelium-1.0-%s.wav",argc>1?argv[1]:"../..",kind==0?"Forest-Odyssey":kind==1?"Psy-Ascension":kind==2?"Tekk-Reactor":"Hitech-Portal");
        w=fopen(path,"wb");assert(w);wav_hdr(w,(uint32_t)frames);peak=clips=seen=transitions=0;energy=0;prev=255;
        song.master_q12=2048;seq_start();assert(arrangement_clock.running);
        for(f=0;f<frames;f+=CTL) {
            mix_block(out,CTL);
            if(arrangement_clock.running) {seen|=1u<<arrangement_clock.index;if(prev!=arrangement_clock.index){prev=arrangement_clock.index;transitions++;}}
            for(i=0;i<CTL*2;i++) {uint32_t v=out[i]<0?(uint32_t)(-(int64_t)out[i]):(uint32_t)out[i];if(v>peak)peak=v;if(v>32767)clips++;energy+=v;}
            for(i=0;i<CTL;i++)wav_put(w,out[2*i],out[2*i+1]);
            fm1_ms=(uint32_t)(f*1000/FS);
        }
        fclose(w);assert(seen==65535u&&transitions==16&&!song.playing&&!arrangement_clock.running&&clips==0&&energy>0);
        printf("PASS %s: 240 bars, 16 section transitions, natural end, peak %u, clipped %u\n",MY_SONG_NAMES[kind],peak,clips);fflush(stdout);
        song.playing=0;transport_req=0;my_song_exit();assert(!memcmp(&user,&arrangement,sizeof user));
    }
    puts("PASS user song order preserved; factory sections never stored in user project slots.");return 0;
}
