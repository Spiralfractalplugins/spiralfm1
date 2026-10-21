// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
/* The studio page in a real browser: the DOM the visitor actually sees.
 *
 * The page is served as it is (web/studio/index.html, web/studio/worklet.js) with the engine
 * (build/studio/engine.wasm) and the samples next to it, and a headless Chrome drives it over
 * the DevTools protocol. No third-party code, no rewriting of the page.
 *
 * What it proves - the two bugs a node test cannot see:
 *   - every parameter the page shows is readable: a drop-down of names or a slider with a value.
 *     (The page used to ask the engine for a *name pointer* and test it for truthiness: a pointer
 *     to an empty string is not 0, so every parameter became a drop-down of blank options.)
 *   - a redraw replaces the rows, it does not add them: choosing a value used to append another
 *     row of controls to every parameter block, again and again.
 *
 * Skipped when there is no browser or no engine build.
 */
import { readFileSync, existsSync, mkdirSync, rmSync, readdirSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { createServer } from "node:http";
import { spawn } from "node:child_process";
import { tmpdir, homedir } from "node:os";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = join(HERE, "..");
const PAGE = join(ROOT, "web/studio/index.html");
const WORKLET = join(ROOT, "web/studio/worklet.js");
const WASM = join(ROOT, "build/studio/engine.wasm");
const BIN = join(ROOT, "build/studio/samples.bin");

let fails = 0;
const ok = (cond, what) => { console.log(`  ${cond ? "ok  " : "FAIL"}   ${what}`); if (!cond) fails++; };
const skip = (why) => { console.log(`studio dom: skipped (${why})`); process.exit(0); };

if (!existsSync(PAGE) || !existsSync(WORKLET)) skip("no web/studio");
if (!existsSync(WASM) || !existsSync(BIN)) skip("engine.wasm is not built (python3 tools/build_studio.py)");

/* ---- the browser: Chrome / Chromium / Edge, or the one a playwright install left behind ---- */
function find_browser() {
  const list = [];
  if (process.env.CHROME_PATH) list.push(process.env.CHROME_PATH);
  const mac = "/Applications";
  for (const app of ["Google Chrome.app", "Chromium.app", "Microsoft Edge.app", "Brave Browser.app"])
    list.push(join(mac, app, "Contents/MacOS", app.replace(".app", "")));
  list.push("/usr/bin/google-chrome", "/usr/bin/google-chrome-stable", "/usr/bin/chromium",
    "/usr/bin/chromium-browser", "/snap/bin/chromium");
  const cache = join(homedir(), "Library/Caches/ms-playwright");
  if (existsSync(cache)) for (const d of readdirSync(cache)) {
    const c = join(cache, d, "chrome-mac-arm64", "Google Chrome for Testing.app", "Contents/MacOS", "Google Chrome for Testing");
    if (existsSync(c)) list.push(c);
    const l = join(cache, d, "chrome-linux", "chrome");
    if (existsSync(l)) list.push(l);
  }
  return list.find((p) => existsSync(p)) || null;
}
const browser = find_browser();
if (!browser) skip("no Chrome / Chromium / Edge found (set CHROME_PATH)");

/* ---- the page's own files, served as the site serves them ---- */
const TYPES = { ".html": "text/html", ".js": "text/javascript", ".wasm": "application/wasm", ".bin": "application/octet-stream" };
const FILES = { "/index.html": [PAGE, ".html"], "/worklet.js": [WORKLET, ".js"], "/engine.wasm": [WASM, ".wasm"], "/samples.bin": [BIN, ".bin"] };
const server = createServer((req, res) => {
  const hit = FILES[new URL(req.url, "http://x").pathname];
  if (!hit) { res.writeHead(404); res.end(); return; }
  res.writeHead(200, { "content-type": TYPES[hit[1]] });
  res.end(readFileSync(hit[0]));
});
await new Promise((r) => server.listen(0, "127.0.0.1", r));
const PORT = server.address().port;

/* ---- the browser, headless, with the DevTools port it picks itself ---- */
const profile = join(tmpdir(), "sfm1-dom-" + process.pid);
mkdirSync(profile, { recursive: true });
const chrome = spawn(browser, [
  "--headless=new", "--remote-debugging-port=0", `--user-data-dir=${profile}`,
  "--no-sandbox", "--disable-gpu", "--mute-audio", "--no-first-run", "--disable-extensions",
  `http://127.0.0.1:${PORT}/index.html`,
], { stdio: ["ignore", "pipe", "pipe"] });
const done = (code) => { try { chrome.kill(); } catch (e) { /* */ } server.close(); try { rmSync(profile, { recursive: true, force: true }); } catch (e) { /* */ } process.exit(code); };

async function devtools_port() {
  for (let i = 0; i < 100; i++) {
    const f = join(profile, "DevToolsActivePort");
    if (existsSync(f)) {
      const txt = readFileSync(f, "utf8").split("\n");
      if (txt[0]) return +txt[0];
    }
    await new Promise((r) => setTimeout(r, 100));
  }
  return null;
}
const dport = await devtools_port();
if (!dport) done(1), console.log("  FAIL   the browser did not open a DevTools port");

/* ---- the page's target, and the DevTools protocol on it ---- */
let ws = null, msgId = 0;
const pending = new Map();
async function connect() {
  for (let i = 0; i < 100; i++) {
    const list = await (await fetch(`http://127.0.0.1:${dport}/json/list`)).json().catch(() => []);
    const page = list.find((t) => t.type === "page" && t.url.includes("index.html"));
    if (page) {
      ws = new WebSocket(page.webSocketDebuggerUrl);
      await new Promise((r, j) => { ws.onopen = r; ws.onerror = j; });
      ws.onmessage = (ev) => {
        const m = JSON.parse(ev.data);
        if (m.id && pending.has(m.id)) { const p = pending.get(m.id); pending.delete(m.id); m.error ? p.j(new Error(JSON.stringify(m.error))) : p.r(m.result); }
      };
      return true;
    }
    await new Promise((r) => setTimeout(r, 100));
  }
  return false;
}
if (!await connect()) { console.log("  FAIL   the browser has no page target"); done(1); }
const send = (method, params = {}) => new Promise((r, j) => { const i = ++msgId; pending.set(i, { r, j }); ws.send(JSON.stringify({ id: i, method, params })); });
const evaluate = async (expression) => {
  const res = await send("Runtime.evaluate", { expression, awaitPromise: true, returnByValue: true });
  if (res.exceptionDetails) throw new Error(res.exceptionDetails.exception?.description || "the page threw");
  return res.result.value;
};
await send("Runtime.enable");

/* the engine boots asynchronously: wait for the page to draw its controls
 * (the static HTML holds empty .ctl placeholders, so they do not count) */
const READY = "document.getElementById('engine').options.length > 0 && document.querySelectorAll('.ctl > select').length > 0";
let ready = false;
for (let i = 0; i < 150 && !ready; i++) {
  ready = await evaluate(READY).catch(() => false);
  if (!ready) await new Promise((r) => setTimeout(r, 100));
}
if (!ready) { console.log("  FAIL   the page drew no controls (the engine did not boot)"); done(1); }

/* ---- what the visitor sees ---- */
const CHECK = `(() => {
  const vis = (el) => el.getClientRects().length > 0;
  const name = (s) => s.id || (s.parentElement?.querySelector("label")?.textContent || "?");
  const res = {};
  const selects = [...document.querySelectorAll("select")].filter(vis);
  res.selects = selects.length;
  res.emptyOptions = [];
  res.blankSelected = [];
  for (const s of selects) {
    for (const o of s.options) if (!o.text.trim()) res.emptyOptions.push(name(s) + "[" + o.value + "]");
    if (!s.selectedOptions.length || !s.selectedOptions[0].text.trim()) res.blankSelected.push(name(s));
  }
  res.noValue = [];
  for (const r of document.querySelectorAll(".ctl input[type=range]")) {
    if (!vis(r)) continue;
    const l = r.parentElement.querySelector("label");
    const v = l && l.querySelector("span:last-child");
    if (!v || !v.textContent.trim()) res.noValue.push(l ? l.textContent : "?");
  }
  const rows = () => [...document.querySelectorAll("[data-p]")].map((b) => b.querySelectorAll(":scope > .ctl").length);
  res.rowsBefore = rows();
  res.rowsWant = [...document.querySelectorAll("[data-p]")].map((b) => b.dataset.p.trim().split(/\\s+/).length);
  /* three redraws: a drop-down, another engine, another track */
  const enumSel = [...document.querySelectorAll(".ctl select")].find((s) => vis(s) && s.options.length > 1);
  if (enumSel) { enumSel.value = String(+enumSel.value + 1); enumSel.dispatchEvent(new Event("change")); }
  res.rowsAfterEnum = rows();
  const eng = document.getElementById("engine");
  eng.value = String((+eng.value + 1) % eng.options.length); eng.dispatchEvent(new Event("change"));
  res.rowsAfterEngine = rows();
  const tabs = document.querySelectorAll("#tabs button");
  if (tabs[1]) tabs[1].click();
  res.rowsAfterTab = rows();
  /* the drum track (the last one): its kit list is filled with the kits' names */
  const last = tabs[tabs.length - 1];
  if (last) last.click();
  res.rowsAfterDrum = rows();
  const kit = document.getElementById("kit");
  res.kitOptions = kit.options.length;
  res.kitEmpty = [...kit.options].filter((o) => !o.text.trim()).length;
  res.kitVisible = vis(kit);
  return res;
})()`;
const res = await evaluate(CHECK);

ok(res.selects > 0, `the page draws its controls (${res.selects} drop-downs, ${res.rowsBefore.reduce((a, b) => a + b, 0)} rows)`);
ok(!res.emptyOptions.length, `every option of every drop-down is named${res.emptyOptions.length ? " (blank: " + res.emptyOptions.slice(0, 5).join(" ") + ")" : ""}`);
ok(!res.blankSelected.length, `every drop-down shows its value${res.blankSelected.length ? " (blank: " + res.blankSelected.join(" ") + ")" : ""}`);
ok(!res.noValue.length, `every slider shows its value${res.noValue.length ? " (blank: " + res.noValue.slice(0, 5).join(" ") + ")" : ""}`);
const same = (a, b) => a.length === b.length && a.every((v, i) => v === b[i]);
ok(same(res.rowsBefore, res.rowsWant), "every parameter block holds one control per parameter");
ok(same(res.rowsAfterEnum, res.rowsBefore) && same(res.rowsAfterEngine, res.rowsBefore) && same(res.rowsAfterTab, res.rowsBefore) && same(res.rowsAfterDrum, res.rowsBefore),
  `a redraw replaces the rows, it does not add them (${res.rowsBefore.join(",")} -> ${res.rowsAfterEnum.join(",")} -> ${res.rowsAfterEngine.join(",")} -> ${res.rowsAfterTab.join(",")} -> ${res.rowsAfterDrum.join(",")})`);
ok(res.kitOptions > 0 && !res.kitEmpty && res.kitVisible, `the drum track's kit list is filled (${res.kitOptions} kits)`);

console.log(fails ? `studio dom: ${fails} failed` : "studio dom: ok");
done(fails ? 1 : 0);
