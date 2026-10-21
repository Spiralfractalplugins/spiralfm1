# AGENTS.md — the agent log for this repository

A running record of what the agents did here: the task, the agents involved, the files that
changed, the commands that were run and what those commands printed. It exists so that a fresh
agent — or a human coming back after a month — can pick the work up without re-deriving it.

**How to use it.** Read `## Current state` first: it says what works right now and how to build
and test it. Then scroll to `## Session log` for the history. When a run changes anything in this
repository, append an entry (newest first) and refresh the `## Current state` block.

**Rules for the entries**

- One entry per run, newest first: `### YYYY-MM-DD — what the run was about`.
- Name every file that was created, changed or deleted, with one line on what changed in it.
- Every "it works" claim needs the command that proves it and the result it printed.
- Record the agents that did the work: the main agent and any subagents, and what each did.
- No secrets, no tokens, no private URLs. Keep it a log, not a diary.

---

## Current state

*Updated 2026-10-21.*

### What this repository is

SPIRALFM1 — the firmware for the M-VAVE FM-1 groovebox: an independent build on top of Felucca
and Mycelium by Leo Kuroshita (Hügelton Instruments). Here live the firmware sources, the tools
that build and flash it, the web pages, and the host tests. `SPIRALFM1.md` describes the feature
set, `meta/` the provenance of each release, `BUILDING.md` how to build it.

### What is built and working

| Piece | Where | State |
| --- | --- | --- |
| Device firmware | `firmware/src/` → `./build.sh` → `build/felucca.fwsc` | builds (Docker on macOS, native on Linux CI) |
| Web installer | `web/index_pkg.html` + `web/fm1pkg.js`, `web/fm1ota.js` | Web MIDI installer for the FM-1, ja/en |
| Web editor | `web/editor.html` | the SysEx editor, ja/en |
| **Studio** | `web/studio/` | the firmware's DSP in the browser, as WebAssembly — see below |
| Site | `web/make_site.py` → `site/` | the Pages tree: installer, editor, studio, the package |
| Host tests | `tests/` → `sh tests/run_tests.sh` | the firmware's C tests, the web suite, the studio wasm test, the studio worklet test, the studio dom test |

### The Studio — the browser engine

The point of the Studio is that the page plays **the firmware's own code**, not a
re-implementation. `web/studio/engine.c` includes `firmware/src/{core,engines,drums,params,voice,
slicer,fx,usb,midi_uart,seq}.c` and exports a small `st_*` API; `web/studio/worklet.js` runs that
engine in an AudioWorklet; the page (`web/studio/index.html`, en/ru) reads the firmware's own
tables — the same track table, the same params, the same punch-in names — so a knob reads what
the device reads.

- Build: `python3 tools/build_studio.py` → `build/studio/engine.wasm` (159 KB) +
  `build/studio/samples.bin` (335 KB). macOS has no wasm-ld, so the script runs a debian
  container (`tools/Dockerfile.studio`); on Linux CI it uses the apt `clang` + `lld`.
- The samples are deliberately **not** inside the wasm: `tools/gen_samples.py` writes the same
  ADPCM blob the flash holds into `samples.bin`, and the worklet copies it into linear memory
  before `st_init` (GRAIN builds its seek index at startup, so it must be there first).
- The page fetches only `engine.wasm`, `samples.bin` and `worklet.js`. No third-party code, no CDN,
  no network access beyond its own directory.
- The page's punch-in mapping is `firmware/src/punch.c`'s `punch_key()`; the key→note mapping is
  `seq.c`'s (F3 + key, quantised).
- The audio thread is tested too: `tests/studio_worklet_test.mjs` runs `web/studio/worklet.js`
  under node with a fake `AudioWorkletProcessor`, because the host test calls `st_render()` itself
  and therefore cannot see a bug in the worklet's resampling.
- The page's controls are tested in a real browser: `tests/studio_dom_test.mjs` serves the page
  and the engine, drives a headless Chrome over the DevTools protocol and looks at the DOM the
  visitor sees (every value readable, a redraw replaces the rows instead of adding them). It
  skips itself when there is no browser; GitHub's runners have one, so CI really runs it.

### Commands

