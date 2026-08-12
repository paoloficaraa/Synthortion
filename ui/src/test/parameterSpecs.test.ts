import { describe, it, expect } from 'vitest'
import { toAPVTS, fromAPVTS, PARAMETER_SPECS } from '../lib/parameterSpecs'

describe('parameterSpecs normalization', () => {
  it('converts gain dB ranges correctly', () => {
    expect(toAPVTS('inputGain', -60)).toBe(0.0)
    expect(toAPVTS('inputGain', 12)).toBe(1.0)
    expect(toAPVTS('inputGain', -24)).toBeCloseTo((-24 + 60) / 72)

    expect(fromAPVTS('INPUT_GAIN', 0.0)).toEqual({ uiKey: 'inputGain', value: -60 })
    expect(fromAPVTS('INPUT_GAIN', 1.0)).toEqual({ uiKey: 'inputGain', value: 12 })
  })

  it('converts drive percentage correctly', () => {
    expect(toAPVTS('drive', 0)).toBe(0.0)
    expect(toAPVTS('drive', 50)).toBe(0.5)
    expect(toAPVTS('drive', 100)).toBe(1.0)

    expect(fromAPVTS('COLOR', 0.5)).toEqual({ uiKey: 'drive', value: 50 })
  })

  it('converts delay time correctly', () => {
    expect(toAPVTS('delayTime', 1)).toBe(0.0)
    expect(toAPVTS('delayTime', 2000)).toBe(1.0)

    expect(fromAPVTS('DELAY_TIME', 0.0)).toEqual({ uiKey: 'delayTime', value: 1 })
    expect(fromAPVTS('DELAY_TIME', 1.0)).toEqual({ uiKey: 'delayTime', value: 2000 })
  })

  it('handles inverted engineActive / PLUGIN_BYPASS bool', () => {
    // engineActive true -> PLUGIN_BYPASS false (0.0)
    expect(toAPVTS('engineActive', true)).toBe(0.0)
    expect(toAPVTS('engineActive', false)).toBe(1.0)

    expect(fromAPVTS('PLUGIN_BYPASS', 0.0)).toEqual({ uiKey: 'engineActive', value: true })
    expect(fromAPVTS('PLUGIN_BYPASS', 1.0)).toEqual({ uiKey: 'engineActive', value: false })
  })

  it('handles driveRoute enums correctly', () => {
    expect(toAPVTS('driveRoute', 'PRE')).toBe(0.0)
    expect(toAPVTS('driveRoute', 'POST')).toBe(1.0)

    expect(fromAPVTS('DRIVE_ROUTE', 0.0)).toEqual({ uiKey: 'driveRoute', value: 'PRE' })
    expect(fromAPVTS('DRIVE_ROUTE', 1.0)).toEqual({ uiKey: 'driveRoute', value: 'POST' })
  })

  it('handles delaySync enums correctly', () => {
    expect(toAPVTS('delaySync', 'SYNC')).toBe(0.0)
    expect(toAPVTS('delaySync', 'FREE')).toBe(0.5)
    expect(toAPVTS('delaySync', 'PING-PONG')).toBe(1.0)

    expect(fromAPVTS('DELAY_SYNC', 0.0)).toEqual({ uiKey: 'delaySync', value: 'SYNC' })
    expect(fromAPVTS('DELAY_SYNC', 0.5)).toEqual({ uiKey: 'delaySync', value: 'FREE' })
    expect(fromAPVTS('DELAY_SYNC', 1.0)).toEqual({ uiKey: 'delaySync', value: 'PING-PONG' })
  })

  it('has specs for all 16 APVTS parameters', () => {
    expect(Object.keys(PARAMETER_SPECS)).toHaveLength(16)
  })
})
