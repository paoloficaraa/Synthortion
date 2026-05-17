# OpenCode Agent Instructions: Synthortion

JUCE 8+ C++20 audio plugin (VST3, AU). Effects chain: Gain -> WarmDistortion -> BitCrusher -> ParametricEQ -> Chorus -> PingPongDelay -> OutputGain, with global `COLOR` macro blending dry/wet.

## Critical Setup

- **Always initialize JUCE submodule before building:**
  ```bash
  git submodule update --init --recursive
  ```

## Build Commands

```bash
# Configure (Ninja recommended)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build
```

**Artifacts:** `build/plugin/Synthortion_artefacts/<Config>/<FORMAT>/`  
(e.g., `build/plugin/Synthortion_artefacts/Debug/VST3/Synthortion.vst3`)

## Platform quirks

- **Linux:** Build artifacts stay in `build/` (no auto-copy to system folders)
- **macOS/Windows:** `COPY_PLUGIN_AFTER_BUILD TRUE` may copy plugins automatically
- **No standalone target:** Only AU and VST3 formats configured

## Linux dependencies

Install before configuring:
```bash
sudo apt-get install libwebkit2gtk-4.1-dev libgtk-3-dev libcurl4-openssl-dev
```
CMake uses `pkg-config`.

## Architecture (entrypoints)

- **DSP core:** `plugin/src/PluginProcessor.cpp` — `AudioPluginAudioProcessor` owns all effects
- **UI:** `plugin/src/PluginEditor.cpp` — fixed 720×490 window, 60Hz `Timer` refresh
- **Headers:** `plugin/include/Synthortion/`
- **Effects:** `WarmDistortion`, `BitCrusher`, `ParametricEQ`, `PingPongDelay`, `SynthortionChorus`
- **JUCE modules:** `juce_audio_utils`, `juce_audio_processors`, `juce_gui_extra`, `juce_dsp`

## Testing & quality

- **No automated tests exist** — manual verification only
- **No `.clang-format`** — follow existing code style in the codebase
- Manual verification requires a DAW/plugin host; spectrum analyzer runs at 4096 FFT size