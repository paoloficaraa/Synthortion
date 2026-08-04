import { describe, it, expect } from 'vitest'
import {
  createFftSignal,
  structuralNoise,
  boostFor,
  rawTarget,
  BIN_COUNT,
  BAR_COUNT,
  BOOST_AMOUNT,
  SMOOTHING,
  PHASE_STEP,
} from '../lib/fftSignal'

describe('fftSignal — prototype structural-noise generator', () => {
  it('starts with a zeroed bin array of prototype length (128)', () => {
    const signal = createFftSignal()

    expect(signal.bins).toHaveLength(BIN_COUNT)
    expect(Array.from(signal.bins).every((bin) => bin === 0)).toBe(true)
  })

  it('computes the structural-noise oscillator for a known bin/phase', () => {
    // sin(0 + 0 * 0.2) * 5 + cos(0 - 0) * 10 = 10
    expect(structuralNoise(0, 0)).toBeCloseTo(10)
    // bin 25, phase 0: 5 * sin(5) + 10 * cos(-2.5)
    expect(structuralNoise(25, 0)).toBeCloseTo(
      5 * Math.sin(5) + 10 * Math.cos(-2.5)
    )
  })

  it('applies the low-mid boost only to the prototype band (11..30)', () => {
    expect(boostFor(10)).toBe(0)
    expect(boostFor(11)).toBe(BOOST_AMOUNT)
    expect(boostFor(29)).toBe(BOOST_AMOUNT)
    expect(boostFor(30)).toBe(0)
  })

  it('builds an active raw target from jitter + noise + boost + 10', () => {
    // random()*20 (10) + structuralNoise(0,0) (10) + boost (0) + 10 = 30
    expect(rawTarget(0, 0, true, () => 0.5)).toBeCloseTo(30)
  })

  it('collapses every target to a floor of 1 when the engine is bypassed', () => {
    expect(rawTarget(0, 0, false, () => 0.99)).toBe(1)
  })

  it('smooths the first active frame with the prototype 0.15 coefficient', () => {
    const signal = createFftSignal(() => 0.5)
    signal.step(true)

    // bin 0 target 30 → 0 + (30 - 0) * 0.15 = 4.5
    expect(signal.bins[0]).toBeCloseTo(4.5)

    // The prototype only advances BAR_COUNT bars; the tail stays silent.
    expect(signal.bins[BAR_COUNT]).toBe(0)
    expect(signal.bins[BIN_COUNT - 1]).toBe(0)
  })

  it('advances the phase so later frames differ from the first', () => {
    const signal = createFftSignal(() => 0.5)
    signal.step(true)
    const first = signal.bins[0]
    signal.step(true)

    expect(signal.bins[0]).not.toBeCloseTo(first)
  })

  it('exposes the phase step used by the oscillator', () => {
    expect(PHASE_STEP).toBe(0.1)
  })

  it('decays bins toward the idle floor when the engine is bypassed', () => {
    const signal = createFftSignal(() => 0.5)
    signal.step(true) // bin 0 = 4.5
    signal.step(false)

    // 4.5 + (1 - 4.5) * SMOOTHING = 3.975
    expect(signal.bins[0]).toBeCloseTo(4.5 + (1 - 4.5) * SMOOTHING)
  })

  it('returns a fresh Float32Array-backed signal per call', () => {
    const a = createFftSignal()
    const b = createFftSignal()
    expect(a.bins).not.toBe(b.bins)
  })
})
