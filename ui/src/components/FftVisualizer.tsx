import { useEffect, useRef, useState } from 'react'
import {
  createOscilloscopeSignal,
  type OscilloscopeSignal,
} from '../lib/oscilloscopeSignal'
import { type GlitchPulser, applyGlitch } from '../lib/glitchPulser'
import { buildGraticule, buildTrace, buildDither, CELL_PX } from '../lib/fftBraille'
import { analyzeBands, createWaterfallHistory } from '../lib/spectrogram'

/* ------------------------------------------------------------------ */
/*  Canvas palette                                                     */
/* ------------------------------------------------------------------ */
const SCOPE_BG = '#000000'
const GRAT_FG = '#333333'
const TRACE_FG = '#f6f6f6'
const TRACE_IDLE = '#666666'
const DITHER_FG = '#888888'
const WATERFALL_FG = '#c7c3ba'

/* ------------------------------------------------------------------ */
/*  Grid geometry                                                      */
/* ------------------------------------------------------------------ */
const MAX_DPR = 2
const CELL_W_FALLBACK = 8
const FONT_VGA = `${CELL_PX}px "Px437 IBM VGA 8x16", "JetBrains Mono", monospace`
const FONT_BRAILLE = `${CELL_PX}px "Braille Terminal", "Px437 IBM VGA 8x16", monospace`
const FONT_LABEL = `${CELL_PX - 6}px "JetBrains Mono", monospace`

/** Share of the band height given to the waveform scope tier. */
const SCOPE_SHARE = 0.6

/** Phosphor persistence: alpha per retained trace generation (oldest first). */
const PHOSPHOR_ALPHAS = [0.15, 0.3]

