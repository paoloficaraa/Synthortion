import { describe, it, expect } from 'vitest'
import {
  HEX_CHARS,
  HEX_SCRAMBLE_DURATION_MS,
  generateHexScramble,
} from '../lib/hexScramble'

describe('hexScramble', () => {
  describe('HEX_CHARS', () => {
    it('contains standard uppercase terminal hex characters', () => {
      expect(HEX_CHARS).toBe('0123456789ABCDEF')
    })
  })

  describe('HEX_SCRAMBLE_DURATION_MS', () => {
    it('is within the specified 30-50 ms window', () => {
      expect(HEX_SCRAMBLE_DURATION_MS).toBeGreaterThanOrEqual(30)
      expect(HEX_SCRAMBLE_DURATION_MS).toBeLessThanOrEqual(50)
    })
  })

  describe('generateHexScramble', () => {
    it('preserves text length', () => {
      const scrambled = generateHexScramble('42%')
      expect(scrambled.length).toBe(3)
    })

    it('scrambles alphanumeric / digit characters with hex digits', () => {
      const fixedRandom = () => 0.625 // maps to index 10 in 16 hex chars -> 'A'
      const scrambled = generateHexScramble('42%', fixedRandom)
      expect(scrambled.length).toBe(3)
      // First two chars should be hex chars ('A')
      expect(scrambled.startsWith('AA')).toBe(true)
    })

    it('returns empty string when input is empty', () => {
      expect(generateHexScramble('')).toBe('')
    })
  })
})
