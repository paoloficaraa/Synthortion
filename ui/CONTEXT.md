# Synthortion — Context

## Terms

- **Synthortion** — the VST plugin product. Its UI is a React web app (`ui/`) that runs inside a host WebView; the DSP engine is a future C++ bridge behind the `DspBridge` seam.
- **Faceplate** — the main control surface (the center hub of the UI) that exposes every plugin parameter.
- **Digital instrument** — the aesthetic direction of the UI: flat, precise, digital (luma-inspired). Explicitly *not* skeuomorphic hardware: no metal gradients, rack screws, bezels, or recessed-well shadows.
- **ASCII surface** — any information display rendered in character glyphs (block-element meters, braille waveform, VGA-style readouts, box-drawing frames) rather than vector primitives. The voice of the terminal inside the digital instrument.
- **Deadlock** — the legacy visual identity (dark industrial & glitch brutalism). Referenced as a stylistic ancestor, not the current direction.
- **Monochrome discipline** — the UI stays high-contrast white/gray/black (Deadlock identity). Saturated color accents are deferred, not part of the current direction; only the existing warm-grey `--accent` and `--danger` red remain.
- **Module** — a processing unit on the faceplate, each with its own power state: **Drive** (DRV), **Bitcrush** (BCR), **Delay** (DLY), **Chorus** (CHR).
- **Drive** — the distortion stage of the plugin (colloquially "warm distortion"); the canonical module name is Drive, not Warm Distortion.
- **Power** — the per-module on/off state (each module can be powered independently, alongside the master `engineActive` bypass).
- **Tweak-driven glitch** — the signature behavior of the visualizer: the scope's ASCII field corrupts proportionally to live parameter-interaction intensity (drag velocity × amplitude) and decays in ~300–500ms. At rest the trace is clean; module power toggles fire a short burst; bypass fires one heavy corruption frame. No dedicated glitch control exists on the faceplate.
- **WebView bridge** — the IPC mechanism facilitating communication between the UI's `DspBridge` interface and the plugin's `AudioProcessor`.
- **Parameter ID** — a unique string identifier mapping a `PluginState` property to an `AudioProcessorValueTreeState` parameter.
- **Control micro-glitch** — during active dragging of a knob or fader, subtle character noise pulses on the control borders/track while preserving 100% legibility of the numeric readout.
- **Boot sequence** — the mount-time terminal overlay: one-shot, staged lines with real values and aligned `[ OK ]` columns, ending on `[READY]`. Skippable by click/Enter; reduced-motion renders the final state immediately.
- **Trim Fader** — the vertical ASCII block fader located in the 48px flanking meter rails (IN/OUT TRIM), replacing horizontal track knobs on the rails to preserve strict vertical alignment and prevent rail overflow.
- **Real-Time Spectrum Analyzer** — the real-time frequency-domain visualization band replacing the dual-mode oscilloscope and waterfall. Receives log-spaced frequency magnitude bins from the C++ DSP engine via the WebView bridge at 60 FPS, rendered on a 240px canvas as a hybrid Braille peak curve with Xerox dither fill (`░▒▓`) on a 20Hz–20kHz logarithmic scale with instant attack and exponential decay ballistics.
- **FFT Bridge Stream** — the lock-free C++ to WebView IPC stream pushing normalized frequency bin data (`spectrumFrame`) from JUCE to the React visualizer at 60Hz.
- **Spectrum Ballistics** — peak tracking with instantaneous attack and smooth exponential decay (~200–300ms) applied to magnitude bins for fluid transient visualization.
- **Cartesian Frame System** — the structural panel boundary system replacing repetitive mock ASCII with precise 1px coordinate rules, crosshairs (`+`), micro-scale calibration ticks, and dithered corner anchors derived from industrial xerox/photocopy aesthetics.
- **Smooth ASCII Knob Slider** — a continuous sub-cell dithered horizontal block slider preserving the canonical bracketed format (`[...]`) while delivering sub-pixel vertical drag responsiveness, fine-step scaling, and jitter-free micro-glitch tactile feedback.

## Bridge & Integration Terms

- **DspBridge** — the integration seam (TypeScript interface in `ui/src/lib/dspBridge.ts`) between the React UI and the C++ DSP backend. Declares `setParameter(parameterId, value)`. Two implementations exist: `noopDspBridge` (no-op, for dev) and the future `WebViewDspBridge` (talks to JUCE `WebBrowserComponent`).
- **Parameter ID** — the canonical identifier for a plugin parameter, matching the JUCE `AudioProcessorValueTreeState` parameter ID (e.g., `INPUT_GAIN`, `COLOR`, `DELAY_TIME`). The single source of truth for parameter addressing across the UI↔DSP boundary.
- **APVTS** — `AudioProcessorValueTreeState`, JUCE's parameter management system. Owns parameter layout, normalization (0..1), automation, preset state, and host synchronization.
- **Normalization layer** — the TypeScript module (`ui/src/lib/parameterSpecs.ts`) that converts between **UI values** (user-facing: dB, Hz, ms, %, semantic enums) and **APVTS values** (normalized 0..1 floats). Driven by a declarative `parameterSpecs` table.
- **Bridge protocol** — the JSON message format exchanged over the WebView channel:
  - JS→C++: `{ type: "setParameter", id: "COLOR", value: 0.42 }`
  - C++→JS: `window.__SYNTORTION_BRIDGE__.onParameterChange("COLOR", 0.42)`
  - Lifecycle: `{ type: "connect" }` / `onConnect({ sampleRate, blockSize, parameterSpecs })` / `{ type: "disconnect" }`
- **UI-only state** — `PluginState` fields with no APVTS counterpart (`driveRoute`, `driveOn`, `bitcrushOn`, `delayOn`, `chorusOn`, `delaySync`, `chorusWide`). These remain in React state and are never sent over the bridge.
- **WebBrowserComponent** — the JUCE class that hosts a WebView (WebView2 on Windows, WebKit on macOS/Linux) and exposes a JavaScript↔C++ message channel. The PluginEditor embeds it; the PluginProcessor receives parameter changes via callback.
- **Parameter spec table** — the declarative source of truth for normalization (`parameterSpecs.ts`). Each entry defines: `uiKey` (PluginState field), `min`/`max` (UI range), `skew` (for skewed ranges like gain), `unit` (display), and optional `invert` (for bypass).