/** Frequency calibration ticks on the waterfall axis. */
const FREQ_TICKS = ['20Hz', '200Hz', '2kHz', '20kHz'] as const

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
 * FftVisualizer — Minimeters-grade dual-mode analyzer band above the
 * faceplate.
 *
 * One canvas, two tiers: an upper oscilloscope rendering the time-domain
 * trace as braille dot-matrix glyphs (U+2800–28FF) with sub-pixel dither and
 * phosphor persistence on a Cartesian graticule, and a lower scrolling
 * waterfall spectrogram shading band energies with density glyphs
 * (` ░▒▓█`). A glitch pulser corrupts both tiers proportionally to live
 * parameter tweaks, decaying in ~300–500 ms. `prefers-reduced-motion`
 * renders a single static clean frame instead of animating.
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
  const motionQuery =
    typeof window !== 'undefined' && typeof window.matchMedia === 'function'
      ? window.matchMedia('(prefers-reduced-motion: reduce)')
      : null
  const staticFrame = motionQuery?.matches ?? false
  const [nonce, setNonce] = useState(0)

  useEffect(() => {
    const ctx = canvasRef.current?.getContext('2d', { alpha: false })
    if (!ctx) return

    let rafId = 0
    let w = 0
    let h = 0
    let wasActive = active
    let waterfall = createWaterfallHistory(2)
    const phosphor: string[][] = []
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

    /** Log-frequency x position (cells) for f in 20Hz–20kHz. */
    const freqCol = (hz: number, cols: number) =>
      Math.round((Math.log(hz / 20) / Math.log(1000)) * (cols - 1))

    /** Divider row between tiers: rules, '+' ticks, freq labels. */
    const buildDivider = (cols: number): string => {
      const cells = new Array<string>(cols).fill('─')
      cells[0] = '├'
      cells[cols - 1] = '┤'
      const hz = [20, 200, 2000, 20000]
      for (let i = 0; i < hz.length; i++) {
        const tick = 1 + freqCol(hz[i], cols - 2)
        cells[tick] = '+'
        const label = FREQ_TICKS[i]
        const start = Math.min(tick + 1, cols - 1 - label.length)
        for (let j = 0; j < label.length; j++) cells[start + j] = label[j]
      }
      return cells.join('')
    }

    let dividerCache = { cols: 0, text: '' }

    const drawFrame = (animate: boolean) => {
      const now = performance.now()
      const dt = Math.min(50, Math.max(1, now - lastFrameTime))
      lastFrameTime = now

      // Bypass transition: collapse the waterfall history once.
      if (wasActive && !active) waterfall.clear()
      wasActive = active

      const cellW =
        (ctx.measureText?.('█') as { width?: number } | undefined)?.width ||
        CELL_W_FALLBACK
      const numCols = Math.max(8, Math.floor(w / cellW))
      const totalRows = Math.max(6, Math.floor(h / CELL_PX))
      const scopeRows = Math.max(4, Math.round(totalRows * SCOPE_SHARE))
      const fallsRows = Math.max(2, totalRows - scopeRows - 1)

      ctx.fillStyle = SCOPE_BG
      ctx.fillRect(0, 0, w, h)
      ctx.textBaseline = 'top'

      // ── Upper tier: Cartesian graticule ──
      const gratKey = `${numCols}x${scopeRows}`
      let grat = graticuleCache.get(gratKey)
      if (!grat) {
        grat = buildGraticule(numCols, scopeRows)
        graticuleCache.set(gratKey, grat)
      }
      ctx.font = FONT_VGA
      ctx.fillStyle = GRAT_FG
      grat.split('\n').forEach((row, i) => ctx.fillText(row, 0, i * CELL_PX))

      // Amplitude calibration readouts along the left rail.
      ctx.font = FONT_LABEL
      ctx.fillText('+6dB', 2, 2)
      ctx.fillText('0dB', 2, (Math.floor(scopeRows / 2) - 1) * CELL_PX + 5)
      ctx.fillText('-INF', 2, (scopeRows - 1) * CELL_PX + 5)

      // ── Upper tier: braille trace + sub-pixel dither + phosphor ──
      const glitchIntensity = glitch ? glitch.step(dt) : 0
      let traceRows = buildTrace(resolvedSignal.samples, numCols, scopeRows)
      const ditherRows = buildDither(resolvedSignal.samples, numCols, scopeRows)
      if (glitchIntensity > 0) {
        traceRows = applyGlitch(traceRows, glitchIntensity, random)
      }
      if (!active) {
        phosphor.length = 0
      } else {
        phosphor.push(traceRows)
        while (phosphor.length > PHOSPHOR_ALPHAS.length) phosphor.shift()
      }

      ctx.font = FONT_BRAILLE
      phosphor.slice(0, -1).forEach((rows, i) => {
        ctx.globalAlpha = PHOSPHOR_ALPHAS[i]
        ctx.fillStyle = active ? TRACE_FG : TRACE_IDLE
        rows.forEach((row, r) => ctx.fillText(row, 0, r * CELL_PX))
      })
      ctx.globalAlpha = 1

      ctx.fillStyle = DITHER_FG
      ditherRows.forEach((row, r) => ctx.fillText(row, 0, r * CELL_PX))

      ctx.fillStyle = active ? TRACE_FG : TRACE_IDLE
      traceRows.forEach((row, r) => ctx.fillText(row, 0, r * CELL_PX))

      // ── Divider: waterfall frequency axis ──
      if (dividerCache.cols !== numCols) {
        dividerCache = { cols: numCols, text: buildDivider(numCols) }
      }
      ctx.font = FONT_VGA
      ctx.fillStyle = GRAT_FG
      ctx.fillText(dividerCache.text, 0, scopeRows * CELL_PX)

      // ── Lower tier: scrolling waterfall spectrogram ──
      if (active && animate) {
        if (waterfall.depth !== fallsRows) waterfall = createWaterfallHistory(fallsRows)
        waterfall.push(analyzeBands(resolvedSignal.samples, numCols - 2))
      }
      const lines = waterfall.lines()
      let display = lines
      if (glitchIntensity > 0) {
        display = applyGlitch(lines, glitchIntensity, random)
      }
      ctx.fillStyle = WATERFALL_FG
      display.forEach((line, i) => {
        const age = display.length - 1 - i // 0 = newest
        ctx.globalAlpha = Math.max(0.15, 1 - (age / fallsRows) * 0.85)
        ctx.fillText(
          line,
          cellW,
          (scopeRows + 1 + (fallsRows - display.length) + i) * CELL_PX,
        )
      })
      ctx.globalAlpha = 1
    }

    let lastFrameTime = performance.now()

    if (motionQuery?.matches) {
      // Static clean frame: one settled step, no glitch, no loop.
      resolvedSignal.step(active)
      glitch?.step(1000)
      drawFrame(false)
    } else {
      const loop = () => {
        rafId = requestAnimationFrame(loop)
        resolvedSignal.step(active)
        drawFrame(true)
      }
      rafId = requestAnimationFrame(loop)
    }

    const onMotionChange = () => {
      // Re-run the whole effect on preference flips.
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
  }, [resolvedSignal, glitch, random, active, nonce])

  return (
    <div
      data-testid="fft-visualizer"
      data-active={active}
      data-mode="dual"
      data-static={staticFrame}
      className="relative w-full h-[240px] shrink-0 bg-bg overflow-hidden border-b border-grid-rule"
    >
      <canvas
        ref={canvasRef}
        className="absolute inset-0 w-full h-full"
        aria-hidden="true"
      />
    </div>
  )
}
