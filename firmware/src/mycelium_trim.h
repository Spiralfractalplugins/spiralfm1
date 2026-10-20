/* 0.16: measured after envelope corrections; -20 LUFS target, peak headroom cap.
 * MODERN MOTION pack (21..28 ANALOG, 29..36 DIGITAL): iterated to -20 LUFS with
 * tests/mycelium_preset_level.c + the BS.1770 method of tools/level_presets.py
 * (CINEMA HIT stays ~2 dB low: its trim is at the +23 top of the table). */
#pragma once
static const int8_t MY_ANALOG_TRIM[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-9,4,-10,-8,-6,8,-14,-32,-14,-18,-19,-8,-9};
static const int8_t MY_DIGITAL_TRIM[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-6,-3,-12,-4,-1,-7,-3,-2,0,-3,-1,-2,-2,2,-10,-3,2,23,-11,-16,-3,-4};
