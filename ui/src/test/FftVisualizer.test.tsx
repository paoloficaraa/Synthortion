import { render, screen } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import { FftVisualizer } from '../components/FftVisualizer'
import { buildGraticule, buildTrace } from '../lib/fftBraille'
import { createOscilloscopeSignal } from '../lib/oscilloscopeSignal'
import { applyGlitch, createGlitchPulser } from '../lib/glitchPulser'

/* ------------------------------------------------------------------ */
/*  buildGraticule unit tests (pure string output, no canvas needed)   */
/* ------------------------------------------------------------------ */
describe('buildGraticule', () => {
  it('contains all four frequency labels', () => {
    const grat = buildGraticule(80)
    expect(grat).toContain('20Hz')
    expect(grat).toContain('200Hz')
    expect(grat).toContain('2kHz')
    expect(grat).toContain('20kHz')
  })

  it('draws a box-drawing frame with corner brackets', () => {
    const grat = buildGraticule(80)
    const rows = grat.split('\n')
    expect(rows[0][0]).toBe('┌')
    expect(rows[0][rows[0].length - 1]).toBe('┐')
    expect(rows[rows.length - 1][0]).toBe('└')
    expect(rows[rows.length - 1][rows[rows.length - 1].length - 1]).toBe('┘')
  })

  it('opens every middle row with a pipe separator', () => {
    const grat = buildGraticule(80)
    const rows = grat.split('\n')
    for (let r = 1; r < rows.length - 1; r++) {
      expect(rows[r][0]).toBe('│')
    }
  })
})

/* ------------------------------------------------------------------ */
/*  buildTrace unit tests                                              */
/* ------------------------------------------------------------------ */
describe('buildTrace', () => {
  it('returns one string per graticule row, each numCols wide', () => {
    const samples = new Float32Array(240)
    samples[0] = -1
    samples[120] = 0
    samples[239] = 1
    const trace = buildTrace(samples, 10)
    expect(trace).toHaveLength(15)
    for (const row of trace) {
      expect([...row]).toHaveLength(10)
    }
  })

  it('emits braille glyphs for a live signal', () => {
    const samples = new Float32Array(240).fill(0.9)
    const trace = buildTrace(samples, 10)
    const hasBraille = [...trace.join('')].some((ch) => {
      const cp = ch.codePointAt(0)!
      return cp >= 0x2800 && cp <= 0x28ff
    })
    expect(hasBraille).toBe(true)
  })

  it('maps +1 to the top graticule row and -1 to the bottom', () => {
    const top = buildTrace(new Float32Array(240).fill(1), 10)
    const bottom = buildTrace(new Float32Array(240).fill(-1), 10)

    // +1 → absRow 0 → cellRow 0; -1 → absRow 59 → cellRow 14
    const hasDot = (row: string[]) =>
      [...row.join('')].some((ch) => {
        const cp = ch.codePointAt(0)!
        return cp >= 0x2800 && cp <= 0x28ff
      })
    expect(hasDot([top[0]])).toBe(true)
    expect(hasDot([top[14]])).toBe(false)
    expect(hasDot([bottom[14]])).toBe(true)
    expect(hasDot([bottom[0]])).toBe(false)
  })
})

/* ------------------------------------------------------------------ */
/*  applyGlitch unit tests                                             */
/* ------------------------------------------------------------------ */
describe('applyGlitch', () => {
  it('returns input unchanged at zero intensity', () => {
    const rows = ['abc', 'def']
    expect(applyGlitch(rows, 0, () => 0)).toEqual(rows)
  })

  it('corrupts trace rows at full intensity', () => {
    const rows = ['abcd', 'efgh']
    // Low random values trip both the corruption (rand < 0.4) and the
    // displacement thresholds, so the row text changes.
    const result = applyGlitch(rows, 1.0, () => 0.05)
    expect(result).toHaveLength(2)
    expect(result[0]).not.toBe(rows[0])
  })

  it('leaves rows untouched when every random draw clears the thresholds', () => {
    // random() = 1 is above corruption (< 0.4) and displacement (> 0.6) gates
    // for a modest intensity, so applyGlitch is a no-op.
    const rows = ['abc', 'def']
    const out = applyGlitch(rows, 0.1, () => 1)
    expect(out).toEqual(rows)
  })

  it('drops out entire rows at full intensity', () => {
    const rows = ['abcd', 'efgh', 'ijkl']
    // First random draw (drop-out gate, < 0.15) trips for every row → all
    // rows blank out before corruption/displacement ever run.
    const result = applyGlitch(rows, 1.0, () => 0.01)
    expect(result).toHaveLength(3)
    for (const row of result) {
      expect(row).toBe(' '.repeat(4))
    }
  })
})

/* ------------------------------------------------------------------ */
/*  Component rendering tests                                         */
/* ------------------------------------------------------------------ */
describe('FftVisualizer', () => {
  it('mounts a 2D canvas on the dark #0f0e0e backdrop', () => {
    const { container } = render(<FftVisualizer active />)

    expect(container.querySelector('canvas')).toBeInTheDocument()
    expect(screen.getByTestId('fft-visualizer')).toHaveClass('bg-bg')
  })

  it('toggles with the engine active state', () => {
    const { container, rerender } = render(<FftVisualizer active />)
    const viz = () => container.querySelector('[data-testid="fft-visualizer"]')

    expect(viz()).toHaveAttribute('data-active', 'true')

    rerender(<FftVisualizer active={false} />)
    expect(viz()).toHaveAttribute('data-active', 'false')

    rerender(<FftVisualizer active />)
    expect(viz()).toHaveAttribute('data-active', 'true')
  })

  it('accepts an injected signal so tests can feed mock frames', () => {
    const signal = createOscilloscopeSignal(() => 0.5)
    const { container } = render(<FftVisualizer active signal={signal} />)

    expect(container.querySelector('canvas')).toBeInTheDocument()
  })

  it('keeps the oscilloscope band full-width above the faceplate', () => {
    const { container } = render(<FftVisualizer active />)

    const viz = container.querySelector('[data-testid="fft-visualizer"]')
    expect(viz).toHaveClass('w-full')
    expect(viz).toHaveClass('border-b')
  })

  it('accepts a glitch pulser prop without crashing', () => {
    const pulser = createGlitchPulser()
    const { container } = render(<FftVisualizer active glitch={pulser} />)
    expect(container.querySelector('canvas')).toBeInTheDocument()
  })
})
