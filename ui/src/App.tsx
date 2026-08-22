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
 * App — single top-level state boundary for the whole plugin with Cartesian faceplate integration.
 *
 * Every parameter the faceplate exposes lives here as controlled props and is
 * pushed to the injected DSP bridge on change. Child components keep no
 * silent state, so binding the real C++ backend is a matter of swapping the
 * bridge. The center hub locks the dual-mode visualizer (240px) between the
 * status header (54px) and the module grid, each separated by a single 1px
 * Cartesian hairline (`#333333`) that meets the flanking 48px meter rails at
 * `+` crosshairs — no layout shift on bypass, no double borders, no AI-slop
 * repeated-rule strings. The high-contrast monochrome xerox aesthetic
 * (pitch-black, stark-white, warm-grey signal, 1px grid-rule) and static CRT
 * scanline texture are shared through `VstLayout`'s `noise-overlay` /
 * `vst-container::before`, keeping all UI surfaces cohesive per DESIGN.md.
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
    <div className="w-full h-full flex flex-col min-h-0 min-w-0 overflow-hidden relative select-none bg-void">
      <VstLayout
        leftColumn={<GainMeter label="IN" active={state.engineActive} delay={50} />}
        rightColumn={<GainMeter label="OUT" active={state.engineActive} delay={260} />}
      >
        {/* Center hub — Header (50px) → Visualizer (~35%) → Faceplate (~65%) share one Cartesian grid */}
        <main className="flex-1 flex flex-col bg-bg min-h-0 overflow-hidden relative">
          <Header
            engineActive={state.engineActive}
            onToggleBypass={(active) => update({ engineActive: active })}
          />
          {/* Dual-mode visualizer band allocated ~35% of center hub height */}
          <FftVisualizer active={state.engineActive} glitch={pulser} />
          {/* Faceplate plateau allocated ~65% of center hub height, expanding to 100% height */}
          <div data-testid="faceplate-plateau" className="basis-[65%] flex-grow flex flex-col p-3 sm:p-4 min-h-0 overflow-hidden bg-bg">
            <MatrixFaceplate state={state} onChange={update} />
          </div>
        </main>
      </VstLayout>
      <SystemBoot />
    </div>
  )
}

export default App