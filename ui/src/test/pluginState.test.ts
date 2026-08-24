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

  it('notifies when the Drive power flag flips', () => {
    const diffs = diffPluginState(initialState, { ...initialState, driveOn: false })
    expect(diffs).toContainEqual({ key: 'driveOn', value: false })
  })

  it('notifies when the Bitcrush power flag flips', () => {
    const diffs = diffPluginState(initialState, { ...initialState, bitcrushOn: false })
    expect(diffs).toContainEqual({ key: 'bitcrushOn', value: false })
  })

  it('notifies when the Delay power flag flips', () => {
    const diffs = diffPluginState(initialState, { ...initialState, delayOn: false })
    expect(diffs).toContainEqual({ key: 'delayOn', value: false })
  })

  it('notifies when the Chorus power flag flips', () => {
    const diffs = diffPluginState(initialState, { ...initialState, chorusOn: false })
    expect(diffs).toContainEqual({ key: 'chorusOn', value: false })
  })

  it('flipping a power flag back on produces a matching diff', () => {
    const off = { ...initialState, driveOn: false }
    const diffs = diffPluginState(off, { ...off, driveOn: true })
    expect(diffs).toContainEqual({ key: 'driveOn', value: true })
  })
})
