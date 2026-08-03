import { render, screen, fireEvent } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import App from '../App'
import { createMockCanvasContext } from './mockCanvasContext'

/**
 * The App hosts two live GainMeters. jsdom has no 2D canvas context, so stub
 * it with a recording-free context to exercise the draw path without console
 * noise. Fake timers keep the meter animation loop quiescent between frames.
 */
describe('App', () => {
  beforeEach(() => {
    vi.useFakeTimers()
    vi.spyOn(HTMLCanvasElement.prototype, 'getContext').mockReturnValue(
      createMockCanvasContext()
    )
  })

  afterEach(() => {
    vi.useRealTimers()
    vi.restoreAllMocks()
  })

  it('renders IN and OUT meter columns with a TRIM knob in each', () => {
    render(<App />)

    expect(screen.getByText('IN')).toBeInTheDocument()
    expect(screen.getByText('OUT')).toBeInTheDocument()

    const trims = screen.getAllByRole('slider', { name: 'TRIM' })
    expect(trims).toHaveLength(2)
  })

  it('places IN/OUT meters in full-height side-bordered rails', () => {
    const { container } = render(<App />)

    const leftRail = container.querySelector('.border-r')
    const rightRail = container.querySelector('.border-l')

    expect(leftRail).toHaveClass('border-[#222]')
    expect(rightRail).toHaveClass('border-[#222]')
    expect(leftRail).toContainElement(screen.getByText('IN'))
    expect(rightRail).toContainElement(screen.getByText('OUT'))
  })

  it('binds both TRIM knobs to the -24..+24 dB range', () => {
    render(<App />)

    const [inTrim, outTrim] = screen.getAllByRole('slider', { name: 'TRIM' })
    expect(inTrim).toHaveAttribute('aria-valuemin', '-24')
    expect(inTrim).toHaveAttribute('aria-valuemax', '24')
    expect(outTrim).toHaveAttribute('aria-valuemin', '-24')
    expect(outTrim).toHaveAttribute('aria-valuemax', '24')
  })

  it('starts both TRIM knobs at 0 dB', () => {
    render(<App />)

    const [inTrim, outTrim] = screen.getAllByRole('slider', { name: 'TRIM' })
    expect(inTrim).toHaveAttribute('aria-valuenow', '0')
    expect(inTrim).toHaveAttribute('aria-valuetext', '0')
    expect(outTrim).toHaveAttribute('aria-valuenow', '0')
    expect(outTrim).toHaveAttribute('aria-valuetext', '0')
  })

  it('formats positive trim with a leading +n dB string', () => {
    render(<App />)

    const [inTrim] = screen.getAllByRole('slider', { name: 'TRIM' })
    // dy = 100 - 80 = 20 → value = 0 + 20 * 0.5 * (48/100) = 4.8 → "+5"
    fireEvent.pointerDown(inTrim, { clientY: 100, pointerId: 1 })
    fireEvent.pointerMove(inTrim, { clientY: 80, pointerId: 1 })
    fireEvent.pointerUp(inTrim, { pointerId: 1 })

    expect(screen.getByText('+5')).toBeInTheDocument()
    expect(inTrim).toHaveAttribute('aria-valuetext', '+5')
  })

  it('formats negative trim as a -n dB string', () => {
    render(<App />)

    const [inTrim] = screen.getAllByRole('slider', { name: 'TRIM' })
    // dy = 100 - 120 = -20 → value = -4.8 → "-5"
    fireEvent.pointerDown(inTrim, { clientY: 100, pointerId: 1 })
    fireEvent.pointerMove(inTrim, { clientY: 120, pointerId: 1 })
    fireEvent.pointerUp(inTrim, { pointerId: 1 })

    expect(screen.getByText('-5')).toBeInTheDocument()
    expect(inTrim).toHaveAttribute('aria-valuetext', '-5')
  })

  it('adjusts the output trim knob independently of the input', () => {
    render(<App />)

    const [inTrim, outTrim] = screen.getAllByRole('slider', { name: 'TRIM' })
    // dy = 200 - 160 = 40 → value = 9.6 → "+10"
    fireEvent.pointerDown(outTrim, { clientY: 200, pointerId: 1 })
    fireEvent.pointerMove(outTrim, { clientY: 160, pointerId: 1 })
    fireEvent.pointerUp(outTrim, { pointerId: 1 })

    expect(outTrim).toHaveAttribute('aria-valuetext', '+10')
    expect(screen.getByText('+10')).toBeInTheDocument()
    expect(inTrim).toHaveAttribute('aria-valuetext', '0')
  })
})
