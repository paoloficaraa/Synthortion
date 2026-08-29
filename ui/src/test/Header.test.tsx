import { render, screen, fireEvent, act, within } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach } from 'vitest'
import { Header } from '../components/Header'
import { parameterStore } from '../lib/parameterStore'

describe('Header', () => {
  beforeEach(() => {
    parameterStore.reset()
  })

  it('renders the brand, bypass LED, preset readout and BROWSE/SAVE actions', () => {
    render(<Header engineActive onToggleBypass={() => {}} />)

    expect(screen.getByRole('heading', { name: 'SYNTHORTION' })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'BROWSE' })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'SAVE' })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'Previous preset' })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'Next preset' })).toBeInTheDocument()
    expect(
      screen.getByRole('button', { name: 'Disable main DSP' })
    ).toBeInTheDocument()
  })

  it('flips the bypass LED and reports the new engine state', () => {
    const onToggleBypass = vi.fn()
    render(<Header engineActive onToggleBypass={onToggleBypass} />)

    const bypass = screen.getByRole('button', { name: 'Disable main DSP' })
    expect(bypass).toHaveAttribute('aria-pressed', 'true')

    fireEvent.click(bypass)

    expect(onToggleBypass).toHaveBeenCalledWith(false)
  })

  it('renders the active category and name in uppercase formatted as [ CATEGORY: NAME ]', () => {
    parameterStore.setActivePreset({
      id: 'factory://Lead/03_Cyber_Neon',
      name: 'Cyber Neon',
      category: 'Lead',
      isDirty: false,
    })

    render(<Header engineActive onToggleBypass={() => {}} />)

    const readout = screen.getByRole('button', { name: /Preset: LEAD: CYBER NEON/i })
    expect(readout).toBeInTheDocument()
    expect(readout.textContent).toContain('[ LEAD: CYBER NEON ]')
    expect(readout.textContent).not.toContain('*')

    const cursor = within(readout).getByText('▊')
    expect(cursor).toHaveAttribute('aria-hidden', 'true')
    expect(cursor).toHaveClass('block-cursor')
  })

  it('renders dirty asterisk * when parameter is edited and clears on preset reload', () => {
    parameterStore.setActivePreset({
      id: 'factory://Lead/03_Cyber_Neon',
      name: 'Cyber Neon',
      category: 'Lead',
      isDirty: false,
    })

    render(<Header engineActive onToggleBypass={() => {}} />)

    const readout = screen.getByRole('button', { name: /Preset: LEAD: CYBER NEON/i })
    expect(readout.textContent).not.toContain('*')

    // Simulate parameter edit triggering dirty state
    act(() => {
      parameterStore.updateParameter('COLOR', 0.95)
    })

    expect(readout.textContent).toContain('[ LEAD: CYBER NEON * ]')

    // Reload / reset active preset clears dirty state
    act(() => {
      parameterStore.setActivePreset({
        id: 'factory://Lead/03_Cyber_Neon',
        name: 'Cyber Neon',
        category: 'Lead',
        isDirty: false,
      })
    })

    expect(readout.textContent).toContain('[ LEAD: CYBER NEON ]')
    expect(readout.textContent).not.toContain('*')
  })

  it('clicking on the central preset readout calls onOpenPresets', () => {
    const onOpenPresets = vi.fn()
    render(
      <Header
        engineActive
        onToggleBypass={() => {}}
        onOpenPresets={onOpenPresets}
      />
    )

    const readout = screen.getByRole('button', { name: /Preset:/i })
    fireEvent.click(readout)

    expect(onOpenPresets).toHaveBeenCalledTimes(1)
  })
  it('invokes parameterStore.stepPreset when < and > stepper buttons are clicked', () => {
    const stepPresetSpy = vi.spyOn(parameterStore, 'stepPreset')
    render(<Header engineActive onToggleBypass={() => {}} />)

    const prevButton = screen.getByRole('button', { name: 'Previous preset' })
    const nextButton = screen.getByRole('button', { name: 'Next preset' })

    fireEvent.click(prevButton)
    expect(stepPresetSpy).toHaveBeenCalledWith('prev')

    fireEvent.click(nextButton)
    expect(stepPresetSpy).toHaveBeenCalledWith('next')

    stepPresetSpy.mockRestore()
  })

  it('invokes custom onStepPrev and onStepNext callbacks when provided', () => {
    const onStepPrev = vi.fn()
    const onStepNext = vi.fn()
    render(
      <Header
        engineActive
        onToggleBypass={() => {}}
        onStepPrev={onStepPrev}
        onStepNext={onStepNext}
      />
    )

    fireEvent.click(screen.getByRole('button', { name: 'Previous preset' }))
    expect(onStepPrev).toHaveBeenCalledTimes(1)

    fireEvent.click(screen.getByRole('button', { name: 'Next preset' }))
    expect(onStepNext).toHaveBeenCalledTimes(1)
  })

  it('cycles category presets correctly and triggers bridge loadPreset', () => {
    const mockLoadPreset = vi.fn()
    parameterStore.setBridge({
      setParameter: vi.fn(),
      loadPreset: mockLoadPreset,
    })

    parameterStore.setPresetCatalog(
      [
        {
          id: 'factory://Lead/03_Cyber_Neon',
          name: 'Cyber Neon',
          category: 'Lead',
          author: 'Synthortion Core',
          description: 'Cyber lead.',
          tags: ['Lead', 'Cyber'],
          isFactory: true,
          filePath: '',
          favorite: false,
          createdAt: '2026-08-28T12:00:00Z',
          modifiedAt: '2026-08-28T12:00:00Z',
        },
        {
          id: 'factory://Lead/04_Vintage_Lead',
          name: 'Vintage Lead',
          category: 'Lead',
          author: 'Synthortion Core',
          description: 'Vintage lead.',
          tags: ['Lead', 'Vintage'],
          isFactory: true,
          filePath: '',
          favorite: false,
          createdAt: '2026-08-28T12:00:00Z',
          modifiedAt: '2026-08-28T12:00:00Z',
        },
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
          createdAt: '2026-08-28T12:00:00Z',
          modifiedAt: '2026-08-28T12:00:00Z',
        },
      ],
      'factory://Lead/03_Cyber_Neon'
    )

    render(<Header engineActive onToggleBypass={() => {}} />)

    const prevButton = screen.getByRole('button', { name: 'Previous preset' })
    const nextButton = screen.getByRole('button', { name: 'Next preset' })

    // Next should load 04_Vintage_Lead in the Lead category
    fireEvent.click(nextButton)
    expect(mockLoadPreset).toHaveBeenLastCalledWith('factory://Lead/04_Vintage_Lead')

    // Prev should wrap to 04_Vintage_Lead from 03_Cyber_Neon
    mockLoadPreset.mockClear()
    fireEvent.click(prevButton)
    expect(mockLoadPreset).toHaveBeenLastCalledWith('factory://Lead/04_Vintage_Lead')
  })
  it('clicking [ BROWSE ] and [ SAVE ] buttons invokes respective modal callbacks', () => {
    const onOpenPresets = vi.fn()
    const onOpenSave = vi.fn()

    render(
      <Header
        engineActive
        onToggleBypass={() => {}}
        onOpenPresets={onOpenPresets}
        onOpenSave={onOpenSave}
      />
    )

    fireEvent.click(screen.getByRole('button', { name: 'BROWSE' }))
    expect(onOpenPresets).toHaveBeenCalledTimes(1)

    fireEvent.click(screen.getByRole('button', { name: 'SAVE' }))
    expect(onOpenSave).toHaveBeenCalledTimes(1)
  })

  it('supports keyboard navigation (ArrowLeft, ArrowRight, [, ], Enter, Space) when readout is focused', () => {
    const onOpenPresets = vi.fn()
    const onStepPrev = vi.fn()
    const onStepNext = vi.fn()

    render(
      <Header
        engineActive
        onToggleBypass={() => {}}
        onOpenPresets={onOpenPresets}
        onStepPrev={onStepPrev}
        onStepNext={onStepNext}
      />
    )

    const readout = screen.getByRole('button', { name: /Preset:/i })

    fireEvent.keyDown(readout, { key: 'ArrowLeft' })
    expect(onStepPrev).toHaveBeenCalledTimes(1)

    fireEvent.keyDown(readout, { key: '[' })
    expect(onStepPrev).toHaveBeenCalledTimes(2)

    fireEvent.keyDown(readout, { key: 'ArrowRight' })
    expect(onStepNext).toHaveBeenCalledTimes(1)

    fireEvent.keyDown(readout, { key: ']' })
    expect(onStepNext).toHaveBeenCalledTimes(2)

    fireEvent.keyDown(readout, { key: 'Enter' })
    expect(onOpenPresets).toHaveBeenCalledTimes(1)

    fireEvent.keyDown(readout, { key: ' ' })
    expect(onOpenPresets).toHaveBeenCalledTimes(2)
  })

  it('renders [ BROWSE ] [ SAVE ] bracket buttons with hover inversion', () => {
    render(<Header engineActive onToggleBypass={() => {}} />)

    for (const label of ['BROWSE', 'SAVE']) {
      const button = screen.getByRole('button', { name: label })
      expect(button.textContent).toContain(`[ ${label} ]`)
      expect(button.querySelectorAll('[aria-hidden="true"]')).toHaveLength(2)
      expect(button).toHaveClass('hover:bg-fg', 'hover:text-bg', 'hover:border-fg')
      expect(button).toHaveClass('border', 'font-mono')
    }
  })

  it('renders Cartesian 1px structural divider without repeated character loops', () => {
    const { container } = render(<Header engineActive onToggleBypass={() => {}} />)

    const header = container.querySelector('header')
    expect(header).toHaveClass('border-b', 'border-grid-rule')

    const allText = container.textContent ?? ''
    expect(allText).not.toMatch(/─{2,}/)
  })

  it('renders with fixed 50px height allocation for DAW window scaling', () => {
    const { container } = render(<Header engineActive onToggleBypass={() => {}} />)
    const header = container.querySelector('header')
    expect(header).toHaveClass('h-[50px]', 'shrink-0')
  })
})
