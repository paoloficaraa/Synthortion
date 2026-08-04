/**
 * Simulated time-domain oscilloscope signal generator.
 *
 * Produces a frame of waveform samples that an oscilloscope can sweep across
 * the canvas. Multiple detuned sinusoids plus random jitter mimic a real
 * analogue scope trace — no WebAudio or analyser node involved.
 */

/** Number of samples per frame (horizontal resolution of the oscilloscope). */
export const SAMPLE_COUNT = 512

/** Amplitude scale — half the canvas height in normalised -1..1 space. */
export const AMPLITUDE = 0.7

/** Attack/release smoothing coefficient. */
export const SMOOTHING = 0.15

/** Phase advance per frame. */
export const PHASE_STEP = 0.08

/**
 * Raw waveform sample for a given index within one frame.
 *
 * The trace is the sum of three detuned sinusoids plus a noise term, giving
 * the classic "analogue scope" look with slight irregularity.
 */
export function rawSample(
  index: number,
  phase: number,
  active: boolean,
  random: () => number,
): number {
  if (!active) return 0
  const t = index / SAMPLE_COUNT
  return (
    Math.sin(phase + t * Math.PI * 4) * 0.4 +
    Math.sin(phase * 1.7 + t * Math.PI * 7.3) * 0.25 +
    Math.cos(phase * 0.6 - t * Math.PI * 2.1) * 0.15 +
    (random() - 0.5) * 0.08
  ) * AMPLITUDE
}

/**
 * Live time-domain signal generator.
 *
 * Holds a circular sample buffer and advances it one frame at a time. The
 * random source is injectable so tests can feed deterministic frames.
 */
export interface TimeSignal {
  /** Current frame of samples in the -1..1 normalised range. */
  readonly samples: Float32Array
  /** Advance the simulation one frame; bypassed engines decay to a flat line. */
  step(active: boolean): void
}

/** Builds a fresh signal generator. */
export function createTimeSignal(random: () => number = Math.random): TimeSignal {
  const samples = new Float32Array(SAMPLE_COUNT)
  const smoothed = new Float32Array(SAMPLE_COUNT)
  let phase = 0

  return {
    get samples() {
      return samples
    },
    step(active) {
      for (let i = 0; i < SAMPLE_COUNT; i++) {
        const target = rawSample(i, phase, active, random)
        smoothed[i] += (target - smoothed[i]) * SMOOTHING
        samples[i] = smoothed[i]
      }
      phase += PHASE_STEP
    },
  }
}
