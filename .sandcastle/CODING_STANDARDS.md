# Coding Standards

## Style

- Use `camelCase` for variables and functions; `PascalCase` for classes and user-defined types.
- Use Allman brace style: braces on their own line, aligned to the control statement column.
- Indent with 4 spaces only (no tabs).
- Binary operators require a space before and after; logical NOT (`!`) must be followed by a space; increment (`++`) and decrement (`--`) operators must not have spaces between themselves and the operand.
- Pointer and reference declarations place the space after `*` or `&`, adjacent to the base type (e.g., `AudioBuffer* buffer`).
- Never declare multiple pointer variables on the same line.
- Template parameters use descriptive names (e.g., `SampleType`), avoid single-character `T`.
- Use `nullptr` exclusively for null pointers; never use `NULL` or `0`.
- Mark functions that cannot throw as `noexcept`.
- Use `override` on all virtual function overrides; omit redundant `virtual` keyword.
- Use C++-style casts (`static_cast`); never use C-style casts.
- Prefer braced initialization `{}` for primitive variables to prevent narrowing.
- Use `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR` for non-copyable classes.

## Control Flow

- Omit `else` block when the preceding `if` ends with `return` (flat code structure, per LLVM style).
- Single-line conditionals may omit braces only when brevity creates no ambiguity.
- Restrict pointer scope: initialize inside the conditional statement when possible.
- Avoid nested ternary operators; prefer `switch` or `if`/`else` chains.

## Memory / Modern C++

- No raw `new`/`delete`; use RAII constructs (`std::unique_ptr`).
- No global statics or singletons within plugin instances (prevents collisions across multiple instances).

## Strings (JUCE 8)

- Do not use the deprecated `T()` macro for string literals.
- `juce::String` and `juce::StringRef` handle native literals automatically.
- For pre-C++20: use `CharPointer_UTF8` escape sequences for special characters. For C++20+: use `u8` prefix.

## Architecture

- Strict separation between Message Thread (UI) and Audio Thread (DSP).
- Inside `processBlock` (audio thread): **no** heap allocation, **no** mutex locks, **no** I/O, **no** string formatting or `DBG` macros.
- Pre-allocate all buffers, filter coefficients, and look-up tables in `prepareToPlay`.
- Use composition over inheritance: assemble DSP chains via `juce::dsp::ProcessorChain`.
- Member declaration order: `LookAndFeel` objects must precede UI components referencing them (C++ destruction order).
- Use `juce::AudioProcessorValueTreeState` (APVTS) for parameter management.
- Cache `std::atomic<float>*` pointers from APVTS instead of string-based lookups in `processBlock`.
- Smooth parameter changes with `juce::LinearSmoothedValue` or `juce::SmoothedValue` (ramp time 5-50ms).

## DSP Pipeline

- Apply oversampling around nonlinear stages (`juce::dsp::Oversampling`) to prevent aliasing.
- Choose FIR half-band filters for linear phase or IIR half-band for low latency.
- Report latency to the host via `setLatencySamples`.
- Place a DC Blocker at the output after downsampling to remove DC offset from asymmetric saturation.
- DC Blocker: IIR filter described by `y[n] = x[n] - x[n-1] + R * y[n-1]` with `R` between `0.99` and `0.999`.
- Include `juce::dsp::Gain` processor at the end for makeup gain control.

## Testing

- Every public function must have at least one unit test.
- Use descriptive test names that explain the expected behavior.
- Catch2 is the unit testing framework.
- Spectra testing: measure oversampling effectiveness and aliasing via FFT/assert matchers in CI.
- Real-time validation: run pluginval CLI to detect heap allocations or locks on the audio thread (release builds, CI pipeline).
- Parameter fuzzing: run pluginval Fuzz Mode before release to catch crashes from rapid parameter changes.
- Test entry point must call `juce::initialiseJuce_GUI()` before tests and `juce::shutdownJuce_GUI()` after.
