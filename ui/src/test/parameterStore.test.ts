import { describe, it, expect, beforeEach } from 'vitest'
import {
  parameterStore,
  toAPVTS,
  fromAPVTS,
  DEFAULT_PARAMETER_DESCRIPTORS,
  type InitPayload,
  type FloatParameterDescriptor,
  type ChoiceParameterDescriptor,
  type PresetHeader,
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

  it('converts drive and bitcrush percentage correctly', () => {
    expect(toAPVTS('drive', 0)).toBe(0.0)
    expect(toAPVTS('drive', 50)).toBe(0.5)
    expect(toAPVTS('drive', 100)).toBe(1.0)
    expect(fromAPVTS('COLOR', 0.5)).toEqual({ uiKey: 'drive', value: 50 })

    expect(toAPVTS('bitcrush', 0)).toBe(0.0)
    expect(toAPVTS('bitcrush', 50)).toBe(0.5)
    expect(toAPVTS('bitcrush', 100)).toBe(1.0)
    expect(fromAPVTS('BITCRUSH', 0.5)).toEqual({ uiKey: 'bitcrush', value: 50 })
  })

  it('converts chorus mix and chorus width percentages correctly', () => {
    expect(toAPVTS('chorus', 0)).toBe(0.0)
    expect(toAPVTS('chorus', 75)).toBe(0.75)
    expect(toAPVTS('chorus', 100)).toBe(1.0)
    expect(fromAPVTS('CHORUS_MIX', 0.75)).toEqual({ uiKey: 'chorus', value: 75 })

    expect(toAPVTS('chorusWidth', 0)).toBe(0.0)
    expect(toAPVTS('chorusWidth', 50)).toBe(0.5)
    expect(toAPVTS('chorusWidth', 100)).toBe(1.0)
    expect(fromAPVTS('CHORUS_WIDTH', 0.5)).toEqual({ uiKey: 'chorusWidth', value: 50 })
    expect(fromAPVTS('CHORUS_WIDE', 0.5)).toEqual({ uiKey: 'chorusWidth', value: 50 })
  })

  it('converts delay time correctly in SYNC and FREE modes including low millisecond values', () => {
    // SYNC mode (0..13 steps)
    expect(toAPVTS('delayTimeSync', 0)).toBe(0.0)
    expect(toAPVTS('delayTimeSync', 6)).toBeCloseTo(6 / 13)
    expect(toAPVTS('delayTimeSync', 13)).toBe(1.0)

    expect(fromAPVTS('DELAY_TIME_SYNC', 0.0)).toEqual({ uiKey: 'delayTimeSync', value: 0 })
    expect(fromAPVTS('DELAY_TIME_SYNC', 6 / 13)).toEqual({ uiKey: 'delayTimeSync', value: 6 })
    expect(fromAPVTS('DELAY_TIME_SYNC', 1.0)).toEqual({ uiKey: 'delayTimeSync', value: 13 })

    // FREE mode (1..2000 ms)
    expect(toAPVTS('delayTimeFree', 1)).toBe(0.0)
    expect(toAPVTS('delayTimeFree', 2000)).toBe(1.0)
    expect(fromAPVTS('DELAY_TIME_FREE', 0.0)).toEqual({ uiKey: 'delayTimeFree', value: 1 })
    expect(fromAPVTS('DELAY_TIME_FREE', 1.0)).toEqual({ uiKey: 'delayTimeFree', value: 2000 })
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

  it('handles delaySync boolean correctly with APVTS boolean polarity', () => {
    expect(toAPVTS('delaySync', true)).toBe(1.0)
    expect(toAPVTS('delaySync', false)).toBe(0.0)

    expect(fromAPVTS('DELAY_SYNC', 1.0)).toEqual({ uiKey: 'delaySync', value: true })
    expect(fromAPVTS('DELAY_SYNC', 0.0)).toEqual({ uiKey: 'delaySync', value: false })
  })

  it('has descriptors for all 17 APVTS parameters', () => {
    expect(Object.keys(DEFAULT_PARAMETER_DESCRIPTORS)).toHaveLength(17)
    expect(parameterStore.getAllDescriptors()).toHaveLength(17)
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
  it('handles boolean descriptors dynamically with invert flag', () => {
    const invertedBool = {
      id: 'PLUGIN_BYPASS',
      name: 'Custom Bypass',
      type: 'bool' as const,
      defaultValue: false,
      currentValue: false,
      invert: true,
      normalizedDefault: 0.0,
      normalizedValue: 0.0,
    }

    parameterStore.hydrate({
      schemaVersion: 1,
      parameters: [invertedBool],
    })

    expect(toAPVTS('engineActive', true)).toBe(0.0)
    expect(toAPVTS('engineActive', false)).toBe(1.0)
    expect(fromAPVTS('PLUGIN_BYPASS', 0.0)).toEqual({ uiKey: 'engineActive', value: true })
    expect(fromAPVTS('PLUGIN_BYPASS', 1.0)).toEqual({ uiKey: 'engineActive', value: false })
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

  it('initializes with default preset state properties', () => {
    expect(parameterStore.getPresetCatalog()).toEqual([])
    expect(parameterStore.getActivePresetId()).toBeNull()
    expect(parameterStore.getActivePresetName()).toBe('Init State')
    expect(parameterStore.getActivePresetCategory()).toBe('Init')
    expect(parameterStore.getIsFactoryPreset()).toBe(true)
    expect(parameterStore.getIsPresetDirty()).toBe(false)
    expect(parameterStore.getPresetOperationToast()).toBeNull()
  })

  it('sets preset catalog and active preset ID, notifying listeners', () => {
    const presetUpdates: number[] = []
    const unsub = parameterStore.subscribePresets(() => {
      presetUpdates.push(parameterStore.getPresetCatalog().length)
    })

    const mockCatalog: PresetHeader[] = [
      {
        id: 'factory://Init/00_Default_Init',
        name: 'Default Init',
        category: 'Init',
        author: 'Synthortion Core',
        description: 'Clean default template.',
        tags: ['Init', 'Default'],
        isFactory: true,
        filePath: '',
        favorite: false,
        createdAt: '2026-08-28T12:00:00Z',
        modifiedAt: '2026-08-28T12:00:00Z',
      },
      {
        id: 'factory://Bass/01_Sub_Destroyer',
        name: 'Sub Destroyer',
        category: 'Bass',
        author: 'Synthortion Core',
        description: 'Sub heavy bass.',
        tags: ['Bass'],
        isFactory: true,
        filePath: '',
        favorite: false,
        createdAt: '2026-08-28T12:00:00Z',
        modifiedAt: '2026-08-28T12:00:00Z',
      },
    ]

    parameterStore.setPresetCatalog(mockCatalog, 'factory://Init/00_Default_Init')

    expect(parameterStore.getPresetCatalog()).toEqual(mockCatalog)
    expect(parameterStore.getActivePresetId()).toBe('factory://Init/00_Default_Init')
    expect(presetUpdates).toHaveLength(1)
    expect(presetUpdates[0]).toBe(2)

    unsub()
  })

  it('updates active preset information and resets dirty state with snapshot', () => {
    parameterStore.setActivePreset({
      id: 'factory://Bass/01_Sub_Destroyer',
      name: 'Sub Destroyer',
      category: 'Bass',
      isFactory: true,
      isDirty: false,
    })

    expect(parameterStore.getActivePresetId()).toBe('factory://Bass/01_Sub_Destroyer')
    expect(parameterStore.getActivePresetName()).toBe('Sub Destroyer')
    expect(parameterStore.getActivePresetCategory()).toBe('Bass')
    expect(parameterStore.getIsFactoryPreset()).toBe(true)
    expect(parameterStore.getIsPresetDirty()).toBe(false)
  })

  it('tracks dirty flag when parameters are edited and clears when restored or preset loaded', () => {
    // Snapshot at initial default state
    parameterStore.setActivePreset({
      id: 'factory://Init/00_Default_Init',
      name: 'Default Init',
      category: 'Init',
      isFactory: true,
    })
    expect(parameterStore.getIsPresetDirty()).toBe(false)

    // Edit parameter via setNormalizedValue
    parameterStore.setNormalizedValue('COLOR', 0.9)
    expect(parameterStore.getIsPresetDirty()).toBe(true)

    // Restore parameter to original snapshot value (0.4)
    parameterStore.setNormalizedValue('COLOR', 0.4)
    expect(parameterStore.getIsPresetDirty()).toBe(false)

    // Edit via patchState
    parameterStore.patchState({ drive: 80 })
    expect(parameterStore.getIsPresetDirty()).toBe(true)

    // Loading a new preset cleans dirty state
    parameterStore.setActivePreset({
      id: 'factory://Lead/03_Cyber_Neon',
      name: 'Cyber Neon',
      category: 'Lead',
      isFactory: true,
    })
    expect(parameterStore.getIsPresetDirty()).toBe(false)
  })

  it('steps cyclically through presets within the active category', () => {
    const mockCatalog: PresetHeader[] = [
      {
        id: 'factory://Init/00_Default_Init',
        name: 'Default Init',
        category: 'Init',
        author: 'Synthortion Core',
        description: 'Clean default template.',
        tags: ['Init'],
        isFactory: true,
        filePath: '',
        favorite: false,
        createdAt: '2026-08-28T12:00:00Z',
        modifiedAt: '2026-08-28T12:00:00Z',
      },
      {
        id: 'factory://Bass/01_Sub_Destroyer',
        name: 'Sub Destroyer',
        category: 'Bass',
        author: 'Synthortion Core',
        description: 'Sub heavy bass.',
        tags: ['Bass'],
        isFactory: true,
        filePath: '',
        favorite: false,
        createdAt: '2026-08-28T12:00:00Z',
        modifiedAt: '2026-08-28T12:00:00Z',
      },
      {
        id: 'factory://Bass/02_Acid_Crush',
        name: 'Acid Crush',
        category: 'Bass',
        author: 'Synthortion Core',
        description: 'Resonant acid bass.',
        tags: ['Bass'],
        isFactory: true,
        filePath: '',
        favorite: false,
        createdAt: '2026-08-28T12:00:00Z',
        modifiedAt: '2026-08-28T12:00:00Z',
      },
      {
        id: 'factory://Lead/03_Cyber_Neon',
        name: 'Cyber Neon',
        category: 'Lead',
        author: 'Synthortion Core',
        description: 'Bright lead.',
        tags: ['Lead'],
        isFactory: true,
        filePath: '',
        favorite: false,
        createdAt: '2026-08-28T12:00:00Z',
        modifiedAt: '2026-08-28T12:00:00Z',
      },
    ]

    const loadCalls: string[] = []
    const mockBridge = {
      setParameter() {},
      loadPreset(id: string) {
        loadCalls.push(id)
      },
    }
    parameterStore.setBridge(mockBridge)
    parameterStore.setPresetCatalog(mockCatalog, 'factory://Bass/01_Sub_Destroyer')
    parameterStore.setActivePreset({
      id: 'factory://Bass/01_Sub_Destroyer',
      name: 'Sub Destroyer',
      category: 'Bass',
      isFactory: true,
    })

    // Step next within Bass -> Acid Crush
    const nextId = parameterStore.stepPreset('next')
    expect(nextId).toBe('factory://Bass/02_Acid_Crush')
    expect(loadCalls).toContain('factory://Bass/02_Acid_Crush')

    // Active preset moves to Acid Crush
    parameterStore.setActivePreset({
      id: 'factory://Bass/02_Acid_Crush',
      name: 'Acid Crush',
      category: 'Bass',
      isFactory: true,
    })

    // Step next again -> wraps around to Sub Destroyer
    const wrapId = parameterStore.stepPreset('next')
    expect(wrapId).toBe('factory://Bass/01_Sub_Destroyer')

    // Step prev -> wraps around to Acid Crush
    parameterStore.setActivePreset({
      id: 'factory://Bass/01_Sub_Destroyer',
      name: 'Sub Destroyer',
      category: 'Bass',
      isFactory: true,
    })
    const prevId = parameterStore.stepPreset('prev')
    expect(prevId).toBe('factory://Bass/02_Acid_Crush')
  })

  it('invokes bridge savePreset and deletePreset actions', () => {
    const saved: unknown[] = []
    const deleted: string[] = []

    const mockBridge = {
      setParameter() {},
      savePreset(data: unknown) {
        saved.push(data)
      },
      deletePreset(id: string) {
        deleted.push(id)
      },
    }
    parameterStore.setBridge(mockBridge)

    parameterStore.savePreset({
      name: 'My Custom Bass',
      category: 'Bass',
      author: 'Author',
      description: 'Test desc',
      tags: ['Bass'],
    })
    expect(saved).toHaveLength(1)
    expect(saved[0]).toEqual({
      name: 'My Custom Bass',
      category: 'Bass',
      author: 'Author',
      description: 'Test desc',
      tags: ['Bass'],
    })

    parameterStore.deletePreset('user://Bass/My_Custom_Bass')
    expect(deleted).toEqual(['user://Bass/My_Custom_Bass'])
  })

  it('manages presetOperationToast messages', () => {
    expect(parameterStore.getPresetOperationToast()).toBeNull()
    parameterStore.setPresetOperationToast('Preset deleted.')
    expect(parameterStore.getPresetOperationToast()).toBe('Preset deleted.')
    parameterStore.setPresetOperationToast(null)
    expect(parameterStore.getPresetOperationToast()).toBeNull()
  })
})
