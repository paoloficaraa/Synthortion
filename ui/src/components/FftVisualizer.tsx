import { useEffect, useRef } from 'react'
import {
  createOscilloscopeSignal,
  type OscilloscopeSignal,
} from '../lib/oscilloscopeSignal'

/**
 * Canvas palette — mirrors the design tokens in `src/styles/globals.css`
 * (`--bg`, `--fg`, `--muted`). The 2D API cannot read CSS custom properties,
 * so these literals are the single source for the scope's colours; keep them
 * in step with the tokens.
 */
const SCOPE_BG = '#0f0e0e' // --bg: scope face
const SCOPE_FG = '#f6f6f6' // --fg: live trace
const SCOPE_MUTED = '#888888' // --muted: idle trace and grid
const GRID_STROKE = 'rgba(136, 136, 136, 0.12)' // muted @ 12%
const ZERO_AXIS_STROKE = 'rgba(136, 136, 136, 0.22)' // muted @ 22%
const LIVE_HALO = 'rgba(246, 246, 246, 0.16)' // fg @ 16%
const IDLE_HALO = 'rgba(136, 136, 136, 0.14)' // muted @ 14%

/** Graticule geometry and trace scale. */
const GRID_COLUMNS = 6
const GRID_ROWS = 4
const TRACE_AMPLITUDE = 0.42 // fraction of scope height the trace swings
const TRACE_HALO_WIDTH = 5
const TRACE_LINE_WIDTH = 1.5
const MAX_DEVICE_PIXEL_RATIO = 2

/** Frequency labels carried over from the FFT visualizer; decorative only. */
const FREQUENCY_LABELS = ['20Hz', '200Hz', '2kHz', '20kHz']

interface FftVisualizerProps {
  /** Whether the engine is live; a bypassed engine decays the trace to idle. */
  active: boolean
  /**
   * Injectable signal source so tests can feed mock frames. Defaults to the
   * time-domain oscillator — visual logic stays isolated from DSP.
   */
  signal?: OscilloscopeSignal
}

/**
 * FftVisualizer — the 2D oscilloscope band above the faceplate.
 *
 * A plain HTML5 Canvas draws the time-domain trace on the `#0f0e0e` backdrop:
 * a faint graticule grid, a zero axis, and a crisp phosphor-style polyline
 * driven by the simulated oscillator. No WebGL, no scene graph — one 2D
 * context, one path per frame, so the loop stays allocation-light.
 */
export function FftVisualizer({ active, signal }: FftVisualizerProps) {
  const fallbackRef = useRef<OscilloscopeSignal | null>(null)
  const resolvedSignal = signal ?? (fallbackRef.current ??= createOscilloscopeSignal())

  const canvasRef = useRef<HTMLCanvasElement | null>(null)
  const activeRef = useRef(active)

  // Keep the live prop readable inside the requestAnimationFrame closure.
  useEffect(() => {
    activeRef.current = active
  }, [active])

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return
    const ctx = canvas.getContext('2d')
    if (!ctx) return

    const dpr = Math.min(window.devicePixelRatio || 1, MAX_DEVICE_PIXEL_RATIO)
    let rafId = 0
    // True while reduced-motion shows a single frame instead of animating.
    // A static frame must be redrawn on resize — resizing the backing store
    // clears the canvas and nothing else would repaint it.
    let staticFrame = false

    const trace = () => {
      resolvedSignal.step(activeRef.current)
      const samples = resolvedSignal.samples
      const width = canvas.clientWidth
      const height = canvas.clientHeight
      if (samples.length < 2) return

      // Scope face
      ctx.fillStyle = SCOPE_BG
      ctx.fillRect(0, 0, width, height)

      // Graticule grid — one path for every line, stroked once.
      ctx.strokeStyle = GRID_STROKE
      ctx.lineWidth = 1
      ctx.beginPath()
      for (let i = 1; i < GRID_COLUMNS; i++) {
        const x = (width / GRID_COLUMNS) * i
        ctx.moveTo(x + 0.5, 0)
        ctx.lineTo(x + 0.5, height)
      }
      for (let i = 1; i < GRID_ROWS; i++) {
        const y = (height / GRID_ROWS) * i
        ctx.moveTo(0, y + 0.5)
        ctx.lineTo(width, y + 0.5)
      }
      ctx.stroke()

      // Zero axis
      ctx.strokeStyle = ZERO_AXIS_STROKE
      ctx.beginPath()
      ctx.moveTo(0, height / 2 + 0.5)
      ctx.lineTo(width, height / 2 + 0.5)
      ctx.stroke()

      // Trace path — one pass over the sample buffer.
      const midY = height / 2
      const amplitude = height * TRACE_AMPLITUDE
      ctx.beginPath()
      for (let i = 0; i < samples.length; i++) {
        const x = (i / (samples.length - 1)) * width
        const y = midY - samples[i] * amplitude
        if (i === 0) ctx.moveTo(x, y)
        else ctx.lineTo(x, y)
      }

      // Soft phosphor halo, then the crisp line on top. No shadowBlur — the
      // double stroke keeps the glow without the fill-rate cost.
      const isLive = activeRef.current
      ctx.strokeStyle = isLive ? LIVE_HALO : IDLE_HALO
      ctx.lineWidth = TRACE_HALO_WIDTH
      ctx.lineCap = 'round'
      ctx.stroke()

      ctx.strokeStyle = isLive ? SCOPE_FG : SCOPE_MUTED
      ctx.lineWidth = TRACE_LINE_WIDTH
      ctx.stroke()
    }

    // Size the backing store to the layout box and normalise coordinates to
    // CSS pixels so all drawing code is resolution-independent and crisp.
    const resize = () => {
      const rect = canvas.getBoundingClientRect()
      const width = Math.max(1, Math.round(rect.width * dpr))
      const height = Math.max(1, Math.round(rect.height * dpr))
      canvas.width = width
      canvas.height = height
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0)
      if (staticFrame) trace()
    }
    resize()

    let resizeObserver: ResizeObserver | undefined
    if (typeof ResizeObserver !== 'undefined') {
      resizeObserver = new ResizeObserver(resize)
      resizeObserver.observe(canvas)
    }

    // Honour reduced-motion: render one static frame instead of animating.
    const reduceMotion = window.matchMedia?.('(prefers-reduced-motion: reduce)')
    if (reduceMotion?.matches) {
      staticFrame = true
      trace()
    } else {
      const draw = () => {
        rafId = requestAnimationFrame(draw)
        trace()
      }
      draw()
    }

    return () => {
      cancelAnimationFrame(rafId)
      resizeObserver?.disconnect()
    }
  }, [resolvedSignal])

  return (
    <div
      data-testid="fft-visualizer"
      data-active={active}
      className="relative w-full h-[240px] bg-bg overflow-hidden border-b border-border shadow-[inset_0_-1px_3px_rgba(0,0,0,0.5)]"
    >
      <canvas
        ref={canvasRef}
        className="absolute inset-0 w-full h-full"
        aria-hidden="true"
      />

      {/* Hardware bezel shadow */}
      <div className="absolute inset-0 pointer-events-none shadow-[inset_0_12px_24px_rgba(0,0,0,0.9)]" />

      {/* Frequency labels (kept from the FFT version) */}
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
