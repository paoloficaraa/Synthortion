import { describe, it, expect } from 'vitest'
import {
  createOscilloscopeSignal,
  waveformAt,
  rawSample,
  SAMPLE_COUNT,
  SMOOTHING,
  PHASE_STEP,
  NOISE_AMOUNT,
} from '../lib/oscilloscopeSignal'

describe('oscilloscopeSignal — time-domain trace generator', () => {
  it('starts with a zeroed sample array of oscilloscope length (240)', () => {
    const signal = createOscilloscopeSignal()

    expect(signal.samples).toHaveLength(SAMPLE_COUNT)
    expect(Array.from(signal.samples).every((sample) => sample === 0)).toBe(true)
  })

  it('computes the time-domain waveform for a known index/phase', () => {
    // sin(0 + 0) * 0.6 + cos(0 - 0) * 0.4 = 0.4
    expect(waveformAt(0, 0)).toBeCloseTo(0.4)
    // index 25, phase 0: 0.6 * sin(5) + 0.4 * cos(-2.5)
    expect(waveformAt(0, 25)).toBeCloseTo(
      0.6 * Math.sin(5) + 0.4 * Math.cos(-2.5)
    )
  })

  it('builds an active raw sample from waveform + zero noise', () => {
    // random() returns 0.5 so the noise term (random - 0.5) * NOISE_AMOUNT is 0
    expect(rawSample(0, 0, true, () => 0.5)).toBeCloseTo(0.4)
  })

  it('collapses every sample to a flat idle line of 0 when bypassed', () => {
    expect(rawSample(0, 0, false, () => 0.99)).toBe(0)
  })

  it('smooths the first active frame with the 0.4 coefficient', () => {
    const signal = createOscilloscopeSignal(() => 0.5)
    signal.step(true)

    // sample 0 target 0.4 → 0 + (0.4 - 0) * 0.4 = 0.16
    expect(signal.samples[0]).toBeCloseTo(0.16)
  })

  it('advances the phase so later frames differ from the first', () => {
    const signal = createOscilloscopeSignal(() => 0.5)
    signal.step(true)
    const first = signal.samples[0]
    signal.step(true)

    expect(signal.samples[0]).not.toBeCloseTo(first)
  })

  it('exposes the phase step used by the oscillator', () => {
    expect(PHASE_STEP).toBe(0.12)
  })

  it('decays samples toward the flat idle line when the engine is bypassed', () => {
    const signal = createOscilloscopeSignal(() => 0.5)
    signal.step(true) // sample 0 = 0.16
    signal.step(false)

    // 0.16 + (0 - 0.16) * 0.4 = 0.096
    expect(signal.samples[0]).toBeCloseTo(0.16 * (1 - SMOOTHING))
  })

  it('scales active samples by the noise amount', () => {
    // random() returns 1 so noise term is +NOISE_AMOUNT / 2
    expect(rawSample(0, 0, true, () => 1)).toBeCloseTo(0.4 + NOISE_AMOUNT / 2)
  })

  it('returns a fresh Float32Array-backed signal per call', () => {
    const a = createOscilloscopeSignal()
    const b = createOscilloscopeSignal()
    expect(a.samples).not.toBe(b.samples)
  })
})
