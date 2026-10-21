/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* SPIRALFM1 Studio: the firmware's DSP as a WebAssembly engine for the browser.
 *
 * One translation unit, the same sources the audio ISR runs (tests/hostsim.c is the host's
 * version of this file): engines, voices, FX, the sequencer, the drum track, PUNCH-IN.
 * Everything the page shows comes out of the firmware's own tables - track_desc, GP, TP,
 * ENGINES, PUNCH_NAME - so a knob reads exactly what the device reads.
 *
 * Built by tools/build_studio.py to web/studio/engine.wasm (no imports, memory exported).
 * The SAMPLE/GRAIN material is NOT in the .wasm: SMP_DATA is empty here and the worklet
 * copies samples.bin (the very same ADPCM blob the flash holds) into it before st_init.
 */
#include <stdint.h>
#include <stddef.h>

#define __attribute__(x)                 /* as tests/hostsim.c: the firmware's sections mean nothing here */
#include "felucca_tables.h"
#define memset felucca_memset
#define memcpy felucca_memcpy
#define memcmp felucca_memcmp
#include "../../firmware/src/libc.c"
#undef memset
#undef memcpy
#undef memcmp
/* clang still emits memset/memcpy calls for struct copies */
void *memset(void *d, int c, size_t n) { return felucca_memset(d, c, n); }
void *memcpy(void *d, const void *s, size_t n) { return felucca_memcpy(d, s, n); }
int memcmp(const void *a, const void *b, size_t n) { return felucca_memcmp(a, b, n); }

static struct { volatile uint32_t notes, buttons; } fm1_in;
static void fm1_irq_off(void) {}
static void fm1_irq_on(void) {}
#define SMP_USER_XIP(k) ((const uint8_t *)studio_blank)      /* no flash: USR1..3 read as an empty slot */
static uint32_t studio_blank[8];
#include "../../firmware/src/core.h"
#include "../../firmware/src/engines.c"
#include "../../firmware/src/drums.c"
#include "../../firmware/src/params.c"
#include "../../firmware/src/voice.c"
#include "../../firmware/src/slicer.c"
#include "../../firmware/src/fx.c"
static void fm1_delay_ms(uint32_t ms) { (void)ms; }
#include "../../firmware/src/usb.c"
#include "../../firmware/src/midi_uart.c"
#include "../../firmware/src/seq.c"
#undef __attribute__

#define EXPORT(rt, name, params) __attribute__((export_name(#name))) rt name params

static int32_t studio_out[CTL * 2];          /* one block: the worklet reads it as Int32Array */
static uint32_t studio_ready;

/* ------------------------------------------------------------ engine --- */
/* power-on as main.c felucca_init does it, without the panel: the three parts with their
 * SPIRALFM1 sounds, the drum track on the Forest kick, the factory groove */
static const uint8_t STUDIO_DEF[NPART][2] = {{0, 16}, {1, 21}, {1, 23}};   /* ui.c TRK_DEF */

static int studio_param_kept(uint32_t i)        /* ui.c param_kept: what a sound leaves alone */
{
    return i == P_LEVEL || i == P_PAN || i == P_MUTE || (i >= P_SLEN && i <= P_SGATE) ||
           (i >= P_ROOT && i <= P_QUANT) || i == P_CHORD;
}

static void studio_apply_preset(track_t *t, uint32_t pi)
{
    const engine_t *e = ENGINES[t->eng_req % NENGINES];
    static const uint8_t FX_DEF[4] = {0, 24, 28, 36};
    const preset_t *pr;
    uint32_t i;
    if (!e->npresets)
        return;
    pi %= e->npresets;
    pr = &e->presets[pi];
    t->preset = (uint8_t)pi;
    for (i = 0; i < P_E0; i++)
        if (!studio_param_kept(i))
            t->p[i] = TP[i].def;
    for (i = 0; i < 8u; i++)
        t->p[P_E0 + i] = (int16_t)pr->e[i];
    t->p[P_ATK] = pr->env[0];
    t->p[P_DEC] = pr->env[1];
    t->p[P_SUS] = pr->env[2];
    t->p[P_REL] = pr->env[3];
    t->p[P_ED_FLT] = pr->fenv;
    t->p[P_ED_FX] = preset_trim(t->eng_req % NENGINES, pi);
    t->p[P_VOICE] = pr->mono ? V_LEGATO : V_POLY;
    for (i = 0; i < 4u; i++) {
        t->p[P_DIST + i] = (int16_t)(pr->fx[i] ? pr->fx[i] - 1 : FX_DEF[i]);
        t->p[P_AMODE + i] = (int16_t)(pr->arp[i] ? pr->arp[i] - 1 : TP[P_AMODE + i].def);
    }
    preset_extras(t->p, pr);
}

