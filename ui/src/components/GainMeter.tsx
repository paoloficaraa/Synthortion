import { useRef, useEffect } from 'react'

/** Block characters for the 8 sub-segment levels (1/EIGHTHS_PER_ROW each). */
const BLOCK_CHARS = ['▁', '▂', '▃', '▄', '▅', '▆', '▇', '█']
const PEAK_GLYPH = '▲'

/** Sub-segments per row; the ladder has BLOCK_CHARS.length eighths per row. */
const EIGHTHS_PER_ROW = BLOCK_CHARS.length

/** Meter configuration. */
const METER_ROWS = 16
const PEAK_ROWS = 2
const CANVAS_WIDTH = 8
const CANVAS_HEIGHT = 256
const FONT_SIZE = 16
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
}

/**
 * GainMeter - 16-row vertical block character ladder meter.
 *
 * Renders a character-cell ladder using fillText with ▁▂▃▄▅▆▇█ block characters
 * (one char per 8px column). Peak rows (top 2) use ▲ when fully filled.
 * Animation uses the same smoothed random signal as the previous implementation.
 */
export function GainMeter({ label, active, delay = 0 }: GainMeterProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const meterRef = useRef<HTMLDivElement>(null)
  const readoutTextRef = useRef<HTMLSpanElement>(null)
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

      const totalEighths = levelRef.current * METER_ROWS * EIGHTHS_PER_ROW

      for (let i = 0; i < METER_ROWS; i++) {
        // Draw from bottom (i = METER_ROWS-1) to top (i = 0)
        const rowsBelow = METER_ROWS - 1 - i
        const eighths = Math.max(
          0,
          Math.min(EIGHTHS_PER_ROW, totalEighths - rowsBelow * EIGHTHS_PER_ROW)
        )

        if (eighths <= 0) {
          if (active || levelRef.current > 0.01) {
            ctx.fillStyle = METER_WELL
            const y = i * FONT_SIZE
            ctx.fillText('·', 0, y)
          }
          continue
        }

        const isPeak = i < PEAK_ROWS
        const fill = Math.max(1, Math.min(EIGHTHS_PER_ROW, Math.round(eighths)))

        let char: string
        if (isPeak && fill >= EIGHTHS_PER_ROW) {
          char = PEAK_GLYPH
        } else if (fill >= EIGHTHS_PER_ROW) {
          char = BLOCK_CHARS[EIGHTHS_PER_ROW - 1]
        } else {
          char = BLOCK_CHARS[fill - 1]
        }

        const color =
          isPeak && fill >= EIGHTHS_PER_ROW
            ? METER_PEAK
            : fill >= EIGHTHS_PER_ROW
              ? METER_LEVEL
              : METER_WELL
        ctx.fillStyle = color

        const y = i * FONT_SIZE
        ctx.fillText(char, 0, y)
      }

      // Update dynamic ARIA attributes and bracketed dB readout
      if (meterRef.current && readoutTextRef.current) {
        if (levelRef.current < 0.01) {
          meterRef.current.setAttribute('aria-valuenow', '-120')
          meterRef.current.setAttribute('aria-valuetext', '-INF')
          readoutTextRef.current.textContent = '-INF'
        } else {
          const db = Math.min(0, Math.max(-60, Math.round((levelRef.current - 1) * 60)))
          const absVal = Math.abs(db).toString().padStart(2, '0')
          const text = `${db < 0 ? '-' : '+'}${absVal}dB`
          meterRef.current.setAttribute('aria-valuenow', db.toString())
          meterRef.current.setAttribute('aria-valuetext', text)
          readoutTextRef.current.textContent = text
        }
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
      <div className="font-ascii text-[9px] text-ink-3 whitespace-pre leading-none flex items-center" aria-hidden="true">
        <span>┌</span><span> </span><span className="text-fg">{label}</span><span> </span><span>┐</span>
      </div>
      <div className="font-mono text-[7px] text-ink-1 mb-1 font-bold leading-none">0</div>
      <div className="flex-1 w-full flex justify-center z-10 shrink min-h-0">
        <div className="font-ascii text-[9px] text-ink-2 leading-none select-none" aria-hidden="true">
          │
        </div>
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
        <div className="font-ascii text-[9px] text-ink-2 leading-none select-none" aria-hidden="true">
          │
        </div>
      </div>
      <div className="font-ascii text-[9px] text-ink-3 whitespace-pre leading-none mt-1" aria-hidden="true">
        └{'─'.repeat(label.length + 2)}┘
      </div>
      <div
        ref={meterRef}
        role="meter"
        aria-valuenow={-120}
        aria-valuemin={-120}
        aria-valuemax={0}
        aria-valuetext="-INF"
        className="font-mono text-[7px] text-ink-1 mt-2 font-bold leading-none"
      >
        <span aria-hidden="true">[ </span>
        <span ref={readoutTextRef}>-INF</span>
        <span aria-hidden="true"> ]</span>
      </div>
    </div>
  )
}