/**
 * Pure string builders for the braille oscilloscope band.
 *
 * `buildGraticule` and `buildTrace` turn grid geometry and time-domain
 * samples into prebuilt text (box-drawing frame + braille waveform) that the
 * canvas draws row-by-row — one `fillText(row, 0, i * CELL_PX)` per line,
 * since Canvas `fillText` ignores embedded newlines. Kept free of
 * React/canvas so the layout maths is unit-testable without a rendering
 * context.
 */

/** Grid cell size in CSS pixels — tiles Px437 8x16 at 16px font-size. */
export const CELL_PX = 16

/** Vertical grid rows in the 240px band. */
export const CELL_ROWS = 15

/** Vertical dot positions (4 per cell row) at the default band height. */
export const DOT_ROWS = CELL_ROWS * 4

/**
 * Braille bit tables — left (dots 1,2,3,7) and right (dots 4,5,6,8). The bit
 * value selects the U+2800 offset for a lit dot within a braille cell.
 */
const DOT_BITS: readonly [readonly number[], readonly number[]] = [
  [0x01, 0x02, 0x04, 0x40], // left column
  [0x08, 0x10, 0x20, 0x80], // right column
] as const

/** Faint glyph used for sub-pixel dither positions that miss a braille dot. */
export const DITHER_GLYPH = '░'

/**
 * Prebuild the graticule string (box-drawing frame with Cartesian crosshairs
 * at the centre-row intersections, plus optional calibration labels embedded
 * in the top border). Called once per canvas width (and on resize). Static —
 * never glitches.
 */
export function buildGraticule(
  numCols: number,
  rows: number = CELL_ROWS,
  labels: readonly string[] = [],
): string {
  const NUM_DIVS = 3
  const spacing = Math.floor((numCols - 1) / (NUM_DIVS + 1))
  const divs = Array.from({ length: NUM_DIVS }, (_, i) => spacing * (i + 1))
  const midRow = Math.floor(rows / 2)

  const out: string[] = []

  // ── Top border with optional calibration labels ──
  {
    let top = '┌'
    for (let d = 0; d <= NUM_DIVS; d++) {
      const label = labels[d]
      const segStart = d === 0 ? 1 : divs[d - 1] + 1
      const segEnd = d < NUM_DIVS ? divs[d] : numCols - 1
      const segLen = segEnd - segStart
      const seg = '─'.repeat(Math.max(0, segLen))

      if (label && segLen > label.length + 2) {
        const pad = Math.floor((segLen - label.length) / 2)
        top += '─'.repeat(pad) + label + '─'.repeat(segLen - pad - label.length)
      } else {
        top += seg
      }
      top += d < NUM_DIVS ? '┬' : '┐'
    }
    out.push(top)
  }

  // ── Middle rows; the centre row crosses divisions with '+' crosshairs ──
  for (let r = 1; r < rows - 1; r++) {
    const isMid = r === midRow
    let row = isMid ? '+' : '│'
    for (let d = 0; d < NUM_DIVS; d++) {
      const segLen = divs[d] - (d === 0 ? 1 : divs[d - 1]) - 1
      row += (isMid ? '─' : ' ').repeat(Math.max(0, segLen))
      row += isMid ? '+' : '│'
    }
    row += (isMid ? '─' : ' ').repeat(Math.max(0, numCols - divs[NUM_DIVS - 1] - 2))
    row += isMid ? '+' : '│'
    out.push(row)
  }

  // ── Bottom border ──
  {
    let bottom = '└'
    for (let d = 0; d <= NUM_DIVS; d++) {
      const segStart = d === 0 ? 1 : divs[d - 1] + 1
      const segEnd = d < NUM_DIVS ? divs[d] : numCols - 1
      bottom += '─'.repeat(Math.max(0, segEnd - segStart))
      bottom += d < NUM_DIVS ? '┴' : '┘'
    }
    out.push(bottom)
  }

  return out.join('\n')
}

/**
 * Accumulate braille bitmasks for the trace. Shared by `buildTrace` and
 * `buildDither`; returns one bitmask per cell row, plus the exact fractional
 * dot positions so the dither pass can anti-alias near-misses.
 */
function traceMasks(
  samples: Float32Array,
  numCols: number,
  rows: number,
): { masks: Uint8Array[]; exact: Float32Array } {
  const dotRows = rows * 4
  const masks = Array.from({ length: rows }, () => new Uint8Array(numCols))
  const exact = new Float32Array(numCols * 2)

  for (let c = 0; c < numCols; c++) {
    for (let s = 0; s < 2; s++) {
      // Map sub-column to a sample index.
      const x = (c * 2 + s) / (numCols * 2 - 1)
      const idx = Math.min(
        Math.round(x * (samples.length - 1)),
        samples.length - 1,
      )
      const y = samples[idx] // [-1, 1]

      // y = +1 → top (absRow 0), y = -1 → bottom (absRow dotRows-1)
      const absExact = ((1 - y) * (dotRows - 1)) / 2
      exact[c * 2 + s] = absExact
      const absRow = Math.round(absExact)
      const cellRow = Math.floor(absRow / 4)
      const dotInCell = absRow % 4

      if (cellRow >= 0 && cellRow < rows) {
        masks[cellRow][c] |= DOT_BITS[s][dotInCell]
      }
    }
  }
  return { masks, exact }
}

/**
 * Build the braille trace strings from time-domain samples.
 * Returns `rows` strings of numCols braille characters (or spaces).
 * Each row is drawn individually via fillText(row, 0, i * CELL_PX).
 */
export function buildTrace(
  samples: Float32Array,
  numCols: number,
  rows: number = CELL_ROWS,
): string[] {
  return traceMasks(samples, numCols, rows).masks.map((row) =>
    Array.from(row, (b) => (b === 0 ? ' ' : String.fromCharCode(0x2800 + b))).join(''),
  )
}

/**
 * Sub-pixel dither overlay: positions whose exact fractional dot lands more
 * than a quarter-dot away from a lit braille dot get a faint dither glyph in
 * the neighbouring dot's cell, anti-aliasing the trace. Returns `rows`
 * strings aligned with `buildTrace`'s output; draw dimmed underneath it.
 */
export function buildDither(
  samples: Float32Array,
  numCols: number,
  rows: number = CELL_ROWS,
): string[] {
  const { masks, exact } = traceMasks(samples, numCols, rows)
  const dither = Array.from({ length: rows }, () => new Array<string>(numCols).fill(' '))

  for (let c = 0; c < numCols; c++) {
    for (let s = 0; s < 2; s++) {
      const absExact = exact[c * 2 + s]
      const frac = absExact - Math.round(absExact)
      if (Math.abs(frac) <= 0.25) continue

      // Shade the dot the trace slipped past (toward the fraction's sign).
      const absRow = Math.round(absExact) + Math.sign(frac)
      const cellRow = Math.floor(absRow / 4)
      const dotInCell = ((absRow % 4) + 4) % 4
      if (cellRow < 0 || cellRow >= rows) continue
      if (masks[cellRow][c] & DOT_BITS[s][dotInCell]) continue
      dither[cellRow][c] = DITHER_GLYPH
    }
  }
  return dither.map((row) => row.join(''))
}
