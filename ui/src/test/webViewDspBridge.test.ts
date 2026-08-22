import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import {
  subscribeToDspSpectrum,
  subscribeToDspChanges,
  webViewDspBridge,
} from '../lib/webViewDspBridge'

describe('webViewDspBridge & subscribeToDspSpectrum', () => {
  beforeEach(() => {
    delete window.__JUCE__
    delete window.__SYNTORTION_BRIDGE__
    delete window.webkit
  })

  afterEach(() => {
    delete window.__JUCE__
    delete window.__SYNTORTION_BRIDGE__
    delete window.webkit
  })

  it('receives spectrum frames via JUCE 8 event listener', () => {
    const removeListener = vi.fn()
    let eventCallback: ((payload: unknown) => void) | undefined

    window.__JUCE__ = {
      backend: {
        addEventListener: vi.fn((event: string, cb: (payload: unknown) => void) => {
          if (event === 'spectrumFrame') {
            eventCallback = cb
          }
          return removeListener
        }),
      },
    }

    const onFrame = vi.fn()
    const unsubscribe = subscribeToDspSpectrum(onFrame)

    expect(window.__JUCE__.backend?.addEventListener).toHaveBeenCalledWith(
      'spectrumFrame',
      expect.any(Function)
    )

    const testMags = new Array(80).fill(0.5)
    eventCallback?.(testMags)

    expect(onFrame).toHaveBeenCalledTimes(1)
    expect(onFrame).toHaveBeenCalledWith(testMags)

    unsubscribe()
    expect(removeListener).toHaveBeenCalledTimes(1)
  })

  it('receives spectrum frames with { magnitudes: [...] } object payload', () => {
    let eventCallback: ((payload: unknown) => void) | undefined
    window.__JUCE__ = {
      backend: {
        addEventListener: vi.fn((event: string, cb: (payload: unknown) => void) => {
          if (event === 'spectrumFrame') {
            eventCallback = cb
          }
          return () => {}
        }),
      },
    }

    const onFrame = vi.fn()
    subscribeToDspSpectrum(onFrame)

    const testMags = new Array(80).fill(0.75)
    eventCallback?.({ magnitudes: testMags })

    expect(onFrame).toHaveBeenCalledWith(testMags)
  })

  it('receives spectrum frames via window.__SYNTORTION_BRIDGE__.onSpectrumFrame fallback', () => {
    const onFrame = vi.fn()
    const unsubscribe = subscribeToDspSpectrum(onFrame)

    expect(window.__SYNTORTION_BRIDGE__?.onSpectrumFrame).toBeDefined()

    const testMags = new Array(80).fill(0.42)
    window.__SYNTORTION_BRIDGE__?.onSpectrumFrame?.(testMags)

    expect(onFrame).toHaveBeenCalledWith(testMags)

    unsubscribe()
    expect(window.__SYNTORTION_BRIDGE__?.onSpectrumFrame).toBeUndefined()
  })

  it('coexists with subscribeToDspChanges without clobbering __SYNTORTION_BRIDGE__', () => {
    const onParamUpdate = vi.fn()
    const onSpectrumFrame = vi.fn()

    const unsubParams = subscribeToDspChanges(onParamUpdate)
    const unsubSpectrum = subscribeToDspSpectrum(onSpectrumFrame)

    expect(window.__SYNTORTION_BRIDGE__?.onParameterChange).toBeDefined()
    expect(window.__SYNTORTION_BRIDGE__?.onSpectrumFrame).toBeDefined()

    unsubSpectrum()
    expect(window.__SYNTORTION_BRIDGE__?.onSpectrumFrame).toBeUndefined()
    expect(window.__SYNTORTION_BRIDGE__?.onParameterChange).toBeDefined()

    unsubParams()
    expect(window.__SYNTORTION_BRIDGE__?.onParameterChange).toBeUndefined()
  })

  it('emits setParameter to JUCE backend when available', () => {
    const emitEvent = vi.fn()
    window.__JUCE__ = {
      backend: {
        emitEvent,
      },
    }

    webViewDspBridge.setParameter('INPUT_GAIN', 0.5)
    expect(emitEvent).toHaveBeenCalledWith('setParameter', {
      parameterId: 'INPUT_GAIN',
      value: 0.5,
    })
  })
})