```sh
./build.sh                                             # device firmware -> build/felucca.fwsc
python3 tools/build_studio.py                          # browser engine -> build/studio/
sh tests/run_tests.sh                                  # every host test + web suite + studio wasm/worklet/dom tests
node web/test_web.mjs                                  # the web suite alone
node tests/studio_wasm_test.mjs                        # the studio engine alone
node tests/studio_worklet_test.mjs                     # the worklet alone (the audio thread in node)
node tests/studio_dom_test.mjs                         # the page alone (a headless browser, the DOM)
python3 web/make_site.py build/felucca.fwsc 1.0 site   # the Pages tree -> site/
python3 -m http.server 8765 --directory site            # serve it locally
```

### Open items

- **The Studio is live.** Commits `7479519` (the studio) and `5aea70e` (the workflow fix); CI
  green; the site at <https://spiralfractalplugins.github.io/spiralfm1/webapp/studio/> serves the
  engine (verified in a browser against the live URL).
- `site/` is a build output and is now in `.gitignore`; it must never be committed.
- The Studio has no MIDI output, no project save/load and no preset export (only the language
  choice is kept in `localStorage`). It is a player and a sequence editor, not a replacement for
  `web/editor.html`.
- `web/studio/` ships the engine's *own* API; if the firmware's tables change, the page's
  `data-p` / `data-g` attribute names must be checked against `st_id_name()`.

---

## Session log

### 2026-10-21 — Studio: the parameter values are visible again, the rows stop multiplying

- **Task.** The user reported two things on the live Studio: the parameter values were not visible
  in the web interface's drop-downs, and the controls multiplied — a new row appeared on every
  choice. Both were real bugs; the second one was invisible to every test that existed.
- **Agents.** Main agent only.
- **Actions.**
  - `web/studio/engine.c` — two new exports: `st_param_is_enum(t, id)` and `st_global_is_enum(id)`
    (the firmware's own rule: the format is `F_ENUM` — `editor.c` ED_PARAM_GET, `icons.c`
    param_icon). Not "does it have names": the LOFI engine's `WAV#` / `DCY` are `F_INT` **with** a
    name table (`eng_lofi.c`), and the device shows them as a value.
  - `web/studio/index.html` — `paramCtl()` / `globalCtl()` ask the engine instead of testing a name
    *pointer* for truthiness; `refresh()` now clears each `[data-p]` block before it draws the rows.
  - `tests/studio_dom_test.mjs` (new) — the page in a headless Chrome over the DevTools protocol:
    every option is named, every drop-down and every slider shows its value, the rows do not grow.
    Takes `STUDIO_DIR`, skips itself when there is no browser.
  - `tests/studio_wasm_test.mjs` — the engine's enum answers (11 lists, 21 values, the globals, the
    drum kit, and the two LOFI parameters that have names but are values); the id map is now all of
    the engine's ids.
  - `web/test_web.mjs` — the page asks the engine (not a pointer) and a redraw replaces the rows.
  - `tests/run_tests.sh` — runs the dom test next to the wasm and the worklet test.
- **Verification.**
  - `python3 tools/build_studio.py` → `engine.wasm: 159,443 B` (was 159,147 B).
  - `node tests/studio_dom_test.mjs` → *studio dom: ok*: 18 drop-downs (was 70), 45 rows, no blank
    option, no blank value; the rows stay `8,4,8,1,1,5,6,5,3,4` through a choice, an engine change,
    a track change and the drum track. **Checked against the old page: it fails there** — 70
    drop-downs with blank options and the rows `8 → 16 → 24 → 32 → 40` — so it is a real
    regression test.
  - The same test with the *old* `engine.wasm` (the browser may still have it cached) → *ok*: the
    page falls back to "a parameter is a list if every value in its range has a name".
  - `node tests/studio_wasm_test.mjs` → *studio wasm: ok*; `node web/test_web.mjs` → *web tests
    passed*; `sh tests/run_tests.sh` → *ALL HOST TESTS PASSED*.
  - CI run `39570191624` → *success*, with *studio dom: ok* on the runner (it has Chrome); Pages
    run `39570191629` → *success*. The second push (`a8c8d48`, the cached-engine fallback): CI
    `39570811203` → *success*, Pages `39570811354` → *success*.
  - The live page (headless Chrome against the live URL): 18 drop-downs, 0 blank options, 0 blank
    values, 0 sliders without a value; the rows stay `8,4,8,1,1,5,6,5,3,4` through a choice, a
    track change and an engine change.
