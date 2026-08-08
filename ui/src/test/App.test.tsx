import { render, screen, fireEvent, act } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import App from '../App'
import { createMockCanvasContext } from './mockCanvasContext'
import { createMockDspBridge } from '../lib/dspBridge'
import { initialState, diffPluginState } from '../lib/pluginState'
import { createGlitchPulser, type GlitchPulser } from '../lib/glitchPulser'

/**
 * The App owns a glitch pulser created on first render. Replace the factory
 * with a recording mock so the pulse-on-parameter-change contract is testable
 * without observing the internal ref.
 */
const createGlitchPulserMock = vi.mocked(createGlitchPulser)
vi.mock('../lib/glitchPulser', () => ({
  createGlitchPulser: vi.fn(() => ({
    pulse: vi.fn(),
    step: vi.fn(() => 0),
    get intensity() {
      return 0
    },
  })),
}))

/** The pulser instance App created on first render. */
function appPulser(): GlitchPulser & { pulse: ReturnType<typeof vi.fn> } {
  const instance = createGlitchPulserMock.mock.results[0]?.value
  return instance as GlitchPulser & { pulse: ReturnType<typeof vi.fn> }
}

/**
 * The App hosts two live GainMeters. jsdom has no 2D canvas context, so stub
 * it with a recording-free context to exercise the draw path without console
 * noise. Fake timers keep the meter animation loop quiescent between frames.
 */
