# Coding Standards — Synthortion (JUCE 8 / C++20)

The reviewer agent enforces these during review (loaded via
`@.sandcastle/CODING_STANDARDS.md`). Concise by design — every rule is enforced.

## Real-Time DSP (audio thread / processBlock)

- **No dynamic allocation**: no `new`/`delete`/`malloc`/`free`, no `vector::push_back`/`resize`, no string building.
- **No blocking sync**: no `mutex`, `unique_lock`, `juce::ScopedLock` (priority inversion). Use lock-free primitives only.
- **No I/O**: no file/network access, no console/logging (`std::cout`, `DBG()`) in the audio thread.
- **No exceptions**: real-time code compiles with `-fno-exceptions`; no `throw`/`catch` in DSP modules.
- **No unbounded loops**: every loop has a fixed, known maximum iteration count.
- **RAII + pre-allocation**: size buffers and state in `prepareToPlay()`, never lazily in `processBlock()`.
- **Inter-thread comms**: `std::atomic` (`memory_order_relaxed` for independent scalars; `acquire`/`release` when publishing non-atomic data) or lock-free SPSC queues via `juce::AbstractFifo`. Never query APVTS/ValueTree from the audio thread.
- **Denormals**: `juce::ScopedNoDenormals` at the top of `processBlock()`.
- **Zipper noise**: smooth parameter changes with `juce::SmoothedValue` (ramp ≈ 10–20 ms) inside the audio thread.
- **Perf**: SoA layout, contiguous/aligned buffers, branchless inner sample loops (hoist `if`/`else` out of the loop).

## Parameters & State

- Centralize parameters in `juce::AudioProcessorValueTreeState` (APVTS).
- Parameter IDs as typed constants in a `ParameterIDs` namespace — no magic strings.
- Audio thread reads parameters via `getRawParameterValue()` pointers captured at construction, `load(std::memory_order_relaxed)`.
- Implement `getStateInformation`/`setStateInformation`; validate the XML tag before `replaceState()`.

## UI (Native + JUCE 8 WebView)

- `repaint()` only on the message thread. Audio→UI data flows through a lock-free FIFO polled by a `juce::Timer` (30–60 Hz).
- WebView (`juce::WebBrowserComponent`): native integration via `Options()` — `.withNativeIntegrationEnabled()`, `.withNativeFunction()`, `.withEventListener()`; serve assets via `.withResourceProvider()` (no local HTTP server, no filesystem access in production).
- **Destruction order (critical — use-after-free)**: declare `WebSliderRelay` members **first**, `WebSliderParameterAttachment` immediately after, and `WebBrowserComponent` **last** (C++ destroys members in reverse declaration order).
- High-frequency visual data (e.g. FFT at 60 fps): WebSocket server on loopback or binary `fetch()` via `ResourceProvider` — never JSON IPC per frame.
- Embed web assets in the binary (`juce_add_binary_data` + `ZipFile`/`MemoryInputStream`).

## Build & Tests

- CMake 3.22+, C++20, `juce_add_plugin` / `juce_add_console_app`.
- Flags: GCC/Clang `-fno-exceptions -O3 -ffast-math`; MSVC `/EHs-c- /O2 /fp:fast`.
- Verify: `cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build --target SynthortionTests && ./build/plugin/SynthortionTests`.
- New DSP/processor behaviour must have a JUCE unit test in `plugin/tests`.
