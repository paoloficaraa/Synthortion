import { vi } from 'vitest'

export interface CanvasFillOp {
  /** fillStyle active when the fillRect was issued */
  style: string
  /** fillRect(x, y, width, height) arguments */
  args: number[]
}

export interface CanvasTextOp {
  /** fillStyle active when fillText was issued */
  style: string
  /** fillText(text, x, y) arguments */
  text: string
  x: number
  y: number
}

/**
 * Builds a fake 2D canvas context for jsdom, which has no real canvas backing.
 *
 * GainMeter's draw loop only touches `fillStyle` and `fillRect`; the
 * spectrum visualizer (SpectrumVisualizer) additionally draws text via `fillText`,
 * measures glyph advance for column layout, and reads `font`/`textBaseline`.
 * Pass `ops` to record every fillRect together with the fillStyle active at
 * call time, or `textOps` to record every fillText — both optional for a
 * recording-free stub used only to exercise the draw path.
 *
 * `measureText` returns a fixed advance of 8px (the Px437 8x16 advance at
 * 16px font-size) so `computeCols` yields a stable, jsdom-independent column
 * count.
 */
export function createMockCanvasContext(options?: {
  ops?: CanvasFillOp[]
  textOps?: CanvasTextOp[]
}): CanvasRenderingContext2D {
  let fillStyle = ''
  let font = ''
  const ops = options?.ops
  const textOps = options?.textOps
  return {
    get fillStyle() {
      return fillStyle
    },
    set fillStyle(value: string) {
      fillStyle = value
    },
    get font() {
      return font
    },
    set font(value: string) {
      font = value
    },
    textBaseline: '',
    strokeStyle: '',
    lineWidth: 1,
    lineCap: 'round',
    fillRect: vi.fn((...args: number[]) => {
      if (ops) ops.push({ style: fillStyle, args })
    }),
    fillText: vi.fn((text: string, x: number, y: number) => {
      if (textOps) textOps.push({ style: fillStyle, text, x, y })
    }),
    measureText: vi.fn((text: string) => ({
      width: text ? 8 : 0,
    })),
    scale: vi.fn(),
    setTransform: vi.fn(),
    beginPath: vi.fn(),
    moveTo: vi.fn(),
    lineTo: vi.fn(),
    stroke: vi.fn(),
  } as unknown as CanvasRenderingContext2D
}
