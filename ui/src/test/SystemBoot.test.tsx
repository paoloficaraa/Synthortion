import { render, screen, fireEvent, act, within } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { prefersReducedMotion } from 'motion-dom'
import { SystemBoot } from '../components/SystemBoot'

/** Number of rendered boot lines: header + four staged steps. */
const BOOT_LINE_COUNT = 5

/**
 * The boot sequence is a one-shot staged terminal overlay: it types out real
 * values with aligned `[ OK ]` columns, ends on `[READY]`, auto-dismisses
 * after ~2.5s, and skips to the final state on click/Enter. Reduced-motion
 * users get the final state rendered immediately. Fake timers drive the
 * lifecycle; jsdom cannot evaluate the clip-path CSS, so the typewriter
 * mechanism is pinned via the inline `--chars` step count and the
 * `data-reduce-motion` marker.
 */
describe('SystemBoot', () => {
  beforeEach(() => {
    vi.useFakeTimers()
  })

  afterEach(() => {
    vi.useRealTimers()
    vi.unstubAllGlobals()
  })

  it('renders the staged boot log with real values and aligned OK columns', () => {
    render(<SystemBoot />)

    expect(screen.getByText('SYNTHORTION v0.1')).toBeInTheDocument()
    expect(screen.getByText('SAMPLE RATE ......')).toBeInTheDocument()
    expect(screen.getByText('48 kHz')).toBeInTheDocument()
    expect(screen.getByText('BUFFER SIZE ....')).toBeInTheDocument()
    expect(screen.getByText('256')).toBeInTheDocument()
    expect(screen.getByText('MODULES:')).toBeInTheDocument()
    expect(screen.getByText('DRV BCR DLY CHR')).toBeInTheDocument()
    expect(screen.getByText('DSP ENGINE ......')).toBeInTheDocument()

    // Status markers: three aligned `[ OK ]` columns ending on `[READY]`.
    expect(screen.getAllByText('[ OK ]')).toHaveLength(3)
    expect(screen.getByText('[READY]')).toBeInTheDocument()
  })

  it('types out via CSS clip: full text stays in the DOM with per-line steps', () => {
    const { container } = render(<SystemBoot />)

    // Every staged line is fully present (no progressive textContent).
    expect(screen.getByText('SAMPLE RATE ......')).toBeInTheDocument()
    expect(screen.getByText('DRV BCR DLY CHR')).toBeInTheDocument()
    expect(screen.getByText('[READY]')).toBeInTheDocument()

    // The reveal is driven by per-line clip-path step counts (characters),
    // not by mutating the text — each line carries its own `--chars` + delay.
    const lines = [...container.querySelectorAll('.boot-line')] as HTMLElement[]
    expect(lines).toHaveLength(BOOT_LINE_COUNT)
    expect(lines[0].style.getPropertyValue('--chars')).toBe('16')
    expect(lines[0].style.getPropertyValue('--type-delay')).toBe('0ms')
    expect(lines[1].style.getPropertyValue('--type-delay')).toBe('180ms')
    expect(lines[2].style.getPropertyValue('--type-delay')).toBe('360ms')
    expect(lines[3].style.getPropertyValue('--type-delay')).toBe('540ms')
    expect(lines[4].style.getPropertyValue('--type-delay')).toBe('720ms')
  })

  it('auto-dismisses after ~2.5s', () => {
    render(<SystemBoot />)

    expect(screen.getByTestId('system-boot-overlay')).toBeInTheDocument()

    // Just before the one-shot window elapses the overlay is still present.
    act(() => {
      vi.advanceTimersByTime(2499)
    })
    expect(screen.getByTestId('system-boot-overlay')).toBeInTheDocument()

    act(() => {
      vi.advanceTimersByTime(1)
    })
    expect(
      screen.queryByTestId('system-boot-overlay')
    ).not.toBeInTheDocument()
  })

  it('skips to the final state on click', () => {
    render(<SystemBoot />)

    fireEvent.click(screen.getByTestId('system-boot-overlay'))

    expect(
      screen.queryByTestId('system-boot-overlay')
    ).not.toBeInTheDocument()
  })

  it('skips to the final state on Enter', () => {
    render(<SystemBoot />)

    fireEvent.keyDown(screen.getByTestId('system-boot-overlay'), {
      key: 'Enter',
    })

    expect(
      screen.queryByTestId('system-boot-overlay')
    ).not.toBeInTheDocument()
  })

  it('renders the blinking block cursor and the decorative stream aria-hidden', () => {
    const { container } = render(<SystemBoot />)

    const cursor = container.querySelector('.block-cursor') as HTMLElement
    expect(cursor).toBeInTheDocument()
    expect(cursor).toHaveAttribute('aria-hidden', 'true')
    expect(cursor.textContent).toBe('▊')

    const stream = container.querySelector('.boot-stream') as HTMLElement
    expect(stream).toBeInTheDocument()
    expect(stream).toHaveAttribute('aria-hidden', 'true')

    // The log keeps the aligned status column: every `[ OK ]`/`[READY]` marker
    // lives in its own `.boot-status` cell inside the boot-log grid.
    const log = container.querySelector('.boot-log')
    const statuses = within(log as HTMLElement).getAllByText(/^\[( OK |READY)\]$/)
    expect(statuses).toHaveLength(4)
  })

  it('renders the final state immediately under reduced motion', () => {
    // framer-motion's `useReducedMotion` reads a module-level singleton that
    // was initialized once at first import. Setting its `.current` before
    // render makes the hook return `true` for the fresh SystemBoot tree.
    const previous = prefersReducedMotion.current
    prefersReducedMotion.current = true

    const { container } = render(<SystemBoot />)

    // The full staged log — including the final `[READY]` — is present at
    // mount (the clip reveal is disabled, so nothing is hidden to type out).
    expect(screen.getByText('SYNTHORTION v0.1')).toBeInTheDocument()
    expect(screen.getByText('[READY]')).toBeInTheDocument()
    expect(screen.getAllByText('[ OK ]')).toHaveLength(3)

    const log = container.querySelector('.boot-log')
    expect(log).toHaveAttribute('data-reduce-motion', 'true')

    // Restore the global so other tests are unaffected.
    prefersReducedMotion.current = previous
  })
})
