import type { ReactNode } from 'react'
import { Knob } from './Knob'
import { Toggle } from './Toggle'
import { CalibrationTicks } from './CalibrationTicks'
import { DELAY_SUBDIVISIONS } from '../lib/parameterStore'
import { initialState, type PluginState } from '../lib/pluginState'
interface MatrixFaceplateProps {
  /** Full plugin state, owned by the App root. */
  state: PluginState
  /** Reports a partial state patch for every faceplate interaction. */
  onChange: (patch: Partial<PluginState>) => void
}

export type ModuleCode = 'DRV' | 'BCR' | 'DLY' | 'CHR'

interface ModuleFrameProps {
  /** Module code rendered in the title bar (DRV/BCR/DLY/CHR). */
  code: ModuleCode
  /** Whether the module's power switch is on. */
  powerOn: boolean
  /** Flips the module power flag; called with the new value. */
  onTogglePower: () => void
  /** Extra section classes (grid span, inner shadow). */
  className?: string
  children: ReactNode
}


/**
 * Module frame — the framed module title bar with a power switch.
 *
 * Replaces the plain corner code chip: each section now reads as a hardware
 * module with its code on the left and an LED power switch on the right.
 * When the module is powered off the controls below are dimmed and inert;
 * the title bar (and its switch) stays fully interactive so the module can
 * be powered back on.
 */