- **Lesson.** A pointer is not a boolean: `st_param_name()` returns an address, and the address of
  an empty string is not 0. Every parameter therefore became a drop-down, and the options of the
  ones that are not lists were blank. Ask the engine *what the parameter is* (its format), not
  whether a string happens to be there. And a redraw that appends must clear first — the rows grew
  because only the macro block was emptied.

### 2026-10-21 — Push: the studio goes live, and the workflows learn to speak YAML

- **Task.** Commit the studio, push it, and make sure the CI and the Pages deploy survive it.
- **Agents.** Main agent only.
- **Actions.**
  - `7479519` — the whole studio (17 files): the engine, the worklet, the page, the build tools,
    the tests, the site copy, the CI steps, the installer link, the README, this log.
  - `5aea70e` — the fix below, and `release.yml` now builds the studio engine too: it publishes
    the site, and the site carries the Studio.
  - `.github/workflows/{ci,pages,release}.yml` — quoted the wasm toolchain step's name: it held a
    `": "`, so YAML read it as a mapping and **both workflows failed to start** (0 s, "this run
    likely failed because of a workflow file issue").
- **Verification.**
  - CI run `39564932226` → *success*: the studio engine built on the runner's clang
    (`engine.wasm: 158,516 B`), then *web tests passed*, *studio wasm: ok*, *studio worklet: ok*,
    *ALL HOST TESTS PASSED*.
  - Pages run `39564932204` → *success*.
  - The live site: `webapp/studio/{index.html, worklet.js, engine.wasm, samples.bin}` are all
    served (engine.wasm as `application/wasm`), and the installer page has the Studio link.
  - Headless Chromium against the live URL: the engine boots (`Sound on · 44100 Hz`), PLAY becomes
    STOP, the sequencer runs through its steps, 2 voices alive, no console errors.
- **Lesson.** A workflow YAML error looks like an infrastructure failure, not a syntax error:
  look at the run's duration first (0 s = the file never parsed).

### 2026-10-21 — Studio: the browser check caught a silent engine

- **Task.** Verify the studio in a real browser, then make it discoverable (installer link,
  README), keep the build output out of git, and start this log.
- **Agents.** Main agent only (opencode, `minimax-m3.1-flash-free`); no subagents — the work was
  linear and small, so it was done directly instead of delegated.
- **Actions.**
  - `web/studio/worklet.js` (fixed) — **the engine was silent in the browser.** The resampler
    decremented its fractional position but never incremented it, so the worklet rendered exactly
    one 32-frame block and then stopped: the transport request was never consumed, `song.playing`
    stayed 0 and the page sat on "PLAY" for ever. Now `this.position += this.ratio` per frame
    (`ratio = st_rate() / sampleRate`), so the engine's samples are consumed at the engine's own
    rate and the context's rate is resampled, not stretched.
  - `tests/studio_worklet_test.mjs` (new) — the worklet under node: the file is loaded untouched,
    a fake `AudioWorkletProcessor` and a fake `registerProcessor` catch it, and blocks of 128
    frames are played through it. It proves that the transport starts, that the groove comes out,
    that the sequencer moves, that STOP stops, and that 48 kHz resampling works.
  - `tests/run_tests.sh` (changed) — runs the worklet test next to the studio wasm test.
  - `AGENTS.md` (new) — this file: the current state and the session log.
  - `.gitignore` — added `site/` under the build outputs (the `web/make_site.py` output was
    showing up as untracked noise).
  - `web/index_pkg.html` — added a "Studio" link next to the Editor link, plus the `studio:` text
    in the ja and en dictionaries; `.ed + .ed { margin-left: 8px; }` so the two buttons sit apart.
  - `README.md` — new "Play it in the browser (Studio)" section: what it is, the Pages URL, how to
    build it, and the note that the DSP is the firmware's own code.
- **Verification.**
  - `python3 tools/build_studio.py` → clean docker rebuild, `engine.wasm: 159,147 B`,
    `samples.bin: 335,374 B` (byte-identical to the earlier build).
  - `node web/test_web.mjs` → *web tests passed* (the studio section included).
  - `node tests/studio_wasm_test.mjs` → *studio wasm: ok* (all 9 engines sound, the sample sets
    are distinct, the pitch grid writes and erases, programmed steps play, a fresh boot is
    reproducible; `build/studio/preview.wav` written).
  - `node tests/studio_worklet_test.mjs` → *studio worklet: ok* (the groove's peak 0.31, the
    sequencer moves, up to 4 voices at once, 44.1 kHz and 48 kHz). **Checked against the old
    worklet: it fails there** (silent, transport never starts) — so it is a real regression test.
  - `sh tests/run_tests.sh` → *ALL HOST TESTS PASSED*.
  - `python3 web/make_site.py build/felucca.fwsc 1.0 site` → `webapp/studio` 537,890 B with all
    four files.
  - Headless Chromium over CDP (the check that found the bug): the installer page shows the
    Studio link; the studio page boots, POWER starts the AudioWorklet (`Sound on · 44100 Hz`),
    PLAY flips the button to STOP, the step bar runs through all 32 steps, the grid playhead
    moves, the page shows "2 voices" — and **no console output and no exceptions on either page**.

### 2026-10-21 — WebAssembly studio: the firmware's DSP in the browser

- **Task.** Run the firmware in the browser: compile `firmware/src/` to WebAssembly and drive it
  from a page, so SPIRALFM1 can be heard without an FM-1.
- **Agents.** Main agent only; no subagents.
- **Actions.**
  - `web/studio/engine.c` (new) — one translation unit that includes the firmware's sources and
    exports the `st_*` API the page needs (engine, presets, params, sequence, punch, drum grid,
    sample zones). The firmware's tables are the page's tables.
  - `web/studio/worklet.js` (new) — the AudioWorklet processor: instantiates the wasm, copies
    `samples.bin` into linear memory, `st_init()`, renders 32-frame blocks and resamples them
    linearly to the context rate; posts the live state (playing, the step of each track, the
    voice count, the punch-in FX) to the page every ~50 ms.
  - `web/studio/index.html` (new) — the page: 4 tracks, engine/preset selects, the 16 params,
    the 27-key keyboard (QWERTY), the 27×32 step grid, the punch-in row, MIDI in, an
    oscilloscope, en/ru.
  - `tools/build_studio.py` (new) + `tools/Dockerfile.studio` (new) — the build: generate the
    studio flavour of the samples, compile with `clang --target=wasm32`, link with `wasm-ld`.
  - `tools/gen_samples.py` (changed) — a "studio" flavour: the same zones and the same ADPCM, but
    `SMP_DATA` is empty in the header and the raw blob goes to `samples.bin`; a separate cache
    (`build/gen_samples.studio.cache`).
  - `web/make_site.py` (changed) — copies the studio into `site/webapp/studio/`, and says so
    loudly when the engine has not been built.
  - `web/test_web.mjs` (changed) — a studio section: the page's strings exist in en and ru, the
    page fetches nothing but its own three files, the worklet copies the samples before `st_init`,
    the wasm exports what the page calls, and `samples.bin` matches the firmware's blob.
  - `tests/studio_wasm_test.mjs` (new) + `tests/run_tests.sh` (changed) — the deeper engine test,
    run by the suite when `build/studio/engine.wasm` exists.
  - `.github/workflows/ci.yml`, `.github/workflows/pages.yml` (changed) — install `clang` + `lld`,
    build the studio before the site build.
- **Verification.**
  - `node web/test_web.mjs` → *web tests passed*.
  - `node tests/studio_wasm_test.mjs` → *studio wasm: ok*.
  - `python3 web/make_site.py …` → `site/webapp/studio/{engine.wasm, samples.bin, index.html,
    worklet.js}`; all four served with the right MIME type.
  - Headless Chromium over CDP: the page loads, the grid and the tabs render, POWER starts the
    AudioWorklet (`Sound on · 44100 Hz`), no console errors, no uncaught exceptions.
- **Outcome.** The firmware now runs in a browser tab. Open items are in `## Current state`.
