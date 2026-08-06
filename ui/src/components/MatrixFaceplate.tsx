import type { ReactNode } from 'react'
import { Knob } from './Knob'
import { Toggle, type ToggleOption } from './Toggle'
import type { DelaySync, DriveRoute, PluginState } from '../lib/pluginState'

interface MatrixFaceplateProps {
  /** Full plugin state, owned by the App root. */
  state: PluginState
  /** Reports a partial state patch for every faceplate interaction. */
  onChange: (patch: Partial<PluginState>) => void
}

const delaySyncOptions: readonly ToggleOption[] = [
  { value: 'SYNC', label: 'SYNC' },
  { value: 'FREE', label: 'FREE' },
  { value: 'PING-PONG', label: 'PANG' },
]

/** Shared faceplate section chrome: absolute corner label + hover wash. */
function SectionFrame({
  code,
  className,
  children,
}: {
  code: string
  className?: string
  children: ReactNode
}) {
  return (
    <section
      className={`p-6 pb-6 flex flex-col items-center justify-between relative group hover:bg-elev-0 transition-colors ${className ?? ''}`}
    >
      <div
        className="absolute top-4 left-4 text-ink-2 font-display text-[9px] uppercase-tracked"
        aria-hidden="true"
      >
        {code}
      </div>
      {children}
    </section>
  )
}

/**
 * Matrix Faceplate — the 5-column center hub.
 *
 * A purely presentational control surface: every value arrives through
 * `state` and every interaction flows straight back up through `onChange` to
 * the App root. It keeps no silent state of its own, which is what makes the
 * root the single boundary a future C++ DSP bridge can bind to.
 */
export function MatrixFaceplate({ state, onChange }: MatrixFaceplateProps) {
  const {
    drive,
    driveRoute,
    bitcrush,
    delayMix,
    delayTime,
    delayFbk,
    delaySync,
    chorus,
    chorusWide,
  } = state

  return (
    <div className="grid grid-cols-5 divide-x divide-border w-full shadow-[inset_0_1px_0_0_rgba(255,255,255,0.04)]">
      {/* DRV — Drive with PRE/POST route toggle */}
      <SectionFrame code="DRV">
        <div className="flex-1 flex flex-col justify-center items-center mt-6 w-full">
          <Knob
            label="Drive"
            value={drive}
            min={0}
            max={100}
            displayValue={`${Math.round(drive)}%`}
            onChange={(value) => onChange({ drive: value })}
          />
        </div>
        <div className="w-full flex gap-1 mt-8 opacity-40 hover:opacity-100 transition-opacity">
          <Toggle
            label="Drive route"
            options={[
              { value: 'PRE', label: 'PRE' },
              { value: 'POST', label: 'POST' },
            ]}
            value={driveRoute}
            onChange={(value) => onChange({ driveRoute: value as DriveRoute })}
          />
        </div>
      </SectionFrame>

      {/* BCR — Bitcrush step-count knob with a spacer to keep knobs aligned */}
      <SectionFrame code="BCR">
        <div className="flex-1 flex flex-col justify-center items-center mt-6 w-full">
          <Knob
            label="Bitcrush"
            value={bitcrush}
            min={2}
            max={24}
            displayValue={`${Math.round(bitcrush)}B`}
            onChange={(value) => onChange({ bitcrush: value })}
          />
        </div>
        <div
          className="w-full flex gap-1 mt-8 opacity-0 pointer-events-none"
          aria-hidden="true"
        >
          <span className="flex-1 border font-mono text-[9px] py-1.5 uppercase border-border">
            -
          </span>
        </div>
      </SectionFrame>

      {/* DLY — dual-column delay: Mix knob + Time/Fbk small knobs + timebase tie */}
      <SectionFrame code="DLY" className="col-span-2 shadow-[inset_1px_0_0_0_var(--elev-6)]">
        <div className="flex-1 flex flex-col items-center justify-center w-full mt-2">
          <Knob
            label="Mix"
            value={delayMix}
            min={0}
            max={100}
            displayValue={`${Math.round(delayMix)}%`}
            onChange={(value) => onChange({ delayMix: value })}
          />
          <div className="flex gap-10 mt-6">
            <Knob
              label="Time"
              value={delayTime}
              min={0}
              max={1000}
              displayValue={`${Math.round(delayTime)}`}
              size="small"
              onChange={(value) => onChange({ delayTime: value })}
            />
            <Knob
              label="Fbk"
              value={delayFbk}
              min={0}
              max={100}
              displayValue={`${Math.round(delayFbk)}%`}
              size="small"
              onChange={(value) => onChange({ delayFbk: value })}
            />
          </div>
        </div>
        <div className="w-[80%] max-w-[200px] flex gap-1 mt-8 opacity-40 hover:opacity-100 transition-opacity mx-auto">
          <Toggle
            label="Delay timebase"
            options={delaySyncOptions}
            value={delaySync}
            onChange={(value) => onChange({ delaySync: value as DelaySync })}
          />
        </div>
      </SectionFrame>

      {/* CHR — Chorus with WIDE on/off toggle */}
      <SectionFrame code="CHR" className="shadow-[inset_1px_0_0_0_var(--elev-6)]">
        <div className="flex-1 flex flex-col justify-center items-center mt-6 w-full">
          <Knob
            label="Chorus"
            value={chorus}
            min={0}
            max={100}
            displayValue={`${Math.round(chorus)}%`}
            onChange={(value) => onChange({ chorus: value })}
          />
        </div>
        <div className="w-full mt-8 opacity-40 hover:opacity-100 transition-opacity">
          <Toggle
            label="Chorus width"
            options={[{ value: 'on', label: 'WIDE' }]}
            value={chorusWide ? 'on' : 'off'}
            onChange={() => onChange({ chorusWide: !chorusWide })}
          />
        </div>
      </SectionFrame>
    </div>
  )
}
