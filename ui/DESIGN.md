---
name: "Synthortion — Digital Instrument & ASCII Terminal Design System"
category: Brand-derived UI System
surface: web (VST host WebView)
colors:
  pitch-black: "#000000"
  scope-face: "#0f0e0e"
  elevated-surface: "#121212"
  stark-white: "#f6f6f6"
  technical-grey: "#888888"
  grid-rule: "#333333"
  accent-warm-grey: "#c7c3ba"
  danger-red: "#ff4444"
---

# Synthortion — Digital Instrument & ASCII Terminal Design System

> Category: Brand-derived UI System
> Surface: web (VST host WebView)

*High-contrast monochrome, digital instrument precision, ASCII-terminal information surfaces. Evolved from the Deadlock identity (dark industrial & glitch brutalism) toward a flat, digital, terminal-voiced instrument — luma-inspired clarity, Deadlock-grade density and contrast.*

The UI is a **digital instrument**: flat, precise, data-dense, with information surfaces rendered in character glyphs. It is explicitly **not** skeuomorphic hardware — no metal gradients, no rack screws, no bezels, no recessed-well chrome. The analog warmth of the old look is replaced by terminal rigor.

## Design language (canonical terms)

- **Digital instrument** — the aesthetic direction: flat, precise, digital. Hardware chrome is banned.
- **ASCII surface** — any information display rendered in character glyphs: block-element meters (`▁▂▃▄▅▆▇█`), braille waveform (`U+2800–28FF`), VGA-style readouts, box-drawing frames (`┌─┐│└┘`). The voice of the terminal inside the instrument.
- **Module** — a processing unit on the faceplate with its own power state: Drive (DRV), Bitcrush (BCR), Delay (DLY), Chorus (CHR). Canonical name is **Drive**, not "Warm Distortion".
- **Power** — per-module on/off state, independent of the master `engineActive` bypass. A powered-off module dims its controls and shows `--` readouts.
- **Tweak-driven glitch** — the signature behavior: the scope's ASCII field corrupts proportionally to live interaction intensity (drag velocity × amplitude), decaying in ~300–500ms. Idle = clean trace. Power toggles fire a short burst; bypass fires one heavy corruption frame. No dedicated glitch control exists.
- **Boot sequence** — the mount-time terminal overlay: one-shot, staged lines with real values and aligned `[ OK ]` columns, ending on `[READY]`. Skippable by click/Enter. Reduced-motion renders the final state immediately.

## Color Palette

| Role | Name | Hex | Usage |
| --- | --- | --- | --- |
| background | Pitch Black | `#000000` | Page canvas, boot overlay, terminal fields |
| scope face | Scope Face | `#0f0e0e` | Oscilloscope graticule field |
| surface | Elevated Surface | `#121212` | Panels, status bar, module frames |
| foreground | Stark White | `#f6f6f6` | Trace, readouts, primary text, active LED |
| muted | Technical Grey | `#888888` | Labels, idle traces, grid lines |
| border | Grid Rule | `#333333` | 1px structural dividers |
| accent | Warm Grey | `#c7c3ba` | Active arcs, focus rings, power-on markers |
| danger | Danger Red | `#ff4444` | Clip/error states only |

### Monochrome discipline
Strong black-and-white with **shades only where needed** (subtle surface separation, graticule dimming). No saturated colors: the warm-grey accent and the danger red are the only chromatic elements, and both are signal-only. Saturated palette expansion is deferred.

## Typography

Three self-hosted voices (no CDN dependency — the WebView may be offline):

- **Display:** Akira Expanded — weights 800, 900 — fallbacks: Impact, Haettenschweiler, sans-serif. Reserved for the SYNTHORTION wordmark and section codes only.
- **UI mono:** JetBrains Mono — weights 400, 700 — labels, values, readouts.
- **ASCII/VGA:** a bitmap terminal font (Px437 IBM VGA 8x16 or VT323) — all ASCII surfaces, at integer multiples of the font's pixel grid, `font-smooth: none`, `line-height: 1`.

### Character vocabulary
- Meters/ladders: `▁▂▃▄▅▆▇█` (U+2581–2588) — one char per column.
- Density: `░▒▓█` — decorative fills only, never text.
- Waveform: braille `U+2800–28FF` — scope only, cells ≥14px.
- Frames: `┌─┐│└┘` (U+2500–257F) — module frames, chassis corners.
- Indicators: `● ○ ◉`, peak `▲`, block cursor `▊` (U+258A).
- Controls: `[====+----]` — filled `=`, pointer `+`, empty `-`.

## Voice & Tone

- **Adjectives:** direct, raw, unpretentious, technical, underground, terminal.
- **Tone:** like a producer in the studio — concise, uppercase-heavy labels, DSP vocabulary (DRIVE, BITCRUSH, DELAY, CHORUS, TRIM, SAMPLE RATE, BUFFER).
- **Avoid:** soft pastels, warm off-whites, premium/curated/boutique copy, corporate polish, playful copy.

## Layout

- **Chassis:** fixed 880px, 1px border, box-drawing corner brackets (`┌ ┐ └ ┘`), subtle static CRT scanlines (one pseudo-element, never animated).
- **Structure:** 3 columns — left IN rail (40px) · center hub · right OUT rail (40px). Center hub = status bar (64px) + ASCII scope (240px) + module grid.
- **Faceplate:** 5-column grid — DRV | BCR | DLY (×2) | CHR. Each section is a framed module: title bar with code + power switch, control area, inactive modules dim with `--` readouts.
- **Radius:** max 2px.
- **Border weight:** 1px.
- **Spacing:** 2/4/8/16/24/32/48/64px.

### Posture rules
- Strict dark-mode-first: #000, never light canvases.
- Sharp radii only; no rounded or playful elements.
- Visible structural grid, 1px solid borders (#333).
- No soft-blur drop shadows — sharp 1–2px offset borders or contrast layering.
- High data density: technical details always visible (sample rate, buffer, values).
- Buttons as terminal brackets: `[ SAVE ] [ LOAD ]` — 1px border, hover inversion.
- Inputs: dark fields with 1px #333 border; focus inverts to solid #FFF bg + #000 text.
- Power-off modules: controls dimmed, `--` readout, LED off.
- Reduced-motion: all glitch, flicker, and beam animation disabled; boot shows final state immediately.

## Accessibility

- All decorative glyph surfaces are `aria-hidden`; every meter carries `role="meter"` with `aria-valuemin/max/now` and `aria-valuetext` in dB.
- Never convey state by color alone (clip markers add a glyph change, e.g. `▲`).
- Boot typewriter animates via CSS `width`/`clip`, never progressive `textContent`, so assistive tech reads the full text.
- Static scanlines and faint CRT texture only; anything animated is gated behind `prefers-reduced-motion`.
