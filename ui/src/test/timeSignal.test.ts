import { describe, it, expect } from 'vitest'
import {
  createTimeSignal,
  rawSample,
  SAMPLE_COUNT,
  AMPLITUDE,
  SMOOTHING,
  PHASE_STEP,
} from '../lib/timeSignal'

describe('timeSignal — oscilloscope waveform generator', () => {
  it('starts with a zeroed sample array of SAMPLE_COUNT length', () => {
    const signal = createTimeSignal()

    expect(signal.samples).toHaveLength(SAMPLE_COUNT)
    expect(Array.from(signal.samples).every((s) => s === 0)).toBe(true)
  })

  it('computes a raw sample for a known index/phase', () => {
    // index 0, phase 0: sin(0)*0.4 + sin(0)*0.25 + cos(0)*0.15 + noise*0.08, all * AMPLITUDE
    // = (0 + 0 + 0.15 + 0) * AMPLITUDE = 0.15 * AMPLITUDE
    const expected = 0.15 * AMPLITUDE
    expect(rawSample(0, 0, true, () => 0.5)).toBeCloseTo(expected)
  })

  it('returns 0 when the engine is bypassed', () => {
    expect(rawSample(0, 0, false, () => 0.99)).toBe(0)
  })

  it('produces samples in the expected normalised range', () => {
    const signal = createTimeSignal(() => 0.5)
    signal.step(true)

    for (let i = 0; i < SAMPLE_COUNT; i++) {
      expect(signal.samples[i]).toBeGreaterThanOrEqual(-1.05)
      expect(signal.samples[i]).toBeLessThanOrEqual(1.05)
    }
  })

  it('smooths the first active frame with the smoothing coefficient', () => {
    const signal = createTimeSignal(() => 0.5)
    signal.step(true)

    // The target for bin 0 is non-zero; after one frame: 0 + (target - 0) * SMOOTHING
    // Since smoothing is applied per-sample, verify the first sample is non-zero and smoothed
    expect(signal.samples[0]).not.toBe(0)
    expect(Math.abs(signal.samples[0])).toBeLessThan(1)
  })

  it('advances the phase so later frames differ from the first', () => {
    const signal = createTimeSignal(() => 0.5)
    signal.step(true)
    const first = signal.samples[0]
    signal.step(true)

    expect(signal.samples[0]).not.toBeCloseTo(first)
  })

  it('exposes the phase step and smoothing coefficient', () => {
    expect(PHASE_STEP).toBe(0.08)
    expect(SMOOTHING).toBe(0.15)
  })

  it('returns a fresh Float32Array-backed signal per call', () => {
    const a = createTimeSignal()
    const b = createTimeSignal()
    expect(a.samples).not.toBe(b.samples)
  })

  it('decays toward zero when bypassed', () => {
    const signal = createTimeSignal(() => 0.5)
    signal.step(true)
    const active = signal.samples[0]
    signal.step(false)

    expect(Math.abs(signal.samples[0])).toBeLessThan(Math.abs(active))
  })
})