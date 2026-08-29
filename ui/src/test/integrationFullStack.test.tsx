import { render, screen, fireEvent, act } from '@testing-library/react'
import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest'
import App from '../App'
import { MatrixFaceplate } from '../components/MatrixFaceplate'
import { Header } from '../components/Header'
import { GainMeter } from '../components/GainMeter'
import { SpectrumVisualizer } from '../components/SpectrumVisualizer'
import { createMockCanvasContext } from './mockCanvasContext'
import {
  subscribeToDspChanges,
  subscribeToDspSpectrum,
  subscribeToDspMeters,
  subscribeToUIPreferences,
  webViewDspBridge,
} from '../lib/webViewDspBridge'
import {
  parameterStore,
  toAPVTS,
  fromAPVTS,
  DEFAULT_PARAMETER_DESCRIPTORS,
  type InitPayload,
} from '../lib/parameterStore'
import { initialState, type PluginState } from '../lib/pluginState'
import { createMockDspBridge } from '../lib/dspBridge'
import { createMockJuceBackend, type MockJuceBackend } from './setup'

describe('Full-Stack Integration: Bridge Handshake, Parameter Automation, Telemetry & UI Components', () => {
  let mockBackend: MockJuceBackend

  beforeEach(() => {
    parameterStore.reset()
    parameterStore.setUIPreferences({
      uiScale: 1.0,
      spectrumDecay: 0.25,
      skipBootSequence: true,
    })
    mockBackend = createMockJuceBackend()
    window.__JUCE__ = {
      backend: mockBackend,
    }
    vi.spyOn(HTMLCanvasElement.prototype, 'getContext').mockReturnValue(
      createMockCanvasContext()
    )
  })

  afterEach(() => {
    delete window.__JUCE__
    vi.restoreAllMocks()
  })

  it('performs complete handshake: connect -> init -> parameterStore hydration -> initial state sync', () => {
    const stateUpdates: Partial<PluginState>[] = []
    const unsubscribeChanges = subscribeToDspChanges((partialState) => {
      stateUpdates.push(partialState)
    })

    // 1. Emits connect event
    expect(mockBackend.emitEvent).toHaveBeenCalledWith('connect', {})

    // 2. C++ backend responds with init payload containing all 16 parameter descriptors and uiPreferences
    const initPayload: InitPayload = {
      schemaVersion: 1,
      parameters: Object.values(DEFAULT_PARAMETER_DESCRIPTORS).map((d) => ({
        ...d,
        normalizedValue: d.normalizedDefault,
      })),
      uiPreferences: {
        uiScale: 1.25,
        spectrumDecay: 0.35,
        skipBootSequence: true,
      },
    }

    mockBackend.trigger('init', initPayload)

    // Verify parameter store is hydrated with all 16 parameters
    for (const desc of Object.values(DEFAULT_PARAMETER_DESCRIPTORS)) {
      const stored = parameterStore.getDescriptor(desc.id)
      expect(stored).toBeDefined()
      expect(stored?.id).toBe(desc.id)
    }

    // Verify UI preferences are stored
    expect(parameterStore.getUIPreferences()).toEqual({
      uiScale: 1.25,
      spectrumDecay: 0.35,
      skipBootSequence: true,
    })

    // Verify state updates were propagated
    expect(stateUpdates.length).toBeGreaterThan(0)
    const combinedState = stateUpdates.reduce((acc, s) => ({ ...acc, ...s }), {})
    expect(combinedState).toMatchObject({
      inputGain: 0,
      outputGain: 0,
      drive: 40,
      bitcrush: 0,
      chorus: 0,
      chorusWidth: 50,
      delayMix: 0,
      delayFbk: 10,
      delaySync: true,
      driveRoute: 'PRE',
      engineActive: true,
      driveOn: true,
      bitcrushOn: true,
      delayOn: true,
      chorusOn: true,
    })

    unsubscribeChanges()
  })

  it('handles bidirectional parameter automation across all 17 APVTS parameters', () => {
    const receivedUpdates: Partial<PluginState>[] = []
    const unsubscribe = subscribeToDspChanges((partial) => {
      receivedUpdates.push(partial)
    })

    // Simulated DAW host automation arriving from C++ via parameterChange
    const hostAutomations = [
      { id: 'INPUT_GAIN', value: 1.0, expectedKey: 'inputGain', expectedVal: 12 },
      { id: 'OUTPUT_GAIN', value: 0.0, expectedKey: 'outputGain', expectedVal: -60 },
      { id: 'COLOR', value: 0.85, expectedKey: 'drive', expectedVal: 85 },
      { id: 'BITCRUSH', value: 0.60, expectedKey: 'bitcrush', expectedVal: 60 },
      { id: 'DELAY_TIME_SYNC', value: 7 / 13, expectedKey: 'delayTimeSync', expectedVal: 7 },
      { id: 'DELAY_TIME_FREE', value: 0.5, expectedKey: 'delayTimeFree', expectedVal: 250.04369287197676 },
      { id: 'DELAY_MIX', value: 0.75, expectedKey: 'delayMix', expectedVal: 75 },
      { id: 'DELAY_FEEDBACK', value: 0.5, expectedKey: 'delayFbk', expectedVal: 47.5 }, // 0.5 * 95 = 47.5
      { id: 'CHORUS_MIX', value: 0.90, expectedKey: 'chorus', expectedVal: 90 },
      { id: 'CHORUS_WIDE', value: 1.0, expectedKey: 'chorusWidth', expectedVal: 100 },
      { id: 'DRIVE_ROUTE', value: 1.0, expectedKey: 'driveRoute', expectedVal: 'POST' },
      { id: 'DELAY_SYNC', value: 0.0, expectedKey: 'delaySync', expectedVal: false },
      { id: 'PLUGIN_BYPASS', value: 1.0, expectedKey: 'engineActive', expectedVal: false },
      { id: 'DRIVE_ON', value: 0.0, expectedKey: 'driveOn', expectedVal: false },
      { id: 'BITCRUSH_ON', value: 0.0, expectedKey: 'bitcrushOn', expectedVal: false },
      { id: 'DELAY_ON', value: 0.0, expectedKey: 'delayOn', expectedVal: false },
      { id: 'CHORUS_ON', value: 0.0, expectedKey: 'chorusOn', expectedVal: false },
    ]
    for (const auto of hostAutomations) {
      mockBackend.trigger('parameterChange', { id: auto.id, value: auto.value })
      const lastUpdate = receivedUpdates[receivedUpdates.length - 1]
      expect(lastUpdate).toBeDefined()
      expect((lastUpdate as Record<string, unknown>)[auto.expectedKey]).toEqual(auto.expectedVal)
    }

    // Outgoing UI interaction calling setParameter -> sends normalized float to C++
    webViewDspBridge.setParameter('COLOR', 0.95)
    expect(mockBackend.emitEvent).toHaveBeenCalledWith('setParameter', {
      id: 'COLOR',
      value: 0.95,
    })

    webViewDspBridge.setParameter('INPUT_GAIN', 0.8)
    expect(mockBackend.emitEvent).toHaveBeenCalledWith('setParameter', {
      id: 'INPUT_GAIN',
      value: 0.8,
    })

    unsubscribe()
  })

  it('receives lock-free 60 FPS telemetry streams (spectrum & meter frames)', () => {
    const receivedSpectrums: number[][] = []
    const receivedMeters: { input: number; output: number }[] = []

    const unsubSpectrum = subscribeToDspSpectrum((magnitudes) => {
      receivedSpectrums.push(magnitudes)
    })

    const unsubMeters = subscribeToDspMeters((peaks) => {
      receivedMeters.push(peaks)
    })

    // Simulate 10 frames of 60 FPS telemetry
    for (let f = 0; f < 10; ++f) {
      const dummySpectrum = new Array(80).fill(0).map((_, i) => (i + f) / 90)
      const dummyMeter = { input: 0.1 * f, output: 0.08 * f }

      mockBackend.trigger('spectrumFrame', dummySpectrum)
      mockBackend.trigger('meterFrame', dummyMeter)
    }

    expect(receivedSpectrums.length).toBe(10)
    expect(receivedSpectrums[0].length).toBe(80)
    expect(receivedSpectrums[9][0]).toBeCloseTo(9 / 90)

    expect(receivedMeters.length).toBe(10)
    expect(receivedMeters[5]).toEqual({ input: 0.5, output: 0.4 })

    unsubSpectrum()
    unsubMeters()
  })

  it('synchronizes UI preferences on runtime changes', () => {
    let currentPrefs = parameterStore.getUIPreferences()
    const unsub = subscribeToUIPreferences((prefs) => {
      currentPrefs = prefs
    })

    mockBackend.trigger('uiPreferencesChange', {
      uiScale: 1.5,
      spectrumDecay: 0.45,
      skipBootSequence: true,
    })

    expect(currentPrefs).toEqual({
      uiScale: 1.5,
      spectrumDecay: 0.45,
      skipBootSequence: true,
    })

    unsub()
  })

  it('handles two-way normalization roundtrip for all parameter types', () => {
    // Gain parameters (-60 dB to +12 dB)
    expect(toAPVTS('inputGain', 0)).toBeCloseTo(0.8333, 2)
    expect(fromAPVTS('INPUT_GAIN', 0.8333)?.value).toBeCloseTo(0, 0)
    expect(toAPVTS('outputGain', -60)).toBe(0.0)
    expect(fromAPVTS('OUTPUT_GAIN', 0.0)?.value).toBe(-60)

    // Percentage parameters (0% to 100%)
    expect(toAPVTS('drive', 50)).toBeCloseTo(0.5, 4)
    expect(fromAPVTS('COLOR', 0.5)?.value).toBe(50)
    expect(toAPVTS('bitcrush', 100)).toBe(1.0)
    expect(fromAPVTS('BITCRUSH', 1.0)?.value).toBe(100)

    // Delay time in SYNC mode (discrete 0..13) vs FREE mode (1..2000 ms)
    expect(toAPVTS('delayTimeSync', 8)).toBeCloseTo(8 / 13, 4)
    expect(fromAPVTS('DELAY_TIME_SYNC', 8 / 13)?.value).toBe(8)
    expect(toAPVTS('delayTimeFree', 250)).toBeCloseTo(0.5, 2)
    expect(fromAPVTS('DELAY_TIME_FREE', 0.0)?.value).toBe(1)
    // Boolean parameters with polarity
    expect(toAPVTS('engineActive', true)).toBe(0.0) // Inverted: PLUGIN_BYPASS = 0 when active
    expect(fromAPVTS('PLUGIN_BYPASS', 0.0)?.value).toBe(true)
    expect(toAPVTS('engineActive', false)).toBe(1.0)
    expect(fromAPVTS('PLUGIN_BYPASS', 1.0)?.value).toBe(false)

    expect(toAPVTS('driveOn', true)).toBe(1.0)
    expect(fromAPVTS('DRIVE_ON', 1.0)?.value).toBe(true)
    expect(toAPVTS('driveOn', false)).toBe(0.0)
    expect(fromAPVTS('DRIVE_ON', 0.0)?.value).toBe(false)

    // Enums
    expect(toAPVTS('driveRoute', 'PRE')).toBe(0.0)
    expect(fromAPVTS('DRIVE_ROUTE', 0.0)?.value).toBe('PRE')
    expect(toAPVTS('driveRoute', 'POST')).toBe(1.0)
    expect(fromAPVTS('DRIVE_ROUTE', 1.0)?.value).toBe('POST')

    expect(toAPVTS('delaySync', true)).toBe(1.0)
    expect(fromAPVTS('DELAY_SYNC', 1.0)?.value).toBe(true)
    expect(toAPVTS('delaySync', false)).toBe(0.0)
    expect(fromAPVTS('DELAY_SYNC', 0.0)?.value).toBe(false)
  })

  it('mounts full React App and propagates UI interactions to DSP bridge', () => {
    const bridge = createMockDspBridge()
    render(<App dspBridge={bridge} />)
    // Toggle Drive power button
    const drivePowerBtn = screen.getByTestId('power-DRV')
    expect(drivePowerBtn).toBeInTheDocument()
    fireEvent.click(drivePowerBtn)
    expect(bridge.calls).toContainEqual({ id: 'DRIVE_ON', value: 0 })

    // Toggle Delay power button
    const delayPowerBtn = screen.getByTestId('power-DLY')
    expect(delayPowerBtn).toBeInTheDocument()
    fireEvent.click(delayPowerBtn)
    expect(bridge.calls).toContainEqual({ id: 'DELAY_ON', value: 0 })

    // Toggle master bypass via header LED
    const bypassBtn = screen.getByRole('button', { name: /Disable main DSP/i })
    expect(bypassBtn).toBeInTheDocument()
    fireEvent.click(bypassBtn)
    expect(bridge.calls).toContainEqual({ id: 'PLUGIN_BYPASS', value: 1 })
  })

  it('mounts MatrixFaceplate as controlled component and responds to prop changes', () => {
    const onChange = vi.fn()
    const { rerender } = render(<MatrixFaceplate state={initialState} onChange={onChange} />)

    // Verify module section headings
    expect(screen.getByText('DRV')).toBeInTheDocument()
    expect(screen.getByText('BCR')).toBeInTheDocument()
    expect(screen.getByText('DLY')).toBeInTheDocument()
    expect(screen.getByText('CHR')).toBeInTheDocument()

    // Test Delay sync toggle button
    const freeButton = screen.getByRole('button', { name: 'FREE' })
    fireEvent.click(freeButton)
    expect(onChange).toHaveBeenCalledWith({ delaySync: false })
    // Rerender with powered off state
    const poweredOffState: PluginState = {
      ...initialState,
      driveOn: false,
      bitcrushOn: false,
      delayOn: false,
      chorusOn: false,
    }
    rerender(<MatrixFaceplate state={poweredOffState} onChange={onChange} />)

    // Verify all 4 power buttons show off state
    const powerButtons = screen.getAllByRole('button', { name: /Turn on .* module/i })
    expect(powerButtons.length).toBe(4)
    powerButtons.forEach((btn) => {
      expect(btn).toHaveAttribute('aria-pressed', 'false')
    })
  })
  it('mounts Header, GainMeter, and SpectrumVisualizer under simulated telemetry updates', () => {
    const onToggleBypass = vi.fn()
    render(
      <Header
        engineActive={true}
        onToggleBypass={onToggleBypass}
      />
    )

    expect(screen.getByText('SYNTHORTION')).toBeInTheDocument()

    const { container: meterContainer } = render(
      <GainMeter label="IN" active={true} channel="input" />
    )
    expect(meterContainer.querySelector('canvas')).toBeInTheDocument()

    const { container: spectrumContainer } = render(
      <SpectrumVisualizer
        active={true}
      />
    )
    expect(spectrumContainer.querySelector('canvas')).toBeInTheDocument()
  })

  it('executes full preset lifecycle flow: search -> load -> parameter change -> dirty flag -> save -> catalog update', () => {
    const initialPresets = [
      {
        id: 'factory://Init/00_Default_Init',
        name: 'Default Init',
        category: 'Init',
        author: 'Synthortion Core',
        description: 'Clean initialization state.',
        tags: ['Init', 'Default'],
        isFactory: true,
        filePath: 'factory://Init/00_Default_Init.synthortionpreset',
        favorite: false,
        createdAt: '2026-08-28T12:00:00Z',
        modifiedAt: '2026-08-28T12:00:00Z',
      },
      {
        id: 'factory://Bass/01_Sub_Destroyer',
        name: 'Sub Destroyer',
        category: 'Bass',
        author: 'Synthortion Core',
        description: 'Heavy sub bass.',
        tags: ['Bass', 'Sub'],
        isFactory: true,
        filePath: 'factory://Bass/01_Sub_Destroyer.synthortionpreset',
        favorite: false,
        createdAt: '2026-08-28T12:00:00Z',
        modifiedAt: '2026-08-28T12:00:00Z',
      },
      {
        id: 'factory://Bass/02_Acid_Crush',
        name: 'Acid Crush',
        category: 'Bass',
        author: 'Synthortion Core',
        description: 'Resonant 303-style bite with coupled downsampling and warm overdrive.',
        tags: ['Bass', 'Acid', '303', 'Bitcrush'],
        isFactory: true,
        filePath: 'factory://Bass/02_Acid_Crush.synthortionpreset',
        favorite: false,
        createdAt: '2026-08-28T12:00:00Z',
        modifiedAt: '2026-08-28T12:00:00Z',
      },
    ]

    // 1. Render Full React App (mounts bridge listener and emits connect)
    render(<App />)
    expect(mockBackend.emitEvent).toHaveBeenCalledWith('connect', {})

    // 2. Initial Handshake from backend carrying initial presets and parameters
    const initPayload: InitPayload = {
      schemaVersion: 1,
      parameters: Object.values(DEFAULT_PARAMETER_DESCRIPTORS).map((d) => ({
        ...d,
        normalizedValue: d.normalizedDefault,
      })),
      uiPreferences: {
        uiScale: 1.0,
        spectrumDecay: 0.25,
        skipBootSequence: true,
      },
      presets: initialPresets,
      activePresetId: 'factory://Init/00_Default_Init',
      activePresetName: 'Default Init',
      activePresetCategory: 'Init',
      isFactoryPreset: true,
    }
    act(() => {
      mockBackend.trigger('init', initPayload)
    })

    // Verify Header shows Default Init without dirty asterisk
    expect(screen.getByRole('button', { name: /Preset: INIT: DEFAULT INIT/i })).toBeInTheDocument()
    // 3. Open PresetBrowserModal by clicking PRESETS button
    const presetsBtn = screen.getByRole('button', { name: 'PRESETS' })
    fireEvent.click(presetsBtn)

    // Verify Modal is open
    expect(screen.getByText('PRESET BROWSER')).toBeInTheDocument()

    // 4. Search for "acid"
    const searchInput = screen.getByPlaceholderText(/SEARCH PRESETS/i)
    fireEvent.change(searchInput, { target: { value: 'acid' } })

    // Verify table lists "Acid Crush"
    const acidOption = screen.getByRole('option', { name: /Acid Crush/i })
    expect(acidOption).toBeInTheDocument()

    // 5. Select preset and click [ LOAD ]
    fireEvent.click(acidOption)
    const loadBtn = screen.getByRole('button', { name: /Load preset/i })
    fireEvent.click(loadBtn)
    // Verify loadPreset IPC event sent to backend
    expect(mockBackend.emitEvent).toHaveBeenCalledWith('loadPreset', {
      id: 'factory://Bass/02_Acid_Crush',
    })
    // Backend responds with parameter changes and presetLoaded confirmation for Acid Crush
    act(() => {
      mockBackend.trigger('parameterChange', { id: 'COLOR', value: 0.70 })
      mockBackend.trigger('parameterChange', { id: 'BITCRUSH', value: 0.35 })
      mockBackend.trigger('presetLoaded', {
        id: 'factory://Bass/02_Acid_Crush',
        name: 'Acid Crush',
        category: 'Bass',
        isFactory: true,
        isDirty: false,
      })
    })
    expect(screen.queryByText('PRESET BROWSER')).not.toBeInTheDocument()
    expect(screen.getByRole('button', { name: /Preset: BASS: ACID CRUSH/i })).toBeInTheDocument()
    expect(parameterStore.getActivePresetId()).toBe('factory://Bass/02_Acid_Crush')
    expect(parameterStore.getIsPresetDirty()).toBe(false)

    // 6. User modifies parameter -> dirty state '*' activates
    act(() => {
      mockBackend.trigger('parameterChange', { id: 'COLOR', value: 0.95 })
    })
    expect(parameterStore.getIsPresetDirty()).toBe(true)
    expect(screen.getByRole('button', { name: /Preset: BASS: ACID CRUSH \*/i })).toBeInTheDocument()
    // 7. Click SAVE button to open SavePresetModal
    const saveBtn = screen.getByRole('button', { name: 'SAVE' })
    fireEvent.click(saveBtn)
    expect(screen.getByRole('heading', { name: 'SAVE PRESET' })).toBeInTheDocument()

    // Enter preset details
    const nameInput = screen.getByLabelText(/Preset Name/i)
    fireEvent.change(nameInput, { target: { value: 'My Acid 303' } })

    const descInput = screen.getByLabelText(/Description/i)
    fireEvent.change(descInput, { target: { value: 'Custom resonant acid sound' } })

    const tagsInput = screen.getByLabelText(/Tags/i)
    fireEvent.change(tagsInput, { target: { value: 'Acid, 303, Custom' } })

    // Submit Save
    const submitSaveBtn = screen.getByRole('button', { name: 'Save Preset' })
    fireEvent.click(submitSaveBtn)

    expect(mockBackend.emitEvent).toHaveBeenCalledWith('savePreset', {
      name: 'My Acid 303',
      category: 'Bass',
      author: 'User',
      description: 'Custom resonant acid sound',
      tags: ['Acid', '303', 'Custom'],
      allowOverwrite: false,
    })

    // 8. Backend responds with save result, updated catalog, and preset loaded
    const newPresetHeader = {
      id: 'user://Bass/My Acid 303',
      name: 'My Acid 303',
      category: 'Bass',
      author: 'User',
      description: 'Custom resonant acid sound',
      tags: ['Acid', '303', 'Custom'],
      isFactory: false,
      filePath: 'C:/Users/paolo/AppData/Roaming/Synthortion/Presets/Bass/My Acid 303.synthortionpreset',
      favorite: false,
      createdAt: '2026-08-29T12:00:00Z',
      modifiedAt: '2026-08-29T12:00:00Z',
    }
    act(() => {
      mockBackend.trigger('presetOperationResult', {
        success: true,
        operation: 'save',
        message: 'Preset saved successfully',
      })
      mockBackend.trigger('presetListUpdated', {
        presets: [...initialPresets, newPresetHeader],
        activePresetId: 'user://Bass/My Acid 303',
      })
      mockBackend.trigger('presetLoaded', {
        id: 'user://Bass/My Acid 303',
        name: 'My Acid 303',
        category: 'Bass',
        isFactory: false,
        isDirty: false,
      })
    })

    // Verify Save modal is closed and Header displays new preset name without dirty asterisk
    expect(screen.queryByRole('heading', { name: 'SAVE PRESET' })).not.toBeInTheDocument()
    expect(screen.getByRole('button', { name: /Preset: BASS: MY ACID 303/i })).toBeInTheDocument()
    expect(parameterStore.getIsPresetDirty()).toBe(false)
    // 9. Re-open PresetBrowserModal and verify new user preset is in catalog
    fireEvent.click(screen.getByRole('button', { name: 'PRESETS' }))
    expect(screen.getByRole('option', { name: /My Acid 303/i })).toBeInTheDocument()
  })

  it('handles disk permission failure gracefully and dispatches toast without UI crash', () => {
    render(<App />)

    // Simulate permission denied error event from C++ backend
    mockBackend.trigger('presetOperationResult', {
      success: false,
      operation: 'save',
      errorCode: 'PermissionDenied',
      message: 'Access is denied: could not write preset to disk',
    })

    // Verify store has toast message
    expect(parameterStore.getPresetOperationToast()).toBe('Access is denied: could not write preset to disk')

    // App remains mounted and interactive
    expect(screen.getByText('SYNTHORTION')).toBeInTheDocument()
  })

  it('cycles through category presets via Header stepper buttons', () => {
    const presets = [
      {
        id: 'factory://Bass/01_Sub_Destroyer',
        name: 'Sub Destroyer',
        category: 'Bass',
        author: 'Synthortion Core',
        description: 'Sub bass.',
        tags: ['Bass'],
        isFactory: true,
        filePath: '',
        favorite: false,
        createdAt: '',
        modifiedAt: '',
      },
      {
        id: 'factory://Bass/02_Acid_Crush',
        name: 'Acid Crush',
        category: 'Bass',
        author: 'Synthortion Core',
        description: 'Acid bass.',
        tags: ['Bass'],
        isFactory: true,
        filePath: '',
        favorite: false,
        createdAt: '',
        modifiedAt: '',
      },
    ]
    render(<App />)

    act(() => {
      mockBackend.trigger('init', {
        schemaVersion: 1,
        parameters: Object.values(DEFAULT_PARAMETER_DESCRIPTORS).map((d) => ({
          ...d,
          normalizedValue: d.normalizedDefault,
        })),
        uiPreferences: {
          uiScale: 1.0,
          spectrumDecay: 0.25,
          skipBootSequence: true,
        },
        presets,
        activePresetId: 'factory://Bass/01_Sub_Destroyer',
        activePresetName: 'Sub Destroyer',
        activePresetCategory: 'Bass',
        isFactoryPreset: true,
        isPresetDirty: false,
      })
    })

    expect(screen.getByRole('button', { name: /Preset: BASS: SUB DESTROYER/i })).toBeInTheDocument()
    // Click Next preset stepper >
    const nextBtn = screen.getByRole('button', { name: 'Next preset' })
    fireEvent.click(nextBtn)

    expect(mockBackend.emitEvent).toHaveBeenCalledWith('loadPreset', {
      id: 'factory://Bass/02_Acid_Crush',
    })

    act(() => {
      mockBackend.trigger('presetLoaded', {
        id: 'factory://Bass/02_Acid_Crush',
        name: 'Acid Crush',
        category: 'Bass',
        isFactory: true,
        isDirty: false,
      })
    })

    expect(screen.getByRole('button', { name: /Preset: BASS: ACID CRUSH/i })).toBeInTheDocument()
    const prevBtn = screen.getByRole('button', { name: 'Previous preset' })
    fireEvent.click(prevBtn)

    expect(mockBackend.emitEvent).toHaveBeenCalledWith('loadPreset', {
      id: 'factory://Bass/01_Sub_Destroyer',
    })
  })
})
