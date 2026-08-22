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

