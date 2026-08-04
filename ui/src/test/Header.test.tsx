import { render, screen, fireEvent } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { Header } from '../components/Header'

describe('Header', () => {
  it('renders the brand, bypass LED, preset LCD and SAVE/LOAD actions', () => {
    render(<Header engineActive onToggleBypass={() => {}} />)

    expect(screen.getByRole('heading', { name: 'SYNTHORTION' })).toBeInTheDocument()
    expect(screen.getByText('INIT_STATE_01')).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'SAVE' })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'LOAD' })).toBeInTheDocument()
    expect(
      screen.getByRole('button', { name: 'Disable main DSP' })
    ).toBeInTheDocument()
  })

  it('flips the bypass LED and reports the new engine state', () => {
    const onToggleBypass = vi.fn()
    render(<Header engineActive onToggleBypass={onToggleBypass} />)

    const bypass = screen.getByRole('button', { name: 'Disable main DSP' })
    expect(bypass).toHaveAttribute('aria-pressed', 'true')

    fireEvent.click(bypass)

    expect(onToggleBypass).toHaveBeenCalledWith(false)
  })
})
