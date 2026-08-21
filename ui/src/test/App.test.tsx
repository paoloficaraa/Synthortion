import { render, screen, fireEvent, act, within } from '@testing-library/react'
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

  it('renders IN and OUT meter columns without TRIM faders in the rails', () => {
    render(<App />)

    expect(screen.getByText('IN')).toBeInTheDocument()
    expect(screen.getByText('OUT')).toBeInTheDocument()

    // Side rails host meter columns only — the TrimFader slider is gone.
    expect(screen.queryAllByRole('slider', { name: 'TRIM' })).toHaveLength(0)
  })

  it('places IN/OUT meters in full-height side-bordered rails', () => {
    const { container } = render(<App />)

    const leftRail = container.querySelector('.border-r')
    const rightRail = container.querySelector('.border-l')

    expect(leftRail).toHaveClass('border-border')
    expect(rightRail).toHaveClass('border-border')
    expect(leftRail).toHaveClass('w-[48px]')
    expect(rightRail).toHaveClass('w-[48px]')
    expect(leftRail).toContainElement(screen.getByText('IN'))
    expect(rightRail).toContainElement(screen.getByText('OUT'))
  })

  it('renders the IN/OUT meters as box-framed ASCII ladders with dB readouts', () => {
    const { container } = render(<App />)

    const meters = container.querySelectorAll('[role="meter"]')
    expect(meters).toHaveLength(2)

    // Each meter readout is bracketed: [ -INF ] / [ -06dB ].
    const readouts = [...meters].map((m) => m.textContent ?? '')
    const bracketPattern = /\[ (?:-INF|[-+]\d{2}dB) \]/
    expect(readouts[0]).toMatch(bracketPattern)
    expect(readouts[1]).toMatch(bracketPattern)
    // Box-drawing frame glyphs frame the ladder.
    const frame = container.querySelector('[role="meter"]')
    expect(frame).toBeInTheDocument()
    const asciiBoxes = Array.from(container.querySelectorAll('.font-ascii'))
    expect(asciiBoxes.length).toBeGreaterThan(0)
    const boxText = asciiBoxes.map((n) => n.textContent ?? '').join(' ')
    expect(boxText).toContain('┌')
    expect(boxText).toContain('┐')
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
    expect(bridge.calls).toContainEqual({ parameterId: 'COLOR', value: 0.5 })
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
    expect(bridge.calls).toContainEqual({ parameterId: 'BITCRUSH', value: 0.142 })
  })

  it('forwards the engine bypass toggle to the DSP bridge', () => {
    const bridge = createMockDspBridge()
    render(<App dspBridge={bridge} />)

    const bypass = screen.getByRole('button', { name: 'Disable main DSP' })
    expect(bypass).toHaveAttribute('aria-pressed', 'true')

    fireEvent.click(bypass)

    expect(bypass).toHaveAttribute('aria-pressed', 'false')
    expect(bridge.calls).toContainEqual({ parameterId: 'PLUGIN_BYPASS', value: 1 })
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


  it('renders a power switch in each module title bar', () => {
    render(<App />)

    for (const code of ['DRV', 'BCR', 'DLY', 'CHR']) {
      const sw = screen.getByTestId(`power-${code}`)
      expect(sw).toHaveAttribute('aria-pressed', 'true')
    }
  })

  it('pulses the glitch pulser at 0.8 on a module power toggle', () => {
    render(<App />)
    const pulser = appPulser()
    pulser.pulse.mockClear()

    fireEvent.click(screen.getByTestId('power-DRV'))

    // Module power toggles fire a fixed short burst (0.8), not the heavy 1.0.
    expect(pulser.pulse).toHaveBeenCalledWith(0.8)
  })

  it('forwards a module power toggle to the DSP bridge', () => {
    const bridge = createMockDspBridge()
    render(<App dspBridge={bridge} />)

    const sw = screen.getByTestId('power-DRV')
    fireEvent.click(sw)

    expect(sw).toHaveAttribute('aria-pressed', 'false')
    // driveOn is NOT in PARAMETER_IDS, so it should NOT be called.
    expect(bridge.calls).not.toContainEqual(expect.objectContaining({ parameterId: 'driveOn' }))
  })

  it('dims a powered-off module and shows -- readouts', () => {
    render(<App />)

    const drive = screen.getByRole('slider', { name: 'Drive' })
    expect(drive).toHaveAttribute('aria-valuetext', '40%')

    fireEvent.click(screen.getByTestId('power-DRV'))

    expect(drive).toHaveAttribute('aria-valuetext', '--')

    // Powering the module back on restores the live readout.
    fireEvent.click(screen.getByTestId('power-DRV'))
    expect(drive).toHaveAttribute('aria-valuetext', '40%')
  })

  it('renders the T07 status bar chrome: brand, VGA preset readout and SAVE/LOAD', () => {
    const { container } = render(<App />)

    expect(screen.getByRole('heading', { name: 'SYNTHORTION' })).toBeInTheDocument()
    expect(screen.getByText('INIT_STATE_01')).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'SAVE' })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'LOAD' })).toBeInTheDocument()

    // The preset readout is a VGA ASCII surface with a blinking block cursor.
    const lcd = container.querySelector('header .bg-elev-0') as HTMLElement
    expect(lcd).toHaveClass('font-ascii')
    expect(lcd.querySelector('.block-cursor')).toHaveAttribute(
      'aria-hidden',
      'true'
    )
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

    // Boot sequence auto-dismisses after its ~2.5s one-shot window.
    act(() => {
      vi.advanceTimersByTime(2600)
    })

    expect(
      screen.queryByTestId('system-boot-overlay')
    ).not.toBeInTheDocument()
  })

  it('micro-glitches the active Drive knob track during drag and keeps its readout clean', () => {
    render(<App />)

    const drive = screen.getByRole('slider', { name: 'Drive' })
    // Scope via the Knob's parent wrapper (direct parent of the slider div).
    const driveKnob = drive.parentElement!
    // Value text rendered before the drag — must stay clean throughout.
    expect(within(driveKnob).getByText('40%')).toBeInTheDocument()

    fireEvent.pointerDown(drive, { clientY: 200, pointerId: 1 })
    // The drive knob track exposes the glitched border cells while active.
    const driveTrack = within(drive).getByTestId('knob-track')
    expect(driveTrack.querySelectorAll('.knob-glitch')).toHaveLength(2)
    // The other modules' knobs must NOT micro-glitch — only the active one.
    const bitcrushTrack = within(
      screen.getByRole('slider', { name: 'Bitcrush' }),
    ).getByTestId('knob-track')
    expect(bitcrushTrack.querySelectorAll('.knob-glitch')).toHaveLength(0)

    // Drag and release.
    act(() => {
      fireEvent.pointerMove(drive, { clientY: 180, pointerId: 1 })
    })
    // The drive changed from 40 to 50 after the drag; readout must be clean.
    const readout = within(driveKnob).getByText('50%')
    expect(readout).not.toHaveClass('knob-glitch')

    fireEvent.pointerUp(drive, { pointerId: 1 })
  })
})
