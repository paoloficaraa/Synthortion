import { render, screen, act } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { GainMeter } from '../components/GainMeter'

interface FillOp {
  style: string
  args: number[]
}

/**
 * Builds a fake 2D context that records every fillRect together with the
 * fillStyle that was active at call time. Kept outside the component so the
 * visual-render path is exercised through the real draw() loop, not by
 * inspecting internal React state.
 */
function createMockContext(ops: FillOp[]) {
  let fillStyle = ''
  return {
    get fillStyle() {
      return fillStyle
    },
    set fillStyle(value: string) {
      fillStyle = value
    },
    fillRect: vi.fn((...args: number[]) => {
      ops.push({ style: fillStyle, args })
    }),
  }
}

describe('GainMeter', () => {
  let ops: FillOp[]

  beforeEach(() => {
    ops = []
    vi.spyOn(HTMLCanvasElement.prototype, 'getContext').mockReturnValue(
      createMockContext(ops) as unknown as CanvasRenderingContext2D
    )
  })

  afterEach(() => {
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
      expect(canvas.width).toBe(12)
      expect(canvas.height).toBe(800)
    })

    it('renders children (e.g. a TRIM knob) inside the column', () => {
      render(
        <GainMeter label="IN" active={false}>
          <button type="button">TRIM</button>
        </GainMeter>
      )

      expect(screen.getByRole('button', { name: 'TRIM' })).toBeInTheDocument()
    })
  })

  describe('canvas draw loop', () => {
    it('clears the canvas and draws 32 void blocks when inactive', () => {
      vi.useFakeTimers()
      try {
        render(<GainMeter label="IN" active={false} />)

        // Inactive meter waits for one animation frame before drawing.
        expect(ops).toHaveLength(0)
        act(() => {
          vi.advanceTimersByTime(16)
        })

        // Background clear + 32 blocks + final decay-to-zero clear.
        expect(ops).toHaveLength(34)
        expect(ops[0]).toEqual({ style: '#020202', args: [0, 0, 12, 800] })

        // Every segment is drawn void (near-black) with the prototype block gap.
        const blocks = ops.slice(1, 33)
        expect(blocks).toHaveLength(32)
        for (const block of blocks) {
          expect(block.style).toBe('#0a0a0a')
          expect(block.args[0]).toBe(0)
          expect(block.args[2]).toBe(12)
          expect(block.args[3]).toBe(21)
        }

        expect(ops[33]).toEqual({ style: '#020202', args: [0, 0, 12, 800] })
      } finally {
        vi.useRealTimers()
      }
    })

    it('stacks the 32 blocks bottom-up with the prototype proportions', () => {
      vi.useFakeTimers()
      try {
        render(<GainMeter label="IN" active={false} />)
        act(() => {
          vi.advanceTimersByTime(16)
        })

        const blocks = ops.slice(1, 33)
        // blockH = 800 / 32 = 25; drawn height = max(2, 25 - 4) = 21.
        expect(blocks[0].args[1]).toBe(800 - 25 + 2)
        expect(blocks[31].args[1]).toBe(800 - 32 * 25 + 2)
      } finally {
        vi.useRealTimers()
      }
    })

    it('does not keep animating once the inactive level decays to zero', () => {
      vi.useFakeTimers()
      try {
        render(<GainMeter label="IN" active={false} />)
        act(() => {
          vi.advanceTimersByTime(16)
        })
        const afterFirstFrame = ops.length

        act(() => {
          vi.advanceTimersByTime(1000)
        })
        expect(ops.length).toBe(afterFirstFrame)
      } finally {
        vi.useRealTimers()
      }
    })

    it('fills grey segments when an active signal is present', () => {
      // Deterministic signal: rawTarget = 0.5 * 0.7 + 0.2 = 0.55.
      vi.spyOn(Math, 'random').mockReturnValue(0.5)

      vi.useFakeTimers()
      try {
        render(<GainMeter label="IN" active />)

        // Active meter draws immediately: background + 4 grey + 28 void.
        expect(ops[0]).toEqual({ style: '#020202', args: [0, 0, 12, 800] })

        const blocks = ops.slice(1, 33)
        const greyCount = blocks.filter((b) => b.style === '#888888').length
        const voidCount = blocks.filter((b) => b.style === '#0a0a0a').length

        expect(greyCount).toBe(4)
        expect(voidCount).toBe(28)
      } finally {
        vi.useRealTimers()
      }
    })

    it('lights peak segments white when the level reaches the top of the scale', () => {
      // Force the 5% "spike" branch: first call ≤ 0.05 selects rawTarget = rnd * 0.99.
      let call = 0
      vi.spyOn(Math, 'random').mockImplementation(() => {
        call += 1
        return call % 2 === 1 ? 0.04 : 1
      })

      vi.useFakeTimers()
      try {
        render(<GainMeter label="IN" active />)

        // Converge the level toward 0.99 → activeCount crosses 29/32 blocks.
        for (let i = 0; i < 12; i++) {
          act(() => {
            vi.advanceTimersByTime(16)
          })
        }

        expect(ops.some((op) => op.style === '#ffffff')).toBe(true)
      } finally {
        vi.useRealTimers()
      }
    })

    it('continues animating each frame while active', () => {
      vi.spyOn(Math, 'random').mockReturnValue(0.5)

      vi.useFakeTimers()
      try {
        render(<GainMeter label="IN" active />)
        const afterFirstFrame = ops.length

        act(() => {
          vi.advanceTimersByTime(16)
        })
        expect(ops.length).toBe(afterFirstFrame + 33)
      } finally {
        vi.useRealTimers()
      }
    })
  })
})