static void studio_set_engine(track_t *t, uint32_t ei)
{
    const engine_t *e = ENGINES[ei % NENGINES];
    uint32_t i;
    t->eng_req = (uint8_t)(ei % NENGINES);
    t->engine = t->eng_req;
    for (i = 0; i < 8u; i++)
        t->p[P_E0 + i] = e->edit[i].def;
    studio_apply_preset(t, 0);
}

EXPORT(void, st_init, (void))
{
    uint32_t i;
    for (i = 0; i < G_COUNT; i++)
        song.g[i] = GP[i].def;
    for (i = 0; i < NTRK; i++) {
        track_t *t = &trk[i];
        uint32_t k;
        for (k = 0; k < P_E0; k++)
            t->p[k] = TP[k].def;
        steps_clear(t);
        if (i < NPART) {
            studio_set_engine(t, STUDIO_DEF[i][0]);
            studio_apply_preset(t, STUDIO_DEF[i][1]);
        }
    }
    TDRUM->p[P_E0] = SF_DRUM_FOREST;              /* the kit the device boots on */
    song.sel = 0;
    song.octave = 0;
    song.master_q12 = 2048;
    spiralfm1_factory_groove();                   /* it makes a sound before you touch anything */
    song.g[G_DUCK] = 28;
    fm1_in.notes = 0;
    kb_prev = 0;
    studio_ready = 1;
}

EXPORT(uint32_t, st_ready, (void)) { return studio_ready; }
EXPORT(uint32_t, st_rate, (void)) { return FS; }
EXPORT(uint32_t, st_block, (void)) { return CTL; }
EXPORT(uint32_t, st_smp_ptr, (void)) { return (uint32_t)(uintptr_t)SMP_DATA; }
EXPORT(uint32_t, st_smp_size, (void)) { return (uint32_t)SMP_DATA_LEN; }

/* one block of the mix: the worklet reads CTL x 2 int32 at the returned address */
EXPORT(uint32_t, st_render, (void))
{
    mix_block(studio_out, CTL);
    return (uint32_t)(uintptr_t)studio_out;
}

/* ------------------------------------------------------------ state --- */
EXPORT(uint32_t, st_playing, (void)) { return song.playing; }

EXPORT(void, st_play, (uint32_t on))
{
    transport_req = (uint8_t)(on ? 1u : 2u);
}

EXPORT(void, st_panic, (void))
{
    uint32_t i;
    panic_req = (uint8_t)((1u << NTRK) - 1u);
    for (i = 0; i < NTRK; i++)
        trk[i].held[0] = 0;
    fm1_in.notes = 0;
}

EXPORT(uint32_t, st_step, (uint32_t t)) { return trk[t % NTRK].seq_idx % trk_len(&trk[t % NTRK]); }

EXPORT(uint32_t, st_voices, (void))
{
    uint32_t i, k, n = 0;
    for (i = 0; i < NTRK; i++)
        for (k = 0; k < NVOICE; k++)
            if (trk[i].v[k].active)
                n++;
    for (k = 0; k < NDRUM; k++)
        if (drums.v[k].active)
            n++;
    return n;
}

EXPORT(uint32_t, st_punch_now, (void)) { return (uint32_t)punch.cur; }
EXPORT(uint32_t, st_punch_n, (void)) { return PUNCH_NFX; }

EXPORT(const char *, st_punch_name, (uint32_t i)) { return PUNCH_NAME[i % PUNCH_NFX]; }

EXPORT(void, st_punch, (int32_t fx))
{
    punch.req = (int8_t)(fx < -1 ? -1 : fx > (int32_t)PUNCH_NFX - 1 ? (int32_t)PUNCH_NFX - 1 : fx);
}

/* ------------------------------------------------------ tracks / kit --- */
EXPORT(uint32_t, st_select, (uint32_t t)) { song.sel = (uint8_t)(t % NTRK); return song.sel; }
EXPORT(uint32_t, st_selected, (void)) { return song.sel; }
EXPORT(uint32_t, st_is_drum, (uint32_t t)) { return is_drum(&trk[t % NTRK]); }
EXPORT(uint32_t, st_tracks, (void)) { return NTRK; }
EXPORT(uint32_t, st_solo, (uint32_t mask)) { song.solo = (uint8_t)(mask & ((1u << NTRK) - 1u)); return song.solo; }
EXPORT(uint32_t, st_solo_get, (void)) { return song.solo; }

EXPORT(uint32_t, st_engines, (void)) { return NENGINES; }

