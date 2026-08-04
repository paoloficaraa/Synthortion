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
})
