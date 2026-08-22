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
- **Cartesian frame** — the structural panel boundary system: 1px hairlines (`#333333`), coordinate crosshairs (`+`) at every intersection, micro-scale calibration ticks (`-`/`+`) and dithered corner anchors. Replaces naive repeated `─`/`│` loops with authentic industrial xerox precision.
- **Dual-mode visualizer** — the real-time analyzer band: upper Braille/dither waveform scope (60fps) and lower scrolling waterfall spectrogram (density ` ░▒▓█`) unified on a single canvas with a shared Cartesian graticule.
- **Smooth ASCII knob** — continuous sub-cell dithered slider in canonical `[....]` bracket format with 36+ gradient states, vertical-drag physics, Shift fine-step (0.1×), and non-destructive border micro-glitch.

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
Strong black-and-white with **shades only where needed** (subtle surface separation, graticule dimming). No saturated colors: the warm-grey accent and the danger red are the only chromatic elements, and both are signal-only. Saturated palette expansion is deferred. The high-contrast xerox aesthetic relies on stark white on pitch black, with dither (`░▒▓█`) as the sole texture — never gradients, never blur shadows.

## Typography

Three self-hosted voices (no CDN dependency — the WebView may be offline):

- **Display:** Akira Expanded — weights 800, 900 — fallbacks: Haettenschweiler, Arial Narrow Bold, sans-serif (Impact removed: Akira is always loaded, never a silent fallback). Reserved for the SYNTHORTION wordmark and section codes only.
- **UI mono:** JetBrains Mono — weights 400, 700 — labels, values, readouts.
- **ASCII/VGA:** a bitmap terminal font (Px437 IBM VGA 8x16 or VT323) — all ASCII surfaces, at integer multiples of the font's pixel grid, `font-smooth: none`, `line-height: 1`, `letter-spacing: 0` on graticule rows to preserve cell alignment.

### Character vocabulary
- Meters/ladders: `▁▂▃▄▅▆▇█` (U+2581–2588) — one char per column, 8 sub-segments per row.
- Density/dither: `░▒▓█` — decorative fills, phosphor decay, and sub-cell knob interpolation only; never body text.
- Waveform: braille `U+2800–28FF` — scope upper tier only, cells ≥14px, rendered via `fillText` row-by-row.
- Frames: `┌─┐│└┘` (U+2500–257F) — chassis outer corners only; interior dividers use Cartesian rules.
- Cartesian: `+ ─ │ ├ ┤` — 1px hairlines and crosshairs at every structural intersection; `+` anchors every panel header and rail corner, `├ ┤` frame the visualizer frequency divider, `─`/`│` are single-cell rules never looped via string repetition.
- Calibration ticks: `-` / `+` / `·` — 7–8px micro markers on module side rules and meter rails, `aria-hidden`, opacity 0.5–0.6.
- Indicators: `● ○ ◉`, peak `▲`, block cursor `▊` (U+258A).
- Controls: `[█░▒▓-]` bracketed track — `█` filled, `░▒▓` sub-cell dither, `-` empty; outer `[` `]` carry micro-glitch flicker.

## Voice & Tone

- **Adjectives:** direct, raw, unpretentious, technical, underground, terminal.
- **Tone:** like a producer in the studio — concise, uppercase-heavy labels, DSP vocabulary (DRIVE, BITCRUSH, DELAY, CHORUS, TRIM, SAMPLE RATE, BUFFER).
- **Avoid:** soft pastels, warm off-whites, premium/curated/boutique copy, corporate polish, playful copy.

## Cartesian Frame System (Anti-AI-Slop)

Replaces naive ASCII loops (`│`/`─` repeated via string replication) and mismatched CSS borders with a disciplined coordinate-frame discipline inspired by *Corrosion Diva Bank* / *Multikit* xerox instruments.

