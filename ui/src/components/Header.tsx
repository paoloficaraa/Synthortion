import { motion } from 'framer-motion'

interface HeaderProps {
  /** Whether the main DSP engine is active (bypass LED lit). */
  engineActive: boolean
  /** Flips the engine bypass state; called with the new value. */
  onToggleBypass: (active: boolean) => void
}

/** Preset name shown in the header VGA readout. */
const PRESET_NAME = 'INIT_STATE_01'

/** Terminal bracket-button labels (`[ SAVE ]` / `[ LOAD ]`). */
const ACTIONS = ['SAVE', 'LOAD'] as const

/**
 * Header — the terminal status bar.
 *
 * Brand on the left beside the engine LED, the VGA preset readout (with a
 * blinking block cursor) in the middle, and the `[ SAVE ] [ LOAD ]` bracket
 * buttons on the right. Hover inverts the bracket buttons; the bypass LED
 * behaviour is unchanged.
 */
export function Header({ engineActive, onToggleBypass }: HeaderProps) {
  return (
    <header className="h-[54px] bg-bg border-b border-border flex items-center justify-between px-6 shrink-0 relative z-10">
      <div className="flex items-center gap-3">
        <motion.button
          type="button"
          onClick={() => onToggleBypass(!engineActive)}
          aria-pressed={engineActive}
          aria-label={engineActive ? 'Disable main DSP' : 'Enable main DSP'}
          title="Bypass"
          whileHover={{ scale: 1.15 }}
          whileTap={{ scale: 0.85 }}
          className="w-3.5 h-3.5 rounded-[1px] border border-border outline-none shadow-well focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-2 focus-visible:ring-offset-bg transition-colors"
          style={{
            backgroundColor: engineActive ? 'var(--fg)' : 'var(--elev-5)',
            borderColor: engineActive ? 'var(--fg)' : undefined,
            boxShadow: engineActive
              ? '0 0 8px rgba(255,255,255,0.7)'
              : undefined,
          }}
        />
        <span className="font-mono text-[9px] text-muted uppercase-tracked select-none" aria-hidden="true">
          [ BYPASS: {engineActive ? 'INACTIVE' : 'ACTIVE'} ]
        </span>
        <h1 className="font-display text-[16px] text-fg display-tracked mt-1 select-none">
          SYNTHORTION
        </h1>
      </div>

      <div className="flex items-center gap-2">
        <span
          className="font-display text-[8px] text-muted uppercase-tracked"
          aria-hidden="true"
        >
          Preset
        </span>
        <div className="min-w-[170px] whitespace-nowrap px-3 py-1 bg-elev-0 border border-border font-ascii text-[14px] leading-none text-fg uppercase-tracked select-none flex items-center justify-between">
          <span>{PRESET_NAME}</span>
          <span className="block-cursor ml-1" aria-hidden="true">
            ▊
          </span>
        </div>
      </div>

      <div className="flex items-center gap-2">
        {ACTIONS.map((label) => (
          <motion.button
            key={label}
            type="button"
            whileTap={{ scale: 0.94 }}
            className="px-3 py-1 border border-border font-mono text-[9px] uppercase-tracked text-muted outline-none focus-visible:ring-2 focus-visible:ring-inset focus-visible:ring-accent hover:bg-fg hover:text-bg hover:border-fg transition-colors"
          >
            <span aria-hidden="true">[ </span>
            {label}
            <span aria-hidden="true"> ]</span>
          </motion.button>
        ))}
      </div>
    </header>
  )
}
