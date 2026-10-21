<p align="center"><img src="assets/spiralfm1/spiralfm1-logo.svg" alt="SPIRALFM1" width="340"></p>

<p align="center"><b>Spiral sound: Forest, Goa, Psytrance, Hitech and TeKK on the M-VAVE FM-1.</b><br>
An independent live groovebox firmware, free and open source (GPL-3.0-only), based on
<a href="https://github.com/isod89/sloop-fm1">SLOOP</a> and
<a href="https://github.com/hugelton/Felucca">Felucca</a> by Leo Kuroshita / Hügelton Instruments
and on Mycelium 1.0 by Mandalars.</p>

<p align="center">
<a href="https://spiralfractalplugins.github.io/spiralfm1/"><b>Install from the browser</b></a> ·
<a href="docs/MANUAL.md">Manual (SLOOP base)</a> ·
<a href="SPIRALFM1.md">What SPIRALFM1 adds</a> ·
<a href="https://spiralfractalplugins.github.io/spiralfm1/webapp/editor/">Web editor</a> ·
<a href="https://github.com/Spiralfractalplugins/spiralfm1/releases">Releases</a> ·
<a href="https://github.com/Spiralfractalplugins/spiralfm1/issues">Report a bug</a>
</p>

<p align="center">
<a href="https://github.com/Spiralfractalplugins/spiralfm1/actions/workflows/ci.yml"><img src="https://github.com/Spiralfractalplugins/spiralfm1/actions/workflows/ci.yml/badge.svg" alt="CI"></a>
<img src="https://img.shields.io/badge/license-GPL--3.0-informational" alt="GPL-3.0">
<img src="https://img.shields.io/badge/M--VAVE-FM--1-informational" alt="M-VAVE FM-1">
<a href="https://github.com/Spiralfractalplugins/spiralfm1/releases/latest"><img src="https://img.shields.io/github/v/release/Spiralfractalplugins/spiralfm1?include_prereleases&label=version" alt="version"></a>
</p>

---

SPIRALFM1 turns the FM-1 into a four-track live groovebox with a taste: **six curated grooves**
(FOREST JAM, PSY ROLL, PSY GALLOP, GOA ACID, HITECH, TEKK), **four 240-bar factory songs**
(Forest Odyssey, Psy Ascension, Tekk Reactor, Hitech Portal), a **PSY FX** page (shared bass/kick
lowpass, synced echo + reverb, feedback + low-end drive, alien ring mod and stutter), **per-track
probability**, ARP latch-roll, kick envelopes, direct SAVE — on top of the Mycelium groovebox layer
and everything SLOOP is: three
synth tracks and a drum machine, nine engines, 68+ factory sounds, 40 drum kits, USB audio, TRS
MIDI, MIDI clock, song mode and a web editor with full backup. Nothing to load: everything you
hear, you play.

