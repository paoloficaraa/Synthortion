import { motion } from 'framer-motion'

export interface ToggleOption {
  /** Stable value reported through onChange when the segment is activated */
  value: string
  /** Label rendered inside the segment */
  label: string
}

interface ToggleProps {
  /** Accessible name for the segmented control group */
  label: string
  /** Segments to render, in display order */
  options: readonly ToggleOption[]
  /** Currently active segment value (single-select) */
  value: string
  /** Called with the segment value when it is activated */
  onChange: (value: string) => void
}

/**
 * Toggle - Segmented hardware row control
 *
 * Ports the prototype's segmented buttons (PRE/POST, SYNC/FREE/PING-PONG,
 * WIDE) into a single-select row. Each segment is a toggle button with
 * `aria-pressed` semantics; the active segment inverts to the foreground
 * color, inactive segments stay muted until hovered.
 *
 * A single-option Toggle doubles as an on/off switch (e.g. WIDE): pass the
 * boolean state as `value` (`'on'` / `'off'`) and flip it in `onChange`.
 */
export function Toggle({ label, options, value, onChange }: ToggleProps) {
  return (
    <div role="group" aria-label={label} className="w-full flex gap-1">
      {options.map((option) => {
        const isActive = option.value === value
        return (
          <motion.button
            key={option.value}
            type="button"
            aria-pressed={isActive}
            onClick={() => onChange(option.value)}
            whileTap={{ scale: 0.96 }}
            whileHover={
              isActive
                ? undefined
                : { backgroundColor: 'var(--fg)', color: 'var(--bg)', borderColor: 'var(--fg)' }
            }
            className={`flex-1 border font-mono text-[9px] py-1.5 uppercase outline-none focus-visible:ring-2 focus-visible:ring-inset focus-visible:ring-accent ${
              isActive
                ? 'bg-fg text-bg border-fg shadow-[inset_0_1px_2px_rgba(0,0,0,0.5)]'
                : 'bg-transparent border-border text-muted'
            }`}
          >
            {option.label}
          </motion.button>
        )
      })}
    </div>
  )
}
