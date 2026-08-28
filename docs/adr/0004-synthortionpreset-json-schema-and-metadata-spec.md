# ADR 0004: `.synthortionpreset` JSON Schema & Metadata Specification

**Status:** Accepted  
**Date:** 2026-08-28  

## Context

Synthortion requires a standardized, portable, human-readable file format (`.synthortionpreset`) for saving, loading, sharing, and embedding factory presets.

Previously:
- DAW session state was serialized to binary XML via APVTS (ADR 0003), but no standardized user-facing or embedded preset file format existed.
- Parameter values in the TypeScript UI and C++ DSP exist at different abstraction levels (user units vs normalized floats).
- Metadata (category taxonomy, author, description, tags, timestamps) needed a formal schema separate from audio DSP parameters.

## Decision

### 1. Document Structure & Top-Level Hierarchy
The `.synthortionpreset` format is a strict UTF-8 JSON document with modular top-level sections:

```json
{
  "$schema": "https://synthortion.audio/schemas/preset.v1.json",
  "schemaVersion": 1,
  "metadata": {
    "name": "Cyber Acid Lead",
    "category": "Lead",
    "author": "Synthortion Core",
    "description": "Aggressive resonant overdrive with ping-pong tape flutter.",
    "tags": ["Lead", "Acid", "Distortion", "Cyberpunk"],
    "favorite": false,
    "createdAt": "2026-08-28T12:00:00Z",
    "modifiedAt": "2026-08-28T12:00:00Z"
  },
  "parameters": {
    "INPUT_GAIN": 0.833333,
    "OUTPUT_GAIN": 0.833333,
    "COLOR": 0.65,
    "BITCRUSH": 0.25,
    "DELAY_TIME_FREE": 0.124562,
    "DELAY_TIME_SYNC": 0.307692,
    "DELAY_MIX": 0.35,
    "DELAY_FEEDBACK": 0.45,
    "CHORUS_MIX": 0.20,
    "CHORUS_WIDE": 0.75,
    "PLUGIN_BYPASS": 0.0,
    "DRIVE_ON": 1.0,
    "BITCRUSH_ON": 1.0,
    "DELAY_ON": 1.0,
    "CHORUS_ON": 1.0,
    "DRIVE_ROUTE": 0.0,
    "DELAY_SYNC": 1.0
  },
  "uiPreferences": {
    "uiScale": 1.0,
    "spectrumDecay": 0.25,
    "skipBootSequence": false
  }
}
```

### 2. Section Specifications

#### `metadata` (Required)
- `name` (string, required): 1–32 characters, sanitized against illegal OS filename characters (`\ / : * ? " < > |`).
- `category` (string, required): Matches standard category subfolder (`Bass`, `Lead`, `Lo-Fi`, `Pad`, `Pluck`, `FX`, `Experimental`, `Init`, or custom user subfolder).
- `author` (string, optional): Author attribution (defaults to `"User"` if omitted).
- `description` (string, optional): Prose summary (defaults to `""`).
- `tags` (string[], optional): Max 8 string tags for indexing and search filtering.
- `favorite` (boolean, optional): User bookmark flag (defaults to `false`).
- `createdAt` (ISO 8601 string, optional): Initial creation timestamp.
- `modifiedAt` (ISO 8601 string, optional): Last modified timestamp.

#### `parameters` (Required)
- Maps canonical APVTS Parameter IDs directly to **normalized `[0.0, 1.0]` floating-point numbers**.
- Booleans are serialized as `0.0` or `1.0`.
- Choice parameters (e.g. `DELAY_TIME_SYNC`) are serialized as normalized index fractions `index / (numChoices - 1)`.
- Eliminates skew calculation mismatch and unit string drift between UI and DSP.

#### `uiPreferences` (Optional)
- Non-DSP visual preferences (`uiScale`, `spectrumDecay`, `skipBootSequence`).
- When loading presets, `uiPreferences` is applied without disrupting global interface window scaling.

### 3. Compatibility & Resilience Rules
- **Missing Parameters**: Any parameter missing from a loaded preset retains its APVTS default value from `createParameterLayout()`.
- **Unknown Parameters**: Unrecognized keys in `parameters` or root are silently ignored to maintain forward compatibility.
- **Value Clamping**: All normalized parameter values are clamped to `[0.0, 1.0]` upon deserialization.
- **Atomic Disk Serialization**: User presets written to disk use `juce::TemporaryFile::overwriteTargetFileWithTemporary()` to guarantee atomic writes.

## Consequences

### Positive
- Direct 1:1 mapping with APVTS host automation bus and JUCE 8 native event bridge.
- Fast metadata extraction without needing to instantiate DSP or parse full parameter trees.
- Resilient to schema evolution across plugin versions.

### Negative / Trade-offs
- Preset JSON is slightly less human-editable due to normalized floats rather than unit values (e.g. `0.833333` vs `0.0 dB`).
