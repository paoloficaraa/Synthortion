# OpenCode Agent Instructions: Synthortion

JUCE 8+ C++20 audio plugin (VST3, AU). Processing chain: `Input -> Gain -> WarmDistortion -> BitCrusher -> ParametricEQ -> Chorus -> PingPongDelay -> OutputGain -> Output` with global `COLOR` macro blending dry/wet.

## Critical Setup

**Always** initialize JUCE submodule before building:
```bash
git submodule update --init --recursive
```

## Build

```bash
# Configure (Ninja recommended)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build
```

**Artifacts location:** `build/plugin/Synthortion_artefacts/<Config>/<FORMAT>/`  
(e.g., `build/plugin/Synthortion_artefacts/Debug/VST3/Synthortion.vst3`)

## Platform Quirks

- **Linux:** Build artifacts remain in `build/` directory (no auto-copy to system plugin folders)
- **macOS/Windows:** `COPY_PLUGIN_AFTER_BUILD TRUE` may copy plugins to local folders automatically
- **No standalone target:** Only `AU` and `VST3` formats configured

## Linux Dependencies

On Linux, install system packages for WebKit/GTK/CURL before configuring:
```bash
sudo apt-get install libwebkit2gtk-4.1-dev libgtk-3-dev libcurl4-openssl-dev
```
CMake uses `pkg-config` to find these.

## Architecture

- **DSP core:** `plugin/src/PluginProcessor.cpp` — `AudioPluginAudioProcessor` owns all effects
- **UI:** `plugin/src/PluginEditor.cpp` — fixed 720×490 window, `Timer` refresh at 60Hz
- **Headers:** `plugin/include/Synthortion/`
- **Effects:** `WarmDistortion`, `BitCrusher`, `ParametricEQ`, `PingPongDelay`, `SynthortionChorus`
- **JUCE modules:** `juce_audio_utils`, `juce_audio_processors`, `juce_gui_extra`, `juce_dsp`

## Testing & Quality

- No automated tests exist
- No `.clang-format` — follow existing code style in the codebase
- Manual verification requires a DAW/plugin host; spectrum analyzer runs at 4096 FFT size
