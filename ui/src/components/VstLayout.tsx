import type { ReactNode } from 'react'

interface VstLayoutProps {
  /** Left column content (Input meter rail) */
  leftColumn?: ReactNode
  /** Center column content (Main controls) */
  children: ReactNode
  /** Right column content (Output meter rail) */
  rightColumn?: ReactNode
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
      <div className="w-[40px] shrink-0 bg-[#050505] flex flex-col items-center py-6 border-r border-[#222] z-10 relative shadow-[inset_0_0_12px_rgba(0,0,0,0.8)]">
        {leftColumn}
      </div>

      {/* Center hub */}
      <div className="flex flex-col flex-1 min-w-0">
        {children}
      </div>

      {/* Right meter rail */}
      <div className="w-[40px] shrink-0 bg-[#050505] flex flex-col items-center py-6 border-l border-[#222] z-10 relative shadow-[inset_0_0_12px_rgba(0,0,0,0.8)]">
        {rightColumn}
      </div>
    </div>
  )
}

export default VstLayout