function ModuleFrame({
  code,
  powerOn,
  onTogglePower,
  className,
  children,
}: ModuleFrameProps) {
  return (
    <section
      className={`p-0 flex flex-col items-stretch justify-between relative bg-elev-0 border-r border-grid-rule last:border-r-0 h-full min-h-0 ${className ?? ''}`}
    >
      {/* Cartesian Header: + [ DRV ] + [ PWR: ON ] + */}
      <div
        className="flex items-center justify-between px-2 h-[24px] shrink-0 font-ascii text-[9px] leading-none select-none border-b border-grid-rule bg-elev-0 text-muted relative"
        aria-hidden="true"
      >
        <div className="flex items-center gap-1">
          <span className="text-ink-3 font-bold">+</span>
          <span className="text-ink-3">[</span>
          <span className="text-fg font-bold tracking-widest">{code}</span>
          <span className="text-ink-3">]</span>
        </div>
        <div className="flex-1 h-px bg-grid-rule mx-2 relative flex items-center justify-center">
          <span className="text-ink-3 text-[8px] bg-elev-0 px-1">+</span>
        </div>
        <div className="flex items-center gap-1 mr-4">
          <span className="text-ink-3">[ PWR:</span>
          <span className={powerOn ? 'text-fg font-bold' : 'text-ink-3'}>
            {powerOn ? 'ON' : 'OFF'}
          </span>
          <span className="text-ink-3">]</span>
          <span className="text-ink-3 font-bold">+</span>
        </div>
      </div>

      {/* Power switch button */}
      <div className="absolute top-1.5 right-2 z-10 flex items-center">
        <button
          type="button"
          onClick={onTogglePower}
          aria-pressed={powerOn}
          aria-label={powerOn ? `Turn off ${code} module` : `Turn on ${code} module`}
          title={powerOn ? 'Power off' : 'Power on'}
          data-testid={`power-${code}`}
          className="w-2.5 h-2.5 rounded-[1px] border border-border outline-none focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-1 focus-visible:ring-offset-bg transition-transform duration-150 hover:scale-[1.25] active:scale-[0.8]"
          style={{
            backgroundColor: powerOn ? 'var(--fg)' : 'var(--elev-5)',
            borderColor: powerOn ? 'var(--fg)' : undefined,
            boxShadow: powerOn
              ? '0 0 0 1px var(--fg)'
              : 'inset 0 1px 2px rgba(0,0,0,0.8)',
          }}
        />
      </div>

      {/* Module Interior (controls + calibration ticks + bottom bar) */}
      <div
        className={`flex-1 flex flex-col items-stretch justify-between w-full min-h-0 transition-opacity duration-150 ${
          powerOn ? '' : 'opacity-30 pointer-events-none'
        }`}
      >
        <div className="relative flex-1 flex flex-col items-center justify-center px-2 sm:px-3 py-2 sm:py-3 min-w-0 w-full overflow-hidden">
          <CalibrationTicks side="left" />
          <CalibrationTicks side="right" />
          {children}
        </div>

        {/* Cartesian Bottom Calibration Bar */}
        <div
          className="flex items-center justify-between px-2 h-[16px] shrink-0 font-ascii text-[8px] leading-none select-none border-t border-grid-rule text-ink-3 bg-elev-0"
          aria-hidden="true"
        >
          <div className="flex items-center gap-1">
            <span>+</span>
            <span className="text-[7px] text-ink-3 tracking-tighter opacity-50">░▒</span>
          </div>
          <div className="flex-1 flex items-center justify-center gap-1 overflow-hidden px-2">
            <span className="h-px w-2 bg-grid-rule" />
            <span className="text-[7px] text-ink-3 tracking-tighter font-mono">CAL.0{code}</span>
            <span className="h-px flex-1 bg-grid-rule" />
            <span className="text-[7px] text-ink-3">::</span>
            <span className="h-px flex-1 bg-grid-rule" />
            <span className="text-[7px] text-ink-3 tracking-tighter font-mono">SYS.X</span>
            <span className="h-px w-2 bg-grid-rule" />
          </div>
          <div className="flex items-center gap-1">
            <span className="text-[7px] text-ink-3 tracking-tighter opacity-50">▒░</span>
            <span>+</span>
          </div>
        </div>
      </div>
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
 *
 * Each of the four sections is a framed module: the code chip is now a title
 * bar with a power switch (LED + state). Powering a module off dims its
 * controls and swaps its numeric readouts for `--`.
 */
export function MatrixFaceplate({ state, onChange }: MatrixFaceplateProps) {
  const {
    drive,
    bitcrush,
    delayMix,
    delayTimeFree,
    delayTimeSync,
    delayFbk,
    delaySync,
    chorus,
    chorusWidth,
    driveOn,
    bitcrushOn,
    delayOn,
    chorusOn,
  } = state
  const isDelaySync = delaySync === true
  const delayTimeDisplay = isDelaySync
    ? (DELAY_SUBDIVISIONS[delayTimeSync] ?? '1/8T')
    : `${Math.round(delayTimeFree)}ms`
  return (
    <div
      data-testid="matrix-faceplate"
      data-engine-active={state.engineActive}
      className="grid grid-cols-5 w-full h-full border border-grid-rule bg-elev-0 flex-1 min-h-0"
    >
      {/* DRV — Drive with PRE/POST route toggle */}
      <ModuleFrame
        code="DRV"
        powerOn={driveOn}
        onTogglePower={() => onChange({ driveOn: !driveOn })}
      >
        <div className="flex-1 flex flex-col justify-center items-center w-full">
          <Knob
            label="Drive"
            value={drive}
            min={0}
            max={100}
            displayValue={`${Math.round(drive)}%`}
            defaultValue={initialState.drive}
            enabled={driveOn}
            onChange={(value) => onChange({ drive: value })}
          />
        </div>
      </ModuleFrame>

      {/* BCR — Bitcrush step-count knob */}
      <ModuleFrame
        code="BCR"
        powerOn={bitcrushOn}
        onTogglePower={() => onChange({ bitcrushOn: !bitcrushOn })}
      >
        <div className="flex-1 flex flex-col justify-center items-center w-full">
          <Knob
            label="Bitcrush"
            value={bitcrush}
            min={0}
            max={100}
            displayValue={`${Math.round(bitcrush)}%`}
            defaultValue={initialState.bitcrush}
            enabled={bitcrushOn}
            onChange={(value) => onChange({ bitcrush: value })}
          />
        </div>
      </ModuleFrame>

      {/* DLY — dual-column delay: Mix knob + Time/Fbk small knobs + timebase tie */}
      <ModuleFrame
        code="DLY"
        powerOn={delayOn}
        onTogglePower={() => onChange({ delayOn: !delayOn })}
        className="col-span-2"
      >
        <div className="flex-1 flex flex-col items-center justify-center w-full gap-2 sm:gap-3 py-1">
          <Knob
            label="Mix"
            value={delayMix}
            min={0}
            max={100}
            displayValue={`${Math.round(delayMix)}%`}
            defaultValue={initialState.delayMix}
            enabled={delayOn}
            onChange={(value) => onChange({ delayMix: value })}
          />
          <div className="flex gap-4 sm:gap-8 items-center justify-center">
            <Knob
              label="Time"
              value={isDelaySync ? delayTimeSync : delayTimeFree}
              min={isDelaySync ? 0 : 1}
              max={isDelaySync ? 13 : 2000}
              displayValue={delayTimeDisplay}
              size="small"
              enabled={delayOn}
              defaultValue={isDelaySync ? initialState.delayTimeSync : initialState.delayTimeFree}
              onChange={(value) => isDelaySync ? onChange({ delayTimeSync: Math.round(value) }) : onChange({ delayTimeFree: Math.round(value) })}
            />
            <Knob
              label="Fbk"
              value={delayFbk}
              min={0}
              max={100}
              displayValue={`${Math.round(delayFbk)}%`}
              size="small"
              defaultValue={initialState.delayFbk}
              enabled={delayOn}
              onChange={(value) => onChange({ delayFbk: value })}
            />
          </div>
          <div className="w-full max-w-[130px] px-1">
            <Toggle
              label="Delay sync"
              options={[
                { value: 'on', label: 'SYNC' },
                { value: 'off', label: 'FREE' },
              ]}
              value={delaySync ? 'on' : 'off'}
              enabled={delayOn}
              onChange={(val) => {
                onChange({ delaySync: val === 'on' })
              }}
            />
          </div>
        </div>
      </ModuleFrame>

      {/* CHR — Chorus with WIDE on/off toggle */}
      <ModuleFrame
        code="CHR"
        powerOn={chorusOn}
        onTogglePower={() => onChange({ chorusOn: !chorusOn })}
      >
        <div className="flex-1 flex flex-col items-center justify-center w-full gap-2 sm:gap-3 py-1">
          <Knob
            label="Chorus"
            value={chorus}
            min={0}
            max={100}
            displayValue={`${Math.round(chorus)}%`}
            enabled={chorusOn}
            onChange={(value) => onChange({ chorus: value })}
            defaultValue={initialState.chorus}
          />
          <Knob
            label="Width"
            value={chorusWidth}
            min={0}
            max={100}
            displayValue={`${Math.round(chorusWidth)}%`}
            size="small"
            enabled={chorusOn}
            onChange={(value) => onChange({ chorusWidth: value })}
            defaultValue={initialState.chorusWidth}
          />
        </div>
      </ModuleFrame>
    </div>
  )
}
