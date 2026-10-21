/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* DIGITAL: four-operator FM. */
/* Four sine operators, eight classic 4-operator algorithms (ALG = P_E0, EDIT 1 KNOB 1),
 * op 4 with feedback, one modulation INDEX shaped by a modulator envelope.
 * Phase modulation wraps naturally in the 32-bit phase. */
static const char *const N_FMALG[] = {"1", "2", "3", "4", "5", "6", "7", "8"};
static const char *const N_RATIO[] = {".5", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "14", "16"};
static const uint16_t RATIO_Q8[15] = {128, 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816,
                                      3072, 3584, 4096};

static void digital_note_on(track_t *t, voice_t *v)
{
    (void)t;
    v->ph[0] = v->ph[1] = v->ph[2] = 0;
    v->s[7] = 0;                 /* op 4 phase */
    v->s[4] = 1 << 24;           /* modulator envelope, Q24 */
    v->s[5] = v->s[6] = 0;       /* feedback history */
}

/* operator increment = carrier * ratio (Q8) without 32-bit overflow; kept below Nyquist */
static inline uint32_t fm_ratio_inc(uint32_t inc, uint32_t r)
{
    uint32_t lim = 0x73000000u / r;                      /* (inc >> 8) * r must stay < 0x73000000 */
    return (inc >> 8) > lim ? 0x73000000u : (inc >> 8) * r;
}

static inline uint32_t digital_mod(int32_t x, int32_t idx) { return (uint32_t)(x * idx) * 2065u; }

static void digital_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    uint32_t alg = (uint32_t)p[P_E0] & 7u, i;
    uint32_t i1 = m->inc;
    uint32_t i2 = fm_ratio_inc(m->inc, RATIO_Q8[p[P_E1] % 15]), i3 = fm_ratio_inc(m->inc, RATIO_Q8[p[P_E2] % 15]);
    uint32_t i4 = fm_ratio_inc(m->inc, RATIO_Q8[p[P_E3] % 15]);
    int32_t me, idx, fb = p[P_E6], fb1, fb2;                 /* fb1, fb2: op 4 feedback history */
    uint32_t ph0, ph1, ph2, ph4;
    /* modulator envelope: decays (MODDEC) towards 25 %, per block */
    v->s[4] += mulq16((1 << 22) - v->s[4], ENV_EXP[p[P_E5] & 127]);
    me = v->s[4] >> 9;                                         /* Q15 */
    idx = (p[P_E4] * me) >> 15;                                 /* 0..127 */
    idx = clamp(idx + ((m->cutoff + m->shape - (64 << 8)) >> 8) + (v->vel - 96) / 4, 0, 127);
    ph0 = v->ph[0];                                             /* state in locals: out[] may alias v->s[] */
    ph1 = v->ph[1];
    ph2 = v->ph[2];
    ph4 = (uint32_t)v->s[7];
    fb1 = v->s[5];
    fb2 = v->s[6];
    for (i = 0; i < n; i++) {
        int32_t o1, o2, o3, o4, s;
        o4 = sine_i((ph4 + digital_mod((fb1 + fb2) >> 1, fb)));
        fb2 = fb1;
        fb1 = o4;
        switch (alg) {
        case 0:
            o3 = sine_i((ph2 + digital_mod(o4, idx)));
            o2 = sine_i((ph1 + digital_mod(o3, idx)));
            s = sine_i((ph0 + digital_mod(o2, idx)));
            break;
        case 1:
            o3 = sine_i(ph2);
            o2 = sine_i((ph1 + digital_mod((o3 + o4) >> 1, idx)));
            s = sine_i((ph0 + digital_mod(o2, idx)));
            break;
        case 2:
            o3 = sine_i(ph2);
            o2 = sine_i((ph1 + digital_mod(o3, idx)));
            s = sine_i((ph0 + digital_mod((o2 + o4) >> 1, idx)));
            break;
        case 3:
            o3 = sine_i((ph2 + digital_mod(o4, idx)));
            o2 = sine_i(ph1);
            s = sine_i((ph0 + digital_mod((o2 + o3) >> 1, idx)));
            break;
        case 4:
            o3 = sine_i((ph2 + digital_mod(o4, idx)));
            o2 = sine_i(ph1);
            o1 = sine_i((ph0 + digital_mod(o2, idx)));
            s = (o1 + o3) >> 1;
            break;
        case 5:
            o3 = sine_i((ph2 + digital_mod(o4, idx)));
            o2 = sine_i((ph1 + digital_mod(o4, idx)));
            o1 = sine_i((ph0 + digital_mod(o4, idx)));
            s = mulq15(o1 + o2 + o3, 10923);           /* / 3 without a divide */
            break;
        case 6:
            o3 = sine_i((ph2 + digital_mod(o4, idx)));
            o2 = sine_i(ph1);
            o1 = sine_i(ph0);
            s = mulq15(o1 + o2 + o3, 10923);           /* / 3 without a divide */
            break;
        default:
            o3 = sine_i(ph2);
            o2 = sine_i(ph1);
            o1 = sine_i(ph0);
            s = (o1 + o2 + o3 + o4) >> 2;
            break;
        }
        ph0 += i1;
        ph1 += i2;
        ph2 += i3;
        ph4 += i4;
        out[i] += mulq15(mulq15(s, amp_at(m, i)), VOICE_FS);   /* full-scale sines: 6 dB below the others */
    }
    v->ph[0] = ph0;
    v->ph[1] = ph1;
    v->ph[2] = ph2;
    v->s[7] = (int32_t)ph4;
    v->s[5] = fb1;
    v->s[6] = fb2;
}

