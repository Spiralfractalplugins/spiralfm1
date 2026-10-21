// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
// SPIRALFM1 Studio: the firmware's DSP (engine.wasm) in the audio thread.
// The page sends the st_* calls it makes itself, as [[name, ...args]]; the engine renders blocks of
// CTL (32) frames at 44.1 kHz and is resampled (linear) to the context's rate when that is another one.
// samples.bin (the ADPCM blob the flash holds) is copied into SMP_DATA before st_init: SAMPLE and GRAIN
// read it, and GRAIN builds its seek index while it starts, so it has to be in place first.
// Every ~50 ms the page gets the state it shows (playing, the step of each track, voices, the punch-in FX).
class SpiralStudioProcessor extends AudioWorkletProcessor {
  constructor(options) {
    super();
    const x = new WebAssembly.Instance(options.processorOptions.module, {}).exports;
    const s = options.processorOptions.samples;
    if (s && s.length) new Uint8Array(x.memory.buffer, x.st_smp_ptr(), x.st_smp_size()).set(s);
    x.st_init();
    this.x = x;
    this.block = x.st_block();
    this.ratio = x.st_rate() / sampleRate;
    this.index = this.block;
    this.position = 0;
    this.a = [0, 0];
    this.b = [0, 0];
    this.frames = 0;
    this.port.onmessage = ({ data }) => {
      for (const [fn, ...args] of data)
        if (fn.startsWith("st_") && typeof x[fn] === "function") x[fn](...args);
    };
  }
  next() {
    if (this.index === this.block) {
      this.samples = new Int32Array(this.x.memory.buffer, this.x.st_render(), this.block * 2);
      this.index = 0;
    }
    const i = this.index++ * 2;
    this.b[0] = Math.max(-1, Math.min(1, this.samples[i] / 32768));
    this.b[1] = Math.max(-1, Math.min(1, this.samples[i + 1] / 32768));
  }
  process(inputs, outputs) {
    const out = outputs[0];
    if (!out.length) return true;
    const n = out[0].length, x = this.x;
    for (let i = 0; i < n; i++) {
      this.position += this.ratio;                    /* the engine runs at its own rate: st_rate() / sampleRate */
      while (this.position >= 1) {
        this.a[0] = this.b[0];
        this.a[1] = this.b[1];
        this.next();
        this.position -= 1;
      }
      for (let c = 0; c < out.length; c++) out[c][i] = this.a[c & 1] + (this.b[c & 1] - this.a[c & 1]) * this.position;
    }
    this.frames += n;
    if (this.frames >= sampleRate / 20) {
      this.frames = 0;
      this.port.postMessage({
        playing: x.st_playing(),
        steps: [0, 1, 2, 3].map((t) => x.st_step(t)),
        voices: x.st_voices(),
        punch: x.st_punch_now(),
      });
    }
    return true;
  }
}
registerProcessor("spiralfm1-dsp", SpiralStudioProcessor);
