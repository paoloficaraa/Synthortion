import { useEffect, useState } from 'react'
import { motion, useReducedMotion } from 'framer-motion'

/** Milliseconds before the boot sequence auto-dismisses (motion allowed). */
const BOOT_DURATION_MS = 1800

/**
 * SystemBoot — the mount-time boot sequence overlay.
 *
 * Plays once when the plugin is first rendered: a fixed full-cover layer with
 * the SYNTHORTION boot text and a skip hint. It auto-dismisses after a short
 * pause; users who prefer reduced motion see a static overlay they can dismiss
 * with a click instead of an animated one.
 */
export function SystemBoot() {
  const [visible, setVisible] = useState(true)
  const reduceMotion = useReducedMotion()

  useEffect(() => {
    if (reduceMotion) return
    const timer = window.setTimeout(() => setVisible(false), BOOT_DURATION_MS)
    return () => window.clearTimeout(timer)
  }, [reduceMotion])

  if (!visible) return null

  return (
    <motion.button
      type="button"
      data-testid="system-boot-overlay"
      aria-label="Dismiss boot sequence"
      onClick={() => setVisible(false)}
      initial={{ opacity: 0 }}
      animate={{ opacity: 1 }}
      transition={{ duration: reduceMotion ? 0 : 0.25 }}
      className="fixed inset-0 z-[100] flex flex-col items-center justify-center gap-6 bg-void cursor-pointer outline-none"
    >
      {/* Soft radial atmosphere behind the boot text */}
      <div className="absolute inset-0 pointer-events-none bg-gradient-glow opacity-[0.12]" />
      <span className="font-display text-[28px] text-fg uppercase-tracked select-none relative z-10">
        SYNTHORTION
      </span>
      <span className="font-mono text-[11px] text-muted uppercase-tracked select-none relative z-10">
        SYSTEM BOOT ... OK
      </span>
      <span className="font-mono text-[9px] text-ink-2 uppercase-tracked select-none relative z-10">
        CLICK TO SKIP
      </span>
    </motion.button>
  )
}
