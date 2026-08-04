import { motion } from 'framer-motion'

interface HeaderProps {
  /** Whether the main DSP engine is active (bypass LED lit). */
  engineActive: boolean
  /** Flips the engine bypass state; called with the new value. */
  onToggleBypass: (active: boolean) => void
}

/** Preset name shown in the header LCD readout. */
const PRESET_NAME = 'INIT_STATE_01'

/**
 * Header — the "Glitch Brutalism" chrome strip.
 *
 * Brand on the left beside the bypass LED, the embedded preset LCD readout in
 * the middle, and the SAVE / LOAD buttons on the right. Motion (hover, press)
 * is handled by Framer Motion so `MotionConfig reducedMotion="user"` can
 * disable it globally.
 */
export function Header({ engineActive, onToggleBypass }: HeaderProps) {
  return (
    <header className="h-[64px] bg-bg border-b border-border flex items-center justify-between px-8 shrink-0 relative z-10">
      <div className="flex items-center gap-5">
        <motion.button
          type="button"
          onClick={() => onToggleBypass(!engineActive)}
          aria-pressed={engineActive}
          aria-label={engineActive ? 'Disable main DSP' : 'Enable main DSP'}
          title="Bypass"
          whileHover={{ scale: 1.15 }}
          whileTap={{ scale: 0.85 }}
          className="w-3.5 h-3.5 rounded-[1px] border border-border outline-none focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-2 focus-visible:ring-offset-bg"
          style={{
            backgroundColor: engineActive ? 'var(--fg)' : '#1a1a1a',
            borderColor: engineActive ? 'var(--fg)' : undefined,
            boxShadow: engineActive
              ? '0 0 8px rgba(255,255,255,0.7)'
              : 'none',
          }}
        />
        <h1 className="font-display text-[16px] text-fg display-tracked mt-1 select-none">
          SYNTHORTION
        </h1>
      </div>

      <div className="flex items-center gap-3">
        <span
          className="font-display text-[8px] text-muted uppercase-tracked"
          aria-hidden="true"
        >
          Preset
        </span>
        <div className="min-w-[132px] px-3 py-1.5 bg-[#050505] border border-border font-mono text-[11px] text-fg uppercase-tracked select-none shadow-[inset_0_1px_3px_rgba(0,0,0,0.9)]">
          {PRESET_NAME}
        </div>
      </div>

      <div className="flex items-center gap-2">
        <motion.button
          type="button"
          whileHover={{ backgroundColor: 'var(--fg)', color: 'var(--bg)' }}
          whileTap={{ scale: 0.94 }}
          className="px-3 py-1 border border-border font-mono text-[9px] uppercase-tracked text-muted bg-transparent outline-none focus-visible:ring-2 focus-visible:ring-inset focus-visible:ring-accent"
        >
          SAVE
        </motion.button>
        <motion.button
          type="button"
          whileHover={{ backgroundColor: 'var(--fg)', color: 'var(--bg)' }}
          whileTap={{ scale: 0.94 }}
          className="px-3 py-1 border border-border font-mono text-[9px] uppercase-tracked text-muted bg-transparent outline-none focus-visible:ring-2 focus-visible:ring-inset focus-visible:ring-accent"
        >
          LOAD
        </motion.button>
      </div>
    </header>
  )
}
