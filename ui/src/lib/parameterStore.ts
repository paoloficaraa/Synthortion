import type { PluginState } from './pluginState'

export type ParameterType = 'float' | 'bool' | 'choice'

export interface BaseParameterDescriptor {
  id: string
  name: string
  type: ParameterType
  normalizedDefault: number
  normalizedValue: number
}

export interface FloatParameterDescriptor extends BaseParameterDescriptor {
  type: 'float'
  min: number
  max: number
  defaultValue: number
  currentValue: number
  skew: number
  step: number
  unit: string
}

export interface BoolParameterDescriptor extends BaseParameterDescriptor {
  type: 'bool'
  defaultValue: boolean
  currentValue: boolean
  labelOn?: string
  labelOff?: string
  invert?: boolean
}

export interface ChoiceParameterDescriptor extends BaseParameterDescriptor {
  type: 'choice'
  choices: string[]
  defaultIndex: number
  currentIndex: number
}

export type ParameterDescriptor =
  | FloatParameterDescriptor
  | BoolParameterDescriptor
  | ChoiceParameterDescriptor

export interface UIPreferences {
  uiScale?: number
  spectrumDecay?: number
  skipBootSequence?: boolean
}

export interface InitPayload {
  schemaVersion: 1
  parameters: ParameterDescriptor[]
  uiPreferences?: UIPreferences
}

export interface SetParameterPayload {
  id: string
  value: number // Normalized [0.0, 1.0]
}

export interface ParameterChangePayload {
  id: string
  value: number // Normalized [0.0, 1.0]
}

export interface MeterFramePayload {
  input: number
  output: number
}

export type SpectrumFramePayload = number[]

export const DELAY_SUBDIVISIONS = [
  '1/32',
  '1/16T',
  '1/16',
  '1/16D',
  '1/8T',
  '1/8',
  '1/8D',
  '1/4T',
  '1/4',
  '1/4D',
  '1/2T',
  '1/2',
  '1/2D',
  '1/1',
] as const

export type DelaySubdivision = (typeof DELAY_SUBDIVISIONS)[number]

