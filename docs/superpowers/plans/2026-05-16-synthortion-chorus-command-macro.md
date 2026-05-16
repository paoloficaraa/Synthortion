# SynthortionChorus Command Macro Refactoring Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Refactor SynthortionChorus to use fixed-engine design where CHORUS_MIX is a command macro that interpolates only depth and stereo phase, while delay, rate, and crossover frequency remain fixed at target values.

**Architecture:** Replace variable parameters with fixed constants; implement Linkwitz-Riley crossover (400Hz) to apply chorus only to high frequencies; use multi-sine LFO with fixed 1.1Hz rate and 5.1ms base delay; interpolate depth (0→0.25) and phase offset (0→45°) from mix; dry path passes low band untouched.

**Tech Stack:** JUCE 8, C++20, juce_dsp (LinkwitzRileyFilter, DelayLine, SmoothedValue)

---

## File Structure

- **plugin/include/Synthortion/SynthortionChorus.h** — Class declaration, constants, crossover filter members
- **plugin/src/SynthortionChorus.cpp** — Implementation: prepare/reset, crossover processing, fixed LFO, depth/phase interpolation, dry/wet mixing
- **plugin/src/PluginProcessor.cpp** — Parameter updates: only setChorusMix, remove redundant setRate/setDepth calls

---

### Task 1: Update SynthortionChorus.h

**Files:**
- Modify: `plugin/include/Synthortion/SynthortionChorus.h`

**Changes:**
- Remove `setRate(float)` and `setDepth(float)` declarations
- Add `using namespace juce::dsp;` for convenience (optional)
- Add member variables:
  - `LinkwitzRileyFilter<float> crossoverFilter[2];` // L/R
  - Target constants: `static constexpr float targetDelayMs = 5.1f;`, `static constexpr float targetRateHz = 1.1f;`, `static constexpr float crossoverFreq = 400.0f;`, `static constexpr float targetPhaseOffsetDeg = 45.0f;`
  - Interpolated state: `float interpolatedDepth = 0.0f;`, `float interpolatedPhaseOffsetRad = 0.0f;`
  - Precomputed: `float baseSamples;` (updated in prepare)
- Keep existing: `DelayLine`, `smoothedMix`, `lfoPhase`, `numVoices`, `sampleRate`
- Keep `setChorusMix(float)` unchanged

- [ ] Write modified header
- [ ] Verify header compiles (will fail until cpp matches)

---

### Task 2: Overhaul SynthortionChorus.cpp — prepare/reset/constants

**Files:**
- Modify: `plugin/src/SynthortionChorus.cpp`

**Steps:**

- [ ] Update `prepare()`:
  - Set `baseSamples = sampleRate * (targetDelayMs / 1000.0f);`
  - Prepare crossover filters: for each channel `crossoverFilter[c].prepare(spec); crossoverFilter[c].setType(LinkwitzRileyFilterType::highpass); crossoverFilter[c].setCutoffFrequency(crossoverFreq);`
  - Existing delayLine prepare stays
  - `smoothedMix.reset(sampleRate, 0.05);`
  - Call `reset()`

- [ ] Update `reset()`:
  - `delayLine.reset();`
  - `crossoverFilter[0].reset(); crossoverFilter[1].reset();`
  - `lfoPhase = 0.0f;`
  - `interpolatedDepth = 0.0f; interpolatedPhaseOffsetRad = 0.0f;`

- [ ] Remove `setRate()` and `setDepth()` implementations entirely

- [ ] Compile check (build target: plugin)

---

### Task 3: Implement Fixed-Engine Process Loop

**Files:**
- Modify: `plugin/src/SynthortionChorus.cpp` (process method)

**Implementation:**

- [ ] Replace entire `process()` body with:

