import { useRef, useState, type KeyboardEvent, type PointerEvent } from 'react'
import { motion } from 'framer-motion'

interface KnobProps {
  /** Label displayed below the knob */
  label: string
  /** Current value */
  value: number
  /** Minimum value */
  min?: number
  /** Maximum value */
  max?: number
  /** Formatted display value (shown while enabled) */
  displayValue: string
  /** Change handler */
  onChange: (value: number) => void
  /** Size variant */
  size?: 'default' | 'small'
  /**
   * Whether the control is enabled (module powered). `false` renders a dimmed
   * track with a `--` readout and makes the control inert, ready for the
   * module power wiring from T04.
   */
  enabled?: boolean
}

/** Pointer cells in the block track — 9 renders the canonical `[====+----]`. */
const TRACK_CELLS = 9

/** Drag sensitivity in value-range percent per CSS pixel (on a 0..100 range). */
const DRAG_SENSITIVITY = 0.5

/** Shift held during drag scales normal sensitivity by this (fine step, ×0.1). */
const FINE_STEP_FACTOR = 0.1

/**
 * Knob — horizontal ASCII block control with live numeric readout.
 *
 * Replaces the rotary knob face: the value renders as a `[====+----]` track
 * (filled `=`, pointer `+`, empty `-`) in the VGA voice with a mono readout
 * beneath. The drag gesture stays vertical and the whole track width is a hit
 * area; Shift held during drag applies a fine step (×0.1 of normal) so wide
 * ranges stay reachable. Keyboard navigation (arrows, Home/End) is unchanged.
 * When `enabled` is false the track is dimmed and the readout shows `--`.
 */
export function Knob({
  label,
  value,
  min = 0,
  max = 100,
  displayValue,
  onChange,
  size = 'default',
  enabled = true,
}: KnobProps) {
  const isDragging = useRef(false)
  const [isDraggingState, setIsDraggingState] = useState(false)
  const startY = useRef(0)
  const startVal = useRef(0)

  const handlePointerDown = (e: PointerEvent<HTMLDivElement>) => {
    if (!enabled) return
    isDragging.current = true
    setIsDraggingState(true)
    startY.current = e.clientY
    startVal.current = value
    // jsdom and some embedders lack pointer capture; guard so tests/harnesses
    // can exercise drag math without a real pointer session.
    if (e.currentTarget.setPointerCapture) {
      e.currentTarget.setPointerCapture(e.pointerId)
    }
  }

  const handlePointerMove = (e: PointerEvent<HTMLDivElement>) => {
    if (!isDragging.current) return
    const dy = startY.current - e.clientY
    const sensitivity =
      DRAG_SENSITIVITY * (e.shiftKey ? FINE_STEP_FACTOR : 1)
    let newValue = startVal.current + dy * sensitivity * ((max - min) / 100)
    newValue = Math.max(min, Math.min(max, newValue))
    onChange(newValue)
  }

  const handlePointerEnd = (e: PointerEvent<HTMLDivElement>) => {
    isDragging.current = false
    setIsDraggingState(false)
    if (e.currentTarget.hasPointerCapture?.(e.pointerId)) {
      e.currentTarget.releasePointerCapture(e.pointerId)
    }
  }

  const handleKeyDown = (e: KeyboardEvent<HTMLDivElement>) => {
    if (!enabled) return
    const step = (max - min) / 100
    const largeStep = (max - min) / 10
    let newValue = value

    switch (e.key) {
      case 'ArrowUp':
      case 'ArrowRight':
        newValue += e.shiftKey ? largeStep : step
        break
      case 'ArrowDown':
      case 'ArrowLeft':
        newValue -= e.shiftKey ? largeStep : step
        break
      case 'Home':
        newValue = min
        break
      case 'End':
        newValue = max
        break
      default:
        return
    }
    e.preventDefault()
    newValue = Math.max(min, Math.min(max, newValue))
    onChange(newValue)
  }

  // ASCII block track — the value maps to a pointer index across the cells;
  // cells before it fill with `=`, after it stay empty `-`.
  const pct = max === min ? 0 : (value - min) / (max - min)
  const pointerIndex = Math.round(pct * (TRACK_CELLS - 1))
  const cells: Array<{ char: string; filled: boolean }> = []
  for (let i = 0; i < TRACK_CELLS; i++) {
    cells.push({
      char: i === pointerIndex ? '+' : i < pointerIndex ? '=' : '-',
      filled: i <= pointerIndex,
    })
  }

  return (
    <div className="flex flex-col items-center gap-1.5">
      <motion.div
        role="slider"
        tabIndex={enabled ? 0 : -1}
        aria-disabled={!enabled}
        aria-label={label}
        aria-orientation="vertical"
        aria-valuemin={min}
        aria-valuemax={max}
        aria-valuenow={Math.round(value)}
        aria-valuetext={enabled ? displayValue : '--'}
        className="relative cursor-ns-resize px-2 py-1 select-none focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-2 focus-visible:ring-offset-bg"
        animate={{ scale: isDraggingState ? 1.05 : 1 }}
        transition={{ duration: 0.14, ease: 'easeOut' }}
        onPointerDown={handlePointerDown}
        onPointerMove={handlePointerMove}
        onPointerUp={handlePointerEnd}
        onPointerCancel={handlePointerEnd}
        onKeyDown={handleKeyDown}
      >
        <span
          className={`font-ascii leading-none ${
            size === 'small' ? 'text-[8px]' : 'text-[16px]'
          } ${enabled ? '' : 'opacity-40'}`}
          aria-hidden="true"
          data-testid="knob-track"
        >
          <span className="text-ink-3">[</span>
          {cells.map((cell, i) => (
            <span key={i} className={cell.filled ? 'text-fg' : 'text-ink-3'}>
              {cell.char}
            </span>
          ))}
          <span className="text-ink-3">]</span>
        </span>
      </motion.div>
      <div className="flex flex-col items-center mt-1">
        <span
          className={`font-mono uppercase-tracked select-none ${
            size === 'small' ? 'text-[9px]' : 'text-[10px]'
          } ${enabled ? 'text-fg' : 'text-ink-3'}`}
        >
          {enabled ? displayValue : '--'}
        </span>
        <span
          className={`font-display text-muted uppercase-tracked select-none mt-1 ${
            size === 'small' ? 'text-[8px]' : 'text-[9px]'
          }`}
          aria-hidden="true"
        >
          {label}
        </span>
      </div>
    </div>
  )
}
