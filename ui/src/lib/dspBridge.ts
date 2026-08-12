/** Value types the C++ bridge understands. */
export type ParameterValue = number | string | boolean

/** A single parameter mutation pushed across the bridge. */
export interface DspCall {
  parameterId: string
  value: ParameterValue
}

/**
 * Integration seam between the React UI and the JUCE C++ DSP backend.
 */
export interface DspBridge {
  setParameter(parameterId: string, value: ParameterValue): void
}

/** Canonical APVTS parameter IDs (must match C++ ParameterID strings). */
export const PARAMETER_IDS = {
  inputGain: 'INPUT_GAIN',
  outputGain: 'OUTPUT_GAIN',
  drive: 'COLOR',
  bitcrush: 'BITCRUSH',
  delayTime: 'DELAY_TIME',
  delayMix: 'DELAY_MIX',
  delayFbk: 'DELAY_FEEDBACK',
  chorus: 'CHORUS_MIX',
  engineActive: 'PLUGIN_BYPASS',
  driveOn: 'DRIVE_ON',
  bitcrushOn: 'BITCRUSH_ON',
  delayOn: 'DELAY_ON',
  chorusOn: 'CHORUS_ON',
  driveRoute: 'DRIVE_ROUTE',
  delaySync: 'DELAY_SYNC',
  chorusWide: 'CHORUS_WIDE',
} as const

/** UI keys that are sent to the DSP bridge. */
export const BRIDGED_UI_KEYS = new Set(Object.keys(PARAMETER_IDS))

/** A recording mock bridge for tests and local development. */
export interface MockDspBridge extends DspBridge {
  calls: DspCall[]
}

/** Builds a fresh recording mock bridge. */
export function createMockDspBridge(): MockDspBridge {
  const calls: DspCall[] = []
  return {
    calls,
    setParameter(parameterId, value) {
      calls.push({ parameterId, value })
    },
  }
}

/** No-op bridge used when no DSP backend is connected. */
export const noopDspBridge: DspBridge = {
  setParameter() {},
}
