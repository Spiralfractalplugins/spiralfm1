// SPDX-License-Identifier: GPL-3.0-only
// SPIRALFM1 Studio: the wasm engine checked without a browser (node tests/studio_wasm_test.mjs).
// It is the studio's own audio thread: load samples.bin into linear memory the way worklet.js does,
// power the engine up, and render. What it has to prove is the part the firmware tests cannot see:
// that the ADPCM blob landed where SMP_DATA points, and that SAMPLE, GRAIN and the sampled
// SPIRALFM1 kits (PSY KICK / FOREST / HITECH) actually sound out of the .wasm.
import { readFileSync, writeFileSync, existsSync } from "node:fs";
import { createHash } from "node:crypto";

const ROOT = new URL("..", import.meta.url).pathname;
const dir = process.env.STUDIO_DIR || ROOT + "build/studio";
if (!existsSync(dir + "/engine.wasm") || !existsSync(dir + "/samples.bin")) {
  console.log("studio wasm: not built (python3 tools/build_studio.py) - skipped");
  process.exit(0);
}
let fail = 0;
const ok = (name, cond, extra = "") => {
  console.log((cond ? "  ok   " : "  FAIL ") + name + (extra ? "  " + extra : ""));
  if (!cond) fail++;
};
const sha = (a) => createHash("sha256").update(new Uint8Array(a.buffer, a.byteOffset, a.byteLength)).digest("hex").slice(0, 12);

// one boot = one instance, the samples in place before st_init (GRAIN reads them at once)
async function boot() {
  const { instance } = await WebAssembly.instantiate(readFileSync(dir + "/engine.wasm"), {});
  const x = instance.exports;
  const bin = new Uint8Array(readFileSync(dir + "/samples.bin"));
  new Uint8Array(x.memory.buffer, x.st_smp_ptr(), x.st_smp_size()).set(bin);
  x.st_init();
  const cstr = (p) => {
    if (!p) return "";
    const b = new Uint8Array(x.memory.buffer);
    let e = p;
    while (b[e]) e++;
    return new TextDecoder().decode(b.subarray(p, e));
  };
  const render = (n) => {
    const f = new Int16Array(n * x.st_block() * 2);
    for (let i = 0; i < n; i++) {
      const b = new Int32Array(x.memory.buffer, x.st_render(), x.st_block() * 2);
      for (let k = 0; k < b.length; k++) f[i * b.length + k] = Math.max(-32768, Math.min(32767, b[k]));
    }
    return f;
  };
  const secs = (s) => Math.round(s * x.st_rate() / x.st_block());
  const at = (name) => {
    for (let k = 0; k < x.st_id_count(); k++) if (cstr(x.st_id_name(k)) === name) return x.st_id(k);
    throw new Error("no such id: " + name);
  };
  const ID = { P_E0: at("P_E0"), P_SLEN: at("P_SLEN") };
  for (const n of ["P_LEVEL", "P_PAN", "P_MUTE", "P_ATK", "P_REL", "P_E1", "P_E2", "P_E3", "P_REV",
                   "G_BPM", "G_SWING", "G_DUCK", "G_DUST"]) ID[n] = at(n);
  return { x, cstr, render, secs, ID, engine: (i) => cstr(x.st_engine_name(i)) };
}
const ST_NOTE = 0, ST_TIE = 1, ST_REST = 2;      // core.h: the step's time
const rms = (f) => {
  let s = 0;
  for (let i = 0; i < f.length; i++) s += f[i] * f[i];
  return Math.round(Math.sqrt(s / f.length));
};
const play = (m, key, hold = 1, after = 0.3) => {                // one key held on the selected track
  m.x.st_keys(key);
  const f = m.render(m.secs(hold));
  m.x.st_keys(0);
  m.render(m.secs(after));
  return f;
};

const m = await boot();
const { x, cstr, render, secs, ID } = m;
const NAMES = [...Array(x.st_engines()).keys()].map((e) => m.engine(e));
ok("engine powered up", x.st_ready() === 1 && x.st_rate() === 44100 && x.st_block() === 32,
  `${x.st_rate()} Hz, ${x.st_block()} frames, ${x.st_tracks()} tracks, ${x.st_engines()} engines`);
ok("samples landed in SMP_DATA", x.st_smp_size() === readFileSync(dir + "/samples.bin").length &&
  sha(new Uint8Array(x.memory.buffer, x.st_smp_ptr(), x.st_smp_size())) ===
  sha(new Uint8Array(readFileSync(dir + "/samples.bin"))));
ok("SAMPLE and GRAIN are built", NAMES.includes("SAMPLE") && NAMES.includes("GRAIN"), NAMES.join(" "));

// ---- the power-on scene: the factory groove on the kit the device boots with
x.st_play(1);
const groove = render(secs(2.5));
ok("factory groove sounds", rms(groove) > 200 && x.st_playing() === 1, `rms ${rms(groove)}, voices ${x.st_voices()}`);
x.st_play(0);
render(secs(0.5));

// ---- the sampled SPIRALFM1 kicks, live on the keys of the drum track
const KIT = {};
for (let k = 0; k < x.st_kits(); k++) KIT[cstr(x.st_kit_name(k))] = k;
x.st_select(3);
const kick = {};
for (const name of ["PSY KICK", "FOREST", "HITECH"]) {
  x.st_param(3, ID.P_E0, KIT[name]);
  const f = play(m, 1);
  kick[name] = [rms(f), sha(f)];
  ok(`${name} sounds`, kick[name][0] > 200, `kit #${KIT[name]}, rms ${kick[name][0]}`);
  ok(`${name} reads back as its own kit`, cstr(x.st_param_name(3, ID.P_E0, KIT[name])) === name);
}
ok("the three kicks are three sounds", new Set(Object.values(kick).map((v) => v[1])).size === 3);

