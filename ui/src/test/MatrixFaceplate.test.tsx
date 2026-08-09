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

  it('forwards a drive route toggle flip through onChange', () => {
    const onChange = vi.fn()
    render(<MatrixFaceplate state={initialState} onChange={onChange} />)

    fireEvent.click(screen.getByRole('button', { name: 'POST' }))

    expect(onChange).toHaveBeenCalledWith({ driveRoute: 'POST' })
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
      '12B'
    )
    expect(screen.getByRole('slider', { name: 'Chorus' })).toHaveAttribute(
      'aria-valuetext',
      '75%'
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
})
