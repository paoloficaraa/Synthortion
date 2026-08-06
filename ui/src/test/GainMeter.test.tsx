import { render, screen, act } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { GainMeter } from '../components/GainMeter'
import { createMockCanvasContext, type CanvasFillOp } from './mockCanvasContext'

/** Geometry shared by GainMeter's draw() loop and these tests. */
const BLOCK_COUNT = 32
const CANVAS_WIDTH = 12
const CANVAS_HEIGHT = 800
const BLOCK_HEIGHT = CANVAS_HEIGHT / BLOCK_COUNT // 25
const BLOCK_GAP = 4
const BLOCK_OFFSET = 2
const BLOCK_DRAWN_HEIGHT = Math.max(2, BLOCK_HEIGHT - BLOCK_GAP) // 21

describe('GainMeter', () => {
  let ops: CanvasFillOp[]

  beforeEach(() => {
    ops = []
    vi.useFakeTimers()
    vi.spyOn(HTMLCanvasElement.prototype, 'getContext').mockReturnValue(
      createMockCanvasContext(ops)
    )
  })

  afterEach(() => {
    vi.useRealTimers()
    vi.restoreAllMocks()
  })

  describe('markup', () => {
    it('renders the label, scale markers and a 12x800 canvas', () => {
      render(<GainMeter label="IN" active={false} />)

      expect(screen.getByText('IN')).toBeInTheDocument()
      expect(screen.getByText('0')).toBeInTheDocument()
      expect(screen.getByText('-INF')).toBeInTheDocument()

      const canvas = document.querySelector('canvas') as HTMLCanvasElement
      expect(canvas).toBeInTheDocument()
      expect(canvas.width).toBe(CANVAS_WIDTH)
      expect(canvas.height).toBe(CANVAS_HEIGHT)
    })

    it('renders children (e.g. a TRIM knob) inside the column', () => {
      render(
        <GainMeter label="IN" active={false}>
          <button type="button">TRIM</button>
        </GainMeter>
      )

      expect(screen.getByRole('button', { name: 'TRIM' })).toBeInTheDocument()
    })

    it('applies the prototype entrance delay to the meter column', () => {
      const { container } = render(<GainMeter label="IN" active={false} delay={50} />)

      const column = container.querySelector('.animate-vst-enter') as HTMLElement
      expect(column).toHaveStyle({ animationDelay: '50ms' })
    })
  })

  describe('canvas draw loop', () => {
    it('clears the canvas and draws 32 void blocks when inactive', () => {
      render(<GainMeter label="IN" active={false} />)

      // Inactive meter waits for one animation frame before drawing.
      expect(ops).toHaveLength(0)
      act(() => {
        vi.advanceTimersByTime(16)
      })

      // Background clear + 32 blocks + final decay-to-zero clear.
      expect(ops).toHaveLength(BLOCK_COUNT + 2)
      expect(ops[0]).toEqual({
        style: '#030303',
        args: [0, 0, CANVAS_WIDTH, CANVAS_HEIGHT],
      })

      // Every segment is drawn void (near-black) with the prototype block gap.
      const blocks = ops.slice(1, 1 + BLOCK_COUNT)
      expect(blocks).toHaveLength(BLOCK_COUNT)
      for (const block of blocks) {
        expect(block.style).toBe('#0a0a0a')
        expect(block.args[0]).toBe(0)
        expect(block.args[2]).toBe(CANVAS_WIDTH)
        expect(block.args[3]).toBe(BLOCK_DRAWN_HEIGHT)
      }

      expect(ops[BLOCK_COUNT + 1]).toEqual({
        style: '#030303',
        args: [0, 0, CANVAS_WIDTH, CANVAS_HEIGHT],
      })
    })

    it('stacks the 32 blocks bottom-up with the prototype proportions', () => {
      render(<GainMeter label="IN" active={false} />)
      act(() => {
        vi.advanceTimersByTime(16)
      })

      const blocks = ops.slice(1, 1 + BLOCK_COUNT)
      // Block i sits at y = CANVAS_HEIGHT - (i + 1) * BLOCK_HEIGHT, drawn 2px lower.
      expect(blocks[0].args[1]).toBe(CANVAS_HEIGHT - BLOCK_HEIGHT + BLOCK_OFFSET)
      expect(blocks[31].args[1]).toBe(
        CANVAS_HEIGHT - BLOCK_COUNT * BLOCK_HEIGHT + BLOCK_OFFSET
      )
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

    it('fills grey segments when an active signal is present', () => {
      // Deterministic signal: rawTarget = 0.5 * 0.7 + 0.2 = 0.55.
      vi.spyOn(Math, 'random').mockReturnValue(0.5)

      render(<GainMeter label="IN" active />)

      // Active meter draws immediately: background + 4 grey + 28 void.
      expect(ops[0]).toEqual({
        style: '#030303',
        args: [0, 0, CANVAS_WIDTH, CANVAS_HEIGHT],
      })

      const blocks = ops.slice(1, 1 + BLOCK_COUNT)
      const greyCount = blocks.filter((b) => b.style === '#888888').length
      const voidCount = blocks.filter((b) => b.style === '#0a0a0a').length

      expect(greyCount).toBe(4)
      expect(voidCount).toBe(BLOCK_COUNT - 4)
    })

    it('lights peak segments white when the level reaches the top of the scale', () => {
      // Force the 5% "spike" branch: first call ≤ 0.05 selects rawTarget = rnd * 0.99.
      let call = 0
      vi.spyOn(Math, 'random').mockImplementation(() => {
        call += 1
        return call % 2 === 1 ? 0.04 : 1
      })

      render(<GainMeter label="IN" active />)

      // Converge the level toward 0.99 → activeCount crosses 29/32 blocks.
      for (let i = 0; i < 12; i++) {
        act(() => {
          vi.advanceTimersByTime(16)
        })
      }

      expect(ops.some((op) => op.style === '#f6f6f6')).toBe(true)
    })

    it('continues animating each frame while active', () => {
      vi.spyOn(Math, 'random').mockReturnValue(0.5)

      render(<GainMeter label="IN" active />)
      const afterFirstFrame = ops.length

      act(() => {
        vi.advanceTimersByTime(16)
      })
      // Every active frame redraws the background plus all 32 blocks.
      expect(ops.length).toBe(afterFirstFrame + BLOCK_COUNT + 1)
    })
  })
})
