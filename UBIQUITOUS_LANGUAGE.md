# Ubiquitous Language

A glossary for the **Synthortion** audio plugin UI restyle to the **DEADLOCK** aesthetic. This vocabulary is shared between design intent and code; using terms precisely keeps the brutalist/glitch grammar coherent across palette, widgets, animation, and glitch overlays.

## Aesthetic

| Term            | Definition                                                                 | Aliases to avoid            |
| --------------- | -------------------------------------------------------------------------- | --------------------------- |
| **DEADLOCK**    | The brutalist industrial/glitch visual identity of Synthortion             | Deadlock look, the new look |
| **Brutalist**   | A style built from binary contrast, hard edges, monolithic typography      | Industrial, raw              |
| **Glitch**      | Digital/CRT-look artifacts drawn procedurally for digital-screen feel       | Noise, artifacts            |
| **Dither**      | A 1-bit noise pattern (pure #000/#FFF per pixel) simulating bitmap printing | Stipple, halftone (subset)  |
| **Scanline**    | A horizontal row of fixed-pixel spacing simulating CRT monitor scan        | Raster line                 |
| **Hard shadow** | A 1px-offset solid duplicate of a shape, drawn without Gaussian blur       | Drop shadow, glow           |

## Palette

| Term           | Definition                                                                  | Aliases to avoid              |
| -------------- | --------------------------------------------------------------------------- | ----------------------------- |
| **Active**     | The bypass state where signal flows; rendered as #FFF blocks                | On, engaged, active state     |
| **Bypassed**   | The state where signal is suppressed; rendered as #000 with #FFF outline   | Off, disabled, inactive       |
| **Inversion**  | A color flip between #FFF and #000 used as hover/drag/state affordance       | Color flip, negation          |
| **Triplet**    | The three parallel #FFF strokes separated by 1px #000 gaps evoking RGB split | RGB ghost, channel split      |
| **CRT-glass**  | The visual layer of glitch overlays above all child components               | Dust overlay, glass layer     |

## Widgets

| Term                | Definition                                                                          | Aliases to avoid                  |
| ------------------- | ----------------------------------------------------------------------------------- | --------------------------------- |
| **Panel**           | A hard-cornered #000 module holding knobs, titled with BebasNeue                    | Section, rack module             |
| **Canonical knob**  | The large #FFF solid-disc knob with segmented/checker value arc                      | Large knob, primary knob          |
| **Outline knob**    | The small #000 disc with 1px #FFF outline + segmented ticks                         | Small knob, secondary knob        |
| **Segment arc**     | A value arc rendered as N discrete blocks rather than a smooth path                 | LED arc, tick ring                |
| **Block toggle**    | The bypass switch rendered as a two-state invertible block                         | Switch, lever                     |
| **LED ladder**      | The meter rendered as N vertical discrete segments                                     | Bar, fill meter                   |
| **Plotter**         | The oscilloscope rendering an audio buffer as a vertical-stroke trace              | Scope, waveform, scope component  |

## Typography

| Term                 | Definition                                                                  | Aliases to avoid                    |
| -------------------- | --------------------------------------------------------------------------- | ----------------------------------- |
| **BebasNeue**        | The primary typeface across all visible text                                | Bebas, headline font                |
| **Service text**     | Reserved role for Montserrat — currently no surface uses it                 | Secondary text, fallback text       |
| **Kerning factor ā0.5** | The tight tracking applied to all BebasNeue labels for monolithic density  | Letter spacing, tracking            |

## Animation

| Term            | Definition                                                                          | Aliases to avoid                  |
| --------------- | ----------------------------------------------------------------------------------- | --------------------------------- |
| **Step**        | A discrete quantized jump in an animated value                                       | Tick, segment, beat                |
| **Step count N** | The per-widget quantization count: large knob 16, small knob 8, meter 16, scope-ghost 3, idle flicker 2, bypass 8 | Quantization steps                |
| **Sweep**       | The scan-charge band moving leftāright across the Plotter in 16 hard steps          | Scan line, charge line             |
| **Burst**       | The one-time glitch sequence fired on plugin window open                             | Boot animation, intro burst        |

## Glitch accents

| Term              | Definition                                                                       | Aliases to avoid                |
| ----------------- | -------------------------------------------------------------------------------- | ------------------------------- |
| **Dead pixel**    | A 1px #FFF dot flickered at random positions across the CRT-glass layer           | Static, dust                    |
| **Slice**         | A horizontal band of the bg shifted ±1ā3px during a Bypass transition             | Band shift, jump, glitch slide   |
| **Tracking bar**  | NOT used ā rejected in favor of the Sweep to avoid double vertical-band effects | Drift bar                        |
| **Twin shadow**   | The hard-shadow affordance drawn around a knob on hover/drag                       | Knob ghost, hover halo          |
| **Hold pulse**    | NOT used ā rejected to avoid visual fatigue during sustained knob tweaking        | Panel strobe, focus pulse        |

## Architecture

| Term                     | Definition                                                                     | Aliases to avoid                    |
| ------------------------ | ----------------------------------------------------------------------------- | ----------------------------------- |
| **GlitchOverlay**        | The module owning all glitch-accent drawing methods                           | Glitch engine, overlay manager       |
| **AnimationController**  | The VBlank-driven animator registry; hosts the global bypass mix              | Animator, controller                 |
| **Bypass mix**           | The shared [0, 1] value coordinating all components during a bypass fade      | Bypass factor, crossfade             |
| **Ring buffer**          | The lock-free AudioScopeRingBuffer feeding Plotter and Meter                  | Scope buffer, audio buffer           |

## Relationships

- A **Panel** contains zero or more **Canonical knobs** or **Outline knobs**
- A **Canonical knob** or **Outline knob** displays its value via a **Segment arc** quantized to that knob's **Step count N**
- A **Block toggle** switches between **Active** and **Bypassed** states; the transition drives the global **Bypass mix** and fires a **Slice**
- A **Plotter** renders a **Trace** per audio channel using **Triplet** ghosting and the **Sweep** overlay
- An **LED ladder** quantizes RMS level to its **Step count N** of segments
- The **GlitchOverlay** draws **Dead pixels**, **Slices**, and the **Burst** onto the **CRT-glass** layer of the **PluginEditor**
- The **AnimationController** drives all **Step** transitions via the **VBlank** timeline

## Example dialogue

> **Dev:** "When I drag the **Canonical knob**, does the **Segment arc** animate smoothly to the new value?"
> **Domain expert:** "No ā it quantizes to **Step count N = 16** for the large knob. Each **Step** is a hard jump, not a blend. The same quantization applies to the **LED ladder** on the **Meter**."
>
> **Dev:** "And the **Twin shadow** on hover uses melatonin?"
> **Domain expert:** "No melatonin anywhere. The **Twin shadow** is a 1px-offset hard duplicate of the knob disc, part of the **GlitchOverlay** grammar. Same module that fires the **Burst** on window open and the **Slice** during a **Bypass** transition."
>
> **Dev:** "So when the **Block toggle** flips, what visually fires?"
> **Domain expert:** "The **Bypass mix** animates over **Step count N = 8** hard steps. During that window the **GlitchOverlay** triggers 2ā3 **Slices** for ~150ms. The **Plotter** keeps running its **Sweep** independently."
>
> **Dev:** "If I see a vertical band scan across the **Plotter**, is that the **Tracking bar**?"
> **Domain expert:** "No ā **Tracking bar** was rejected. That's the **Sweep**, which is the only vertical-band effect on the **Plotter**. The horizontal **Dead pixels** live on the **CRT-glass** layer over everything."

## Flagged ambiguities

- **"glow"** was used in the old code (knob glow, LED glow) for Gaussian blur effects. The DEADLOCK grammar has **no glow**; any "shine" comes from **Inversion** + **Twin shadow** + **Hard edges**. The word `glow` (and the `melatonin_blur` library that produced it) is banned from the restyled codebase to prevent confusion.
- **"shadow"** was ambiguous between Gaussian drop shadows (deleted) and the **Hard shadow** / **Twin shadow** motif (kept). Code referring to "shadow" should be migrated to the explicit term **Twin shadow** or **Hard shadow**; the unqualified word is treated as ambiguous.
- **"scope"** was used colloquially for both `OscilloscopeComponent` (the **Plotter**) and `AudioScopeRingBuffer` (the **Ring buffer**). Use **Plotter** for the visual component and **Ring buffer** for the data source.
- **"copper"** appeared in `copperAccentColourId` / `copperBrightColourId` from the old palette. After the Q20 enum rewrite, copper-related names are deleted; the canonical accent is simply #FFF and the colour ID is `accentColourId`.
- **"active"** in the old code meant "not bypassed" at the component level (`activeLevel = 1 - bypassMix`). The DEADLOCK grammar splits this into **Active** (the bypassed=false state on the **Block toggle**, rendered #FFF) and the per-frame **activeLevel** value is renamed to `signalVisible` or removed entirely.
- **"noise"** was used ambiguously for both smooth grayscale grain (old) and binary **Dither** (new). The two are incompatible; old "noise" code is deleted and replaced with explicit **Dither** + **Scanline** calls in the **GlitchOverlay**.
- **"small knob"** vs **"large knob"** were inconsistent names during the interview; canonical terms are **Canonical knob** (large, solid disc) and **Outline knob** (small, outlined disc).
