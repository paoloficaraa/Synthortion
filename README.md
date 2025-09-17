# Synthortion

🎛️ **A creative distortion plugin for music production**

Synthortion is a professional-grade audio plugin that provides warm, musical distortion with advanced features for creative sound design and mixing.

## ✨ Features

### 🔥 **Warm Distortion Engine**

- **Multiple Saturation Types:**
  - **Smooth**: Classic tanh-based saturation for warm, musical distortion
  - **Tube**: Asymmetric tube-style saturation with authentic valve characteristics
  - **Tape**: Soft-knee tape saturation with natural compression
- **8x Oversampling**: Eliminates aliasing artifacts for pristine audio quality
- **Subtle Bit Crushing**: Adds digital character with 14-bit quantization
- **Dry/Wet Mix Control**: Blend processed and original signal

### 🎚️ **4-Band Parametric EQ**

- **Low Cut**: High-pass filter (20Hz - 1kHz) with adjustable Q
- **Low Mid**: Parametric band (100Hz - 2kHz) with ±15dB gain and adjustable Q
- **High Mid**: Parametric band (1kHz - 8kHz) with ±15dB gain and adjustable Q
- **High Cut**: Low-pass filter (5kHz - 20kHz) with adjustable Q

### 📊 **Real-Time Analysis**

- **Spectrum Analyzer**: Visual frequency response display
- **Input/Output Meters**: Discrete LED-style RMS level monitoring
- **Live Parameter Feedback**: Immediate visual response to changes

### 🎨 **Professional Interface**

- **Custom Alkane-Inspired Design**: Dark theme with purple accents
- **High-DPI Support**: Crisp visuals on all screen resolutions
- **Intuitive Layout**: Logical control grouping for efficient workflow

## 🛠️ **Technical Specifications**

- **Sample Rates**: 44.1kHz - 192kHz+
- **Bit Depth**: 32-bit floating point processing
- **Latency**: ~5ms (due to oversampling)
- **Formats**: VST3, AU
- **Platforms**: Windows, macOS

## 🚀 **Installation**

### Prerequisites

- **JUCE Framework 8.0+**
- **CMake 3.22+**
- **C++20 Compatible Compiler**
  - Visual Studio 2019+ (Windows)
  - Xcode 12+ (macOS)
  - GCC 10+ or Clang 12+ (Linux)

### Building from Source

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

### Build Configurations

- **Debug**: `cmake -B build -DCMAKE_BUILD_TYPE=Debug`
- **Release**: `cmake -B build -DCMAKE_BUILD_TYPE=Release`

## 🎵 **Usage**

### Basic Operation

1. **Load** Synthortion in your DAW as a VST3/AU plugin
2. **Set Drive** amount for desired saturation intensity
3. **Choose Saturation Type** (Smooth/Tube/Tape) for different characters
4. **Adjust EQ** bands to shape the frequency response
5. **Use Dry/Wet Mix** to blend the effect with the original signal

### Pro Tips

- **Start with low drive settings** and gradually increase for musical results
- **Use the spectrum analyzer** to visualize frequency changes in real-time
- **Tape mode** works great on drums for vintage character
- **Tube mode** excels on bass and lead sounds
- **EQ before distortion** for different tonal shaping

## 🏗️ **Architecture**

### Audio Processing Chain

```
Input → EQ → Oversampling ↑ → Saturation → Bit Crush → Oversampling ↓ → Dry/Wet Mix → Output
                                                      ↓
                                               Spectrum Analysis
```

### Key Components

- **`WarmDistortion`**: Core distortion processor with oversampling
- **`ParametricEQ`**: 4-band IIR filter implementation
- **`SpectrumAnalyzer`**: Real-time FFT analysis
- **`SynthortionLookAndFeel`**: Custom UI theme
- **`VerticalDiscreteMeter`**: LED-style level meters

## 🧪 **Development**

### Project Structure

```
Synthortion/
├── CMakeLists.txt              # Main CMake configuration
├── libs/juce/                  # JUCE framework (submodule)
└── plugin/
    ├── CMakeLists.txt          # Plugin CMake config
    ├── include/Synthortion/    # Public headers
    │   ├── PluginProcessor.h
    │   ├── PluginEditor.h
    │   ├── WarmDistortion.h
    │   ├── ParametricEQ.h
    │   ├── SpectrumAnalyzer.h
    |   ├── VerticalDiscreteMeter.h
    |   └── SynthortionLookAndFeel.h
    └── src/                    # Implementation files
        ├── PluginProcessor.cpp
        ├── PluginEditor.cpp
        ├── WarmDistortion.cpp
        ├── ParametricEQ.cpp
        ├── SpectrumAnalyzer.cpp
        └── SynthortionLookAndFeel.cpp
```

### Code Style

- **C++20 features** where beneficial
- **JUCE conventions** for audio processing
- **RAII** for resource management
- **Const correctness** throughout
- **Comprehensive documentation** with Doxygen-style comments

## 📝 **License**

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

### Development Workflow

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 🐛 **Bug Reports**

Found a bug? Please open an issue with:

- **OS and version**
- **DAW and version**
- **Plugin format** (VST3/AU)
- **Steps to reproduce**
- **Expected vs actual behavior**

## 💫 **Acknowledgments**

- **JUCE Framework** - Cross-platform audio framework
- **Raw Material Software** - JUCE development team
- **Audio programming community** - Inspiration and knowledge sharing

## 📧 **Contact**

**Paolo Ficara** - [@paoloficaraa](https://github.com/paoloficaraa)
