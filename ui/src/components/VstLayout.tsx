import type { ReactNode } from 'react'
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
      className={`w-[48px] shrink-0 bg-elev-0 flex flex-col items-center py-6 ${borderClass} border-border z-10 relative`}
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
    <div
      className="w-full h-full flex flex-col flex-1 min-h-0 min-w-0"
    >
      <div className="vst-container flex flex-row w-full h-full flex-1 min-h-0 relative noise-overlay">
        <CornerBracket position="tl" />
        <CornerBracket position="tr" />
        <CornerBracket position="bl" />
        <CornerBracket position="br" />

        {/* Left meter rail */}
        <MeterRail side="left">{leftColumn}</MeterRail>

        {/* Center hub */}
        <div className="flex flex-col flex-1 min-w-0 min-h-0">
          {children}
        </div>

        {/* Right meter rail */}
        <MeterRail side="right">{rightColumn}</MeterRail>
      </div>
    </div>
  )
}
