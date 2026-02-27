# Synthortion

![Version](https://img.shields.io/badge/version-0.1.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![C++](https://img.shields.io/badge/C%2B%2B-20-orange.svg)
![JUCE](https://img.shields.io/badge/JUCE-8.0%2B-purple.svg)

Professional distortion and creative effects plugin built with JUCE and C++20.

Synthortion combines tape-inspired saturation, character effects, a 4-band EQ, and a real-time spectrum analyzer in a focused workflow centered around a single macro control: `COLOR`.

---

## Features

### Warm Distortion Core
- Tape-style nonlinear saturation with asymmetric behavior
- 8x oversampling (`2^3`) in the distortion stage to reduce aliasing
- Drive-dependent filtering and high-frequency excitation
- Subtle analog-style noise modeling and wow/flutter modulation at higher drive
- Optional automatic volume compensation

### Creative FX Section
- `BitCrusher` with sample-rate reduction, quantization, dither, and DAC noise
- Stereo `PingPongDelay` with damped feedback path and smoothed parameters
- `SynthortionChorus` with dry/wet mixing
- Global `COLOR` macro that scales effect intensity and adds extra distortion drive

### 4-Band Parametric EQ
- Low Cut (HP), Low Mid bell, High Mid bell, High Cut (LP)
- Wide `Q` range and ±15 dB for both mid bands
- `EQ BYPASS` toggle for A/B
- Live EQ curve overlay in the analyzer

### Analysis & Metering
- Real-time FFT analyzer (`fftOrder = 12`, 4096 samples)
- Peak-hold spectrum behavior
- Input/Output discrete RMS meters

---

## Technical Snapshot

| Item | Value |
|---|---|
| Language | C++20 |
| Framework | JUCE 8+ |
| Build System | CMake 3.22+ |
| Plugin Formats | VST3, AU |
| Audio Path | Stereo (mono/stereo buses supported) |
| Distortion Oversampling | 8x |
| FFT Size | 4096 |

Notes:
- The plugin currently builds as `AU` and `VST3` (no standalone target in current CMake configuration).
- Reported latency depends on oversampling/filter behavior and host compensation.

---

## Processing Chain

`Input -> Input Gain -> WarmDistortion -> BitCrusher -> ParametricEQ -> Chorus -> PingPongDelay -> Output Gain -> Output`

A global dry/wet stage (driven by `COLOR`) blends latency-matched dry signal with the processed chain.

---

## Parameters

### Main
- `COLOR` (`0..1`): global macro; increases wet blend and adds extra distortion drive
- `INPUT_GAIN` (`-24..+24 dB`)
- `OUTPUT_GAIN` (`-24..+24 dB`)

### Distortion
- `DRIVE` (`0..1`) *(engine parameter; not currently exposed as a dedicated front-panel knob)*
- `VOLUME_COMPENSATION` (bool)

### FX
- `BITCRUSH` (`0..1`)
- `DAC_NOISE` (`0..1`)
- `DELAY_TIME` (`1..2000 ms`)
- `DELAY_MIX` (`0..1`)
- `DELAY_FEEDBACK` (`0..0.95`)
- `CHORUS_MIX` (`0..1`)

### EQ
- `LOW_CUT_FREQ` (`0..1000 Hz`, `0` disables)
- `LOW_CUT_Q` (`0.025..40`)
- `LOW_MID_FREQ` (`100..2000 Hz`)
- `LOW_MID_GAIN` (`-15..+15 dB`)
- `LOW_MID_Q` (`0.025..40`)
- `HIGH_MID_FREQ` (`1000..8000 Hz`)
- `HIGH_MID_GAIN` (`-15..+15 dB`)
- `HIGH_MID_Q` (`0.025..40`)
- `HIGH_CUT_FREQ` (`5000..20000 Hz`, `20000` disables)
- `HIGH_CUT_Q` (`0.025..40`)
- `EQ_BYPASS` (bool)

---

## Build

### Prerequisites
- CMake 3.22+
- C++20 compiler
- JUCE submodule initialized

### Clone + Build

```bash
git clone https://github.com/paoloficaraa/Synthortion.git
cd Synthortion
git submodule update --init --recursive

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Debug build:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

---

## Repository Layout

```text
Synthortion/
├── CMakeLists.txt
├── README.md
├── libs/
│   └── juce/
└── plugin/
    ├── CMakeLists.txt
    ├── include/Synthortion/
    │   ├── PluginProcessor.h
    │   ├── PluginEditor.h
    │   ├── WarmDistortion.h
    │   ├── Bitcrusher.h
    │   ├── ParametricEQ.h
    │   ├── PingPongDelay.h
    │   ├── SynthortionChorus.h
    │   ├── SpectrumAnalyzer.h
    │   ├── VerticalDiscreteMeter.h
    │   └── SynthortionLookAndFeel.h
    └── src/
        ├── PluginProcessor.cpp
        ├── PluginEditor.cpp
        ├── WarmDistortion.cpp
        ├── BitCrusher.cpp
        ├── ParametricEQ.cpp
        ├── PingPongDelay.cpp
        ├── SynthortionChorus.cpp
        ├── SpectrumAnalyzer.cpp
        ├── VerticalDiscreteMeter.cpp
        └── SynthortionLookAndFeel.cpp
```

---

## Current Status

- Preset browser/management: not implemented yet
- Standalone app target: not enabled in current `juce_add_plugin` formats
- Main UI is fixed-size (non-resizable)

---

## License

MIT. See `LICENSE`.

JUCE licensing details are available in `libs/juce/LICENSE.md`.

---

## Author

Paolo Ficara  
GitHub: [@paoloficaraa](https://github.com/paoloficaraa)
