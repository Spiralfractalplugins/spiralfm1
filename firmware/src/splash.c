/* SPDX-License-Identifier: GPL-3.0-only */
/* Spiral logo, decoded in strips within the existing canvas. */
#include "spiralfm1_logo.h"
static void sloop_splash(void)
{
    uint32_t row;
    lcd_fill(0, 0, 240, 240, C_BLACK);
    for (row = 0; row < SF_LOGO_H; row += 120u) {
        uint32_t rows = SF_LOGO_H - row > 120u ? 120u : SF_LOGO_H - row;
        uint32_t lo = row * SF_LOGO_W, hi = (row + rows) * SF_LOGO_W, pos = 0, i;
        cv_begin(SF_LOGO_W, rows, C_BLACK);
        for (i = 0; i < sizeof SF_LOGO_RLE && pos < hi; i++) {
            uint32_t run = (SF_LOGO_RLE[i] >> 4) + 1u, c = SF_LOGO_RLE[i] & 15u, p;
            for (p = pos < lo ? lo : pos; p < pos + run && p < hi; p++)
                cv_px[p - lo] = swap16(SF_LOGO_PAL[c]);
            pos += run;
        }
        cv_blit(0, row);
    }
    cv_begin(240, 52, C_BLACK);
    cv_text(120-text_w(&FONT_S,"SPIRALFRACTAL")/2, 0, &FONT_S,"SPIRALFRACTAL",RGB(110,235,255));
    cv_text(120-text_w(&FONT_S,FELUCCA_VERSION)/2, 18, &FONT_S,FELUCCA_VERSION,RGB(197,155,255));
    cv_text(120-text_w(&FONT_S,"BASED ON MYCELIUM")/2, 35, &FONT_S,"BASED ON MYCELIUM",RGB(80,104,76));
    cv_blit(0, 188);
}
