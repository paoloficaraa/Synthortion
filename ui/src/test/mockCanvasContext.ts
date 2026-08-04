import { vi } from 'vitest'

export interface CanvasFillOp {
  /** fillStyle active when the fillRect was issued */
  style: string
  /** fillRect(x, y, width, height) arguments */
  args: number[]
}

/**
 * Builds a fake 2D canvas context for jsdom, which has no real canvas backing.
 *
 * GainMeter's draw loop only touches `fillStyle` and `fillRect`; the 2D
 * oscilloscope trace touches `strokeStyle`, `beginPath`/`moveTo`/`lineTo`/`stroke`
 * and shadow props. Pass an `ops` array to record every fillRect together with
 * the fillStyle active at call time so tests can assert on the rendered
 * segments; omit it for a recording-free stub used only to exercise the draw
 * path.
 *
 * Any other canvas method or property access falls back to a no-op, so every
 * draw loop in the app can run headless in jsdom without crashing.
 */
export function createMockCanvasContext(ops?: CanvasFillOp[]): CanvasRenderingContext2D {
  let fillStyle = ''
  const backing: Record<string, unknown> = {}
  const target = {
    get fillStyle() {
      return fillStyle
    },
    set fillStyle(value: string) {
      fillStyle = value
    },
    fillRect: vi.fn((...args: number[]) => {
      if (ops) ops.push({ style: fillStyle, args })
    }),
  }

  return new Proxy(target, {
    get(obj, prop) {
      if (prop in obj) return Reflect.get(obj, prop)
      if (prop in backing) return backing[String(prop)]
      // Unknown method → no-op so draw loops don't crash in jsdom.
      return () => {}
    },
    set(obj, prop, value) {
      // Record property writes (strokeStyle, lineWidth, shadowBlur, …) so
      // reads round-trip instead of returning a no-op function.
      backing[String(prop)] = value
      return Reflect.set(obj, prop, value)
    },
  }) as unknown as CanvasRenderingContext2D
}
