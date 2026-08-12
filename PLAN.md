# Plan: Connect Synthortion WebView UI to DSP

## Current State

- **UI**: React app with `PluginState` hoisted to `App`, diffed via `diffPluginState`, pushed through `DspBridge.setParameter()`.
- **DSP**: C++ JUCE processor with APVTS parameters. `processBlock` runs: inputGain → warmDistortion → bitCrusher → chorus → pingPongDelay → outputGain.
- **Bridge**: TypeScript `PARAMETER_IDS` maps UI keys → APVTS IDs. `webViewDspBridge` sends JSON via `postMessage`. C++ `handleMessage` parses and calls `setValueNotifyingHost()`.
- **Editor**: `PluginEditor` has a `WebBrowserComponent` pointing to `localhost:5173` (dev only), plus a full native UI (sliders, labels, buttons) that's now redundant.

## Gaps

| # | Gap | Impact |
|---|-----|--------|
| 1 | **No Vite build pipeline** — PluginEditor hardcodes `localhost:5173`. Production plugin has no bundled UI. | Plugin can't work outside dev. |
| 2 | **7 UI-only state fields have no APVTS parameter** — `driveOn`, `bitcrushOn`, `delayOn`, `chorusOn`, `driveRoute`, `delaySync`, `chorusWide`. | Module power/routing is invisible to DSP. |
| 3 | **No normalization** — UI sends raw values (e.g. `drive: 50`, `bitcrush: 14`) to `setValueNotifyingHost()` which expects 0–1 normalized floats. | All non-gain parameters are wrong in DSP. |
| 4 | **No DSP→UI sync** — Automation, preset loads, host-driven changes never reach React state. | UI shows stale values after host interaction. |
| 5 | **No initial state hydration** — JS doesn't know current APVTS values on mount. | UI starts at factory defaults, ignoring DAW state. |
| 6 | **Native UI still exists** — Old sliders/labels/buttons in PluginEditor overlap the WebView. | Visual mess, wasted resources. |
| 7 | **processBlock ignores per-module power** — All 4 effects always run (only master `PLUGIN_BYPASS` gates them). | Module power toggles are cosmetic only. |

## Phases

### Phase 1: Normalization Layer
**Files**: `ui/src/lib/parameterSpecs.ts` (new), `ui/src/lib/dspBridge.ts`

Create the declarative parameter spec table that maps each APVTS parameter to its UI key, range, and normalization formula. Implement `toAPVTS(uiKey, value)` and `fromAPVTS(apvtsKey, value)` functions.

**Key decisions**:
- APVTS `COLOR`, `BITCRUSH`, `DELAY_MIX`, `DELAY_FEEDBACK`, `CHORUS_MIX` are 0–1. UI values are 0–100 → divide by 100.
- `INPUT_GAIN`, `OUTPUT_GAIN`: UI dB → APVTS dB (both use same range, no conversion needed).
- `DELAY_TIME`: UI ms → APVTS ms (both use same 1–2000 range).
- `PLUGIN_BYPASS`: UI `engineActive: true` → APVTS `0.0` (inverted: DSP bypass ON = engine OFF).

### Phase 2: Missing APVTS Parameters
**Files**: `plugin/src/PluginProcessor.cpp`, `plugin/include/Synthortion/PluginProcessor.h`, `ui/src/lib/dspBridge.ts`

Add 7 new APVTS parameters:
- `DRIVE_ON` (bool, default true)
- `BITCRUSH_ON` (bool, default true)
- `DELAY_ON` (bool, default true)
- `CHORUS_ON` (bool, default true)
- `DRIVE_ROUTE` (bool: false=PRE, true=POST, default false)
- `DELAY_SYNC` (float: 0=SYNC, 1=FREE, 2=PING-PONG, default 0)
- `CHORUS_WIDE` (bool, default false)

Update `PARAMETER_IDS` in TypeScript to include all 16 parameters.

### Phase 3: Process Block Per-Module Power
**File**: `plugin/src/PluginProcessor.cpp`

Wire per-module `*On` parameters into `processBlock`:
```cpp
const bool driveOn = driveOnParam->load() > kBooleanThreshold;
const bool bitcrushOn = bitcrushOnParam->load() > kBooleanThreshold;
const bool delayOn = delayOnParam->load() > kBooleanThreshold;
const bool chorusOn = chorusOnParam->load() > kBooleanThreshold;

if (!bypass) {
    if (driveOn)  warmDistortion.process(...);
    if (bitcrushOn) bitCrusher.process(...);
    if (chorusOn) chorus.process(...);
    if (delayOn) pingPongDelay.process(...);
}
```

Implement drive route ordering: when `DRIVE_ROUTE = POST`, run distortion after the other effects instead of before.

### Phase 4: Vite Build Pipeline
**Files**: `plugin/CMakeLists.txt`, `plugin/` (new binary data), build script

1. Build the React UI: `cd ui && npm run build` → `ui/dist/`
2. Add a CMake step that runs Vite build and generates JUCE BinaryData from the output files.
3. Change `PluginEditor` to load from `BinaryData` instead of `localhost:5173`:
   ```cpp
   webView->goToURL("data:text/html," + BinaryData::index_html.toString());
   ```
4. For dev mode, keep `localhost:5173` fallback via a build-time flag or runtime check.

### Phase 5: Bidirectional State Sync (DSP→UI)
**Files**: `plugin/src/PluginProcessor.cpp`, `plugin/src/PluginEditor.cpp`, `ui/src/lib/dspBridge.ts`

1. **C++ side**: Add an `AudioProcessorParameter::Listener` to the PluginProcessor that watches all APVTS parameters. On any change, call:
   ```cpp
   webView->evaluateJavascript(
       "window.__SYNTORTION_BRIDGE__.onParameterChange('" + paramId + "', " + String(value) + ")"
   );
   ```

2. **UI side**: On mount, register `window.__SYNTORTION_BRIDGE__` with an `onParameterChange` handler that maps APVTS ID back to UI key and calls `setState()`.

3. **Initial hydration**: When JS connects (sends `{ type: "connect" }`), C++ responds with a full parameter snapshot so the UI initializes to DAW state.

### Phase 6: Editor Cleanup
**Files**: `plugin/src/PluginEditor.cpp`, `plugin/include/Synthortion/PluginEditor.h`

Remove the entire native UI (all `juce::Label`, `juce::Slider`, `juce::ToggleButton` members). The editor becomes a thin shell:
- Constructor: create `WebBrowserComponent`, load UI, add message listener.
- `resized()`: `webView->setBounds(getLocalBounds())`.
- Message handler: parse JS messages, forward to `handleMessage()`.

### Phase 7: End-to-End Verification
1. Build UI (`npm run build`).
2. Build plugin with CMake.
3. Load in a DAW (or standalone).
4. Verify: knob turns → DSP parameter changes → audio changes.
5. Verify: DAW automation → DSP parameter changes → UI updates.
6. Verify: module power toggles → individual effects bypass.
7. Verify: preset recall → all parameters restore correctly.

## Dependency Order

```
Phase 1 (normalization) ─┐
Phase 2 (missing params) ─┤
Phase 3 (processBlock)   ─┼─→ Phase 4 (build pipeline) → Phase 7 (verify)
Phase 5 (bidirectional)  ─┘
Phase 6 (editor cleanup) ────→ (independent, can parallel Phase 4)
```

Phases 1–3 and 5 are independent of each other (all modify different files). Phase 4 depends on Phase 1 (normalization must work before we build). Phase 6 is a cleanup pass. Phase 7 verifies everything together.