**Rules:**
- **1px hairline only.** Every structural divider — module `border-r`, header `border-b`, visualizer `border-b`, faceplate outer `border`, rail `border-l/r`, chassis `border` — is a single 1px solid line at `#333333` (`--border` / `--elev-7`). No double borders at junctions; shared edges collapse to one rule.
- **Crosshair intersections.** Every header, divider, and frame corner that meets a perpendicular rule carries a `+` (8px `font-ascii`, `text-ink-3`) centered on the intersection. Module headers: `+ [ CODE ] +` with a central `+` on the horizontal rule. Header bar: `+` at bottom-left and bottom-right where it meets the vertical rails. Visualizer divider: `├ ── + 20Hz ── + 2kHz ── ┤` with `+` ticks at log-frequency columns. Faceplate bottom bar: `+ ░▒ ── CAL.0CODE ── :: ── SYS.X ── ▒░ +`.
- **Micro-calibration ticks.** Three 7px marks per vertical rule (`-` `+` `-`) at `justify-between` on the inner 2px gutter of each module frame and each meter rail. Rendered `aria-hidden`, `opacity-60`, `font-ascii`, pointer-events none — they telegraph industrial calibration without competing for focus.
- **Dithered framing.** Bottom calibration bars and decorative streams use halftone `░▒▓` at 50% opacity — never as a border, only as a low-contrast anchor that sells the xerox photocopy texture.
- **No repeated character spans.** A divider is a CSS `border` or an `h-px`/`w-px` `bg-grid-rule` element, not a repeated `─` string. The sole exception is the visualizer frequency divider, which builds its `─` row cell-by-cell to embed `+` ticks and `20Hz` labels without collision. Tests enforce `not.toMatch(/─{2,}/)` in header text content.
- **Accessibility.** All `+`, `─`, `│`, `░▒▓`, `┌┐` chrome is `aria-hidden="true"`; interactive semantics live on `role="slider"` / `role="meter"` / `aria-pressed` exclusively.

## Dual-Mode Visualizer Architecture

A single 240px canvas band sits between the status header and the module grid — `h-[240px] shrink-0` with `min-h-0` on the center hub so the band never shifts the grid.

**Tiers (single canvas, 60fps):**
1. **Upper scope (60% height):** Cartesian graticule (`+` crosshairs + 1px rules) in `#333333`, phosphor trace in `#f6f6f6` (idle `#666666`), sub-pixel dither in `#888888`. Braille cells (`U+2800–28FF`) map the time-domain buffer at 16px cells with two-generation phosphor persistence. Glitch corruption is proportional to interaction intensity and decays ~300–500ms.
2. **Divider row (1 cell):** `├ ┤` framed rule with log-frequency `+` ticks at 20Hz / 200Hz / 2kHz / 20kHz and inline labels, cached per column count.
3. **Lower waterfall (remaining rows minus 1):** Scrolling history fed by multi-band energy analysis, rendered with density glyphs ` ░▒▓█`, per-row age alpha, color `#c7c3ba`. Bypass clears history to idle line; animation stops.

**Graticule & calibration:** Frequency scale 20Hz→20kHz (log), amplitude dB, `+` crosshairs, phosphor scanline persistence via canvas alpha, static CRT scanline dual-gradient overlay (one `::before` pseudo-element on `.vst-container`, never animated).

**Reduced motion:** `prefers-reduced-motion: reduce` renders a single static clean frame with one step and no RAF loop; all glitch/flicker/stream animations disabled.

**Testing seam:** Signal, glitch pulser and PRNG are injectable; tests never assert canvas pixels, only DOM: `data-active`, `data-mode="dual"`, `data-static`, canvas presence, bypass decay, glitch propagation, and resize via `ResizeObserver`.


## Control Surface Physics

- **Continuous drag:** Normalized float `0..1` without discrete stepping; `DRAG_SENSITIVITY=0.5%` per px, `FINE_STEP_FACTOR=0.1` while `Shift` held. `dy = startY - clientY` → `value = startVal + dy * sensitivity * (range/100)`, clamped `[min,max]`.
- **Sub-cell dither:** 9-cell bracketed track `[.........]` → `totalSteps=36` (4 dither levels per cell via `░▒▓█`), `currentStep=round(pct*36)`, per-cell `cellStep` selects `█` / `░▒▓` / `-`.
- **Micro-glitch:** Outer `[` `]` carry `.knob-glitch` (`knob-flicker 0.1s steps(1) infinite`, `opacity 0.7` at 50%); numeric readout and dither cells stay 100% legible, never corrupted.
- **Keyboard & ARIA:** `role="slider"` `aria-valuemin/max/now/text` `aria-orientation="vertical"` `tabIndex=0` (disabled `-1`), `ArrowUp/Right` (+step), `Shift+Arrow` (+largeStep), `Home`/`End`, double-click resets to `defaultValue`.

