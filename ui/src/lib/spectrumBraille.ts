/**
 * Spectrum Analyzer Braille, Halftone Dither, and Cartesian Graticule Engine.
 *
 * Provides pure mathematical mappings and ASCII/Braille string builders
 * for the 80-band real-time 20 Hz – 20 kHz Post-FX spectrum analyzer.
 */

/** Lowest analyzed frequency (Hz). */
export const MIN_HZ = 20

/** Highest analyzed frequency (Hz). */
export const MAX_HZ = 20_000

/** Number of log-spaced magnitude bands streamed from the C++ DSP engine. */
export const NUM_BANDS = 80

/** Standard grid cell height in CSS pixels. */
export const CELL_PX = 16

/** Density ramp for halftone dither gradient under the curve, low -> high density. */
export const DITHER_RAMP = [' ', '░', '▒', '▓', '█'] as const

/** Braille bitmasks for dot positions in a 2x4 cell matrix. */
const LEFT_DOTS: readonly number[] = [0x01, 0x02, 0x04, 0x40]
const RIGHT_DOTS: readonly number[] = [0x08, 0x10, 0x20, 0x80]

/** Frequency calibration ticks across the audio spectrum. */
export const FREQ_TICKS = [
  { hz: MIN_HZ, label: '20Hz' },
  { hz: 200, label: '200Hz' },
  { hz: 2000, label: '2kHz' },
  { hz: MAX_HZ, label: '20kHz' },
] as const

/**
 * Calculate the 0-indexed column corresponding to a given frequency in Hz
 * on a logarithmic scale between MIN_HZ (20 Hz) and MAX_HZ (20 kHz).
 */
export function freqToCol(hz: number, cols: number): number {
  if (cols <= 1) return 0
  const clampedHz = Math.max(MIN_HZ, Math.min(MAX_HZ, hz))
  const ratio = Math.log(clampedHz / MIN_HZ) / Math.log(MAX_HZ / MIN_HZ)
  return Math.round(ratio * (cols - 1))
}

/**
 * Interpolate an array of magnitude values to an arbitrary target length
 * using sub-bin linear interpolation.
 */
export function interpolateBands(
  bands: Float32Array | readonly number[] | number[],
  targetLength: number,
): Float32Array {
  const result = new Float32Array(targetLength)
  if (targetLength <= 0 || bands.length === 0) return result

  if (bands.length === 1) {
    result.fill(bands[0])
    return result
  }

  const srcLen = bands.length
  const step = (srcLen - 1) / Math.max(1, targetLength - 1)

  for (let i = 0; i < targetLength; i++) {
    const srcPos = i * step
    const i0 = Math.floor(srcPos)
    const i1 = Math.min(srcLen - 1, i0 + 1)
    const frac = srcPos - i0
    const v0 = bands[i0] ?? 0
    const v1 = bands[i1] ?? v0
    result[i] = (1 - frac) * v0 + frac * v1
  }

  return result
}

/** Result of generating spectrum braille trace and halftone dither. */
export interface SpectrumTraceAndDither {
  /** Stark white Braille peak contour (U+2800..U+28FF) row by row. */
  trace: string[]
  /** Warm grey halftone dither fill ( ░▒▓█) row by row. */
  dither: string[]
}

/**
 * Build both the Braille peak trace and the halftone dither gradient fill
 * for the spectrum analyzer given normalized magnitudes [0.0, 1.0].
 *
 * @param magnitudes 80-band normalized float array.
 * @param numCols Character columns on the canvas.
 * @param numRows Character rows on the canvas.
 */
