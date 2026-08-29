import type { PresetHeader } from './parameterStore'

export type { PresetHeader } from './parameterStore'

/**
 * Top-level preset management state.
 */
export interface PresetState {
  presetCatalog: PresetHeader[]
  activePresetId: string | null
  activePresetName: string
  activePresetCategory: string
  isFactoryPreset: boolean
  isPresetDirty: boolean
  presetOperationToast: string | null
}

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
  delayTimeFree: number
  delayTimeSync: number
  delayFbk: number
  delaySync: boolean
  delayOn: boolean
  chorus: number
  chorusWidth: number
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
  bitcrush: 0,
  bitcrushOn: true,
  delayMix: 0,
  delayTimeFree: 250,
  delayTimeSync: 4,
  delayFbk: 10,
  delaySync: true,
  delayOn: true,
  chorus: 0,
  chorusWidth: 50,
  chorusOn: true,
  inputGain: 0,
  outputGain: 0,
  engineActive: true,
}


/** A diff of a single property change in PluginState. */
export interface StateDiff<K extends keyof PluginState = keyof PluginState> {
  key: K
  value: PluginState[K]
}

/**
 * Diff two plugin states into property changes.
 *
 * This is the exact boundary logic the App runs before every render commit:
 * any key that changed — power flags included — lands as `{ key, value }`.
 * Hoisted here so the state diffing contract is testable without a live render.
 */
export function diffPluginState(prev: PluginState, next: PluginState): StateDiff[] {
  const diffs: StateDiff[] = []
  for (const key of Object.keys(next) as Array<keyof PluginState>) {
    if (next[key] !== prev[key]) {
      diffs.push({ key, value: next[key] })
    }
  }
  return diffs
}
