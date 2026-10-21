#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Draw the original SPIRALFM1 mark: two logarithmic spiral arms, a signal ring and nodes.
No external pictures, font outlines or upstream logo paths are used as input.
Writes assets/spiralfm1/spiralfm1-symbol.svg and assets/spiralfm1/spiralfm1-logo.svg.
"""
import math
from pathlib import Path

OUT = Path(__file__).resolve().parents[1] / "assets" / "spiralfm1"
CYAN, VIOLET, LIME, CREAM = "#6ee7ff", "#c59bff", "#c8ff69", "#f3f1e8"


def arm(cx, cy, r0, r1, turns, phase, k=1.0):
    """r0..r1 over `turns` turns, `phase` radians offset; k bends the pitch of the spiral."""
    t1 = turns * 2 * math.pi
    steps = max(48, int(turns * 64))
    pts = []
    for i in range(steps + 1):
        t = t1 * i / steps
        r = r0 * math.exp(math.log(r1 / r0) * (t / t1) ** k)
        pts.append((cx + r * math.cos(t + phase), cy + r * math.sin(t + phase)))
    d = " ".join(("M" if i == 0 else "L") + f"{x:.2f} {y:.2f}" for i, (x, y) in enumerate(pts))
    return d, pts[-1]


def mark(x=0, y=0):
    """The 160x160 symbol, optionally offset."""
    g = [f'<g transform="translate({x} {y})" fill="none">']
    g.append(f' <circle cx="80" cy="80" r="68" stroke="{VIOLET}" stroke-width="2" opacity=".45"/>')
    g.append(f' <path d="M12 80A68 68 0 0 1 80 12" stroke="{CYAN}" stroke-width="6" stroke-linecap="round"/>')
    g.append(f' <path d="M148 80A68 68 0 0 1 80 148" stroke="{CYAN}" stroke-width="6" stroke-linecap="round"/>')
    d1, e1 = arm(80, 80, 9, 58, 2.25, -math.pi / 2, 1.15)
    d2, e2 = arm(80, 80, 9, 58, 2.25, math.pi / 2, 1.15)
    g.append(f' <path d="{d1}" stroke="{LIME}" stroke-width="6" stroke-linecap="round"/>')
    g.append(f' <path d="{d2}" stroke="{VIOLET}" stroke-width="5" stroke-linecap="round" opacity=".9"/>')
    g.append(f' <circle cx="80" cy="80" r="7" fill="{CREAM}"/>')
    for ex, ey in (e1, e2):
        g.append(f' <circle cx="{ex:.2f}" cy="{ey:.2f}" r="6" fill="{CYAN}"/>')
    g.append(" </g>")
    return "\n ".join(g)


SYMBOL = ('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 160 160" fill="none">\n'
          ' <title>SPIRALFM1 - spiral mark</title>\n' + mark() + "\n</svg>\n")

LOGO = ('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 730 160" fill="none">\n'
        ' <title>SPIRALFM1 - wordmark</title>\n' + mark() + "\n"
        '<text x="184" y="93" fill="#' + CREAM[1:] + '" font-family="Arial,Helvetica,sans-serif"'
        ' font-size="66" font-weight="800" letter-spacing="6">SPIRALFM1</text>'
        f'<text x="190" y="122" fill="{CYAN}" font-family="Arial,Helvetica,sans-serif"'
        ' font-size="14" letter-spacing="5">SPIRAL SOUND ENGINE &#183; SPIRALFRACTAL</text></svg>\n')

if __name__ == "__main__":
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "spiralfm1-symbol.svg").write_text(SYMBOL, encoding="utf-8")
    (OUT / "spiralfm1-logo.svg").write_text(LOGO, encoding="utf-8")
    print("wrote spiralfm1-symbol.svg + spiralfm1-logo.svg")
