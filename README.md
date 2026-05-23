# Synthortion

![Version](https://img.shields.io/badge/version-0.1.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![C++](https://img.shields.io/badge/C%2B%2B-20-orange.svg)
![JUCE](https://img.shields.io/badge/JUCE-8.0%2B-purple.svg)

Professional distortion and creative effects plugin built with JUCE and C++20.

Synthortion combines tape-inspired saturation, character effects, and a real-time spectrum analyzer, centered around a single macro control: `COLOR`.

---

## Features

### Warm Distortion Core
- Tape-style nonlinear saturation with asymmetric behavior
- 8x oversampling (`2^3`) in the distortion stage to reduce aliasing
- Drive-dependent filtering and high-frequency excitation
- Analog-style noise modeling and wow/flutter modulation at higher drive
- Optional automatic volume compensation

### Creative FX Section
- `BitCrusher` — sample-rate reduction, quantization, dither, and DAC noise
- Stereo `PingPongDelay` — damped feedback path with smoothed parameters
- `SynthortionChorus` — dry/wet mixing
- Global `COLOR` macro — scales effect intensity and adds extra distortion drive

### Analysis & Metering
- Real-time FFT analyzer (4096 samples)
- Peak-hold spectrum behavior
- Input/Output discrete RMS meters

### Processing Chain

```
Input → Input Gain → WarmDistortion → BitCrusher → Chorus → PingPongDelay → Output Gain → Output
```

A global dry/wet stage (driven by `COLOR`) blends the latency-matched dry signal with the processed chain.

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

> **Note:** The plugin currently builds as `AU` and `VST3` (no standalone target). Reported latency depends on oversampling/filter behavior and host compensation.

---

## Parameters

### Main
| Parameter | Range | Description |
|---|---|---|
| `COLOR` | 0..1 | Global macro; increases wet blend and adds extra distortion drive |
| `INPUT_GAIN` | -24..+24 dB | Input level |
| `OUTPUT_GAIN` | -24..+24 dB | Output level |

### Distortion
| Parameter | Range | Description |
|---|---|---|
| `DRIVE` | 0..1 | Distortion intensity (not exposed as front-panel knob) |
| `VOLUME_COMPENSATION` | bool | Automatic output level compensation |

### FX
| Parameter | Range | Description |
|---|---|---|
| `BITCRUSH` | 0..1 | Bit crushing intensity |
| `DAC_NOISE` | 0..1 | DAC noise floor |
| `DELAY_TIME` | 1..2000 ms | Delay time |
| `DELAY_MIX` | 0..1 | Delay wet/dry mix |
| `DELAY_FEEDBACK` | 0..0.95 | Delay feedback |
| `CHORUS_MIX` | 0..1 | Chorus wet/dry mix |

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

For a debug build:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

---

## Repository Layout

```
Synthortion/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── libs/
│   └── juce/          # JUCE framework (submodule)
├── modules/
│   └── gin/           # Gin DSP library (submodule)
└── plugin/
    ├── CMakeLists.txt
    ├── include/Synthortion/
    │   ├── PluginProcessor.h
    │   ├── PluginEditor.h
    │   ├── WarmDistortion.h
    │   ├── Bitcrusher.h
    │   ├── PingPongDelay.h
    │   ├── SynthortionChorus.h
    │   └── SynthortionLookAndFeel.h
    └── src/
        ├── PluginProcessor.cpp
        ├── PluginEditor.cpp
        ├── WarmDistortion.cpp
        ├── BitCrusher.cpp
        ├── PingPongDelay.cpp
        ├── SynthortionChorus.cpp
        └── SynthortionLookAndFeel.cpp
```

---

## Current Status

- Preset browser/management: not implemented yet
- Standalone app target: not enabled in current CMake configuration
- Main UI: fixed-size (non-resizable)

---

## License

MIT. See `LICENSE`.

JUCE licensing details are available in `libs/juce/LICENSE.md`.

## Author

Paolo Ficara — [@paoloficaraa](https://github.com/paoloficaraa)