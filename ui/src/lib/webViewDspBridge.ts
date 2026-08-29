import type { DspBridge } from './dspBridge'
import {
  fromAPVTS,
  parameterStore,
  type InitPayload,
  type UIPreferences,
  type PresetListUpdatedPayload,
  type PresetLoadedPayload,
  type PresetOperationResultPayload,
  type SavePresetData,
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
export const webViewDspBridge: DspBridge & {
  requestPresetList: () => void
  loadPreset: (id: string) => void
  savePreset: (data: SavePresetData) => void
  deletePreset: (id: string) => void
} = {
  setParameter(id: string, value: number) {
    if (window.__JUCE__?.backend?.emitEvent) {
      window.__JUCE__.backend.emitEvent('setParameter', { id, value })
    } else {
      console.warn('[Synthortion] JUCE backend not available')
    }
  },
  requestPresetList() {
    if (window.__JUCE__?.backend?.emitEvent) {
      window.__JUCE__.backend.emitEvent('requestPresetList', {})
    } else {
      console.warn('[Synthortion] JUCE backend not available')
    }
  },
  loadPreset(id: string) {
    if (window.__JUCE__?.backend?.emitEvent) {
      window.__JUCE__.backend.emitEvent('loadPreset', { id })
    } else {
      console.warn('[Synthortion] JUCE backend not available')
    }
  },
  savePreset(data: SavePresetData) {
    if (window.__JUCE__?.backend?.emitEvent) {
      window.__JUCE__.backend.emitEvent('savePreset', data)
    } else {
      console.warn('[Synthortion] JUCE backend not available')
    }
  },
  deletePreset(id: string) {
    if (window.__JUCE__?.backend?.emitEvent) {
      window.__JUCE__.backend.emitEvent('deletePreset', { id })
    } else {
      console.warn('[Synthortion] JUCE backend not available')
    }
  },
}

parameterStore.setBridge(webViewDspBridge)

/** Helper to subscribe to JUCE native events with safe cleanup. */
function addNativeEventListener(
  event: string,
  handler: (data: unknown) => void
): () => void {
  if (window.__JUCE__?.backend?.addEventListener) {
    const cleanup = window.__JUCE__.backend.addEventListener(event, handler)
    return () => {
      if (cleanup) cleanup()
    }
  }
  return () => {}
}

function handlePresetListUpdated(data: unknown, onCatalog?: (payload: PresetListUpdatedPayload) => void): void {
  if (data && typeof data === 'object' && 'presets' in data && Array.isArray(data.presets)) {
    const payload = data as PresetListUpdatedPayload
    parameterStore.setPresetCatalog(payload.presets, payload.activePresetId)
    onCatalog?.(payload)
  }
}

function handlePresetLoaded(data: unknown, onLoaded?: (payload: PresetLoadedPayload) => void): void {
  if (
    data &&
    typeof data === 'object' &&
    'id' in data &&
    typeof data.id === 'string' &&
    'name' in data &&
    typeof data.name === 'string'
  ) {
    const payload = data as PresetLoadedPayload
    parameterStore.setActivePreset({
      id: payload.id,
      name: payload.name,
      category: payload.category ?? 'User',
      isFactory: payload.isFactory ?? payload.id.startsWith('factory://'),
      isDirty: payload.isDirty ?? false,
    })
    onLoaded?.(payload)
  }
}

function handlePresetOperationResult(data: unknown, onResult?: (payload: PresetOperationResultPayload) => void): void {
  if (data && typeof data === 'object' && 'success' in data && 'operation' in data) {
    const payload = data as PresetOperationResultPayload
    if (payload.message) {
      parameterStore.setPresetOperationToast(payload.message)
    }
    onResult?.(payload)
  }
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

  const unsubs = [
    // 1. Listen for parameterChange events
    addNativeEventListener('parameterChange', (data) => {
      if (data && typeof data === 'object') {
        const id = 'id' in data && typeof data.id === 'string' ? data.id : undefined
        const value = 'value' in data && typeof data.value === 'number' ? data.value : undefined
        if (id !== undefined && value !== undefined) {
          handleParamChange(id, value)
        }
      }
    }),

    // 2. Listen for init handshake event
    addNativeEventListener('init', (data) => {
      if (data && typeof data === 'object') {
        const payload = data as InitPayload
        parameterStore.hydrate(payload)
        if ('parameters' in data && Array.isArray(data.parameters)) {
          const patch: Partial<PluginState> = {}
          for (const param of data.parameters) {
            if (param && typeof param === 'object' && 'id' in param && typeof param.id === 'string') {
              const normVal =
                'normalizedValue' in param && typeof param.normalizedValue === 'number'
                  ? param.normalizedValue
                  : 'value' in param && typeof param.value === 'number'
                    ? param.value
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
    }),

    // 3. Listen for preset events
    addNativeEventListener('presetListUpdated', (data) => handlePresetListUpdated(data)),
    addNativeEventListener('presetLoaded', (data) => handlePresetLoaded(data)),
    addNativeEventListener('presetOperationResult', (data) => handlePresetOperationResult(data)),
  ]

  // 4. Dispatch connection event to C++ backend
  if (window.__JUCE__?.backend?.emitEvent) {
    window.__JUCE__.backend.emitEvent('connect', {})
  }

  return () => {
    for (const unsub of unsubs) unsub()
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
  return addNativeEventListener('meterFrame', (data) => {
    if (data && typeof data === 'object' && 'input' in data && 'output' in data) {
      const input = data.input
      const output = data.output
      if (typeof input === 'number' && typeof output === 'number') {
        onFrame({ input, output })
      }
    }
  })
}

/**
 * Register listener for incoming 60 FPS C++ DSP spectrum frames (`spectrumFrame` event).
 */
export function subscribeToDspSpectrum(onFrame: SpectrumFrameCallback): () => void {
  return addNativeEventListener('spectrumFrame', (data) => {
    if (Array.isArray(data)) {
      onFrame(data.filter((x): x is number => typeof x === 'number'))
    }
  })
}

/**
 * Register listener for runtime UI preferences changes (`uiPreferencesChange` event).
 */
export function subscribeToUIPreferences(
  onPreferences: (prefs: UIPreferences) => void
): () => void {
  const unsubStore = parameterStore.subscribePreferences(onPreferences)
  const unsubNative = addNativeEventListener('uiPreferencesChange', (data) => {
    if (data && typeof data === 'object') {
      const prefs = data as Partial<UIPreferences>
      parameterStore.setUIPreferences(prefs)
    }
  })
  return () => {
    unsubStore()
    unsubNative()
  }
}
/**
 * Register listener for incoming preset catalog updates (`presetListUpdated` event).
 */
export function subscribeToPresetCatalog(
  onCatalog: (payload: PresetListUpdatedPayload) => void
): () => void {
  return addNativeEventListener('presetListUpdated', (data) => handlePresetListUpdated(data, onCatalog))
}

/**
 * Register listener for preset loaded notification (`presetLoaded` event).
 */
export function subscribeToPresetLoaded(
  onLoaded: (payload: PresetLoadedPayload) => void
): () => void {
  return addNativeEventListener('presetLoaded', (data) => handlePresetLoaded(data, onLoaded))
}

/**
 * Register listener for preset operation results (`presetOperationResult` event).
 */
export function subscribeToPresetOperationResult(
  onResult: (payload: PresetOperationResultPayload) => void
): () => void {
  return addNativeEventListener('presetOperationResult', (data) => handlePresetOperationResult(data, onResult))
}

