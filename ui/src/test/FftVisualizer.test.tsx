import { render, screen } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import { FftVisualizer } from '../components/FftVisualizer'
import { createTimeSignal } from '../lib/timeSignal'

describe('FftVisualizer (2D Oscilloscope)', () => {
  it('preserves the prototype frequency labels', () => {
    render(<FftVisualizer active />)

    expect(screen.getByText('20Hz')).toBeInTheDocument()
    expect(screen.getByText('200Hz')).toBeInTheDocument()
    expect(screen.getByText('2kHz')).toBeInTheDocument()
    expect(screen.getByText('20kHz')).toBeInTheDocument()
  })

  it('mounts an HTML canvas (no WebGL)', () => {
    const { container } = render(<FftVisualizer active />)

    expect(container.querySelector('canvas')).toBeInTheDocument()
    expect(screen.getByTestId('fft-visualizer')).toHaveClass('bg-bg')
  })

  it('toggles with the engine active state', () => {
    const { container, rerender } = render(<FftVisualizer active />)
    const viz = () => container.querySelector('[data-testid="fft-visualizer"]')

    expect(viz()).toHaveAttribute('data-active', 'true')

    rerender(<FftVisualizer active={false} />)
    expect(viz()).toHaveAttribute('data-active', 'false')

    rerender(<FftVisualizer active />)
    expect(viz()).toHaveAttribute('data-active', 'true')
  })

  it('accepts an injected signal so tests can feed mock frames', () => {
    const signal = createTimeSignal(() => 0.5)
    const { container } = render(<FftVisualizer active signal={signal} />)

    expect(container.querySelector('canvas')).toBeInTheDocument()
  })

  it('keeps the oscilloscope band full-width above the faceplate', () => {
    const { container } = render(<FftVisualizer active />)

    const viz = container.querySelector('[data-testid="fft-visualizer"]')
    expect(viz).toHaveClass('w-full')
    expect(viz).toHaveClass('border-b')
  })
})
