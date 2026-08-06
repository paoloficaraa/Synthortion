# Synthortion — Context

## Terms

- **Synthortion** — the VST plugin product. Its UI is a React web app (`ui/`) that runs inside a host WebView; the DSP engine is a future C++ bridge behind the `DspBridge` seam.
- **Faceplate** — the main control surface (the center hub of the UI) that exposes every plugin parameter.
- **Digital instrument** — the aesthetic direction of the UI: flat, precise, digital (luma-inspired). Explicitly *not* skeuomorphic hardware: no metal gradients, rack screws, bezels, or recessed-well shadows.
- **ASCII surface** — any information display rendered in character glyphs (block-element meters, braille waveform, VGA-style readouts, box-drawing frames) rather than vector primitives. The voice of the terminal inside the digital instrument.
- **Deadlock** — the legacy visual identity (dark industrial & glitch brutalism). Referenced as a stylistic ancestor, not the current direction.
- **Monochrome discipline** — the UI stays high-contrast white/gray/black (Deadlock identity). Saturated color accents are deferred, not part of the current direction; only the existing warm-grey `--accent` and `--danger` red remain.
- **Module** — a processing unit on the faceplate, each with its own power state: **Drive** (DRV), **Bitcrush** (BCR), **Delay** (DLY), **Chorus** (CHR).
- **Drive** — the distortion stage of the plugin (colloquially "warm distortion"); the canonical module name is Drive, not Warm Distortion.
- **Power** — the per-module on/off state (each module can be powered independently, alongside the master `engineActive` bypass).
- **Tweak-driven glitch** — the signature behavior of the visualizer: the scope's ASCII field corrupts proportionally to live parameter-interaction intensity (drag velocity × amplitude) and decays in ~300–500ms. At rest the trace is clean; module power toggles fire a short burst; bypass fires one heavy corruption frame. No dedicated glitch control exists on the faceplate.
- **Boot sequence** — the mount-time terminal overlay: one-shot, staged lines with real values and aligned `[ OK ]` columns, ending on `[READY]`. Skippable by click/Enter; reduced-motion renders the final state immediately.
