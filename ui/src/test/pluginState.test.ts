import { describe, it, expect } from 'vitest'
import { initialState, diffPluginState } from '../lib/pluginState'

/**
 * T02 — module power state contract. The four power flags live on the top-level
 * PluginState, default to on, and flow to the bridge exactly like every other
 * parameter via the App-boundary diff.
 */
describe('pluginState module power flags', () => {
  it('defaults all four module power flags to true', () => {
    expect(initialState.driveOn).toBe(true)
    expect(initialState.bitcrushOn).toBe(true)
    expect(initialState.delayOn).toBe(true)
    expect(initialState.chorusOn).toBe(true)
  })

  it('produces no bridge calls for an unchanged state (initial mount boundary)', () => {
    expect(diffPluginState(initialState, initialState)).toEqual([])
  })

  it('notifies the bridge when the Drive power flag flips', () => {
    const calls = diffPluginState(initialState, { ...initialState, driveOn: false })
    expect(calls).toContainEqual({ parameterId: 'driveOn', value: false })
  })

  it('notifies the bridge when the Bitcrush power flag flips', () => {
    const calls = diffPluginState(initialState, { ...initialState, bitcrushOn: false })
    expect(calls).toContainEqual({ parameterId: 'bitcrushOn', value: false })
  })

  it('notifies the bridge when the Delay power flag flips', () => {
    const calls = diffPluginState(initialState, { ...initialState, delayOn: false })
    expect(calls).toContainEqual({ parameterId: 'delayOn', value: false })
  })

  it('notifies the bridge when the Chorus power flag flips', () => {
    const calls = diffPluginState(initialState, { ...initialState, chorusOn: false })
    expect(calls).toContainEqual({ parameterId: 'chorusOn', value: false })
  })

  it('flipping a power flag back on produces a matching bridge call', () => {
    const off = { ...initialState, driveOn: false }
    const calls = diffPluginState(off, { ...off, driveOn: true })
    expect(calls).toContainEqual({ parameterId: 'driveOn', value: true })
  })
})
