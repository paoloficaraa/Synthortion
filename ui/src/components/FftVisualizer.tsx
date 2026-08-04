import { useRef, useEffect } from 'react'
import { createTimeSignal, type TimeSignal } from '../lib/timeSignal'

/** Frequency labels preserved from the prototype's FFT band. */
const FREQUENCY_LABELS = ['20Hz', '200Hz', '2kHz', '20kHz']

/** Classic oscilloscope phosphor green. */
const TRACE_COLOR = '#39ff14'

/** Subtle grid line colour on the dark backdrop. */
const GRID_COLOR = 'rgba(255,255,255,0.04)'

/** Centre-line colour — faint horizontal guide. */
const CENTRE_COLOR = 'rgba(57,255,20,0.12)'

interface FftVisualizerProps {
  /** Master bypass; mirrors the prototype's engineActive toggle. */
  active: boolean
  /**
   * Injectable signal source so tests can feed mock frames. Defaults to the
   * time-domain generator — visual logic stays isolated from DSP.
   */
  signal?: TimeSignal
}

/**
 * FftVisualizer — 2D HTML5 Canvas oscilloscope.
 *
 * Replaces the former React Three Fiber 3D spectrum with a crisp, high-
 * performance 2D canvas trace. The scope sweeps a time-domain waveform left
 * to right, drawing a phosphor-green trace on the dark `#0f0e0e` backdrop.
 * No WebGL dependencies.
 */
export function FftVisualizer({ active, signal }: FftVisualizerProps) {
  const fallbackRef = useRef<TimeSignal | null>(null)
  const resolvedSignal = signal ?? (fallbackRef.current ??= createTimeSignal())
  const canvasRef = useRef<HTMLCanvasElement | null>(null)
  const rafRef = useRef(0)
  const drawRef = useRef<() => void>(() => {})
  // Backing-store size in device pixels, cached so the draw loop below never
  // reads layout per frame — updated only when the canvas actually resizes.
  const sizeRef = useRef({ width: 0, height: 0 })

  // Keep the canvas backing store matched to its CSS size × devicePixelRatio
  // so the trace stays crisp on high-DPI displays. Observing size (rather than
  // measuring per frame) keeps the rAF loop pure draw work.
  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return

    const resize = () => {
      const dpr = typeof window !== 'undefined' ? window.devicePixelRatio || 1 : 1
      const rect = canvas.getBoundingClientRect()
      const width = Math.max(1, Math.round(rect.width * dpr))
      const height = Math.max(1, Math.round(rect.height * dpr))
      if (canvas.width !== width || canvas.height !== height) {
        canvas.width = width
        canvas.height = height
      }
      sizeRef.current = { width, height }
    }

    resize()
    if (typeof ResizeObserver !== 'undefined') {
      const observer = new ResizeObserver(resize)
      observer.observe(canvas)
      return () => observer.disconnect()
    }
    window.addEventListener('resize', resize)
    return () => window.removeEventListener('resize', resize)
  }, [])

  // Refresh the draw closure after every render so it always reads the latest
  // `active` and `resolvedSignal` without re-scheduling the rAF loop below.
  useEffect(() => {
    drawRef.current = () => {
      const canvas = canvasRef.current
      if (!canvas) return

      const ctx = canvas.getContext('2d')
      if (!ctx) return

      const { width: w, height: h } = sizeRef.current
      if (w < 1 || h < 1) return

      // Draw in CSS pixel coordinates; the backing store is DPR-scaled above.
      const dpr = typeof window !== 'undefined' ? window.devicePixelRatio || 1 : 1
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0)

      resolvedSignal.step(active)
      const { samples } = resolvedSignal

      // Clear
      ctx.fillStyle = '#0f0e0e'
      ctx.fillRect(0, 0, w, h)

      // Grid lines (vertical)
      ctx.strokeStyle = GRID_COLOR
      ctx.lineWidth = 1
      const cols = 10
      for (let i = 1; i < cols; i++) {
        const x = (i / cols) * w
        ctx.beginPath()
        ctx.moveTo(x, 0)
        ctx.lineTo(x, h)
        ctx.stroke()
      }

      // Centre horizontal line
      ctx.strokeStyle = CENTRE_COLOR
      ctx.beginPath()
      ctx.moveTo(0, h / 2)
      ctx.lineTo(w, h / 2)
      ctx.stroke()

      // Oscilloscope trace
      ctx.strokeStyle = TRACE_COLOR
      ctx.lineWidth = 2
      ctx.shadowColor = TRACE_COLOR
      ctx.shadowBlur = 6
      ctx.beginPath()
      for (let i = 0; i < samples.length; i++) {
        const x = (i / (samples.length - 1)) * w
        const y = h / 2 - samples[i] * (h / 2) * 0.9
        if (i === 0) ctx.moveTo(x, y)
        else ctx.lineTo(x, y)
      }
      ctx.stroke()

      // Reset shadow for next frame
      ctx.shadowBlur = 0

      rafRef.current = requestAnimationFrame(() => drawRef.current())
    }
  })

  // Start (and stop) the sweep loop once; the closure is kept fresh above.
  useEffect(() => {
    rafRef.current = requestAnimationFrame(() => drawRef.current())
    return () => cancelAnimationFrame(rafRef.current)
  }, [])

  return (
    <div
      data-testid="fft-visualizer"
      data-active={active}
      className="relative w-full h-[240px] bg-bg overflow-hidden border-b border-border shadow-[inset_0_-1px_3px_rgba(0,0,0,0.5)]"
    >
      <canvas ref={canvasRef} className="block w-full h-full" />

      {/* Hardware bezel shadow */}
      <div className="absolute inset-0 pointer-events-none shadow-[inset_0_12px_24px_rgba(0,0,0,0.9)]" />

      {/* Frequency labels (kept from the prototype) */}
      <div
        className="absolute bottom-3 right-4 flex gap-6 opacity-30"
        aria-hidden="true"
      >
        {FREQUENCY_LABELS.map((label) => (
          <span key={label} className="font-mono text-[9px] text-fg">
            {label}
          </span>
        ))}
      </div>
    </div>
  )
}
