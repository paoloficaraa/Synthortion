/** Chassis corner anchors for the box-drawing brackets. */
type Corner = 'tl' | 'tr' | 'bl' | 'br'

/** Box-drawing glyph for each corner (`┌ ┐ └ ┘`). */
const GLYPH: Record<Corner, string> = {
  tl: '┌',
  tr: '┐',
  bl: '└',
  br: '┘',
}

/** Absolute placement for each corner of the chassis. */
const POSITION_CLASS: Record<Corner, string> = {
  tl: 'top-0 left-0',
  tr: 'top-0 right-0',
  bl: 'bottom-0 left-0',
  br: 'bottom-0 right-0',
}

interface CornerBracketProps {
  /** Which corner of the chassis the bracket anchors. */
  position: Corner
}

/**
 * CornerBracket — decorative box-drawing corner for the chassis.
 *
 * Pure chrome: a single `┌ ┐ └ ┘` glyph in the ASCII/VGA face laid over the
 * chassis corners, replacing the old decorative rack screws. `pointer-events-none`
 * keeps it from stealing focus from controls, and `aria-hidden` removes it from
 * the accessibility tree.
 */
export function CornerBracket({ position }: CornerBracketProps) {
  return (
    <span
      data-testid="corner-bracket"
      aria-hidden="true"
      className={`absolute font-ascii text-[16px] leading-none text-ink-3 pointer-events-none select-none ${POSITION_CLASS[position]}`}
    >
      {GLYPH[position]}
    </span>
  )
}
