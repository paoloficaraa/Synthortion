# OpenCode Agent Instructions: Synthortion

This repository contains a JUCE 8+ C++20 audio plugin (Synthortion). 

## Architecture & Layout
- **Headers:** `plugin/include/Synthortion/`
- **Sources:** `plugin/src/`
- **Entrypoints:** `PluginProcessor.cpp` (audio logic/DSP) and `PluginEditor.cpp` (UI/GUI).
- **Submodules:** JUCE is included as a git submodule in `libs/juce`. If JUCE headers are missing, verify the submodule is initialized.

## Build Setup (CMake)
The project uses CMake (minimum 3.22) and requires C++20. 

### First-time Setup
Always initialize JUCE submodule before configuring:
```bash
git submodule update --init --recursive
```

### Local Building
```bash
# Configure (Ninja recommended for speed)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build
```
*Note: The CI workflow omits `-G Ninja`; both approaches work. For faster local builds, install Ninja.*

### Artifact Locations
After building, the plugin binaries (VST3, AU) will be located in:
`build/plugin/Synthortion_artefacts/<Config>/` (e.g., `build/plugin/Synthortion_artefacts/Debug/VST3/Synthortion.vst3`).

## Project Quirks & Conventions
- **No Standalone Target:** The `CMakeLists.txt` only configures `VST3` and `AU` formats (`FORMATS AU VST3`). There is currently no standalone app target.
- **Copy After Build:** `COPY_PLUGIN_AFTER_BUILD TRUE` is set. On macOS and Windows, CMake will try to automatically copy the compiled plugin to the user's local plugin folders (e.g., `~/Library/Audio/Plug-Ins/VST3/` or `%LOCALAPPDATA%\Programs\Common\VST3\`). On Linux, artifacts remain in `build/plugin/Synthortion_artefacts/<Config>/` and are not copied automatically.
- **Tests/Linting:** There are currently no automated unit tests (`tests` directory) or custom formatters (`.clang-format`) in the repository, rely on JUCE conventions and manual verification via a plugin host.
- **JUCE Modules:** Uses `juce_audio_utils`, `juce_audio_processors`, `juce_gui_extra`, and `juce_dsp`. These are linked via the submodule at `libs/juce`.
