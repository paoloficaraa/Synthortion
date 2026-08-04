/** Value types the C++ bridge understands. */
export type ParameterValue = number | string | boolean

/** A single parameter mutation pushed across the bridge. */
export interface DspCall {
  parameterId: string
  value: ParameterValue
}

/**
 * Integration seam between the React UI and the future C++ DSP backend.
 *
 * A real implementation will be backed by `juce::WebBrowserComponent` native
 * IPC; the mock records every mutation so integration tests can prove that UI
 * manipulation lands at the top-level boundary as parameter changes.
 */
export interface DspBridge {
  setParameter(parameterId: string, value: ParameterValue): void
}

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
