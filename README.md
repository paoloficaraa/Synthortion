# Synthortion

![Version](https://img.shields.io/badge/version-0.1.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![C++](https://img.shields.io/badge/C%2B%2B-20-orange.svg)
![JUCE](https://img.shields.io/badge/JUCE-8.0%2B-purple.svg)
![React](https://img.shields.io/badge/React-19-61dafb.svg)
![TypeScript](https://img.shields.io/badge/TypeScript-5.0%2B-blue.svg)
![Formats](https://img.shields.io/badge/formats-VST3%20%7C%20AU-lightgrey.svg)

**Synthortion** is a professional distortion and creative multi-effects plugin built with **JUCE 8** and **C++20**, featuring an integrated **React 19 / TypeScript** user interface embedded via a high-performance native WebView bridge.

Combining tape-inspired non-linear saturation, lo-fi bitcrushing, vintage 3-voice BBD chorus, and host-synced ping-pong delay, Synthortion wraps high-grade DSP in a strict monochrome digital instrument aesthetic with ASCII-terminal surfaces and a 60 FPS real-time Braille / Xerox dither spectrum visualizer.

---

## Architecture

Synthortion couples a headless C++20 DSP engine to a modern web frontend via JUCE 8's native event IPC:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                 HOST DAW                                    │
│  (Audio Processing, Parameter Automation, Preset Management, Playhead/BPM)  │
└──────────────────────────────────────┬──────────────────────────────────────┘
                                       │
                                       ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           JUCE AudioProcessor                               │
│  ┌────────────────────────┐                  ┌───────────────────────────┐  │
│  │         APVTS          │                  │     AudioCaptureFifo      │  │
│  │  (Parameters & State)  │                  │  (Lock-free Audio Ring)   │  │
│  └───────────┬────────────┘                  └─────────────┬─────────────┘  │
│              │                                             │                │
│              ▼                                             ▼                │
│  ┌────────────────────────┐                  ┌───────────────────────────┐  │
│  │       DSP Chain        │                  │     SpectrumAnalyzer      │  │
│  │ WarmDistortion         │                  │  2048-pt FFT, 80 Bands,   │  │
│  │ BitCrusher             │                  │  Peak / Decay Ballistics  │  │
│  │ SynthortionChorus      │                  └─────────────┬─────────────┘  │
│  │ PingPongDelay          │                                │                │
│  └────────────────────────┘                                │                │
└──────────────────────────────────────┬─────────────────────┼────────────────┘
                                       │                     │
         JUCE 8 Native IPC             │ setParameter /      │ spectrumFrame (60Hz)
         (WebBrowserComponent)         │ parameterChange     │ meterFrame (60Hz)
                                       ▼                     ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           React 19 WebView UI                               │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  Matrix Faceplate & Cartesian Frame System (1px hairlines, crosshairs)│  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │  60 FPS Braille (U+2800) & Xerox Dither (░▒▓) Real-Time Visualizer     │  │
│  ├───────────────────────────────────────────────────────────────────────┤  │
│  │  Smooth Sub-Cell ASCII Knob Sliders & Faders with Track Micro-Glitch   │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Key Features

### 1. Drive (Warm Distortion)
- **Dynamic Bias Asymmetric Saturation:** Non-linear transfer function $f(x, b) = \frac{\tanh(x+b)-\tanh(b)}{1-\tanh^2(b)}$ with dynamic bias $b(d) = 0.25d(1-0.4d)$ for rich 2nd-order harmonic warmth.
- **Power-Law Drive Curve:** Smooth progressive drive scaling ($d^{2.2}$) ensuring subtle warmth at low settings and controlled saturation at maximum drive.
- **4x Polyphase Oversampling:** $2^2$ polyphase IIR oversampling stage to minimize aliasing.
- **Dynamic Filtering & Exciter:** Drive-dependent high-frequency low-pass damping and gated excitation stage above 5 kHz.
- **Analog Modeling:** Multi-stage pink noise, tape bias noise, and subtle wow & flutter modulation.
- **Loudness Auto-Gain Compensation:** Analytical loudness model automatically maintains perceived output levels across drive sweeps.

### 2. Creative Multi-FX
- **BitCrusher:** Progressive coupled lo-fi trajectory smoothly scaling bit depth (16-bit down to 4-bit) and sample rate (native down to 1.5 kHz) with fractional phase accumulation, linear interpolation, and TPDF dither.
- **SynthortionChorus:** Vintage 3-voice BBD chorus architecture with decoupled multi-rate LFOs (0.45 Hz, 1.25 Hz, 2.45 Hz), Linkwitz-Riley 4th-order crossover at 320 Hz for mono low-end preservation, adjustable stereo phase width spread ($0^\circ \to 60^\circ$), and volume-normalized summing.
- **PingPongDelay:** Dual-mode stereo ping-pong delay featuring **Host Sync** (14 musical subdivisions from 1/32 to 1/1 incl. triplets and dotted notes locked to DAW BPM) and **Free Time** (1 ms to 2000 ms), 3rd-order Lagrange interpolation, and a damped feedback network.
- **Flexible Routing (`DRIVE_ROUTE`):** Instantly swap the signal chain order between `PRE` (Drive → FX) and `POST` (FX → Drive).
- **Independent Module Power:** Per-module toggleable bypass alongside the master engine bypass.

### 3. Analysis & Metering
- **Real-Time Spectrum Analyzer:** 2048-sample Hann-windowed FFT pooled into 80 log-spaced frequency bands (20 Hz to 20 kHz) with sub-bin linear interpolation for low-end precision and peak bin aggregation for highs.
- **Asymmetric Ballistics:** Instantaneous peak attack and exponential decay (~180 ms) for smooth transient display.
- **Telemetric Streaming:** Lock-free, 60 FPS spectrum and peak meter frames delivered directly to the WebView canvas.
- **Discrete Peak Meters:** High-resolution input and output peak tracking with vertical ASCII block faders.

### 4. Digital Instrument UI & Design System
- **Monochrome Discipline:** Strict 100% black-and-white / technical grey aesthetic with 0% chromatic saturation.
- **Cartesian Frame System:** 1px coordinate rules (`#333333`), crosshairs (`+`) at panel junctions, and micro-scale calibration ticks.
- **ASCII Surfaces:** Braille waveform curves (`U+2800–28FF`), Xerox density dithering (`░▒▓█`), bracketed sub-cell knob sliders (`[...]`), and terminal hex-decoding animations.
- **Tweak-Driven Micro-Glitch:** Non-destructive CRT jitter and scanline pulse reactive to drag velocity and module toggles.
- **Responsive Scaling:** Responsive layout supporting interface sizes from 768×480 up to 1920×1200 (default 960×600).

---

## Processing Chain

```
Input ──► [Input Gain] ──► [Routing Switch] ──► [Output Gain] ──► Output
                                │
          ┌─────────────────────┴─────────────────────┐
          ▼ (PRE Route)                               ▼ (POST Route)
     [ Drive (DRV) ]                             [ Bitcrusher (BCR) ]
            │                                             │
     [ Bitcrusher (BCR) ]                        [ Chorus (CHR) ]
            │                                             │
     [ Chorus (CHR) ]                            [ Ping-Pong Delay (DLY) ]
            │                                             │
     [ Ping-Pong Delay (DLY) ]                   [ Drive (DRV) ]
          └─────────────────────┬─────────────────────┘
                                │
                                ▼
```

---

## Parameters

### Global & Master Controls
| Parameter ID | UI Key | Range | Default | Unit | Description |
|---|---|---|---|---|---|
| `PLUGIN_BYPASS` | `engineActive` | Bool | `false` (Active) | — | Master plugin bypass toggle (inverted in UI) |
| `INPUT_GAIN` | `inputGain` | -60.0 .. +12.0 | `0.0` | dB | Master input gain with per-sample smoothing |
| `OUTPUT_GAIN` | `outputGain` | -60.0 .. +12.0 | `0.0` | dB | Master output gain with per-sample smoothing |

### Drive Module (Warm Distortion)
| Parameter ID | UI Key | Range | Default | Unit | Description |
|---|---|---|---|---|---|
| `COLOR` | `drive` | 0.0 .. 100.0 | `40.0` | % | Distortion drive intensity & non-linear saturation |
| `DRIVE_ON` | `driveOn` | Bool | `true` | — | Drive module power toggle |
| `DRIVE_ROUTE` | `driveRoute` | Bool | `false` (PRE) | — | Processing order: `false` = PRE (Drive → FX), `true` = POST (FX → Drive) |

### Bitcrusher Module
| Parameter ID | UI Key | Range | Default | Unit | Description |
|---|---|---|---|---|---|
| `BITCRUSH` | `bitcrush` | 0.0 .. 100.0 | `0.0` | % | Lo-fi bit reduction & downsampling intensity |
| `BITCRUSH_ON` | `bitcrushOn` | Bool | `true` | — | Bitcrusher module power toggle |

### Chorus Module
| Parameter ID | UI Key | Range | Default | Unit | Description |
|---|---|---|---|---|---|
| `CHORUS_MIX` | `chorus` | 0.0 .. 100.0 | `75.0` | % | Chorus dry/wet mix balance |
| `CHORUS_WIDE` | `chorusWidth` | 0.0 .. 100.0 | `50.0` | % | Stereo LFO phase width spread ($0^\circ \to 60^\circ$) |
| `CHORUS_ON` | `chorusOn` | Bool | `true` | — | Chorus module power toggle |

### Delay Module
| Parameter ID | UI Key | Range | Default | Unit | Description |
|---|---|---|---|---|---|
| `DELAY_TIME_FREE` | `delayTimeFree` | 1.0 .. 2000.0 | `250.0` | ms | Free delay time (used when `DELAY_SYNC` is off) |
| `DELAY_TIME_SYNC` | `delayTimeSync` | Choice (0..13) | `6` (1/8D) | — | Host-synced subdivision (1/32 to 1/1, triplets, dotted) |
| `DELAY_SYNC` | `delaySync` | Bool | `true` | — | Delay timebase mode: `true` = Host Sync, `false` = Free Time |
| `DELAY_MIX` | `delayMix` | 0.0 .. 100.0 | `30.0` | % | Delay dry/wet mix balance |
| `DELAY_FEEDBACK` | `delayFbk` | 0.0 .. 95.0 | `50.0` | % | Delay feedback regeneration level |
| `DELAY_ON` | `delayOn` | Bool | `true` | — | Delay module power toggle |

### UI Preferences (Persisted in Session State)
| Preference Property | Default | Type | Description |
|---|---|---|---|
| `uiScale` | `1.0` | Double | UI display zoom scaling factor |
| `spectrumDecay` | `0.25` | Double | Spectrum analyzer ballistics decay constant |
| `skipBootSequence` | `false` | Bool | Skip terminal boot sequence on plugin load |

---

## Technical Snapshot

| Item | Specification |
|---|---|
| **C++ Standard** | C++20 |
| **Framework** | JUCE 8.0+ |
| **Frontend Stack** | React 19, TypeScript, Tailwind CSS 4, Vite |
| **Bridge IPC** | JUCE 8 `WebBrowserComponent` Native Event API |
| **Plugin Formats** | VST3, AU |
| **Audio Buses** | Stereo In / Stereo Out (Mono and Stereo host bus layouts) |
| **Distortion Oversampling** | 4x Polyphase IIR ($2^2$) |
| **FFT Engine** | 2048-point Hann FFT $\to$ 80 Log-Spaced Frequency Bands |
| **State Persistence** | Synchronous JUCE XML Binary Container with `<UIPreferences>` Tree |

---

## Installation Guide

### 🪟 Windows Installation (VST3)

1. **Download & Extract:**
   - Extract `Synthortion_Windows_x64_VST3.zip`. You will find the `Synthortion.vst3` bundle directory.

2. **Copy to VST3 Folder:**
   - Copy the entire `Synthortion.vst3` folder into the system VST3 directory:
     ```text
     C:\Program Files\Common Files\VST3\
     ```
     *(The destination must be `C:\Program Files\Common Files\VST3\Synthortion.vst3`)*.

3. **Prerequisites (WebView2):**
   - Synthortion requires the **Microsoft Edge WebView2 Runtime** (pre-installed on Windows 10/11; if missing, download it for free from [Microsoft's official site](https://developer.microsoft.com/en-us/microsoft-edge/webview2/)).

4. **Load in DAW:**
   - Open your DAW (FL Studio, Ableton Live, Reaper, Cubase, Studio One, Bitwig, etc.).
   - Run a plugin scan (**Rescan plugin list**).
   - Load **Synthortion** as an audio effect on any audio or instrument track.

---

### 🍏 macOS Installation (VST3 & Audio Unit / AU)

1. **Download & Extract:**
   - Download the artifacts from GitHub Actions:
     - `Synthortion_mac_universal_vst3.zip` (for VST3 hosts like Ableton, FL Studio, Reaper, Cubase)
     - `Synthortion_mac_universal_au.zip` (for Logic Pro, GarageBand, Ableton)
   - Extract the zip files to retrieve `Synthortion.vst3` and `Synthortion.component`.

2. **Copy to Plug-Ins Folders:**
   - **VST3:** Copy `Synthortion.vst3` to:
     ```text
     /Library/Audio/Plug-Ins/VST3/
     ```
     *(or `~/Library/Audio/Plug-Ins/VST3/` for current user only)*.
   - **Audio Unit (AU):** Copy `Synthortion.component` to:
     ```text
     /Library/Audio/Plug-Ins/Components/
     ```
     *(or `~/Library/Audio/Plug-Ins/Components/` for current user only)*.

3. **Bypass Apple Gatekeeper (Important):**
   Because pre-release test builds are not codesigned with an Apple Developer certificate, macOS will block the plugin by default (*"Cannot be opened because it is from an unidentified developer"*).
   Open **Terminal** and run the following commands to remove the quarantine flag:
   ```bash
   # For VST3:
   sudo xattr -cr /Library/Audio/Plug-Ins/VST3/Synthortion.vst3

   # For Audio Unit (AU):
   sudo xattr -cr /Library/Audio/Plug-Ins/Components/Synthortion.component
   ```

4. **Load in DAW:**
   - **Logic Pro:** Open **Logic Pro > Settings > Plug-in Manager**, locate *Synthortion*, and click **Reset & Rescan Selection**.
   - **Ableton / Reaper / FL Studio:** Open preferences, ensure VST3/AU plug-in sources are enabled, and run a **Rescan**.

---

## Building from Source

### Prerequisites
- **CMake 3.22+**
- **C++20 compliant compiler** (MSVC 2022 on Windows, Apple Clang on macOS, GCC 12+ / Clang 16+ on Linux)
- **Node.js 18+** and **npm** (to build the React UI bundle)
- **JUCE 8 submodule** initialized

### 1. Clone & Initialize Submodules

```bash
git clone https://github.com/paoloficaraa/Synthortion.git
cd Synthortion
git submodule update --init --recursive
```

### 2. Build the React Frontend

The UI assets must be compiled into `ui/dist` before building the C++ binaries:

```bash
cd ui
npm install
npm run build
cd ..
```

### 3. Build the Plugin (CMake)

**On Windows (Visual Studio / Ninja):**
```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```
*(The build automatically copies `Synthortion.vst3` to `C:\Program Files\Common Files\VST3\`)*

**On macOS (Universal Binary: Apple Silicon & Intel):**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release --parallel
```

---

## Running Tests

### C++ DSP Unit Tests

Run the standalone DSP sanity test suite:

```bash
ctest --test-dir build --output-on-failure
```

Or execute the test binary directly:

```bash
./build/plugin/SynthortionTests
```

### Frontend Unit & Integration Tests

Run the frontend Vitest suite (component tests, bridge mocks, and state diffing):

```bash
cd ui
npm run test:run
```

---

## Repository Layout

```
Synthortion/
├── CMakeLists.txt              # Root CMake project configuration
├── README.md                   # Plugin documentation
├── LICENSE                     # MIT License
├── docs/
│   └── adr/                    # Architecture Decision Records
│       ├── 0001-parameter-id-schema-and-bridge-protocol.md
│       ├── 0002-unified-native-event-bridge-protocol.md
│       └── 0003-state-serialization-and-ui-preferences-schema.md
├── libs/
│   └── juce/                   # JUCE 8 framework (submodule)
├── plugin/
│   ├── CMakeLists.txt          # Plugin target, bundle copies, & test config
│   ├── include/Synthortion/
│   │   ├── PluginProcessor.h   # Audio processor & APVTS parameter layout
│   │   ├── PluginEditor.h      # WebView wrapper & native event handlers
│   │   ├── DspModule.h         # C++20 DspModule concept definition
│   │   ├── WarmDistortion.h    # Dynamic bias saturation & oversampler
│   │   ├── Bitcrusher.h        # Lo-fi bit & sample-rate reduction
│   │   ├── PingPongDelay.h     # Host-synced stereo ping-pong delay
│   │   ├── SynthortionChorus.h # 3-voice vintage BBD chorus
│   │   ├── SpectrumAnalyzer.h  # 2048-pt FFT engine & log band pooling
│   │   └── AudioCaptureFifo.h  # Lock-free audio FIFO for telemetry
│   ├── src/
│   │   ├── PluginProcessor.cpp # DSP graph execution & state serialization
│   │   ├── PluginEditor.cpp    # IPC bridging, timers, & asset serving
│   │   ├── WarmDistortion.cpp
│   │   ├── BitCrusher.cpp
│   │   ├── PingPongDelay.cpp
│   │   ├── SynthortionChorus.cpp
│   │   └── SpectrumAnalyzer.cpp
│   └── tests/
│       └── Main.cpp            # JUCE unit test runner
└── ui/
    ├── package.json            # Frontend dependencies & scripts
    ├── vite.config.ts          # Vite build & bundle configuration
    ├── DESIGN.md               # Digital instrument & ASCII design system
    ├── index.html              # HTML shell for WebBrowserComponent
    └── src/
        ├── App.tsx             # Root React view & bridge hydration
        ├── components/         # Matrix faceplate, visualizer, knobs, meters
        ├── lib/                # Parameter store, DSP bridge, glitch pulser
        └── styles/             # Tailwind CSS & custom scanline/ASCII styles
```

---

## Architecture Decision Records (ADRs)

Key architectural decisions are documented in `docs/adr/`:

- **[ADR 0001](docs/adr/0001-parameter-id-schema-and-bridge-protocol.md):** Parameter ID Schema & Bridge Protocol — Establishes APVTS parameter IDs as the single source of truth and defines the normalization layer.
- **[ADR 0002](docs/adr/0002-unified-native-event-bridge-protocol.md):** Unified Native Event Bridge Protocol — Standardizes IPC on JUCE 8 `WebBrowserComponent` native events (`emitEventIfBrowserIsVisible`), replacing legacy string evaluation.
- **[ADR 0003](docs/adr/0003-state-serialization-and-ui-preferences-schema.md):** APVTS State Serialization & UI Preference ValueTree Schema — Defines synchronous XML binary state persistence and `<UIPreferences>` child subtree integration.

---

## License

Released under the [MIT License](LICENSE).

JUCE licensing terms are available in `libs/juce/LICENSE.md`.

---

## Author

**Paolo Ficara** — [@paoloficaraa](https://github.com/paoloficaraa)