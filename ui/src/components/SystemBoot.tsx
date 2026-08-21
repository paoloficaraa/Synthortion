import { useEffect, useState, type CSSProperties } from 'react'
import { useReducedMotion } from 'framer-motion'

/** Milliseconds before the boot sequence auto-dismisses. */
const BOOT_DURATION_MS = 2500

/** Stagger between staged log lines in the typewriter reveal. */
const LINE_STAGGER_MS = 180

/** Brand header line of the boot log. */
const BOOT_HEADER = 'SYNTHORTION v0.1'

/** Decorative density stream shown under the log (`aria-hidden`). */
const BOOT_STREAM = '░▒▓█░▒▓█░▒▓█░▒▓█░▒▓█░▒▓█░▒▓█░▒▓█░▒▓█░▒▓█'

/**
 * Staged boot log lines — real values (sample rate, buffer size, module list)
 * with a status marker. The label carries the dot-leader padding; the status
 * column aligns across every row and the last line ends on `[READY]`.
 */
const BOOT_STEPS = [
  { label: 'SAMPLE RATE ......', value: '48 kHz', status: '[ OK ]' },
  { label: 'BUFFER SIZE ....', value: '256', status: '[ OK ]' },
  { label: 'MODULES:', value: 'DRV BCR DLY CHR', status: '[ OK ]' },
  { label: 'DSP ENGINE ......', value: '', status: '[READY]' },
]

/** Total characters in a line — the typewriter's step count (one per glyph). */
function lineChars(label: string, value: string, status: string): number {
  return label.length + value.length + status.length
}

/**
 * SystemBoot — the mount-time staged terminal boot sequence.
 *
 * Plays once when the plugin is first rendered: a fixed full-cover layer that
 * types out a staged log of real startup values, ending on `[READY]`, with a
 * blinking block cursor. The typewriter is purely visual — it animates CSS
 * `clip-path` over the fully-present DOM text, so assistive tech reads the
 * complete log. It auto-dismisses after ~2.5s; a click or Enter skips to the
 * final state. Users who prefer reduced motion see the final state rendered
 * immediately (no clip animation, no blinking).
 */
export function SystemBoot() {
  const [visible, setVisible] = useState(true)
  const reduceMotion = useReducedMotion()

  useEffect(() => {
    const timer = window.setTimeout(() => setVisible(false), BOOT_DURATION_MS)
    return () => window.clearTimeout(timer)
  }, [])

  if (!visible) return null
  return (
    <button
      type="button"
      data-testid="system-boot-overlay"
      aria-label="Dismiss boot sequence"
      onClick={() => setVisible(false)}
      onKeyDown={(event) => {
        if (event.key === 'Enter') setVisible(false)
      }}
      className="fixed inset-0 z-[100] flex flex-col items-center justify-center gap-6 bg-void cursor-pointer outline-none"
    >
      {/* Soft overlay (gradient removed for slop) */}
      <div className="absolute inset-0 pointer-events-none opacity-[0.12]" />
      <div
        className="boot-log relative z-10"
        data-reduce-motion={reduceMotion ? 'true' : undefined}
      >
        <div
          className="boot-line boot-header"
          style={
            {
              '--chars': BOOT_HEADER.length,
              '--type-delay': '0ms',
            } as CSSProperties
          }
        >
          <span>{BOOT_HEADER}</span>
        </div>
        {BOOT_STEPS.map((step, i) => (
          <div
            key={step.label}
            className="boot-line"
            style={
              {
                '--chars': lineChars(step.label, step.value, step.status),
                '--type-delay': `${(i + 1) * LINE_STAGGER_MS}ms`,
              } as CSSProperties
            }
          >
            <span className="boot-label">{step.label}</span>
            <span className="boot-value">{step.value}</span>
            <span className="boot-status">
              {step.status}
              {i === BOOT_STEPS.length - 1 && (
                <span className="block-cursor" aria-hidden="true">
                  ▊
                </span>
              )}
            </span>
          </div>
        ))}
      </div>

      <div className="boot-stream" aria-hidden="true">
        {BOOT_STREAM}
      </div>

      <span className="font-mono text-[9px] text-ink-2 uppercase-tracked select-none relative z-10">
        CLICK OR PRESS ENTER TO SKIP
      </span>
    </button>
  )
}
