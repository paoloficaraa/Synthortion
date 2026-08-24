# ADR 0001: Parameter ID Schema & Bridge Protocol

**Status:** Accepted  
**Date:** 2026-08-11

## Context

Synthortion has two surfaces that must share parameter state:

- **React UI** (in `ui/`) — runs in a JUCE `WebBrowserComponent` WebView. State is hoisted to `App` as `PluginState` (see `ui/src/lib/pluginState.ts`). Changes are diffed via `diffPluginState(prev, next)` → array of `{ parameterId, value }` and pushed to a `DspBridge`.
- **C++ DSP** (in `plugin/`) — uses JUCE `AudioProcessorValueTreeState` (APVTS) with parameters registered via `createParameterLayout()` (see `plugin/src/PluginProcessor.cpp`).

Today the UI uses a `noopDspBridge`; parameters never reach the DSP. The task is to connect them.

## Decision

### 1. Canonical Parameter IDs

Use **APVTS parameter IDs** as the single source of truth. The UI must adopt these IDs when calling `DspBridge.setParameter(id, value)`.

| UI Field (`PluginState` key) | APVTS Parameter ID | Type | Notes |
|------------------------------|-------------------|------|-------|
| `inputGain` | `INPUT_GAIN` | float | dB, -60..+12 |
| `outputGain` | `OUTPUT_GAIN` | float | dB, -60..+12 |
| `drive` | `COLOR` | float | 0..100 (UI) → 0..1 (APVTS) |
| `driveRoute` | — | enum | UI-only (PRE/POST routing); handled in DSP processBlock |
| `driveOn` | — | bool | UI-only (module power); DSP reads `bypassParam` + per-module logic |
| `bitcrush` | `BITCRUSH` | float | 1..16 bits (UI) → 0..1 (APVTS) |
| `bitcrushOn` | — | bool | UI-only |
| `delayTime` | `DELAY_TIME` | float | ms, 1..2000 (UI) → 0..1 (APVTS) |
| `delayMix` | `DELAY_MIX` | float | 0..100% (UI) → 0..1 (APVTS) |
| `delayFbk` | `DELAY_FEEDBACK` | float | 0..95% (UI) → 0..1 (APVTS) |
| `delaySync` | — | enum | UI-only (SYNC/FREE/PING-PONG); DSP derives from host tempo |
| `delayOn` | — | bool | UI-only |
| `chorus` | `CHORUS_MIX` | float | 0..100% (UI) → 0..1 (APVTS) |
| `chorusWide` | — | bool | UI-only (stereo width); DSP has fixed width |
| `chorusOn` | — | bool | UI-only |
| `engineActive` | `PLUGIN_BYPASS` | bool | Inverted: UI `true` = DSP `false` |

**Rationale:** APVTS owns the parameter layout, automation, and state persistence. The UI adapts.

### 2. Value Normalization

UI values are **user-facing** (Hz, ms, dB, %, semantic enums). APVTS values are **normalized 0..1**.

A **normalization layer** lives in the UI bridge (TypeScript). It converts both directions:

- `toAPVTS(uiKey, uiValue) → number (0..1)`
- `fromAPVTS(apvtsKey, apvtsValue) → uiValue`

The bridge holds a **parameter spec table** (see below) with range, skew, and unit info so conversions are deterministic and testable.

### 3. Bridge Message Protocol

**JS → C++** (parameter change)

```ts
// Posted via window.chrome.webview.postMessage (or JUCE WebBrowserComponent JS bridge)
{ type: "setParameter", id: "COLOR", value: 0.42 }
```

**C++ → JS** (initial sync / automation / preset load)

```ts
// Dispatched via WebBrowserComponent::evaluateJavascript
window.__SYNTORTION_BRIDGE__.onParameterChange("COLOR", 0.42)
```

**Connection lifecycle**

```ts
// JS → C++ on mount
{ type: "connect" }

// C++ → JS on ready
window.__SYNTORTION_BRIDGE__.onConnect({ sampleRate, blockSize, parameterSpecs })

// JS → C++ on unmount
{ type: "disconnect" }
```

### 4. Parameter Spec Table (Single Source for Normalization)

```ts
// ui/src/lib/parameterSpecs.ts (new file)
export const parameterSpecs = {
  INPUT_GAIN:        { uiKey: "inputGain",        min: -60,  max: 12,   skew: 0,   unit: "dB"   },
  OUTPUT_GAIN:       { uiKey: "outputGain",       min: -60,  max: 12,   skew: 0,   unit: "dB"   },
  COLOR:             { uiKey: "drive",            min: 0,    max: 100,  skew: 0.5, unit: "%"    },
  BITCRUSH:          { uiKey: "bitcrush",         min: 1,    max: 16,   skew: 0.3, unit: "bits" },
  DELAY_TIME:        { uiKey: "delayTime",        min: 1,    max: 2000, skew: 0.25,unit: "ms"   },
  DELAY_MIX:         { uiKey: "delayMix",         min: 0,    max: 100,  skew: 0,   unit: "%"    },
  DELAY_FEEDBACK:    { uiKey: "delayFbk",         min: 0,    max: 95,   skew: 0,   unit: "%"    },
  CHORUS_MIX:        { uiKey: "chorus",           min: 0,    max: 100,  skew: 0,   unit: "%"    },
  PLUGIN_BYPASS:     { uiKey: "engineActive",     min: 0,    max: 1,    skew: 0,   unit: "bool", invert: true },
} as const
```

### 5. UI-Only State (Not in APVTS)

The following `PluginState` fields have **no APVTS counterpart** and are **not sent over the bridge**:

- `driveRoute` (PRE/POST) — DSP handles routing internally in `processBlock`
- `driveOn`, `bitcrushOn`, `delayOn`, `chorusOn` — per-module power; DSP implements as conditional processing
- `delaySync` (SYNC/FREE/PING-PONG) — DSP derives from host tempo + delay time
- `chorusWide` — DSP has fixed stereo width

These remain purely in React state. If the C++ side ever needs them, add APVTS parameters and extend the spec table.

### 6. C++ Message Handler

In `PluginProcessor` (or `PluginEditor`):

- Add `juce::WebBrowserComponent` to `PluginEditor`
- Register a `juce::WebBrowserComponent::JavascriptCallback` or use `juce::WebView2` message channel
- On `{ type: "setParameter", id, value }`:
  ```cpp
  if (auto* param = apvts.getParameter(id))
      param->setValueNotifyingHost(value);
  ```
- On connect: send full parameter snapshot + `parameterSpecs` (for UI normalization)
- On parameter change (APVTS listener): `webview.evaluateJavascript("window.__SYNTORTION_BRIDGE__.onParameterChange(...)")`

## Consequences

- **UI adopts APVTS IDs** — `diffPluginState` output must map UI keys → APVTS IDs before calling bridge.
- **Normalization is explicit and testable** — spec table drives both directions; no magic numbers scattered.
- **UI-only state stays in React** — no over-engineering the DSP for display concerns.
- **Single source of truth** — APVTS owns automation, presets, host sync; UI is a view.

## Related

- ADR 0002: C++ WebView Integration (to be written)
- ADR 0003: UI Bridge Implementation (to be written)