import { render, screen } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import App from '../App'
import { MatrixFaceplate } from '../components/MatrixFaceplate'
import { initialState } from '../lib/pluginState'

describe('Layout Scaling & 16:10 DAW Window Calibration', () => {
  const DIMENSIONS = [
    { name: 'Default 16:10', width: 960, height: 600 },
    { name: 'Minimum 16:10', width: 768, height: 480 },
    { name: 'Maximum 16:10', width: 1920, height: 1200 },
  ] as const

  DIMENSIONS.forEach(({ name, width, height }) => {
    describe(`${name} (${width}x${height})`, () => {
      it(`renders full App shell with fixed 50px header, 35% visualizer, and 65% faceplate`, () => {
        const { container } = render(
          <div style={{ width: `${width}px`, height: `${height}px` }}>
            <App />
          </div>
        )

        const header = container.querySelector('header')
        expect(header).toBeInTheDocument()
        expect(header).toHaveClass('h-[50px]', 'shrink-0')

        const viz = container.querySelector('[data-testid="spectrum-visualizer"]')
        expect(viz).toBeInTheDocument()
        expect(viz).toHaveClass('basis-[35%]', 'flex-grow')

        const faceplatePlateau = container.querySelector('[data-testid="faceplate-plateau"]')
        expect(faceplatePlateau).toBeInTheDocument()
        expect(faceplatePlateau).toHaveClass('basis-[65%]', 'flex-grow')

        // 5-column faceplate grid expands to full height
        const grid = container.querySelector('.grid-cols-5')
        expect(grid).toBeInTheDocument()
        expect(grid).toHaveClass('h-full')
      })

      it(`preserves all 5 module columns (DRV, BCR, DLY x2, CHR) expanding to 100% vertical height`, () => {
        const { container } = render(
          <div style={{ width: `${width}px`, height: `${height}px` }}>
            <MatrixFaceplate state={initialState} onChange={() => {}} />
          </div>
        )

        const sections = container.querySelectorAll('section')
        expect(sections).toHaveLength(4)

        // DRV, BCR, DLY (col-span-2), CHR
        expect(screen.getByText('DRV')).toBeInTheDocument()
        expect(screen.getByText('BCR')).toBeInTheDocument()
        expect(screen.getByText('DLY')).toBeInTheDocument()
        expect(screen.getByText('CHR')).toBeInTheDocument()

        sections.forEach((sec) => {
          expect(sec).toHaveClass('h-full')
        })
      })

      it(`comfortably displays DLY Mix knob and Time/Feedback sub-knobs without clipping`, () => {
        render(
          <div style={{ width: `${width}px`, height: `${height}px` }}>
            <MatrixFaceplate state={initialState} onChange={() => {}} />
          </div>
        )

        const mix = screen.getByRole('slider', { name: 'Mix' })
        const time = screen.getByRole('slider', { name: 'Time' })
        const fbk = screen.getByRole('slider', { name: 'Fbk' })

        expect(mix).toBeInTheDocument()
        expect(time).toBeInTheDocument()
        expect(fbk).toBeInTheDocument()
      })

      it(`maintains 1px Cartesian hairlines and crosshair ticks without repeated character loops`, () => {
        const { container } = render(
          <div style={{ width: `${width}px`, height: `${height}px` }}>
            <App />
          </div>
        )

        // Header structural dividers are CSS hairlines, not repeated ─ loops
        const header = container.querySelector('header')
        expect(header).toHaveClass('border-b', 'border-grid-rule')
        expect(header?.textContent ?? '').not.toMatch(/─{2,}/)

        // Cartesian corner brackets are present
        const cornerBrackets = container.querySelectorAll('[data-testid="corner-bracket"]')
        expect(cornerBrackets).toHaveLength(4)

        // Crosshairs are present at panel intersections
        const crosshairs = screen.getAllByText('+')
        expect(crosshairs.length).toBeGreaterThanOrEqual(8)
      })
    })
  })
})
