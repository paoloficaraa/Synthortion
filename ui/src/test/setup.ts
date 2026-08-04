import { createElement } from 'react'
import '@testing-library/jest-dom'
import { vi } from 'vitest'

/**
 * R3F's <Canvas> needs a real WebGL context, which jsdom does not provide.
 *
 * Stub the reconciler and frame hook so FftVisualizer (and anything that
 * embeds it, e.g. App) can render its DOM chrome — frequency labels, canvas
 * mount, active state — without a GL context. The GL scene itself is untested
 * by design (see the spec's testing decisions); the signal generator that
 * drives it is tested directly at the module seam.
 */
vi.mock('@react-three/fiber', () => ({
  Canvas: () => createElement('canvas', { 'data-testid': 'r3f-canvas' }),
  useFrame: () => {},
}))

vi.mock('@react-three/drei', () => ({
  PerspectiveCamera: () => null,
}))

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
