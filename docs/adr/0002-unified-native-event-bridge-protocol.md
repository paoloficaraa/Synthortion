# ADR 0002: Unified Native Event Bridge Protocol

**Status:** Accepted  
**Date:** 2026-08-24  

## Context

In the initial implementation of the Synthortion bridge (ADR 0001), communication between C++ (`PluginEditor`) and the TypeScript WebView UI (`webViewDspBridge.ts`) used a hybrid dual-dispatch approach:
- C++ dispatched parameter changes, spectrum frames, and meter frames using both `WebBrowserComponent::emitEventIfBrowserIsVisible` (JUCE 8 native event API) and `WebBrowserComponent::evaluateJavascript` (string-concatenated JavaScript calling functions on `window.__SYNTORTION_BRIDGE__`).
- TypeScript supported multiple legacy fallbacks (`window.__SYNTORTION_BRIDGE__`, WebKit message handlers, `window.postMessage`).
- Parameter keys were inconsistent (`parameterId` vs `id`), and telemetry frames incurred string formatting and memory allocation overhead on every 60 Hz timer tick.

With JUCE 8 fully standardizing `WebBrowserComponent::Options::withEventListener` and `emitEventIfBrowserIsVisible`, dual-dispatch is redundant, inefficient, and prone to string formatting injection bugs.

## Decision

### 1. Pure Native Event Communication
- Eliminate all `evaluateJavascript` string evaluation, JavaScript function injection, and legacy window fallbacks (`window.__SYNTORTION_BRIDGE__`, WebKit handlers).
- Standardize all IPC on native JUCE 8 `WebBrowserComponent` events (`window.__JUCE__.backend.emitEvent` in JS and `WebBrowserComponent::emitEventIfBrowserIsVisible` in C++).

### 2. Standardized Message Schemas & Normalization
- All parameter updates in both directions (`setParameter` from UI → C++, `parameterChange` from C++ → UI) strictly use the shape `{ id: string, value: number }`.
- `value` is always a normalized `[0.0, 1.0]` float matching APVTS host automation.
- Parameter metadata, units, and ranges are provided dynamically to the UI during the initialization handshake (ADR 0001 / Issue #109), allowing the UI to handle value mapping without static duplication.

### 3. Reactive Lifecycle Handshake
- **UI Mount**: React UI registers native event listeners, then dispatches `connect`:
  ```typescript
  window.__JUCE__.backend.emitEvent('connect', {})
  ```
- **C++ Response**: `PluginEditor` receives `connect` and responds with a single `init` event:
  ```cpp
  webView->emitEventIfBrowserIsVisible("init", juce::var(initPayloadDynamicObject));
  ```
- No unsolicited push events on page load; eliminates race conditions during startup or Vite HMR reloads.

### 4. High-Frequency Telemetry Streams
- `spectrumFrame`: Dispatches an array of 80 logarithmic band magnitudes (`Array<float>`) directly as `juce::var`.
- `meterFrame`: Dispatches an object containing normalized peak levels (`{ input: float, output: float }`).
- Both are dispatched from the 60 Hz timer via `emitEventIfBrowserIsVisible`. If the plugin window is minimized or hidden, JUCE automatically skips serialization and dispatch, conserving CPU.

### 5. Boundary Validation & Error Resilience
- **C++**: `setParameter` verifies parameter ID existence in APVTS, checks `std::isfinite(value)`, and clamps to `[0.0f, 1.0f]`. Invalid messages are discarded in Release and logged/asserted in Debug.
- **TypeScript**: Inbound payloads are validated with type guards; malformed frames are safely dropped.

## Consequences

### Positive
- Zero string concatenation and JavaScript parsing overhead on 60 FPS animation frames.
- Deterministic single-envelope initialization with no race conditions.
- Strict type safety and alignment across C++ APVTS and TypeScript frontend.
- Clean separation between plugin host runtime and isolated headless unit test environments (via mock bridges).

### Negative / Trade-offs
- The UI strictly requires the JUCE 8 `window.__JUCE__.backend` environment in production; standalone browser development requires a local mock bridge provider.
