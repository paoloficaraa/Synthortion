import { describe, it, expect } from 'vitest'
import {
  CORRUPTION_GLYPHS,
  GLITCH_DECAY_TAU_MS,
  calculateDragVelocity,
  corruptTrackCells,
} from '../lib/trackGlitch'

describe('trackGlitch', () => {
  describe('CORRUPTION_GLYPHS', () => {
    it('contains the full set of specified corruption glyphs', () => {
      const expected = ['░', '▒', '▓', '╱', '╲', '╳', '▲', '::', '~', '+', '*']
      expect(CORRUPTION_GLYPHS).toEqual(expected)
    })
  })

  describe('GLITCH_DECAY_TAU_MS', () => {
    it('is within the specified 120-150 ms decay window', () => {
      expect(GLITCH_DECAY_TAU_MS).toBeGreaterThanOrEqual(120)
      expect(GLITCH_DECAY_TAU_MS).toBeLessThanOrEqual(150)
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

})
