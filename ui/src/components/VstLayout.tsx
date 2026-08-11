import type { ReactNode } from 'react'
import { motion } from 'framer-motion'
import { CornerBracket } from './CornerBracket'

interface VstLayoutProps {
  /** Left column content (Input meter rail) */
  leftColumn?: ReactNode
  /** Center column content (Main controls) */
  children: ReactNode
  /** Right column content (Output meter rail) */
  rightColumn?: ReactNode
}

interface MeterRailProps {
  /** Which side of the rail faces the center hub */
  side: 'left' | 'right'
  /** Rail content (typically a GainMeter) */
  children?: ReactNode
}

/** Fixed-width meter rail that flanks the center hub. */
function MeterRail({ side, children }: MeterRailProps) {
  const borderClass = side === 'left' ? 'border-r' : 'border-l'
  return (
    <div
      className={`w-[48px] shrink-0 bg-gradient-well flex flex-col items-center py-6 ${borderClass} border-elev-6 z-10 relative shadow-[inset_0_0_12px_rgba(0,0,0,0.8)]`}
    >
      {children}
    </div>
  )
}

/**
 * VstLayout - 3-column glass frame shell for VST plugin UI
 *
 * Renders a horizontal flex-row layout with:
 * - Left: Input gain meter rail (48px width)
 * - Center: Main controls hub (flexible)
 * - Right: Output gain meter rail (48px width)
 *
 * Full-height borders separate meter columns from the center hub. The panel
 * enters with a subtle rise+fade (disabled by `prefers-reduced-motion`), and
 * the four corners carry the ASCII box-drawing corner brackets.
 */
export function VstLayout({ leftColumn, children, rightColumn }: VstLayoutProps) {
  return (
    <motion.div
      initial={{ opacity: 0, y: 12, scale: 0.99 }}
      animate={{ opacity: 1, y: 0, scale: 1 }}
      transition={{ duration: 0.22, ease: 'easeOut' }}
    >
      <div className="vst-container flex flex-row mx-auto max-w-full relative noise-overlay">
        <CornerBracket position="tl" />
        <CornerBracket position="tr" />
        <CornerBracket position="bl" />
        <CornerBracket position="br" />

        {/* Left meter rail */}
        <MeterRail side="left">{leftColumn}</MeterRail>

        {/* Center hub */}
        <div className="flex flex-col flex-1 min-w-0">
          {children}
        </div>

        {/* Right meter rail */}
        <MeterRail side="right">{rightColumn}</MeterRail>
      </div>
    </motion.div>
  )
}
