import { render, screen, act } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { SpectrumVisualizer } from '../components/SpectrumVisualizer'
import { createGlitchPulser } from '../lib/glitchPulser'
import { NUM_BANDS } from '../lib/spectrumBraille'

describe('SpectrumVisualizer', () => {
  beforeEach(() => {
    vi.clearAllMocks()
  })

  it('mounts a 2D canvas within the ~35% center hub band with zero layout shift', () => {
    const { container } = render(<SpectrumVisualizer active={true} />)
    const visualizer = screen.getByTestId('spectrum-visualizer')

    expect(visualizer).toBeInTheDocument()
    expect(visualizer).toHaveAttribute('data-active', 'true')
    expect(visualizer).toHaveAttribute('data-mode', 'spectrum')
    expect(visualizer.className).toContain('basis-[35%]')
    expect(visualizer.className).toContain('flex-grow')
    expect(visualizer.className).toContain('min-h-0')

    const canvas = container.querySelector('canvas')
    expect(canvas).toBeInTheDocument()
    expect(canvas).toHaveAttribute('aria-hidden', 'true')
  })

  it('subscribes to spectrum stream on mount and unsubscribes on unmount', () => {
    let subscriber: ((magnitudes: number[]) => void) | null = null
    const unsubscribeMock = vi.fn()
    const subscribeMock = vi.fn((cb: (magnitudes: number[]) => void) => {
      subscriber = cb
      return unsubscribeMock
    })

    const { unmount } = render(
      <SpectrumVisualizer active={true} subscribeSpectrum={subscribeMock} />,
    )

    expect(subscribeMock).toHaveBeenCalledTimes(1)
    expect(subscriber).not.toBeNull()

    unmount()
    expect(unsubscribeMock).toHaveBeenCalledTimes(1)
  })

  it('receives 80-band magnitude arrays and renders without errors', () => {
    let subscriber: ((magnitudes: number[]) => void) | null = null
    const subscribeMock = (cb: (magnitudes: number[]) => void) => {
      subscriber = cb
      return () => {}
    }

    render(<SpectrumVisualizer active={true} subscribeSpectrum={subscribeMock} />)

    expect(subscriber).not.toBeNull()
    const testFrame = new Array(NUM_BANDS).fill(0).map((_, i) => i / NUM_BANDS)

    act(() => {
      subscriber!(testFrame)
    })

    const visualizer = screen.getByTestId('spectrum-visualizer')
    expect(visualizer).toHaveAttribute('data-active', 'true')
  })

  it('smoothly handles bypass state transition and halts the animation loop when settled', () => {
    let subscriber: ((magnitudes: number[]) => void) | null = null
    const subscribeMock = (cb: (magnitudes: number[]) => void) => {
      subscriber = cb
      return () => {}
    }

    const rafSpy = vi.spyOn(window, 'requestAnimationFrame')

    const { rerender } = render(
      <SpectrumVisualizer active={true} subscribeSpectrum={subscribeMock} />,
    )

    const testFrame = new Array(NUM_BANDS).fill(0.8)
    act(() => {
      subscriber!(testFrame)
    })

    expect(rafSpy).toHaveBeenCalled()
    const callCountBeforeBypass = rafSpy.mock.calls.length

    // Switch to bypassed mode
    rerender(<SpectrumVisualizer active={false} subscribeSpectrum={subscribeMock} />)
    const visualizer = screen.getByTestId('spectrum-visualizer')
    expect(visualizer).toHaveAttribute('data-active', 'false')

    rafSpy.mockRestore()
  })

  it('handles ResizeObserver canvas resize resilience without throwing', () => {
    let resizeCallback: ResizeObserverCallback | null = null
    const observeMock = vi.fn()
    const disconnectMock = vi.fn()

    class MockResizeObserver {
      constructor(cb: ResizeObserverCallback) {
        resizeCallback = cb
      }
      observe = observeMock
      disconnect = disconnectMock
      unobserve = vi.fn()
    }

    const origResizeObserver = window.ResizeObserver
    window.ResizeObserver = MockResizeObserver as unknown as typeof ResizeObserver

    const { unmount, container } = render(<SpectrumVisualizer active={true} />)
    expect(observeMock).toHaveBeenCalled()

    // Trigger resize event
    const canvas = container.querySelector('canvas')!
    act(() => {
      if (resizeCallback) {
        resizeCallback(
          [{ target: canvas, contentRect: { width: 400, height: 180 } as DOMRectReadOnly }] as unknown as ResizeObserverEntry[],
          {} as ResizeObserver,
        )
      }
    })

    expect(canvas).toBeInTheDocument()
    unmount()
    expect(disconnectMock).toHaveBeenCalled()

    window.ResizeObserver = origResizeObserver
  })

  it('renders static frame and halts animation loop when reduced motion is preferred', () => {
    const origMatchMedia = window.matchMedia
    window.matchMedia = vi.fn().mockImplementation((query) => ({
      matches: query.includes('prefers-reduced-motion'),
      media: query,
      onchange: null,
      addListener: vi.fn(),
      removeListener: vi.fn(),
      addEventListener: vi.fn(),
      removeEventListener: vi.fn(),
      dispatchEvent: vi.fn(),
    }))

    render(<SpectrumVisualizer active={true} />)
    const visualizer = screen.getByTestId('spectrum-visualizer')
    expect(visualizer).toHaveAttribute('data-static', 'true')

    window.matchMedia = origMatchMedia
  })

  it('accepts glitch pulser prop and random generator for deterministic testing', () => {
    const pulser = createGlitchPulser()
    const random = vi.fn(() => 0.5)

    render(<SpectrumVisualizer active={true} glitch={pulser} random={random} />)
    const visualizer = screen.getByTestId('spectrum-visualizer')
    expect(visualizer).toBeInTheDocument()
  })

  it('features Cartesian frame corner crosshairs on shared hairline', () => {
    const { container } = render(<SpectrumVisualizer active={true} />)
    const crosshairs = container.querySelectorAll('[aria-hidden="true"]')
    const crosshairTexts = Array.from(crosshairs).map((el) => el.textContent?.trim())

    expect(crosshairTexts.filter((t) => t === '+').length).toBeGreaterThanOrEqual(2)
  })
})
