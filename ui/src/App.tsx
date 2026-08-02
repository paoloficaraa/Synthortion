import { VstLayout } from './components/VstLayout'
import { GainMeter } from './components/GainMeter'
import { Knob } from './components/Knob'
import { useState } from 'react'

function App() {
  const [inputGain, setInputGain] = useState(0)
  const [outputGain, setOutputGain] = useState(0)

  return (
    <div className="flex items-start justify-center min-h-screen py-8">
      <VstLayout
        leftColumn={
          <GainMeter label="IN" active={true}>
            <Knob
              label="TRIM"
              value={inputGain}
              min={-24}
              max={24}
              displayValue={inputGain > 0 ? `+${Math.round(inputGain)}` : `${Math.round(inputGain)}`}
              size="small"
              onChange={setInputGain}
            />
          </GainMeter>
        }
        rightColumn={
          <GainMeter label="OUT" active={true}>
            <Knob
              label="TRIM"
              value={outputGain}
              min={-24}
              max={24}
              displayValue={outputGain > 0 ? `+${Math.round(outputGain)}` : `${Math.round(outputGain)}`}
              size="small"
              onChange={setOutputGain}
            />
          </GainMeter>
        }
      >
        <main className="flex-1 flex flex-col bg-bg border-t border-[#222]">
          <header className="h-[64px] bg-bg border-b border-border flex items-center justify-center px-8 shrink-0">
            <h1 className="font-display text-[16px] text-fg display-tracked mt-1 select-none">
              SYNTHORTION
            </h1>
          </header>
          <div className="flex-1 flex items-center justify-center p-8">
            <p className="font-body text-muted uppercase-tracked">
              Vintage Industrial VST Interface
            </p>
          </div>
        </main>
      </VstLayout>
    </div>
  )
}

export default App