> **Status: 1.0, beta.** Install at your own risk and [report](https://github.com/Spiralfractalplugins/spiralfm1/issues)
> what you find. Flashing never touches the boot area: you can go back to
> [SLOOP](https://isod89.github.io/sloop-fm1/) or the
> [official firmware](https://www.m-vave.com/download) at any time. Your projects, presets,
> samples and settings survive updates.

## Screenshots

<p align="center"><img src="assets/spiralfm1/spiralfm1-logo.svg" alt="The SPIRALFM1 mark" width="260"></p>

The device photo below is from the Mycelium 1.0 build this firmware started from; the UI text and
splash on SPIRALFM1 read differently now.

<p align="center"><img src="assets/screens/base-mycelium-1.0-display.jpg" alt="The 1.0 base build on a real FM-1" width="420"></p>

More: [assets/screens/](assets/screens) (live pages, layers, the web editor).

## What SPIRALFM1 adds over SLOOP

| | |
| --- | --- |
| **Grooves** | HOME hold → GROOVES: six 32-step factory grooves, 144–196 BPM: Forest quarter kicks with three bass sixteenths, TeKK offbeat bass with end fill, HiTech rolling hats. Loading one never touches your saved project slots. |
| **Songs** | HOME hold → SONGS: four factory songs, 240 bars, 16 sections each (atmosphere → build → drop → break), read-only; USER SONG restores your own order. SAVE from the overview banks the four song screens. |
| **PSY FX** | The FX page opens a dedicated panel: KB LP (one lowpass over dry bass and the kick), SPACE (clocked echo and reverb together), FB+DRV (feedback and low-end drive), ALIEN+ (ring modulation and stutter on lead and FX only). |
| **Sound bank** | 21 curated factory presets browsable on device (Psy / Goa / HiTech / Forest) plus the new **MODERN MOTION pack: 16 more presets** after the movement-bass, wobble, growl and big-room sound of modern soft synths (Vital, Serum 2) — original designs, no imported preset data. Three original drum kits: PSY KICK, FOREST, HITECH, built from 15 procedurally generated one-shots. |
| **Look** | Original spiral mark and wordmark (drawn by `tools/gen_spiralfm1_svg.py`), the 86-glyph pixel icon atlas, neon track colours, USB name "SPIRALFM1", manufacturer "Spiralfractal". |

The full notes: [SPIRALFM1.md](SPIRALFM1.md). Changelog: [CHANGELOG.md](CHANGELOG.md).

## Install

From the browser (Chrome or Edge, Web MIDI): open the
[installer page](https://spiralfractalplugins.github.io/spiralfm1/), put the FM-1 on USB, hold
**POWER → B1** while you tap **Install**. Make a backup first with the
[web editor](https://spiralfractalplugins.github.io/spiralfm1/webapp/editor/) (Projects → Backup).

From the command line (needs `pip3 install mido python-rtmidi`):

```sh
python3 tools/fm1_install.py build/felucca.fwsc
python3 tools/fm1_install.py --info          # identity of the connected FM-1
```

Windows: `tools/windows/INSTALL-SPIRALFM1.bat` builds in WSL and opens the installer locally.

## Your first beat

1. HOME hold → **GROOVES** → pick FOREST JAM → **OCT+** twice to confirm (transport stopped).
2. Play any key: the beat starts. OCT+ / OCT- transpose, the four knobs live on the track page.
3. FX tap: PSY FX; ARP tap: GALLOP → OFFBEAT → FULL-ON bass patterns.

Details in [SPIRALFM1.md](SPIRALFM1.md) and the [SLOOP manual](docs/MANUAL.md) (everything else is
SLOOP's: tracks, layers, recording, drums, songs, MIDI, USB audio, the editor).

## Build and tests

```sh
pip3 install Pillow
tools/get_toolchain.sh && tools/get_sdk_files.sh   # once
./build.sh                                         # -> build/felucca.fwsc
sh tests/run_tests.sh                              # every host test, no hardware
```

See [BUILDING.md](BUILDING.md) (macOS needs Docker; CI runs on Linux) and
[CONTRIBUTING.md](CONTRIBUTING.md) — hear any preset without a device with
`build/host/hostsim ENGINE PRESET MONO out.wav`.

## Compatibility

Any M-VAVE FM-1 on the official firmware V15 (the update page starts from it). The package
identity is `FM-1_900`; provenance of every release: [meta/](meta).

## Documentation

- [SPIRALFM1.md](SPIRALFM1.md) — what SPIRALFM1 adds, version by version ([Deutsch](docs/SPIRALFM1-de.md), [Русский](docs/SPIRALFM1-ru.md))
- [docs/MANUAL.md](docs/MANUAL.md) — the full SLOOP base manual (every page, layer, sound, kit)
- [docs/quickstart-fr.md](docs/quickstart-fr.md) — démarrage rapide en français
- [web/EDITOR_PROTOCOL.md](web/EDITOR_PROTOCOL.md) — the editor's SysEx protocol
- [BUILDING.md](BUILDING.md) · [CHANGELOG.md](CHANGELOG.md) · [LICENSING.md](LICENSING.md)

## Credits and basis

SPIRALFM1 is an independent firmware, not a fork of one repository but a build of its own on top
of three:

- **Mycelium 1.0** (Mandalars, 2026) — the groovebox layer this version starts from: the six
  curated grooves, the four 240-bar factory songs, the PSY FX panel, per-track probability, the
  curated sound bank and the icon atlas. Renamed artwork, retuned presets and regenerated drums
  since then; the base release is recorded in [`meta/BASE-MYCELIUM-*.json`](meta), this one in
  [`meta/SPIRALFM1-PROVENANCE.json`](meta/SPIRALFM1-PROVENANCE.json).
- [Leo Kuroshita / Hügelton Instruments](https://hugelton.com) — Felucca, the platform all of this runs on
- [3dSam / isod89](https://github.com/isod89/sloop-fm1) — SLOOP, the groovebox base
- The FM-1 hacking community: [awesome-fm-1](https://github.com/cicloid/awesome-fm-1) and
  [FM-1-RE](https://github.com/AL-255/FM-1-RE) for the reverse-engineered update path

## Licence

GPL-3.0-only for the code; assets are separately licensed (CC0 samples, OFL and MIT fonts) — the
full map with texts in [`LICENSING.md`](LICENSING.md) and [`LICENSES/`](LICENSES).

"M-VAVE" and "FM-1" are trademarks of their respective owners. SPIRALFM1 is independent firmware:
not affiliated with, endorsed by or supported by them. SPIRALFM1 never enables the Bluetooth /
Wi-Fi radio of the hardware.
