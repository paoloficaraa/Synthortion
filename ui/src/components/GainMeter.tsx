import { useRef, useEffect, useCallback } from 'react'
import { subscribeToDspMeters } from '../lib/webViewDspBridge'

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

/** Gutter micro-ticks for the ladder well — deduped column. */
function GutterTicks({ offset }: { offset: string }) {
  return (
    <div
      className={`absolute top-1 bottom-1 left-1/2 ${offset} flex flex-col justify-between pointer-events-none select-none font-ascii text-[6px] leading-none text-ink-3 opacity-40`}
      aria-hidden="true"
    >
      <span>-</span>
      <span>+</span>
      <span>-</span>
    </div>
  )
}
/** Colors matching the design tokens. */
const METER_VOID = '#030303'
const METER_WELL = '#0a0a0a'
const METER_LEVEL = '#888888'
const METER_PEAK = '#ffffff'

interface GainMeterProps {
  /** Label displayed above the meter */
  label: string
  /** Whether the meter is actively showing signal */
  active: boolean
  /** "input" or "output" rail selector */
  channel?: 'input' | 'output'
  /** Animation delay in ms */
  delay?: number
}

/**
 * GainMeter — 16-row vertical block-character ladder meter with Cartesian xerox framing.
 *
 * Renders a character-cell ladder on a 8×256 canvas using fillText with ▁▂▃▄▅▆▇█
 * block characters (one char per 8px column). Peak rows (top 2) use ▲ when fully
 * filled. The chassis carries the Cartesian coordinate discipline: 1px hairline
 * well (`--elev-6`), micro calibration ticks (`-`/`+`) along the rail, `+`
 * crosshairs at the top/bottom where the ladder meets the horizontal rules,
 * and a dithered halftone anchor (`░▒`/`▒░`) tying the meter into the high-
 * contrast xerox instrument language. Animation uses a smoothed random signal;
 * bypass decays to void; `aria-hidden` hides every decorative glyph while
 * `role="meter"` exposes dB semantics. Monochrome discipline per DESIGN.md.
 */
export function GainMeter({ label, active, channel = 'input', delay = 0 }: GainMeterProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const meterRef = useRef<HTMLDivElement>(null)
  const readoutTextRef = useRef<HTMLSpanElement>(null)
  const levelRef = useRef(0)
  const targetRef = useRef(0)

  const updateLevel = useCallback(() => {
    if (active) {
      const target = targetRef.current
      if (target > levelRef.current) {
        // Instant attack
        levelRef.current = target
      } else {
        // Exponential decay ~250ms half-life at 60fps (0.956 per 16ms frame)
        levelRef.current *= 0.956
        if (levelRef.current < 0.0005) levelRef.current = 0
      }
    } else {
      // Bypass decay ~100ms half-life (0.895 per frame)
      levelRef.current *= 0.895
      if (levelRef.current < 0.0005) levelRef.current = 0
    }
  }, [active])
  useEffect(() => {
    if (!active) {
      targetRef.current = 0
      return
    }
    return subscribeToDspMeters((frame) => {
      targetRef.current = channel === 'input' ? frame.input : frame.output
    })
  }, [active, channel])

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return

    const ctx = canvas.getContext('2d', { alpha: false })
    if (!ctx) return

    let raf: number

    const draw = () => {
      updateLevel()

      // Clear background
      ctx.fillStyle = METER_VOID
      ctx.fillRect(0, 0, canvas.width, canvas.height)

      // Set up font for block characters
      ctx.font = `${FONT_SIZE}px "Px437 IBM VGA8", "IBM VGA 8", monospace`
      ctx.textBaseline = TEXT_BASELINE

      const totalEighths = levelRef.current * METER_ROWS * EIGHTHS_PER_ROW

      for (let i = 0; i < METER_ROWS; i++) {
        const rowsBelow = METER_ROWS - 1 - i
        const eighths = Math.max(0, Math.min(EIGHTHS_PER_ROW, totalEighths - rowsBelow * EIGHTHS_PER_ROW))

        if (eighths <= 0) {
          if (active || levelRef.current > 0.01) {
            ctx.fillStyle = METER_WELL
            ctx.fillText('·', 0, i * FONT_SIZE)
          }
          continue
        }

        const isPeak = i < PEAK_ROWS
        const fill = Math.max(1, Math.min(EIGHTHS_PER_ROW, Math.round(eighths)))
        const char = isPeak && fill >= EIGHTHS_PER_ROW ? PEAK_GLYPH : BLOCK_CHARS[fill - 1]
        ctx.fillStyle = isPeak && fill >= EIGHTHS_PER_ROW ? METER_PEAK : METER_LEVEL
        ctx.fillText(char, 0, i * FONT_SIZE)
      }

      // Update dynamic ARIA attributes and bracketed dB readout
      if (meterRef.current && readoutTextRef.current) {
        if (levelRef.current < 0.001) {
          meterRef.current.setAttribute('aria-valuenow', '-120')
          meterRef.current.setAttribute('aria-valuetext', '-INF')
          readoutTextRef.current.textContent = '[ -INF ]'
        } else {
          const db = Math.min(0, Math.max(-60, Math.round(20 * Math.log10(levelRef.current + 1e-6))))
          const absVal = Math.abs(db).toString().padStart(2, '0')
          const text = db <= -60 ? '[ -INF ]' : `[ ${db < 0 ? '-' : '+'}${absVal}dB ]`

          meterRef.current.setAttribute('aria-valuenow', db.toString())
          meterRef.current.setAttribute('aria-valuetext', text)
          readoutTextRef.current.textContent = text
        }
      }

      raf = requestAnimationFrame(draw)
    }

    raf = requestAnimationFrame(draw)
    return () => cancelAnimationFrame(raf)
  }, [updateLevel])
  return (
    <div
      className="w-full flex flex-col items-center animate-vst-enter relative"
      style={{ animationDelay: `${delay}ms` }}
      role="meter"
      aria-label={`${label} Meter`}
      aria-valuemin={-60}
      aria-valuemax={0}
      ref={meterRef}
    >
      {/* Accessible readout text — visually hidden */}
      <span className="sr-only" ref={readoutTextRef}>[ -INF ]</span>

      {/* Visual meter rail — aria-hidden to decorative glyphs */}
      <div aria-hidden="true" className="flex flex-col items-center">
        <div className="font-ascii text-[9px] whitespace-pre leading-none flex items-center gap-1">
          <span className="text-ink-3">+</span>
          <span>┌</span>
          <span> </span>
          <span className="text-fg font-bold tracking-widest">{label}</span>
          <span> </span>
          <span>┐</span>
          <span className="text-ink-3">+</span>
        </div>

        <div className="relative mt-1">
          <canvas
            ref={canvasRef}
            width={CANVAS_WIDTH}
            height={CANVAS_HEIGHT}
            className="block"
          />
          <GutterTicks offset="right-1" />
        </div>

        <div className="font-ascii text-[9px] whitespace-pre leading-none flex items-center gap-1 mt-1">
          <span className="text-ink-3">+</span>
          <span>└</span>
          <span> </span>
          <span className="text-fg">dB</span>
          <span> </span>
          <span>┘</span>
          <span className="text-ink-3">+</span>
        </div>
      </div>
    </div>
  )
}