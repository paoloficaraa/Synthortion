/**
 * Hex Scramble — numeric decoding animation for the ASCII slider value label.
 *
 * Rapid adjustments and double-click reset cause numeric value readouts to
 * scramble briefly through terminal hex characters for 30–50 ms before
 * settling cleanly to the formatted display value.
 */

/** Terminal hex characters used during decoding scramble. */
export const HEX_CHARS = '0123456789ABCDEF'

/** Duration of the hex scramble animation in milliseconds (spec: 30–50 ms). */
export const HEX_SCRAMBLE_DURATION_MS = 40

/**
 * Scrambles a display text string into terminal hex characters,
 * replacing alphanumeric characters while preserving punctuation/units.
 */
export function generateHexScramble(
  text: string,
  random: () => number = Math.random
): string {
  if (!text) return ''

  return text
    .split('')
    .map((char) => {
      // Keep whitespace untouched
      if (char === ' ') return char
      // Scramble alphanumeric or replace with random hex char
      if (/[0-9a-zA-Z]/.test(char)) {
        const hexIdx = Math.floor(random() * HEX_CHARS.length)
        return HEX_CHARS[hexIdx]
      }
      return char
    })
    .join('')
}
