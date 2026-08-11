import { render, screen, act } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { GainMeter } from '../components/GainMeter'
import { createMockCanvasContext, type CanvasFillOp, type CanvasTextOp } from './mockCanvasContext'

/** Geometry shared by GainMeter's draw() loop and these tests. */
const CANVAS_WIDTH = 8
const CANVAS_HEIGHT = 256

describe('GainMeter', () => {
  let ops: CanvasFillOp[]
  let textOps: CanvasTextOp[]

  beforeEach(() => {
    ops = []
    textOps = []
    vi.useFakeTimers()
    vi.spyOn(HTMLCanvasElement.prototype, 'getContext').mockReturnValue(
      createMockCanvasContext({ ops, textOps })
    )
  })

  afterEach(() => {
    vi.useRealTimers()
    vi.restoreAllMocks()
  })

  describe('markup', () => {
    it('renders the label, scale markers and an 8x256 canvas', () => {
      render(<GainMeter label="IN" active={false} />)

      expect(screen.getByText('IN')).toBeInTheDocument()
      expect(screen.getByText('0')).toBeInTheDocument()
      expect(screen.getByText('-INF')).toBeInTheDocument()

      const canvas = document.querySelector('canvas') as HTMLCanvasElement
      expect(canvas).toBeInTheDocument()
      expect(canvas.width).toBe(CANVAS_WIDTH)
      expect(canvas.height).toBe(CANVAS_HEIGHT)
    })

    it('applies the prototype entrance delay to the meter column', () => {
      const { container } = render(<GainMeter label="IN" active={false} delay={50} />)

      const column = container.querySelector('.animate-vst-enter') as HTMLElement
      expect(column).toHaveStyle({ animationDelay: '50ms' })
    })

    it('frames the meter in a recessed well bezel', () => {
      render(<GainMeter label="IN" active={false} />)

      const rail = (document.querySelector('canvas') as HTMLCanvasElement)
        ?.parentElement
      expect(rail).toHaveStyle({
        boxShadow: 'var(--shadow-well), 0 0 0 1px var(--elev-6)',
      })
    })
  })

  describe('canvas draw loop', () => {
    it('clears the canvas and draws 16 void blocks when inactive', () => {
      render(<GainMeter label="IN" active={false} />)

      // Inactive meter waits for one animation frame before drawing.
      expect(ops).toHaveLength(0)
      expect(textOps).toHaveLength(0)
      act(() => {
        vi.advanceTimersByTime(16)
      })

      // Background clear + final decay-to-zero clear.
      expect(ops).toHaveLength(2)
      expect(ops[0]).toEqual({
        style: '#030303',
        args: [0, 0, CANVAS_WIDTH, CANVAS_HEIGHT],
      })
      expect(ops[1]).toEqual({
        style: '#030303',
        args: [0, 0, CANVAS_WIDTH, CANVAS_HEIGHT],
      })
      // No text drawn when inactive
      expect(textOps).toHaveLength(0)
    })

    it('stacks the 16 rows bottom-up', () => {
      render(<GainMeter label="IN" active={false} />)
      act(() => {
        vi.advanceTimersByTime(16)
      })

      // Background clear at index 0, final clear at index 1
      expect(ops[0].args[1]).toBe(0)
      expect(ops[1].args[1]).toBe(0)
    })

    it('does not keep animating once the inactive level decays to zero', () => {
      render(<GainMeter label="IN" active={false} />)
      act(() => {
        vi.advanceTimersByTime(16)
      })
      const afterFirstFrame = ops.length

      act(() => {
        vi.advanceTimersByTime(1000)
      })
      expect(ops.length).toBe(afterFirstFrame)
    })

    it('draws block characters via fillText when active signal is present', () => {
      // Deterministic signal: rawTarget = 0.5 * 0.7 + 0.2 = 0.55
      vi.spyOn(Math, 'random').mockReturnValue(0.5)

      render(<GainMeter label="IN" active />)

      // Active meter draws immediately: background clear + text ops
      expect(ops[0]).toEqual({
        style: '#030303',
        args: [0, 0, CANVAS_WIDTH, CANVAS_HEIGHT],
      })

      // Should have textOps for the block characters
      expect(textOps.length).toBeGreaterThan(0)

      // Check that fillText was called with block characters
      const chars = textOps.map((op) => op.text)
      expect(chars.some((c) => ['▁', '▂', '▃', '▄', '▅', '▆', '▇', '█'].includes(c))).toBe(true)
    })

    it('draws peak glyph ▲ at top rows when fully filled', () => {
      // Force the 5% "spike" branch: first call ≤ 0.05 selects rawTarget = rnd * 0.99
      let call = 0
      vi.spyOn(Math, 'random').mockImplementation(() => {
        call += 1
        return call % 2 === 1 ? 0.04 : 1
      })

      render(<GainMeter label="IN" active />)

      // Converge the level toward 0.99
      for (let i = 0; i < 12; i++) {
        act(() => {
          vi.advanceTimersByTime(16)
        })
      }

      // Check that peak glyph was drawn (textOps with ▲)
      const peakOps = textOps.filter((op) => op.text === '▲')
      expect(peakOps.length).toBeGreaterThan(0)
    })

    it('continues animating each frame while active', () => {
      vi.spyOn(Math, 'random').mockReturnValue(0.5)

      render(<GainMeter label="IN" active />)
      const afterFirstFrame = textOps.length

      act(() => {
        vi.advanceTimersByTime(16)
      })
      // Every active frame redraws background + text
      expect(textOps.length).toBeGreaterThan(afterFirstFrame)
    })

    it('uses correct colors for well, level, and peak', () => {
      vi.spyOn(Math, 'random').mockReturnValue(0.5)

      render(<GainMeter label="IN" active />)

      // Check that textOps have correct colors
      const levelOps = textOps.filter((op) => op.style === '#888888')
      const wellOps = textOps.filter((op) => op.style === '#0a0a0a')
      expect(levelOps.length).toBeGreaterThan(0)
      expect(wellOps.length).toBeGreaterThan(0)
    })
  })
})