import { render, screen } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import { VstLayout } from '../components/VstLayout'

describe('VstLayout', () => {
  it('renders three-column layout with children', () => {
    render(
      <VstLayout
        leftColumn={<div data-testid="left">Input</div>}
        rightColumn={<div data-testid="right">Output</div>}
      >
        <div data-testid="center">Main Controls</div>
      </VstLayout>
    )

    expect(screen.getByTestId('left')).toBeInTheDocument()
    expect(screen.getByTestId('center')).toBeInTheDocument()
    expect(screen.getByTestId('right')).toBeInTheDocument()
    expect(screen.getByText('Input')).toBeInTheDocument()
    expect(screen.getByText('Main Controls')).toBeInTheDocument()
    expect(screen.getByText('Output')).toBeInTheDocument()
  })

  it('renders center column without side columns', () => {
    render(
      <VstLayout>
        <div data-testid="center">Main Controls</div>
      </VstLayout>
    )

    expect(screen.getByTestId('center')).toBeInTheDocument()
  })

  it('applies vst-container class with noise overlay', () => {
    const { container } = render(
      <VstLayout>
        <div>Content</div>
      </VstLayout>
    )

    const vstContainer = container.querySelector('.vst-container')
    expect(vstContainer).toBeInTheDocument()
    expect(vstContainer).toHaveClass('noise-overlay')
  })

  it('applies flex-row layout for 3-column structure', () => {
    const { container } = render(
      <VstLayout>
        <div>Content</div>
      </VstLayout>
    )

    const vstContainer = container.querySelector('.vst-container')
    expect(vstContainer).toHaveClass('flex-row')
  })

  it('renders four box-drawing corner brackets in the ASCII face', () => {
    const { container } = render(
      <VstLayout>
        <div>Content</div>
      </VstLayout>
    )

    const brackets = container.querySelectorAll('[data-testid="corner-bracket"]')
    expect(brackets).toHaveLength(4)
    expect([...brackets].map((b) => b.textContent)).toEqual([
      '┌',
      '┐',
      '└',
      '┘',
    ])
  })
})
