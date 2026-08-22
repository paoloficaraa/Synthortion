import { describe, it, expect } from 'vitest'
import {
  CORRUPTION_GLYPHS,
  GLITCH_DECAY_TAU_MS,
  calculateDragVelocity,
  corruptTrackCells,
  createTrackGlitchController,
} from '../lib/trackGlitch'

describe('trackGlitch', () => {
  describe('CORRUPTION_GLYPHS', () => {
    it('contains the full set of specified corruption glyphs', () => {
      const expected = ['░', '▒', '▓', '╱', '╲', '╳', '▲', '::', '~', '+', '*']
      expect(CORRUPTION_GLYPHS).toEqual(expected)
    })
  })

  describe('calculateDragVelocity', () => {
    it('calculates |dy| / dt in pixels per millisecond', () => {
      // 20px over 10ms -> 2.0 px/ms
      const v = calculateDragVelocity({
        currentY: 180,
        lastY: 200,
        currentTime: 100,
        lastTime: 90,
      })
      expect(v).toBe(2.0)
    })

    it('returns 0 if dt is 0 or negative', () => {
      const v = calculateDragVelocity({
        currentY: 180,
        lastY: 200,
        currentTime: 100,
        lastTime: 100,
      })
      expect(v).toBe(0)
    })

    it('returns 0 when shiftKey is true (fine adjustment suppression)', () => {
      const v = calculateDragVelocity({
        currentY: 180,
        lastY: 200,
        currentTime: 100,
        lastTime: 90,
        shiftKey: true,
      })
      expect(v).toBe(0)
    })
  })

  describe('corruptTrackCells', () => {
    const cleanCells = [
      { char: '█', filled: true },
      { char: '█', filled: true },
      { char: '▒', filled: true },
      { char: '-', filled: false },
      { char: '-', filled: false },
    ]

    it('returns clean cells untouched when intensity is 0', () => {
      const result = corruptTrackCells(cleanCells, 0)
      expect(result).toEqual(cleanCells)
    })

    it('replaces cells with corruption glyphs when intensity > 0', () => {
      // Deterministic PRNG that always triggers corruption
      const fixedRandom = () => 0.1
      const result = corruptTrackCells(cleanCells, 1.0, fixedRandom)
      expect(result).not.toEqual(cleanCells)
      // Every corrupted cell should have a char from CORRUPTION_GLYPHS
      result.forEach((cell) => {
        expect(CORRUPTION_GLYPHS).toContain(cell.char)
      })
    })
  })

  describe('createTrackGlitchController', () => {
    it('initializes with zero intensity', () => {
      const controller = createTrackGlitchController()
      expect(controller.intensity).toBe(0)
    })

    it('triggers glitch on fast drag (velocity >= threshold)', () => {
      const controller = createTrackGlitchController()
      controller.onPointerMove({
        currentY: 150,
        currentTime: 20,
        shiftKey: false,
      })
      // Threshold is FAST_DRAG_VELOCITY_THRESHOLD
      expect(controller.intensity).toBeGreaterThan(0)
    })

    it('does not trigger glitch on slow drag', () => {
      const controller = createTrackGlitchController()
      // Initial move to set baseline
      controller.onPointerDown({ startY: 200, startTime: 0 })
      // Move 2px over 50ms -> 0.04 px/ms (< 0.4 threshold)
      controller.onPointerMove({
        currentY: 198,
        currentTime: 50,
        shiftKey: false,
      })
      expect(controller.intensity).toBe(0)
    })

    it('does not trigger glitch when shiftKey is held (fine adjustment)', () => {
      const controller = createTrackGlitchController()
      controller.onPointerDown({ startY: 200, startTime: 0 })
      controller.onPointerMove({
        currentY: 100,
        currentTime: 10,
        shiftKey: true,
      })
      expect(controller.intensity).toBe(0)
    })

    it('decays exponentially over ~120-150 ms to zero', () => {
      const controller = createTrackGlitchController()
      controller.onPointerDown({ startY: 200, startTime: 0 })
      controller.onPointerMove({
        currentY: 100,
        currentTime: 10,
        shiftKey: false,
      })
      expect(controller.intensity).toBe(1)

      // Step by GLITCH_DECAY_TAU_MS (~135ms)
      controller.step(GLITCH_DECAY_TAU_MS)
      expect(controller.intensity).toBeCloseTo(Math.exp(-1), 2)

      // Step further until fully decayed
      controller.step(300)
      expect(controller.intensity).toBe(0)
    })
  })
})
