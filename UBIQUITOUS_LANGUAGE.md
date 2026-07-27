# Ubiquitous Language

Glossary for the Synthortion audio plugin: a minimal-JUCE interim state before a WebView-based UI rewrite.

## Architecture layers

| Term                 | Definition                                                                    | Aliases to avoid        |
| -------------------- | ----------------------------------------------------------------------------- | ----------------------- |
| **Audio engine**     | The `PluginProcessor` + four DSP modules; processes audio, owns APVTS. Untouched across UI changes. | Processor, audio core, back end |
| **Minimal editor**   | The temporary `PluginEditor` (stock JUCE controls, no custom L&F, no Timer). Removed everything decorative. | Stock editor, stripped editor, interim UI |
| **Future WebView UI**| The planned next-step editor using `juce::WebBrowserComponent` (HTML+CSS+JS in a webview host). | Web UI, webview editor, redesign |
| **APVTS**            | `juce::AudioProcessorValueTreeState`: the parameter state manager binding DAW automation ↔ UI sliders ↔ DSP params. | Parameter tree, value tree |

## UI controls (minimal editor)

| Term              | Definition                                                            | Aliases to avoid                        |
| ----------------- | --------------------------------------------------------------------- | --------------------------------------- |
| **Stock slider**  | A plain `juce::Slider` drawn in rotary mode, no custom L&F.           | Knob, dial, AnimatedKnob                |
| **Stock label**   | A plain `juce::Label` displaying parameter name or numeric value.     | Title, value text, BebasNeue label      |
| **Bypass toggle** | A plain `juce::ToggleButton` bound to the `PLUGIN_BYPASS` parameter.  | Push button, bypass switch, LED button  |
| **Attachment**    | A `SliderAttachment` or `ButtonAttachment` linking a stock control to an APVTS parameter ID. | Binding, connection, param link         |
| **Value formatter**| Static function (`formatDB`, `formatPercentage`, `formatMilliseconds`) converting raw float → display string. | Value text callback, display format     |

## Parameters

| Term            | Parameter ID       | Range          | Default | Domain meaning                                      |
| --------------- | ------------------ | -------------- | ------- | --------------------------------------------------- |
| **Input Gain**  | `INPUT_GAIN`       | –60..+12 dB    | 0 dB    | Pre-DSP level trim.                                 |
| **Color**       | `COLOR`            | 0..1           | 0       | Drive intensity — the main distortion character. Labeled "COLOR" on the front-panel. |
| **Bit Crush**   | `BITCRUSH`         | 0..1           | 0       | Sample-rate/bit-depth reduction mix.                 |
| **Chorus Mix**  | `CHORUS_MIX`       | 0..1           | 0       | Wet/dry blend of the 3-LFO chorus.                  |
| **Delay Time**  | `DELAY_TIME`       | 1..2000 ms     | 250 ms  | Ping-pong delay interval.                            |
| **Delay Feedback**| `DELAY_FEEDBACK` | 0..0.95        | 0.4     | Feedback amount for the delay repeats.               |
| **Delay Mix**   | `DELAY_MIX`        | 0..1           | 0       | Wet/dry blend of the delay output.                   |
| **Output Gain** | `OUTPUT_GAIN`      | –60..+12 dB    | 0 dB    | Post-DSP level trim.                                 |
| **Volume Compensation**| `VOLUME_COMPENSATION`| 0..1 (bool) | 1    | Toggle: compensates loudness increase from distortion. Not exposed in stock UI. |
| **Bypass**      | `PLUGIN_BYPASS`    | false/true     | false   | Master bypass: skips all DSP modules when active.    |

## Sections (layout regions)

| Term              | Definition                                                                  | Aliases to avoid             |
| ----------------- | --------------------------------------------------------------------------- | ---------------------------- |
| **Header**        | Top 60 px bar: "Synthortion" title left, bypass toggle right. No panel.     | Top bar, title bar, header strip |
| **Sidebar**       | Left 120 px column: Input Gain slider top half, Output Gain slider bottom half. | Input/output bar, lateral panel |
| **Distortion section** | Main area slot: Color (drive) + Bit Crush sliders. Below the header separator, bounded at y=240. | Drive block, COLOR panel   |
| **Chorus section**| Single-slider slot for Chorus Mix, below the distortion area.               | Chorus block, modulation slot |
| **Delay section** | Three-slider slot: Time + Feedback + Mix.                                   | Delay block, echo section   |
| **Coming Soon**   | Top-right 200×160 area: text-only `Label` reading "COMING SOON". Placeholder for future expansion. | Filler, spare slot           |
| **Separator**     | A 1 px black horizontal line drawn at section boundaries in `paint()`.       | Divider rule, grid line     |

## Sections (physical layout sketch)

