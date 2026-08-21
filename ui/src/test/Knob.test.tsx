import { useState, type ComponentProps } from 'react'
import { render, screen, fireEvent, within } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { Knob } from '../components/Knob'

/**
 * Renders a controlled Knob so repeated keyboard presses accumulate.
 * Returns the mock onChange plus the slider element scoped to this render
 * (safe to call multiple times within one test).
 */
function renderKnob(
  initial: number,
  props: Partial<ComponentProps<typeof Knob>> = {}
) {
  const onChange = vi.fn()

  function Harness() {
    const [value, setValue] = useState(initial)
    return (
      <Knob
        label="Drive"
        value={value}
        min={0}
        max={100}
        displayValue={`${Math.round(value)}%`}
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

/** The concatenated `[====+----]` track string from the rendered knob. */
function trackText(): string | null {
  return screen.getByTestId('knob-track').textContent
}

describe('Knob', () => {
  describe('accessibility', () => {
    it('exposes slider role and value attributes', () => {
      render(
        <Knob
          label="Drive"
          value={42}
          min={0}
          max={100}
          displayValue="42%"
          onChange={() => {}}
        />
      )

      const slider = screen.getByRole('slider', { name: 'Drive' })
      expect(slider).toHaveAttribute('aria-valuemin', '0')
      expect(slider).toHaveAttribute('aria-valuemax', '100')
      expect(slider).toHaveAttribute('aria-valuenow', '42')
      expect(slider).toHaveAttribute('aria-valuetext', '42%')
      expect(slider).toHaveAttribute('aria-orientation', 'vertical')
    })

    it('marks a disabled knob inert and out of tab order', () => {
      render(
        <Knob
          label="Drive"
          value={42}
          min={0}
          max={100}
          displayValue="42%"
          enabled={false}
          onChange={() => {}}
        />
      )

      const slider = screen.getByRole('slider', { name: 'Drive' })
      expect(slider).toHaveAttribute('aria-disabled', 'true')
      expect(slider).toHaveAttribute('tabindex', '-1')
      // The readout collapses to `--` and stays out of the value contract.
      expect(slider).toHaveAttribute('aria-valuetext', '--')
    })
  })

  describe('block track', () => {
    it('renders the canonical [====+----] pattern at mid value', () => {
      render(
        <Knob
          label="Drive"
          value={50}
          min={0}
          max={100}
          displayValue="50%"
          onChange={() => {}}
        />
      )

      expect(trackText()).toBe('[████▒----]')
    })

    it('maps the pointer to the value across the track', () => {
      const { rerender } = render(
        <Knob
          label="Drive"
          value={0}
          min={0}
          max={100}
          displayValue="0%"
          onChange={() => {}}
        />
      )

      // pct 0 → completely empty
      expect(trackText()).toBe('[---------]')

      rerender(
        <Knob
          label="Drive"
          value={100}
          min={0}
          max={100}
          displayValue="100%"
          onChange={() => {}}
        />
      )
      // pct 1 → everything solid
      expect(trackText()).toBe('[█████████]')

      rerender(
        <Knob

          label="Drive"
          value={12.5}
          min={0}
          max={100}
          displayValue="13%"
          onChange={() => {}}
        />
      )
      // 12.5% = 4.5 steps out of 36. So 1st cell (4) is filled, 2nd cell is (4.5-4=0.5 -> 1 step -> '░').
      expect(trackText()).toBe('[█░-------]')
    })
  })

  describe('pointer drag', () => {
    it('maps vertical drag 1:1 with sensitivity', () => {
      const { slider, onChange } = renderKnob(50)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      // dy = startY - clientY = 200 - 180 = 20 → 20 * 0.5 * 1.0 = +10
      fireEvent.pointerMove(slider, { clientY: 180, pointerId: 1 })
      fireEvent.pointerUp(slider, { pointerId: 1 })

      expect(onChange).toHaveBeenCalledWith(60)
    })

    it('applies a fine step when Shift is held during drag', () => {
      const { slider, onChange } = renderKnob(50)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      // dy = 20 → normal would be +10; Shift scales sensitivity by 0.1 → +1
      fireEvent.pointerMove(slider, { clientY: 180, pointerId: 1, shiftKey: true })
      fireEvent.pointerUp(slider, { pointerId: 1 })

      expect(onChange).toHaveBeenCalledWith(51)
    })

    it('clamps upward drag to max', () => {
      const { slider, onChange } = renderKnob(90)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      fireEvent.pointerMove(slider, { clientY: -5000, pointerId: 1 })
      fireEvent.pointerUp(slider, { pointerId: 1 })

      expect(onChange).toHaveBeenCalledWith(100)
    })

    it('clamps downward drag to min', () => {
      const { slider, onChange } = renderKnob(10)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      fireEvent.pointerMove(slider, { clientY: 5000, pointerId: 1 })
      fireEvent.pointerUp(slider, { pointerId: 1 })

      expect(onChange).toHaveBeenCalledWith(0)
    })
  })
  describe('active-drag micro-glitch', () => {
    it('applies a micro-glitch class to the track border cells while dragging', () => {
      const { slider, onChange } = renderKnob(50)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      // While active, the border spans `[` and `]` are flagged for flickering.
      const track = screen.getByTestId('knob-track')
      const borders = Array.from(track.querySelectorAll('.knob-glitch'))
      // Exactly the two border cells are glitched — not the inner track cells.
      expect(borders).toHaveLength(2)

      fireEvent.pointerMove(slider, { clientY: 180, pointerId: 1 })
      fireEvent.pointerUp(slider, { pointerId: 1 })
      expect(onChange).toHaveBeenCalledWith(60)
    })

    it('clears the micro-glitch class when the drag ends', () => {
      const { slider } = renderKnob(50)

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      expect(screen.getByTestId('knob-track').querySelectorAll('.knob-glitch'))
        .toHaveLength(2)

      fireEvent.pointerUp(slider, { pointerId: 1 })
      expect(screen.getByTestId('knob-track').querySelectorAll('.knob-glitch'))
        .toHaveLength(0)
    })

    it('keeps the numeric readout free of the micro-glitch class', () => {
      const { slider } = renderKnob(50, {
        displayValue: '50%',
      })

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      // The displayValue readout must never carry the glitch class —
      // 100% numeric legibility is an acceptance criterion.
      const readout = screen.getByText('50%')
      expect(readout).not.toHaveClass('knob-glitch')

      fireEvent.pointerUp(slider, { pointerId: 1 })
    })
  })

  describe('keyboard step boundaries', () => {
    it('increments by step on ArrowUp', () => {
      const { slider, onChange } = renderKnob(50)
      fireEvent.keyDown(slider, { key: 'ArrowUp' })
      // step = (max - min) / 100 = 1
      expect(onChange).toHaveBeenCalledWith(51)
    })

    it('decrements by step on ArrowDown', () => {
      const { slider, onChange } = renderKnob(50)
      fireEvent.keyDown(slider, { key: 'ArrowDown' })
      expect(onChange).toHaveBeenCalledWith(49)
    })

    it('moves by large step when Shift is held', () => {
      const { slider, onChange } = renderKnob(50)
      fireEvent.keyDown(slider, { key: 'ArrowUp', shiftKey: true })
      // largeStep = (max - min) / 10 = 10
      expect(onChange).toHaveBeenCalledWith(60)
    })

    it('jumps to min on Home and max on End', () => {
      const { slider, onChange } = renderKnob(50)
      fireEvent.keyDown(slider, { key: 'Home' })
      expect(onChange).toHaveBeenCalledWith(0)
      fireEvent.keyDown(slider, { key: 'End' })
      expect(onChange).toHaveBeenCalledWith(100)
    })

    it('clamps arrow steps at the range boundaries', () => {
      const nearMax = renderKnob(99.8)
      fireEvent.keyDown(nearMax.slider, { key: 'ArrowUp' })
      expect(nearMax.onChange).toHaveBeenCalledWith(100)

      const nearMin = renderKnob(0.2)
      fireEvent.keyDown(nearMin.slider, { key: 'ArrowDown' })
      expect(nearMin.onChange).toHaveBeenCalledWith(0)
    })

    it('accumulates repeated key presses across controlled updates', () => {
      const { slider, onChange } = renderKnob(50)
      fireEvent.keyDown(slider, { key: 'ArrowUp' })
      fireEvent.keyDown(slider, { key: 'ArrowUp' })
      expect(onChange).toHaveBeenLastCalledWith(52)
    })
  })

  describe('enabled state', () => {
    it('renders a dimmed track and -- readout when disabled', () => {
      render(
        <Knob
          label="Drive"
          value={42}
          min={0}
          max={100}
          displayValue="42%"
          enabled={false}
          onChange={() => {}}
        />
      )

      const track = screen.getByTestId('knob-track')
      expect(track).toHaveClass('opacity-40')
      // The track still shows the value pattern (42%), dimmed.
      expect(track.textContent).toBe('[███▓-----]')
      expect(screen.getByText('--')).toBeInTheDocument()
      expect(screen.queryByText('42%')).not.toBeInTheDocument()
    })

    it('restores the live readout when re-enabled', () => {
      const { rerender } = render(
        <Knob
          label="Drive"
          value={42}
          min={0}
          max={100}
          displayValue="42%"
          enabled={false}
          onChange={() => {}}
        />
      )

      expect(screen.getByText('--')).toBeInTheDocument()

      rerender(
        <Knob
          label="Drive"
          value={42}
          min={0}
          max={100}
          displayValue="42%"
          enabled={true}
          onChange={() => {}}
        />
      )

      expect(screen.queryByText('--')).not.toBeInTheDocument()
      expect(screen.getByText('42%')).toBeInTheDocument()
    })

    it('ignores pointer drag while disabled', () => {
      const { slider, onChange } = renderKnob(50, { enabled: false })

      fireEvent.pointerDown(slider, { clientY: 200, pointerId: 1 })
      fireEvent.pointerMove(slider, { clientY: 180, pointerId: 1 })
      fireEvent.pointerUp(slider, { pointerId: 1 })

      expect(onChange).not.toHaveBeenCalled()
    })

    it('ignores keyboard input while disabled', () => {
      const { slider, onChange } = renderKnob(50, { enabled: false })

      fireEvent.keyDown(slider, { key: 'ArrowUp' })
      fireEvent.keyDown(slider, { key: 'Home' })

      expect(onChange).not.toHaveBeenCalled()
    })
  })
})
