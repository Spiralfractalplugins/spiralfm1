#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
"""Build the browser engine of /webapp/studio/: web/studio/engine.c -> build/studio/engine.wasm.

The DSP is the firmware's own (engines, voices, FX, sequencer, drums, PUNCH-IN), compiled for
wasm32 exactly like tests/hostsim.c compiles it for the host. The SAMPLE/GRAIN material stays out
of the .wasm: gen_samples writes build/gen/studio/samples.bin and a felucca_samples.h in which
SMP_DATA is an empty array - the worklet fetches the blob into linear memory before st_init.
build/gen/studio goes on the include path before build/gen, so that is the header the engine gets.

  python3 tools/build_studio.py [--clang CC --lld LD | --docker]

macOS has no wasm-ld, so the default there is a throwaway debian container with clang + lld
(the same packages the pages workflow installs). --inner is what that container runs.
"""
import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
SRC = HERE.parent
OUT = SRC / "build" / "studio"
GENDIR = SRC / "build" / "gen" / "studio"
IMG = "spiralfm1-studio-builder"
GEN_HDR = "felucca_samples.h"           # the name eng_sample.c includes
GEN_BIN = "samples.bin"


def sh(cmd, **kw):
    print("+ " + " ".join(map(str, cmd)), flush=True)
    return subprocess.run(list(map(str, cmd)), check=True, **kw)


def gen_samples():
    GENDIR.mkdir(parents=True, exist_ok=True)
    sh([sys.executable, HERE / "gen_samples.py", GENDIR / GEN_HDR, GENDIR / GEN_BIN])


def compile_link(clang, lld):
    OUT.mkdir(parents=True, exist_ok=True)
    obj = OUT / "engine.o"
    cflags = ["--target=wasm32", "-O2", "-ffreestanding", "-fno-builtin", "-fvisibility=hidden",
              "-Wall", "-Wno-unused-function", "-Wno-incompatible-library-redeclaration",
              "-Ibuild/gen/studio", "-Ibuild/gen", "-Ifirmware/src"]
    sh([clang, *cflags, "-c", "web/studio/engine.c", "-o", obj], cwd=SRC)
    sh([lld, "--no-entry", "--gc-sections", "-z", "stack-size=262144",
        "-o", OUT / "engine.wasm", obj], cwd=SRC)
    shutil.copyfile(GENDIR / GEN_BIN, OUT / GEN_BIN)


def have(cmd):
    return shutil.which(cmd) is not None


def local_toolchain():
    """(clang, wasm-ld) if this machine can do wasm32 on its own, else None"""
    if os.environ.get("STUDIO_CLANG"):
        clang, lld = os.environ["STUDIO_CLANG"], os.environ.get("STUDIO_LLD")
        return (clang, lld) if lld else None
    for d in ("/opt/homebrew/opt/llvm/bin", "/usr/local/opt/llvm/bin", "/opt/local/bin", "/usr/bin"):
        cc, ld = Path(d) / "clang", Path(d) / "wasm-ld"
        if cc.exists() and ld.exists():
            r = subprocess.run([str(cc), "--print-targets"], capture_output=True, text=True)
            if "wasm32" in r.stdout:
                return str(cc), str(ld)
    return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--inner", action="store_true", help="run the compile+link (inside the container)")
    ap.add_argument("--docker", action="store_true", help="build in debian:bookworm-slim")
    ap.add_argument("--clang", help="clang with the wasm32 target")
    ap.add_argument("--lld", help="wasm-ld")
    a = ap.parse_args()
    gen_samples()
    if a.inner:                                   # what the container runs: apt has clang + wasm-ld on PATH
        compile_link("clang", "wasm-ld")
    elif a.clang or a.lld:
        if not (a.clang and a.lld):
            raise SystemExit("build_studio: --clang and --lld go together")
        compile_link(a.clang, a.lld)
        print("studio: local", a.clang)
    else:
        tool = local_toolchain()
        if tool and not a.docker:
            compile_link(*tool)
            print(f"studio: local {tool[0]}")
        else:
            if not have("docker"):
                raise SystemExit("build_studio: no docker and no local wasm toolchain "
                                 "(apt install clang lld, or brew install llvm)")
            if subprocess.run(["docker", "image", "inspect", IMG], capture_output=True).returncode:
                sh(["docker", "build", "-t", IMG, "-f", "tools/Dockerfile.studio", "tools"], cwd=SRC)
            sh(["docker", "run", "--rm", "-v", f"{SRC}:/src", "-w", "/src", IMG,
                "python3", "/src/tools/build_studio.py", "--inner"], cwd=SRC)
            print("studio: docker " + IMG)
    for f in ("engine.wasm", GEN_BIN):
        p = OUT / f
        print(f"  {f}: {p.stat().st_size:,} B")


if __name__ == "__main__":
    main()