EXPORT(const char *, st_engine_name, (uint32_t e)) { return ENGINES[e % NENGINES]->name; }
EXPORT(const char *, st_engine_title, (uint32_t e)) { return N_ENGNAME[e % NENGINES]; }
EXPORT(uint32_t, st_engine_color, (uint32_t e)) { return ENGINES[e % NENGINES]->color; }
EXPORT(uint32_t, st_presets, (uint32_t e)) { return ENGINES[e % NENGINES]->npresets; }

EXPORT(const char *, st_preset_name, (uint32_t e, uint32_t p))
{
    const engine_t *en = ENGINES[e % NENGINES];
    return en->npresets ? en->presets[p % en->npresets].name : "";
}

EXPORT(uint32_t, st_engine_of, (uint32_t t)) { return trk[t % NTRK].engine % NENGINES; }
EXPORT(uint32_t, st_preset_of, (uint32_t t)) { return trk[t % NTRK].preset; }

EXPORT(void, st_track_engine, (uint32_t t, uint32_t e))
{
    track_t *tr = &trk[t % NTRK];
    if (is_drum(tr))
        return;
    panic_req |= (uint8_t)(1u << trk_index(tr));
    studio_set_engine(tr, e);
}

EXPORT(void, st_track_preset, (uint32_t t, uint32_t p))
{
    track_t *tr = &trk[t % NTRK];
    if (is_drum(tr))
        return;
    panic_req |= (uint8_t)(1u << trk_index(tr));
    studio_apply_preset(tr, p);
}

EXPORT(uint32_t, st_macro, (uint32_t t, uint32_t k))
{
    return ENGINES[trk[t % NTRK].engine % NENGINES]->macro[k % 4u];
}

/* the drum track's kit: the same parameter the device edits, so its names come from DRUM_KIT_NAMES */
EXPORT(uint32_t, st_kits, (void)) { return sizeof(DRUM_KIT_NAMES) / sizeof(DRUM_KIT_NAMES[0]); }

EXPORT(const char *, st_kit_name, (uint32_t k))
{
    return DRUM_KIT_NAMES[k % (sizeof(DRUM_KIT_NAMES) / sizeof(DRUM_KIT_NAMES[0]))];
}

EXPORT(uint32_t, st_smp_sets, (void)) { return SMP_NSETS; }

EXPORT(const char *, st_smp_set_name, (uint32_t i)) { return SMP_SET_NAMES[i % SMP_NSETS]; }

/* ---------------------------------------------------------- params --- */
static const param_desc_t *studio_desc(uint32_t t, uint32_t id)
{
    track_t *tr = &trk[t % NTRK];
    return id < P_COUNT ? track_desc(tr, id) : 0;
}

EXPORT(void, st_param, (uint32_t t, uint32_t id, int32_t v))
{
    const param_desc_t *d = studio_desc(t, id);
    if (d)
        trk[t % NTRK].p[id] = (int16_t)clamp(v, d->min, d->max);
}

EXPORT(int32_t, st_param_get, (uint32_t t, uint32_t id))
{
    return id < P_COUNT ? trk[t % NTRK].p[id] : 0;
}

EXPORT(int32_t, st_param_min, (uint32_t t, uint32_t id))
{
    const param_desc_t *d = studio_desc(t, id);
    return d ? d->min : 0;
}

EXPORT(int32_t, st_param_max, (uint32_t t, uint32_t id))
{
    const param_desc_t *d = studio_desc(t, id);
    return d ? d->max : 0;
}

EXPORT(const char *, st_param_label, (uint32_t t, uint32_t id))
{
    const param_desc_t *d = studio_desc(t, id);
    return d ? d->label : "";
}

EXPORT(const char *, st_param_name, (uint32_t t, uint32_t id, int32_t v))
{
    const param_desc_t *d = studio_desc(t, id);
    if (!d || !d->names)
        return "";
    return d->names[(uint32_t)clamp(v, d->min, d->max)];
}

static char studio_text[16];

EXPORT(const char *, st_param_text, (uint32_t t, uint32_t id))
{
    const param_desc_t *d = studio_desc(t, id);
    const char *unit;
    if (!d)
        return "";
    param_format(d, trk[t % NTRK].p[id], studio_text, &unit);
    str_cpy(studio_text + str_len(studio_text), unit, sizeof studio_text - str_len(studio_text) - 1u);
    return studio_text;
}

EXPORT(void, st_global, (uint32_t id, int32_t v))
{
    if (id < G_COUNT && GP[id].max >= GP[id].min)
        song.g[id] = (int16_t)clamp(v, GP[id].min, GP[id].max);
}

