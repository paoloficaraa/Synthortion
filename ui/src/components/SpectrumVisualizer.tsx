import { useEffect, useRef, useState } from 'react'
import {
  NUM_BANDS,
  CELL_PX,
  buildSpectrumTraceAndDither,
  buildSpectrumGraticule,
} from '../lib/spectrumBraille'
import { subscribeToDspSpectrum, type SpectrumFrameCallback } from '../lib/webViewDspBridge'
import { type GlitchPulser, applyGlitch } from '../lib/glitchPulser'

/* ------------------------------------------------------------------ */
/*  Canvas palette                                                     */
/* ------------------------------------------------------------------ */
const SCOPE_BG = '#0f0e0e'
const GRAT_FG = '#333333'
const TRACE_FG = '#f6f6f6'
const TRACE_IDLE = '#666666'
const DITHER_FG = '#c7c3ba'

/* ------------------------------------------------------------------ */
/*  Grid geometry & Typography                                         */
/* ------------------------------------------------------------------ */
const MAX_DPR = 2
const CELL_W_FALLBACK = 8
const FONT_VGA = `${CELL_PX}px "Px437 IBM VGA 8x16", "JetBrains Mono", monospace`
const FONT_BRAILLE = `${CELL_PX}px "Braille Terminal", "Px437 IBM VGA 8x16", monospace`


