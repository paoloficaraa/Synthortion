# Coding & Architecture Standards: VST3 / AU (JUCE 8 / C++20)

## 1. C++ Style & Code Formatting
- **Naming**: `camelCase` for variables and functions; `PascalCase` for classes, structs, types, and namespaces.
- **Braces & Indentation**: Allman braces (braces on their own line, aligned with control statement). 4 spaces per indentation level, no tabs.
- **Spacing**: Single space around binary operators. `!` followed by space (`! condition`). No spaces inside pointer/reference qualifiers on types (`AudioBuffer* buffer`), space after pointer/ref symbol. Do not declare multiple pointers on the same line.
- **Templates**: Use descriptive template parameter names (e.g., `SampleType`), never single letters like `T`.
- **Pointers & Casts**: Use `nullptr` exclusively (never `NULL` or `0`). Use C++-style casts (`static_cast`, `reinterpret_cast`) only; C-style casts are strictly forbidden. Use braced initialization `{}` for primitives.
- **Specifiers**: Mark non-throwing functions with `noexcept`. Use `override` on virtual function overrides and omit the `virtual` keyword.
- **Macro Guard**: Use `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR` in classes to prevent copy construction and track memory leaks.

## 2. Control Flow & Memory Management
- **Control Flow**:
  - Omit `else` after a `return` statement.
  - Avoid nested ternary operators; use `switch` or `if-else` blocks instead.
  - Single-line conditionals may omit braces if completely unambiguous.
  - Restrict pointer scope by initializing pointers directly within conditional statements where possible.
- **Memory & RAII**:
  - Enforce strict RAII (`std::unique_ptr`, `std::shared_ptr` where ownership is shared). Raw `new`/`delete` are strictly prohibited.
  - No global statics or singletons within plugin instances.
- **Strings (JUCE 8 / C++20)**:
  - Do not use the deprecated `T()` macro.
  - Use `juce::String` and `juce::StringRef` for string handling.
  - Pre-C++20: `CharPointer_UTF8` escapes. C++20+: `u8` prefix for UTF-8 literals.

## 3. Real-Time Audio Thread Constraints (`processBlock`)
The audio thread operates under strict hard real-time deadlines (e.g., ~2.9ms for 128 samples @ 44.1kHz). **Execution must be 100% deterministic.**
- **Strictly Prohibited Operations in `processBlock`**:
  1. **Heap Allocation/Deallocation**: No `new`, `delete`, `malloc`, `free`, `std::vector::push_back`, or `std::vector::resize`.
  2. **Blocking Synchronization**: No `std::mutex`, `std::unique_lock`, or `juce::ScopedLock` (causes priority inversion).
  3. **File System & Network I/O**: No reading/writing files or socket operations.
  4. **Console I/O & Logging**: No `std::cout`, `printf`, or JUCE `DBG()` macros.
  5. **C++ Exception Handling**: Compile with exceptions disabled (`-fno-exceptions`). No `try`/`catch`/`throw`.
  6. **Unbounded Loops**: All loops must have a fixed, pre-calculated upper bound.
- **Resource Pre-allocation**:
  - All buffers, work vectors, and DSP engines must be allocated/prepared during `prepareToPlay(sampleRate, samplesPerBlock)`.
- **Diagnostic Safety**:
  - Use `AudioThreadGuard` (enabled via `JUCE_ENABLE_AUDIO_GUARD` in debug builds) to trap real-time safety violations (e.g., unintended heap allocs or `ValueTree` access).

## 4. Inter-Thread Communication & Lock-Free Data Transfer
- **Scalar Parameters**:
  - Use `std::atomic<T>` for individual values (gain, frequency, bypass).
  - Use `std::memory_order_relaxed` for independent parameter reads in `processBlock` to avoid hardware fence overhead on ARM64/x86.
  - Use `std::memory_order_release` / `std::memory_order_acquire` when publishing/reading dependent non-atomic data.
- **Complex Data Structures (MIDI, Waveforms, FFT Analysis)**:
  - Use Single-Producer Single-Consumer (SPSC) lock-free ring buffers using `juce::AbstractFifo`.
  - Audio thread acts as Producer (`push`), Message thread acts as Consumer (`pop`). Never block or allocate if the queue is full; drop frames safely.

```cpp
class AudioToUIMessageQueue {
public:
    AudioToUIMessageQueue() : fifo(1024) { ringBuffer.resize(1024); }

    bool push(const AudioFrameData& frame) noexcept { // Audio Thread (Producer)
        int s1, n1, s2, n2;
        fifo.prepareToWrite(1, s1, n1, s2, n2);
        if (n1 > 0) {
            ringBuffer[static_cast<size_t>(s1)] = frame;
            fifo.finishedWrite(n1);
            return true;
        }
        return false; // Queue full - drop frame, do not block
    }

    bool pop(AudioFrameData& frame) noexcept { // Message Thread (Consumer)
        int s1, n1, s2, n2;
        fifo.prepareToRead(1, s1, n1, s2, n2);
        if (n1 > 0) {
            frame = ringBuffer[static_cast<size_t>(s1)];
            fifo.finishedRead(n1);
            return true;
        }
        return false;
    }
private:
    juce::AbstractFifo fifo;
    std::vector<AudioFrameData> ringBuffer;
};
```

