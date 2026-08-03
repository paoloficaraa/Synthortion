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
 * GainMeter's draw loop only touches `fillStyle` and `fillRect`. Pass an `ops`
 * array to record every fillRect together with the fillStyle active at call
 * time so tests can assert on the rendered segments; omit it for a
 * recording-free stub used only to exercise the draw path.
 */
export function createMockCanvasContext(ops?: CanvasFillOp[]): CanvasRenderingContext2D {
  let fillStyle = ''
  return {
    get fillStyle() {
      return fillStyle
    },
    set fillStyle(value: string) {
      fillStyle = value
    },
    fillRect: vi.fn((...args: number[]) => {
      if (ops) ops.push({ style: fillStyle, args })
    }),
  } as unknown as CanvasRenderingContext2D
}
