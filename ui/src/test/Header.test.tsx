import { render, screen, fireEvent, within } from '@testing-library/react'
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

  it('renders the preset readout in the VGA face with a blinking block cursor', () => {
    const { container } = render(<Header engineActive onToggleBypass={() => {}} />)

    const lcd = container.querySelector('.bg-elev-0') as HTMLElement
    expect(lcd).toBeInTheDocument()
    expect(lcd).toHaveClass('font-ascii')

    const cursor = within(lcd).getByText('▊')
    expect(cursor).toHaveAttribute('aria-hidden', 'true')
    expect(cursor).toHaveClass('block-cursor')

    // The readout keeps its full preset name next to the cursor.
    expect(within(lcd).getByText('INIT_STATE_01')).toBeInTheDocument()
  })
  it('renders [ SAVE ] [ LOAD ] bracket buttons with hover inversion', () => {
    const { container } = render(<Header engineActive onToggleBypass={() => {}} />)

    for (const label of ['SAVE', 'LOAD']) {
      const button = screen.getByRole('button', { name: label })
      // Terminal bracket chrome wraps the label; the brackets are decorative.
      expect(button.textContent).toContain(`[ ${label} ]`)
      expect(button.querySelectorAll('[aria-hidden="true"]')).toHaveLength(2)
      // Hover inversion: the button flips to foreground on hover.
      expect(button).toHaveClass('hover:bg-fg', 'hover:text-bg', 'hover:border-fg')
      expect(button).toHaveClass('border', 'font-mono')
    }

    // No stray bracket text leaks into the accessibility tree.
    expect(container.querySelectorAll('[aria-hidden="true"]').length).toBeGreaterThanOrEqual(
      4
    )
  })
})
