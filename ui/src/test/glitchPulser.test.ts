import { describe, it, expect } from 'vitest'
import { createGlitchPulser } from '../lib/glitchPulser'

describe('createGlitchPulser', () => {
  it('starts at zero intensity', () => {
    const pulser = createGlitchPulser()
    expect(pulser.intensity).toBe(0)
  })

  it('clamps a pulse into 0..1', () => {
    const pulser = createGlitchPulser()
    pulser.pulse(2)
    expect(pulser.intensity).toBe(1)
    pulser.pulse(-1)
    expect(pulser.intensity).toBe(0)
  })

  it('decays exponentially toward zero after a pulse', () => {
    const pulser = createGlitchPulser()
    pulser.pulse(1)
    expect(pulser.intensity).toBe(1)

    const after = pulser.step(150)
    // exp(-1) ≈ 0.3679 — strictly below the pulse, still above the threshold
    expect(after).toBeGreaterThan(0)
    expect(after).toBeLessThan(1)
    expect(pulser.intensity).toBeCloseTo(Math.exp(-1), 3)
  })

  it('snaps to zero below the threshold', () => {
    const pulser = createGlitchPulser()
    pulser.pulse(1)
    // ~10 half-lives: decayed far below MIN_THRESHOLD (0.001)
    for (let i = 0; i < 20; i++) pulser.step(150)
    expect(pulser.intensity).toBe(0)
  })

  it('accumulates nothing — a later pulse overwrites the prior level', () => {
    const pulser = createGlitchPulser()
    pulser.pulse(0.5)
    pulser.pulse(0.2)
    expect(pulser.intensity).toBe(0.2)
  })

  it('returns the same intensity from step and the getter', () => {
    const pulser = createGlitchPulser()
    pulser.pulse(0.8)
    const fromStep = pulser.step(16)
    expect(pulser.intensity).toBe(fromStep)
  })
})
