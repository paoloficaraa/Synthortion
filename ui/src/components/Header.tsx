import { useEffect, useState, type KeyboardEvent } from 'react'
import { parameterStore } from '../lib/parameterStore'

export interface HeaderProps {
  /** Whether the main DSP engine is active (bypass LED lit). */
  engineActive: boolean
  /** Flips the engine bypass state; called with the new value. */
  onToggleBypass: (active: boolean) => void
  /** Opens the Preset Browser modal overlay. */
  onOpenPresets?: () => void
  /** Opens the Save Preset modal overlay. */
  onOpenSave?: () => void
  /** Steps to the previous preset in the active category. */
  onStepPrev?: () => void
  /** Steps to the next preset in the active category. */
  onStepNext?: () => void
}

/** Terminal bracket-button labels for modal actions. */
const ACTIONS = [
  { label: 'BROWSE', key: 'presets' },
  { label: 'SAVE', key: 'save' },
] as const

/**
 * Header — the terminal status bar with ASCII preset readout and category steppers.
 *
 * Brand on the left beside the engine LED, the VGA preset readout with `<` and `>`
 * category-aware steppers in the middle, and `[ BROWSE ] [ SAVE ]` bracket buttons
 * on the right. Hover inverts the bracket buttons; the bypass LED behavior is unchanged.
 */
