import { useRef, useEffect, type ReactNode } from 'react'

/**
 * Canvas palette — token-equivalent hex for the meter draw loop. The 2D API
 * cannot read CSS custom properties, so these literals stay in step with the
 * design tokens in `globals.css` (--void, --elev-1, --ink-4, --fg). Keep them
 * in lock-step with the tokens; the meter backdrop deliberately matches
 * `--void` so it reads as the same recessed well as the app chassis.
 */
const METER_VOID = '#030303' // --void: meter track backdrop
const METER_WELL = '#0a0a0a' // --elev-1: unfilled segment blocks
const METER_LEVEL = '#888888' // --ink-4: active signal blocks
const METER_PEAK = '#f6f6f6' // --fg: top-of-scale peak blocks

interface GainMeterProps {
  /** Label displayed above the meter */
  label: string
  /** Whether the meter is actively showing signal */
  active: boolean
  /** Animation delay in ms */
  delay?: number
  /** Optional children (typically a Knob) */
  children?: ReactNode
}

/**
 * GainMeter - 32-segment volume meter with brutalist aesthetic
 *
 * Renders a vertical meter bar with segmented blocks.
 * Peak levels shown in the foreground ink, normal levels in gray.
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
      ctx.fillStyle = METER_VOID
      ctx.fillRect(0, 0, canvas.width, canvas.height)

      if (active) {
        const rawTarget = Math.random() > 0.05 ? Math.random() * 0.7 + 0.2 : Math.random() * 0.99
        levelRef.current += (rawTarget - levelRef.current) * 0.25
      } else {
        levelRef.current += (0 - levelRef.current) * 0.2
      }

      const blocks = 32
      const blockH = canvas.height / blocks
      const activeCount = Math.floor(levelRef.current * blocks)

      for (let i = 0; i < blocks; i++) {
        const y = canvas.height - (i + 1) * blockH
        const isFilled = i < activeCount
        const isPeak = i >= blocks - 3

        if (isFilled) {
          ctx.fillStyle = isPeak ? METER_PEAK : METER_LEVEL
        } else {
          ctx.fillStyle = METER_WELL
        }

        ctx.fillRect(0, y + 2, canvas.width, Math.max(2, blockH - 4))
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
          className="w-[6px] h-full bg-elev-0"
          style={{ boxShadow: 'var(--shadow-well), 0 0 0 1px var(--elev-6)' }}
        >
          <canvas
            ref={canvasRef}
            width={12}
            height={800}
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
