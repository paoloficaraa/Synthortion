import { useEffect, useRef } from 'react'
import {
  createOscilloscopeSignal,
  type OscilloscopeSignal,
} from '../lib/oscilloscopeSignal'
import { applyGlitch, type GlitchPulser } from '../lib/glitchPulser'
const CELL_PX = 16

/* ------------------------------------------------------------------ */
/*  Canvas palette                                                     */
/* ------------------------------------------------------------------ */
const SCOPE_BG = '#000000'
const GRAT_FG = '#333333'
const TRACE_FG = '#f6f6f6'
const TRACE_IDLE = '#666666'

/* ------------------------------------------------------------------ */
/*  Grid geometry                                                      */
/* ------------------------------------------------------------------ */
const MAX_DPR = 2
const FONT_GRAT = `${CELL_PX - 6}px "JetBrains Mono", monospace`
/* ================================================================== */
/*  Component                                                          */
/* ================================================================== */

interface FftVisualizerProps {
  /** Whether the engine is live; a bypassed engine decays the trace. */
  active: boolean
  /**
   * Injectable signal source so tests can feed mock frames. Defaults to the
   * time-domain oscillator — visual logic stays isolated from DSP.
   */
  signal?: OscilloscopeSignal
  /**
   * Injectable glitch pulser. When present the trace corrupts during
   * parameter tweaks and decays back to clean.
   */
  glitch?: GlitchPulser
  /**
   * Injectable PRNG for deterministic glitch in tests.
   * Defaults to `Math.random`.
   */
  random?: () => number
}

/**
 * FftVisualizer — the 2D oscilloscope band above the faceplate.
 *
 * A plain HTML5 Canvas draws a braille dot-matrix waveform on a box-drawing
 * graticule grid. The waveform is rendered as a prebuilt string of braille
 * characters (U+2800–28FF) via `fillText`; the graticule is box-drawing
 * glyphs in the VGA pixel font. A glitch pulser corrupts the trace field
 * proportionally to live parameter tweaks, decaying in ~300–500 ms.
 */
export function FftVisualizer({
  active,
  signal,
  glitch,
  random = Math.random,
}: FftVisualizerProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const fallbackRef = useRef<OscilloscopeSignal | null>(null)
  const resolvedSignal = signal ?? (fallbackRef.current ??= createOscilloscopeSignal())
  const reduceMotion = typeof window !== 'undefined' ? window.matchMedia('(prefers-reduced-motion: reduce)') : null

  useEffect(() => {
    const ctx = canvasRef.current?.getContext('2d', { alpha: false })
    if (!ctx) return

    let rafId: number
    let w = 0
    let h = 0

    const resize = () => {
      if (!canvasRef.current) return
      const rect = canvasRef.current.getBoundingClientRect()
      const dpr = Math.min(window.devicePixelRatio || 1, MAX_DPR)
      w = rect.width
      h = rect.height
      canvasRef.current.width = w * dpr
      canvasRef.current.height = h * dpr
      ctx.scale(dpr, dpr)
    }

    let resizeObserver: ResizeObserver | null = null
    if (typeof ResizeObserver !== 'undefined') {
      resizeObserver = new ResizeObserver(resize)
      if (canvasRef.current) resizeObserver.observe(canvasRef.current)
    }
    resize()

    const drawGraticule = () => {
      ctx.strokeStyle = GRAT_FG
      ctx.lineWidth = 1
      ctx.beginPath()
      // Vertical lines
      const NUM_DIVS = 4
      for (let i = 1; i < NUM_DIVS; i++) {
        const x = (w / NUM_DIVS) * i
        ctx.moveTo(x, 0)
        ctx.lineTo(x, h)
        // Crosshairs
        ctx.moveTo(x - 4, h / 2)
        ctx.lineTo(x + 4, h / 2)
      }
      // Center horizontal line
      ctx.moveTo(0, h / 2)
      ctx.lineTo(w, h / 2)
      ctx.stroke()

      // Readouts
      ctx.fillStyle = GRAT_FG
      ctx.font = FONT_GRAT
      ctx.textAlign = 'center'
      ctx.textBaseline = 'top'
      const labels = ['20', '200', '2k', '20k']
      for (let i = 1; i < NUM_DIVS; i++) {
        const x = (w / NUM_DIVS) * i
        ctx.fillText(labels[i-1], x, 4)
      }
    }

    const drawTrace = () => {
      const samples = resolvedSignal.samples
      const len = samples.length
      
      // Phosphor decay: dim existing canvas instead of clear

      drawGraticule()

      // Calculate glitch offset
      const g = glitch?.step() ?? 0
      let yOffset = 0
      if (g > 0 && random() > 0.5) {
        yOffset = (random() - 0.5) * h * g * 0.2
      }

      ctx.beginPath()
      ctx.strokeStyle = active ? TRACE_FG : TRACE_IDLE
      ctx.lineWidth = 1.5
      
      for (let i = 0; i < len; i++) {
        const x = (i / (len - 1)) * w
        const y = h / 2 - (samples[i] * (h / 2) * 0.8) + yOffset
        if (i === 0) ctx.moveTo(x, y)
        else ctx.lineTo(x, y)
      }
      ctx.stroke()
    }

    if (reduceMotion?.matches) {
      const drawStatic = () => {
        rafId = requestAnimationFrame(drawStatic)
        resolvedSignal.step(active)
        drawTrace()
      }
      rafId = requestAnimationFrame(drawStatic)
    } else {
      const drawLive = () => {
        rafId = requestAnimationFrame(drawLive)
        resolvedSignal.step(active)
        drawTrace()
      }
      rafId = requestAnimationFrame(drawLive)
    }

    return () => {
      cancelAnimationFrame(rafId)
      resizeObserver?.disconnect()
    }
  }, [resolvedSignal, glitch, random, active, reduceMotion])
  return (
    <div
      data-testid="fft-visualizer"
      data-active={active}
      className="relative w-full h-[136px] shrink-0 bg-bg overflow-hidden"
    >
      {/* ASCII Bottom Border Partition */}
      <div className="absolute bottom-0 left-0 right-0 font-ascii text-[10px] text-ink-3 leading-none flex overflow-hidden pointer-events-none z-10" aria-hidden="true">
        {Array.from({ length: 120 }).map((_, i) => <span key={i}>─</span>)}
      </div>
      <canvas
        ref={canvasRef}
        className="absolute inset-0 w-full h-full"
        aria-hidden="true"
      />

    </div>
  )
}
