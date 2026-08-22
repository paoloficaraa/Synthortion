/**
 * Track Glitch — velocity-driven physical interaction feedback for
 * the horizontal ASCII slider track.
 *
 * Fast pointer dragging triggers corruption glyphs along the track that decay
 * exponentially over ~120–150 ms back to clean dither state. Slow dragging
 * and fine adjustment (Shift key) produce zero track glitching.
 */

/** Specified corruption glyphs along the slider track. */
export const CORRUPTION_GLYPHS: readonly string[] = [
  '░',
  '▒',
  '▓',
  '╱',
  '╲',
  '╳',
  '▲',
  '::',
  '~',
  '+',
  '*',
] as const

/** Velocity threshold (px/ms) above which fast dragging triggers track glitch. */
export const FAST_DRAG_VELOCITY_THRESHOLD = 0.4

/** Exponential decay time constant for track corruption in milliseconds (~120-150ms). */
export const GLITCH_DECAY_TAU_MS = 135

/** Snaps glitch intensity to 0 below this threshold. */
const MIN_GLITCH_THRESHOLD = 0.05

export interface DragVelocityInput {
  currentY: number
  lastY: number
  currentTime: number
  lastTime: number
  shiftKey?: boolean
}

/**
 * Calculates pointer drag velocity in |dy| / dt (px/ms).
 * Returns 0 if shiftKey is active or if elapsed time is zero/negative.
 */
export function calculateDragVelocity({
  currentY,
  lastY,
  currentTime,
  lastTime,
  shiftKey = false,
}: DragVelocityInput): number {
  if (shiftKey) return 0
  const dt = currentTime - lastTime
  if (dt <= 0) return 0
  const dy = Math.abs(currentY - lastY)
  return dy / dt
}

export interface TrackCell {
  char: string
  filled: boolean
}

/**
 * Corrupts track cells intermittently when intensity > 0.
 * When intensity is 0, cells remain 100% clean and untouched.
 */
export function corruptTrackCells(
  cells: TrackCell[],
  intensity: number,
  random: () => number = Math.random
): TrackCell[] {
  if (intensity <= 0) return cells

  return cells.map((cell) => {
    // Intermittent digital corruption along the slider track
    if (random() < intensity * 0.6) {
      const glyph =
        CORRUPTION_GLYPHS[Math.floor(random() * CORRUPTION_GLYPHS.length)]
      return {
        char: glyph,
        filled: cell.filled,
      }
    }
    return cell
  })
}

export interface TrackGlitchController {
  readonly intensity: number
  onPointerDown(input: { startY: number; startTime: number }): void
  onPointerMove(input: {
    currentY: number
    currentTime: number
    shiftKey?: boolean
  }): number
  step(dtMs: number): number
  reset(): void
}

/**
 * Creates a track glitch controller that tracks pointer velocity and decays
 * corruption intensity exponentially toward 0.
 */
export function createTrackGlitchController(): TrackGlitchController {
  let _intensity = 0
  let _lastY = 0
  let _lastTime = 0

  return {
    get intensity() {
      return _intensity
    },

    onPointerDown({ startY, startTime }) {
      _lastY = startY
      _lastTime = startTime
    },

    onPointerMove({ currentY, currentTime, shiftKey = false }) {
      const v = calculateDragVelocity({
        currentY,
        lastY: _lastY,
        currentTime,
        lastTime: _lastTime,
        shiftKey,
      })
      _lastY = currentY
      _lastTime = currentTime

      if (v >= FAST_DRAG_VELOCITY_THRESHOLD) {
        _intensity = Math.min(1.0, Math.max(0.5, v / 1.5))
      }
      return _intensity
    },

    step(dtMs) {
      if (_intensity <= 0) return 0
      _intensity *= Math.exp(-dtMs / GLITCH_DECAY_TAU_MS)
      if (_intensity < MIN_GLITCH_THRESHOLD) {
        _intensity = 0
      }
      return _intensity
    },

    reset() {
      _intensity = 0
      _lastY = 0
      _lastTime = 0
    },
  }
}
