# Ubiquitous Language

Glossary for the Synthortion audio plugin: a WebView-based React UI replacing the minimal-JUCE interim editor.

## Architecture layers

| Term                 | Definition                                                                    | Aliases to avoid        |
| -------------------- | ----------------------------------------------------------------------------- | ----------------------- |
| **Audio engine**     | The `PluginProcessor` + four DSP modules; processes audio, owns APVTS. Untouched across UI changes. | Processor, audio core, back end |
| **WebView UI**       | The production editor: a `juce::WebBrowserComponent` hosting a React/Vite single-page app inside the plugin window. | Web UI, webview editor, HTML editor, future UI |
| **APVTS**            | `juce::AudioProcessorValueTreeState`: the parameter state manager binding DAW automation ↔ UI controls ↔ DSP params. | Parameter tree, value tree |
| **IPC bridge**       | The serialized message channel (JSON or binary) between the WebView UI and the Audio engine's C++ thread. | Message bus, JS bridge, data link |

## Technology stack

| Term                 | Definition                                                                    | Aliases to avoid        |
| -------------------- | ----------------------------------------------------------------------------- | ----------------------- |
| **Tailwind CSS**     | Utility-first CSS framework used exclusively for design system structure, layout, spacing, and CSS variable theming. | CSS framework, utility classes |
| **Framer Motion**    | React animation library handling micro-interactions: hover states, entrance animations, and UI feedback on controls. | Animation lib, motion |
| **React Three Fiber**| React renderer for Three.js, used exclusively for the 3D FFT Visualizer scene (GLSL shaders, geometry, particles). | R3F, Three.js, WebGL layer |
| **CSS Modules**      | Vite-native scoped CSS files (`.module.css`) co-located with components for styles that exceed Tailwind utilities. | Scoped CSS, local styles |

## Visual identity

| Term                     | Definition                                                              | Aliases to avoid        |
| ------------------------ | ----------------------------------------------------------------------- | ----------------------- |
| **Glitch Brutalism**     | The plugin's design language: sharp corners, hard shadows, monochromatic palette, no soft gradients or rounded edges. | Dark theme, industrial theme, brutalist |
| **Vintage Industrial palette** | The three-color foundation of the UI: `#0f0e0e` (dark), `#f6f6f6` (light), `#c7c3ba` (warm accent). | Color scheme, theme colors |
| **Noise overlay**        | A fractal-noise SVG texture composited over the entire plugin chassis via CSS `::after`, giving a subtle grain. | Grain, texture, dither |

## UI components

| Term                 | Definition                                                                    | Aliases to avoid        |
| -------------------- | ----------------------------------------------------------------------------- | ----------------------- |
| **VstLayout**        | The root layout component: a 3-column `flex-row` container (Left Meter, Center Hub, Right Meter) with rack screws. | Main container, shell, wrapper |
| **PresetHeader**     | The top 64 px bar containing the bypass toggle, plugin title, and the embedded LCD preset selector. | Header, top bar, title bar |
| **Knob**             | A vector SVG rotary control with polar math for the notch position, vertical pointer-drag interaction, and `focus-visible` accessibility. | Dial, rotary, slider, stock slider |
| **Value arc**        | The circular SVG stroke around a Knob that fills proportionally to the current parameter value; colored `#c7c3ba`. | Circus, active arc, progress ring, indicator |
| **ToggleSwitch**     | A binary button (e.g., PRE/POST, WIDE) bound to a boolean parameter, styled as a flat Brutalist pill. | Button, switch, selector |
| **GainMeter**        | A full-height 2D Canvas component rendering 32 discrete segmented blocks to visualize signal level (Input or Output). | VU meter, level meter, volume bar |
| **FftVisualizer**    | A React Three Fiber `<Canvas>` scene rendering audio-reactive 3D geometry with hard shadows, sharp corners, and glitchy particles. | Spectrum, analyser, oscilloscope, waveform |
| **Glitch particles** | Lightweight floating particle systems inside the FftVisualizer, adding subtle motion and industrial atmosphere. | Dust, sparks, ambient particles |

## Parameters

| Term            | Parameter ID       | Range          | Default | Domain meaning                                      |
| --------------- | ------------------ | -------------- | ------- | --------------------------------------------------- |
| **Input Gain**  | `INPUT_GAIN`       | –24..+24 dB    | 0 dB    | Pre-DSP level trim, controlled by the left GainMeter's TRIM Knob. |
| **Drive**       | `COLOR`            | 0..100%        | 0       | Drive intensity — the main distortion character. Labeled "DRV" on the faceplate. |
| **Bit Crush**   | `BITCRUSH`         | 0..100%        | 0       | Sample-rate/bit-depth reduction mix. Labeled "BCR". |
| **Delay Time**  | `DELAY_TIME`       | 1..2000 ms     | 250 ms  | Ping-pong delay interval. Labeled "DLY".            |
| **Delay Feedback**| `DELAY_FEEDBACK` | 0..95%         | 40%     | Feedback amount for the delay repeats.               |
| **Delay Mix**   | `DELAY_MIX`        | 0..100%        | 0       | Wet/dry blend of the delay output.                   |
| **Chorus Mix**  | `CHORUS_MIX`       | 0..100%        | 0       | Wet/dry blend of the 3-LFO chorus. Labeled "CHR".   |
| **Chorus Wide** | `CHORUS_WIDE`      | false/true     | false   | Toggle: widens the stereo image of the chorus effect. |
| **Output Gain** | `OUTPUT_GAIN`      | –24..+24 dB    | 0 dB    | Post-DSP level trim, controlled by the right GainMeter's TRIM Knob. |
| **Bypass**      | `PLUGIN_BYPASS`    | false/true     | false   | Master bypass: skips all DSP modules when active.    |

