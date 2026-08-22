import { useEffect, useMemo, useRef, useState } from 'react'
import { VstLayout } from './components/VstLayout'
import { Header } from './components/Header'
import { SystemBoot } from './components/SystemBoot'
import { GainMeter } from './components/GainMeter'
import { MatrixFaceplate } from './components/MatrixFaceplate'
import { FftVisualizer } from './components/FftVisualizer'
import { initialState, diffPluginState, type PluginState } from './lib/pluginState'
import { noopDspBridge, type DspBridge, PARAMETER_IDS } from './lib/dspBridge'
import { toAPVTS } from './lib/parameterSpecs'
import { subscribeToDspChanges } from './lib/webViewDspBridge'
import { createGlitchPulser } from './lib/glitchPulser'

/** Module power flags pulse a fixed 0.8 burst (spec: short glitch). */
const MODULE_POWER_KEYS: ReadonlySet<string> = new Set([
  'driveOn',
  'bitcrushOn',
  'delayOn',
  'chorusOn',
])

interface AppProps {
  /** Integration seam for the future C++ DSP bridge. */
  dspBridge?: DspBridge
}
/**
 * App — single top-level state boundary for the whole plugin.
 *
 * Every parameter the faceplate exposes lives here as controlled props and is
 * pushed to the injected DSP bridge on change. Child components keep no
 * silent state, so binding the real C++ backend is a matter of swapping the
 * bridge.
 */
function App({ dspBridge = noopDspBridge }: AppProps) {
  const [state, setState] = useState<PluginState>(initialState)
  const prevStateRef = useRef<PluginState>(initialState)
  // Stable for the component's lifetime — created once, mutated by pulse()
  // from the effect below, read by the visualizer as a prop.
  const pulser = useMemo(() => createGlitchPulser(), [])
  // proportionally to the value delta. The initial mount is skipped.
  useEffect(() => {
    const prev = prevStateRef.current
    prevStateRef.current = state
    for (const call of diffPluginState(prev, state)) {
      const apvtsId = PARAMETER_IDS[call.parameterId as keyof typeof PARAMETER_IDS]
      if (apvtsId) {
        const normalized = toAPVTS(call.parameterId as keyof PluginState, call.value)
        dspBridge.setParameter(apvtsId, normalized)
      }

      // Pulse the glitch pulser with |Δvalue| / 100.
      // Module power toggles fire a fixed 0.8 burst; other non-numeric
      // changes (master bypass, route ties) produce a heavy 1.0 burst.
      const prevVal = prev[call.parameterId as keyof PluginState]
      const newVal = call.value
      let delta
      if (typeof prevVal === 'number' && typeof newVal === 'number') {
        delta = Math.abs(newVal - prevVal) / 100
      } else if (MODULE_POWER_KEYS.has(call.parameterId)) {
        delta = 0.8
      } else {
        delta = 1.0
      }
      pulser.pulse(Math.min(1, delta))
    }
  }, [state, dspBridge, pulser])
  const update = (patch: Partial<PluginState>) => {
    setState((prev) => ({ ...prev, ...patch }))
  }

  useEffect(() => {
    const unsubscribe = subscribeToDspChanges(update)
    return () => unsubscribe()
  }, [])
  return (
    <div className="w-full h-full flex flex-col min-h-0 min-w-0 overflow-hidden relative select-none">
      <VstLayout
        leftColumn={
          <GainMeter label="IN" active={state.engineActive} delay={50} />
        }
        rightColumn={
          <GainMeter label="OUT" active={state.engineActive} delay={260} />
        }
      >
        <main className="flex-1 flex flex-col bg-bg min-h-0 overflow-hidden relative">
          <Header
            engineActive={state.engineActive}
            onToggleBypass={(active) => update({ engineActive: active })}
          />
          <FftVisualizer active={state.engineActive} glitch={pulser} />
          <div className="flex-1 flex items-center justify-center p-8">
            <MatrixFaceplate state={state} onChange={update} />
          </div>
        </main>
      </VstLayout>
      <SystemBoot />
    </div>
  )
}

export default App