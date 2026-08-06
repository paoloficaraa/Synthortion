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

  describe('machined face', () => {
    it('renders a unique metal radial gradient for the knob face', () => {
      const { rerender } = render(
        <Knob
          label="Drive"
          value={42}
          min={0}
          max={100}
          displayValue="42%"
          onChange={() => {}}
        />
      )

      const gradients = Array.from(document.querySelectorAll('radialGradient'))
      expect(gradients).toHaveLength(1)
      const id = gradients[0]?.getAttribute('id')
      expect(id).toMatch(/^knob-face-/)
      // The face fill references that gradient by id.
      expect(
        Array.from(document.querySelectorAll('circle')).find(
          (circle) => circle.getAttribute('fill') === `url(#${id})`
        )
      ).toBeTruthy()

      const stops = Array.from(
        gradients[0]?.querySelectorAll('stop') ?? []
      ).map((stop) => stop.getAttribute('stop-color'))
      expect(stops).toEqual(['#3a3a3a', '#1a1a1a', '#0c0c0c'])

      // Two knobs render two distinct gradient ids (no SVG id collisions).
      rerender(
        <div>
          <Knob
            label="Drive"
            value={42}
            min={0}
            max={100}
            displayValue="42%"
            onChange={() => {}}
          />
          <Knob
            label="Mix"
            value={10}
            min={0}
            max={100}
            displayValue="10%"
            onChange={() => {}}
          />
        </div>
      )
      const ids = Array.from(
        document.querySelectorAll('radialGradient')
      ).map((g) => g.getAttribute('id'))
      expect(new Set(ids).size).toBe(2)
    })
  })

  describe('polar arc math', () => {
    const radius = 22
    const C = 2 * Math.PI * radius
    const maxArcLength = C * 0.75

    /** The active-value arc is the circle stroked with the accent token. */
    const activeArc = () =>
      Array.from(document.querySelectorAll('circle')).find(
        (circle) => circle.getAttribute('stroke') === 'var(--accent)'
      )

    it('renders the active arc proportional to value on the 3/4 trajectory', () => {
      render(
        <Knob
          label="Drive"
          value={0}
          min={0}
          max={100}
          displayValue="0%"
          onChange={() => {}}
        />
      )

      // pct 0 → offset = C - max(0.001, 0) = C - 0.001
      expect(Number(activeArc()?.getAttribute('stroke-dashoffset'))).toBeCloseTo(
        C - 0.001,
        5
      )
    })

    it('reaches a full 3/4 arc at max value and half at mid value', () => {
      const { rerender } = render(
        <Knob
          label="Drive"
          value={100}
          min={0}
          max={100}
          displayValue="100%"
          onChange={() => {}}
        />
      )

      // pct 1 → offset = C - maxArcLength = C * 0.25
      expect(Number(activeArc()?.getAttribute('stroke-dashoffset'))).toBeCloseTo(
        C * 0.25,
        5
      )

      rerender(
        <Knob
          label="Drive"
          value={50}
          min={0}
          max={100}
          displayValue="50%"
          onChange={() => {}}
        />
      )

      // pct 0.5 → offset = C - maxArcLength / 2
      expect(Number(activeArc()?.getAttribute('stroke-dashoffset'))).toBeCloseTo(
        C - maxArcLength / 2,
        5
      )
    })
  })
})
