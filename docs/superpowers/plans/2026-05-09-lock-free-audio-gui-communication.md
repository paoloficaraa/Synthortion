# Lock-Free Audio→GUI Communication Implementation Plan

> **For agentic workers:** keep this plan code-only. Focus on spectrum-data handoff only; leave meter atomics alone. Before editing, run the installed `find-skills` utility (`npx skills find`) to identify the best installed skill for this task; if `find-skills` is unavailable or returns no clear match, inspect installed skills and pick the best fit manually.

**Goal:** Move spectrum data from the audio thread to the GUI without locks, allocations, or blocking work.

**Scope:** `plugin/include/Synthortion/PluginProcessor.h`, `plugin/src/PluginProcessor.cpp`, `plugin/include/Synthortion/PluginEditor.h`, `plugin/src/PluginEditor.cpp`

## Task 1: Add a minimal lock-free queue for spectrum data

Introduce a fixed-size `AudioToGuiQueue` only if the spectrum analyzer still needs a thread-safe handoff. Use `juce::AbstractFifo` plus preallocated storage, and keep the queue limited to spectrum samples. Do not add any extra queueing for meter values.

## Task 2: Push spectrum samples from `processBlock` without blocking

Find the audio path that currently feeds spectrum visualization and send the required samples into the queue from `processBlock`. Keep the producer side non-allocating and avoid any wait or lock behavior.

## Task 3: Drain the queue on the GUI update path

Update the editor timer or refresh callback to pull samples from the queue, forward them into the spectrum analyzer, and keep the existing EQ bypass state synchronized from the processor parameters.

## Task 4: Build and keep the change set code-only

Build the plugin and fix any compile errors introduced by the queue wiring. Do not generate markdown docs or manual test plans as part of this plan.