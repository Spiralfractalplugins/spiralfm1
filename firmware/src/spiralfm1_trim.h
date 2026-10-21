/* 1.0: re-measured after the SPIRALFM1 retune (delta against the Mycelium 1.0 render); -20 LUFS target, peak headroom cap.
 * MODERN MOTION pack (21..28 ANALOG, 29..36 DIGITAL): iterated to -20 LUFS with
 * tests/spiralfm1_preset_level.c + the BS.1770 method of tools/level_presets.py
 * (CINEMA HIT stays ~2 dB low: its trim is at the +23 top of the table). */
#pragma once
static const int8_t SF_ANALOG_TRIM[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-7,3,-10,-1,-4,8,-6,-33,-15,-19,-25,-7,-12};
static const int8_t SF_DIGITAL_TRIM[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,10,0,2,-3,-14,-3,3,-9,-2,3,0,-3,-1,-2,-2,2,-7,-4,4,23,-9,-12,-5,-4};
