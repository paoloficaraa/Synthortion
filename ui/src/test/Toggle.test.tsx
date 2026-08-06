import { render, screen, fireEvent } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { Toggle, type ToggleOption } from '../components/Toggle'

const prePost: ToggleOption[] = [
  { value: 'PRE', label: 'PRE' },
  { value: 'POST', label: 'POST' },
]

describe('Toggle', () => {
  it('renders every segment as a button', () => {
    render(
      <Toggle label="Drive route" options={prePost} value="PRE" onChange={() => {}} />
    )

    expect(screen.getByRole('button', { name: 'PRE' })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'POST' })).toBeInTheDocument()
  })

  it('marks the active segment aria-pressed and the rest not', () => {
    render(
      <Toggle label="Drive route" options={prePost} value="PRE" onChange={() => {}} />
    )

    expect(screen.getByRole('button', { name: 'PRE' })).toHaveAttribute(
      'aria-pressed',
      'true'
    )
    expect(screen.getByRole('button', { name: 'POST' })).toHaveAttribute(
      'aria-pressed',
      'false'
    )
  })

  it('applies active styling to the selected segment and inactive otherwise', () => {
    render(
      <Toggle label="Drive route" options={prePost} value="PRE" onChange={() => {}} />
    )

    const pre = screen.getByRole('button', { name: 'PRE' })
    const post = screen.getByRole('button', { name: 'POST' })

    // Active segment inverts to the foreground (prototype btnClass).
    expect(pre).toHaveClass('bg-fg', 'text-bg', 'border-fg')
    // Inactive segment stays muted/transparent until hovered.
    expect(post).toHaveClass('bg-transparent', 'text-muted', 'border-border')
  })

  it('calls onChange with the clicked segment value', () => {
    const onChange = vi.fn()
    render(
      <Toggle label="Drive route" options={prePost} value="PRE" onChange={onChange} />
    )

    fireEvent.click(screen.getByRole('button', { name: 'POST' }))
    expect(onChange).toHaveBeenCalledWith('POST')
  })

  it('exposes the segmented group with an accessible name', () => {
    render(
      <Toggle label="Drive route" options={prePost} value="PRE" onChange={() => {}} />
    )

    expect(screen.getByRole('group', { name: 'Drive route' })).toBeInTheDocument()
  })

  it('sets the segment row in a recessed hardware track', () => {
    const { container } = render(
      <Toggle label="Drive route" options={prePost} value="PRE" onChange={() => {}} />
    )

    const group = container.querySelector('[role="group"]') as HTMLElement
    expect(group).toHaveClass('bg-elev-1')
    expect(group).toHaveClass('shadow-[inset_0_1px_2px_rgba(0,0,0,0.6)]')
  })

  it('supports a single-option on/off segment (WIDE)', () => {
    const onChange = vi.fn()
    render(
      <Toggle
        label="Chorus width"
        options={[{ value: 'on', label: 'WIDE' }]}
        value="on"
        onChange={onChange}
      />
    )

    const wide = screen.getByRole('button', { name: 'WIDE' })
    expect(wide).toHaveAttribute('aria-pressed', 'true')

    fireEvent.click(wide)
    expect(onChange).toHaveBeenCalledWith('on')
  })
})