static const preset_t DIGITAL_PRESETS[] = {
    /* ALG R2 R3 R4 INDEX MODDEC FDBK - ; ALG 5 (4 here) = two stacks: 2 -> 1 and 4 -> 3 (the tine) */
    {"RHODES", {4, 1, 1, 14, 26, 35, 0, 0}, {0, 85, 35, 55}, 0, 0, FX(0, 30, 12, 26), XP(P_LD_AMP + 1, 26, P_LRATE + 1, 84)},
    {"DX RHODES", {4, 1, 1, 14, 85, 45, 0, 0}, {0, 82, 40, 55}, 0, 0, FX(0, 45, 20, 30)},
    {"WURLI", {4, 1, 2, 3, 62, 35, 8, 0}, {0, 75, 30, 45}, 0, 0, FX(10, 20, 10, 20), XP(P_LD_AMP + 1, 16, P_LRATE + 1, 89)},
    {"CLAV", {3, 1, 3, 5, 90, 25, 20, 0}, {0, 58, 34, 18}, 0, 0, FX(10, 10, 6, 10)},
    /* house piano: bright, fast hammer, the tine stack for the attack */
    {"M1 PIANO", {4, 1, 2, 13, 74, 30, 12, 0}, {0, 86, 30, 48}, 0, 0, FX(0, 24, 14, 26)},
    /* afro house / amapiano: round keys with a bell on top */
    {"AFRO KEYS", {4, 1, 1, 7, 40, 50, 0, 0}, {0, 82, 40, 60}, 0, 0, FX(0, 30, 26, 36)},
    {"TRAP BELL", {4, 8, 1, 4, 80, 70, 0, 0}, {0, 92, 0, 70}, 0, 0, FX(0, 0, 30, 40)},
    {"MUSIC BOX", {4, 4, 3, 9, 45, 75, 0, 0}, {0, 90, 0, 75}, 0, 0, FX(0, 10, 32, 45)},
    {"MARIMBA", {4, 4, 1, 1, 60, 30, 0, 0}, {0, 80, 0, 60}, 0, 0, FX(0, 0, 22, 30)},
    {"KALIMBA", {4, 4, 1, 6, 52, 24, 0, 0}, {0, 78, 0, 58}, 0, 0, FX(0, 0, 26, 38)},
    /* pluggnb / cloud rap: a soft bell, lots of space */
    {"PLUGG BELL", {4, 3, 1, 4, 56, 60, 0, 0}, {0, 90, 0, 82}, 0, 0, FX(0, 22, 36, 56)},
    {"GLASS PAD", {4, 2, 1, 5, 34, 100, 0, 0}, {62, 92, 112, 92}, 0, 0, FX(0, 50, 26, 62), XP(P_LD_PIT + 1, 1, P_LRATE + 1, 40)},
    {"FM BASS", {0, 1, 1, 1, 34, 40, 8, 0}, {0, 60, 70, 25}, 0, 1, FX(0, 0, 0, 6), XP(P_GLIDE + 1, 36, P_TRANS + 1, -24)},
    /* SPIRALFM1: original FM sketches, retuned for 1.0; not copied tutorial presets. */
    {"SPIRAL BASS", {1, 1, 2, 3, 34, 30, 7, 0}, {0, 34, 2, 14}, 0, 1, FX(4, 0, 0, 2), XP(P_TRANS + 1, -24, P_VOICE + 1, V_MONO, P_LD_SHP + 1, 12)},
    {"VORTEX TALK", {1, 3, 5, 7, 84, 42, 34, 0}, {0,64,46,38}, 0, 0, FX(10, 0, 22, 16), XP(P_LD_SHP + 1, 30, P_LRATE + 1, 54, P_LWAVE + 1, 2)},
    {"PHASE BLIP", {4, 5, 8, 11, 62, 20, 16, 0}, {0,48,0,40}, 0, 0, FX(0, 0, 28, 24)},
    {"ORBIT GROWL", {2, 2, 6, 9, 104, 50, 58, 0}, {0,66,52,40}, 0, 0, FX(16, 0, 16, 14), XP(P_LD_SHP + 1, 36, P_LRATE + 1, 38, P_LFADE + 1, 60)},
    {"HELIX DRONE", {5, 1, 7, 12, 34, 112, 12, 0}, {18,98,102,92}, 0, 0, FX(0, 28, 26, 52), XP(P_LD_SHP + 1, 16, P_LRATE + 1, 18, P_LWAVE + 1, 4)},
    {"TORUS METAL", {3, 9, 11, 14, 98, 26, 30, 0}, {0,56,0,48}, 0, 0, FX(0, 0, 24, 22)},
    {"FRACTAL DROP", {1, 2, 4, 8, 94, 32, 40, 0}, {0,60,0,46}, 0, 0, FX(8, 0, 26, 28), XP(P_ED_PIT + 1, 30)},
    {"NIGHT WHORL", {6, 1, 5, 10, 44, 92, 20, 0}, {10,84,74,82}, 0, 0, FX(0, 20, 22, 48), XP(P_LD_SHP + 1, 22, P_LRATE + 1, 28)},
    /* SPIRALFM1: electronic squelches, inharmonic hits and pitch-falling FX. */
    {"ACID COIL", {1, 3, 6, 8, 116, 26, 70, 0}, {0,64,42,40}, 0, 0, FX(20,0,20,14), XP(P_LD_SHP+1,44,P_LRATE+1,68,P_LWAVE+1,1)},
    {"PHASE CHIRP", {4, 2, 7, 10, 110, 20, 54, 0}, {0,54,22,44}, 0, 0, FX(14,0,24,18), XP(P_ED_PIT+1,38,P_LD_SHP+1,26,P_LRATE+1,60)},
    {"METAL HIT", {2, 10, 12, 14, 112, 22, 46, 0}, {0,58,0,50}, 0, 0, FX(8,0,14,26), XP(P_ED_PIT+1,12)},
    {"RUST IMPACT", {1, 0, 9, 13, 120, 38, 92, 0}, {0, 70, 0, 48}, 0, 0, FX(26,0,12,38), XP(P_ED_PIT+1,54)},
    {"HITECH ZAP", {0, 3, 11, 14, 124, 16, 76, 0}, {0,46,0,28}, 0, 0, FX(16,0,18,10), XP(P_ED_PIT+1,68)},
    {"LASER FALL", {3, 2, 8, 12, 100, 52, 44, 0}, {0,72,0,44}, 0, 0, FX(12,0,32,20), XP(P_ED_PIT+1,88,P_LD_SHP+1,14,P_LRATE+1,80)},
    {"LIQUID FM", {1, 1, 4, 8, 94, 42, 40, 0}, {0,70,42,46}, 0, 0, FX(8,0,28,16), XP(P_LD_SHP+1,44,P_LRATE+1,58)},
    {"COIL BURST", {6, 4, 8, 12, 122, 16, 90, 0}, {0,50,0,40}, 0, 0, FX(20,0,20,28), XP(P_ED_PIT+1,46)},
    /* MODERN MOTION: original FM sketches after the wobble / growl / glass-pluck sound of modern soft
     * synths (Vital, Serum): LFO on the carrier mix, inharmonic cinematic hits, risers. Not copied patches. */
    {"FM WOBBLE", {0, 1, 2, 2, 52, 30, 18, 0}, {0, 66, 127, 30}, 0, 1, FX(10, 0, 6, 8), XP(P_LRATE + 1, 58, P_LWAVE + 1, 1, P_LD_FLT + 1, 60, P_TRANS + 1, -24)},
    {"MODERN GROWL", {2, 1, 3, 7, 108, 44, 64, 0}, {0, 62, 64, 34}, 0, 0, FX(20, 0, 12, 14), XP(P_LRATE + 1, 42, P_LWAVE + 1, 2, P_LD_SHP + 1, 50, P_ED_PIT + 1, -16)},
    {"GLASS PLUCK", {4, 1, 2, 13, 62, 40, 0, 0}, {0, 80, 16, 54}, 0, 0, FX(0, 22, 28, 46), XP(P_LRATE + 1, 64, P_LD_AMP + 1, 12)},
    {"CINEMA HIT", {3, 6, 9, 12, 120, 26, 74, 0}, {0, 54, 0, 50}, 0, 0, FX(26, 0, 18, 48), XP(P_ED_PIT + 1, 42)},
    {"NEON SHIM", {5, 2, 5, 11, 44, 92, 8, 0}, {40, 94, 110, 90}, 0, 0, FX(0, 42, 32, 68), XP(P_LRATE + 1, 54, P_LD_AMP + 1, 24, P_LFADE + 1, 44)},
    {"RISER FX", {4, 4, 7, 11, 86, 36, 34, 0}, {0, 88, 126, 52}, 0, 0, FX(10, 0, 38, 62), XP(P_LRATE + 1, 98, P_LWAVE + 1, 2, P_LD_SHP + 1, 54, P_LFADE + 1, 90)},
    {"VOCAL FM", {5, 1, 4, 8, 72, 48, 22, 0}, {0, 70, 86, 42}, 0, 0, FX(14, 14, 22, 30), XP(P_LRATE + 1, 50, P_LD_SHP + 1, 34)},
    {"DROP SUB", {0, 1, 1, 1, 22, 22, 4, 0}, {0, 52, 84, 18}, 0, 1, FX(0, 0, 0, 4), XP(P_ED_PIT + 1, 44, P_GLIDE + 1, 20, P_TRANS + 1, -24, P_VOICE + 1, V_MONO)},
};

static const engine_t ENG_DIGITAL = {
    "DIGITAL", {"OPS", "MOD"},
    {
        {"ALG", F_ENUM, 0, 7, 0, N_FMALG, 0},
        {"R2", F_ENUM, 0, 14, 1, N_RATIO, 0},
        {"R3", F_ENUM, 0, 14, 1, N_RATIO, 0},
        {"R4", F_ENUM, 0, 14, 1, N_RATIO, 0},
        {"IDX", F_PCT, 0, 127, 60, 0, 0},
        {"MDEC", F_TIME, 0, 127, 60, 0, 0},
        {"FB", F_PCT, 0, 127, 0, 0, 0},
        {"-", F_INT, 0, 0, 0, 0, 0},
    },
    DIGITAL_PRESETS, sizeof(DIGITAL_PRESETS) / sizeof(DIGITAL_PRESETS[0]), -1, digital_note_on, digital_render,
    0xFD20, {P_E4, P_E5, P_E6, P_REL},
};
