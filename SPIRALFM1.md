# SPIRALFM1 for the M-VAVE FM-1 - 1.0

Forest, Goa, Psytrance, Hitech and techno for the M-VAVE FM-1.
An independent firmware by Spiralfractal: built on Mycelium 1.0 (Mandalars), which is itself
based on SLOOP/Felucca. The firmware is GPL-3.0-only and keeps its origin notices.
SPIRALFM1 brings its own mark (a spiral instead of mycelium), renamed and retuned factory
presets and newly synthesised drum hits; the grooves, songs and PSY FX handling are those of
Mycelium 1.0.

Hold HOME → GROOVES → choose a groove → OCT+ → OCT+ to confirm.
Save your own work first: loading replaces the current loop and its sounds, but no saved
project slots. Transport and recording must be stopped. Then play any key: the beat starts.
After a deliberate STOP, PLAY starts it again.
An update never replaces a saved scene: load the new groove explicitly.

Six grooves, 32 steps each: FOREST JAM 156 BPM, PSY ROLL 148,
PSY GALLOP 146, GOA ACID 144, HITECH 196, TEKK 170.
Forest: quarter-note kicks, three bass sixteenths between them, offbeat hats, no snare.
TeKK: offbeat bass, quarter-note kicks with an end fill, sparse clap.
Loading clears solo and restores drum level and filter states.

Tap ARP: GALLOP → OFFBEAT → FULL-ON. Replaces the bass pattern only;
the root note, the other tracks, the tempo and a running transport stay as they are.
The switch is blocked while recording / counting in or while the arranger is active.
Holding ARP still offers roll.

Tap FX and let go to open PSY FX:
1 KB LP: one shared lowpass for the dry bass on track 1 and kick notes 35/36.
  Left open, right closed; fixed resonance 25/127, about twenty percent.
2 SPACE: clocked echo and reverb together, sends for lead and FX.
  At 0 nothing new is fed in; reverb tails ring out.
3 FB+DRV: echo feedback and low-end drive together; left both 0,
  right feedback 100/127 and drive 127/127 (3x soft saturation before the lowpass).
4 ALIEN: ring modulation on dry lead and FX only (tracks 2/3).
  Left bypass, right metallic modulation up to 2400 Hz; smoothed intensity.
  Kick, bass and all drums stay untouched. Echo / reverb sends remain separate.
Holding FX still offers the punch effects. The project format stays compatible.

21 curated factory presets; three different Psy / Forest / Hitech drum kits.
The kits are built from 15 procedurally synthesised hits
(`tools/gen_spiral_drums.py`): oscillators, pitch envelopes, noise and saturation,
fixed seeds, mono PCM16 at 44.1 kHz. Origin and hashes:
assets/spiralfm1/samples/SOURCE.json, DRUM-SOURCES.json and ORIGINAL-SOUNDS.json.
The earlier Mycelium selection drew on purchased Loopmasters hits; it is fully replaced by our
own synthesis. Distribution rights of the firmware base stay separately marked
(CC0 sound bank, see LICENSING.md).

Own SPIRALFM1 spiral logo at start (drawn by tools/gen_spiralfm1_svg.py), dark background.
Tracks 1-4: neon green, turquoise, neon yellow, neon pink.
USB name SPIRALFM1, manufacturer Spiralfractal. Mark, wordmark and artwork are own designs.
USB / TRS MIDI: channels 1-3 synths, channel 10 drums, channel 4 the selected track.
Notes, velocity, clock / start / stop; ordinary sound-control CCs are not implemented.
GLO → SYSTEM → SYNC selects USB / TRS / INT.

Checked: memory and CPU budget, sequencer, autostart, menus, FX, bass switching,
drum kits, TeKK and 20,000 random UI inputs. 18 audio verification renders from the real
host synthesis code with no over-driven samples; no device recordings.
Listening and handling tests on the FM-1 are still due after installation.
Dash-Glitch tutorials were not reconstructed in detail; no cooperation is claimed.
Before any public release, check the inherited asset licences and comply with the GPL.

