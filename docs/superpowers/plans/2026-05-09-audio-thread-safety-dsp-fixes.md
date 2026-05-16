# Audio Thread Safety & DSP Fixes Implementation Plan

> **For agentic workers:** keep this plan code-only. Use semantic anchors inside the implementation, not exact line numbers or generated review docs. Before editing, run the installed `find-skills` utility (`npx skills find`) to identify the best installed skill for this task; if `find-skills` is unavailable or returns no clear match, inspect installed skills and pick the best fit manually.

**Goal:** Remove audio-thread allocation, keep bypass logic aligned with the parameter endpoints, and validate the chorus path only where it affects shipped DSP.

**Scope:** `plugin/src/PluginProcessor.cpp`, `plugin/src/SynthortionChorus.cpp`, `plugin/include/Synthortion/SynthortionChorus.h`

## Task 1: Remove audio-thread allocation from `processBlock`

Find the dry-buffer copy path inside `processBlock` and replace any heap-owning copy such as `makeCopyOf` with a reuse of the preallocated member buffer. Resize or allocate only in `prepareToPlay`, then use `copyFrom` or an equivalent non-allocating transfer in the audio callback.

## Task 2: Align EQ bypass checks with the parameter endpoints

Find the EQ application path in `processBlock` and the mirrored parameter update path, then apply semantic bypass checks:

- low cut is bypassed only at the minimum frequency endpoint
- high cut is bypassed only at the maximum frequency endpoint
- keep the check consistent between the audio callback and any deferred update function

## Task 3: Verify the chorus implementation before changing it

Inspect `SynthortionChorus.{h,cpp}` and only touch the code if it fails the intended design:

- multiple delay voices, not a single JUCE stock chorus wrapper
- modulation and stereo behavior implemented in the class itself
- any required filter or crossover shaping kept in the audio path, not in docs

If the implementation already matches the desired architecture, leave it unchanged.

## Task 4: Build and fix regressions introduced by the refactor

Build the plugin and resolve any compile or runtime issues caused by the audio-thread changes. Do not add documentation or test-plan files as part of this plan.