export function Header({
  engineActive,
  onToggleBypass,
  onOpenPresets,
  onOpenSave,
  onStepPrev,
  onStepNext,
}: HeaderProps) {
  const [presetInfo, setPresetInfo] = useState(() => ({
    name: parameterStore.getActivePresetName(),
    category: parameterStore.getActivePresetCategory(),
    isDirty: parameterStore.getIsPresetDirty(),
  }))

  useEffect(() => {
    const sync = () => {
      setPresetInfo({
        name: parameterStore.getActivePresetName(),
        category: parameterStore.getActivePresetCategory(),
        isDirty: parameterStore.getIsPresetDirty(),
      })
    }
    sync()
    return parameterStore.subscribePresets(sync)
  }, [])

  const handleStepPrev = () => {
    if (onStepPrev) {
      onStepPrev()
    } else {
      parameterStore.stepPreset('prev')
    }
  }

  const handleStepNext = () => {
    if (onStepNext) {
      onStepNext()
    } else {
      parameterStore.stepPreset('next')
    }
  }

  const handleReadoutKeyDown = (e: KeyboardEvent<HTMLButtonElement>) => {
    if (e.key === 'ArrowLeft' || e.key === '[') {
      e.preventDefault()
      handleStepPrev()
    } else if (e.key === 'ArrowRight' || e.key === ']') {
      e.preventDefault()
      handleStepNext()
    } else if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault()
      onOpenPresets?.()
    }
  }

  const categoryUpper = (presetInfo.category || 'INIT').toUpperCase()
  const nameUpper = (presetInfo.name || 'DEFAULT').toUpperCase()
  const formattedText = `${categoryUpper}: ${nameUpper}${presetInfo.isDirty ? ' *' : ''}`

  return (
    <header className="h-[50px] bg-bg flex items-center justify-between px-6 shrink-0 relative z-10 border-b border-grid-rule">
      {/* Cartesian Coordinate Corner Marks */}
      <div
        className="absolute bottom-0 left-0 -mb-[4px] -ml-[3px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none"
        aria-hidden="true"
      >
        +
      </div>
      <div
        className="absolute bottom-0 right-0 -mb-[4px] -mr-[3px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none"
        aria-hidden="true"
      >
        +
      </div>

      {/* Left: Bypass switch + LED + Title */}
      <div className="flex items-center gap-3">
        <button
          type="button"
          onClick={() => onToggleBypass(!engineActive)}
          aria-pressed={engineActive}
          aria-label={engineActive ? 'Disable main DSP' : 'Enable main DSP'}
          title={engineActive ? 'Bypass DSP' : 'Engage DSP'}
          className="w-3.5 h-3.5 rounded-[1px] border cursor-pointer outline-none shadow-well focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-2 focus-visible:ring-offset-bg transition-all duration-150 hover:scale-[1.15] active:scale-[0.85] flex items-center justify-center"
          style={{
            backgroundColor: engineActive ? 'var(--fg)' : 'var(--elev-3)',
            borderColor: engineActive ? 'var(--fg)' : 'var(--muted)',
            boxShadow: engineActive
              ? '0 0 0 1px var(--fg)'
              : '0 0 0 1px var(--border)',
          }}
        >
          {!engineActive && (
            <span
              className="w-1 h-1 bg-muted rounded-[1px] pointer-events-none"
              aria-hidden="true"
            />
          )}
        </button>
        <span
          className="font-mono text-[9px] text-muted uppercase-tracked select-none"
          aria-hidden="true"
        >
          [ BYPASS: {engineActive ? 'INACTIVE' : 'ACTIVE'} ]
        </span>
        <h1 className="font-display text-[16px] text-fg display-tracked mt-1 select-none">
          SYNTHORTION
        </h1>
      </div>

      {/* Center: ASCII Stepper & Readout [ < ] [ CATEGORY: NAME * ] [ > ] */}
      <div className="flex items-center gap-1.5">
        <button
          type="button"
          onClick={handleStepPrev}
          aria-label="Previous preset"
          className="px-2 py-1 border border-border font-mono text-[9px] uppercase-tracked text-muted outline-none focus-visible:ring-2 focus-visible:ring-inset focus-visible:ring-accent hover:bg-fg hover:text-bg hover:border-fg transition-all duration-150 active:scale-[0.94] cursor-pointer"
        >
          <span aria-hidden="true">[ </span>
          &lt;
          <span aria-hidden="true"> ]</span>
        </button>

        <button
          type="button"
          onClick={onOpenPresets}
          onKeyDown={handleReadoutKeyDown}
          aria-label={`Preset: ${formattedText}`}
          aria-haspopup="dialog"
          className="min-w-[190px] whitespace-nowrap px-3 py-1 bg-elev-0 border border-border font-ascii text-[14px] leading-none text-fg uppercase-tracked select-none flex items-center justify-between cursor-pointer outline-none focus-visible:ring-2 focus-visible:ring-inset focus-visible:ring-accent hover:border-fg transition-colors duration-150"
        >
          <div className="flex items-center">
            <span aria-hidden="true">[ </span>
            <span>{formattedText}</span>
            <span aria-hidden="true"> ]</span>
          </div>
          <span className="block-cursor ml-1" aria-hidden="true">
            ▊
          </span>
        </button>

        <button
          type="button"
          onClick={handleStepNext}
          aria-label="Next preset"
          className="px-2 py-1 border border-border font-mono text-[9px] uppercase-tracked text-muted outline-none focus-visible:ring-2 focus-visible:ring-inset focus-visible:ring-accent hover:bg-fg hover:text-bg hover:border-fg transition-all duration-150 active:scale-[0.94] cursor-pointer"
        >
          <span aria-hidden="true">[ </span>
          &gt;
          <span aria-hidden="true"> ]</span>
        </button>
      </div>

      {/* Right: Modal trigger bracket buttons [ PRESETS ] [ SAVE ] */}
      <div className="flex items-center gap-2">
        {ACTIONS.map(({ label, key }) => {
          const onClickHandler = key === 'presets' ? onOpenPresets : onOpenSave
          return (
            <button
              key={label}
              type="button"
              onClick={onClickHandler}
              className="px-3 py-1 border border-border font-mono text-[9px] uppercase-tracked text-muted outline-none focus-visible:ring-2 focus-visible:ring-inset focus-visible:ring-accent hover:bg-fg hover:text-bg hover:border-fg transition-all duration-150 active:scale-[0.94] cursor-pointer"
            >
              <span aria-hidden="true">[ </span>
              {label}
              <span aria-hidden="true"> ]</span>
            </button>
          )
        })}
      </div>
    </header>
  )
}
