import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import {
  subscribeToDspSpectrum,
  subscribeToDspChanges,
  subscribeToDspMeters,
  subscribeToUIPreferences,
  subscribeToPresetCatalog,
  subscribeToPresetLoaded,
  subscribeToPresetOperationResult,
  webViewDspBridge,
} from '../lib/webViewDspBridge'
import {
  parameterStore,
  type InitPayload,
  type PresetHeader,
  type PresetListUpdatedPayload,
  type PresetLoadedPayload,
  type PresetOperationResultPayload,
} from '../lib/parameterStore'
import { createMockJuceBackend, type MockJuceBackend } from './setup'

describe('webViewDspBridge & Native JUCE 8 Event Protocol', () => {
  let mockBackend: MockJuceBackend

  beforeEach(() => {
    parameterStore.reset()
    mockBackend = createMockJuceBackend()
    window.__JUCE__ = {
      backend: {
        emitEvent: mockBackend.emitEvent,
        addEventListener: mockBackend.addEventListener,
      },
    }
  })

  afterEach(() => {
    delete window.__JUCE__
  })

  it('emits connect event upon subscribing to DSP changes', () => {
    const onUpdate = vi.fn()
    const unsubscribe = subscribeToDspChanges(onUpdate)

    expect(mockBackend.emitEvent).toHaveBeenCalledWith('connect', {})
    unsubscribe()
  })

  it('hydrates parameter store and triggers initial state update on init handshake event', () => {
    const onUpdate = vi.fn()
    const unsubscribe = subscribeToDspChanges(onUpdate)

    const initPayload: InitPayload = {
      schemaVersion: 1,
      parameters: [
        {
          id: 'COLOR',
          name: 'Drive',
          type: 'float',
          min: 0,
          max: 100,
          defaultValue: 40,
          currentValue: 80,
          skew: 1,
          step: 1,
          unit: '%',
          normalizedDefault: 0.4,
          normalizedValue: 0.8,
        },
      ],
      uiPreferences: {
        uiScale: 1.2,
        spectrumDecay: 0.3,
        skipBootSequence: true,
      },
    }

    mockBackend.trigger('init', initPayload)

    expect(onUpdate).toHaveBeenCalledWith(
      expect.objectContaining({
        drive: 80,
      })
    )
    expect(parameterStore.getUIPreferences()).toEqual({
      uiScale: 1.2,
      spectrumDecay: 0.3,
      skipBootSequence: true,
    })

    unsubscribe()
  })

  it('receives parameterChange events and updates state and parameter store', () => {
    const onUpdate = vi.fn()
    const unsubscribe = subscribeToDspChanges(onUpdate)

    mockBackend.trigger('parameterChange', {
      id: 'INPUT_GAIN',
      value: 1.0, // normalized max (+12 dB)
    })

    expect(onUpdate).toHaveBeenCalledWith({ inputGain: 12 })
    expect(parameterStore.getDescriptor('INPUT_GAIN')?.normalizedValue).toBe(1.0)

    unsubscribe()
  })

  it('receives spectrum frames via spectrumFrame event', () => {
    const onFrame = vi.fn()
    const unsubscribe = subscribeToDspSpectrum(onFrame)

    expect(mockBackend.addEventListener).toHaveBeenCalledWith(
      'spectrumFrame',
      expect.any(Function)
    )

    const testMags = new Array(80).fill(0.5)
    mockBackend.trigger('spectrumFrame', testMags)

    expect(onFrame).toHaveBeenCalledTimes(1)
    expect(onFrame).toHaveBeenCalledWith(testMags)

    unsubscribe()
  })


  it('receives meter frames via meterFrame event', () => {
    const onFrame = vi.fn()
    const unsubscribe = subscribeToDspMeters(onFrame)

    expect(mockBackend.addEventListener).toHaveBeenCalledWith('meterFrame', expect.any(Function))

    mockBackend.trigger('meterFrame', { input: 0.8, output: 0.6 })

    expect(onFrame).toHaveBeenCalledWith({ input: 0.8, output: 0.6 })

    unsubscribe()
  })

  it('receives uiPreferencesChange events via subscribeToUIPreferences', () => {
    const onPrefs = vi.fn()
    const unsubscribe = subscribeToUIPreferences(onPrefs)

    expect(mockBackend.addEventListener).toHaveBeenCalledWith(
      'uiPreferencesChange',
      expect.any(Function)
    )

    mockBackend.trigger('uiPreferencesChange', {
      uiScale: 1.5,
      spectrumDecay: 0.5,
    })
    expect(onPrefs).toHaveBeenCalledTimes(1)
    expect(onPrefs).toHaveBeenCalledWith(
      expect.objectContaining({
        uiScale: 1.5,
        spectrumDecay: 0.5,
      })
    )
    unsubscribe()
  })

  it('emits setParameter to JUCE backend with normalized value', () => {
    webViewDspBridge.setParameter('INPUT_GAIN', 0.5)
    expect(mockBackend.emitEvent).toHaveBeenCalledWith('setParameter', {
      id: 'INPUT_GAIN',
      value: 0.5,
    })
  })

  it('emits requestPresetList to JUCE backend', () => {
    webViewDspBridge.requestPresetList()
    expect(mockBackend.emitEvent).toHaveBeenCalledWith('requestPresetList', {})
  })

  it('emits loadPreset with preset id to JUCE backend', () => {
    webViewDspBridge.loadPreset('factory://Bass/01_Sub_Destroyer')
    expect(mockBackend.emitEvent).toHaveBeenCalledWith('loadPreset', {
      id: 'factory://Bass/01_Sub_Destroyer',
    })
  })

  it('emits savePreset with preset data payload to JUCE backend', () => {
    webViewDspBridge.savePreset({
      name: 'Custom Lead',
      category: 'Lead',
      author: 'Tester',
      description: 'Test preset',
      tags: ['Lead', 'Custom'],
      allowOverwrite: true,
    })
    expect(mockBackend.emitEvent).toHaveBeenCalledWith('savePreset', {
      name: 'Custom Lead',
      category: 'Lead',
      author: 'Tester',
      description: 'Test preset',
      tags: ['Lead', 'Custom'],
      allowOverwrite: true,
    })
  })

  it('emits deletePreset with preset id to JUCE backend', () => {
    webViewDspBridge.deletePreset('user://Lead/Custom_Lead')
    expect(mockBackend.emitEvent).toHaveBeenCalledWith('deletePreset', {
      id: 'user://Lead/Custom_Lead',
    })
  })

  it('subscribes to preset catalog updates and updates parameter store', () => {
    const onCatalog = vi.fn()
    const unsubscribe = subscribeToPresetCatalog(onCatalog)

    expect(mockBackend.addEventListener).toHaveBeenCalledWith(
      'presetListUpdated',
      expect.any(Function)
    )

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
        id: 'user://Lead/My_Lead',
        name: 'My Lead',
        category: 'Lead',
        author: 'User',
        description: 'User lead patch.',
        tags: ['Lead'],
        isFactory: false,
        filePath: '/path/to/My_Lead.synthortionpreset',
        favorite: true,
        createdAt: '2026-08-28T12:00:00Z',
        modifiedAt: '2026-08-28T12:00:00Z',
      },
    ]

    const payload: PresetListUpdatedPayload = {
      presets: mockCatalog,
      activePresetId: 'factory://Init/00_Default_Init',
    }

    mockBackend.trigger('presetListUpdated', payload)

    expect(onCatalog).toHaveBeenCalledWith(payload)
    expect(parameterStore.getPresetCatalog()).toHaveLength(2)
    expect(parameterStore.getActivePresetId()).toBe('factory://Init/00_Default_Init')

    unsubscribe()
  })

  it('subscribes to presetLoaded events and updates active preset state in store', () => {
    const onLoaded = vi.fn()
    const unsubscribe = subscribeToPresetLoaded(onLoaded)

    expect(mockBackend.addEventListener).toHaveBeenCalledWith(
      'presetLoaded',
      expect.any(Function)
    )

    const payload: PresetLoadedPayload = {
      id: 'factory://Bass/01_Sub_Destroyer',
      name: 'Sub Destroyer',
      category: 'Bass',
      isFactory: true,
      isDirty: false,
    }

    mockBackend.trigger('presetLoaded', payload)

    expect(onLoaded).toHaveBeenCalledWith(payload)
    expect(parameterStore.getActivePresetId()).toBe('factory://Bass/01_Sub_Destroyer')
    expect(parameterStore.getActivePresetName()).toBe('Sub Destroyer')
    expect(parameterStore.getActivePresetCategory()).toBe('Bass')
    expect(parameterStore.getIsFactoryPreset()).toBe(true)
    expect(parameterStore.getIsPresetDirty()).toBe(false)

    unsubscribe()
  })

  it('maintains clean isPresetDirty state when parameterChange events precede presetLoaded', () => {
    const onUpdate = vi.fn()
    const onLoaded = vi.fn()
    const unsubDsp = subscribeToDspChanges(onUpdate)
    const unsubLoaded = subscribeToPresetLoaded(onLoaded)

    // Incoming parameter changes from C++ preset loading
    mockBackend.trigger('parameterChange', { id: 'COLOR', value: 0.85 })
    mockBackend.trigger('parameterChange', { id: 'CHORUS_WIDE', value: 0.0 })

    // Subsequent presetLoaded event
    mockBackend.trigger('presetLoaded', {
      id: 'factory://Bass/01_Sub_Destroyer',
      name: 'Sub Destroyer',
      category: 'Bass',
      isFactory: true,
      isDirty: false,
    })

    expect(parameterStore.getIsPresetDirty()).toBe(false)

    // Editing parameter now flags dirty
    parameterStore.setNormalizedValue('COLOR', 0.5)
    expect(parameterStore.getIsPresetDirty()).toBe(true)

    unsubDsp()
    unsubLoaded()
  })

  it('subscribes to presetOperationResult events and updates store toast', () => {
    const onResult = vi.fn()
    const unsubscribe = subscribeToPresetOperationResult(onResult)

    expect(mockBackend.addEventListener).toHaveBeenCalledWith(
      'presetOperationResult',
      expect.any(Function)
    )

    const payload: PresetOperationResultPayload = {
      success: true,
      operation: 'save',
      message: 'Preset saved successfully.',
    }

    mockBackend.trigger('presetOperationResult', payload)

    expect(onResult).toHaveBeenCalledWith(payload)
    expect(parameterStore.getPresetOperationToast()).toBe('Preset saved successfully.')

    unsubscribe()
  })

  it('does not throw when window.__JUCE__ is undefined', () => {
    delete window.__JUCE__

    expect(() => {
      webViewDspBridge.setParameter('INPUT_GAIN', 0.5)
    }).not.toThrow()

    const onUpdate = vi.fn()
    expect(() => {
      const unsub = subscribeToDspChanges(onUpdate)
      unsub()
    }).not.toThrow()
  })
})
