import type { ReactNode } from 'react'

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
      className={`w-[40px] shrink-0 bg-[#050505] flex flex-col items-center py-6 ${borderClass} border-[#222] z-10 relative shadow-[inset_0_0_12px_rgba(0,0,0,0.8)]`}
    >
      {children}
    </div>
  )
}

/**
 * VstLayout - 3-column glass frame shell for VST plugin UI
 *
 * Renders a horizontal flex-row layout with:
 * - Left: Input gain meter rail (40px width)
 * - Center: Main controls hub (flexible)
 * - Right: Output gain meter rail (40px width)
 *
 * Full-height borders separate meter columns from the center hub.
 */
export function VstLayout({ leftColumn, children, rightColumn }: VstLayoutProps) {
  return (
    <div className="vst-container flex flex-row mx-auto max-w-full relative noise-overlay">
      {/* Left meter rail */}
      <MeterRail side="left">{leftColumn}</MeterRail>

      {/* Center hub */}
      <div className="flex flex-col flex-1 min-w-0">
        {children}
      </div>

      {/* Right meter rail */}
      <MeterRail side="right">{rightColumn}</MeterRail>
    </div>
  )
}
