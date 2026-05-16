# Parameter Mapping Fixes Implementation Plan

> **For agentic workers:** keep this plan code-only. Use semantic anchors in the source, not exact line numbers. Before editing, run the installed `find-skills` utility (`npx skills find`) to identify the best installed skill for this task; if `find-skills` is unavailable or returns no clear match, inspect installed skills and pick the best fit manually.

**Goal:** Update EQ frequency parameters to perceptually sensible logarithmic ranges and keep bypass behavior aligned with the new endpoints.

**Scope:** `plugin/src/PluginProcessor.cpp`

## Task 1: Update all EQ frequency parameters in one pass

Locate `createParameterLayout` and revise the frequency parameters together instead of as separate edits.

- `LOW_CUT_FREQ`: `juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.2f)`, default `20.0f`
- `LOW_MID_FREQ`: `juce::NormalisableRange<float>(100.0f, 2000.0f, 1.0f, 0.3f)`, default `500.0f`
- `HIGH_MID_FREQ`: `juce::NormalisableRange<float>(2000.0f, 12000.0f, 1.0f, 0.3f)`, default `5000.0f`
- `HIGH_CUT_FREQ`: `juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.2f)`, default `20000.0f`
- Keep parameter IDs and labels unchanged
- Keep `DELAY_MIX` at a neutral default of `0.0f`

## Task 2: Align EQ bypass logic with the new parameter endpoints

Find the DSP path that applies the EQ in `processBlock`, plus any mirrored update path such as `updateAllDSPParameters`, and make the bypass checks semantic:

- low cut bypasses only when the cutoff is at the minimum endpoint (`<= 20.0f`)
- high cut bypasses only when the cutoff is at the maximum endpoint (`>= 20000.0f`)
- do not use zero-based or loose threshold checks once the new ranges are in place

## Task 3: Build and verify the parameter changes

Build the plugin and resolve any compile errors introduced by the parameter layout update. If a warning or failure appears, fix the source code rather than adjusting the plan.