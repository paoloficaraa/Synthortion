# Synthortion

![Version](https://img.shields.io/badge/version-0.1.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![C++](https://img.shields.io/badge/C%2B%2B-20-orange.svg)
![JUCE](https://img.shields.io/badge/JUCE-8.0%2B-purple.svg)

🎛️ **A professional creative distortion and effects plugin for music production**

Synthortion is a high-quality audio plugin combining vintage-inspired tape saturation, modern effects processing, and a powerful parametric EQ—all wrapped in an intuitive, professional interface with real-time spectrum analysis.

---

## ✨ Features

### 🔥 Warm Distortion Engine
- **Tape Saturation**: Authentic soft-knee tape compression and saturation
- **8x Oversampling**: Eliminates aliasing artifacts for pristine audio quality
- **Drive-Dependent Filtering**: Simulates analog component saturation
- **High-Frequency Exciter**: Adds clarity and presence to your sound
- **Automatic Volume Compensation**: Maintains perceived loudness across drive settings

### 🎨 Creative Effects Chain
- **BitCrusher**: Vintage digital character with controllable bit depth reduction and DAC noise
- **Ping-Pong Delay**: Stereo delay with damping filters and smooth parameter modulation
- **Chorus**: Rich modulation for width and movement
- **Color Control**: Master effects intensity knob for quick workflow

### 🎚️ 4-Band Parametric EQ
- **Low Cut**: High-pass filter (20Hz - 1kHz) with adjustable Q (0.025 - 40.0)
- **Low Mid**: Parametric band (100Hz - 2kHz) with ±15dB gain
- **High Mid**: Parametric band (1kHz - 8kHz) with ±15dB gain
- **High Cut**: Low-pass filter (5kHz - 20kHz) with adjustable Q
- **EQ Bypass**: Quick A/B comparison
- **Live Curve Visualization**: See your EQ settings on the spectrum analyzer

### 📊 Real-Time Analysis
- **Spectrum Analyzer**: High-resolution FFT analysis (4096 samples)
- **Peak Hold Display**: Professional-style peak tracking
- **EQ Curve Overlay**: Visual feedback of your EQ settings
- **Input/Output Meters**: Discrete LED-style RMS level monitoring

### 🎨 Professional Interface
- **Custom Dark Theme**: Elegant design optimized for long sessions
- **High-DPI Support**: Crisp visuals on all screen resolutions
- **Intuitive Layout**: Logical control grouping for efficient workflow
- **Real-Time Parameter Display**: Visual feedback for all controls

---

## 🛠️ Technical Specifications

| Feature | Specification |
|---------|---------------|
| **Sample Rates** | 44.1kHz - 192kHz+ (host-dependent) |
| **Bit Depth** | 32-bit floating point processing |
| **Oversampling** | 8x (2^3) for distortion stage |
| **FFT Resolution** | 4096 samples (fftOrder = 12) |
| **Latency** | ~5-10ms (due to oversampling filters) |
| **Formats** | VST3, AU, Standalone |
| **Platforms** | Windows, macOS |

---

## 🚀 Installation

### Option 1: Download Pre-built Binaries (Coming Soon)
1. Download the latest release from [Releases](https://github.com/paoloficaraa/Synthortion/releases)
2. Extract the archive
3. Copy the plugin to your plugin folder:
   - **Windows VST3**: `C:\Program Files\Common Files\VST3\`
   - **macOS VST3**: `~/Library/Audio/Plug-Ins/VST3/`
   - **macOS AU**: `~/Library/Audio/Plug-Ins/Components/`

### Option 2: Build from Source

#### Prerequisites
- **JUCE Framework 8.0+** (included as submodule)
- **CMake 3.22+**
- **C++20 Compatible Compiler**:
  - Visual Studio 2019+ (Windows)
  - Xcode 12+ (macOS)
  - GCC 10+ or Clang 12+ (Linux)

#### Build Steps

1. **Clone the repository:**
   ```bash
   git clone https://github.com/paoloficaraa/Synthortion.git
   cd Synthortion
   ```

2. **Initialize submodules:**
   ```bash
   git submodule update --init --recursive
   ```

3. **Generate build files:**
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   ```

4. **Build the plugin:**
   ```bash
   cmake --build build --config Release
   ```

5. **Install (optional):**
   ```bash
   cmake --install build
   ```

#### Build Configurations
- **Debug**: `cmake -B build -DCMAKE_BUILD_TYPE=Debug`
- **Release**: `cmake -B build -DCMAKE_BUILD_TYPE=Release`

---

## 🎵 Usage Guide

### Quick Start
1. **Load Synthortion** in your DAW as a VST3/AU plugin
2. **Adjust Input Gain** to drive the signal into the saturation stage
3. **Set COLOR knob** to control overall effects intensity
4. **Fine-tune individual effects** (BitCrush, Delay, Chorus)
5. **Shape tone with EQ** before or after distortion
6. **Monitor levels** with input/output meters
7. **Adjust Output Gain** for final level matching

### Parameter Overview

#### Main Controls
- **COLOR (0-100%)**: Master effects intensity control
  - Controls the overall wetness of BitCrush, Delay, and Chorus
  - Adds up to 30% extra drive to the distortion stage
- **Input Gain (-24dB to +24dB)**: Pre-distortion level control
- **Output Gain (-24dB to +24dB)**: Post-processing level control

#### Distortion Section
- **Drive (0-100%)**: Tape saturation intensity
- **Volume Compensation**: Automatic loudness compensation (toggle)

#### Effects Section
- **BitCrush Mix (0-100%)**: Digital degradation amount
- **DAC Noise (0-100%)**: Simulates digital-to-analog conversion artifacts
- **Delay Time (1-2000ms)**: Ping-pong delay time
- **Delay Mix (0-100%)**: Delay effect amount (scaled by COLOR)
- **Delay Feedback (0-95%)**: Delay regeneration amount
- **Chorus Mix (0-100%)**: Chorus effect amount (scaled by COLOR)

#### EQ Section
- **Low Cut Freq (20Hz-1kHz)**: High-pass filter frequency (0 = disabled)
- **Low Cut Q (0.025-40.0)**: Filter resonance
- **Low Mid Freq (100Hz-2kHz)**: Low-mid band center frequency
- **Low Mid Gain (±15dB)**: Low-mid boost/cut
- **Low Mid Q (0.025-40.0)**: Low-mid bandwidth
- **High Mid Freq (1kHz-8kHz)**: High-mid band center frequency
- **High Mid Gain (±15dB)**: High-mid boost/cut
- **High Mid Q (0.025-40.0)**: High-mid bandwidth
- **High Cut Freq (5kHz-20kHz)**: Low-pass filter frequency (20kHz = disabled)
- **High Cut Q (0.025-40.0)**: Filter resonance
- **EQ Bypass**: Disable entire EQ section

### Pro Tips
- **Start with COLOR at 0%** and adjust individual effects first for precise control
- **Use COLOR for quick intensity changes** during mixing
- **Low drive + high input gain** = subtle warmth
- **High drive + low input gain** = aggressive saturation
- **BitCrush at low levels (10-20%)** adds subtle character
- **Ping-pong delay** with short times (50-150ms) creates stereo width
- **Use spectrum analyzer** to visualize frequency changes in real-time
- **EQ before distortion** changes the saturation character
- **Volume compensation ON** maintains consistent levels while dialing drive

---

## 🏗️ Architecture

### Audio Processing Chain
```
Input → Input Gain → Warm Distortion → BitCrusher → Parametric EQ → Chorus → Ping-Pong Delay → Output Gain → Output
                           ↓                                           ↓
                     (8x Oversampling)                         (Spectrum Analysis)
```

### Key Components

| Component | Description |
|-----------|-------------|
| `WarmDistortion` | Tape saturation with oversampling, drive-dependent filtering, and high-frequency exciter |
| `BitCrusher` | Digital degradation with sample rate reduction, bit depth reduction, and DAC noise |
| `ParametricEQ` | 4-band IIR filter with frequency response visualization |
| `PingPongDelay` | Stereo delay with damping filters and smooth parameter modulation |
| `SynthortionChorus` | Modulation effect with dry/wet mixing |
| `SpectrumAnalyzer` | Real-time FFT analysis with peak hold and EQ curve overlay |
| `VerticalDiscreteMeter` | LED-style RMS level meters |
| `SynthortionLookAndFeel` | Custom UI theme |

### Project Structure
```
Synthortion/
├── CMakeLists.txt              # Main CMake configuration
├── LICENSE                     # MIT License
├── README.md                   # This file
├── libs/
│   └── juce/                   # JUCE framework (submodule)
└── plugin/
    ├── CMakeLists.txt          # Plugin CMake config
    ├── include/Synthortion/    # Public headers
    │   ├── PluginProcessor.h
    │   ├── PluginEditor.h
    │   ├── WarmDistortion.h
    │   ├── BitCrusher.h
    │   ├── ParametricEQ.h
    │   ├── PingPongDelay.h
    │   ├── SynthortionChorus.h
    │   ├── SpectrumAnalyzer.h
    │   ├── VerticalDiscreteMeter.h
    │   └── SynthortionLookAndFeel.h
    └── src/                    # Implementation files
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

## 🧪 Development

### Code Style Guidelines
- **C++20 features** where beneficial (constexpr, concepts, ranges)
- **JUCE conventions** for audio processing
- **RAII** for resource management
- **Const correctness** throughout
- **Comprehensive documentation** with Doxygen-style comments
- **Meaningful variable names** over abbreviations

### Development Workflow
1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes and test thoroughly
4. Commit with descriptive messages: `git commit -m 'Add amazing feature'`
5. Push to your fork: `git push origin feature/amazing-feature`
6. Open a Pull Request with detailed description

### Building for Development
```bash
# Debug build with assertions and logging
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# Run in standalone mode for testing
./build/plugin/Synthortion_artefacts/Debug/Standalone/Synthortion
```

### Testing Checklist
- [ ] Test at multiple sample rates (44.1kHz, 48kHz, 96kHz, 192kHz)
- [ ] Verify no audio glitches or clicks
- [ ] Check CPU usage is reasonable
- [ ] Test parameter automation in DAW
- [ ] Verify preset save/load functionality
- [ ] Test on both Windows and macOS (if possible)
- [ ] Check for memory leaks

---

## 🐛 Bug Reports & Feature Requests

Found a bug or have an idea? Please [open an issue](https://github.com/paoloficaraa/Synthortion/issues) with:

### For Bugs
- **OS and version** (e.g., Windows 11, macOS 14.2)
- **DAW and version** (e.g., Ableton Live 12, Logic Pro 11)
- **Plugin format** (VST3, AU, Standalone)
- **Sample rate** being used
- **Steps to reproduce** the issue
- **Expected vs actual behavior**
- **Screenshots or audio examples** if applicable

### For Feature Requests
- **Clear description** of the feature
- **Use case** - why would this be useful?
- **Proposed implementation** (if you have ideas)

---

## 📋 Roadmap

### Version 0.2.0 (Planned)
- [ ] Preset management system
- [ ] Additional saturation modes (Tube, Smooth)
- [ ] Oversampling quality options (2x, 4x, 8x, 16x)
- [ ] MIDI learn for parameters
- [ ] Undo/redo support

### Version 0.3.0 (Future)
- [ ] Mid/Side processing mode
- [ ] Multiband distortion
- [ ] Additional modulation options
- [ ] Resizable UI
- [ ] Accessibility improvements

---

## 📝 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for full details.

### Third-Party Licenses
- **JUCE Framework**: ISC License / GPL v3 (depending on usage)
  - Used under ISC License for this open-source project
  - See `libs/juce/LICENSE.md` for details

---

## 💫 Acknowledgments

- **[JUCE Framework](https://juce.com/)** by Raw Material Software - Exceptional cross-platform audio framework
- **Audio DSP Community** - For sharing knowledge and best practices
- **Open Source Contributors** - Thank you for your contributions!

### Special Thanks
- The JUCE community for excellent documentation and examples
- Audio plugin developers who inspire with their work
- Beta testers providing valuable feedback

---

## 📧 Contact & Support

**Paolo Ficara**
- GitHub: [@paoloficaraa](https://github.com/paoloficaraa)
- Project: [Synthortion](https://github.com/paoloficaraa/Synthortion)

### Support
- 📖 Check the [Issues](https://github.com/paoloficaraa/Synthortion/issues) page for known problems
- 💬 Open a [Discussion](https://github.com/paoloficaraa/Synthortion/discussions) for questions
- 🐛 Report bugs via [Issue Tracker](https://github.com/paoloficaraa/Synthortion/issues/new)

---

## 🌟 Show Your Support

If you find Synthortion useful, please consider:
- ⭐ Starring the repository
- 🍴 Forking and contributing
- 📢 Sharing with fellow producers
- 💬 Providing feedback

---

<p align="center">Made with ❤️ and 🎛️ by Paolo Ficara</p>
<p align="center">© 2025 Paolo Ficara - MIT License</p>
