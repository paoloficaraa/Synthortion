import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import {
  subscribeToDspSpectrum,
  subscribeToDspChanges,
  subscribeToDspMeters,
  subscribeToUIPreferences,
  webViewDspBridge,
} from '../lib/webViewDspBridge'
import { parameterStore, type InitPayload } from '../lib/parameterStore'
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

  it('receives spectrum frames with { magnitudes: [...] } object payload', () => {
    const onFrame = vi.fn()
    const unsubscribe = subscribeToDspSpectrum(onFrame)

    const testMags = new Array(80).fill(0.75)
    mockBackend.trigger('spectrumFrame', { magnitudes: testMags })

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
