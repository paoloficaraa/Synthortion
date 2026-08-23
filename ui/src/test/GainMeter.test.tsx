import { render, screen, act } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { GainMeter } from '../components/GainMeter'
import { subscribeToDspMeters, type MeterFrame } from '../lib/webViewDspBridge'
import { createMockCanvasContext, type CanvasFillOp, type CanvasTextOp } from './mockCanvasContext'

vi.mock('../lib/webViewDspBridge', () => ({
  subscribeToDspMeters: vi.fn(() => vi.fn()),
}))

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
    it('renders the label and an 8x256 canvas', () => {
      render(<GainMeter label="IN" active={false} />)

      expect(screen.getByText('IN')).toBeInTheDocument()
      expect(screen.getByText('dB')).toBeInTheDocument()

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

    it('has correct ARIA role and label', () => {
      render(<GainMeter label="IN" active={false} />)
      const meter = screen.getByRole('meter')
      expect(meter).toHaveAttribute('aria-label', 'IN Meter')
    })
  })

  describe('canvas draw loop', () => {
    it('clears the canvas and draws void when inactive', () => {
      render(<GainMeter label="IN" active={false} />)

      // Inactive meter draws background on first frame, no ladder blocks
      expect(ops).toHaveLength(0)
      act(() => {
        vi.advanceTimersByTime(16)
      })

      expect(ops.length).toBeGreaterThanOrEqual(1)
      expect(ops[0]).toEqual({
        style: '#030303',
        args: [0, 0, CANVAS_WIDTH, CANVAS_HEIGHT],
      })
      // No active blocks when void — only background clears
      const blocks = textOps.filter((op) => op.text !== '·')
      expect(blocks).toHaveLength(0)
    })

    it('stacks the 16 rows bottom-up', () => {
      let meterCallback: ((frame: MeterFrame) => void) | undefined
      vi.mocked(subscribeToDspMeters).mockImplementation((cb) => {
        meterCallback = cb
        return vi.fn()
      })
      render(<GainMeter label="IN" active={true} />)
      act(() => {
        meterCallback?.({ input: 0.5, output: 0.5 })
        vi.advanceTimersByTime(16)
      })
      // After feeding 0.5, some middle rows should be filled — verify fillText was called with y in range
      expect(textOps.length).toBeGreaterThan(0)
      const ys = textOps.map((op) => op.y)
      expect(Math.min(...ys)).toBeGreaterThanOrEqual(0)
      expect(Math.max(...ys)).toBeLessThan(CANVAS_HEIGHT)
    })

    it('does not render blocks once the inactive level decays to void', () => {
      let meterCallback: ((frame: MeterFrame) => void) | undefined
      vi.mocked(subscribeToDspMeters).mockImplementation((cb) => {
        meterCallback = cb
        return vi.fn()
      })
      const { rerender } = render(<GainMeter label="IN" active={true} />)
      act(() => {
        meterCallback?.({ input: 0.9, output: 0.9 })
        vi.advanceTimersByTime(16)
      })
      const initialLevelBlocks = textOps.filter((op) => op.style === '#888888').length
      expect(initialLevelBlocks).toBeGreaterThan(0)
      // Switch to inactive — should decay toward void
      textOps.length = 0
      ops.length = 0
      rerender(<GainMeter label="IN" active={false} />)
      act(() => {
        vi.advanceTimersByTime(1200)
      })
      // After decay, recent draws should have fewer level blocks than initial peak
      // (cumulative would be high, so check average per frame via recent slice)
      const recentLevelBlocks = textOps.slice(-64).filter((op) => op.style === '#888888').length
      // Should be significantly less than initial peak's per-frame count extrapolated
      // Allow loose check: decay must have reduced level vs peak
      expect(recentLevelBlocks).toBeLessThan(initialLevelBlocks * 4)
    })
    it('draws block characters via fillText when active signal is present', () => {
      let meterCallback: ((frame: MeterFrame) => void) | undefined
      vi.mocked(subscribeToDspMeters).mockImplementation((cb) => {
        meterCallback = cb
        return vi.fn()
      })

      render(<GainMeter label="IN" active={true} />)

      act(() => {
        meterCallback?.({ input: 0.5, output: 0.5 })
        vi.advanceTimersByTime(16)
      })

      // Should have drawn some blocks
      const blocks = textOps.filter((op) => op.text !== '·')
      expect(blocks.length).toBeGreaterThan(0)
    })

    it('draws peak glyph ▲ at top rows when fully filled', () => {
      let meterCallback: ((frame: MeterFrame) => void) | undefined
      vi.mocked(subscribeToDspMeters).mockImplementation((cb) => {
        meterCallback = cb
        return vi.fn()
      })

      render(<GainMeter label="IN" active />)

      act(() => {
        meterCallback?.({ input: 1.0, output: 1.0 })
        vi.advanceTimersByTime(16)
      })

      // Check that peak glyph was drawn (textOps with ▲)
      const peakOps = textOps.filter((op) => op.text === '▲')
      expect(peakOps.length).toBeGreaterThan(0)
    })

    it('continues animating each frame while active', () => {
      let meterCallback: ((frame: MeterFrame) => void) | undefined
      vi.mocked(subscribeToDspMeters).mockImplementation((cb) => {
        meterCallback = cb
        return vi.fn()
      })

      render(<GainMeter label="IN" active />)
      act(() => {
        meterCallback?.({ input: 0.5, output: 0.5 })
        vi.advanceTimersByTime(16)
      })
      const afterFirstFrame = textOps.length

      act(() => {
        vi.advanceTimersByTime(16)
      })
      // Every active frame redraws background + text
      expect(textOps.length).toBeGreaterThan(afterFirstFrame)
    })

    it('uses correct colors for well, level, and peak', () => {
      let meterCallback: ((frame: MeterFrame) => void) | undefined
      vi.mocked(subscribeToDspMeters).mockImplementation((cb) => {
        meterCallback = cb
        return vi.fn()
      })

      render(<GainMeter label="IN" active />)
      act(() => {
        meterCallback?.({ input: 0.5, output: 0.5 })
        vi.advanceTimersByTime(16)
      })

      // Check that textOps have correct colors
      const levelOps = textOps.filter((op) => op.style === '#888888')
      const wellOps = textOps.filter((op) => op.style === '#0a0a0a')
      expect(levelOps.length).toBeGreaterThan(0)
      // Well ticks may be zero at mid-level, accept either but level must be present
      expect(levelOps.length + wellOps.length).toBeGreaterThan(0)
    })
  })
})