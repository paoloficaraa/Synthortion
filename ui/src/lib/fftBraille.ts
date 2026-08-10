/**
 * Pure string builders for the braille oscilloscope band.
 *
 * `buildGraticule` and `buildTrace` turn grid geometry and time-domain
 * samples into prebuilt text (box-drawing frame + braille waveform) that the
 * canvas draws via a single `fillText` per frame. Kept free of React/canvas so
 * the layout maths is unit-testable without a rendering context.
 */

/** Grid cell size in CSS pixels — tiles Px437 8x16 at 16px font-size. */
export const CELL_PX = 16

/** Vertical grid rows in the 240px band. */
export const CELL_ROWS = 15

/** Vertical dot positions (4 per cell row). */
export const DOT_ROWS = CELL_ROWS * 4

/**
 * Braille bit tables — left (dots 1,2,3,7) and right (dots 4,5,6,8). The bit
 * value selects the U+2800 offset for a lit dot within a braille cell.
 */
const DOT_BITS: readonly [readonly number[], readonly number[]] = [
  [0x01, 0x02, 0x04, 0x40], // left column
  [0x08, 0x10, 0x20, 0x80], // right column
] as const

/**
 * Prebuild the graticule string (box-drawing frame + frequency labels).
 * Called once per canvas width (and on resize). Static — never glitches.
 */
export function buildGraticule(numCols: number): string {
  const LABELS = ['20Hz', '200Hz', '2kHz', '20kHz']
  const NUM_DIVS = 3
  const spacing = Math.floor((numCols - 1) / (NUM_DIVS + 1))
  const divs = Array.from({ length: NUM_DIVS }, (_, i) => spacing * (i + 1))

  const rows: string[] = []

  // ── Top border with frequency labels ──
  {
    let top = '┌'
    for (let d = 0; d <= NUM_DIVS; d++) {
      const label = LABELS[d]
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
    rows.push(top)
  }

  // ── Middle rows (1–13) ──
  for (let r = 1; r < CELL_ROWS - 1; r++) {
    let row = '│'
    for (let d = 0; d < NUM_DIVS; d++) {
      const segLen = divs[d] - (d === 0 ? 1 : divs[d - 1]) - 1
      row += ' '.repeat(Math.max(0, segLen))
      row += '│'
    }
    row += ' '.repeat(Math.max(0, numCols - divs[NUM_DIVS - 1] - 2))
    row += '│'
    rows.push(row)
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
    rows.push(bottom)
  }

  return rows.join('\n')
}

/**
 * Build the braille trace strings from time-domain samples.
 * Returns CELL_ROWS strings of numCols braille characters (or spaces).
 * Each row is drawn individually via fillText(row, 0, i * CELL_PX).
 */
export function buildTrace(
  samples: Float32Array,
  numCols: number,
): string[] {
  // Accumulate bitmask per (cellRow, col).
  const masks = Array.from({ length: CELL_ROWS }, () =>
    new Uint8Array(numCols),
  )

  for (let c = 0; c < numCols; c++) {
    for (let s = 0; s < 2; s++) {
      // Map sub-column to a sample index.
      const x = (c * 2 + s) / (numCols * 2 - 1)
      const idx = Math.min(
        Math.round(x * (samples.length - 1)),
        samples.length - 1,
      )
      const y = samples[idx] // [-1, 1]

      // y = +1 → top (absRow 0), y = -1 → bottom (absRow DOT_ROWS-1)
      const absRow = Math.round(((1 - y) * (DOT_ROWS - 1)) / 2)
      const cellRow = Math.floor(absRow / 4)
      const dotInCell = absRow % 4

      if (cellRow >= 0 && cellRow < CELL_ROWS) {
        masks[cellRow][c] |= DOT_BITS[s][dotInCell]
      }
    }
  }

  return masks.map((row) =>
    Array.from(row, (b) =>
      b === 0 ? ' ' : String.fromCharCode(0x2800 + b),
    ).join(''),
  )
}
