# ADR 0003: APVTS State Serialization & UI Preference ValueTree Schema

**Status:** Accepted  
**Date:** 2026-08-24  

## Context

`AudioPluginAudioProcessor::getStateInformation` and `setStateInformation` are responsible for persisting and restoring plugin state across DAW sessions, preset changes, and host undo/redo operations.

In the initial implementation:
- `setStateInformation` spawned a background detached `std::thread` combined with `juce::MessageManager::callAsync`. This introduced severe race conditions, non-deterministic parameter restoration at DAW session startup, and potential use-after-free crashes if the processor was deleted while the thread ran.
- Non-DSP UI preferences (e.g., interface zoom scale, spectrum analyzer ballistics decay, boot animation preferences) had no persistence mechanism or schema definition.
- There was no protocol to synchronize restored UI preferences with the React WebView frontend upon preset loading or host undo/redo.

## Decision

### 1. Encoding Format: JUCE XML Binary
Serialize the complete `apvts.state` `juce::ValueTree` using JUCE's standard XML binary format via `juce::copyXmlToBinary` in `getStateInformation` and `juce::getXmlFromBinary` in `setStateInformation`.
- Provides resilience against reordered or missing nodes across versions.
- Standard JUCE container format with low overhead and robust parsing.

### 2. ValueTree Schema & Subtree Hierarchy
The root `juce::ValueTree` type remains `Parameters` (owned by APVTS), annotated with a `version="1"` attribute. Non-parameter UI preferences are stored in a dedicated `<UIPreferences>` child subtree inside `apvts.state`:

```xml
<Parameters version="1">
  <PARAM id="INPUT_GAIN" value="0.0"/>
  <PARAM id="COLOR" value="0.4"/>
  <PARAM id="DELAY_TIME" value="250.0"/>
  <PARAM id="DELAY_FEEDBACK" value="0.4"/>
  <!-- other APVTS parameters -->
  <UIPreferences uiScale="1.0" spectrumDecay="0.25" skipBootSequence="false"/>
</Parameters>
```

### 3. Synchronous Threading Model
Eliminate detached threads and asynchronous callbacks in `setStateInformation`:
- Parse the binary XML buffer into a `juce::XmlElement` synchronously.
- Convert XML to `juce::ValueTree` and call `apvts.replaceState(newState)` directly on the calling thread.
- `apvts.replaceState()` updates atomic parameter floats immediately and notifies APVTS listeners in a thread-safe manner without blocking the audio thread or causing teardown race conditions.

### 4. Backwards & Forwards Compatibility Rules
- **Missing `<UIPreferences>`**: If a session file or older preset lacks the `<UIPreferences>` node, fallback to factory defaults (`uiScale: 1.0`, `spectrumDecay: 0.25`, `skipBootSequence: false`).
- **Missing Parameters**: `apvts.replaceState()` automatically preserves existing/default values for any parameters omitted in the loaded state.
- **Root Tag Flexibility**: Accept `<Parameters>` and legacy `<Synthortion>` / `<SynthortionState>` root tags gracefully.
- **Version Attribute**: Defaults to `version="1"` if omitted.

### 5. UI Bridge Synchronization
- **Mount / Initial Connection**: The C++ native bridge delivers `uiPreferences` as part of the `init` handshake payload (`InitPayload` from Issue #109 / ADR 0002).
- **Runtime Preset Load & Undo/Redo**: When `setStateInformation` is invoked while the editor is active, `PluginEditor` listens to `apvts.state` changes and dispatches a `uiPreferencesChange` native event via `emitEventIfBrowserIsVisible("uiPreferencesChange", juce::var(uiPrefsObject))`.

## Consequences

### Positive
- Guaranteed deterministic and crash-free state restoration on session startup and preset loading.
- Seamless persistence of UI preferences alongside DSP parameters in a single atomic container.
- Full backwards compatibility with presets across schema revisions.
- Clean integration with the JUCE 8 native event bridge.

### Negative / Trade-offs
- UI preference additions must be registered in the schema and validated during XML parsing.