## 5. DSP Optimization & Pipeline Standards
- **Denormal / Subnormal Floats**:
  - Decay in IIR filters/delays causes subnormal floats, triggering CPU microcode exceptions (10x-100x CPU spikes).
  - **Mandatory**: Instantiate `juce::ScopedNoDenormals noDenormals;` at the top of `processBlock` to enforce FTZ (Flush-To-Zero) and DAZ (Denormals-Are-Zero).
- **Parameter Smoothing (Zipper Noise Prevention)**:
  - Smooth rapid parameter changes using `juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>` (or Exponential) with a 5–50ms ramp duration (default: 20ms).
- **Memory Layout & SIMD Vectorization**:
  - Prefer Structure of Arrays (SoA) over Array of Structures (AoS) for contiguous multi-channel buffer storage.
  - Align audio buffers to 32/64-byte boundaries for AVX-2 / AVX-512 / ARM Neon SIMD vectorization.
  - Hoist conditional branches (`if`/`else`) outside inner sample processing loops.
- **DSP Processing Pipeline Rules**:
  - **Oversampling**: Oversample non-linear stages (saturators, soft clippers) using `juce::dsp::Oversampling` (FIR half-band for linear phase or IIR half-band for low latency).
  - **Latency Reporting**: Report processing delays to the host DAW via `setLatencySamples()`.
  - **DC Blocker**: Place a DC blocker post-downsampling: y[n] = x[n] - x[n-1] + R * y[n-1] (R in 0.99..0.999).
  - **Gain Staging**: End DSP chain with `juce::dsp::Gain` for makeup gain.
  - **Processor Chains**: Compose modular DSP units using `juce::dsp::ProcessorChain`.

## 6. Parameter Management (`APVTS`) & State Serialization
- **Centralized Management**: Use `juce::AudioProcessorValueTreeState` (APVTS).
- **Parameter Identifiers**:
  - Define all parameter IDs in a dedicated namespace (`ParameterIDs`) as `inline constexpr const char*` to avoid magic strings.
- **Initialization**:
  - Build layout via `AudioProcessorValueTreeState::ParameterLayout`. Use `juce::ParameterID` with versioning, `AudioParameterFloat`, and `NormalisableRange` (including skew factors for logarithmic frequency scaling).
- **Audio Thread Access**:
  - Store cached raw atomic pointers (`std::atomic<float>*`) fetched via `APVTS::getRawParameterValue()` during processor construction/prep.
  - Read via `rawPtr->load(std::memory_order_relaxed)` inside `processBlock`.
  - **Forbidden in `processBlock`**: Never call `APVTS::getParameter()` or access `juce::ValueTree` directly.
- **State Serialization & Undo/Redo**:
  - Encapsulate complete plugin state in `juce::ValueTree` managed by APVTS.
  - Implement `getStateInformation()` and `setStateInformation()` by serializing `APVTS::copyState()` to/from XML/binary memory blocks.

## 7. UI Architecture: JUCE Native vs WebView (JUCE 8)

### Option A: JUCE Native UI (`juce::Component`)
- Render using `juce::Graphics` (Direct2D on Windows, Metal on macOS).
- Bind UI controls via APVTS attachments (`SliderAttachment`, `ButtonAttachment`, `ComboBoxAttachment`).
- Member order: Declare `LookAndFeel` variables *before* any dependent UI components.
- Visual updates (VU meters, FFT): Write audio data to SPSC FIFO in audio thread. In UI, run a `juce::Timer` (30–60 Hz) to poll the FIFO and trigger `repaint()`. Never call `repaint()` from the audio thread.

### Option B: JUCE 8 WebView UI (`juce::WebBrowserComponent`)
- **Runtime Engines**: Windows (WebView2 / Edge Chromium), macOS/iOS (WebKit), Linux (GTK WebKit2) with WebGL/WebGPU acceleration.
- **3-Tier Architecture**: Frontend Web UI (HTML5/CSS/JS/TS/React/Vue/Svelte) <-> Native IPC Layer (`window.__JUCE__.backend`) <-> C++ Backend (`PluginEditor`/`PluginProcessor`).
- **Configuration Options**:
  - `.withNativeIntegrationEnabled()`: Injects JS bridge.
  - `.withNativeFunction()`: Exposes C++ async functions callable from JS (using `juce::var`).
  - `.withEventListener()`: Registers JS event callbacks in C++.
  - `.withResourceProvider()`: Serves web assets directly from binary memory / ZIP archive without an HTTP server or disk access.

