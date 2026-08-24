import { describe, it, expect, beforeEach } from 'vitest'
import {
  parameterStore,
  toAPVTS,
  fromAPVTS,
  DEFAULT_PARAMETER_DESCRIPTORS,
  type InitPayload,
  type FloatParameterDescriptor,
  type ChoiceParameterDescriptor,
} from '../lib/parameterStore'

describe('parameterStore dynamic normalization & hydration', () => {
  beforeEach(() => {
    parameterStore.reset()
  })

  it('converts gain dB ranges correctly with defaults', () => {
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

  it('has descriptors for all 16 APVTS parameters', () => {
    expect(Object.keys(DEFAULT_PARAMETER_DESCRIPTORS)).toHaveLength(16)
    expect(parameterStore.getAllDescriptors()).toHaveLength(16)
  })

  it('hydrates dynamic parameter descriptors and recalculates ranges', () => {
    const customDrive: FloatParameterDescriptor = {
      id: 'COLOR',
      name: 'Custom Drive',
      type: 'float',
      min: 0,
      max: 200,
      defaultValue: 100,
      currentValue: 100,
      skew: 0.5,
      step: 0.5,
      unit: '%',
      normalizedDefault: 0.5,
      normalizedValue: 0.5,
    }

    const payload: InitPayload = {
      schemaVersion: 1,
      parameters: [customDrive],
      uiPreferences: {
        uiScale: 1.25,
        spectrumDecay: 0.4,
        skipBootSequence: true,
      },
    }

    parameterStore.hydrate(payload)

    const desc = parameterStore.getDescriptor('COLOR') as FloatParameterDescriptor
    expect(desc).toBeDefined()
    expect(desc.max).toBe(200)
    expect(desc.skew).toBe(0.5)

    // With skew 0.5: proportion = (100 - 0)/200 = 0.5 -> normalized = 0.5^0.5 ~= 0.7071
    const normalized = toAPVTS('drive', 100)
    expect(normalized).toBeCloseTo(Math.pow(0.5, 0.5))

    // Denormalize: norm 0.5 -> proportion = 0.5^(1/0.5) = 0.5^2 = 0.25 -> val = 0 + 0.25*200 = 50
    const denorm = fromAPVTS('COLOR', 0.5)
    expect(denorm).toEqual({ uiKey: 'drive', value: 50 })

    expect(parameterStore.getUIPreferences()).toEqual({
      uiScale: 1.25,
      spectrumDecay: 0.4,
      skipBootSequence: true,
    })
  })

  it('handles choice descriptors dynamically', () => {
    const customSync: ChoiceParameterDescriptor = {
      id: 'DELAY_SYNC',
      name: 'Sync Mode',
      type: 'choice',
      choices: ['FREE', '1/4', '1/8', '1/16', 'TRIPLET'],
      defaultIndex: 0,
      currentIndex: 0,
      normalizedDefault: 0,
      normalizedValue: 0,
    }

    parameterStore.hydrate({
      schemaVersion: 1,
      parameters: [customSync],
    })

    expect(toAPVTS('delaySync', 'FREE')).toBe(0.0)
    expect(toAPVTS('delaySync', '1/8')).toBe(0.5)
    expect(toAPVTS('delaySync', 'TRIPLET')).toBe(1.0)

    expect(fromAPVTS('DELAY_SYNC', 0.0)).toEqual({ uiKey: 'delaySync', value: 'FREE' })
    expect(fromAPVTS('DELAY_SYNC', 0.5)).toEqual({ uiKey: 'delaySync', value: '1/8' })
    expect(fromAPVTS('DELAY_SYNC', 1.0)).toEqual({ uiKey: 'delaySync', value: 'TRIPLET' })
  })

  it('updates parameter state and notifies listeners on updateParameter', () => {
    let notified = false
    const unsub = parameterStore.subscribe(() => {
      notified = true
    })

    parameterStore.updateParameter('INPUT_GAIN', 1.0)
    const desc = parameterStore.getDescriptor('INPUT_GAIN') as FloatParameterDescriptor
    expect(desc.normalizedValue).toBe(1.0)
    expect(desc.currentValue).toBe(12)
    expect(notified).toBe(true)

    unsub()
  })

  it('updates UI preferences and notifies preference listeners', () => {
    const prefUpdates: Array<{ uiScale?: number }> = []
    const unsub = parameterStore.subscribePreferences((prefs) => {
      prefUpdates.push(prefs)
    })

    parameterStore.setUIPreferences({ uiScale: 1.5 })
    expect(parameterStore.getUIPreferences().uiScale).toBe(1.5)
    expect(prefUpdates).toHaveLength(1)
    expect(prefUpdates[0].uiScale).toBe(1.5)

    unsub()
  })
})
