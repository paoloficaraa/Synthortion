import { useRef, useEffect, type ReactNode } from 'react'

/** Block characters for the 8 sub-segment levels (1/8 each). */
const BLOCK_CHARS = ['▁', '▂', '▃', '▄', '▅', '▆', '▇', '█']
const PEAK_GLYPH = '▲'

/** Meter configuration. */
const METER_ROWS = 16
const PEAK_ROWS = 2
const CANVAS_WIDTH = 8
const CANVAS_HEIGHT = 256
const FONT_SIZE = 16
const LINE_HEIGHT = 1
const TEXT_BASELINE = 'hanging' as CanvasTextBaseline

/** Colors matching the design tokens. */
const METER_VOID = '#030303'
const METER_WELL = '#0a0a0a'
const METER_LEVEL = '#888888'
const METER_PEAK = '#f6f6f6'

interface GainMeterProps {
  /** Label displayed above the meter */
  label: string
  /** Whether the meter is actively showing signal */
  active: boolean
  /** Animation delay in ms */
  delay?: number
  /** Optional children (typically a TrimFader) */
  children?: ReactNode
}

/**
 * GainMeter - 16-row vertical block character ladder meter.
 *
 * Renders a character-cell ladder using fillText with ▁▂▃▄▅▆▇█ block characters
 * (one char per 8px column). Peak rows (top 2) use ▲ when fully filled.
 * Animation uses the same smoothed random signal as the previous implementation.
 */
export function GainMeter({ label, active, delay = 0, children }: GainMeterProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const levelRef = useRef(0)

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return

    const ctx = canvas.getContext('2d')
    if (!ctx) return

    let raf: number

    const draw = () => {
      // Clear background
      ctx.fillStyle = METER_VOID
      ctx.fillRect(0, 0, canvas.width, canvas.height)

      // Set up font for block characters
      ctx.font = `${FONT_SIZE}px "Px437 IBM VGA8", "IBM VGA 8", monospace`
      ctx.textBaseline = TEXT_BASELINE

      if (active) {
        // Same signal logic as before: mostly 0.2-0.9 range, 5% spikes to 0.99
        const rawTarget = Math.random() > 0.05 ? Math.random() * 0.7 + 0.2 : Math.random() * 0.99
        levelRef.current += (rawTarget - levelRef.current) * 0.25
      } else {
        levelRef.current += (0 - levelRef.current) * 0.2
      }

      const totalEighths = levelRef.current * METER_ROWS * 8

      for (let i = 0; i < METER_ROWS; i++) {
        // Draw from bottom (i = METER_ROWS-1) to top (i = 0)
        const rowsBelow = METER_ROWS - 1 - i
        const eighths = Math.max(0, Math.min(8, totalEighths - rowsBelow * 8))

        if (eighths <= 0) continue

        const isPeak = i < PEAK_ROWS
        const fill = Math.max(1, Math.min(8, Math.round(eighths)))

        let char: string
        if (isPeak && fill >= 8) {
          char = PEAK_GLYPH
        } else if (fill >= 8) {
          char = BLOCK_CHARS[7]
        } else {
          char = BLOCK_CHARS[fill - 1]
        }

        const color = isPeak && fill >= 8 ? METER_PEAK : fill >= 8 ? METER_LEVEL : METER_WELL
        ctx.fillStyle = color

        // Each row is 16px tall (FONT_SIZE), drawn at y = i * FONT_SIZE
        const y = i * FONT_SIZE
        ctx.fillText(char, 0, y)
      }

      if (active || levelRef.current > 0.01) {
        raf = requestAnimationFrame(draw)
      } else {
        ctx.fillStyle = METER_VOID
        ctx.fillRect(0, 0, canvas.width, canvas.height)
      }
    }

    if (active) draw()
    else raf = requestAnimationFrame(draw)

    return () => cancelAnimationFrame(raf)
  }, [active])

  return (
    <div
      className="w-full flex flex-col items-center animate-vst-enter"
      style={{ animationDelay: `${delay}ms` }}
    >
      <div className="font-display text-[8px] text-ink-3 uppercase-tracked mb-5">
        {label}
      </div>
      <div className="font-mono text-[7px] text-ink-1 mb-2 font-bold leading-none">0</div>
      <div className="flex-1 w-full flex justify-center z-10 shrink min-h-0">
        <div
          className="w-[8px] h-[256px] bg-elev-0"
          style={{ boxShadow: 'var(--shadow-well), 0 0 0 1px var(--elev-6)' }}
        >
          <canvas
            ref={canvasRef}
            width={CANVAS_WIDTH}
            height={CANVAS_HEIGHT}
            className="w-full h-full block"
          />
        </div>
      </div>
      <div className="font-mono text-[7px] text-ink-1 mt-2 font-bold leading-none">
        -INF
      </div>
      {children && (
        <div className="mt-5 mb-1 flex flex-col items-center relative z-20">
          {children}
        </div>
      )}
    </div>
  )
}