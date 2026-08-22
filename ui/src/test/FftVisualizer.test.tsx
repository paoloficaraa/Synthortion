import { render, screen } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { FftVisualizer } from '../components/FftVisualizer'
import { createOscilloscopeSignal } from '../lib/oscilloscopeSignal'
import {
  buildGraticule,
  buildTrace,
  buildDither,
  buildTraceAndDither,
  CELL_ROWS,
} from '../lib/fftBraille'
import { applyGlitch, createGlitchPulser } from '../lib/glitchPulser'

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
  })

  it('accepts a glitch pulser prop without crashing', () => {
    const pulser = createGlitchPulser()
    const { container } = render(<FftVisualizer active glitch={pulser} />)
    expect(container.querySelector('canvas')).toBeInTheDocument()
  })

  it('declares dual-mode on the container', () => {
    render(<FftVisualizer active />)
    expect(screen.getByTestId('fft-visualizer')).toHaveAttribute(
      'data-mode',
      'dual',
    )
  })

  it('renders a static frame when reduced motion is preferred', () => {
    const original = window.matchMedia
    window.matchMedia = ((query: string) => ({
      matches: true,
      media: query,
      onchange: null,
      addListener: () => {},
      removeListener: () => {},
      addEventListener: () => {},
      removeEventListener: () => {},
      dispatchEvent: () => false,
    })) as unknown as typeof window.matchMedia
    try {
      const rafSpy = vi.spyOn(window, 'requestAnimationFrame')
      render(<FftVisualizer active />)
      expect(screen.getByTestId('fft-visualizer')).toHaveAttribute(
        'data-static',
        'true',
      )
      // No rAF loop at all — the frame is drawn once, statically.
      expect(rafSpy).not.toHaveBeenCalled()
      rafSpy.mockRestore()
    } finally {
      window.matchMedia = original
    }
  })

  it('animates when motion is allowed', () => {
    render(<FftVisualizer active />)
    expect(screen.getByTestId('fft-visualizer')).toHaveAttribute(
      'data-static',
      'false',
    )
  })
})

/* ------------------------------------------------------------------ */
/*  Graticule / trace / dither helpers                                */
/* ------------------------------------------------------------------ */
describe('buildGraticule', () => {
  it('frames the grid with box-drawing corners', () => {
    const rows = buildGraticule(40).split('\n')
    expect(rows).toHaveLength(CELL_ROWS)
    expect(rows[0][0]).toBe('┌')
    expect(rows[0].at(-1)).toBe('┐')
    expect(rows[rows.length - 1][0]).toBe('└')
    expect(rows[rows.length - 1].at(-1)).toBe('┘')
  })

  it('renders Cartesian crosshairs on the centre row at every division', () => {
    const rows = buildGraticule(41, 9).split('\n')
    const mid = rows[Math.floor(9 / 2)]
    expect(mid[0]).toBe('+')
    expect(mid.at(-1)).toBe('+')
    // Interior intersections: one per vertical division.
    expect((mid.match(/\+/g) ?? []).length).toBeGreaterThanOrEqual(5)
  })

  it('renders a crisp top border frame with ┬ ticks', () => {
    const top = buildGraticule(60, 9).split('\n')[0]
    expect(top[0]).toBe('┌')
    expect(top.at(-1)).toBe('┐')
    expect(top).toContain('┬')
  })
})

describe('buildTrace / buildDither', () => {
  it('builds one braille string per grid row at the requested height', () => {
    const rows = buildTrace(new Float32Array(240), 30, 9)
    expect(rows).toHaveLength(9)
    for (const row of rows) expect(row).toHaveLength(30)
  })

  it('keeps braille glyphs inside the U+2800–U+28FF block', () => {
    const samples = new Float32Array(240).map((_, i) => Math.sin(i * 0.1))
    for (const row of buildTrace(samples, 30, 9)) {
      for (const ch of row) {
        const cp = ch.codePointAt(0)!
        expect(cp === 0x20 || (cp >= 0x2800 && cp <= 0x28ff)).toBe(true)
      }
    }
  })

  it('lights the top dot row for a full-scale positive DC signal', () => {
    const rows = buildTrace(new Float32Array(240).fill(1), 8, 4)
    expect(rows[0]).toContain('⠉')
  })

  it('dithers fractional positions but leaves exact hits clean', () => {
    // y = 1 lands exactly on dot row 0 for any height → no dither glyphs.
    const exact = buildDither(new Float32Array(240).fill(1), 8, 4)
    for (const row of exact) expect(row).toBe(' '.repeat(8))
    // y = 0.07 lands part-way between dots (absRow 16.275 of 35) → dither.
    const fuzzy = buildDither(new Float32Array(240).fill(0.07), 8, 9)
    expect(fuzzy.join('')).toContain('░')
  })
  it('builds combined trace and dither in one single pass', () => {
    const { trace, dither } = buildTraceAndDither(new Float32Array(240).fill(0.07), 8, 9)
    expect(trace).toHaveLength(9)
    expect(dither).toHaveLength(9)
    expect(dither.join('')).toContain('░')
  })
})

