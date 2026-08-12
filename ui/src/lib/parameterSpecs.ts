import type { PluginState } from './pluginState'

export interface ParameterSpec {
  apvtsId: string
  uiKey: keyof PluginState
  min: number
  max: number
  unit: string
  invert?: boolean
}

export const PARAMETER_SPECS: Record<string, ParameterSpec> = {
  INPUT_GAIN: { apvtsId: 'INPUT_GAIN', uiKey: 'inputGain', min: -60, max: 12, unit: 'dB' },
  OUTPUT_GAIN: { apvtsId: 'OUTPUT_GAIN', uiKey: 'outputGain', min: -60, max: 12, unit: 'dB' },
  COLOR: { apvtsId: 'COLOR', uiKey: 'drive', min: 0, max: 100, unit: '%' },
  BITCRUSH: { apvtsId: 'BITCRUSH', uiKey: 'bitcrush', min: 0, max: 100, unit: '%' },
  DELAY_TIME: { apvtsId: 'DELAY_TIME', uiKey: 'delayTime', min: 1, max: 2000, unit: 'ms' },
  DELAY_MIX: { apvtsId: 'DELAY_MIX', uiKey: 'delayMix', min: 0, max: 100, unit: '%' },
  DELAY_FEEDBACK: { apvtsId: 'DELAY_FEEDBACK', uiKey: 'delayFbk', min: 0, max: 95, unit: '%' },
  CHORUS_MIX: { apvtsId: 'CHORUS_MIX', uiKey: 'chorus', min: 0, max: 100, unit: '%' },
  PLUGIN_BYPASS: { apvtsId: 'PLUGIN_BYPASS', uiKey: 'engineActive', min: 0, max: 1, unit: 'bool', invert: true },
  DRIVE_ON: { apvtsId: 'DRIVE_ON', uiKey: 'driveOn', min: 0, max: 1, unit: 'bool' },
  BITCRUSH_ON: { apvtsId: 'BITCRUSH_ON', uiKey: 'bitcrushOn', min: 0, max: 1, unit: 'bool' },
  DELAY_ON: { apvtsId: 'DELAY_ON', uiKey: 'delayOn', min: 0, max: 1, unit: 'bool' },
  CHORUS_ON: { apvtsId: 'CHORUS_ON', uiKey: 'chorusOn', min: 0, max: 1, unit: 'bool' },
  DRIVE_ROUTE: { apvtsId: 'DRIVE_ROUTE', uiKey: 'driveRoute', min: 0, max: 1, unit: 'enum' },
  DELAY_SYNC: { apvtsId: 'DELAY_SYNC', uiKey: 'delaySync', min: 0, max: 2, unit: 'enum' },
  CHORUS_WIDE: { apvtsId: 'CHORUS_WIDE', uiKey: 'chorusWide', min: 0, max: 1, unit: 'bool' },
}

/** Maps UI key -> ParameterSpec */
export const UI_KEY_TO_SPEC: Record<keyof PluginState, ParameterSpec> = Object.values(
  PARAMETER_SPECS
).reduce(
  (acc, spec) => {
    acc[spec.uiKey] = spec
    return acc
  },
  {} as Record<keyof PluginState, ParameterSpec>
)

/** Convert UI value to normalized APVTS float (0..1) */
export function toAPVTS(uiKey: keyof PluginState, uiValue: unknown): number {
  const spec = UI_KEY_TO_SPEC[uiKey]
  if (!spec) return typeof uiValue === 'number' ? uiValue : 0

  if (spec.unit === 'bool') {
    const boolVal = Boolean(uiValue)
    if (spec.invert) return boolVal ? 0.0 : 1.0
    return boolVal ? 1.0 : 0.0
  }

  if (spec.uiKey === 'driveRoute') {
    return uiValue === 'POST' ? 1.0 : 0.0
  }

  if (spec.uiKey === 'delaySync') {
    if (uiValue === 'FREE') return 0.5
    if (uiValue === 'PING-PONG') return 1.0
    return 0.0 // SYNC
  }

  if (typeof uiValue === 'number') {
    const clamped = Math.max(spec.min, Math.min(spec.max, uiValue))
    return (clamped - spec.min) / (spec.max - spec.min)
  }

  return 0
}

/** Convert normalized APVTS float (0..1) to UI value */
export function fromAPVTS(
  apvtsId: string,
  normalizedValue: number
): { uiKey: keyof PluginState; value: unknown } | null {
  const spec = PARAMETER_SPECS[apvtsId]
  if (!spec) return null

  if (spec.unit === 'bool') {
    const boolVal = normalizedValue > 0.5
    const finalBool = spec.invert ? !boolVal : boolVal
    return { uiKey: spec.uiKey, value: finalBool }
  }

  if (spec.uiKey === 'driveRoute') {
    return { uiKey: spec.uiKey, value: normalizedValue > 0.5 ? 'POST' : 'PRE' }
  }

  if (spec.uiKey === 'delaySync') {
    if (normalizedValue >= 0.75) return { uiKey: spec.uiKey, value: 'PING-PONG' }
    if (normalizedValue >= 0.25) return { uiKey: spec.uiKey, value: 'FREE' }
    return { uiKey: spec.uiKey, value: 'SYNC' }
  }

  const raw = spec.min + normalizedValue * (spec.max - spec.min)
  const clamped = Math.max(spec.min, Math.min(spec.max, raw))
  return { uiKey: spec.uiKey, value: clamped }
}
