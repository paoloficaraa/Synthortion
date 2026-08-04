import { useEffect, useRef, useState } from 'react'
import { MotionConfig } from 'framer-motion'
import { VstLayout } from './components/VstLayout'
import { Header } from './components/Header'
import { SystemBoot } from './components/SystemBoot'
import { GainMeter } from './components/GainMeter'
import { Knob } from './components/Knob'
import { MatrixFaceplate } from './components/MatrixFaceplate'
import { FftVisualizer } from './components/FftVisualizer'
import { initialState, type PluginState } from './lib/pluginState'
import { noopDspBridge, type DspBridge } from './lib/dspBridge'

interface AppProps {
  /** Integration seam for the future C++ DSP bridge. */
  dspBridge?: DspBridge
}

/** Format a TRIM value in dB, prefixing positive values with a "+". */
function formatTrimValue(value: number): string {
  return value > 0 ? `+${Math.round(value)}` : `${Math.round(value)}`
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

  // Push changed parameters to the bridge. The initial mount is skipped so
  // the bridge only records user-driven mutations.
  useEffect(() => {
    const prev = prevStateRef.current
    prevStateRef.current = state
    for (const key of Object.keys(state) as Array<keyof PluginState>) {
      if (state[key] !== prev[key]) {
        dspBridge.setParameter(key, state[key])
      }
    }
  }, [state, dspBridge])

  const update = (patch: Partial<PluginState>) => {
    setState((prev) => ({ ...prev, ...patch }))
  }

  return (
    <MotionConfig reducedMotion="user">
      <div className="flex items-start justify-center min-h-screen py-8">
        <VstLayout
          leftColumn={
            <GainMeter label="IN" active={state.engineActive} delay={50}>
              <Knob
                label="TRIM"
                value={state.inputGain}
                min={-24}
                max={24}
                displayValue={formatTrimValue(state.inputGain)}
                size="small"
                onChange={(value) => update({ inputGain: value })}
              />
            </GainMeter>
          }
          rightColumn={
            <GainMeter label="OUT" active={state.engineActive} delay={260}>
              <Knob
                label="TRIM"
                value={state.outputGain}
                min={-24}
                max={24}
                displayValue={formatTrimValue(state.outputGain)}
                size="small"
                onChange={(value) => update({ outputGain: value })}
              />
            </GainMeter>
          }
        >
          <main className="flex-1 flex flex-col bg-bg border-t border-[#222]">
            <Header
              engineActive={state.engineActive}
              onToggleBypass={(active) => update({ engineActive: active })}
            />
            <FftVisualizer active={state.engineActive} />
            <div className="flex-1 flex items-center justify-center p-8">
              <MatrixFaceplate state={state} onChange={update} />
            </div>
          </main>
        </VstLayout>
      </div>
      <SystemBoot />
    </MotionConfig>
  )
}

export default App
