import { useState, type ComponentProps } from 'react'
import { render, screen, fireEvent, within } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { TrimFader } from '../components/TrimFader'

const MIN_DB = -24
const MAX_DB = 24

function renderFader(
  initial: number,
  props: Partial<ComponentProps<typeof TrimFader>> = {}
) {
  const onChange = vi.fn()

  function Harness() {
    const [value, setValue] = useState(initial)
    return (
      <TrimFader
        label="TRIM"
        value={value}
        displayValue={`${Math.round(value)}`}
        onChange={(next) => {
          setValue(next)
          onChange(next)
        }}
        {...props}
      />
    )
  }

  const { container } = render(<Harness />)
  return { onChange, slider: within(container).getByRole('slider') }
}

function trackText(): string | null {
  return screen.getByTestId('trim-track').textContent
}

describe('TrimFader', () => {
  describe('accessibility', () => {
    it('exposes slider role and value attributes', () => {
      render(
        <TrimFader label="TRIM" value={6} displayValue="+6" onChange={() => {}} />
      )

      const slider = screen.getByRole('slider', { name: 'TRIM' })
      expect(slider).toHaveAttribute('aria-valuemin', String(MIN_DB))
      expect(slider).toHaveAttribute('aria-valuemax', String(MAX_DB))
      expect(slider).toHaveAttribute('aria-valuenow', '6')
      expect(slider).toHaveAttribute('aria-valuetext', '+6')
      expect(slider).toHaveAttribute('aria-orientation', 'vertical')
    })

    it('marks a disabled fader inert and out of tab order', () => {
      render(
        <TrimFader
          label="TRIM"
          value={6}
          displayValue="+6"
          enabled={false}
          onChange={() => {}}
        />
      )

      const slider = screen.getByRole('slider', { name: 'TRIM' })
      expect(slider).toHaveAttribute('aria-disabled', 'true')
      expect(slider).toHaveAttribute('tabindex', '-1')
      expect(slider).toHaveAttribute('aria-valuetext', '--')
    })
  })

  describe('block track', () => {
    it('renders 10 rows with pointer at mid value', () => {
      render(
        <TrimFader label="TRIM" value={0} displayValue="0" onChange={() => {}} />
      )

      const text = trackText()
      expect(text).toBeTruthy()
      // 10 rows, each one character
      expect(text!.length).toBe(10)
      // At value=0, pct=0.5, pointerRow=round(4.5)=5 from bottom → row 4 from top
      // The pointer ▶ should be present
      expect(text).toContain('▶')
    })

    it('places pointer at bottom when value is min', () => {
      render(
        <TrimFader
          label="TRIM"
          value={MIN_DB}
          displayValue="-24"
          onChange={() => {}}
        />
      )

      const text = trackText()!
      // pct=0, pointerRow=0 from bottom → last char is ▶
      const lastChar = text[text.length - 1]
      expect(lastChar).toBe('▶')
    })

    it('places pointer at top when value is max', () => {
      render(
        <TrimFader
          label="TRIM"
          value={MAX_DB}
          displayValue="+24"
          onChange={() => {}}
        />
      )

      const text = trackText()!
      // pct=1, pointerRow=9 from bottom → first char is ▶
      expect(text[0]).toBe('▶')
    })

    it('fills █ below the pointer and · above', () => {
      render(
        <TrimFader label="TRIM" value={0} displayValue="0" onChange={() => {}} />
      )

      const text = trackText()!
      const pointerIdx = text.indexOf('▶')
      expect(pointerIdx).toBeGreaterThan(-1)

      // Below pointer (higher index) should be █
      for (let i = pointerIdx + 1; i < text.length; i++) {
        expect(text[i]).toBe('█')
      }
      // Above pointer (lower index) should be ·
      for (let i = 0; i < pointerIdx; i++) {
        expect(text[i]).toBe('·')
      }
    })

    it('renders the live displayValue readout', () => {
      render(
        <TrimFader label="TRIM" value={6} displayValue="+6" onChange={() => {}} />
      )
      expect(screen.getByText('+6')).toBeInTheDocument()
    })
  })

  describe('pointer drag', () => {
    it('maps vertical drag with sensitivity', () => {
      const { slider, onChange } = renderFader(0)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      // dy = 200 - 180 = 20 → 20 * 0.5 * (48/100) = 4.8 dB
      fireEvent.pointerMove(slider, { clientY: 180, pointerId: 1 })
      fireEvent.pointerUp(slider, { pointerId: 1 })

      expect(onChange).toHaveBeenCalledWith(4.8)
    })

    it('applies a fine step when Shift is held during drag', () => {
      const { slider, onChange } = renderFader(0)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      // dy = 20 → normal would be +4.8; Shift scales sensitivity by 0.1 → +0.48
      fireEvent.pointerMove(slider, { clientY: 180, pointerId: 1, shiftKey: true })
      fireEvent.pointerUp(slider, { pointerId: 1 })

      expect(onChange).toHaveBeenCalledWith(0.48)
    })

    it('clamps upward drag to max', () => {
      const { slider, onChange } = renderFader(20)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      fireEvent.pointerMove(slider, { clientY: -5000, pointerId: 1 })
      fireEvent.pointerUp(slider, { pointerId: 1 })

      expect(onChange).toHaveBeenCalledWith(MAX_DB)
    })

    it('clamps downward drag to min', () => {
      const { slider, onChange } = renderFader(-20)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      fireEvent.pointerMove(slider, { clientY: 5000, pointerId: 1 })
      fireEvent.pointerUp(slider, { pointerId: 1 })

      expect(onChange).toHaveBeenCalledWith(MIN_DB)
    })
  })
  describe('active-drag micro-glitch', () => {
    it('applies a micro-glitch class to track cells while dragging', () => {
      const { slider, onChange } = renderFader(0)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      // Track character spans flagged for flickering while the pointer is down.
      const track = screen.getByTestId('trim-track')
      const glitched = track.querySelectorAll('.trim-glitch')
      expect(glitched.length).toBeGreaterThan(0)

      fireEvent.pointerMove(slider, { clientY: 180, pointerId: 1 })
      fireEvent.pointerUp(slider, { pointerId: 1 })
      expect(onChange).toHaveBeenCalledWith(4.8)
    })

    it('clears the micro-glitch class when the drag ends', () => {
      const { slider } = renderFader(0)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      expect(
        screen.getByTestId('trim-track').querySelectorAll('.trim-glitch').length,
      ).toBeGreaterThan(0)

      fireEvent.pointerUp(slider, { pointerId: 1 })
      expect(
        screen.getByTestId('trim-track').querySelectorAll('.trim-glitch').length,
      ).toBe(0)
    })

    it('keeps the numeric readout free of the micro-glitch class', () => {
      const { slider } = renderFader(0, { displayValue: '+0' })

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      // Readout must stay clean: 100% numeric legibility is an acceptance
      // criterion of issue #64.
      const readout = screen.getByText('+0')
      expect(readout).not.toHaveClass('trim-glitch')

      fireEvent.pointerUp(slider, { pointerId: 1 })
    })
  })

  describe('keyboard step boundaries', () => {
    it('increments by 1 dB on ArrowUp', () => {
      const { slider, onChange } = renderFader(0)
      fireEvent.keyDown(slider, { key: 'ArrowUp' })
      expect(onChange).toHaveBeenCalledWith(1)
    })

    it('decrements by 1 dB on ArrowDown', () => {
      const { slider, onChange } = renderFader(0)
      fireEvent.keyDown(slider, { key: 'ArrowDown' })
      expect(onChange).toHaveBeenCalledWith(-1)
    })

    it('moves by 6 dB when Shift is held', () => {
      const { slider, onChange } = renderFader(0)
      fireEvent.keyDown(slider, { key: 'ArrowUp', shiftKey: true })
      expect(onChange).toHaveBeenCalledWith(6)
    })

    it('jumps to min on Home and max on End', () => {
      const { slider, onChange } = renderFader(0)
      fireEvent.keyDown(slider, { key: 'Home' })
      expect(onChange).toHaveBeenCalledWith(MIN_DB)
      fireEvent.keyDown(slider, { key: 'End' })
      expect(onChange).toHaveBeenCalledWith(MAX_DB)
    })

    it('clamps arrow steps at the range boundaries', () => {
      const nearMax = renderFader(MAX_DB - 0.5)
      fireEvent.keyDown(nearMax.slider, { key: 'ArrowUp' })
      expect(nearMax.onChange).toHaveBeenCalledWith(MAX_DB)

      const nearMin = renderFader(MIN_DB + 0.5)
      fireEvent.keyDown(nearMin.slider, { key: 'ArrowDown' })
      expect(nearMin.onChange).toHaveBeenCalledWith(MIN_DB)
    })

    it('accumulates repeated key presses across controlled updates', () => {
      const { slider, onChange } = renderFader(0)
      fireEvent.keyDown(slider, { key: 'ArrowUp' })
      fireEvent.keyDown(slider, { key: 'ArrowUp' })
      expect(onChange).toHaveBeenLastCalledWith(2)
    })
  })

  describe('enabled state', () => {
    it('renders a dimmed track and -- readout when disabled', () => {
      render(
        <TrimFader
          label="TRIM"
          value={6}
          displayValue="+6"
          enabled={false}
          onChange={() => {}}
        />
      )

      const track = screen.getByTestId('trim-track')
      expect(track).toHaveClass('opacity-40')
      expect(screen.getByText('--')).toBeInTheDocument()
      expect(screen.queryByText('+6')).not.toBeInTheDocument()
    })

    it('restores the live readout when re-enabled', () => {
      const { rerender } = render(
        <TrimFader
          label="TRIM"
          value={6}
          displayValue="+6"
          enabled={false}
          onChange={() => {}}
        />
      )

      expect(screen.getByText('--')).toBeInTheDocument()

      rerender(
        <TrimFader
          label="TRIM"
          value={6}
          displayValue="+6"
          enabled={true}
          onChange={() => {}}
        />
      )

      expect(screen.queryByText('--')).not.toBeInTheDocument()
      expect(screen.getByText('+6')).toBeInTheDocument()
    })

    it('ignores pointer drag while disabled', () => {
      const { slider, onChange } = renderFader(0, { enabled: false })

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      fireEvent.pointerMove(slider, { clientY: 180, pointerId: 1 })
      fireEvent.pointerUp(slider, { pointerId: 1 })

      expect(onChange).not.toHaveBeenCalled()
    })

    it('ignores keyboard input while disabled', () => {
      const { slider, onChange } = renderFader(0, { enabled: false })

      fireEvent.keyDown(slider, { key: 'ArrowUp' })
      fireEvent.keyDown(slider, { key: 'Home' })

      expect(onChange).not.toHaveBeenCalled()
    })
  })
})