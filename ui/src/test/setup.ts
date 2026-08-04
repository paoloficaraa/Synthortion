import '@testing-library/jest-dom'

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
