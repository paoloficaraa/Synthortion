interface CalibrationTicksProps {
  side: 'left' | 'right'
  className?: string
}

/**
 * CalibrationTicks — micro-scale 1px tick marks for Cartesian frames.
 * Three 7px marks (`-` `+` `-`) at justify-between, aria-hidden, industrial xerox anchor.
 * Shared between ModuleFrame and MeterRail to avoid duplication.
 */
export function CalibrationTicks({ side, className }: CalibrationTicksProps) {
  const sideClass = side === 'left' ? 'left-0 items-start pl-0.5' : 'right-0 items-end pr-0.5'
  return (
    <div
      className={`absolute top-2 bottom-2 w-1 flex flex-col justify-between pointer-events-none select-none text-ink-3 font-ascii text-[7px] leading-none opacity-60 ${sideClass} ${className ?? ''}`}
      aria-hidden="true"
    >
      <span>-</span>
      <span>+</span>
      <span>-</span>
    </div>
  )
}
