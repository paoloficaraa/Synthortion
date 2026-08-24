import type { DspBridge, ParameterValue } from './dspBridge'
import {
  fromAPVTS,
  parameterStore,
  type InitPayload,
  type UIPreferences,
} from './parameterStore'
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
  }
}

/** Real WebView bridge using JUCE 8 native WebBrowserComponent events. */
export const webViewDspBridge: DspBridge = {
  setParameter(id: string, value: ParameterValue) {
    if (window.__JUCE__?.backend?.emitEvent) {
      window.__JUCE__.backend.emitEvent('setParameter', { id, value })
    } else {
      console.warn('[Synthortion] JUCE backend not available')
    }
  },
}

/**
 * Register listener for incoming C++ DSP parameter changes (automation, presets, initial hydration).
 * Also performs the initial connection handshake by emitting `connect`.
 */
export function subscribeToDspChanges(
  onUpdate: (patch: Partial<PluginState>) => void
): () => void {
  const handleParamChange = (id: string, value: number) => {
    parameterStore.updateParameter(id, value)
    const res = fromAPVTS(id, value)
    if (res) {
      const patch: Partial<PluginState> = {}
      Object.assign(patch, { [res.uiKey]: res.value })
      onUpdate(patch)
    }
  }

  let juceCleanupParam: (() => void) | undefined
  let juceCleanupInit: (() => void) | undefined
  let juceCleanupPrefs: (() => void) | undefined

  if (window.__JUCE__?.backend?.addEventListener) {
    // 1. Listen for parameterChange events
    juceCleanupParam = window.__JUCE__.backend.addEventListener('parameterChange', (data) => {
      if (data && typeof data === 'object') {
        const id = 'id' in data && typeof data.id === 'string' ? data.id : undefined
        const value = 'value' in data && typeof data.value === 'number' ? data.value : undefined
        if (id !== undefined && value !== undefined) {
          handleParamChange(id, value)
        }
      }
    })

    // 2. Listen for init handshake event
    juceCleanupInit = window.__JUCE__.backend.addEventListener('init', (data) => {
      if (data && typeof data === 'object') {
        const payload = data as InitPayload
        parameterStore.hydrate(payload)
        if ('parameters' in data && Array.isArray(data.parameters)) {
          const patch: Partial<PluginState> = {}
          for (const param of data.parameters) {
            if (param && typeof param === 'object' && 'id' in param && typeof param.id === 'string') {
              const normVal = 'normalizedValue' in param && typeof param.normalizedValue === 'number'
                ? param.normalizedValue
                : 0
              const res = fromAPVTS(param.id, normVal)
              if (res) {
                Object.assign(patch, { [res.uiKey]: res.value })
              }
            }
          }
          if (Object.keys(patch).length > 0) {
            onUpdate(patch)
          }
        }
      }
    })

    // 3. Listen for runtime UI preference changes
    juceCleanupPrefs = window.__JUCE__.backend.addEventListener('uiPreferencesChange', (data) => {
      if (data && typeof data === 'object') {
        parameterStore.setUIPreferences(data as Partial<UIPreferences>)
      }
    })
  }

  // 4. Dispatch connection event to C++ backend
  if (window.__JUCE__?.backend?.emitEvent) {
    window.__JUCE__.backend.emitEvent('connect', {})
  }

  return () => {
    if (juceCleanupParam) juceCleanupParam()
    if (juceCleanupInit) juceCleanupInit()
    if (juceCleanupPrefs) juceCleanupPrefs()
  }
}

/** Callback type for real-time 80-band spectrum analyzer frames. */
export type SpectrumFrameCallback = (magnitudes: number[]) => void

/** Real-time peak levels for input and output rails. */
export interface MeterFrame {
  input: number
  output: number
}

/** Callback type for real-time meter updates. */
export type MeterFrameCallback = (frame: MeterFrame) => void

/**
 * Register listener for incoming 60 FPS C++ DSP meter frames (`meterFrame` event).
 */
export function subscribeToDspMeters(onFrame: MeterFrameCallback): () => void {
  const handler = (data: unknown) => {
    if (data && typeof data === 'object' && 'input' in data && 'output' in data) {
      const input = data.input
      const output = data.output
      if (typeof input === 'number' && typeof output === 'number') {
        onFrame({ input, output })
      }
    }
  }

  let cleanup: (() => void) | undefined
  if (window.__JUCE__?.backend?.addEventListener) {
    cleanup = window.__JUCE__.backend.addEventListener('meterFrame', handler)
  }
  return () => {
    if (cleanup) cleanup()
  }
}

/**
 * Register listener for incoming 60 FPS C++ DSP spectrum frames (`spectrumFrame` event).
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

  let cleanup: (() => void) | undefined
  if (window.__JUCE__?.backend?.addEventListener) {
    cleanup = window.__JUCE__.backend.addEventListener('spectrumFrame', handler)
  }
  return () => {
    if (cleanup) cleanup()
  }
}

/**
 * Register listener for runtime UI preferences changes (`uiPreferencesChange` event).
 */
export function subscribeToUIPreferences(
  onPreferences: (prefs: UIPreferences) => void
): () => void {
  const handler = (data: unknown) => {
    if (data && typeof data === 'object') {
      const prefs = data as Partial<UIPreferences>
      parameterStore.setUIPreferences(prefs)
      onPreferences(parameterStore.getUIPreferences())
    }
  }

  let cleanup: (() => void) | undefined
  if (window.__JUCE__?.backend?.addEventListener) {
    cleanup = window.__JUCE__.backend.addEventListener('uiPreferencesChange', handler)
  }
  const unsubStore = parameterStore.subscribePreferences(onPreferences)
  return () => {
    if (cleanup) cleanup()
    unsubStore()
  }
}