- **CRITICAL RULE: C++ Editor Header Member Destruction Order**:
  In C++, class members are destroyed in **EXACT REVERSE** order of declaration in the `.h` file.
  `WebSliderRelay` and attachments interact with `WebBrowserComponent`. If `WebBrowserComponent` is destroyed *after* the relays/attachments, dangling references cause use-after-free crashes upon closing the editor GUI.

  **Mandatory Declaration Order in `PluginEditor.h`**:
  ```cpp
  private:
      MyAudioProcessor& processor;

      // 1. RELAYS FIRST (Destroyed LAST)
      juce::WebSliderRelay gainRelay { "gain" };
      juce::WebSliderRelay cutoffRelay { "cutoff" };

      // 2. ATTACHMENTS SECOND
      juce::WebSliderParameterAttachment gainAttachment;
      juce::WebSliderParameterAttachment cutoffAttachment;

      // 3. WEBVIEW COMPONENT LAST (Destroyed FIRST)
      juce::WebBrowserComponent webView;
  ```

- **High-Frequency Visual Data Streaming to WebView**:
  Do NOT send high-rate UI data (e.g. 60 FPS FFT/meters) via standard IPC JSON string messaging (excessive serialization overhead). Use either:
  1. Internal loopback WebSocket Server in C++ emitting binary `ArrayBuffer` payloads from an SPSC FIFO to JS.
  2. Binary endpoint via `ResourceProvider` polled via JS `fetch()` requests returning raw binary byte arrays.

## 8. Build System & CMake Configuration (JUCE 8 / C++20)
- **Standard Target**: C++20 (`set(CMAKE_CXX_STANDARD 20)`), `juce_add_plugin` generating VST3, AU, and Standalone.
- **Compile Flags & Definitions**:
  - Definitions: `JUCE_WEB_BROWSER=1`, `JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1`, `JUCE_DISABLE_CAUTION_PARAMETERS=1`.
  - MSVC: `/EHs-c- /O2 /fp:fast`
  - GCC/Clang: `-fno-exceptions -O3 -ffast-math`
- **Web UI Resource Embedding**:
  - Zip frontend `dist` directory into `webui.zip` via custom CMake target.
  - Embed into binary using `juce_add_binary_data`.
  - Extract/serve from memory via `juce::ZipFile` inside `ResourceProvider` callback.

## 9. Testing & Quality Assurance
- **Unit Testing**: Public functions must have at least one unit test with descriptive naming using Catch2.
- **Validation**:
  - Run `pluginval` CLI for real-time validation (trap heap allocations / locks on audio thread).
  - Run `pluginval` in **Fuzz Mode** for automated parameter stress testing prior to release.
- **JUCE GUI Test Setup**: Wrap GUI tests in `juce::initialiseJuce_GUI()` and `juce::shutdownJuce_GUI()`.
---

## 7. WebView UI Standards (React / TypeScript / Vite)

### 7.1 File Structure
- All UI source code lives in `ui/`.
- Component files use `PascalCase.tsx` (e.g., `VstLayout.tsx`, `Knob.tsx`).
- Styles co-located as CSS Modules: `Knob.module.css`.
- Place shared types in `ui/src/types/`, shared utilities in `ui/src/utils/`.

### 7.2 Component Architecture
- **State**: Hoist all parameter state to `App.tsx` as controlled props (no internal state for values bound to DSP params).
- **Props**: Define strict TypeScript interfaces for every component's props.
- **Pure components**: Use `React.memo` for stable, frequently rendered components (e.g., `Knob`, `GainMeter`).
- **3D**: Use `@react-three/fiber` exclusively for the `FftVisualizer`. Never mix Three.js directly.

### 7.3 Styling (Glitch Brutalism)
- **Colors**: Use CSS custom properties defined on `:root` in `ui/src/index.css`.
  - `--bg: #0f0e0e` (dark industrial background)
  - `--fg: #f6f6f6` (light text and accents)
  - `--accent: #c7c3ba` (warm value arc — reserved exclusively for knob arcs)
- **Shapes**: Prefer sharp corners and hard shadows. Avoid `border-radius`, `blur()`, or soft gradients.
- **Noise overlay**: The `.vst-container` must include the fractal-noise SVG `::after` pseudo-element for grain texture.

### 7.4 Interaction Patterns
- **Knob drag**: Vertical mouse drag only (up = increase value, down = decrease).
- **Keyboard**: All interactive controls must support `focus-visible` and arrow-key (or Enter) activation.
- **Touch**: Ensure controls work on touch devices (pointer events, not just mouse events).

### 7.5 Testing
- **Unit tests**: Use Vitest with `@testing-library/react` for component integration.
- **Visual**: Use Playwright for visual regression tests of the full WebView UI.
- **Mocking**: Mock `window.__JUCE__.backend` IPC bridge when testing state flow.

### 7.6 Accessible Communication
- Follow the glossary in `UBIQUITOUS_LANGUAGE.md`.
- Never use legacy terms: "circus" (use "value arc"), "stock slider" (use "knob"), "HTML editor" (use "WebView UI").
