/* SPDX-License-Identifier: GPL-3.0-only */
/* Official Mandalars logo, decoded in strips within the existing canvas. */
#include "mycelium_logo.h"
static void sloop_splash(void)
{
    uint32_t row;
    lcd_fill(0, 0, 240, 240, C_BLACK);
    for (row = 0; row < MY_LOGO_H; row += 120u) {
        uint32_t rows = MY_LOGO_H - row > 120u ? 120u : MY_LOGO_H - row;
        uint32_t lo = row * MY_LOGO_W, hi = (row + rows) * MY_LOGO_W, pos = 0, i;
        cv_begin(MY_LOGO_W, rows, C_BLACK);
        for (i = 0; i < sizeof MY_LOGO_RLE && pos < hi; i++) {
            uint32_t run = (MY_LOGO_RLE[i] >> 4) + 1u, c = MY_LOGO_RLE[i] & 15u, p;
            for (p = pos < lo ? lo : pos; p < pos + run && p < hi; p++)
                cv_px[p - lo] = swap16(MY_LOGO_PAL[c]);
            pos += run;
        }
        cv_blit(0, row);
    }
    cv_begin(240, 52, C_BLACK);
    cv_text(120-text_w(&FONT_S,"MANDALARS.ORG")/2, 0, &FONT_S,"MANDALARS.ORG",RGB(200,255,105));
    cv_text(120-text_w(&FONT_S,FELUCCA_VERSION)/2, 18, &FONT_S,FELUCCA_VERSION,RGB(197,155,255));
    cv_text(120-text_w(&FONT_S,"SLOOP / FELUCCA")/2, 35, &FONT_S,"SLOOP / FELUCCA",RGB(80,104,76));
    cv_blit(0, 188);
}
