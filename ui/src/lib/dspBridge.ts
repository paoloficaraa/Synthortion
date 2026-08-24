import { UI_KEY_TO_APVTS_ID } from './parameterStore'

export type {
  ParameterType,
  BaseParameterDescriptor,
  FloatParameterDescriptor,
  BoolParameterDescriptor,
  ChoiceParameterDescriptor,
  ParameterDescriptor,
  UIPreferences,
  InitPayload,
  SetParameterPayload,
  ParameterChangePayload,
  MeterFramePayload,
  SpectrumFramePayload,
} from './parameterStore'
/** A single parameter mutation pushed across the bridge. */
export interface DspCall {
  id: string
  value: number
}

/**
 * Integration seam between the React UI and the JUCE C++ DSP backend.
 */
export interface DspBridge {
  setParameter(id: string, value: number): void
}

/** Canonical APVTS parameter IDs (must match C++ ParameterID strings). */
export const PARAMETER_IDS = UI_KEY_TO_APVTS_ID

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
    setParameter(id, value) {
      calls.push({ id, value })
    },
  }
}
/** No-op bridge used when no DSP backend is connected. */
export const noopDspBridge: DspBridge = {
  setParameter() {},
}