export interface SpectrumVisualizerProps {
  /** Whether the engine is live; a bypassed engine decays the trace to floor. */
  active: boolean
  /**
   * Injectable spectrum stream subscriber. Defaults to the C++ WebView bridge
   * 60 FPS spectrum stream.
   */
  subscribeSpectrum?: (onFrame: SpectrumFrameCallback) => () => void
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
 * SpectrumVisualizer — 60 FPS Real-time Post-FX Frequency Spectrum Analyzer.
 *
 * Renders an 80-band log-frequency magnitude spectrum spanning 20 Hz to 20 kHz
 * as an upper Braille peak contour (U+2800..U+28FF) in stark white (#f6f6f6)
 * over a halftone dither gradient fill ( ░▒▓█) in warm grey (#c7c3ba) within a
 * 1px Cartesian graticule featuring '+' crosshairs and frequency calibration marks
 * at 20 Hz, 200 Hz, 2 kHz, and 20 kHz.
 */
export function SpectrumVisualizer({
  active,
  subscribeSpectrum = subscribeToDspSpectrum,
  glitch,
  random = Math.random,
}: SpectrumVisualizerProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const motionQuery =
    typeof window !== 'undefined' && typeof window.matchMedia === 'function'
      ? window.matchMedia('(prefers-reduced-motion: reduce)')
      : null
  const staticFrame = motionQuery?.matches ?? false
  const [nonce, setNonce] = useState(0)

  // Current incoming magnitude stream and rendered decay state
  const targetMagsRef = useRef<Float32Array>(new Float32Array(NUM_BANDS))
  const displayMagsRef = useRef<Float32Array>(new Float32Array(NUM_BANDS))

  // 1. Subscribe to the lock-free 60 FPS spectrum stream from the C++ DSP engine
  useEffect(() => {
    const unsubscribe = subscribeSpectrum((magnitudes: number[]) => {
      const target = targetMagsRef.current
      const len = Math.min(magnitudes.length, NUM_BANDS)
      for (let i = 0; i < len; i++) {
        target[i] = magnitudes[i] ?? 0
      }
    })
    return () => {
      unsubscribe()
    }
  }, [subscribeSpectrum])

  // 2. Render loop on canvas
  useEffect(() => {
    const ctx = canvasRef.current?.getContext('2d', { alpha: false })
    if (!ctx) return

    let rafId = 0
    let w = 0
    let h = 0
    let isDecaying = false
    const graticuleCache = new Map<string, string>()

    const resize = () => {
      if (!canvasRef.current) return
      const rect = canvasRef.current.getBoundingClientRect()
      const dpr = Math.min(window.devicePixelRatio || 1, MAX_DPR)
      w = rect.width
      h = rect.height
      canvasRef.current.width = w * dpr
      canvasRef.current.height = h * dpr
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0)
    }

    let resizeObserver: ResizeObserver | null = null
    if (typeof ResizeObserver !== 'undefined') {
      resizeObserver = new ResizeObserver(resize)
      if (canvasRef.current) resizeObserver.observe(canvasRef.current)
    }
    resize()

    let lastFrameTime = performance.now()

    const drawFrame = (): boolean => {
      const now = performance.now()
      const dt = Math.min(50, Math.max(1, now - lastFrameTime))
      lastFrameTime = now

      // Smooth ballistics and bypass decay
      const target = targetMagsRef.current
      const display = displayMagsRef.current
      let hasActiveSignal = false

      if (!active) {
        // Bypass mode: exponential decay to 0 floor
        const decay = Math.exp(-dt / 120)
        let maxVal = 0
        for (let i = 0; i < NUM_BANDS; i++) {
          const v = (display[i] ?? 0) * decay
          display[i] = v
          if (v > maxVal) maxVal = v
        }
        if (maxVal > 0.0005) {
          hasActiveSignal = true
          isDecaying = true
        } else {
          display.fill(0)
          isDecaying = false
        }
      } else {
        // Live mode: update display buffer
        hasActiveSignal = true
        isDecaying = true
        for (let i = 0; i < NUM_BANDS; i++) {
          const t = target[i] ?? 0
          const curr = display[i] ?? 0
          if (t >= curr) {
            // Instant attack
            display[i] = t
          } else {
            // Smooth decay (~180 ms)
            const decay = Math.exp(-dt / 180)
            display[i] = curr * decay + t * (1 - decay)
          }
        }
      }

      const cellW =
        (ctx.measureText?.('█') as { width?: number } | undefined)?.width || CELL_W_FALLBACK
      const numCols = Math.max(8, Math.floor(w / cellW))
      const numRows = Math.max(4, Math.floor(h / CELL_PX))

      ctx.fillStyle = SCOPE_BG
      ctx.fillRect(0, 0, w, h)
      ctx.textBaseline = 'top'

      // ── Layer 1: Cartesian graticule with frequency labels and crosshairs ──
      const gratKey = `${numCols}x${numRows}`
      let grat = graticuleCache.get(gratKey)
      if (!grat) {
        grat = buildSpectrumGraticule(numCols, numRows)
        graticuleCache.set(gratKey, grat)
      }
      ctx.font = FONT_VGA
      ctx.fillStyle = GRAT_FG
      grat.split('\n').forEach((row, i) => ctx.fillText(row, 0, i * CELL_PX))

      // ── Layer 2: Halftone dither gradient fill & Braille peak contour ──
      const glitchIntensity = glitch ? glitch.step(dt) : 0
      const generated = buildSpectrumTraceAndDither(display, numCols, numRows)
      let traceRows = generated.trace
      let ditherRows = generated.dither

      if (glitchIntensity > 0) {
        traceRows = applyGlitch(traceRows, glitchIntensity, random)
        ditherRows = applyGlitch(ditherRows, glitchIntensity, random)
      }

      // Render halftone dither gradient fill under the curve in warm grey
      ctx.font = FONT_VGA
      ctx.fillStyle = DITHER_FG
      ditherRows.forEach((row, r) => {
        if (row.trim().length > 0) {
          ctx.fillText(row, 0, r * CELL_PX)
        }
      })

      // Render primary Braille peak contour in stark white
      ctx.font = FONT_BRAILLE
      ctx.fillStyle = active ? TRACE_FG : TRACE_IDLE
      traceRows.forEach((row, r) => {
        if (row.trim().length > 0) {
          ctx.fillText(row, 0, r * CELL_PX)
        }
      })

      return active || hasActiveSignal || (glitchIntensity > 0)
    }

    if (motionQuery?.matches) {
      // Static clean frame: one draw, no animation loop
      glitch?.step(1000)
      drawFrame()
    } else {
      const loop = () => {
        const shouldContinue = drawFrame()
        if (shouldContinue) {
          rafId = requestAnimationFrame(loop)
        }
      }
      rafId = requestAnimationFrame(loop)
    }

    const onMotionChange = () => {
      cancelAnimationFrame(rafId)
      resizeObserver?.disconnect()
      motionQuery?.removeEventListener('change', onMotionChange)
      setNonce((n) => n + 1)
    }
    motionQuery?.addEventListener('change', onMotionChange)

    return () => {
      cancelAnimationFrame(rafId)
      resizeObserver?.disconnect()
      motionQuery?.removeEventListener('change', onMotionChange)
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [active, glitch, random, nonce])

  return (
    <div
      data-testid="spectrum-visualizer"
      data-active={active}
      data-mode="spectrum"
      data-static={staticFrame}
      className="relative w-full basis-[35%] flex-grow min-h-0 bg-bg border-b border-grid-rule"
    >
      <div className="absolute inset-0 overflow-hidden">
        <canvas ref={canvasRef} className="absolute inset-0 w-full h-full" aria-hidden="true" />
      </div>
      {/* Cartesian bottom corners — '+' centered on shared hairline where scope band meets flanking rails */}
      <div
        className="absolute bottom-0 left-0 translate-y-[4px] -translate-x-[3px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none"
        aria-hidden="true"
      >
        +
      </div>
      <div
        className="absolute bottom-0 right-0 translate-y-[4px] translate-x-[3px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none"
        aria-hidden="true"
      >
        +
      </div>
    </div>
  )
}
