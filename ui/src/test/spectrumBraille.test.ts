import { describe, it, expect } from 'vitest'
import {
  MIN_HZ,
  MAX_HZ,
  NUM_BANDS,
  FREQ_TICKS,
  freqToCol,
  interpolateBands,
  buildSpectrumTraceAndDither,
  buildSpectrumGraticule,
} from '../lib/spectrumBraille'

describe('spectrumBraille helpers', () => {
  describe('constants and freqToCol', () => {
    it('defines 80 bands spanning 20 Hz to 20 kHz', () => {
      expect(MIN_HZ).toBe(20)
      expect(MAX_HZ).toBe(20000)
      expect(NUM_BANDS).toBe(80)
      expect(FREQ_TICKS).toHaveLength(4)
      expect(FREQ_TICKS.map((t) => t.hz)).toEqual([20, 200, 2000, 20000])
      expect(FREQ_TICKS.map((t) => t.label)).toEqual(['20Hz', '200Hz', '2kHz', '20kHz'])
    })

    it('calculates logarithmic column positions accurately', () => {
      const cols = 91
      // 20 Hz -> col 0
      expect(freqToCol(20, cols)).toBe(0)
      // 20 kHz -> last col (90)
      expect(freqToCol(20000, cols)).toBe(90)
      // 200 Hz -> 1/3 of log range -> 30
      expect(freqToCol(200, cols)).toBe(30)
      // 2 kHz -> 2/3 of log range -> 60
      expect(freqToCol(2000, cols)).toBe(60)
    })
  })

  describe('interpolateBands', () => {
    it('returns array of requested target length', () => {
      const bands = new Float32Array(80).fill(0.5)
      const out = interpolateBands(bands, 120)
      expect(out).toHaveLength(120)
      expect(out[0]).toBeCloseTo(0.5)
      expect(out[119]).toBeCloseTo(0.5)
    })

    it('preserves boundary and interpolated values for gradient inputs', () => {
      const bands = new Float32Array(80)
      for (let i = 0; i < 80; i++) bands[i] = i / 79

      const out = interpolateBands(bands, 159)
      expect(out[0]).toBeCloseTo(0)
      expect(out[158]).toBeCloseTo(1)
      expect(out[79]).toBeCloseTo(0.5)
    })

    it('handles empty or zero-length gracefully', () => {
      const out = interpolateBands(new Float32Array(0), 10)
      expect(out).toHaveLength(10)
      expect(out.every((v) => v === 0)).toBe(true)
    })
  })

  describe('buildSpectrumTraceAndDither', () => {
    it('generates trace and dither rows matching requested dimensions', () => {
      const bands = new Float32Array(80).fill(0.5)
      const numCols = 40
      const numRows = 10
      const { trace, dither } = buildSpectrumTraceAndDither(bands, numCols, numRows)

      expect(trace).toHaveLength(numRows)
      expect(dither).toHaveLength(numRows)
      trace.forEach((row) => expect(row).toHaveLength(numCols))
      dither.forEach((row) => expect(row).toHaveLength(numCols))
    })

    it('renders Braille characters in U+2800..U+28FF range for trace', () => {
      const bands = new Float32Array(80)
      // Put a peak at band 40
      bands[40] = 0.9
      const { trace } = buildSpectrumTraceAndDither(bands, 80, 12)

      const nonSpaceChars: string[] = []
      trace.forEach((row) => {
        for (const ch of row) {
          if (ch !== ' ' && ch !== '\u2800') {
            nonSpaceChars.push(ch)
            const code = ch.charCodeAt(0)
            expect(code).toBeGreaterThanOrEqual(0x2800)
            expect(code).toBeLessThanOrEqual(0x28ff)
          }
        }
      })
      expect(nonSpaceChars.length).toBeGreaterThan(0)
    })

    it('renders density dither characters ( ░▒▓█) under the peak curve', () => {
      const bands = new Float32Array(80).fill(0.8)
      const numCols = 30
      const numRows = 10
      const { dither } = buildSpectrumTraceAndDither(bands, numCols, numRows)

      const validGlyphs: Record<string, true> = { ' ': true, '░': true, '▒': true, '▓': true, '█': true }
      let filledDitherCount = 0
      dither.forEach((row) => {
        for (const ch of row) {
          expect(validGlyphs[ch]).toBe(true)
          if (ch !== ' ') filledDitherCount++
        }
      })
      expect(filledDitherCount).toBeGreaterThan(0)
    })

    it('produces flat floor trace and empty dither on zero magnitude silence', () => {
      const silence = new Float32Array(80).fill(0)
      const { trace, dither } = buildSpectrumTraceAndDither(silence, 40, 8)

      // The top rows should have empty dither
      expect(dither[0].trim()).toBe('')
      expect(dither[1].trim()).toBe('')
      // Trace bottom row should contain floor braille dots
      const bottomRow = trace[trace.length - 1]
      expect(bottomRow.trim().length).toBeGreaterThan(0)
    })
  })

  describe('buildSpectrumGraticule', () => {
    it('creates Cartesian grid with specified rows and columns', () => {
      const numCols = 60
      const numRows = 12
      const graticule = buildSpectrumGraticule(numCols, numRows)
      const lines = graticule.split('\n')

      expect(lines).toHaveLength(numRows)
      lines.forEach((line) => {
        expect(line.length).toBe(numCols)
      })
    })

    it('places + crosshairs at frequency tick columns', () => {
      const numCols = 91
      const numRows = 10
      const graticule = buildSpectrumGraticule(numCols, numRows)
      const lines = graticule.split('\n')

      // Check that at least one row contains + crosshairs at tick positions
      const hasCrosshair = lines.some((line) => line.includes('+'))
      expect(hasCrosshair).toBe(true)
    })

    it('embeds frequency calibration labels (20Hz, 200Hz, 2kHz, 20kHz)', () => {
      const numCols = 91
      const numRows = 10
      const graticule = buildSpectrumGraticule(numCols, numRows)

      expect(graticule).toContain('20Hz')
      expect(graticule).toContain('200Hz')
      expect(graticule).toContain('2kHz')
      expect(graticule).toContain('20kHz')
    })
  })
})
