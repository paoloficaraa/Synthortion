/**
 * Simulated time-domain oscilloscope signal generator.
 *
 * Replaces the prototype's structural-noise FFT generator with a pure,
 * renderer-independent waveform source for the 2D canvas oscilloscope. The
 * signal is a stack of detuned sinusoids (a musical-style tone) advanced one
 * frame at a time by an injectable phase step — no WebAudio or DSP backend,
 * matching the issue's out-of-scope note.
 */

/** Number of time-domain samples per frame — the trace resolution. */
export const SAMPLE_COUNT = 240

/** Phase advance per frame from the prototype's `phase += 0.12`. */
export const PHASE_STEP = 0.12

/** Attack/decay smoothing coefficient applied to each sample per frame. */
export const SMOOTHING = 0.4

/** Peak-to-peak amplitude of the injected sample noise (± half this). */
export const NOISE_AMOUNT = 0.04

/**
 * Time-domain waveform oscillator.
 *
 * A fundamental plus a detuned overtone, so the trace reads as a living
 * musical tone rather than a bare sine sweep. The result stays within roughly
 * [-1, 1]; active samples are clamped in `step`.
 */
export function waveformAt(phase: number, index: number): number {
  return (
    Math.sin(phase + index * 0.2) * 0.6 +
    Math.cos(phase * 1.5 - index * 0.1) * 0.4
  )
}

/**
 * Raw per-sample target for one frame.
 *
 * Active engines add a small random jitter to the waveform so the trace has a
 * subtle analogue shimmer. Bypassed engines collapse every sample to a flat
 * idle line of 0 so the trace decays to the centre graticule.
 */
export function rawSample(
  index: number,
  phase: number,
  active: boolean,
  random: () => number
): number {
  if (!active) return 0
  return waveformAt(phase, index) + (random() - 0.5) * NOISE_AMOUNT
}

/**
 * Live time-domain signal generator.
 *
 * Holds the current frame's samples and advances them one frame at a time.
 * The random source is injectable so tests can feed deterministic mock frames
 * without a canvas or a DSP backend (acceptance criterion: isolated visual
 * logic).
 */
export interface OscilloscopeSignal {
  /** Current time-domain samples, normalised to roughly [-1, 1]. */
  readonly samples: Float32Array
  /** Advance the simulation one frame; a bypassed engine decays to idle. */
  step(active: boolean): void
}

/** Builds a fresh signal generator sharing the prototype's initial state. */
export function createOscilloscopeSignal(
  random: () => number = Math.random
): OscilloscopeSignal {
  const samples = new Float32Array(SAMPLE_COUNT)
  let phase = 0

  return {
    get samples() {
      return samples
    },
    step(active) {
      for (let i = 0; i < SAMPLE_COUNT; i++) {
        const target = Math.max(
          -1,
          Math.min(1, rawSample(i, phase, active, random))
        )
        samples[i] += (target - samples[i]) * SMOOTHING
      }
      phase += PHASE_STEP
    },
  }
}