// ---- SAMPLE: every built-in set decodes out of the blob
const SAMPLE = NAMES.indexOf("SAMPLE"), GRAIN = NAMES.indexOf("GRAIN");
function set_shot(set) {
  x.st_select(0);
  x.st_track_engine(0, SAMPLE);
  x.st_param(0, ID.P_SLEN, 1);
  x.st_param(0, ID.P_E0, set);
  return play(m, 1 << 12);
}
const setSig = {};
for (let s = 0; s < x.st_smp_sets(); s++) {
  const name = cstr(x.st_smp_set_name(s)), f = set_shot(s);
  setSig[name] = sha(f);
  ok(`SAMPLE ${name} sounds`, rms(f) > 100, `rms ${rms(f)}`);
}
ok("the sets are distinct", new Set(Object.values(setSig)).size === x.st_smp_sets(),
  `${new Set(Object.values(setSig)).size} of ${x.st_smp_sets()}`);
ok("GRAIN names its sources after SAMPLE", (() => {
  x.st_select(0);
  x.st_track_engine(0, GRAIN);
  return cstr(x.st_param_name(0, ID.P_E0, 0)) === cstr(x.st_smp_set_name(0));
})());

// ---- every engine sounds and EDIT 1 reads as its own control (what the page builds from)
for (let e = 0; e < x.st_engines(); e++) {
  x.st_select(0);
  x.st_track_engine(0, e);
  x.st_param(0, ID.P_SLEN, 1);
  const f = play(m, 1 << 12);
  ok(`${m.engine(e)}: sounds, EDIT 1 described, presets listed`,
    rms(f) > 60 && cstr(x.st_param_label(0, ID.P_E0)).length > 0 &&
    cstr(x.st_param_text(0, ID.P_E0)).length > 0 && x.st_presets(e) > 0 &&
    cstr(x.st_preset_name(e, 0)).length > 0,
    `${cstr(x.st_param_label(0, ID.P_E0))}=${cstr(x.st_param_text(0, ID.P_E0))}, ` +
    `${x.st_presets(e)} presets, first "${cstr(x.st_preset_name(e, 0))}"`);
}

// ---- the page's grid: it clears, writes and reads back both kinds of track
x.st_select(3);
x.st_steps_clear(3);
for (const i of [0, 4, 8, 12]) x.st_lane_toggle(i, 0);
ok("four kicks on the grid", [0, 4, 8, 12].every((i) => x.st_lane_get(i, 0) === 1) &&
  [1, 2, 3, 5, 6, 7].every((i) => x.st_lane_get(i, 0) === 0));
x.st_steps_clear(0);
for (const i of [0, 6, 10]) x.st_step_toggle(0, i, 48);
x.st_step_toggle(0, 10, 48);
ok("the pitch grid writes and erases", x.st_step_has(0, 0, 48) === 1 && x.st_step_has(0, 6, 48) === 1 &&
  x.st_step_has(0, 10, 48) === 0 && x.st_step_time(0, 0) === ST_NOTE && x.st_step_time(0, 10) === ST_REST);
x.st_play(1);
const programmed = render(secs(2));
x.st_play(0);
ok("programmed steps play", rms(programmed) > 200, `rms ${rms(programmed)}`);

// ---- determinism: two fresh boots of the same scene render the same bytes
const a = await boot(), b = await boot();
a.x.st_play(1);
b.x.st_play(1);
const fa = a.render(a.secs(1.5)), fb = b.render(b.secs(1.5));
ok("a fresh boot is reproducible", sha(fa) === sha(fb), `rms ${rms(fa)}`);

// ---- the preview a human listens to
x.st_select(0);
x.st_track_engine(0, SAMPLE);
x.st_param(0, ID.P_SLEN, 16);
x.st_param(3, ID.P_E0, KIT.FOREST);
x.st_select(3);
x.st_steps_clear(3);
for (const i of [0, 4, 8, 12]) x.st_lane_toggle(i, 0);
for (const i of [4, 12]) x.st_lane_toggle(i, 2);
x.st_select(0);
x.st_play(1);
const wav = render(secs(6));
x.st_play(0);
writeFileSync(dir + "/preview.wav", wav_bytes(wav));
console.log(`  wrote ${dir}/preview.wav (${(wav.length / x.st_rate() / 2).toFixed(1)} s)`);

function wav_bytes(f) {
  const b = new ArrayBuffer(44 + f.length * 2), v = new DataView(b);
  const s = (o, t) => { for (let i = 0; i < t.length; i++) v.setUint8(o + i, t.charCodeAt(i)); };
  s(0, "RIFF"); v.setUint32(4, 36 + f.length * 2, true); s(8, "WAVEfmt "); v.setUint32(16, 16, true);
  v.setUint16(20, 1, true); v.setUint16(22, 2, true); v.setUint32(24, x.st_rate(), true);
  v.setUint32(28, x.st_rate() * 4, true); v.setUint16(32, 4, true); v.setUint16(34, 16, true);
  s(36, "data"); v.setUint32(40, f.length * 2, true);
  new Int16Array(b, 44).set(f);
  return Buffer.from(b);
}

console.log(fail ? `studio wasm: ${fail} failed` : "studio wasm: ok");
process.exit(fail ? 1 : 0);
