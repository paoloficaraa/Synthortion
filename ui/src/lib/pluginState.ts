/** Drive route selectable on the faceplate. */
export type DriveRoute = 'PRE' | 'POST'

/** Delay timebase tie on the faceplate. */
export type DelaySync = 'SYNC' | 'FREE' | 'PING-PONG'

/**
 * Full plugin state hoisted to the App root as controlled props.
 *
 * This object is the single top-level boundary the future C++ DSP bridge
 * binds to: every faceplate control mutates it, and no child component owns
 * silent state of its own.
 */
export interface PluginState {
  drive: number
  driveRoute: DriveRoute
  bitcrush: number
  delayMix: number
  delayTime: number
  delayFbk: number
  delaySync: DelaySync
  chorus: number
  chorusWide: boolean
  inputGain: number
  outputGain: number
  engineActive: boolean
}

/** Factory defaults, matching the prototype's initial values. */
export const initialState: PluginState = {
  drive: 40,
  driveRoute: 'PRE',
  bitcrush: 12,
  delayMix: 30,
  delayTime: 250,
  delayFbk: 50,
  delaySync: 'SYNC',
  chorus: 75,
  chorusWide: false,
  inputGain: 0,
  outputGain: 0,
  engineActive: true,
}
