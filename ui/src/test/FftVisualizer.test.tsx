import { render, screen, act } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { FftVisualizer } from '../components/FftVisualizer'
import { createOscilloscopeSignal } from '../lib/oscilloscopeSignal'
import { applyGlitch, createGlitchPulser } from '../lib/glitchPulser'
import { createMockCanvasContext, type CanvasTextOp } from './mockCanvasContext'

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
})

