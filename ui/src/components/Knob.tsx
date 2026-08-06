import { useId, useRef, useState, type KeyboardEvent, type PointerEvent } from 'react'
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
  /** Formatted display value */
  displayValue: string
  /** Change handler */
  onChange: (value: number) => void
  /** Size variant */
  size?: 'default' | 'small'
}

/**
 * Knob - Rotary control with SVG polar trajectory
 *
 * Supports:
 * - Vertical mouse drag tracking
 * - Keyboard navigation (Arrow keys, Home/End)
 * - Focus-visible ring for accessibility
 */
export function Knob({
  label,
  value,
  min = 0,
  max = 100,
  displayValue,
  onChange,
  size = 'default',
}: KnobProps) {
  const isDragging = useRef(false)
  const [isDraggingState, setIsDraggingState] = useState(false)
  const startY = useRef(0)
  const startVal = useRef(0)

  const handlePointerDown = (e: PointerEvent<HTMLDivElement>) => {
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
    const sensitivity = 0.5
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

  const radius = size === 'small' ? 14 : 22
  const viewBoxSize = size === 'small' ? 40 : 64
  const center = viewBoxSize / 2
  const innerRadius = radius - (size === 'small' ? 5 : 7)
  const indicatorTip = radius + (size === 'small' ? 1 : 2)
  const pct = (value - min) / (max - min)
  const angle = -135 + pct * 270

  const C = 2 * Math.PI * radius
  const maxArcLength = C * 0.75
  const strokeDashoffset = C - Math.max(0.001, pct * maxArcLength)
  const bgStrokeDashoffset = C - maxArcLength

  // Unique id per knob so the machined-face gradient never collides across the
  // eight on-panel instances (SVG url(#) fragments resolve to the first match).
  const faceGradientId = `knob-face-${useId().replace(/:/g, '')}`

  return (
    <div className="flex flex-col items-center gap-1.5">
      <motion.div
        role="slider"
        tabIndex={0}
        aria-label={label}
        aria-orientation="vertical"
        aria-valuemin={min}
        aria-valuemax={max}
        aria-valuenow={Math.round(value)}
        aria-valuetext={displayValue}
        className="relative cursor-ns-resize group rounded-full shadow-lift focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-4 focus-visible:ring-offset-bg flex items-center justify-center"
        style={{ width: viewBoxSize, height: viewBoxSize }}
        animate={{ scale: isDraggingState ? 1.05 : 1 }}
        transition={{ duration: 0.14, ease: 'easeOut' }}
        onPointerDown={handlePointerDown}
        onPointerMove={handlePointerMove}
        onPointerUp={handlePointerEnd}
        onPointerCancel={handlePointerEnd}
        onKeyDown={handleKeyDown}
      >
        <svg
          className="absolute inset-0 w-full h-full"
          viewBox={`0 0 ${viewBoxSize} ${viewBoxSize}`}
        >
          <defs>
            {/* Machined metal face — mirrors the --gradient-metal token (SVG
                cannot read CSS custom properties, so the stops are literal). */}
            <radialGradient
              id={faceGradientId}
              gradientUnits="objectBoundingBox"
              cx="38%"
              cy="34%"
              r="75%"
            >
              <stop offset="0%" stopColor="#3a3a3a" />
              <stop offset="45%" stopColor="#1a1a1a" />
              <stop offset="100%" stopColor="#0c0c0c" />
            </radialGradient>
          </defs>
          {/* Background arc */}
          <circle
            cx={center}
            cy={center}
            r={radius}
            fill="none"
            stroke="var(--border)"
            strokeWidth={size === 'small' ? '2' : '3'}
            strokeDasharray={C}
            strokeDashoffset={bgStrokeDashoffset}
            transform={`rotate(135 ${center} ${center})`}
            strokeLinecap="round"
          />
          {/* Soft glow under the active arc — a wide low-opacity accent stroke
              (the FftVisualizer double-stroke trick, no shadowBlur cost). */}
          <circle
            cx={center}
            cy={center}
            r={radius}
            fill="none"
            stroke="rgba(199, 195, 186, 0.22)"
            strokeWidth={size === 'small' ? '5' : '7'}
            strokeDasharray={C}
            strokeDashoffset={strokeDashoffset}
            transform={`rotate(135 ${center} ${center})`}
            strokeLinecap="round"
          />
          {/* Active value arc */}
          <circle
            cx={center}
            cy={center}
            r={radius}
            fill="none"
            stroke="var(--accent)"
            strokeWidth={size === 'small' ? '2' : '3'}
            strokeDasharray={C}
            strokeDashoffset={strokeDashoffset}
            transform={`rotate(135 ${center} ${center})`}
            strokeLinecap="round"
            className="transition-all duration-75"
          />
          {/* Inner circle — the machined knob face */}
          <circle
            cx={center}
            cy={center}
            r={innerRadius}
            fill={`url(#${faceGradientId})`}
            stroke="var(--elev-6)"
            strokeWidth="1"
          />
          {/* Indicator line */}
          <g transform={`rotate(${angle} ${center} ${center})`}>
            <line
              x1={center}
              y1={center - innerRadius}
              x2={center}
              y2={center - indicatorTip}
              stroke="var(--fg)"
              strokeWidth={size === 'small' ? '1.5' : '2'}
              strokeLinecap="round"
            />
          </g>
        </svg>
      </motion.div>
      <div className="flex flex-col items-center mt-1">
        <span
          className={`font-mono text-fg uppercase-tracked select-none ${
            size === 'small' ? 'text-[9px]' : 'text-[10px]'
          }`}
        >
          {displayValue}
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
