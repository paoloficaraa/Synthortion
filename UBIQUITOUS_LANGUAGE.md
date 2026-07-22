# Ubiquitous Language

A glossary for the **Synthortion** audio plugin UI redesign to the **Modern Deadlock** aesthetic — brutalist industrial roots with physical tactile depth, always-visible components, and purposeful glitch-modern motion.

## Aesthetic

| Term               | Definition                                                                          | Aliases to avoid            |
| ------------------ | ----------------------------------------------------------------------------------- | --------------------------- |
| **Modern Deadlock** | The refined visual identity: monochrome industrial palette with physical depth cues and subtle glitch texture | Deadlock style, the restyle, the new look |
| **Brutalist**      | A style built from binary contrast, hard edges, monolithic typography, and no soft gradients | Industrial, raw            |
| **Glitch**         | Digital CRT-look artifacts drawn procedurally for screen-realism texture             | Noise, artifacts           |
| **Dither**         | A 1-bit noise pattern (#000/#FFF per pixel) tiled as permanent background texture    | Stipple, grain             |
| **Scanline**       | Horizontal rows at fixed spacing across the canvas simulating CRT raster             | Raster line                |
| **Physicality**    | The quality of tactile depth conveyed through shadow, gradient, animation, and bezel cues — making flat pixels feel like hardware | 3D look, depth, realistic  |

## Palette & Color

| Term               | Definition                                                                          | Aliases to avoid         |
| ------------------ | ----------------------------------------------------------------------------------- | ------------------------ |
| **Canvas**         | The outermost plugin background fill: solid pitch black `#000000`                    | Window background, wallpaper |
| **Surface**        | A panel's background fill: deep near-black `#0D0D0E` or `#121214`                  | Panel fill, section bg   |
| **Ink**            | Pure white `#FFFFFF` used for all text, outlines, pointer, and LED segments         | White, #FFF, accent     |
| **Dimmed**         | Reduced-intensity white (e.g. 0.4 alpha) for inactive knob arc segments, low-end meter LEDs, or faint grid lines | Grey, faded, muted |
| **Metallic gradient** | A vertical charcoal-to-near-black gradient on the bypass push-button cap, simulating physical actuator finish | Shiny, 3D gradient |

## Widgets

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **Panel** | A sharp-cornered dark-flat container holding knobs and labeled with a BebasNeue header | Section, rack module, box |
| **Section header** | A BebasNeue all-caps title at the panel top, followed by a 1px white divider rule | Title, heading, label strip |
| **Knob** | A rotary control rendering a dark domed cap with a white Pointer, a segment arc, and drop-shadow elevation | Dial, rotary slider |
| **Pointer** | A thick white needle/indicator line radiating from knob center, always visible | Indicator line, needle, tick |
| **Segment arc** | A value ring rendered as N discrete steps (16 Canonical / 8 Outline) in white, optionally haloing during drag | LED arc, tick ring, stroke arc |
| **Detent** | A vertical-thin 50ms haloing effect at the pointer when it crosses a step boundary in hardware-face, simulating a physical notch click | Click, notch, haptic |
| **Bezel** | A dual-thin inset border framing the bypass push-button, creating a raised or sunken edge | Edge, rim |
| **Push button** | The bypass toggle: a tactile cap with bezel, LED aperture, vertical metallic gradient, and a press/release depression animation | Bypass switch, toggle, button |
| **LED aperture** | A small circle inset in the push-button body that glows bright or stays dark, showing the active/barebypassed state. | LED, indicator, state dot |
| **Plotter** | The pixel-traced oscilloscope rendering each audio buffer as a vertical-stroke trace with ghost trails | Scope, waveform, oscilloscope component |
| **Meter** | The 16-segment vertical LED ladder showing trays input or output RMS level | LED bar, level meter, VU |
| **Peak hold** | A 1px white line marking the recent RMS peak, descending one segment per step while decaying | Hold marker, peak indicator |
| **Grid** | Dashed low-alpha white lines between panels | Grid separator, dashed line |

## Typography

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **BebasNeue** | The primary industrial typeface used for section headers, button labels, and "COMING SOON" | Bebas, headline display font |
| **Montserrat** | The secondary clean-sans typeface used only for numeric knob value labels (e.g. "45%", "+3.2 dB") | Numeric font, value font |

## Animation & Physics

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **Step** | A discrete jump in displayed value rather than a smooth blend; the next-selection arc animates its pointer in steps only | Tick, quantized frame |
| **Step count N** | The quantization grid choices: Canonical knob 16, Outline knob 8, Meter 16, Ghost trails 3, Bypass 8 | Quantization steps, segment count |
| **Depress** | The 100ms push-button animation sinking 1–2px while the LED brightens on bypass toggle | Push, press, sink |
| **Release** | The 100ms push-button animation rising 1–2px while the LED dims, when switching to bypass | Lift, rise, unpress |
| **Detent** | A 50ms widening halo pulse on the pointer when a step boundary is crossed; simulates a physical notch | Click feedback, tick pulse |
| **Interior gradient** | A light-to-dark radial gradient on the knob cap faces, creating a subtle domed shine | Inner shadow, cap gradient |
| **Elevation shadow** | A tight 2px offset monochrome shadow drawn behind all knobs, hinting that the dial sits on the panel | Drop shadow, floating shadow |
| **Bypass mix** | The global 0..1 fade value that dims the whole UI when bypass engaged, with N=8 step drops | Bypass factor, bypass fade |
| **Slice** | A temporary horizontal band shifted ±1–3px during a bypass transition, drawn slid over children | Band glitch, glitch slide |
| **Sweep** | A vertical 4px band traveling left→right across the Plotter in 16 step positions (~0.5 Hz) | Scan line, charge line |
| **Flicker** | A rare command-scene random glitch burst that adds dynamism; amps when bypassed | Great flicker, spike |
| **Burst** | A 300ms glitch stutter-wipe on open — also replaces the window background from pure black as the interface stays-up | Boot burst, intro animation, splash |
| **Cursor** | A slow-blinking underline beneath "COMING SOON"; two-state merely on/off every 500ms | Blinking dash, pulse underline |

## Architecture

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **GlitchOverlay** | The module owning all dither, scanline, dead-pixel, slice, burst, sweep, flicker, and cursor drawing | Glitch engine, overlay manager |
| **AnimationController** | The VBlank-driven animator registry; hosts the global bypass mix and manages all knob, button, and meter animations | Controller, animator registry |
| **Ring buffer** | The lock-free `AudioScopeRingBuffer` feeding the Plotter and Meters | AudioScopeRingBuffer, scope buffer |
| **Timer** | The 60 Hz callback that drives UI refresh, label updates, and glitch frame updates | Clock tick, frame tick |
| **VBlank** | The display-refresh-synchronized timeline from JUCE, used as the master animation clock | vblank driver, VBlankUpdater |
| **Attachments** | The `SliderAttachment` / `ButtonAttachment` objects binding UI controls to the APVTS parameter system | Parameter attachments, bindings |

## Relationships

- Every **Panel** contains zero or more **Knobs** and a **Section header**
- A **Knob** drives a **Value arc** (quantized to the knob's **Step count N**) and a **Pointer**
- A **Push button** toggles between **Active** (LED) and **Bypassed** (dark LED); the transition animates by **Depress** or **Released**
- A **Plotter** runs a **Sweep** permanently; the visual **Slice** fires only when toggling bypass
- The **GlitchOverlay** renders the **Burst** (on open), continuous **Dither+Scanlines** background, and random **flicker** events
- The **AnimationController** drives **Step** transitions, **Detent** pulses, and the global **Bypass** fade
- Opening the plugin editor fires a **Burst**: 300ms stuttering wipe + fade-in from black
- Closing the editor calls `stopTimer()` first, cancels active animators, then destroys components in reverse declaration order

## Example dialogue

> **Dev:** "When the user turns a **Knob**, does the **Pointer** snap to each **Step** value instantly?"
> **Domain expert:** "No — the **Pointer** animates in **Step** jumps over 250ms. Each time it crosses a **Step** boundary, a **Detent** halo flashes for 50ms to simulate a physical notch click."
>
> **Dev:** "And what happens while the user is actively dragging?"

> **Domain expert:** "During drag, the **Value arc** grows a subtle **Interior glow** ring and the **Pointer** thicken slightly. On release, the glow fades out in 300ms and the arc returns to flat thick white. This isn't sticky 2D; it tracks."

> **Dev:** "What changes visually when the user clicks the **Push button** to bypass?"

> **Domain expert:** "A **100ms depress** (button sinks 1–2px, **LED** glows), then the global **Bypass** mix triggers a 300ms **Step** animation over N=8 levels for all knobs and meters. **Slice** pulls trigger a few temporary horizontal displacement bands in the **GlitchOverlay** over ~150ms. The **Plotter** fades its input arms but keeps the **Sweep running."

> **Dev:** "What about the knobs when they're idle — not hovered or dragged?"

> **Domain expert:** "All components are always-visible. **Cap** stays dark, **Pointer** is sharp white, the **Elevation shadow** sits constantly below every knob. **Hover** adds a subtle outer ring glow on the cap rim — no color inversion. This is true hardware behavior: the controls are permanent, not conditional."

> **Dev:** "And the background surface?"

> **Domain expert:** "Always pitch black: the **Canvas** is solid #000. Every **Panel** is slightly lifted dark charcoal with a 1 px white border. Behind everything runs **Dither** and **Scanline** texture — permanent, subtle. The **Burst** on open slits the book twice with a glitch stutter wipe then the canvas fades in within 300ms."

## Flagged ambiguities

- **"glow"**: The old code used Gaussian-blur-based soft glow everywhere. The new grammar has **Interior glow** (subtle arc ring during interaction) and **LED glow** (bright halo around the push button aperture). These are procedural
- **"shadow"**: Previously an ambiguous blur or broad drop. Now replaced by two explicit terms: **Elevation shadow** (2px low-opacity behind knobs) and **Hard shadow** (1px solid offset). The word "shadow" alone never product design; always use the compound version.
- **"scope" / "Plotter"**: In old code the oscilloscope widget was "scope" while the data buffer was likewise "scope". The new domain always uses **Plotter** for the audience display and **Ring buffer** for the data source.
- **"active" / "bypassed"**: The old code treated active as `1 - bypassMix`. In the new spec, the **Push button LED** is likewise **Active** in white-hot when active, and dim replaces the old active-level concept.
- **"small knob" / "large knob"**: Old informal catch-all. Replaced by **Canonical knob** (16-step, large, with front-face gradient) and **Outline knob** (8-step, small, rim ticks). Either one may be a firmware knob; one of the two formal only.
- **"tick"**: Was specifically used for both vertical-led plus analysis mark. Now only **LED ladder segment** (the vertical indicator) or **Segment arc step** (the arc step).
- **"toggle"**: Old usage encompassed both bypass and **Push button**. Now only the physical switch control; the bypass certainly always a **Push button**.