/* SPDX-License-Identifier: GPL-3.0-only */
/* MODERN MOTION pack (Vital / Serum 2 inspired, original designs): every new ANALOG preset from 21
 * and every DIGITAL preset from 29 has edit parameters inside their descriptor ranges, extra
 * parameters inside the TP ranges, no preset name is longer than 12 characters, the pack names are
 * unique across the two engines, and each preset renders audible, bounded samples over a wide pitch
 * and cutoff sweep. Direct engine calls, like tests/mycelium_digital_test.c. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define __attribute__(x)
#include "felucca_tables.h"
#include "../firmware/src/core.h"
#include "../firmware/src/dsp.c"
#include "../firmware/src/eng_analog.c"
#include "../firmware/src/eng_digital.c"

enum { A_FIRST = 21, D_FIRST = 29 };
static uint32_t inc_note(int note)
{
    return (uint32_t)(440.0 * pow(2.0, (note - 69) / 12.0) * 4294967296.0 / FS);
}

static int check_engine(const engine_t *eng, const preset_t *pr, int from, int pi)
{
    uint32_t j;
    if (from >= 0 && pi < (uint32_t)from)
        return 0;
    if (strlen(pr->name) > 12u) {
        fprintf(stderr, "FAIL %s: name longer than 12\n", pr->name);
        return 1;
    }
    for (j = 0; j < 8u; j++)
        if (pr->e[j] < eng->edit[j].min || pr->e[j] > eng->edit[j].max) {
            fprintf(stderr, "FAIL %s: %s = %d out of [%d..%d]\n",
                    pr->name, eng->edit[j].label, pr->e[j], eng->edit[j].min, eng->edit[j].max);
            return 1;
        }
    for (j = 0; j + 1u < 8u; j += 2u)
        if (pr->x[j] - 1 >= P_E0) {
            fprintf(stderr, "FAIL %s: extra param id %d is not a common parameter\n",
                    pr->name, pr->x[j] - 1);
            return 1;
        }
    return 0;
}

static int render_engine(const engine_t *eng, void (*note_on)(track_t *, voice_t *),
                         void (*render)(track_t *, voice_t *, int32_t *, uint32_t, const vmod_t *),
                         const preset_t *pr, const char *what)
{
    int peak = 0, note;
    uint32_t sweep;
    for (sweep = 0; sweep <= 127u; sweep++) {
        for (note = 24; note <= 96; note += 12) {
            track_t t;
            voice_t v;
            memset(&t, 0, sizeof t);
            memset(&v, 0, sizeof v);
            for (int j = 0; j < 8; j++)
                t.p[P_E0 + j] = pr->e[j];
            for (int j = 0; j < 4; j++)
                t.p[P_ATK + j] = pr->env[j];
            v.vel = 96;
            note_on(&t, &v);
            vmod_t m;
            memset(&m, 0, sizeof m);
            m.inc = inc_note(note);
            m.amp0 = m.amp1 = 16384;
            m.cutoff = (int32_t)sweep << 8;
            for (int b = 0; b < 30; b++) {
                int32_t out[CTL] = {0};
                render(&t, &v, out, CTL, &m);
                for (int j = 0; j < CTL; j++) {
                    int a = abs(out[j]);
                    if (a > 32767) {
                        fprintf(stderr, "FAIL %s: output %d over 16 bits\n", pr->name, a);
                        return 1;
                    }
                    if (a > peak)
                        peak = a;
                }
            }
        }
    }
    if (peak < 64) {
        fprintf(stderr, "FAIL %s: %s sweep stays silent (peak %d)\n", pr->name, what, peak);
        return 1;
    }
    printf("PASS %s: %s sweep, peak %d\n", pr->name, what, peak);
    return 0;
}

int main(void)
{
    int bad = 0;
    uint32_t na = (uint32_t)(sizeof ANALOG_PRESETS / sizeof ANALOG_PRESETS[0]);
    uint32_t nd = (uint32_t)(sizeof DIGITAL_PRESETS / sizeof DIGITAL_PRESETS[0]);
    if (na < A_FIRST + 8u || nd < D_FIRST + 8u) {
        fprintf(stderr, "FAIL the MODERN MOTION pack is not in the tables\n");
        return 1;
    }
    for (uint32_t pi = 0; pi < na; pi++)
        bad |= check_engine(&ENG_ANALOG, &ANALOG_PRESETS[pi], A_FIRST, (int)pi);
    for (uint32_t pi = 0; pi < nd; pi++)
        bad |= check_engine(&ENG_DIGITAL, &DIGITAL_PRESETS[pi], D_FIRST, (int)pi);
    for (uint32_t pi = A_FIRST; pi < na; pi++)
        for (uint32_t pj = D_FIRST; pj < nd; pj++)
            if (!strcmp(ANALOG_PRESETS[pi].name, DIGITAL_PRESETS[pj].name)) {
                fprintf(stderr, "FAIL duplicate pack name %s\n", ANALOG_PRESETS[pi].name);
                bad = 1;
            }
    for (uint32_t pi = A_FIRST; pi < na; pi++)
        bad |= render_engine(&ENG_ANALOG, analog_note_on, analog_render, &ANALOG_PRESETS[pi], "pitch / cutoff");
    for (uint32_t pi = D_FIRST; pi < nd; pi++)
        bad |= render_engine(&ENG_DIGITAL, digital_note_on, digital_render, &DIGITAL_PRESETS[pi], "pitch / index");
    if (bad)
        return 1;
    printf("MODERN MOTION pack: %u ANALOG + %u DIGITAL presets, %d checked\n",
           na - A_FIRST, nd - D_FIRST, (int)((na - A_FIRST) + (nd - D_FIRST)));
    return 0;
}
