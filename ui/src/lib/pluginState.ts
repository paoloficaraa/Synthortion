import type { DspCall } from './dspBridge'


/**
 * Full plugin state hoisted to the App root as controlled props.
 *
 * This object is the single top-level boundary the future C++ DSP bridge
 * binds to: every faceplate control mutates it, and no child component owns
 * silent state of its own. Each of the four modules also carries its own
 * power flag, defaulting to on, so the bridge can honour per-module bypass
 * independently of the master `engineActive`.
 */
export interface PluginState {
  drive: number
  driveOn: boolean
  driveRoute: 'PRE' | 'POST'
  bitcrush: number
  bitcrushOn: boolean
  delayMix: number
  delayTime: number
  delayFbk: number
  delaySync: 'SYNC' | 'FREE' | 'PING-PONG'
  delayOn: boolean
  chorus: number
  chorusWide: boolean
  chorusOn: boolean
  inputGain: number
  outputGain: number
  engineActive: boolean
}

/** Factory defaults, matching the prototype's initial values. */
export const initialState: PluginState = {
  drive: 40,
  driveOn: true,
  driveRoute: 'PRE',
  bitcrush: 12,
  bitcrushOn: true,
  delayMix: 30,
  delayTime: 250,
  delayFbk: 50,
  delaySync: 'SYNC',
  delayOn: true,
  chorus: 75,
  chorusWide: false,
  chorusOn: true,
  inputGain: 0,
  outputGain: 0,
  engineActive: true,
}


/**
 * Diff two plugin states into the bridge calls the App root must push.
 *
 * This is the exact boundary logic the App runs before every render commit:
 * any key that changed — power flags included — lands at the bridge as
 * `{ parameterId, value }`. Hoisted here so the state→bridge contract is
 * testable without a live render.
 */
export function diffPluginState(prev: PluginState, next: PluginState): DspCall[] {
  const calls: DspCall[] = []
  for (const key of Object.keys(next) as Array<keyof PluginState>) {
    if (next[key] !== prev[key]) {
      calls.push({ parameterId: key, value: next[key] })
    }
  }
  return calls
}
