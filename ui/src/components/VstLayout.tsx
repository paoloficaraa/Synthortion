import type { ReactNode } from 'react'
import { CornerBracket } from './CornerBracket'
import { CalibrationTicks } from './CalibrationTicks'

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


/** Fixed-width meter rail that flanks the center hub — Cartesian terminal frame. */
function MeterRail({ side, children }: MeterRailProps) {
  const borderClass = side === 'left' ? 'border-r' : 'border-l'
  return (
    <div
      className={`w-[48px] shrink-0 bg-elev-0 flex flex-col items-center py-6 ${borderClass} border-border z-10 relative`}
    >
      {/* Rail ticks along inner hairline — shared CalibrationTicks */}
      <CalibrationTicks side={side === 'left' ? 'right' : 'left'} className="opacity-50" />
      {/* Chassis corner crosshairs */}
      <div
        className={`absolute top-0 ${side === 'left' ? 'right-0 translate-x-[3px]' : 'left-0 -translate-x-[3px]'} -translate-y-[1px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none`}
        aria-hidden="true"
      >
        +
      </div>
      <div
        className={`absolute bottom-0 ${side === 'left' ? 'right-0 translate-x-[3px]' : 'left-0 -translate-x-[3px]'} translate-y-[4px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none`}
        aria-hidden="true"
      >
        +
      </div>
      {/* Shared-hairline crosshairs at header (50px) intersection */}
      <div
        className={`absolute top-[50px] ${side === 'left' ? 'right-0 translate-x-[3px]' : 'left-0 -translate-x-[3px]'} -translate-y-[4px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none`}
        aria-hidden="true"
      >
        +
      </div>
      <div className="relative z-10 w-full flex flex-col items-center">{children}</div>
    </div>
  )
}

/**
 * VstLayout — 3-column instrument chassis with Cartesian frame integration.
 *
 * Renders a horizontal flex-row layout with:
 * - Left: Input gain meter rail (48px, Cartesian ticks, 1px #333 hairline on inner edge)
 * - Center: Main controls hub (Header 54px → Dual-Mode Visualizer 240px → Faceplate grid, each separated by a single 1px grid-rule)
 * - Right: Output gain meter rail (48px, mirrored)
 *
 * The `+` crosshairs at rail/header and rail/visualizer intersections tie the flanking rails
 * into the shared Cartesian coordinate system, eliminating the repetitive `│`/`─` AI-slop framing.
 * Full-height 1px borders separate meter columns from the center hub. The panel enters with a
 * subtle rise+fade (disabled by `prefers-reduced-motion`), the four corners carry the ASCII
 * box-drawing corner brackets, and the container carries the static CRT scanline + noise-overlay
 * xerox texture at z-40/50 behind the 1px hairlines at z-10.
 */
export function VstLayout({ leftColumn, children, rightColumn }: VstLayoutProps) {
  return (
    <div className="w-full h-full flex flex-col flex-1 min-h-0 min-w-0">
      <div className="vst-container flex flex-row w-full h-full flex-1 min-h-0 relative noise-overlay">
        <CornerBracket position="tl" />
        <CornerBracket position="tr" />
        <CornerBracket position="bl" />
        <CornerBracket position="br" />

        {/* Left meter rail */}
        <MeterRail side="left">{leftColumn}</MeterRail>

        {/* Center hub — Header → Visualizer → Faceplate share a single 1px Cartesian grid */}
        <div className="flex flex-col flex-1 min-w-0 min-h-0 bg-bg">
          {children}
        </div>

        {/* Right meter rail */}
        <MeterRail side="right">{rightColumn}</MeterRail>
      </div>
    </div>
  )
}
