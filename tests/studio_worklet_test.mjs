// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
/* The AudioWorklet in node: the browser's audio thread without a browser.
 *
 * web/studio/worklet.js is used as it is - no copy, no rewrite. A fake AudioWorkletProcessor and
 * a fake registerProcessor catch it, and the test plays blocks through it the way a browser does
 * (128 frames per process() call). What it proves: the engine really runs in the audio thread,
 * the transport starts, the sequencer moves and the samples come out.
 *
 * The host test (tests/studio_wasm_test.mjs) calls st_render() itself, so it cannot see a bug in
 * the resampling loop of the worklet. That is exactly what this file is for.
 */
import { readFileSync, existsSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = join(HERE, "..");
const WASM = join(ROOT, "build/studio/engine.wasm"), BIN = join(ROOT, "build/studio/samples.bin");
if (!existsSync(WASM) || !existsSync(BIN)) {
  console.log("studio worklet: engine.wasm is not built, the test is skipped (python3 tools/build_studio.py)");
  process.exit(0);
}

let fails = 0;
const ok = (cond, what) => { console.log(`  ${cond ? "ok  " : "FAIL"}   ${what}`); if (!cond) fails++; };

/* ---- the browser's globals, so that the worklet file runs untouched ---- */
class FakePort {
  constructor() { this.onmessage = null; this.sent = []; }
  postMessage(m) { this.sent.push(m); }
  send(m) { if (this.onmessage) this.onmessage({ data: m }); }
}
class FakeProcessor {
  constructor() { this.port = new FakePort(); }
}
let Processor = null;
globalThis.AudioWorkletProcessor = FakeProcessor;
globalThis.registerProcessor = (name, cls) => {
  if (name !== "spiralfm1-dsp") throw new Error("the worklet registers itself as " + name);
  Processor = cls;
};

const make = (rate) => {
  globalThis.sampleRate = rate;
  Processor = null;
  new Function(readFileSync(join(ROOT, "web/studio/worklet.js"), "utf8"))();
  const module = new WebAssembly.Module(readFileSync(WASM));
  const samples = new Uint8Array(readFileSync(BIN));
  const p = new Processor({ processorOptions: { module, samples } });
  const N = 128, out = [[new Float32Array(N), new Float32Array(N)]];
  let peak = 0, frames = 0;
  const run = (n) => {
    for (let i = 0; i < n / N; i++) {
      p.process([], out);
      for (const ch of out[0]) for (const v of ch) peak = Math.max(peak, Math.abs(v));
      frames += N;
    }
  };
  const state = () => (p.port.sent.length ? p.port.sent[p.port.sent.length - 1] : null);
  return { p, run, state, peak: () => peak, frames: () => frames };
};

/* ---- at the engine's own rate (44.1 kHz) ---- */
{
  const t = make(44100);
  t.run(128 * 20);
  ok(t.peak() === 0, "nothing plays before PLAY is pressed");
  t.p.port.send([["st_play", 1]]);
  t.run(128 * 400);                       /* ~1.2 s of audio */
  const s = t.state();
  ok(!!s && s.playing === 1, "the transport runs in the audio thread (playing = 1)");
  ok(!!s && typeof s.steps[0] === "number", "the state carries the step of every track");
  ok(t.peak() > 0.001, `the groove comes out (peak ${t.peak().toFixed(4)})`);
  const first = s ? s.steps.slice() : [];
  t.run(128 * 800);                       /* ~2.4 s more */
  const s2 = t.state();
  ok(!!s2 && s2.steps.some((v, i) => v !== first[i]), "the sequencer moves (the steps change)");
  const maxVoices = Math.max(...t.p.port.sent.map((m) => (m && m.voices) || 0));
  ok(maxVoices >= 1, `voices are alive (up to ${maxVoices} at once)`);
  t.p.port.send([["st_play", 0]]);
  t.run(128 * 200);
  const s3 = t.state();
  ok(!!s3 && s3.playing === 0, "STOP stops the transport");
}

/* ---- another context rate: the engine is resampled, not stretched ---- */
{
  const t = make(48000);
  t.p.port.send([["st_play", 1]]);
  t.run(128 * 600);
  const s = t.state();
  ok(!!s && s.playing === 1, "at 48 kHz the transport runs too");
  ok(t.peak() > 0.001, `at 48 kHz the groove comes out (peak ${t.peak().toFixed(4)})`);
}

console.log(fails ? "studio worklet: FAILED" : "studio worklet: ok");
process.exit(fails ? 1 : 0);
