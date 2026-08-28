# Synthortion — Context

## Terms

- **Synthortion** — the VST plugin product. Its UI is a React web app (`ui/`) that runs inside a host WebView; the DSP engine is a future C++ bridge behind the `DspBridge` seam.
- **Faceplate** — the main control surface (the center hub of the UI) that exposes every plugin parameter.
- **Digital instrument** — the aesthetic direction of the UI: flat, precise, digital (luma-inspired). Explicitly *not* skeuomorphic hardware: no metal gradients, rack screws, bezels, or recessed-well shadows.
- **ASCII surface** — any information display rendered in character glyphs (block-element meters, braille waveform, VGA-style readouts, box-drawing frames) rather than vector primitives. The voice of the terminal inside the digital instrument.
- **Deadlock** — the legacy visual identity (dark industrial & glitch brutalism). Referenced as a stylistic ancestor, not the current direction.
- **Monochrome discipline** — the UI stays strict high-contrast white/gray/black. 0% chromatic saturation: no warm tints or red accents; hierarchy is expressed via typography, character density (░▒▓█), and luminance ramps.
- **Module** — a processing unit on the faceplate, each with its own power state: **Drive** (DRV), **Bitcrush** (BCR), **Delay** (DLY), **Chorus** (CHR).
- **Drive** — the distortion stage of the plugin (colloquially "warm distortion"); the canonical module name is Drive, not Warm Distortion.
- **Power** — the per-module on/off state (each module can be powered independently, alongside the master `engineActive` bypass).
- **Tweak-driven glitch** — the signature behavior of the visualizer: the scope's ASCII field corrupts proportionally to live parameter-interaction intensity (drag velocity × amplitude) and decays in ~300–500ms. At rest the trace is clean; module power toggles fire a short burst; bypass fires one heavy corruption frame. No dedicated glitch control exists on the faceplate.
- **WebView bridge** — the IPC mechanism facilitating communication between the UI's `DspBridge` interface and the plugin's `AudioProcessor`.
- **Parameter ID** — a unique string identifier mapping a `PluginState` property to an `AudioProcessorValueTreeState` parameter.
- **Control micro-glitch** — during active dragging of a knob, subtle border flicker and ±1px CRT scanline jitter pulse on the track; rapid dragging triggers intermittent digital corruption glyphs that decay exponentially in ~120–150ms, while rapid adjustments and double-click reset trigger a 30–50ms terminal hex decoding animation on the value readout before cleanly settling.
- **Boot sequence** — the mount-time terminal overlay: one-shot, staged lines with real values and aligned `[ OK ]` columns, ending on `[READY]`. Skippable by click/Enter; reduced-motion renders the final state immediately.
- **Trim Fader** — the vertical ASCII block fader located in the 48px flanking meter rails (IN/OUT TRIM), replacing horizontal track knobs on the rails to preserve strict vertical alignment and prevent rail overflow.
- **Real-Time Spectrum Analyzer** — the real-time frequency-domain visualization band replacing the dual-mode oscilloscope and waterfall. Receives log-spaced frequency magnitude bins from the C++ DSP engine via the WebView bridge at 60 FPS, rendered on a proportional ~35% height canvas as a hybrid Braille peak curve with Xerox dither fill (`░▒▓`) on a 20Hz–20kHz logarithmic scale with instant attack and exponential decay ballistics.
- **FFT Bridge Stream** — the lock-free C++ to WebView IPC stream pushing normalized frequency bin data (`spectrumFrame`) from JUCE to the React visualizer at 60Hz.
- **Spectrum Ballistics** — peak tracking with instantaneous attack and smooth exponential decay (~200–300ms) applied to magnitude bins for fluid transient visualization.
- **Cartesian Frame System** — the structural panel boundary system replacing repetitive mock ASCII with precise 1px coordinate rules, crosshairs (`+`), micro-scale calibration ticks, and dithered corner anchors derived from industrial xerox/photocopy aesthetics.
- **Smooth ASCII Knob Slider** — a continuous sub-cell dithered horizontal block slider preserving the canonical bracketed format (`[...]`) while delivering sub-pixel vertical drag responsiveness, fine-step scaling, velocity-reactive track glitch decay, and terminal hex numeric decoding.