export const DEFAULT_PARAMETER_DESCRIPTORS: Record<string, ParameterDescriptor> = {
  INPUT_GAIN: {
    id: 'INPUT_GAIN',
    name: 'Input Gain',
    type: 'float',
    min: -60,
    max: 12,
    defaultValue: 0,
    currentValue: 0,
    skew: 1,
    step: 0.1,
    unit: 'dB',
    normalizedDefault: (0 - -60) / (12 - -60),
    normalizedValue: (0 - -60) / (12 - -60),
  },
  OUTPUT_GAIN: {
    id: 'OUTPUT_GAIN',
    name: 'Output Gain',
    type: 'float',
    min: -60,
    max: 12,
    defaultValue: 0,
    currentValue: 0,
    skew: 1,
    step: 0.1,
    unit: 'dB',
    normalizedDefault: (0 - -60) / (12 - -60),
    normalizedValue: (0 - -60) / (12 - -60),
  },
  COLOR: {
    id: 'COLOR',
    name: 'Drive',
    type: 'float',
    min: 0,
    max: 100,
    defaultValue: 40,
    currentValue: 40,
    skew: 1,
    step: 1,
    unit: '%',
    normalizedDefault: 0.4,
    normalizedValue: 0.4,
  },
  BITCRUSH: {
    id: 'BITCRUSH',
    name: 'Bitcrush',
    type: 'float',
    min: 0,
    max: 100,
    defaultValue: 0,
    currentValue: 0,
    skew: 1,
    step: 1,
    unit: '%',
    normalizedDefault: 0.0,
    normalizedValue: 0.0,
  },
  DELAY_TIME: {
    id: 'DELAY_TIME',
    name: 'Delay Time',
    type: 'float',
    min: 1,
    max: 2000,
    defaultValue: 250,
    currentValue: 250,
    skew: 1,
    step: 1,
    unit: 'ms',
    normalizedDefault: (250 - 1) / (2000 - 1),
    normalizedValue: (250 - 1) / (2000 - 1),
  },
  DELAY_MIX: {
    id: 'DELAY_MIX',
    name: 'Delay Mix',
    type: 'float',
    min: 0,
    max: 100,
    defaultValue: 30,
    currentValue: 30,
    skew: 1,
    step: 1,
    unit: '%',
    normalizedDefault: 0.3,
    normalizedValue: 0.3,
  },
  DELAY_FEEDBACK: {
    id: 'DELAY_FEEDBACK',
    name: 'Delay Feedback',
    type: 'float',
    min: 0,
    max: 95,
    defaultValue: 50,
    currentValue: 50,
    skew: 1,
    step: 1,
    unit: '%',
    normalizedDefault: 50 / 95,
    normalizedValue: 50 / 95,
  },
  CHORUS_MIX: {
    id: 'CHORUS_MIX',
    name: 'Chorus Mix',
    type: 'float',
    min: 0,
    max: 100,
    defaultValue: 75,
    currentValue: 75,
    skew: 1,
    step: 1,
    unit: '%',
    normalizedDefault: 0.75,
    normalizedValue: 0.75,
  },
  PLUGIN_BYPASS: {
    id: 'PLUGIN_BYPASS',
    name: 'Bypass',
    type: 'bool',
    defaultValue: false,
    currentValue: false,
    invert: true,
    normalizedDefault: 0.0,
    normalizedValue: 0.0,
  },
  DRIVE_ON: {
    id: 'DRIVE_ON',
    name: 'Drive Power',
    type: 'bool',
    defaultValue: true,
    currentValue: true,
    normalizedDefault: 1.0,
    normalizedValue: 1.0,
  },
  BITCRUSH_ON: {
    id: 'BITCRUSH_ON',
    name: 'Bitcrush Power',
    type: 'bool',
    defaultValue: true,
    currentValue: true,
    normalizedDefault: 1.0,
    normalizedValue: 1.0,
  },
  DELAY_ON: {
    id: 'DELAY_ON',
    name: 'Delay Power',
    type: 'bool',
    defaultValue: true,
    currentValue: true,
    normalizedDefault: 1.0,
    normalizedValue: 1.0,
  },
  CHORUS_ON: {
    id: 'CHORUS_ON',
    name: 'Chorus Power',
    type: 'bool',
    defaultValue: true,
    currentValue: true,
    normalizedDefault: 1.0,
    normalizedValue: 1.0,
  },
  DRIVE_ROUTE: {
    id: 'DRIVE_ROUTE',
    name: 'Drive Route',
    type: 'choice',
    choices: ['PRE', 'POST'],
    defaultIndex: 0,
    currentIndex: 0,
    normalizedDefault: 0.0,
    normalizedValue: 0.0,
  },
  DELAY_SYNC: {
    id: 'DELAY_SYNC',
    name: 'Delay Sync',
    type: 'bool',
    defaultValue: true,
    currentValue: true,
    normalizedDefault: 1.0,
    normalizedValue: 1.0,
  },
  CHORUS_WIDTH: {
    id: 'CHORUS_WIDTH',
    name: 'Chorus Width',
    type: 'float',
    min: 0,
    max: 100,
    defaultValue: 50,
    currentValue: 50,
    skew: 1,
    step: 1,
    unit: '%',
    normalizedDefault: 0.5,
    normalizedValue: 0.5,
  },
}

export const APVTS_ID_TO_UI_KEY: Record<string, keyof PluginState> = {
  INPUT_GAIN: 'inputGain',
  OUTPUT_GAIN: 'outputGain',
  COLOR: 'drive',
  BITCRUSH: 'bitcrush',
  DELAY_TIME: 'delayTime',
  DELAY_MIX: 'delayMix',
  DELAY_FEEDBACK: 'delayFbk',
  CHORUS_MIX: 'chorus',
  PLUGIN_BYPASS: 'engineActive',
  DRIVE_ON: 'driveOn',
  BITCRUSH_ON: 'bitcrushOn',
  DELAY_ON: 'delayOn',
  CHORUS_ON: 'chorusOn',
  DRIVE_ROUTE: 'driveRoute',
  DELAY_SYNC: 'delaySync',
  CHORUS_WIDTH: 'chorusWidth',
  CHORUS_WIDE: 'chorusWidth',
}

export const UI_KEY_TO_APVTS_ID: Record<keyof PluginState, string> = {
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
  chorusWidth: 'CHORUS_WIDTH',
}

