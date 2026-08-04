import { useEffect, useRef } from 'react'
import {
  createOscilloscopeSignal,
  type OscilloscopeSignal,
} from '../lib/oscilloscopeSignal'

/** Frequency labels preserved from the prototype's FFT band. */
const FREQUENCY_LABELS = ['20Hz', '200Hz', '2kHz', '20kHz']

interface FftVisualizerProps {
  /** Master bypass; mirrors the prototype's engineActive toggle. */
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

    const dpr = Math.min(window.devicePixelRatio || 1, 2)
    let rafId = 0

    // Size the backing store to the layout box and normalise coordinates to
    // CSS pixels so all drawing code is resolution-independent and crisp.
    const resize = () => {
      const rect = canvas.getBoundingClientRect()
      const width = Math.max(1, Math.round(rect.width * dpr))
      const height = Math.max(1, Math.round(rect.height * dpr))
      canvas.width = width
      canvas.height = height
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0)
    }
    resize()

    let resizeObserver: ResizeObserver | undefined
    if (typeof ResizeObserver !== 'undefined') {
      resizeObserver = new ResizeObserver(resize)
      resizeObserver.observe(canvas)
    }

    const trace = (stepSignal: boolean) => {
      if (stepSignal) resolvedSignal.step(activeRef.current)
      const samples = resolvedSignal.samples
      const width = canvas.clientWidth
      const height = canvas.clientHeight
      if (samples.length < 2) return

      // Scope face
      ctx.fillStyle = '#0f0e0e'
      ctx.fillRect(0, 0, width, height)

      // Graticule grid
      const gridColumns = 6
      const gridRows = 4
      ctx.strokeStyle = 'rgba(136, 136, 136, 0.12)'
      ctx.lineWidth = 1
      for (let i = 1; i < gridColumns; i++) {
        const x = (width / gridColumns) * i
        ctx.beginPath()
        ctx.moveTo(x + 0.5, 0)
        ctx.lineTo(x + 0.5, height)
        ctx.stroke()
      }
      for (let i = 1; i < gridRows; i++) {
        const y = (height / gridRows) * i
        ctx.beginPath()
        ctx.moveTo(0, y + 0.5)
        ctx.lineTo(width, y + 0.5)
        ctx.stroke()
      }

      // Zero axis
      ctx.strokeStyle = 'rgba(136, 136, 136, 0.22)'
      ctx.beginPath()
      ctx.moveTo(0, height / 2 + 0.5)
      ctx.lineTo(width, height / 2 + 0.5)
      ctx.stroke()

      // Trace path — one pass over the sample buffer.
      const midY = height / 2
      const amplitude = height * 0.42
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
      ctx.strokeStyle = isLive ? 'rgba(246, 246, 246, 0.16)' : 'rgba(136, 136, 136, 0.14)'
      ctx.lineWidth = 5
      ctx.lineCap = 'round'
      ctx.stroke()

      ctx.strokeStyle = isLive ? '#f6f6f6' : '#888888'
      ctx.lineWidth = 1.5
      ctx.stroke()
    }

    // Honour reduced-motion: render one static frame instead of animating.
    const reduceMotion = window.matchMedia?.('(prefers-reduced-motion: reduce)')
    if (reduceMotion?.matches) {
      trace(true)
    } else {
      const draw = () => {
        rafId = requestAnimationFrame(draw)
        trace(true)
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