export function buildSpectrumTraceAndDither(
  magnitudes: Float32Array | readonly number[] | number[],
  numCols: number,
  numRows: number,
): SpectrumTraceAndDither {
  const safeCols = Math.max(1, numCols)
  const safeRows = Math.max(1, numRows)
  const totalDotRows = safeRows * 4

  // Sample magnitudes at 2x sub-column resolution (left dot col and right dot col)
  const subCols = safeCols * 2
  const subMags = interpolateBands(magnitudes, subCols)

  const traceGrid: string[][] = Array.from({ length: safeRows }, () =>
    new Array<string>(safeCols).fill(' '),
  )
  const ditherGrid: string[][] = Array.from({ length: safeRows }, () =>
    new Array<string>(safeCols).fill(' '),
  )

  for (let c = 0; c < safeCols; c++) {
    const leftMag = Math.max(0, Math.min(1, subMags[c * 2] ?? 0))
    const rightMag = Math.max(0, Math.min(1, subMags[c * 2 + 1] ?? leftMag))

    // Dot Y coordinate: 0 at the top, totalDotRows - 1 at the bottom
    const yLeft = Math.max(
      0,
      Math.min(totalDotRows - 1, Math.round((1 - leftMag) * (totalDotRows - 1))),
    )
    const yRight = Math.max(
      0,
      Math.min(totalDotRows - 1, Math.round((1 - rightMag) * (totalDotRows - 1))),
    )

    const rowLeft = Math.floor(yLeft / 4)
    const rowRight = Math.floor(yRight / 4)
    const peakRow = Math.min(rowLeft, rowRight)

    // 1. Render Braille peak contour (U+2800..U+28FF)
    for (let r = 0; r < safeRows; r++) {
      let mask = 0
      if (r === rowLeft) {
        mask |= LEFT_DOTS[yLeft % 4]
      }
      if (r === rowRight) {
        mask |= RIGHT_DOTS[yRight % 4]
      }

      if (mask > 0) {
        traceGrid[r][c] = String.fromCharCode(0x2800 | mask)
      }
    }

    // 2. Render halftone dither gradient fill ( ░▒▓█) under the peak curve
    const maxMag = Math.max(leftMag, rightMag)
    if (maxMag > 0.005) {
      for (let r = peakRow + 1; r < safeRows; r++) {
        const span = Math.max(1, safeRows - 1 - peakRow)
        const depth = (r - peakRow) / span
        // Map depth under peak to density: '░', '▒', '▓', '█'
        const rampIdx = Math.min(
          DITHER_RAMP.length - 1,
          Math.max(1, Math.floor(depth * (DITHER_RAMP.length - 1)) + 1),
        )
        ditherGrid[r][c] = DITHER_RAMP[rampIdx]
      }
    }
  }

  return {
    trace: traceGrid.map((row) => row.join('')),
    dither: ditherGrid.map((row) => row.join('')),
  }
}

/**
 * Prebuild Cartesian graticule for the spectrum analyzer featuring
 * 1px Cartesian hairlines, crosshairs (+) at frequency tick columns,
 * and calibration labels at 20 Hz, 200 Hz, 2 kHz, and 20 kHz.
 */
export function buildSpectrumGraticule(numCols: number, numRows: number): string {
  const safeCols = Math.max(8, numCols)
  const safeRows = Math.max(3, numRows)

  const tickCols = FREQ_TICKS.map((t) => freqToCol(t.hz, safeCols))
  const lines: string[] = []

  // Middle horizontal reference lines (e.g., -20 dB, -40 dB levels)
  const midRow1 = Math.floor(safeRows * 0.33)
  const midRow2 = Math.floor(safeRows * 0.66)
  const bottomRow = safeRows - 1

  for (let r = 0; r < safeRows; r++) {
    const chars = new Array<string>(safeCols).fill(' ')

    if (r === 0) {
      // Top hairline with crosshairs
      chars.fill('─')
      chars[0] = '┌'
      chars[safeCols - 1] = '┐'
      tickCols.forEach((c) => {
        if (c > 0 && c < safeCols - 1) chars[c] = '+'
      })
    } else if (r === midRow1 || r === midRow2) {
      // Horizontal reference level hairlines with crosshairs at frequency columns
      chars.fill('─')
      chars[0] = '├'
      chars[safeCols - 1] = '┤'
      tickCols.forEach((c) => {
        if (c > 0 && c < safeCols - 1) chars[c] = '+'
      })
    } else if (r === bottomRow) {
      // Bottom calibration baseline with ticks and labels
      chars.fill('─')
      chars[0] = '└'
      chars[safeCols - 1] = '┘'

      for (let i = 0; i < FREQ_TICKS.length; i++) {
        const { hz, label } = FREQ_TICKS[i]
        const col = freqToCol(hz, safeCols)

        if (i === FREQ_TICKS.length - 1) {
          // Rightmost label (20kHz) placed left of tick so boundary stays clean
          const start = Math.max(1, col - label.length)
          for (let j = 0; j < label.length; j++) chars[start + j] = label[j]
          chars[col] = '+'
        } else {
          chars[col] = '+'
          const start = col + 1
          if (start + label.length < safeCols - 1) {
            for (let j = 0; j < label.length; j++) chars[start + j] = label[j]
          }
        }
      }
    } else {
      // Background row with subtle vertical frequency ticks
      chars[0] = '│'
      chars[safeCols - 1] = '│'
      tickCols.forEach((c) => {
        if (c > 0 && c < safeCols - 1) chars[c] = '·'
      })
    }

    lines.push(chars.join(''))
  }

  return lines.join('\n')
}
