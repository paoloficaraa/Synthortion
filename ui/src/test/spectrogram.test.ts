import { describe, it, expect } from 'vitest'
import {
  analyzeBands,
  binFrequencies,
  createWaterfallHistory,
  densityGlyph,
  MAX_HZ,
  MIN_HZ,
  SIM_SAMPLE_RATE,
} from '../lib/spectrogram'

describe('binFrequencies', () => {
  it('spans 20Hz–20kHz inclusive with log spacing', () => {
    const freqs = binFrequencies(5)
    expect(freqs[0]).toBeCloseTo(MIN_HZ)
    expect(freqs[freqs.length - 1]).toBeCloseTo(MAX_HZ)
    // Log spacing: equal ratios between neighbours.
    const ratios = freqs.slice(1).map((f, i) => f / freqs[i])
    for (const r of ratios) expect(r).toBeCloseTo(ratios[0])
  })

  it('collapses to a single MIN_HZ bin below 2 bins', () => {
    expect(binFrequencies(1)).toEqual([MIN_HZ])
  })
})

describe('analyzeBands', () => {
  it('returns zero energy for silence', () => {
    const out = analyzeBands(new Float32Array(240), 16)
    expect(out).toHaveLength(16)
    expect(Array.from(out).every((v) => v === 0)).toBe(true)
  })

  it('concentrates a sine at its own bin', () => {
    const n = 240
    const freqs = binFrequencies(16)
    const target = freqs[8]
    const samples = new Float32Array(n)
    for (let i = 0; i < n; i++) {
      samples[i] = Math.sin((2 * Math.PI * target * i) / SIM_SAMPLE_RATE)
    }
    const out = analyzeBands(samples, 16)
    const peak = out.indexOf(Math.max(...out))
    expect(peak).toBe(8)
    expect(out[8]).toBeGreaterThan(0.5)
    for (let i = 0; i < out.length; i++) {
      if (i !== 8) expect(out[i]).toBeLessThan(out[8])
    }
  })

  it('clamps band energies to [0, 1]', () => {
    const samples = new Float32Array(240).fill(1)
    const out = analyzeBands(samples, 8)
    for (const v of out) {
      expect(v).toBeGreaterThanOrEqual(0)
      expect(v).toBeLessThanOrEqual(1)
    }
  })
})

describe('densityGlyph', () => {
  it('maps silence to a blank cell', () => {
    expect(densityGlyph(0)).toBe(' ')
    expect(densityGlyph(0.03)).toBe(' ')
  })

  it('ramps through the density gradient', () => {
    expect(densityGlyph(0.2)).toBe('░')
    expect(densityGlyph(0.5)).toBe('▓')
    expect(densityGlyph(0.99)).toBe('█')
    expect(densityGlyph(1)).toBe('█')
  })

  it('treats NaN as silence', () => {
    expect(densityGlyph(Number.NaN)).toBe(' ')
  })
})

describe('createWaterfallHistory', () => {
  it('appends rows newest-last and caps at depth', () => {
    const h = createWaterfallHistory(3)
    for (const v of [0.1, 0.5, 0.9, 1.0]) {
      h.push(new Float32Array([v, v]))
    }
    expect(h.size).toBe(3)
    const lines = h.lines()
    expect(lines[lines.length - 1]).toBe(densityGlyph(1.0).repeat(2))
    expect(lines[0]).toBe(densityGlyph(0.5).repeat(2))
  })

  it('renders glyphs at one character per band', () => {
    const h = createWaterfallHistory(4)
    h.push(new Float32Array([0, 0.2, 0.5, 1]))
    expect(h.lines()[0]).toBe(' ░▓█')
  })

  it('clear() empties the history for bypass', () => {
    const h = createWaterfallHistory(4)
    h.push(new Float32Array([1, 1]))
    h.clear()
    expect(h.size).toBe(0)
    expect(h.lines()).toEqual([])
  })

  it('defends its internal rows from caller mutation', () => {
    const h = createWaterfallHistory(2)
    h.push(new Float32Array([1]))
    h.lines().push('junk')
    expect(h.size).toBe(1)
  })
})