EXPORT(int32_t, st_global_get, (uint32_t id)) { return id < G_COUNT ? song.g[id] : 0; }
EXPORT(int32_t, st_global_min, (uint32_t id)) { return id < G_COUNT ? GP[id].min : 0; }
EXPORT(int32_t, st_global_max, (uint32_t id)) { return id < G_COUNT ? GP[id].max : 0; }
EXPORT(const char *, st_global_label, (uint32_t id)) { return id < G_COUNT ? GP[id].label : ""; }

EXPORT(const char *, st_global_name, (uint32_t id, int32_t v))
{
    const param_desc_t *d = id < G_COUNT ? &GP[id] : 0;
    if (!d || !d->names)
        return "";
    return d->names[(uint32_t)clamp(v, d->min, d->max)];
}

static char studio_gtext[16];

EXPORT(const char *, st_global_text, (uint32_t id))
{
    const char *unit;
    if (id >= G_COUNT)
        return "";
    param_format(&GP[id], song.g[id], studio_gtext, &unit);
    str_cpy(studio_gtext + str_len(studio_gtext), unit, sizeof studio_gtext - str_len(studio_gtext) - 1u);
    return studio_gtext;
}

/* the names the page asks ids by: P_* for st_param_*, G_* for st_global_* */
static const struct { const char *name; uint32_t id; } STUDIO_ID[] = {
    {"P_LEVEL", P_LEVEL}, {"P_ATK", P_ATK}, {"P_DEC", P_DEC}, {"P_SUS", P_SUS}, {"P_REL", P_REL},
    {"P_ED_FLT", P_ED_FLT}, {"P_ED_PIT", P_ED_PIT}, {"P_ED_SHP", P_ED_SHP}, {"P_ED_FX", P_ED_FX},
    {"P_LRATE", P_LRATE}, {"P_LWAVE", P_LWAVE}, {"P_AMODE", P_AMODE}, {"P_ARATE", P_ARATE},
    {"P_AOCT", P_AOCT}, {"P_AGATE", P_AGATE}, {"P_APROB", P_APROB}, {"P_AHOLD", P_AHOLD},
    {"P_ROOT", P_ROOT}, {"P_SCALE", P_SCALE}, {"P_QUANT", P_QUANT}, {"P_TRANS", P_TRANS},
    {"P_SLEN", P_SLEN}, {"P_SDIV", P_SDIV}, {"P_SSWING", P_SSWING}, {"P_SGATE", P_SGATE},
    {"P_DIST", P_DIST}, {"P_CHOR", P_CHOR}, {"P_DLY", P_DLY}, {"P_REV", P_REV},
    {"P_VOICE", P_VOICE}, {"P_GLIDE", P_GLIDE}, {"P_PAN", P_PAN}, {"P_MUTE", P_MUTE},
    {"P_SLCR", P_SLCR}, {"P_SLRATE", P_SLRATE}, {"P_SLDEPTH", P_SLDEPTH}, {"P_CHORD", P_CHORD},
    {"P_E0", P_E0}, {"P_E1", P_E1}, {"P_E2", P_E2}, {"P_E3", P_E3},
    {"P_E4", P_E4}, {"P_E5", P_E5}, {"P_E6", P_E6}, {"P_E7", P_E7},
    {"G_BPM", G_BPM}, {"G_SWING", G_SWING}, {"G_CLOCK", G_CLOCK}, {"G_TUNE", G_TUNE},
    {"G_DTIME", G_DTIME}, {"G_DFDBK", G_DFDBK}, {"G_DCOLOR", G_DCOLOR}, {"G_DMIX", G_DMIX},
    {"G_RSIZE", G_RSIZE}, {"G_RDAMP", G_RDAMP}, {"G_CRATE", G_CRATE}, {"G_CDEPTH", G_CDEPTH},
    {"G_ROUTE", G_ROUTE}, {"G_NAME", G_NAME}, {"G_DRLVL", G_DRLVL}, {"G_DRREV", G_DRREV},
    {"G_DUST", G_DUST}, {"G_DUCK", G_DUCK}, {"G_FILT", G_FILT}, {"G_ROLL", G_ROLL},
};

EXPORT(uint32_t, st_id_count, (void)) { return sizeof(STUDIO_ID) / sizeof(STUDIO_ID[0]); }

EXPORT(const char *, st_id_name, (uint32_t k))
{
    return k < sizeof(STUDIO_ID) / sizeof(STUDIO_ID[0]) ? STUDIO_ID[k].name : 0;
}

EXPORT(uint32_t, st_id, (uint32_t k))
{
    return k < sizeof(STUDIO_ID) / sizeof(STUDIO_ID[0]) ? STUDIO_ID[k].id : 0;
}

