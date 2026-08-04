/**
 * Simulated structural-noise FFT signal generator.
 *
 * Ports the prototype's inline math (vst-interface.html) into a pure,
 * renderer-independent module so the visualizer can be driven by mock frames
 * in tests and by a future DSP bridge in production — no WebAudio FFT is
 * involved, matching the issue's out-of-scope note.
 */

/** Number of spectrum bins, matching the prototype's `new Float32Array(128)`. */
export const BIN_COUNT = 128

/** Number of bars the prototype renders across the spectrum. */
export const BAR_COUNT = 100

/** Low-mid "bass" boost band, ported from the prototype's `i > 10 && i < 30`. */
export const BOOST_START = 10
export const BOOST_END = 30 // exclusive
export const BOOST_AMOUNT = 20

/** Attack/release smoothing coefficient from the prototype's `* 0.15`. */
export const SMOOTHING = 0.15

/** Phase advance per frame from the prototype's `phase += 0.1`. */
export const PHASE_STEP = 0.1

/**
 * Prototype structural-noise oscillator.
 *
 * Two detuned sinusoids sweep the spectrum, which is what makes the simulated
 * FFT feel alive without a real analyser node.
 */
export function structuralNoise(bin: number, phase: number): number {
  return Math.sin(phase + bin * 0.2) * 5 + Math.cos(phase * 1.5 - bin * 0.1) * 10
}

/** Prototype low-mid "bass" boost applied to bins 11..29. */
export function boostFor(bin: number): number {
  return bin > BOOST_START && bin < BOOST_END ? BOOST_AMOUNT : 0
}

/**
 * Raw per-bin target for one frame.
 *
 * Active engines combine a random jitter with the structural noise, the bass
 * boost and a +10 offset, exactly as the prototype does. Bypassed engines
 * collapse every bin to a floor of 1 so bars decay to an idle stub.
 */
export function rawTarget(
  bin: number,
  phase: number,
  active: boolean,
  random: () => number
): number {
  if (!active) return 1
  return random() * 20 + structuralNoise(bin, phase) + boostFor(bin) + 10
}

/**
 * Live structural-noise signal generator.
 *
 * Holds per-bin levels and advances them one frame at a time. The random
 * source is injectable so tests can feed deterministic mock frames without a
 * canvas or a DSP backend (acceptance criterion: isolated visual logic).
 */
export interface FftSignal {
  /** Current per-bin levels on the prototype's ~0..100 scale. */
  readonly bins: Float32Array
  /** Advance the simulation one frame; a bypassed engine decays to idle. */
  step(active: boolean): void
}

/** Builds a fresh signal generator sharing the prototype's initial state. */
export function createFftSignal(random: () => number = Math.random): FftSignal {
  const bins = new Float32Array(BIN_COUNT)
  let phase = 0

  return {
    get bins() {
      return bins
    },
    step(active) {
      for (let i = 0; i < BAR_COUNT; i++) {
        const target = Math.max(1, rawTarget(i, phase, active, random))
        bins[i] += (target - bins[i]) * SMOOTHING
      }
      phase += PHASE_STEP
    },
  }
}