```cpp
void SynthortionChorus::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels == 0 || numSamples == 0) return;

    auto* leftData = buffer.getWritePointer(0);
    auto* rightData = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    const float maxModSamples = static_cast<float>(sampleRate) * 0.01f; // 10ms max modulation
    const float phaseIncrement = juce::MathConstants<float>::twoPi * targetRateHz / static_cast<float>(sampleRate);
    const float stereoPhaseOffsetRad = juce::MathConstants<float>::pi * (interpolatedPhaseOffsetRad / juce::MathConstants<float>::pi); // already in rad

    for (int i = 0; i < numSamples; ++i)
    {
        const float mix = smoothedMix.getNextValue();

        // Interpolate depth and phase from mix (0→target values)
        interpolatedDepth = mix * 0.25f; // Depth target 0.25
        interpolatedPhaseOffsetRad = mix * juce::MathConstants<float>::pi * (targetPhaseOffsetDeg / 180.0f); // 45° → π/4

        // Advance LFO phase
        lfoPhase += phaseIncrement;
        if (lfoPhase >= juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;

        // Split input via crossover per channel
        float lowL = 0.0f, highL = 0.0f;
        float lowR = 0.0f, highR = 0.0f;

        crossoverFilter[0].processSample(0, leftData[i], lowL, highL);
        if (rightData != nullptr)
            crossoverFilter[1].processSample(1, rightData[i], lowR, highR);

        // Process high band through chorus (fixed delay + multi sine LFO)
        float voiceOutL = 0.0f;
        float voiceOutR = 0.0f;

        for (int v = 0; v < numVoices; ++v)
        {
            float phaseOffset = v * (juce::MathConstants<float>::twoPi / numVoices);

            // Multi-sine LFO: A*(sin(ωt) + 0.5*sin(3ωt))
            float lfoL = std::sin(lfoPhase + phaseOffset) + 0.5f * std::sin(3.0f * (lfoPhase + phaseOffset));
            float lfoR = std::sin(lfoPhase + phaseOffset + interpolatedPhaseOffsetRad) + 0.5f * std::sin(3.0f * (lfoPhase + phaseOffset + interpolatedPhaseOffsetRad));

            float delayTimeSamplesL = baseSamples + (lfoL * interpolatedDepth * maxModSamples);
            float delayTimeSamplesR = baseSamples + (lfoR * interpolatedDepth * maxModSamples);

            voiceOutL += delayLine.popSample(0, delayTimeSamplesL, false);
            if (rightData != nullptr)
                voiceOutR += delayLine.popSample(1, delayTimeSamplesR, false);
        }

        voiceOutL /= numVoices;
        voiceOutR /= numVoices;

        // Feedback path with tanh saturation
        const float saturatedL = std::tanh(voiceOutL * 1.5f);
        const float filteredL = feedbackFilter[0].processSample(saturatedL); // Keep existing lowpass filters

        if (rightData != nullptr)
        {
            const float saturatedR = std::tanh(voiceOutR * 1.5f);
            const float filteredR = feedbackFilter[1].processSample(saturatedR);
            delayLine.pushSample(1, highR + (filteredR * 0.3f));
        }

        delayLine.pushSample(0, highL + (filteredL * 0.3f));

        // Dry/wet mixing: low stays dry; high band crossfades
        const float dryGain = std::cos(mix * juce::MathConstants<float>::halfPi);
        const float wetGain = std::sin(mix * juce::MathConstants<float>::halfPi);

        leftData[i] = lowL + (highL * dryGain) + (filteredL * wetGain);

        if (rightData != nullptr)
        {
            rightData[i] = lowR + (highR * dryGain) + (filteredR * wetGain);
        }
    }
}
```

- [ ] Verify no allocations in loop (all locals are POD)
- [ ] Compile

---

### Task 4: Update PluginProcessor.cpp

**Files:**
- Modify: `plugin/src/PluginProcessor.cpp`

**Changes:**

- [ ] In `updateAllDSPParameters()`, remove any lines that call `chorus.setRate()` or `chorus.setDepth()` if present. Current code only calls `chorus.setChorusMix(chorusMixParam->load());` — leave as is.

- [ ] No other changes needed; `CHORUS_MIX` parameter already exists and is connected.

---

### Task 5: Real-Time Safety Review

**Check:**
- [ ] No heap allocations in `process()` (no `new`, `std::vector`, `String`, etc.)
- [ ] All state updates are simple float assignments (depth/phase from mix)
- [ ] `SmoothedValue` only for mix; other params are direct assignments per sample
- [ ] No locks or atomics in audio thread (existing code uses atomic param loads, acceptable)
- [ ] Crossover filters and delayLine are pre-prepared in `prepare()`

---

### Task 6: Build Verification

**Commands:**
```bash
git submodule update --init --recursive
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target plugin
```

- [ ] Build succeeds without warnings (treat warnings as errors if configured)
- [ ] No runtime crashes in plugin host (manual test in DAW or JUCE AudioPluginHost)

---

### Task 7: Manual Testing Procedure

**Steps:**
1. Load plugin in AudioPluginHost or DAW
2. Set `CHORUS_MIX` to 0.0 → verify chorus depth and rate are effectively bypassed (only high band passes, minimal modulation)
3. Set `CHORUS_MIX` to 1.0 → verify:
   - Audible 5.1ms delay modulation
   - LFO rate ~1.1Hz
   - Stereo width ~45° phase offset
   - Only frequencies >400Hz are chorused
4. Sweep `CHORUS_MIX` from 0→1 → depth and stereo width increase smoothly, rate stays constant
5. Check for clicks/pops during parameter changes (smoothedMix should prevent)
- [ ] Document test results (notes file optional)

---

## Self-Review Check

**Spec coverage:**
- Fixed delay 5.1ms ✓
- Fixed rate 1.1Hz ✓
- Interpolated depth 0→0.25 ✓
- Interpolated phase 0°→45° ✓
- Crossover 400Hz high-pass for wet ✓
- Multi-sine LFO ✓
- Constant-power dry/wet ✓
- No setRate/setDepth exposed ✓

**No placeholders.** All code provided in steps.

**Type consistency:** `interpolatedPhaseOffsetRad` computed from mix each sample; `baseSamples` computed in prepare; all other types match JUCE conventions.

---