## Bridge & Integration Terms

- **DspBridge** — the integration seam (TypeScript interface in `ui/src/lib/dspBridge.ts`) between the React UI and the C++ DSP backend. Declares `setParameter(id, value)`.
- **Parameter ID** — the canonical identifier for a plugin parameter, matching the JUCE `AudioProcessorValueTreeState` parameter ID (e.g., `INPUT_GAIN`, `COLOR`, `DELAY_TIME`). The single source of truth for parameter addressing across the UI↔DSP boundary.
- **APVTS** — `AudioProcessorValueTreeState`, JUCE's parameter management system. Owns parameter layout, normalization (0..1), automation, preset state, and host synchronization.
- **Native Event Bridge Protocol** — the unified IPC event mechanism using JUCE 8 `WebBrowserComponent` native event emission and listeners (`window.__JUCE__.backend.emitEvent` / `window.__JUCE__.backend.addEventListener`). Eliminates legacy `evaluateJavascript` and `window.__SYNTORTION_BRIDGE__` dual dispatch.
- **Handshake Protocol** — the reactive initialization sequence where the UI emits `connect` upon mounting and C++ responds with `init` carrying `schemaVersion: 1` and the complete array of parameter descriptors (`ParameterDescriptor[]`).
- **Normalized IPC Parameter Events** — `setParameter` (UI → C++) and `parameterChange` (C++ → UI) both using `{ id: string, value: number }` with normalized `[0.0, 1.0]` floats matching JUCE APVTS host automation.
- **Telemetry Frame Streams** — 60 FPS lock-free telemetry events (`spectrumFrame` passing `number[]` magnitudes, `meterFrame` passing `{ input: number, output: number }` peaks) dispatched via `emitEventIfBrowserIsVisible`, automatically dropping when the editor is occluded or closed.
- **Host Parameter Mapping** — all plugin parameters (including module power states `DRIVE_ON`, `BITCRUSH_ON`, `DELAY_ON`, `CHORUS_ON`, routing `DRIVE_ROUTE`, delay sync `DELAY_SYNC`, and chorus width `CHORUS_WIDE`) are registered in APVTS and synchronized across the bridge.
- **WebBrowserComponent** — the JUCE class that hosts a WebView (WebView2 on Windows, WebKit on macOS/Linux) and exposes a JavaScript↔C++ message channel. The PluginEditor embeds it; the PluginProcessor receives parameter changes via callback.
## DSP Architecture Terms

- **DSP Module Concept (`synthortion::dsp::DspModule`)** — the static C++20 interface contract adhered to by all headless DSP components (`WarmDistortion`, `BitCrusher`, `PingPongDelay`, `SynthortionChorus`), requiring `prepare(const juce::dsp::ProcessSpec&)`, `process(juce::AudioBuffer<float>&, const Params&)`, `reset() noexcept`, and `getLatencySamples() const noexcept`.
- **Parameter Structs** — plain-old-data structs (`WarmDistortionParams`, `BitCrusherParams`, `PingPongDelayParams`, `ChorusParams`) passed per-block into DSP module `process()` calls, decoupling internal DSP state from APVTS and atomic pointer lookups.
- **Encapsulated Smoothing** — internal parameter smoothing owned by individual DSP modules via `juce::SmoothedValue` or `juce::LinearSmoothedValue` rather than hoisted into `PluginProcessor`, ensuring headless modules avoid parameter zipper noise during isolated execution and unit testing.
- **UI Preferences ValueTree (`UIPreferences`)** — a dedicated child subtree within the APVTS root `Parameters` ValueTree holding persistent non-DSP UI preferences (`uiScale`, `spectrumDecay`, `skipBootSequence`) saved atomically in DAW sessions and presets.
- **Synchronous State Serialization** — strictly executing `getStateInformation` and `setStateInformation` synchronously via JUCE XML binary containers (`copyXmlToBinary` / `getXmlFromBinary`), eliminating detached background threads and race conditions during host session loading.