## New performance handling in 0.14
ENV tap: the sound envelope. Hold ENV for at least 0.7 seconds: a temporary psy fill
with hats and claps, only while the beat runs. Releasing ends the fill without
changing any pattern. LFO tap: modulation. Hold LFO for at least 0.7 seconds:
FOREST / GOA / HITECH patterns for lead and FX in a circle. That replaces only those
two patterns; save your own work first. Bass, drums and tempo stay.
Recording / count-in and an active song arranger block both hold actions.
Hold HOME → LOGO → OCT+: look at the original logo, OCT- back to the menu.
PSY FX keeps showing the last turned knob large, with a neon bar and its target label,
even with ZOOM off.

## Probability and start screen in 0.15
HOME / track overview, tracks 2 and 3: KNOB 1 PROB, separately per track.
100 percent plays every programmed NOTE step; 0 percent none; in between every
NOTE step is decided anew. Chords and ratchets belong to that decision together.
Skipped steps do not fire ratchets, ties do not roll again. The pattern is not deleted.
Directly played notes stay playable; bass and drums get no probability gate.
Stored in the existing P_APROB parameter, which the arpeggiator also uses.
Factory presets start at 100 percent; changing the sound can reset that sound parameter.
On bass / drums, KNOB 1 in the track overview stays the global swing.
When powering on, the SPIRALFM1 logo stays for about four seconds.
The watchdog is served regularly while that screen shows.

## Last test version 0.16
After installing: save your own work first, then load a new factory groove or pick the factory
presets again. Saved scenes otherwise keep their old parameters. The update replaces no saved
projects.
Bass: track 1 → ENV tap → ATK/DEC/SUS/REL.
Kick: track 4 → ENV tap → ATK/DEC/SUS/REL. The first turn activates the separate
kick envelope and starts at attack 0, decay 70, sustain 127, release 127.
The kick envelope acts only on notes 35/36, not on hats / claps / snares. Without activation
sample kicks stay unchanged. Repeated kicks restart the envelope.
Release follows the note-off or the automatic one-shot gate: a quarter note at the
current tempo, multiplied by the existing drum GATE value (SGATE/128).
Holding ENV stays the temporary psy fill, a short press opens the envelope.
FX KNOB 4 ALIEN+: ring modulation and stutter together on lead and FX. Depth rises
from 0 to 127, the stutter grid switches 1/8 → 1/16 → 1/32. Knob 0 switches stutter off.
Takes over the slicer settings of tracks 2/3; bass and drums stay outside.
16 lead / FX / atmosphere envelopes revised: longer tails, audible sustain
on leads and faster attack on atmospheres. Percussive hits stay at sustain 0.
All 21 presets re-measured, individual trims corrected with a -20 LUFS target and peak reserve.
Host checks passed; the physical listening and handling test by Lars is still outstanding.

## Base release Mycelium 1.0
Released on 08.10.2026 by Lars (Mandalars). Basis: 0.16-test. Only the version display was
changed to Mycelium 1.0. Sounds, patterns and handling stayed as they were.

## Current 1.0 state: own drums and epic songs
The historic LTWB / Loopmasters selection is replaced in this revision.
15 own drums: oscillators, pitch envelopes, noise and saturation.
Generator and SHA256 provenance in tools/gen_spiral_drums.py and
assets/spiralfm1/samples/ORIGINAL-SOUNDS.json.
No external audio inputs. The extra hip-hop pack is no longer in the release source tree.
The GPL firmware base and the separately marked CC0 sound bank remain.
Hold HOME → SONGS: Forest Odyssey / Psy Ascension / Tekk Reactor / Hitech Portal.
Each song 240 bars, 16 sections; A atmosphere, B build, C drop, D break.
Virtual scenes overwrite no project slots. USER SONG restores your own order.
Factory songs are read-only; pick them again after a restart.
Hold ARP for at least 700 ms: roll mode and logo latch. Tap ARP again: leave without a bass change.
Played notes roll only while their key is held. Four complete songs and UI fuzz checked.
Not yet installed on the device; a new physical listening test is required.

Installation: the current 1.0 package transferred completely, restart and running device MIDI
identity verified. Own drums, songs and ARP latch are installed; the physical listening test of
the new functions is outstanding.

