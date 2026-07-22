# Coding Standards

## Style
- `camelCase` vars/functions; `PascalCase` classes/types
- Allman braces (braces on own line, aligned to control statement)
- 4 spaces indent, no tabs
- Spaces around binary ops; `!` followed by space; no space around `++`/`--`
- Pointer/ref: space after `*`/`&` (e.g. `AudioBuffer* buffer`); no multi-ptr declarations same line
- Template params: descriptive names (e.g. `SampleType`), not `T`
- `nullptr` only; no `NULL`/`0`
- `noexcept` on non-throwing functions; `override` on virtual overrides (omit `virtual`)
- C++-style casts (`static_cast`), no C-style; braced init `{}` for primitives
- `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR` for non-copyables

## Control Flow
- Omit `else` after `return`; avoid nested ternaries (use `switch`/`if-else`)
- Single-line conditionals may omit braces if unambiguous
- Restrict pointer scope: init inside conditionals

## Memory / Modern C++
- RAII (`std::unique_ptr`), no raw `new`/`delete`
- No global statics/singletons in plugin instances

## Strings (JUCE 8)
- No `T()` macro; `juce::String`/`StringRef` handle native literals
- Pre-C++20: `CharPointer_UTF8` escapes. C++20+: `u8` prefix

## Audio Thread Safety
- `processBlock`: **no** heap alloc, mutexes, I/O, string formatting, `DBG`
- Pre-allocate buffers/coefficients/tables in `prepareToPlay`
- Compose DSP chains via `juce::dsp::ProcessorChain`
- Member order: `LookAndFeel` before dependent UI components
- APVTS for parameters; cache `std::atomic<float>*` (no string lookups in `processBlock`)
- Smooth params with `juce::LinearSmoothedValue`/`SmoothedValue` (5-50ms ramp)

## DSP Pipeline
- Oversample nonlinear stages (`juce::dsp::Oversampling`): FIR half-band (linear phase) or IIR half-band (low latency)
- Report latency via `setLatencySamples`
- DC Blocker at output post-downsample: `y[n] = x[n] - x[n-1] + R * y[n-1]`, `R` 0.99-0.999
- `juce::dsp::Gain` at end for makeup gain

## Testing
- Public functions must have ≥1 unit test; descriptive names
- Catch2 framework
- pluginval CLI for real-time validation (heap allocs/locks on audio thread)
- pluginval Fuzz Mode for parameter fuzzing pre-release
- `juce::initialiseJuce_GUI()` / `juce::shutdownJuce_GUI()` around tests