## Effect Suite Overhaul Terms

- **Dynamic Bias Asymmetric Saturation** — the mathematical transfer function $f(x, b) = \frac{\tanh(x+b)-\tanh(b)}{1-\tanh^2(b)}$ with $b(d) = 0.25d(1-0.4d)$ and power-law drive tapering ($d^{2.2}$), introducing subtle 2nd-order harmonic warmth for synthesizer waveforms without harsh break-up at low settings.
- **Progressive Lo-Fi Degradation Curve** — the coupled parameter curve in the Bitcrusher smoothly interpolating bit depth (16-bit to 4-bit) and downsampling (48 kHz to 2 kHz) along a gentle musical trajectory with anti-aliasing interpolation.
- **Host-Synced Ping-Pong Delay** — the dual-mode stereo ping-pong delay with toggleable timebase: `SYNC` (14 discrete musical subdivisions from 1/32 to 1/1 synchronized with DAW BPM via JUCE `AudioPlayHead`) or `FREE` (1 ms to 2000 ms continuous).
- **FL-Style Vintage Stereo Chorus** — the 3-voice BBD delay chorus architecture with independent multi-rate LFOs (0.45 Hz, 1.25 Hz, 2.45 Hz), Linkwitz-Riley 4th-order low-cut crossover at 320 Hz for mono low-end preservation, and volume-normalized multi-tap summing ($G_{\text{norm}} \approx 0.4387$).
- **Chorus Width Scaling** — the stereo phase offset control modulating the inter-channel LFO phase relationship ($0^\circ \to 90^\circ$) to allow deep chorus detune without exaggerated stereo disassociation.

## Preset System Terms

- **Preset Format (`.synthortionpreset`)** — the versioned JSON schema containing metadata (`name`, `category`, `author`, `description`, `tags`), APVTS parameter mappings, and `uiPreferences`.
- **Factory Presets** — immutable presets embedded directly into C++ binary data, accessible without disk dependencies and exposed to DAW host program lists.
- **User Presets** — user-created preset files stored in standard OS application data paths (`%APPDATA%/Synthortion/Presets/` on Windows, `~/Library/Audio/Presets/Synthortion/` on macOS) with category subfolders.
- **Preset Browser Modal** — the terminal-styled ASCII modal overlay in the React UI providing category navigation, search filtering, tag inspection, preset deletion, and direct preset recall.
- **Preset Native Bridge Protocol** — the JUCE 8 native event IPC events (`requestPresetList`, `presetListUpdated`, `loadPreset`, `savePreset`, `deletePreset`, `presetLoaded`) coordinating file I/O, APVTS synchronization, and UI state hydration.
- **PresetManager** — the C++ subsystem owned by `AudioPluginAudioProcessor` orchestrating immutable embedded factory presets, OS directory scanning, atomic file I/O via `juce::TemporaryFile`, in-memory catalog indexing, and synchronous APVTS state replacement.
- **Preset Catalog** — the unified in-memory registry of preset descriptors (`PresetHeader`) combining embedded factory presets and indexed user presets for 60 FPS ASCII browser navigation without disk latency.
- **Atomic Preset Swap** — the disk writing safeguard utilizing `juce::TemporaryFile` to write JSON payloads to a temporary file before atomically replacing the destination `.synthortionpreset`, preventing file corruption during host crashes or write interruptions.
- **Dirty Preset Indicator (`*`)** — the visual asterisk flag rendered in the VGA header readout when live APVTS parameter state deviates from the currently loaded preset snapshot.
- **Factory Preset Registry** — the immutable in-memory array of embedded `.synthortionpreset` JSON resources compiled into the plugin binary via CMake `juce_add_binary_data`.
