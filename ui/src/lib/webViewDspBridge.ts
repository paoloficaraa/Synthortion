import type { DspBridge, ParameterValue } from './dspBridge'
import { fromAPVTS } from './parameterSpecs'
import type { PluginState } from './pluginState'

/** Global JUCE WebView interface injected by WebBrowserComponent in JUCE 8. */
declare global {
  interface Window {
    __JUCE__?: {
      backend?: {
        emitEvent?: (eventId: string, data: unknown) => void
        addEventListener?: (
          eventId: string,
          callback: (payload: unknown) => void
        ) => () => void
      }
    }
    webkit?: {
      messageHandlers?: {
        messageHandler?: {
          postMessage: (msg: string) => void
        }
      }
    }
    __SYNTORTION_BRIDGE__?: {
      onParameterChange?: (parameterId: string, value: number) => void
      onSpectrumFrame?: (magnitudes: number[]) => void
      onMeterFrame?: (payload: MeterFrame) => void
    }
  }
}

/** Real WebView bridge supporting JUCE 8 native integration & fallback protocols. */
export const webViewDspBridge: DspBridge = {
  setParameter(parameterId: string, value: ParameterValue) {
    const payload = { parameterId, value }
    const jsonString = JSON.stringify(payload)

    if (window.__JUCE__?.backend?.emitEvent) {
      // JUCE 8 native WebView event emission
      window.__JUCE__.backend.emitEvent('setParameter', payload)
    } else if (window.webkit?.messageHandlers?.messageHandler) {
      // WebKit native handler
      window.webkit.messageHandlers.messageHandler.postMessage(jsonString)
    } else if (typeof window !== 'undefined' && window.postMessage) {
      // General postMessage fallback
      window.postMessage(jsonString, '*')
    } else {
      console.warn('[Synthortion] WebView bridge not available')
    }
  },
}

/**
 * Register listener for incoming C++ DSP parameter changes (automation, presets, initial hydration).
 */
export function subscribeToDspChanges(
  onUpdate: (patch: Partial<PluginState>) => void
): () => void {
  const handleParamChange = (parameterId: string, value: number) => {
    const res = fromAPVTS(parameterId, value)
    if (res) {
      onUpdate({ [res.uiKey]: res.value })
    }
  }

  // 1. Register JUCE 8 event listener if available
  let juceCleanup: (() => void) | undefined
  if (window.__JUCE__?.backend?.addEventListener) {
    juceCleanup = window.__JUCE__.backend.addEventListener('parameterChange', (data) => {
      if (data && typeof data === 'object' && 'parameterId' in data && 'value' in data) {
        const parameterId = data.parameterId
        const value = data.value
        if (typeof parameterId === 'string' && typeof value === 'number') {
          handleParamChange(parameterId, value)
        }
      }
    })
  } else {
    // 2. Register global function callback fallback
    if (!window.__SYNTORTION_BRIDGE__) {
      window.__SYNTORTION_BRIDGE__ = {}
    }
    window.__SYNTORTION_BRIDGE__.onParameterChange = handleParamChange
  }
  // Notify backend of connection
  if (window.__JUCE__?.backend?.emitEvent) {
    window.__JUCE__.backend.emitEvent('connect', {})
  }

  return () => {
    if (juceCleanup) juceCleanup()
    if (window.__SYNTORTION_BRIDGE__) {
      delete window.__SYNTORTION_BRIDGE__.onParameterChange
      if (!window.__SYNTORTION_BRIDGE__.onSpectrumFrame) {
        delete window.__SYNTORTION_BRIDGE__
      }
    }
  }
}

/** Callback type for real-time 80-band spectrum analyzer frames. */
export type SpectrumFrameCallback = (magnitudes: number[]) => void

/** Shared helper: JUCE event listener with fallback to __SYNTORTION_BRIDGE__ */
function bridgeSubscribe(
  eventId: string,
  handler: (payload: unknown) => void,
  setupFallback: () => void,
  clearFallback: () => void
): () => void {
  let juceCleanup: (() => void) | undefined
  if (window.__JUCE__?.backend?.addEventListener) {
    juceCleanup = window.__JUCE__.backend.addEventListener(eventId, handler)
  } else {
    setupFallback()
  }
  return () => {
    if (juceCleanup) juceCleanup()
    clearFallback()
  }
}

/** Real-time peak levels for input and output rails. */
export interface MeterFrame {
  /** Normalized peak [0.0, 4.0] for input stage ( >1.0 = clip). */
  input: number
  /** Normalized peak [0.0, 4.0] for output stage. */
  output: number
}

/** Callback type for real-time meter updates. */
export type MeterFrameCallback = (frame: MeterFrame) => void

/**
 * Register listener for incoming 60 FPS C++ DSP meter frames (`meterFrame` event).
 * Designed for canvas/animation-frame subscribers to bypass React re-rendering.
 *
 * @param onFrame Callback receiving MeterFrame with input/output peaks [0.0, 4.0] ( >1.0 clip).
 * @returns Cleanup function to unsubscribe.
 */
export function subscribeToDspMeters(onFrame: MeterFrameCallback): () => void {
  const handler = (data: unknown) => {
    if (data && typeof data === 'object' && 'input' in data && 'output' in data) {
      const candidate = data as unknown as { input: unknown; output: unknown }
      if (typeof candidate.input === 'number' && typeof candidate.output === 'number') {
        onFrame({ input: candidate.input, output: candidate.output })
      }
    }
  }
  return bridgeSubscribe(
    'meterFrame',
    handler,
    () => {
      if (!window.__SYNTORTION_BRIDGE__) window.__SYNTORTION_BRIDGE__ = {}
      window.__SYNTORTION_BRIDGE__.onMeterFrame = (payload: MeterFrame) => handler(payload)
    },
    () => {
      if (window.__SYNTORTION_BRIDGE__?.onMeterFrame) delete window.__SYNTORTION_BRIDGE__.onMeterFrame
    }
  )
}



/**
 * Register listener for incoming 60 FPS C++ DSP spectrum frames (`spectrumFrame` event).
 * Designed for canvas/animation-frame subscribers to bypass React re-rendering.
 *
 * @param onFrame Callback receiving an array of 80 normalized magnitudes [0.0, 1.0].
 * @returns Cleanup function to unsubscribe and release bridge bindings.
 */
export function subscribeToDspSpectrum(onFrame: SpectrumFrameCallback): () => void {
  const handler = (data: unknown) => {
    if (Array.isArray(data)) {
      onFrame(data.filter((x): x is number => typeof x === 'number'))
    } else if (data && typeof data === 'object' && 'magnitudes' in data) {
      const mags = data.magnitudes
      if (Array.isArray(mags)) {
        onFrame(mags.filter((x): x is number => typeof x === 'number'))
      }
    }
  }
  return bridgeSubscribe(
    'spectrumFrame',
    handler,
    () => {
      if (!window.__SYNTORTION_BRIDGE__) window.__SYNTORTION_BRIDGE__ = {}
      window.__SYNTORTION_BRIDGE__.onSpectrumFrame = (magnitudes: number[]) => handler(magnitudes)
    },
    () => {
      if (window.__SYNTORTION_BRIDGE__?.onSpectrumFrame) {
        delete window.__SYNTORTION_BRIDGE__.onSpectrumFrame
        if (!window.__SYNTORTION_BRIDGE__.onParameterChange) delete (window as unknown as Record<string, unknown>).__SYNTORTION_BRIDGE__
      }
    }
  )
}