export function normalizeValue(descriptor: ParameterDescriptor, uiValue: unknown): number {
  if (descriptor.type === 'bool') {
    const boolVal = uiValue === 'SYNC' ? true : uiValue === 'FREE' ? false : Boolean(uiValue)
    if (descriptor.invert) {
      return boolVal ? 0.0 : 1.0
    }
    return boolVal ? 1.0 : 0.0
  }

  if (descriptor.type === 'choice') {
    const choices = descriptor.choices
    if (!choices || choices.length <= 1) return 0.0
    let index = choices.indexOf(String(uiValue))
    if (index === -1) {
      if (typeof uiValue === 'number') {
        index = Math.max(0, Math.min(choices.length - 1, Math.round(uiValue)))
      } else {
        index = 0
      }
    }
    return index / (choices.length - 1)
  }

  if (descriptor.type === 'float') {
    if (typeof uiValue !== 'number' || isNaN(uiValue)) return descriptor.normalizedDefault ?? 0.0
    const clamped = Math.max(descriptor.min, Math.min(descriptor.max, uiValue))
    const range = descriptor.max - descriptor.min
    if (range <= 0) return 0.0
    let proportion = (clamped - descriptor.min) / range
    if (descriptor.skew && descriptor.skew > 0 && descriptor.skew !== 1) {
      proportion = Math.pow(proportion, descriptor.skew)
    }
    return Math.max(0.0, Math.min(1.0, proportion))
  }

  return 0.0
}

export function denormalizeValue(
  descriptor: ParameterDescriptor,
  normalizedValue: number
): unknown {
  const norm = Math.max(0.0, Math.min(1.0, isNaN(normalizedValue) ? 0.0 : normalizedValue))

  if (descriptor.type === 'bool') {
    const boolVal = norm > 0.5
    if (descriptor.id === 'DELAY_SYNC') {
      return boolVal ? 'SYNC' : 'FREE'
    }
    if (descriptor.invert) {
      return !boolVal
    }
    return boolVal
  }

  if (descriptor.type === 'choice') {
    const choices = descriptor.choices
    if (!choices || choices.length === 0) return ''
    if (choices.length === 1) return choices[0]
    const index = Math.min(choices.length - 1, Math.round(norm * (choices.length - 1)))
    return choices[index]
  }

  if (descriptor.type === 'float') {
    let proportion = norm
    if (descriptor.skew && descriptor.skew > 0 && descriptor.skew !== 1) {
      proportion = Math.pow(norm, 1.0 / descriptor.skew)
    }
    const raw = descriptor.min + proportion * (descriptor.max - descriptor.min)
    return Math.max(descriptor.min, Math.min(descriptor.max, raw))
  }

  return 0
}

export class ParameterStore {
  private descriptors: Map<string, ParameterDescriptor> = new Map()
  private uiPreferences: UIPreferences = {
    uiScale: 1.0,
    spectrumDecay: 0.25,
    skipBootSequence: false,
  }
  private listeners: Set<() => void> = new Set()
  private prefListeners: Set<(prefs: UIPreferences) => void> = new Set()

  constructor() {
    this.reset()
  }

  reset(): void {
    this.descriptors.clear()
    for (const [id, desc] of Object.entries(DEFAULT_PARAMETER_DESCRIPTORS)) {
      this.descriptors.set(id, { ...desc })
    }
    this.uiPreferences = {
      uiScale: 1.0,
      spectrumDecay: 0.25,
      skipBootSequence: false,
    }
    this.notify()
  }

  hydrate(payload: InitPayload | ParameterDescriptor[]): void {
    const extractNormVal = (obj: unknown, fallback: number): number => {
      if (obj && typeof obj === 'object') {
        if ('normalizedValue' in obj && typeof obj.normalizedValue === 'number') {
          return obj.normalizedValue
        }
        if ('value' in obj && typeof obj.value === 'number') {
          return obj.value
        }
      }
      return fallback
    }

    const processDesc = (desc: ParameterDescriptor) => {
      if (desc && desc.id) {
        const existing = this.descriptors.get(desc.id)
        const normVal = extractNormVal(desc, existing?.normalizedValue ?? 0)

        if (existing) {
          this.descriptors.set(desc.id, {
            ...existing,
            ...desc,
            normalizedValue: normVal,
          })
        } else {
          this.descriptors.set(desc.id, {
            ...desc,
            normalizedValue: normVal,
          })
        }
        this.updateParameter(desc.id, normVal)
      }
    }

    if (Array.isArray(payload)) {
      for (const desc of payload) {
        processDesc(desc)
      }
    } else if (payload && typeof payload === 'object') {
      if (Array.isArray(payload.parameters)) {
        for (const desc of payload.parameters) {
          processDesc(desc)
        }
      }
      if (payload.uiPreferences) {
        this.setUIPreferences(payload.uiPreferences)
      }
    }
    this.notify()
  }

