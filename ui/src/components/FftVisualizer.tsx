import { useEffect, useRef } from 'react'
import {
  createOscilloscopeSignal,
  type OscilloscopeSignal,
} from '../lib/oscilloscopeSignal'
import { applyGlitch, type GlitchPulser } from '../lib/glitchPulser'
import {
  buildGraticule,
  buildTrace,
  CELL_PX,
} from '../lib/fftBraille'

/* ------------------------------------------------------------------ */
/*  Canvas palette — mirrors tokens in src/styles/globals.css          */
/* ------------------------------------------------------------------ */
const SCOPE_BG = '#0f0e0e'
const GRAT_FG = 'rgba(136, 136, 136, 0.25)'
const TRACE_FG = '#f6f6f6'
const TRACE_IDLE = '#888888'
const TRACE_HALO_LIVE = 'rgba(246, 246, 246, 0.12)'
const TRACE_HALO_IDLE = 'rgba(136, 136, 136, 0.10)'

/* ------------------------------------------------------------------ */
/*  Grid geometry                                                      */
/* ------------------------------------------------------------------ */
const MAX_DPR = 2

/* ------------------------------------------------------------------ */
/*  Font stacks (canvas fillText)                                      */
/* ------------------------------------------------------------------ */
const FONT_GRAT = `${CELL_PX}px "Px437 IBM VGA 8x16", monospace`
const FONT_TRACE = `${CELL_PX}px "Braille Terminal", monospace`

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
  const fallbackRef = useRef<OscilloscopeSignal | null>(null)
  const resolvedSignal = signal ?? (fallbackRef.current ??= createOscilloscopeSignal())

  const canvasRef = useRef<HTMLCanvasElement | null>(null)
  const activeRef = useRef(active)

  useEffect(() => {
    activeRef.current = active
  }, [active])

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return
    const ctx = canvas.getContext('2d')
    if (!ctx) return

    const dpr = Math.min(window.devicePixelRatio || 1, MAX_DPR)
    let rafId = 0
    let staticFrame = false
    let lastTimestamp = 0

    /** Compute number of character columns from the canvas CSS width. */
    const computeCols = (cssWidth: number): number => {
      // Px437 8x16 advance = 8px at 16px font-size.
      ctx.font = FONT_GRAT
      const charWidth = ctx.measureText('─').width || 8
      return Math.max(1, Math.floor(cssWidth / charWidth))
    }

    const trace = () => {
      resolvedSignal.step(activeRef.current)
      const samples = resolvedSignal.samples
      const width = canvas.clientWidth
      const height = canvas.clientHeight
      if (samples.length < 2 || width === 0) return

      const numCols = computeCols(width)
      const now = performance.now()
      const dt = lastTimestamp ? Math.min(now - lastTimestamp, 100) : 16
      lastTimestamp = now

      // ── Scope face ──
      ctx.fillStyle = SCOPE_BG
      ctx.fillRect(0, 0, width, height)

      // ── Graticule (static — box-drawing in VGA face, row-by-row) ──
      const gratRows = buildGraticule(numCols).split('\n')
      ctx.font = FONT_GRAT
      ctx.textBaseline = 'top'
      ctx.fillStyle = GRAT_FG
      for (let i = 0; i < gratRows.length; i++) {
        ctx.fillText(gratRows[i], 0, i * CELL_PX)
      }

      // ── Trace (dynamic — braille, drawn row-by-row) ──
      let traceRows = buildTrace(samples, numCols)

      // ── Glitch corruption ──
      const glitchIntensity = glitch?.step(dt) ?? 0
      if (glitchIntensity > 0) {
        traceRows = applyGlitch(traceRows, glitchIntensity, random)
      }

      const isLive = activeRef.current

      // Soft halo behind the trace.
      ctx.font = FONT_TRACE
      ctx.fillStyle = isLive ? TRACE_HALO_LIVE : TRACE_HALO_IDLE
      for (let i = 0; i < traceRows.length; i++) {
        ctx.fillText(traceRows[i], 0, i * CELL_PX)
      }

      // Crisp trace.
      ctx.fillStyle = isLive ? TRACE_FG : TRACE_IDLE
      for (let i = 0; i < traceRows.length; i++) {
        ctx.fillText(traceRows[i], 0, i * CELL_PX)
      }
    }

    // ── Size backing store to layout box ──
    const resize = () => {
      const rect = canvas.getBoundingClientRect()
      const w = Math.max(1, Math.round(rect.width * dpr))
      const h = Math.max(1, Math.round(rect.height * dpr))
      canvas.width = w
      canvas.height = h
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0)
      if (staticFrame) trace()
    }
    resize()

    let resizeObserver: ResizeObserver | undefined
    if (typeof ResizeObserver !== 'undefined') {
      resizeObserver = new ResizeObserver(resize)
      resizeObserver.observe(canvas)
    }

    // ── Reduced-motion: single static frame, no animation ──
    const reduceMotion = window.matchMedia?.(
      '(prefers-reduced-motion: reduce)',
    )
    if (reduceMotion?.matches) {
      staticFrame = true
      lastTimestamp = performance.now()
      trace()
    } else {
      const draw = () => {
        rafId = requestAnimationFrame(draw)
        trace()
      }
      rafId = requestAnimationFrame(draw)
    }

    return () => {
      cancelAnimationFrame(rafId)
      resizeObserver?.disconnect()
    }
  }, [resolvedSignal, glitch, random])

  return (
    <div
      data-testid="fft-visualizer"
      data-active={active}
      className="relative w-full h-[136px] shrink-0 bg-bg overflow-hidden border-b border-border shadow-[inset_0_-1px_3px_rgba(0,0,0,0.5)]"
    >
      <canvas
        ref={canvasRef}
        className="absolute inset-0 w-full h-full"
        aria-hidden="true"
      />

      {/* Hardware bezel shadow */}
      <div className="absolute inset-0 pointer-events-none shadow-[inset_0_12px_24px_rgba(0,0,0,0.9)]" />
    </div>
  )
}