```
+-------------------------------------------------------------------+
| [Synthortion]                                          [BYPASS ☐] |  ← Header (y 0–60)
+-------------------------------------------------------------------+
|  ║             |  DISTORTION                      DELAY            |
|  ║  INPUT      |  ⊙ COLOR    ⊙ BIT CRUSH          ⊙ TIME  ⊙ FDBK ⊙ MIX |
|  ║  [gain]     |                                     [Coming Soon] |
|  ║             |  CHORUS                                           |
|  ║  OUTPUT     |  ⊙ CHORUS MIX                                      |
|  ║  [gain]     |                                                    |
+-------------------------------------------------------------------+
```

## DSP modules

| Term                 | Class                    | Role                                                     |
| -------------------- | ------------------------ | -------------------------------------------------------- |
| **WarmDistortion**   | `WarmDistortion`         | Tape-style asymmetric saturation with drive-dependent filter, pink noise, wow/flutter. |
| **BitCrusher**       | `BitCrusher`             | Sample-rate and bit-depth reducer.                       |
| **Chorus**           | `SynthortionChorus`      | 3-LFO stereo chorus with crossover split.                |
| **Delay**            | `PingPongDelay`          | Stereo ping-pong delay with damping filter.              |
| **Ring buffer**      | `AudioScopeRingBuffer`   | Lock-free dual-channel buffer feeding the (now removed) oscilloscope. **Deleted.** |

## Relationships

- The **Minimal editor** is an interim state before the **Future WebView UI** replaces it entirely.
- Each **Stock slider** is bound to one **Parameter** via an **Attachment** and the **APVTS**.
- When **Bypass** is active, the entire DSP chain (WarmDistortion → BitCrusher → Chorus → Delay) is skipped in `processBlock`; only Input/Output Gains pass through.
- The **Audio engine** is shared across all editor states and is never modified.
- Sections are purely visual — no `PanelComponent`, no custom container. Separators are painted directly.

## Example dialogue

> **Dev:** "We're deleting every custom component — `AnimatedKnob`, `PanelComponent`, `SynthortionLookAndFeel`, `GlitchOverlay`, the oscilloscope, the meters. What does the user see?"

> **Domain expert:** "A plain window with eight **stock sliders** (rotary), their labels, and a **bypass toggle**. Positions match the old layout: input/output on the left sidebar, distortion up top, chorus below, delay on the right. Dark grey fill, 1 px black **separators**. That's it. No animations, no glitch, no custom fonts."

> **Dev:** "And automation still works?"

> **Domain expert:** "Yes — each **stock slider** is bound to its **Parameter** via an **Attachment**. DAW automation and preset recall are unchanged. The **Audio engine** and **APVTS** are untouched."

> **Dev:** "What about the oscilloscope and meters? Those were fed from the ring buffer."

> **Domain expert:** "The **Ring buffer** is deleted. No visualisation. Later, when we build the **Future WebView UI**, we'll re-add monitoring in HTML — but right now the scope and meters simply don't exist."

> **Dev:** "When the user toggles bypass, does the audio crossfade smoothly?"

> **Domain expert:** "No — it's a hard mute of the DSP chain. The old visual crossfade (`bypassMix`) was rendered by `AnimationController` and only cosmetic. The processor has always toggled bypass with a boolean check. No behaviour changed."

## Flagged ambiguities

- **"Knob" / "slider"**: The previous glossary used "Knob" for the custom `AnimatedKnob` (segment-arc rotary with pointer/detent). In the minimal editor, "Slider" refers to a stock `juce::Slider` in rotary mode. These are semantically different: a knob has discrete steps and hardware-face rendering; a stock slider has none of that. Always use **Stock slider** for the interim, **Knob** only when referring to the removed custom component.
- **"Bypass button" / "Bypass toggle"**: The old UI had a `BypassSwitch` + `BypassComponent` with push-button animation, LED, bezel. The new editor uses a plain `juce::ToggleButton`. Use **Bypass toggle** for the new control; **Push button** for the removed component.
- **"Panel"**: Previously a `PanelComponent` with dark fill, section header, and decorative border. Now "section" is just a layout region bounded by painted separator lines. No container object exists.
- **"Glitch" / "CRT"**: These terms (Dither, Scanline, Sweep, Burst, Slice, Flicker) described the `GlitchOverlay` class. That class is deleted. These words no longer refer to any active UI element. Use only in historical context.
- **"AnimationController" / "VBlank" / "Bypass mix"**: All refer to deleted animation infrastructure. The bypass mix uniform was a visual fade value; the audio never used it. Dead concepts.
- **"BebasNeue" / "Montserrat"**: The bundled fonts are deleted. The old usage of these fonts for section headers (BebasNeue) and value labels (Montserrat) is replaced by JUCE default typeface. These font names are now historical only.
