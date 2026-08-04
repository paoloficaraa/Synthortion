import '@testing-library/jest-dom'

/**
 * jsdom does not implement a real 2D rendering context, so any component that
 * calls `canvas.getContext('2d')` would log "Not implemented" and bail. Stub
 * the context with a Proxy that absorbs every method call and property write,
 * letting the oscilloscope's draw path run (harmlessly) in tests without the
 * canvas package.
 */
function createCanvas2DContext(): CanvasRenderingContext2D {
  const noop = (): void => undefined
  return new Proxy({} as Record<string | symbol, unknown>, {
    get(target, prop) {
      if (prop in target) return target[prop]
      return noop
    },
    set(target, prop, value) {
      target[prop] = value
      return true
    },
  }) as unknown as CanvasRenderingContext2D
}

HTMLCanvasElement.prototype.getContext = ((contextId: string) => {
  if (contextId === '2d') {
    return createCanvas2DContext() as unknown as CanvasRenderingContext2D
  }
  return null
}) as typeof HTMLCanvasElement.prototype.getContext

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