describe('App', () => {
  beforeEach(() => {
    vi.useFakeTimers()
    createGlitchPulserMock.mockClear()
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

    expect(leftRail).toHaveClass('border-elev-6')
    expect(rightRail).toHaveClass('border-elev-6')
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

  it('renders the Matrix Faceplate control rows (DRV, BCR, DLY, CHR)', () => {
    render(<App />)

    expect(screen.getByText('DRV')).toBeInTheDocument()
    expect(screen.getByText('BCR')).toBeInTheDocument()
    expect(screen.getByText('DLY')).toBeInTheDocument()
    expect(screen.getByText('CHR')).toBeInTheDocument()

    expect(screen.getByRole('slider', { name: 'Drive' })).toBeInTheDocument()
    expect(screen.getByRole('slider', { name: 'Bitcrush' })).toBeInTheDocument()
    expect(screen.getByRole('slider', { name: 'Mix' })).toBeInTheDocument()
    expect(screen.getByRole('slider', { name: 'Time' })).toBeInTheDocument()
    expect(screen.getByRole('slider', { name: 'Fbk' })).toBeInTheDocument()
    expect(screen.getByRole('slider', { name: 'Chorus' })).toBeInTheDocument()
  })

  it('renders the faceplate controls at the prototype default state', () => {
    render(<App />)

    expect(screen.getByRole('slider', { name: 'Drive' })).toHaveAttribute(
      'aria-valuenow',
      '40'
    )
    expect(screen.getByRole('slider', { name: 'Bitcrush' })).toHaveAttribute(
      'aria-valuenow',
      '12'
    )
    expect(screen.getByRole('slider', { name: 'Mix' })).toHaveAttribute(
      'aria-valuenow',
      '30'
    )
    expect(screen.getByRole('slider', { name: 'Time' })).toHaveAttribute(
      'aria-valuenow',
      '250'
    )
    expect(screen.getByRole('slider', { name: 'Fbk' })).toHaveAttribute(
      'aria-valuenow',
      '50'
    )
    expect(screen.getByRole('slider', { name: 'Chorus' })).toHaveAttribute(
      'aria-valuenow',
      '75'
    )
  })

  it('does not notify the DSP bridge on initial mount', () => {
    const bridge = createMockDspBridge()
    render(<App dspBridge={bridge} />)

    expect(bridge.calls).toEqual([])
  })

  it('covers all four module power flags at the App bridge boundary', () => {
    const bridge = createMockDspBridge()
    render(<App dspBridge={bridge} />)

    // The power flags are part of the boundary state; on mount they default
    // on and must stay silent (zero bridge calls preserved).
    expect(bridge.calls).toEqual([])

    // Flipping any power flag notifies the bridge as { parameterId, value },
    // exactly like every other parameter — the App boundary diff owns this
    // contract for all four modules.
    const flags = ['driveOn', 'bitcrushOn', 'delayOn', 'chorusOn'] as const
    for (const flag of flags) {
      const calls = diffPluginState(initialState, { ...initialState, [flag]: false })
      expect(calls).toContainEqual({ parameterId: flag, value: false })
    }
  })

  it('mutates App state and notifies the DSP bridge on drive knob drag', () => {
    const bridge = createMockDspBridge()
    render(<App dspBridge={bridge} />)

    const drive = screen.getByRole('slider', { name: 'Drive' })
    // dy = 200 - 180 = 20 → value = 40 + 20 * 0.5 * (100/100) = 50
    fireEvent.pointerDown(drive, { clientY: 200, pointerId: 1 })
    fireEvent.pointerMove(drive, { clientY: 180, pointerId: 1 })
    fireEvent.pointerUp(drive, { pointerId: 1 })

    expect(drive).toHaveAttribute('aria-valuenow', '50')
    expect(drive).toHaveAttribute('aria-valuetext', '50%')
    expect(bridge.calls).toContainEqual({ parameterId: 'drive', value: 50 })
  })

  it('forwards the bitcrush knob drag to the DSP bridge', () => {
    const bridge = createMockDspBridge()
    render(<App dspBridge={bridge} />)

    const bitcrush = screen.getByRole('slider', { name: 'Bitcrush' })
    // min 2, max 24; dy = 20 → value = 12 + 20 * 0.5 * (22/100) = 14.2 → "14B"
    fireEvent.pointerDown(bitcrush, { clientY: 200, pointerId: 1 })
    fireEvent.pointerMove(bitcrush, { clientY: 180, pointerId: 1 })
    fireEvent.pointerUp(bitcrush, { pointerId: 1 })

    expect(bitcrush).toHaveAttribute('aria-valuetext', '14B')
    expect(bridge.calls).toContainEqual({ parameterId: 'bitcrush', value: 14.2 })
  })

  it('forwards a drive route toggle flip to the DSP bridge', () => {
    const bridge = createMockDspBridge()
    render(<App dspBridge={bridge} />)

    const post = screen.getByRole('button', { name: 'POST' })
    expect(post).toHaveAttribute('aria-pressed', 'false')

    fireEvent.click(post)

    expect(post).toHaveAttribute('aria-pressed', 'true')
    expect(bridge.calls).toContainEqual({ parameterId: 'driveRoute', value: 'POST' })
  })

  it('forwards the engine bypass toggle to the DSP bridge', () => {
    const bridge = createMockDspBridge()
    render(<App dspBridge={bridge} />)

    const bypass = screen.getByRole('button', { name: 'Disable main DSP' })
    expect(bypass).toHaveAttribute('aria-pressed', 'true')

    fireEvent.click(bypass)

    expect(bypass).toHaveAttribute('aria-pressed', 'false')
    expect(bridge.calls).toContainEqual({ parameterId: 'engineActive', value: false })
  })

  it('renders the FFT visualizer above the faceplate, toggling with the engine', () => {
    const { container } = render(<App />)

    const viz = screen.getByTestId('fft-visualizer')
    expect(viz).toHaveAttribute('data-active', 'true')
    // The braille scope is a canvas-backed band; the graticule labels live on
    // the canvas (buildGraticule), not in DOM text.
    expect(container.querySelector('canvas')).toBeInTheDocument()

    const bypass = screen.getByRole('button', { name: 'Disable main DSP' })
    fireEvent.click(bypass)

    expect(screen.getByTestId('fft-visualizer')).toHaveAttribute(
      'data-active',
      'false'
    )
  })

  it('pulses the glitch pulser on a knob drag, proportional to the delta', () => {
    render(<App />)
    const pulser = appPulser()

    const drive = screen.getByRole('slider', { name: 'Drive' })
    // dy = 200 - 180 = 20 → value = 40 + 20 * 0.5 * (100/100) = 50
    fireEvent.pointerDown(drive, { clientY: 200, pointerId: 1 })
    fireEvent.pointerMove(drive, { clientY: 180, pointerId: 1 })
    fireEvent.pointerUp(drive, { pointerId: 1 })

    // |50 - 40| / 100 = 0.1
    expect(pulser.pulse).toHaveBeenCalledWith(0.1)
  })

  it('pulses the glitch pulser at full intensity on a non-numeric change', () => {
    render(<App />)
    const pulser = appPulser()
    pulser.pulse.mockClear()

    const bypass = screen.getByRole('button', { name: 'Disable main DSP' })
    fireEvent.click(bypass)

    // engineActive flips false → boolean change → full-intensity burst
    expect(pulser.pulse).toHaveBeenCalledWith(1)
  })

  it('forwards the chorus WIDE toggle to the DSP bridge', () => {
    const bridge = createMockDspBridge()
    render(<App dspBridge={bridge} />)

    const wide = screen.getByRole('button', { name: 'WIDE' })
    expect(wide).toHaveAttribute('aria-pressed', 'false')

    fireEvent.click(wide)

    expect(wide).toHaveAttribute('aria-pressed', 'true')
    expect(bridge.calls).toContainEqual({ parameterId: 'chorusWide', value: true })
  })

  it('renders the T06 header chrome: brand, preset LCD and SAVE/LOAD', () => {
    render(<App />)

    expect(screen.getByRole('heading', { name: 'SYNTHORTION' })).toBeInTheDocument()
    expect(screen.getByText('INIT_STATE_01')).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'SAVE' })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'LOAD' })).toBeInTheDocument()
  })

  it('mounts four box-drawing corner brackets on the chassis', () => {
    const { container } = render(<App />)

    expect(container.querySelectorAll('[data-testid="corner-bracket"]')).toHaveLength(
      4
    )
  })

  it('plays the SYSTEM_BOOT overlay on mount and auto-dismisses it', () => {
    render(<App />)

    expect(
      screen.getByTestId('system-boot-overlay')
    ).toBeInTheDocument()

    // Boot sequence auto-dismisses after its fixed pause.
    act(() => {
      vi.advanceTimersByTime(2000)
    })

    expect(
      screen.queryByTestId('system-boot-overlay')
    ).not.toBeInTheDocument()
  })
})