  getDescriptor(id: string): ParameterDescriptor | undefined {
    return (
      this.descriptors.get(id) ??
      (id === 'CHORUS_WIDE' ? this.descriptors.get('CHORUS_WIDTH') : undefined)
    )
  }

  getAllDescriptors(): ParameterDescriptor[] {
    return Array.from(this.descriptors.values())
  }

  updateParameter(id: string, normalizedValue: number): void {
    const desc = this.descriptors.get(id)
    if (desc) {
      desc.normalizedValue = normalizedValue
      if (desc.type === 'float') {
        desc.currentValue = denormalizeValue(desc, normalizedValue) as number
      } else if (desc.type === 'bool') {
        desc.currentValue = denormalizeValue(desc, normalizedValue) as boolean
      } else if (desc.type === 'choice') {
        desc.currentIndex = Math.min(
          desc.choices.length - 1,
          Math.round(normalizedValue * (desc.choices.length - 1))
        )
      }
      this.notify()
    }
  }

  getUIPreferences(): UIPreferences {
    return { ...this.uiPreferences }
  }

  setUIPreferences(prefs: Partial<UIPreferences>): void {
    this.uiPreferences = { ...this.uiPreferences, ...prefs }
    this.prefListeners.forEach((cb) => cb(this.getUIPreferences()))
  }

  subscribe(listener: () => void): () => void {
    this.listeners.add(listener)
    return () => {
      this.listeners.delete(listener)
    }
  }

  subscribePreferences(listener: (prefs: UIPreferences) => void): () => void {
    this.prefListeners.add(listener)
    return () => {
      this.prefListeners.delete(listener)
    }
  }

  isDelaySynced(): boolean {
    const syncDesc = this.getDescriptor('DELAY_SYNC')
    if (!syncDesc) return true
    if (syncDesc.type === 'bool') {
      return (syncDesc.normalizedValue ?? 1) > 0.5
    }
    if (syncDesc.type === 'choice') {
      return syncDesc.choices?.[syncDesc.currentIndex ?? 0] === 'SYNC'
    }
    return true
  }

  toNormalized(uiKeyOrApvtsId: string, uiValue: unknown, isSynced?: boolean): number {
    const apvtsId = UI_KEY_TO_APVTS_ID[uiKeyOrApvtsId as keyof PluginState] ?? uiKeyOrApvtsId
    const descriptor = this.getDescriptor(apvtsId)
    if (descriptor) {
      if (apvtsId === 'DELAY_TIME') {
        const synced = isSynced !== undefined ? isSynced : this.isDelaySynced()
        if (
          typeof uiValue === 'number' &&
          synced &&
          uiValue <= DELAY_SUBDIVISIONS.length - 1
        ) {
          return Math.max(0, Math.min(1, uiValue / (DELAY_SUBDIVISIONS.length - 1)))
        }
      }
      return normalizeValue(descriptor, uiValue)
    }
    return typeof uiValue === 'number' ? uiValue : 0.0
  }

  fromNormalized(
    apvtsId: string,
    normalizedValue: number
  ): { uiKey: keyof PluginState; value: unknown } | null {
    const descriptor = this.getDescriptor(apvtsId)
    const uiKey = APVTS_ID_TO_UI_KEY[apvtsId]
    if (!descriptor || !uiKey) {
      return null
    }
    if (apvtsId === 'DELAY_TIME' && this.isDelaySynced()) {
      const step = Math.min(
        DELAY_SUBDIVISIONS.length - 1,
        Math.max(0, Math.round(normalizedValue * (DELAY_SUBDIVISIONS.length - 1)))
      )
      return { uiKey, value: step }
    }
    const value = denormalizeValue(descriptor, normalizedValue)
    return { uiKey, value }
  }

  private notify(): void {
    this.listeners.forEach((cb) => cb())
  }
}

export const parameterStore = new ParameterStore()

export function toAPVTS(
  uiKey: keyof PluginState,
  uiValue: unknown,
  isSynced?: boolean
): number {
  return parameterStore.toNormalized(uiKey, uiValue, isSynced)
}

export function fromAPVTS(
  apvtsId: string,
  normalizedValue: number
): { uiKey: keyof PluginState; value: unknown } | null {
  return parameterStore.fromNormalized(apvtsId, normalizedValue)
}
