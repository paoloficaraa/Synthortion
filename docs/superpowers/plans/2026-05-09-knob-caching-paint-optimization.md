# Knob Caching and Paint Optimization Implementation Plan

> **For agentic workers:** keep this plan code-only. Use JUCE-native caching, not a custom sprite-sheet system. Before editing, run the installed `find-skills` utility (`npx skills find`) to identify the best installed skill for this task; if `find-skills` is unavailable or returns no clear match, inspect installed skills and pick the best fit manually.

**Goal:** Reduce rotary-knob repaint cost using existing JUCE buffering and image reuse instead of a custom cache class.

**Scope:** `plugin/include/Synthortion/SynthortionLookAndFeel.h`, `plugin/src/SynthortionLookAndFeel.cpp`, `plugin/src/PluginEditor.cpp`, `plugin/include/Synthortion/PluginEditor.h`

## Task 1: Remove the custom knob cache approach

Delete any `KnobCache` files or references if they exist, and remove any new members, includes, or helper methods that were added for sprite-sheet generation. Do not replace them with another custom cache class.

## Task 2: Use JUCE-native buffering for static controls

For knob-heavy components that do not need to redraw every frame, enable `setBufferedToImage(true)` on the relevant JUCE components so the framework handles the cached rendering path.

## Task 3: Reuse a single rendered knob image where custom drawing is still needed

If a knob style still needs custom vector artwork, render that artwork once into a `juce::Image` inside the LookAndFeel, then reuse it and apply `juce::AffineTransform` for the knob rotation or placement. Keep the caching local to the LookAndFeel and keyed only by the minimal style state needed.

## Task 4: Wire the editor to the native caching path

Update `PluginEditor` and `SynthortionLookAndFeel` so the rotary-slider drawing path uses the buffered/image approach directly. Remove any stale sprite-sheet or lookup-table logic, then build to confirm no dangling references remain.