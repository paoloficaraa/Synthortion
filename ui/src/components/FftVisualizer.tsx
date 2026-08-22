import { useEffect, useRef, useState } from 'react'
import {
  createOscilloscopeSignal,
  type OscilloscopeSignal,
} from '../lib/oscilloscopeSignal'
import { type GlitchPulser, applyGlitch } from '../lib/glitchPulser'
import {
  buildGraticule,
  buildTraceAndDither,
  CELL_PX,
} from '../lib/fftBraille'
import {
  analyzeBands,
  createWaterfallHistory,
  MIN_HZ,
  MAX_HZ,
} from '../lib/spectrogram'

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

/** Share of the band height given to the waveform scope tier. */
const SCOPE_SHARE = 0.6

/** Phosphor persistence: alpha per retained trace generation (oldest first). */
const PHOSPHOR_ALPHAS = [0.15, 0.3]

/** Frequency calibration ticks on the waterfall axis. */
const FREQ_TICKS = [
  { hz: MIN_HZ, label: '20Hz' },
  { hz: 200, label: '200Hz' },
  { hz: 2000, label: '2kHz' },
  { hz: MAX_HZ, label: '20kHz' },
] as const

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

    /** Log-frequency x position (cells) between MIN_HZ and MAX_HZ. */
    const freqCol = (hz: number, cols: number) =>
      Math.round((Math.log(hz / MIN_HZ) / Math.log(MAX_HZ / MIN_HZ)) * (cols - 1))

    /** Divider row between tiers: rules, '+' ticks, freq labels without collision. */
    const buildDivider = (cols: number): string => {
      const cells = new Array<string>(cols).fill('─')
      cells[0] = '├'
      cells[cols - 1] = '┤'

      for (let i = 0; i < FREQ_TICKS.length; i++) {
        const { hz, label } = FREQ_TICKS[i]
        const tick = 1 + freqCol(hz, cols - 2)

        if (i === FREQ_TICKS.length - 1) {
          // Rightmost tick (20kHz): place label to the left so '+' stays visible at the boundary.
          const start = Math.max(1, tick - label.length)
          for (let j = 0; j < label.length; j++) cells[start + j] = label[j]
          cells[tick] = '+'
        } else {
          cells[tick] = '+'
          const start = tick + 1
          if (start + label.length < cols - 1) {
            for (let j = 0; j < label.length; j++) cells[start + j] = label[j]
          }
        }
      }
      return cells.join('')
    }

    let dividerCache = { cols: 0, text: '' }
    let lastFrameTime = performance.now()

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
      const waterfallRows = Math.max(2, totalRows - scopeRows - 1)

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

      // ── Upper tier: braille trace + sub-pixel dither + phosphor ──
      const glitchIntensity = glitch ? glitch.step(dt) : 0
      const generated = buildTraceAndDither(resolvedSignal.samples, numCols, scopeRows)
      let traceRows = generated.trace
      let ditherRows = generated.dither

      if (glitchIntensity > 0) {
        traceRows = applyGlitch(traceRows, glitchIntensity, random)
        ditherRows = applyGlitch(ditherRows, glitchIntensity, random)
      }

      if (!active) {
        phosphor.length = 0
      } else {
        phosphor.push(traceRows)
        // Keep active frame plus historical frames for each configured decay alpha.
        while (phosphor.length > PHOSPHOR_ALPHAS.length + 1) phosphor.shift()
      }

      ctx.font = FONT_BRAILLE
      // Render historical frames from oldest to newest with configured alpha decay.
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
        if (waterfall.depth !== waterfallRows) waterfall = createWaterfallHistory(waterfallRows)
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
        ctx.globalAlpha = Math.max(0.15, 1 - (age / waterfallRows) * 0.85)
        ctx.fillText(
          line,
          cellW,
          (scopeRows + 1 + (waterfallRows - display.length) + i) * CELL_PX,
        )
      })
      ctx.globalAlpha = 1
    }

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
      className="relative w-full basis-[35%] flex-grow min-h-0 bg-bg border-b border-grid-rule"
    >
      <div className="absolute inset-0 overflow-hidden">
        <canvas ref={canvasRef} className="absolute inset-0 w-full h-full" aria-hidden="true" />
      </div>
      {/* Cartesian bottom corners — + centered on shared hairline where scope band meets flanking rails */}
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
