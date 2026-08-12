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
          callback: (payload: { parameterId: string; value: number }) => void
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
      if (data && typeof data.parameterId === 'string' && typeof data.value === 'number') {
        handleParamChange(data.parameterId, data.value)
      }
    })
  }

  // 2. Register global function callback fallback
  window.__SYNTORTION_BRIDGE__ = {
    onParameterChange: handleParamChange,
  }

  // Notify backend of connection
  if (window.__JUCE__?.backend?.emitEvent) {
    window.__JUCE__.backend.emitEvent('connect', {})
  }

  return () => {
    if (juceCleanup) juceCleanup()
    delete window.__SYNTORTION_BRIDGE__
  }
}
