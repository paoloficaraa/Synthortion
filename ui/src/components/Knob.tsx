import { useRef, useState, type KeyboardEvent, type PointerEvent } from 'react'

interface KnobProps {
  /** Label displayed below the knob */
  label: string
  /** Current value */
  value: number
  /** Default value for double-click reset */
  defaultValue?: number
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

/** CRT noise symbols for character-scrambling glitch corruption during drag. */
const GLITCH_CHARS = ['░', '▒', '▓', '█', '┼', '▚', '?', '#', '!', '&']

/** Formats ASCII scale ruler indicators (e.g. `0% . . + . . 100%`, `2B . . + . . 24B`). */
function formatRulerText(min: number, max: number, displayValue: string): string {
  const unitMatch = displayValue.match(/[%a-zA-Z]+$/)
  const unit = unitMatch ? unitMatch[0] : ''
  const minStr = `${min}${unit}`
  const maxStr = max >= 1000 ? `${max / 1000}k${unit}` : `${max}${unit}`
  return `${minStr} . . + . . ${maxStr}`
}
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
  defaultValue,
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

  const handleDoubleClick = () => {
    if (!enabled || defaultValue === undefined) return
    onChange(defaultValue)
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

  // ASCII block track with progressive dither
  const pct = max === min ? 0 : (value - min) / (max - min)
  const ditherChars = ['-', '░', '▒', '▓']
  const totalSteps = TRACK_CELLS * 4
  const currentStep = Math.round(pct * totalSteps)

  const cells: Array<{ char: string; filled: boolean }> = []
  for (let i = 0; i < TRACK_CELLS; i++) {
    const cellStep = currentStep - i * 4
    let char = '-'
    let filled = false
    if (cellStep >= 4) {
      char = '█'
      filled = true
    } else if (cellStep <= 0) {
      char = '-'
      filled = false
    } else {
      char = ditherChars[cellStep]
      filled = true
    }

    if (isDraggingState) {
      char = GLITCH_CHARS[(i + Math.round(value)) % GLITCH_CHARS.length]
    }
    cells.push({ char, filled })
  }

  return (
    <div className="flex flex-col items-center gap-1.5">
      <div
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
        style={{
          transform: isDraggingState ? 'scale(1.05)' : 'scale(1)',
          transition: 'transform 0.14s ease-out'
        }}
        onPointerDown={handlePointerDown}
        onPointerMove={handlePointerMove}
        onPointerUp={handlePointerEnd}
        onPointerCancel={handlePointerEnd}
        onDoubleClick={handleDoubleClick}
        onKeyDown={handleKeyDown}
      >
        <span
          className={`font-ascii leading-none inline-block whitespace-nowrap ${
            size === 'small' ? 'text-[8px]' : 'text-[16px]'
          } ${enabled ? '' : 'opacity-40'}`}
          aria-hidden="true"
          data-testid="knob-track"
        >
          <span className={`text-ink-3 ${isDraggingState ? 'knob-glitch' : ''}`}>[</span>
          {cells.map((cell, i) => (
            <span key={i} className={cell.filled ? 'text-fg' : 'text-ink-3'}>
              {cell.char}
            </span>
          ))}
          <span className={`text-ink-3 ${isDraggingState ? 'knob-glitch' : ''}`}>]</span>
        </span>
        <div className="text-[6px] text-ink-3 mt-0.5 whitespace-nowrap">
          {formatRulerText(min, max, displayValue)}
        </div>
      </div>
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
