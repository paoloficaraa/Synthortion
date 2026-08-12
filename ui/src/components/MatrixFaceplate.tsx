import type { ReactNode } from 'react'
import { motion } from 'framer-motion'
import { Knob } from './Knob'
import type { PluginState } from '../lib/pluginState'

interface MatrixFaceplateProps {
  /** Full plugin state, owned by the App root. */
  state: PluginState
  /** Reports a partial state patch for every faceplate interaction. */
  onChange: (patch: Partial<PluginState>) => void
}


interface ModuleFrameProps {
  /** Module code rendered in the title bar (DRV/BCR/DLY/CHR). */
  code: string
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
      className={`p-0 flex flex-col items-stretch justify-between relative group ${className ?? ''}`}
    >
      {/* ASCII box-drawing header: ┌──[ DRV ]──[ PWR: ON ]──┐ */}
      <div
        className="flex items-center justify-between px-2 h-[24px] font-ascii text-[10px] leading-none select-none"
        aria-hidden="true"
      >
        <span className="text-ink-3 whitespace-pre">┌──[ </span>
        <span className="text-fg">{code}</span>
        <span className="text-ink-3 whitespace-pre"> ]</span>
        <span className="flex-1 text-ink-3 text-center px-1 truncate">
          {'─'.repeat(8)}
        </span>
        <span className="text-ink-3 whitespace-pre">[ PWR: </span>
        <span className={powerOn ? 'text-fg' : 'text-ink-3'}>
          {powerOn ? 'ON ' : 'OFF'}
        </span>
        <span className="text-ink-3 whitespace-pre">]──┐</span>
      </div>

      {/* Box-drawing left border is implied by section edges; keep the
          interactive title controls absolutely positioned for keyboard/a11y. */}
      <div className="absolute top-1 right-2 z-10 flex items-center">
        <motion.button
          type="button"
          onClick={onTogglePower}
          aria-pressed={powerOn}
          aria-label={powerOn ? `Turn off ${code} module` : `Turn on ${code} module`}
          title={powerOn ? 'Power off' : 'Power on'}
          data-testid={`power-${code}`}
          whileHover={{ scale: 1.25 }}
          whileTap={{ scale: 0.8 }}
          className="w-2.5 h-2.5 rounded-full border border-border outline-none focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-1 focus-visible:ring-offset-bg transition-colors"
          style={{
            backgroundColor: powerOn ? 'var(--fg)' : 'var(--elev-5)',
            borderColor: powerOn ? 'var(--fg)' : undefined,
            boxShadow: powerOn
              ? '0 0 6px rgba(255,255,255,0.6)'
              : 'inset 0 1px 2px rgba(0,0,0,0.8)',
          }}
        />
      </div>
      <div className="relative flex-1 flex items-stretch w-full min-h-0">
        <div className="font-ascii text-[10px] text-ink-3 select-none py-2 px-0.5 flex flex-col justify-between" aria-hidden="true">
          <span>│</span>
          <span>│</span>
        </div>
        <div className="px-2 pt-4 pb-3 flex-1 flex flex-col items-center justify-between min-w-0">
          <div
            className={`flex-1 flex flex-col items-center justify-between w-full ${
              powerOn ? '' : 'opacity-30 pointer-events-none'
            }`}
          >
            {children}
          </div>
        </div>
        <div className="font-ascii text-[10px] text-ink-3 select-none py-2 px-0.5 flex flex-col justify-between" aria-hidden="true">
          <span>│</span>
          <span>│</span>
        </div>
      </div>
      <div
        className="flex items-center px-2 h-[18px] font-ascii text-[10px] leading-none select-none"
        aria-hidden="true"
      >
        <span className="text-ink-3 whitespace-pre">└</span>
        <span className="flex-1 text-ink-3 text-center truncate">
          {'─'.repeat(20)}
        </span>
        <span className="text-ink-3 whitespace-pre">┘</span>
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
    delayTime,
    delayFbk,
    chorus,
    driveOn,
    bitcrushOn,
    delayOn,
    chorusOn,
  } = state

  return (
    <div className="grid grid-cols-5 divide-x divide-border w-full shadow-[inset_0_1px_0_0_rgba(255,255,255,0.04)]">
      {/* DRV — Drive with PRE/POST route toggle */}
      <ModuleFrame
        code="DRV"
        powerOn={driveOn}
        onTogglePower={() => onChange({ driveOn: !driveOn })}
      >
        <div className="flex-1 flex flex-col justify-center items-center mt-6 w-full">
          <Knob
            label="Drive"
            value={drive}
            min={0}
            max={100}
            displayValue={`${Math.round(drive)}%`}
            enabled={driveOn}
            onChange={(value) => onChange({ drive: value })}
          />
        </div>
      </ModuleFrame>

      {/* BCR — Bitcrush step-count knob with a spacer to keep knobs aligned */}
      <ModuleFrame
        code="BCR"
        powerOn={bitcrushOn}
        onTogglePower={() => onChange({ bitcrushOn: !bitcrushOn })}
      >
        <div className="flex-1 flex flex-col justify-center items-center mt-6 w-full">
          <Knob
            label="Bitcrush"
            value={bitcrush}
            min={2}
            max={24}
            displayValue={`${Math.round(bitcrush)}B`}
            enabled={bitcrushOn}
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
      </ModuleFrame>

      {/* DLY — dual-column delay: Mix knob + Time/Fbk small knobs + timebase tie */}
      <ModuleFrame
        code="DLY"
        powerOn={delayOn}
        onTogglePower={() => onChange({ delayOn: !delayOn })}
        className="col-span-2 shadow-[inset_1px_0_0_0_var(--elev-6)]"
      >
        <div className="flex-1 flex flex-col items-center justify-center w-full mt-2">
          <Knob
            label="Mix"
            value={delayMix}
            min={0}
            max={100}
            displayValue={`${Math.round(delayMix)}%`}
            enabled={delayOn}
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
              enabled={delayOn}
              onChange={(value) => onChange({ delayTime: value })}
            />
            <Knob
              label="Fbk"
              value={delayFbk}
              min={0}
              max={100}
              displayValue={`${Math.round(delayFbk)}%`}
              size="small"
              enabled={delayOn}
              onChange={(value) => onChange({ delayFbk: value })}
            />
          </div>
        </div>
      </ModuleFrame>

      {/* CHR — Chorus with WIDE on/off toggle */}
      <ModuleFrame
        code="CHR"
        powerOn={chorusOn}
        onTogglePower={() => onChange({ chorusOn: !chorusOn })}
        className="shadow-[inset_1px_0_0_0_var(--elev-6)]"
      >
        <div className="flex-1 flex flex-col justify-center items-center mt-6 w-full">
          <Knob
            label="Chorus"
            value={chorus}
            min={0}
            max={100}
            displayValue={`${Math.round(chorus)}%`}
            enabled={chorusOn}
            onChange={(value) => onChange({ chorus: value })}
          />
        </div>
      </ModuleFrame>
    </div>
  )
}
