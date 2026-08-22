/**
 * Waterfall spectrogram helpers for the visualizer's lower tier.
 *
 * `analyzeBands` turns a frame of time-domain samples into log-spaced band
 * energies (20 Hz–20 kHz, Goertzel per bin) and `createWaterfallHistory`
 * keeps a fixed-depth stack of density-glyph rows (` ░▒▓█`) for the
 * scrolling waterfall. Pure functions/classes — no React, no canvas — so the
 * layout maths is unit-testable without a rendering context.
 */

/** Density ramp for waterfall cells, quietest → loudest. */
export const WATERFALL_GLYPHS = [' ', '░', '▒', '▓', '█'] as const

/** Lowest analysed frequency (Hz). */
export const MIN_HZ = 20

/** Highest analysed frequency (Hz). */
export const MAX_HZ = 20_000

/** Simulated sample rate the time-domain frame is interpreted at (Hz). */
export const SIM_SAMPLE_RATE = 48_000

/**
 * Log-spaced bin centre frequencies between MIN_HZ and MAX_HZ (inclusive
 * endpoints). `bins < 2` collapses to a single MIN_HZ bin.
 */
export function binFrequencies(bins: number): number[] {
  if (bins < 2) return [MIN_HZ]
  const ratio = MAX_HZ / MIN_HZ
  return Array.from({ length: bins }, (_, i) =>
    MIN_HZ * Math.pow(ratio, i / (bins - 1)),
  )
}

/**
 * Magnitude of `samples` at `hz` via the Goertzel algorithm, normalised so a
 * full-scale sine at that frequency returns ≈1.
 */
function goertzel(samples: Float32Array, hz: number): number {
  const n = samples.length
  if (n === 0) return 0
  const w = (2 * Math.PI * hz) / SIM_SAMPLE_RATE
  const coeff = 2 * Math.cos(w)
  let s1 = 0
  let s2 = 0
  for (let i = 0; i < n; i++) {
    const s0 = samples[i] + coeff * s1 - s2
    s2 = s1
    s1 = s0
  }
  const power = s1 * s1 + s2 * s2 - coeff * s1 * s2
  return (2 * Math.sqrt(Math.max(0, power))) / n
}

/**
 * Analyse one frame of time-domain samples into `bins` log-spaced band
 * energies, compressed and clamped to [0, 1] for glyph mapping. Silence
 * yields all zeros.
 */
export function analyzeBands(
  samples: Float32Array,
  bins: number,
): Float32Array {
  const freqs = binFrequencies(bins)
  const out = new Float32Array(bins)
  for (let i = 0; i < bins; i++) {
    // sqrt compression lifts quiet detail out of the noise floor.
    out[i] = Math.min(1, Math.sqrt(goertzel(samples, freqs[i])))
  }
  return out
}

/**
 * Map a band energy (0–1) to a density glyph. Energies at or below the
 * silence threshold render as a blank cell.
 */
export function densityGlyph(magnitude: number): string {
  if (!(magnitude > 0.03)) return WATERFALL_GLYPHS[0]
  const idx = Math.min(
    WATERFALL_GLYPHS.length - 1,
    1 + Math.floor(magnitude * (WATERFALL_GLYPHS.length - 1)),
  )
  return WATERFALL_GLYPHS[idx]
}

/**
 * Scrolling waterfall history.
 *
 * Newest row is last; `lines()` returns oldest → newest so callers can draw
 * bottom-aligned with age-based phosphor dimming. Fixed depth: pushing a
 * `(depth + 1)`-th row drops the oldest. `clear()` empties the history so a
 * bypassed engine leaves no stale energy on screen.
 */
export interface WaterfallHistory {
  /** Append one analysed frame as a density-glyph row. */
  push(magnitudes: Float32Array): void
  /** Drop all rows (bypass/reset). */
  clear(): void
  /** Rows oldest → newest, each `width` glyphs long. */
  lines(): string[]
  /** Number of rows currently held. */
  readonly size: number
  /** Maximum retained rows. */
  readonly depth: number
}

/** Build a waterfall history holding up to `depth` rows. */
export function createWaterfallHistory(depth: number): WaterfallHistory {
  const rows: string[] = []
  return {
    push(magnitudes) {
      rows.push(Array.from(magnitudes, densityGlyph).join(''))
      if (rows.length > depth) rows.splice(0, rows.length - depth)
    },
    clear() {
      rows.length = 0
    },
    lines() {
      return rows.slice()
    },
    get size() {
      return rows.length
    },
    get depth() {
      return depth
    },
  }
}
