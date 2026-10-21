# Changelog

All notable changes to SPIRALFM1. Format: [Keep a Changelog](https://keepachangelog.com/).
SPIRALFM1 is an independent firmware built on Mycelium 1.0 (Mandalars), which is itself a
derivative of SLOOP (isod89/sloop-fm1) and Felucca (hugelton/Felucca); the upstream history is
summarised at the bottom and lives in `docs/MANUAL.md` and `meta/`.

## [1.0] - 2026-10-20

First SPIRALFM1 release, spun off from Mycelium 1.0 as its own project.

### Added
- Own identity: spiral mark and wordmark drawn from geometric primitives
  (`tools/gen_spiralfm1_svg.py` → `assets/spiralfm1/`), new splash bitmap
  (`tools/gen_spiralfm1_logo.py`), UI title, USB product "SPIRALFM1" and manufacturer
  "Spiralfractal".
- `meta/SPIRALFM1-SOUNDBANK.json` and the `meta/BASE-MYCELIUM-*.json` records of the release
  this project started from.
- Version notes in three languages: `SPIRALFM1.md` in English at the root, the German original in
  `docs/SPIRALFM1-de.md` and a Russian translation in `docs/SPIRALFM1-ru.md`.

### Changed
- Renamed the project everywhere: files, tests, tools, docs, CI, installer site and the
  `spiralfm1-*` package name; on the device the SYSTEM/ABOUT page credits Mycelium, SLOOP and
  Felucca, the recovery screen reads "SPIRALFM1 USB RESCUE", the editor's INFO string is
  "SPIRALFM1 1.0" and the web editor / installer pages carry the new name (with the same
  credits kept in their footers).
- Sound: the twelve fungal/forest factory presets are renamed to the spiral set (SPIRAL BASS,
  VORTEX TALK, PHASE BLIP, ORBIT GROWL, HELIX DRONE, TORUS METAL, FRACTAL DROP, NIGHT WHORL,
  ACID COIL, PHASE CHIRP, VORTEX SQ, COIL BURST) and every one of them — plus the MODERN MOTION
  pack — is retuned (algorithm and ratio choices, modulation index, envelopes, LFO target /
  rate / shape, FX sends).
- Drums: the 15 procedural one-shots of PSY KICK, FOREST and HITECH are regenerated from a new
  synthesis (`tools/gen_spiral_drums.py`): FM-swept kicks, four-partial metal hats, band-passed
  claps; per-role peak targets keep every kit within ~1 dB of the old loudness.
- Level trims re-measured for the retuned presets (`firmware/src/spiralfm1_trim.h`).

## Base: Mycelium 1.0 - 2026-10-08 (Mandalars)

Released by Lars (Mandalars) as Mycelium 1.0, from 0.16-test.

### Added (0.x series, as Mycelium)
- MODERN MOTION sound pack: 8 ANALOG + 8 DIGITAL (FM) factory presets, original designs after the
  movement-bass, growl, wobble and big-room sound of modern soft synths; browsable on device.
- Six curated grooves (Forest / Psy / Goa / HiTech / TeKK), 32 steps each; four factory
  songs of 240 bars (Forest Odyssey, Psy Ascension, Tekk Reactor, Hitech Portal).
- 21 curated factory presets (PSY/GOA/HiTech additions to ANALOG and DIGITAL), three original
  procedural drum kits (PSY KICK, FOREST, HITECH) with 15 self-generated drum one-shots
  (`tools/gen_spiral_drums.py`, provenance in `assets/spiralfm1/samples/`).
- PSY FX page: shared LP for bass+kick, synced echo+reverb, feedback+low-end drive, alien
  ring modulation; later ALIEN+ with stutter. Per-track probability, kick envelope, direct
  SAVE of song banks, drum distortion, ARP latch and roll, logo splash.
- Original icon atlas (86 glyphs, generated from scratch), USB name "Mycelium",
  manufacturer "Mandalars".
- Repository layout: manual and French quickstart in `docs/`, release metadata in `meta/`,
  Windows launchers in `tools/windows/`, third-party licence texts in `LICENSES/`; CI
  (`.github/workflows/`) builds, tests, publishes the Pages installer site and the release.
- `tools/get_sdk_files.sh`: fetches only the three AC79 SDK files the package needs, with their
  SHA-256 checked, instead of cloning the whole JieLi SDK.

## Base: SLOOP 2.3 (upstream)

Four-track groovebox, nine engines, 68 factory presets, 37 drum kits, song mode, USB audio,
TRS MIDI, MIDI clock, web editor with backup/restore — see `docs/MANUAL.md`.
