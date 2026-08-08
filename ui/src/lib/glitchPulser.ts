/**
 * Glitch Pulser — exponential-decay intensity controller for the
 * tweak-driven scope corruption.
 *
 * The visualizer reads `intensity` each rAF frame; the App calls `pulse(n)`
 * on every parameter mutation. The intensity decays to zero exponentially
 * with a half-life of ~150 ms, giving a300–500 ms corruption window.
 */

/** Exponential-decay time constant in ms. */
const DECAY_TAU = 150

/** Below this threshold the intensity snaps to zero. */
const MIN_THRESHOLD = 0.001

/** Characters used for glitch corruption. */
const CORRUPT_CHARS = '▓▒░█@#'

/**
 * Apply glitch corruption to braille trace rows.
 * Mutations: character corruption and row displacement — both proportional to
 * `intensity` (0–1). The graticule is a separate string and is never passed
 * here, so the frame and labels stay crisp.
 */
export function applyGlitch(
  rows: string[],
  intensity: number,
  random: () => number,
): string[] {
  if (intensity <= 0) return rows

  // 1. Character corruption — replace random braille chars with glitch glyphs.
  const corrupted = rows.map((row) => {
    const chars = [...row]
    for (let i = 0; i < chars.length; i++) {
      if (chars[i] === ' ') continue
      if (random() < intensity * 0.4) {
        chars[i] = CORRUPT_CHARS[Math.floor(random() * CORRUPT_CHARS.length)]
      }
    }
    return chars.join('')
  })

  // 2. Row displacement — shift some rows horizontally.
  const maxShift = Math.max(1, Math.round(intensity * 4))
  return corrupted.map((row) => {
    if (random() > intensity * 0.6) return row
    const shift =
      Math.floor(random() * (maxShift * 2 + 1)) - maxShift
    if (shift === 0) return row
    if (shift > 0) return ' '.repeat(shift) + row.slice(0, row.length - shift)
    return row.slice(-shift) + ' '.repeat(-shift)
  })
}

export interface GlitchPulser {
  /** Set glitch intensity (clamped 0–1). Overwrites any prior level. */
  pulse(intensity: number): void

  /** Advance decay by `dtMs` ms; returns the new intensity. */
  step(dtMs: number): number

  /** Current glitch intensity (0–1). */
  readonly intensity: number
}

/**
 * Builds a fresh glitch pulser whose intensity decays exponentially
 * toward zero after each pulse.
 */
export function createGlitchPulser(): GlitchPulser {
  let _intensity = 0

  return {
    pulse(i) {
      _intensity = Math.max(0, Math.min(1, i))
    },

    step(dtMs) {
      _intensity *= Math.exp(-dtMs / DECAY_TAU)
      if (_intensity < MIN_THRESHOLD) _intensity = 0
      return _intensity
    },

    get intensity() {
      return _intensity
    },
  }
}
