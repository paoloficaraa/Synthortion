import '@testing-library/jest-dom'
import { vi } from 'vitest'

/**
 * jsdom's canvas 2D context is only a partial stub — the oscilloscope draw
 * loop would crash if it actually ran. Stub `requestAnimationFrame` to a
 * no-op so the canvas visual stays untested by design (the signal generator
 * that drives it is tested directly at the module seam).
 */
vi.stubGlobal('requestAnimationFrame', (): number => 0)
vi.stubGlobal('cancelAnimationFrame', (): void => {})

/**
 * jsdom has no `window.matchMedia`, which Framer Motion's `useReducedMotion`
 * and `MotionConfig reducedMotion="user"` rely on. Default to "no preference"
 * so motion features are enabled in tests unless a test mocks the query.
 */
if (!window.matchMedia) {
  window.matchMedia = (query: string): MediaQueryList =>
    ({
      matches: false,
      media: query,
      onchange: null,
      addListener: () => {},
      removeListener: () => {},
      addEventListener: () => {},
      removeEventListener: () => {},
      dispatchEvent: () => false,
    }) as unknown as MediaQueryList
}
