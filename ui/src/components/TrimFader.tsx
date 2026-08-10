import { useRef, type KeyboardEvent, type PointerEvent } from 'react'
import { motion } from 'framer-motion'

interface TrimFaderProps {
  /** Current dB value (-24..+24) */
  value: number
  /** Formatted display value (shown while enabled) */
  displayValue: string
  /** Change handler */
  onChange: (value: number) => void
  /** Whether the control is enabled */
  enabled?: boolean
  /** Label for accessibility */
  label?: string
}

/** Number of track cells in the fader (each cell = 1 row). */
const TRACK_ROWS = 10

/** Drag sensitivity in value-range percent per CSS pixel (on a 0..100 range). */
const DRAG_SENSITIVITY = 0.5

/** Shift held during drag scales normal sensitivity by this (fine step, ×0.1). */
const FINE_STEP_FACTOR = 0.1

/** Block characters for the track. */
const BLOCK_FULL = '█'
const BLOCK_EMPTY = '·'
const POINTER = '▶'

const MIN_DB = -24
const MAX_DB = 24

/**
 * TrimFader — compact vertical block fader with position marker and live dB readout.
 *
 * Renders a 10-row track: `█` below the pointer, `·` above, `▶` at the pointer.
 * Drag is vertical (clientY), Shift applies fine step (×0.1). Keyboard:
 * ArrowUp/Down (Shift = large step), Home/End. Clamped at -24..+24 dB.
 * When disabled, track is dimmed and readout shows `--`.
 */
export function TrimFader({
  value,
  displayValue,
  onChange,
  enabled = true,
  label = 'TRIM',
}: TrimFaderProps) {
  const isDragging = useRef(false)
  const startY = useRef(0)
  const startVal = useRef(0)

  const handlePointerDown = (e: PointerEvent<HTMLDivElement>) => {
    if (!enabled) return
    isDragging.current = true
    startY.current = e.clientY
    startVal.current = value
    if (e.currentTarget.setPointerCapture) {
      e.currentTarget.setPointerCapture(e.pointerId)
    }
  }

  const handlePointerMove = (e: PointerEvent<HTMLDivElement>) => {
    if (!isDragging.current) return
    const dy = startY.current - e.clientY
    const sensitivity =
      DRAG_SENSITIVITY * (e.shiftKey ? FINE_STEP_FACTOR : 1) * ((MAX_DB - MIN_DB) / 100)
    let newValue = startVal.current + dy * sensitivity
    newValue = Math.max(MIN_DB, Math.min(MAX_DB, newValue))
    onChange(newValue)
  }

  const handlePointerEnd = (e: PointerEvent<HTMLDivElement>) => {
    isDragging.current = false
    if (e.currentTarget.hasPointerCapture?.(e.pointerId)) {
      e.currentTarget.releasePointerCapture(e.pointerId)
    }
  }

  const handleKeyDown = (e: KeyboardEvent<HTMLDivElement>) => {
    if (!enabled) return
    const step = 1
    const largeStep = 6
    let newValue = value

    switch (e.key) {
      case 'ArrowUp':
        newValue += e.shiftKey ? largeStep : step
        break
      case 'ArrowDown':
        newValue -= e.shiftKey ? largeStep : step
        break
      case 'Home':
        newValue = MIN_DB
        break
      case 'End':
        newValue = MAX_DB
        break
      default:
        return
    }
    e.preventDefault()
    newValue = Math.max(MIN_DB, Math.min(MAX_DB, newValue))
    onChange(newValue)
  }

  const pct = (value - MIN_DB) / (MAX_DB - MIN_DB)
  const pointerRow = Math.round(pct * (TRACK_ROWS - 1))

  const track = Array.from({ length: TRACK_ROWS }, (_, i) => {
    const rowFromBottom = TRACK_ROWS - 1 - i
    if (rowFromBottom === pointerRow) return POINTER
    return rowFromBottom < pointerRow ? BLOCK_FULL : BLOCK_EMPTY
  })

  const readout = enabled ? displayValue : '--'

  return (
    <div className="flex flex-col items-center gap-1.5" data-testid="trim-fader">
      <motion.div
        role="slider"
        tabIndex={enabled ? 0 : -1}
        aria-disabled={!enabled}
        aria-label={label}
        aria-orientation="vertical"
        aria-valuemin={MIN_DB}
        aria-valuemax={MAX_DB}
        aria-valuenow={Math.round(value)}
        aria-valuetext={readout}
        className="relative cursor-ns-resize px-1 py-1 select-none focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-2 focus-visible:ring-offset-bg"
        onPointerDown={handlePointerDown}
        onPointerMove={handlePointerMove}
        onPointerUp={handlePointerEnd}
        onPointerCancel={handlePointerEnd}
        onKeyDown={handleKeyDown}
      >
        <div
          className={`font-ascii leading-[1.1] ${
            enabled ? '' : 'opacity-40'
          }`}
          aria-hidden="true"
          data-testid="trim-track"
        >
          {track.map((char, i) => (
            <span key={i} className={char === POINTER ? 'text-fg' : char === BLOCK_FULL ? 'text-fg' : 'text-ink-3'}>
              {char}
            </span>
          ))}
        </div>
      </motion.div>
      <div className="flex flex-col items-center mt-1">
        <span
          className={`font-mono uppercase-tracked select-none ${
            enabled ? 'text-fg' : 'text-ink-3'
          }`}
        >
          {readout}
        </span>
        <span
          className={`font-display text-muted uppercase-tracked select-none mt-1 text-[8px]`}
          aria-hidden="true"
        >
          {label}
        </span>
      </div>
    </div>
  )
}