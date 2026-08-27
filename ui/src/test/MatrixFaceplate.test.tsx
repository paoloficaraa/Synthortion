import { render, screen, fireEvent } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { MatrixFaceplate } from '../components/MatrixFaceplate'
import { initialState } from '../lib/pluginState'

/**
 * The faceplate is a purely presentational control surface: it receives the
 * whole plugin state as props and reports every interaction through
 * `onChange`. These tests pin that contract down (no component-owned state).
 */
describe('MatrixFaceplate', () => {
  it('renders the four control columns with their section labels', () => {
    render(<MatrixFaceplate state={initialState} onChange={() => {}} />)

    expect(screen.getByText('DRV')).toBeInTheDocument()
    expect(screen.getByText('BCR')).toBeInTheDocument()
    expect(screen.getByText('DLY')).toBeInTheDocument()
    expect(screen.getByText('CHR')).toBeInTheDocument()
  })

  it('is presentational: re-renders from props and reports via onChange', () => {
    const onChange = vi.fn()
    const { rerender } = render(
      <MatrixFaceplate state={initialState} onChange={onChange} />
    )

    expect(screen.getByRole('slider', { name: 'Drive' })).toHaveAttribute(
      'aria-valuenow',
      '40'
    )

    rerender(
      <MatrixFaceplate state={{ ...initialState, drive: 80 }} onChange={onChange} />
    )
    expect(screen.getByRole('slider', { name: 'Drive' })).toHaveAttribute(
      'aria-valuenow',
      '80'
    )
  })

  it('forwards a drive knob drag through onChange', () => {
    const onChange = vi.fn()
    render(<MatrixFaceplate state={initialState} onChange={onChange} />)

    const drive = screen.getByRole('slider', { name: 'Drive' })
    fireEvent.pointerDown(drive, { clientY: 200, pointerId: 1 })
    fireEvent.pointerMove(drive, { clientY: 180, pointerId: 1 })
    fireEvent.pointerUp(drive, { pointerId: 1 })

    expect(onChange).toHaveBeenCalledWith({ drive: 50 })
  })


  it('renders a power switch in each module title bar', () => {
    render(<MatrixFaceplate state={initialState} onChange={() => {}} />)

    for (const code of ['DRV', 'BCR', 'DLY', 'CHR']) {
      const sw = screen.getByTestId(`power-${code}`)
      expect(sw).toHaveAttribute('aria-pressed', 'true')
      expect(sw).toHaveAttribute(
        'aria-label',
        `Turn off ${code} module`
      )
    }
  })

  it('forwards a drive power toggle through onChange', () => {
    const onChange = vi.fn()
    render(<MatrixFaceplate state={initialState} onChange={onChange} />)

    fireEvent.click(screen.getByTestId('power-DRV'))

    expect(onChange).toHaveBeenCalledWith({ driveOn: false })
  })

  it('forwards each remaining power toggle through onChange', () => {
    const onChange = vi.fn()
    render(<MatrixFaceplate state={initialState} onChange={onChange} />)

    fireEvent.click(screen.getByTestId('power-BCR'))
    fireEvent.click(screen.getByTestId('power-DLY'))
    fireEvent.click(screen.getByTestId('power-CHR'))

    expect(onChange).toHaveBeenCalledWith({ bitcrushOn: false })
    expect(onChange).toHaveBeenCalledWith({ delayOn: false })
    expect(onChange).toHaveBeenCalledWith({ chorusOn: false })
  })

  it('shows a powered-off power switch as un-pressed with an "on" label', () => {
    render(
      <MatrixFaceplate
        state={{ ...initialState, driveOn: false }}
        onChange={() => {}}
      />
    )

    const sw = screen.getByTestId('power-DRV')
    expect(sw).toHaveAttribute('aria-pressed', 'false')
    expect(sw).toHaveAttribute('aria-label', 'Turn on DRV module')
  })

  it('renders -- readouts for a powered-off module', () => {
    render(
      <MatrixFaceplate
        state={{ ...initialState, driveOn: false, delayOn: false }}
        onChange={() => {}}
      />
    )

    expect(screen.getByRole('slider', { name: 'Drive' })).toHaveAttribute(
      'aria-valuetext',
      '--'
    )
    expect(screen.getByRole('slider', { name: 'Mix' })).toHaveAttribute(
      'aria-valuetext',
      '--'
    )
    expect(screen.getByRole('slider', { name: 'Time' })).toHaveAttribute(
      'aria-valuetext',
      '--'
    )
    expect(screen.getByRole('slider', { name: 'Fbk' })).toHaveAttribute(
      'aria-valuetext',
      '--'
    )

    // Powered-on modules keep their numeric readouts.
    expect(screen.getByRole('slider', { name: 'Bitcrush' })).toHaveAttribute(
      'aria-valuetext',
      '0%'
    )
    expect(screen.getByRole('slider', { name: 'Chorus' })).toHaveAttribute(
      'aria-valuetext',
      '75%'
    )
    expect(screen.getByRole('slider', { name: 'Width' })).toHaveAttribute(
      'aria-valuetext',
      '50%'
    )
  })

  it('dims a powered-off module section', () => {
    const { container } = render(
      <MatrixFaceplate
        state={{ ...initialState, driveOn: false }}
        onChange={() => {}}
      />
    )

    // The DRV section is the first grid cell; its controls wrapper carries the
    // dimmed + inert classes when the module is off.
    const sections = container.querySelectorAll('section')
    const drv = sections[0]
    const drvControls = drv.querySelector('div.opacity-30')
    expect(drvControls).not.toBeNull()
    expect(drvControls).toHaveClass('pointer-events-none')

    // A powered-on section does not carry the dimmed wrapper class.
    const chr = sections[3]
    expect(chr.querySelector('div.opacity-30')).toBeNull()
  })

  it('renders Cartesian crosshairs (+) and technical coordinate codes without naive character loops', () => {
    const { container } = render(
      <MatrixFaceplate state={initialState} onChange={() => {}} />
    )

    // No naive ASCII loop spans with repeated │ or ─
    const allText = container.textContent ?? ''
    expect(allText).not.toMatch(/│{2,}/)
    expect(allText).not.toMatch(/─{2,}/)

    // Cartesian coordinate crosshairs (+) are rendered
    const crosshairs = screen.getAllByText('+')
    expect(crosshairs.length).toBeGreaterThanOrEqual(4)

    // All decorative framing elements carry aria-hidden="true"
    const ariaHiddenElements = container.querySelectorAll('[aria-hidden="true"]')
    expect(ariaHiddenElements.length).toBeGreaterThanOrEqual(4)
  })

  it('allocates 2 columns to the DLY module in the 5-column grid', () => {
    const { container } = render(
      <MatrixFaceplate state={initialState} onChange={() => {}} />
    )

    const sections = container.querySelectorAll('section')
    expect(sections).toHaveLength(4)
    // DLY is the 3rd module (index 2) and spans 2 columns
    const dly = sections[2]
    expect(dly).toHaveClass('col-span-2')
  })

  it('expands all module frames to 100% vertical height without clipping', () => {
    const { container } = render(
      <MatrixFaceplate state={initialState} onChange={() => {}} />
    )

    const grid = container.firstChild as HTMLElement
    expect(grid).toHaveClass('h-full')

    const sections = container.querySelectorAll('section')
    expect(sections).toHaveLength(4)
    sections.forEach((section) => {
      expect(section).toHaveClass('h-full')
    })
  })

  it('renders DLY section with Mix knob and Time/Feedback sub-knobs with dual-row spacing', () => {
    render(<MatrixFaceplate state={initialState} onChange={() => {}} />)

    const mixSlider = screen.getByRole('slider', { name: 'Mix' })
    const timeSlider = screen.getByRole('slider', { name: 'Time' })
    const fbkSlider = screen.getByRole('slider', { name: 'Fbk' })

    expect(mixSlider).toBeInTheDocument()
    expect(timeSlider).toBeInTheDocument()
    expect(fbkSlider).toBeInTheDocument()
  })

  it('renders inline Cartesian [ SYNC ] / [ FREE ] toggle in DLY module and forwards mode changes', () => {
    const onChange = vi.fn()
    const { rerender } = render(
      <MatrixFaceplate state={initialState} onChange={onChange} />
    )

    const syncBtn = screen.getByRole('button', { name: 'SYNC' })
    const freeBtn = screen.getByRole('button', { name: 'FREE' })

    expect(syncBtn).toHaveAttribute('aria-pressed', 'true')
    expect(freeBtn).toHaveAttribute('aria-pressed', 'false')

    // Click FREE button -> switches delaySync to FREE and updates delayTime to ms default
    fireEvent.click(freeBtn)
    expect(onChange).toHaveBeenCalledWith({ delaySync: 'FREE', delayTime: 250 })

    // Rerender in FREE mode
    rerender(
      <MatrixFaceplate
        state={{ ...initialState, delaySync: 'FREE', delayTime: 250 }}
        onChange={onChange}
      />
    )
    expect(screen.getByRole('button', { name: 'FREE' })).toHaveAttribute(
      'aria-pressed',
      'true'
    )
    expect(screen.getByRole('button', { name: 'SYNC' })).toHaveAttribute(
      'aria-pressed',
      'false'
    )

    // Click SYNC button -> switches delaySync to SYNC and updates delayTime to grid subdivision index
    fireEvent.click(screen.getByRole('button', { name: 'SYNC' }))
    expect(onChange).toHaveBeenCalledWith({ delaySync: 'SYNC', delayTime: 6 })
  })

  it('renders dynamic fractional readouts in SYNC and milliseconds in FREE', () => {
    const { rerender } = render(
      <MatrixFaceplate
        state={{ ...initialState, delaySync: 'SYNC', delayTime: 6 }}
        onChange={() => {}}
      />
    )
    const timeSlider = screen.getByRole('slider', { name: 'Time' })
    expect(timeSlider).toHaveAttribute('aria-valuetext', '1/8D')

    // Different subdivision step (index 8 -> 1/4)
    rerender(
      <MatrixFaceplate
        state={{ ...initialState, delaySync: 'SYNC', delayTime: 8 }}
        onChange={() => {}}
      />
    )
    expect(screen.getByRole('slider', { name: 'Time' })).toHaveAttribute(
      'aria-valuetext',
      '1/4'
    )

    // FREE mode (250ms)
    rerender(
      <MatrixFaceplate
        state={{ ...initialState, delaySync: 'FREE', delayTime: 250 }}
        onChange={() => {}}
      />
    )
    expect(screen.getByRole('slider', { name: 'Time' })).toHaveAttribute(
      'aria-valuetext',
      '250ms'
    )
  })

  it('renders secondary Width knob in CHR module and forwards interaction through onChange', () => {
    const onChange = vi.fn()
    render(<MatrixFaceplate state={initialState} onChange={onChange} />)

    const widthSlider = screen.getByRole('slider', { name: 'Width' })
    expect(widthSlider).toBeInTheDocument()
    expect(widthSlider).toHaveAttribute('aria-valuenow', '50')
    expect(widthSlider).toHaveAttribute('aria-valuetext', '50%')

    // Drag width knob (dy = 200 - 180 = 20 -> 50 + 20*0.5 = 60)
    fireEvent.pointerDown(widthSlider, { clientY: 200, pointerId: 1 })
    fireEvent.pointerMove(widthSlider, { clientY: 180, pointerId: 1 })
    fireEvent.pointerUp(widthSlider, { pointerId: 1 })

    expect(onChange).toHaveBeenCalledWith({ chorusWidth: 60 })
  })
})
