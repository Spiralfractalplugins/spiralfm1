# Changelog

All notable changes to MYCELIUM. Format: [Keep a Changelog](https://keepachangelog.com/).
MYCELIUM is a derivative of SLOOP (isod89/sloop-fm1) and Felucca (hugelton/Felucca);
upstream history lives in `docs/MANUAL.md` and the upstream releases.

## [Unreleased]

### Added
- MODERN MOTION sound pack: 8 ANALOG + 8 DIGITAL (FM) factory presets — original designs
  inspired by the movement-bass, growl, wobble and big-room sound of modern soft synths
  (Vital, Serum 2); no third-party preset data imported. All 16 are browsable on device and
  in `meta/MYCELIUM-SOUNDBANK.json`.
- `tests/mycelium_motion_pack_test.c`: parameter ranges, name lengths, uniqueness and an
  audible/bounded render sweep for every pack preset; wired into `tests/run_tests.sh`.
- CI: `.github/workflows/ci.yml` (build + host tests + artifact), `pages.yml` (installer and
  editor on GitHub Pages, built from the current firmware), `release.yml` (tag v* → `.fwsc`,
  site zip and SHA256SUMS on the release).
- `tools/get_sdk_files.sh`: fetches only the three AC79 SDK files the package needs, with
  their SHA-256 checked, instead of cloning the whole JieLi SDK.
- Issue templates (bug report, sound request) and `CONTRIBUTING.md`.

### Changed
- Repository layout (like jangada / TripDeck): manual and French quickstart moved to
  `docs/`, release metadata to `meta/`, Windows launchers to `tools/windows/`, third-party
  licence texts collected in `LICENSES/`; the generated Pages tree is no longer committed —
  CI publishes it.
- `web/make_site.py` builds a MYCELIUM-branded site (`mycelium-<version>.fwsc`, Mycelium logo).
- Level trims measured for the new presets (`firmware/src/mycelium_trim.h`).

## [1.0] - 2026-10-08

Released by Lars (Mandalars). Basis: 0.16-test; version display only, renamed to MYCELIUM 1.0.

### Added (0.x series, as Mycelium)
- Six curated grooves (Forest / Psy / Goa / HiTech / TeKK), 32 steps each; four factory
  songs of 240 bars (Forest Odyssey, Psy Ascension, Tekk Reactor, Hitech Portal).
- 21 curated factory presets (PSY/GOA/HiTech additions to ANALOG and DIGITAL), three original
  procedural drum kits (PSY KICK, FOREST, HITECH) with 15 self-generated drum one-shots
  (`tools/gen_mandalars_drums.py`, provenance in `assets/mycelium/samples/`).
- PSY FX page: shared LP for bass+kick, synced echo+reverb, feedback+low-end drive, alien
  ring modulation; later ALIEN+ with stutter. Per-track probability, kick envelope, direct
  SAVE of song banks, drum distortion, ARP latch and roll, logo splash.
- Original Mycelium icon atlas (86 glyphs, generated from scratch), USB name "Mycelium",
  manufacturer "Mandalars".

## Base: SLOOP 2.3 (upstream)

Four-track groovebox, nine engines, 68 factory presets, 37 drum kits, song mode, USB audio,
TRS MIDI, MIDI clock, web editor with backup/restore — see `docs/MANUAL.md`.