/* --------------------------------------------------------- patterns --- */
/* synth parts: a step holds up to 4 notes; the drum track: 16 lanes, one per white key */
EXPORT(uint32_t, st_step_has, (uint32_t t, uint32_t i, uint32_t note))
{
    const step_t *s = &trk[t % NTRK].step[i % NSTEP];
    uint32_t k;
    for (k = 0; k < s->n; k++)
        if (s->note[k] == note)
            return 1u;
    return 0u;
}

EXPORT(uint32_t, st_step_toggle, (uint32_t t, uint32_t i, uint32_t note))
{
    track_t *tr = &trk[t % NTRK];
    step_t *s = &tr->step[i % NSTEP];
    uint32_t k, found = 0;
    if (is_drum(tr) || note > 127u)
        return 0u;
    for (k = 0; k < s->n; k++)
        if (s->note[k] == note) {
            for (; k + 1u < s->n; k++)
                s->note[k] = s->note[k + 1u];
            s->n--;
            found = 1;
            break;
        }
    if (found) {
        s->time = s->n ? ST_NOTE : ST_REST;
        return 0u;
    }
    if (s->n < 4u) {
        s->note[s->n++] = (uint8_t)note;
        s->time = ST_NOTE;
        s->vel = 100;
        tr->seq_active = 1;
        return 1u;
    }
    return 0u;                                        /* the step already holds four notes */
}

/* an empty pattern (the page's CLEAR) */
EXPORT(void, st_steps_clear, (uint32_t t))
{
    steps_clear(&trk[t % NTRK]);
}

/* 0 empty, ST_NOTE / ST_TIE / ST_REST; a TIE holds what the previous step started */
EXPORT(uint32_t, st_step_time, (uint32_t t, uint32_t i))
{
    return trk[t % NTRK].step[i % NSTEP].time;
}

EXPORT(uint32_t, st_step_tie, (uint32_t t, uint32_t i))
{
    track_t *tr = &trk[t % NTRK];
    step_t *s = &tr->step[i % NSTEP];
    if (is_drum(tr))
        return s->time;
    if (s->time == ST_TIE)
        s->time = s->n ? ST_NOTE : ST_REST;
    else if (!s->n) {                             /* hold a step with no note of its own */
        s->time = ST_TIE;
        tr->seq_active = 1;
    }
    return s->time;
}

EXPORT(uint32_t, st_lane_get, (uint32_t i, uint32_t lane))
{
    return (uint32_t)dstep_has(&TDRUM->dstep[i % NSTEP], lane % DRUM_LANES);
}

EXPORT(uint32_t, st_lane_toggle, (uint32_t i, uint32_t lane))
{
    dstep_t *s = &TDRUM->dstep[i % NSTEP];
    uint32_t l = lane % DRUM_LANES;
    if (dstep_has(s, l))
        dstep_clr(s, l);
    else
        dstep_set(s, l, LV_NORM, 0);
    TDRUM->seq_active = 1;
    return (uint32_t)dstep_has(s, l);
}

EXPORT(uint32_t, st_lane_level, (uint32_t i, uint32_t lane))
{
    return dstep_lvl(&TDRUM->dstep[i % NSTEP], lane % DRUM_LANES);
}

EXPORT(void, st_lane_level_set, (uint32_t i, uint32_t lane, uint32_t lvl))
{
    dstep_t *s = &TDRUM->dstep[i % NSTEP];
    uint32_t l = lane % DRUM_LANES;
    dstep_set(s, l, lvl & 3u, dstep_rat(s, l));
}

EXPORT(uint32_t, st_lane_note, (uint32_t lane)) { return LANE_NOTE[lane % DRUM_LANES]; }
EXPORT(uint32_t, st_lane_of_key, (uint32_t k)) { return lane_of_key(k % 27u); }

/* ------------------------------------------------------------- input --- */
EXPORT(void, st_keys, (uint32_t mask)) { fm1_in.notes = mask; }      /* 27 keys, as the panel reports them */
EXPORT(int32_t, st_octave_get, (void)) { return song.octave; }

EXPORT(void, st_octave, (int32_t o))
{
    song.octave = (int8_t)clamp(o, -4, 4);
}

EXPORT(void, st_midi, (uint32_t status, uint32_t d1, uint32_t d2))
{
    midi_in_q[mi_w++ % MQ] = (status & 0xFFu) << 8 | (d1 & 0x7Fu) << 16 | (d2 & 0x7Fu) << 24;
}

EXPORT(uint32_t, st_bpm, (void)) { return (uint32_t)song.g[G_BPM]; }