## Direct SAVE and distortion
SAVE tap from the overview, the drum page or SONG: four finished factory song banks.
USER SONG opens your own sections; there EDIT to secure the chain. KNOB 4 in TRACKS:
P_DIST for bass / drums, P_PAN for synth tracks 2/3. Drum KIT KNOB 4: P_DIST, the drum GRID
keeps hit level. Drum distortion sits before pan / reverb and the kick / bass lowpass, and a
zero value changes nothing. FX KNOB 4 stays ALIEN+. Six grooves checked at maximum bass and
drum distortion without clipping.

## MODERN MOTION: 16 new factory presets
Ten years of modern soft-synth club sound on the FM-1: eight ANALOG and eight DIGITAL (FM)
presets after the character of the movement basses, wobbles, growls, risers, big-room leads and
glass plucks of Vital and Serum 2 — own designs, no imported preset data (the licences of those
synths allow no import; only the direction of the sound is referenced). ANALOG 21-28: MODERN
PLUCK, GROWL VA, BIG ROOM LD, TRANCE RISE, DARK MOTION, MIDNIGHT PAD, VAPOR KEY, TECH SNAP.
DIGITAL 29-36: FM WOBBLE, MODERN GROWL, GLASS PLUCK, CINEMA HIT, NEON SHIM, RISER FX, VOCAL FM,
DROP SUB. All 16 are in the curated BANK browser of the device (PRESETS knob) and recorded in
meta/SPIRALFM1-SOUNDBANK.json; level trims measured with
tests/spiralfm1_preset_level.c (BS.1770, target -20 LUFS as in 0.16), checked by
tests/spiralfm1_motion_pack_test.c in tests/run_tests.sh.

## SPIRALFM1 1.0: own mark, retuned sounds, new drums
An own version instead of a fork: name, wordmark and spiral mark newly drawn
(`tools/gen_spiralfm1_svg.py`, `tools/gen_spiralfm1_logo.py`), USB product "SPIRALFM1",
manufacturer "Spiralfractal", title in the menu and the splash line "BASED ON MYCELIUM".
Renamed the same way everywhere: SYSTEM/ABOUT writes "MYCELIUM / SLOOP / FELUCCA" under the own
name, recovery shows "SPIRALFM1 USB RESCUE", the editor reports "SPIRALFM1 1.0" as engine
identity, web editor and installer carry the new name (their footers keep the origin), and the
package is `spiralfm1-<version>.fwsc`.

Sound bank: the fungus / forest named factory presets are now named after the spiral, and every
sound is additionally retuned (algorithm and ratio choices, modulation index, envelopes, LFO
target, rate and shape, FX sends; on ANALOG oscillator mix, cutoff, resonance and drive):

| Mycelium 1.0 | SPIRALFM1 1.0 |
| --- | --- |
| ROOT BASS | SPIRAL BASS |
| SPORE TALK | VORTEX TALK |
| HYPHA BLIP | PHASE BLIP |
| BARK GROWL | ORBIT GROWL |
| MOSS DRONE | HELIX DRONE |
| GILL METAL | TORUS METAL |
| FUNGAL DROP | FRACTAL DROP |
| NIGHT SPORE | NIGHT WHORL |
| ACID FOAM | ACID COIL |
| SWAMP CHIRP | PHASE CHIRP |
| SWAMP SQ | VORTEX SQ |
| SPORE BURST | COIL BURST |

The MODERN MOTION packs (ANALOG and DIGITAL) keep their names but are likewise retuned;
PSY BASS, HI BASS, GOA ACID, PSY KICK, METAL HIT, RUST IMPACT, HITECH ZAP, LASER FALL and
LIQUID FM stay, with corrected values.

Drums: all 15 hits newly synthesised — kick with an FM sideband instead of a noise click,
four-partial metal for the hats, band-pass shaped claps, two-tone snare. Each role has its
own peak target; all kits stay within about 1 dB of the old loudness, and the spectral balance
of hi-hat and snare stays open at the top.

Level: every preset re-measured (`tests/spiralfm1_preset_level.c`, BS.1770, target -20 LUFS),
trims in `firmware/src/spiralfm1_trim.h`. Golden renders, UI and song checks and the noise run
are updated with the new sounds (`sh tests/run_tests.sh`).

## Other languages
German (the original text): [docs/SPIRALFM1-de.md](docs/SPIRALFM1-de.md) ·
Russian: [docs/SPIRALFM1-ru.md](docs/SPIRALFM1-ru.md)
