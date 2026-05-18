# OpenGL Spectrum Analyzer Implementation Plan

> **For agentic workers:** keep this plan code-only. Replace the old CPU analyzer completely; the new analyzer is a direct drop-in. Before editing, run the installed `find-skills` utility (`npx skills find`) to identify the best installed skill for this task; if `find-skills` is unavailable or returns no clear match, inspect installed skills and pick the best fit manually.

**Goal:** Move spectrum rendering to an OpenGL-backed component and remove the legacy CPU analyzer from the shipped code path.

**Scope:** `plugin/include/Synthortion/OpenGLSpectrumAnalyzer.h`, `plugin/src/OpenGLSpectrumAnalyzer.cpp`, `plugin/src/shaders/*.glsl`, `plugin/src/PluginEditor.cpp`, `plugin/include/Synthortion/PluginEditor.h`, plus the legacy `SpectrumAnalyzer` files and their build references

## Task 1: Add the OpenGL spectrum analyzer and connect it to the existing data flow

Create the new `OpenGLSpectrumAnalyzer` component, its shaders, and the minimal rendering pipeline it needs. Keep the class self-contained, and wire it to the existing spectrum data source and EQ reference without introducing compatibility wrappers.

## Task 2: Replace the editor usage directly

Update `PluginEditor` to instantiate and use `OpenGLSpectrumAnalyzer` in place of the old analyzer. Remove stale includes, member variables, callbacks, and any code that still assumes the CPU analyzer exists.

## Task 3: Delete the legacy CPU analyzer and its build references

Remove `plugin/include/Synthortion/SpectrumAnalyzer.h` and `plugin/src/SpectrumAnalyzer.cpp`, then remove them from any CMake or source lists so the old analyzer no longer ships or compiles.