## Layout

- **Chassis:** fluid `w-full h-full` with `1px solid var(--border)` on `.vst-container`, box-drawing corner brackets (`┌ ┐ └ ┘` at 16px `font-ascii` `text-ink-3`, `aria-hidden`), static CRT scanlines (`.vst-container::before` dual `repeating-linear-gradient` + `linear-gradient`, `z-index 40`, `pointer-events none`), and `noise-overlay` turbulence (`opacity 0.025`, `mix-blend-mode overlay`, `z-index 50`).
- **Structure:** 3 columns — left IN rail (48px) · center hub · right OUT rail (48px). Center hub = status bar (54px `h-[54px]` `shrink-0` `border-b border-grid-rule`) + ASCII scope (240px `h-[240px]` `shrink-0` `border-b border-grid-rule`) + module grid (flexible, `flex-1 flex items-center justify-center px-6 py-6`, inner faceplate `grid grid-cols-5 border border-grid-rule`).
- **Faceplate:** 5-column grid — DRV | BCR | DLY (×2) | CHR. Each section is a framed module: header `h-[24px]` `border-b` with `+ [ CODE ] + [ PWR: ON ] +` and LED power switch, control area with `CalibrationTicks`, bottom bar `h-[16px]` `border-t` with `+ ░▒ CAL.0CODE :: SYS.X ▒░ +`.
- **Meter rails:** `w-[48px] shrink-0 bg-elev-0 flex flex-col items-center py-6 border-r/l border-border` with `CalibrationTicks` mirroring modules (plus `+` at header 54px and visualizer 294px shared hairlines), ladder `w-[8px] h-[256px]` `bg-elev-0` `boxShadow: var(--shadow-well), 0 0 0 1px var(--elev-6)`, top `+ ┌ IN ┐ +` / `0` / `│` gutters, bottom `+ ░▒ └─(1px hairline)─┘ ▒░ +` (CSS `h-px bg-ink-3`, never repeated `─`), bracketed dB readout `[ -INF ]`.
- **Radius:** max 2px (`rounded-[1px]` on LEDs only).
- **Border weight:** 1px (`#333333`) — the single hairline token.
- **Spacing:** 2/4/8/16/24/32/48/64px.

### Posture rules
- Strict dark-mode-first: #000, never light canvases.
- Sharp radii only; no rounded or playful elements.
- Visible structural grid, 1px solid borders (#333) — one rule, never doubled at junctions; crosshairs mark every intersection.
- No soft-blur drop shadows — sharp 1–2px offset borders or contrast layering; chassis depth via elevation ramp (`--elev-0 … --elev-7`) and hairlines only.
- High data density: technical details always visible (sample rate, buffer, values).
- Buttons as terminal brackets: `[ SAVE ] [ LOAD ]` — 1px border, hover inversion (`hover:bg-fg hover:text-bg hover:border-fg`).
- Inputs: dark fields with 1px #333 border; focus inverts to solid #FFF bg + #000 text.
- Power-off modules: controls dimmed (`opacity-30 pointer-events-none`), `--` readout, LED off (`bg-elev-5` with inset shadow).
- Reduced-motion: all glitch, flicker, beam animation and boot `[ OK ]` typewriter disabled; boot shows final state immediately.

## Accessibility

- All decorative glyph surfaces (`+`, `─`, `│`, `░▒▓`, `┌┐`, calibration ticks, dither) are `aria-hidden="true"`; every meter carries `role="meter"` with `aria-valuemin/max/now` and `aria-valuetext` in dB; every knob carries `role="slider"` with full `aria-*` set.
- Never convey state by color alone (clip markers add a glyph change, e.g. `▲` on peak rows; power LED adds `aria-pressed`).
- Boot typewriter animates via CSS `clip-path`/`width`, never progressive `textContent`, so assistive tech reads the full text.
- Static scanlines and faint CRT texture only; anything animated is gated behind `prefers-reduced-motion` (`reduce` disables `vst-enter`, `knob-flicker`, `stream-scroll`, `cursor-blink`, and the visualizer RAF loop).
- Decorative ASCII never appears in `textContent` loops that would bloat the accessibility tree — frame chrome lives in `aria-hidden` spans or canvas.