## Layout regions (3-column design)

| Term                   | Definition                                                                     | Aliases to avoid           |
| ---------------------- | ------------------------------------------------------------------------------ | -------------------------- |
| **Left Meter column**  | Narrow left column: full-height GainMeter (Input) with embedded TRIM Knob, bordered right. | Input sidebar, left panel |
| **Center Hub**         | Wide central column: PresetHeader on top, FftVisualizer below it, 5-knob Matrix Faceplate at the bottom. | Main area, center panel |
| **Right Meter column** | Narrow right column: full-height GainMeter (Output) with embedded TRIM Knob, bordered left. | Output sidebar, right panel |
| **Matrix Faceplate**   | A `grid-cols-5` area inside the Center Hub containing the five main DSP Knobs (DRV, BCR, DLY, DLY-FBK/MIX, CHR) with their ToggleSwitches. | Controls grid, knob panel |

## DSP modules

| Term                 | Class                    | Role                                                     |
| -------------------- | ------------------------ | -------------------------------------------------------- |
| **WarmDistortion**   | `WarmDistortion`         | Tape-style asymmetric saturation with drive-dependent filter, pink noise, wow/flutter. |
| **BitCrusher**       | `BitCrusher`             | Sample-rate and bit-depth reducer.                       |
| **Chorus**           | `SynthortionChorus`      | 3-LFO stereo chorus with crossover split.                |
| **Delay**            | `PingPongDelay`          | Stereo ping-pong delay with damping filter.              |

## Relationships

- The **WebView UI** replaces the deleted Minimal editor entirely; it is hosted inside `juce::WebBrowserComponent`.
- The **WebView UI** communicates with the **Audio engine** exclusively through the **IPC bridge**; no shared memory crosses the boundary.
- Each **Knob** is a controlled React component (`value`, `onChange`) whose top-level state in `App.tsx` maps 1:1 to an **APVTS** parameter via the **IPC bridge**.
- A **GainMeter** contains exactly one **Knob** (the TRIM control) and one 2D Canvas visualization for signal level.
- The **FftVisualizer** renders inside a React Three Fiber `<Canvas>` and reacts to the `engineActive` state; it contains sharp 3D geometry and **Glitch particles**.
- The **Value arc** is the only UI element using the `#c7c3ba` accent color; all other elements use `#0f0e0e` (dark) or `#f6f6f6` (light).
- The **Matrix Faceplate** is divided into 5 equal columns by `border` dividers, each column hosting one DSP parameter's Knob and optional ToggleSwitches.

## Example dialogue

> **Dev:** "When the user drags a **Knob** vertically, what happens on the C++ side?"

> **Domain expert:** "The **Knob** fires its `onChange` callback, which updates the controlled state in `App.tsx`. That state change is serialized and sent over the **IPC bridge** to the **Audio engine**, which writes it into the **APVTS**. DAW automation records from there."

> **Dev:** "And the **GainMeter** — is it reading real audio levels?"

> **Domain expert:** "Not yet. Right now the **GainMeter** renders simulated blocks using `Math.random()`. Once the **IPC bridge** is wired, the **Audio engine** will push peak level data into the **WebView UI**, and the **GainMeter** canvas will consume that instead."

> **Dev:** "The **FftVisualizer** is being upgraded to 3D. Does the 3D scene need to match the old 2D bars exactly?"

> **Domain expert:** "No. The 3D scene introduces sharp geometric primitives with hard shadows and **Glitch particles**. The math for audio reactivity (vertex displacement driven by signal) carries over, but the visual output is intentionally different — it's an upgrade, not a port. It must still respect **Glitch Brutalism**: no soft edges, no bloom, no rounded forms."

> **Dev:** "What color does the **Value arc** use?"

> **Domain expert:** "Always `#c7c3ba` from the **Vintage Industrial palette**. That warm tone is reserved exclusively for the arc stroke. The Knob body and track use `#0f0e0e` and `#f6f6f6`."

## Flagged ambiguities

- **"Circus" / "Value arc"**: During planning, "circus" was used informally to mean the circular SVG stroke that tracks the Knob's current value. The canonical term is **Value arc**. "Circus" must not appear in code or documentation.
- **"Knob" / "Stock slider"**: In the previous glossary, "Knob" referred to the deleted `AnimatedKnob` custom component, while "Stock slider" was the interim `juce::Slider`. Now **Knob** refers exclusively to the new React SVG rotary control. "Stock slider" is a dead term — the Minimal editor no longer exists.
- **"Minimal editor"**: The temporary JUCE-only editor described in the previous glossary. It has been fully superseded by the **WebView UI**. Do not reference it as a current or future state.
- **"FFT"**: The FftVisualizer does not perform an actual Fast Fourier Transform. It uses procedural math (sin/cos structural noise) to simulate audio-reactive motion. The name is aspirational — actual FFT data will arrive later via the **IPC bridge**. Do not assume real frequency analysis is happening.
- **"Shadow"**: In the context of the **FftVisualizer**, shadows are hard-edged and cast by directional lights onto flat geometry. In the context of CSS (e.g., `box-shadow` on the chassis), shadows are inset industrial effects. These are visually and technically distinct.
