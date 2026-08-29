import { render, screen, fireEvent } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach } from 'vitest'
import { SavePresetModal } from '../components/SavePresetModal'
import { parameterStore, type PresetHeader } from '../lib/parameterStore'

const MOCK_PRESETS: PresetHeader[] = [
  {
    id: 'factory://Lead/03_Cyber_Neon',
    name: 'Cyber Neon',
    category: 'Lead',
    author: 'Synthortion Core',
    description: 'Bright resonant lead with sync delay and wide chorus.',
    tags: ['Lead', 'Cyber', 'Neon', 'Bright'],
    isFactory: true,
    filePath: 'factory://Lead/03_Cyber_Neon.synthortionpreset',
    favorite: false,
    createdAt: '2026-08-28T12:00:00Z',
    modifiedAt: '2026-08-28T12:00:00Z',
  },
  {
    id: 'user://Bass/Dark_Matter',
    name: 'Dark Matter',
    category: 'Bass',
    author: 'User Producer',
    description: 'Custom dark distorted bass patch.',
    tags: ['Bass', 'Dark'],
    isFactory: false,
    filePath: '%APPDATA%/Synthortion/Presets/Bass/Dark_Matter.synthortionpreset',
    favorite: false,
    createdAt: '2026-08-29T10:00:00Z',
    modifiedAt: '2026-08-29T10:00:00Z',
  },
]

describe('SavePresetModal', () => {
  beforeEach(() => {
    parameterStore.reset()
    parameterStore.setPresetCatalog(MOCK_PRESETS, 'factory://Lead/03_Cyber_Neon')
  })

  it('renders nothing when isOpen is false', () => {
    const { container } = render(
      <SavePresetModal isOpen={false} onClose={() => {}} />
    )
    expect(container).toBeEmptyDOMElement()
  })

  it('renders Cartesian modal frame, title, inputs, and close button when isOpen is true', () => {
    render(<SavePresetModal isOpen={true} onClose={() => {}} />)

    const dialog = screen.getByRole('dialog', { name: 'Save Preset' })
    expect(dialog).toBeInTheDocument()
    expect(screen.getByRole('heading', { name: 'SAVE PRESET' })).toBeInTheDocument()
    expect(screen.getByLabelText(/Preset Name/i)).toBeInTheDocument()
    expect(screen.getByRole('combobox', { name: 'Category' })).toBeInTheDocument()
    expect(screen.getByLabelText('Author')).toBeInTheDocument()
    expect(screen.getByLabelText('Description')).toBeInTheDocument()
    expect(screen.getByLabelText('Tags')).toBeInTheDocument()
    expect(screen.getByRole('button', { name: /close|esc/i })).toBeInTheDocument()
  })

  it('calls onClose when close button is clicked or Escape key is pressed', () => {
    const handleClose = vi.fn()
    render(<SavePresetModal isOpen={true} onClose={handleClose} />)

    const closeBtn = screen.getByRole('button', { name: /close|esc/i })
    fireEvent.click(closeBtn)
    expect(handleClose).toHaveBeenCalledTimes(1)

    fireEvent.keyDown(window, { key: 'Escape' })
    expect(handleClose).toHaveBeenCalledTimes(2)
  })

  it('disables save button when preset name is empty', () => {
    render(<SavePresetModal isOpen={true} onClose={() => {}} />)

    const nameInput = screen.getByLabelText(/Preset Name/i)
    fireEvent.change(nameInput, { target: { value: '   ' } })

    const saveBtn = screen.getByRole('button', { name: 'Save Preset' })
    expect(saveBtn).toBeDisabled()
    expect(screen.getByText(/NAME REQUIRED/i)).toBeInTheDocument()
  })

  it('validates against illegal filename characters (\\ / : * ? " < > |)', () => {
    render(<SavePresetModal isOpen={true} onClose={() => {}} />)

    const nameInput = screen.getByLabelText(/Preset Name/i)
    const saveBtn = screen.getByRole('button', { name: 'Save Preset' })

    // Type illegal character '/'
    fireEvent.change(nameInput, { target: { value: 'Bad/Name' } })
    expect(saveBtn).toBeDisabled()
    expect(screen.getByText(/ILLEGAL CHARACTERS/i)).toBeInTheDocument()

    // Type illegal character '*'
    fireEvent.change(nameInput, { target: { value: 'Lead*Star' } })
    expect(saveBtn).toBeDisabled()
    expect(screen.getByText(/ILLEGAL CHARACTERS/i)).toBeInTheDocument()

    // Type illegal character '?'
    fireEvent.change(nameInput, { target: { value: 'IsItLead?' } })
    expect(saveBtn).toBeDisabled()
    expect(screen.getByText(/ILLEGAL CHARACTERS/i)).toBeInTheDocument()

    // Valid sanitized name
    fireEvent.change(nameInput, { target: { value: 'Valid Lead Name' } })
    expect(saveBtn).not.toBeDisabled()
    expect(screen.queryByText(/ILLEGAL CHARACTERS/i)).not.toBeInTheDocument()
  })

  it('validates maximum length of 32 characters', () => {
    render(<SavePresetModal isOpen={true} onClose={() => {}} />)

    const nameInput = screen.getByLabelText(/Preset Name/i)
    const saveBtn = screen.getByRole('button', { name: 'Save Preset' })

    // 33 characters
    fireEvent.change(nameInput, {
      target: { value: 'This Preset Name Exceeds Thirty Two Characters' },
    })
    expect(saveBtn).toBeDisabled()
    expect(screen.getByText(/MAX 32 CHARACTERS/i)).toBeInTheDocument()
  })

  it('saves new user preset and calls parameterStore.savePreset', () => {
    const saveSpy = vi.fn()
    parameterStore.setBridge({ setParameter: () => {}, savePreset: saveSpy })
    const handleClose = vi.fn()

    render(<SavePresetModal isOpen={true} onClose={handleClose} />)

    fireEvent.change(screen.getByLabelText(/Preset Name/i), {
      target: { value: 'Hyper Lead' },
    })
    fireEvent.change(screen.getByRole('combobox', { name: 'Category' }), {
      target: { value: 'Lead' },
    })
    fireEvent.change(screen.getByLabelText('Author'), {
      target: { value: 'CyberProducer' },
    })
    fireEvent.change(screen.getByLabelText('Description'), {
      target: { value: 'Massive aggressive lead sound.' },
    })
    fireEvent.change(screen.getByLabelText('Tags'), {
      target: { value: 'Lead, Aggressive, Sync' },
    })

    const saveBtn = screen.getByRole('button', { name: 'Save Preset' })
    fireEvent.click(saveBtn)

    expect(saveSpy).toHaveBeenCalledWith({
      name: 'Hyper Lead',
      category: 'Lead',
      author: 'CyberProducer',
      description: 'Massive aggressive lead sound.',
      tags: ['Lead', 'Aggressive', 'Sync'],
      allowOverwrite: false,
    })
    expect(handleClose).toHaveBeenCalled()
  })

  it('shows overwrite warning when saving with existing preset name in the category', () => {
    const saveSpy = vi.fn()
    parameterStore.setBridge({ setParameter: () => {}, savePreset: saveSpy })
    const handleClose = vi.fn()

    render(<SavePresetModal isOpen={true} onClose={handleClose} />)

    // Type duplicate name 'Cyber Neon' in category 'Lead'
    fireEvent.change(screen.getByLabelText(/Preset Name/i), {
      target: { value: 'Cyber Neon' },
    })
    fireEvent.change(screen.getByRole('combobox', { name: 'Category' }), {
      target: { value: 'Lead' },
    })

    const initialSaveBtn = screen.getByRole('button', { name: 'Save Preset' })
    fireEvent.click(initialSaveBtn)

    // Save should NOT be called yet; overwrite warning should appear
    expect(saveSpy).not.toHaveBeenCalled()
    expect(screen.getByText(/OVERWRITE EXISTING PRESET\?/i)).toBeInTheDocument()

    // Cancel overwrite
    const cancelBtn = screen.getByRole('button', { name: /Cancel overwrite/i })
    fireEvent.click(cancelBtn)
    expect(screen.queryByText(/OVERWRITE EXISTING PRESET\?/i)).not.toBeInTheDocument()
    expect(saveSpy).not.toHaveBeenCalled()

    // Click save again and confirm overwrite
    const reSaveBtn = screen.getByRole('button', { name: 'Save Preset' })
    fireEvent.click(reSaveBtn)
    const confirmBtn = screen.getByRole('button', { name: /Confirm overwrite/i })
    fireEvent.click(confirmBtn)

    expect(saveSpy).toHaveBeenCalledWith({
      name: 'Cyber Neon',
      category: 'Lead',
      author: 'User',
      description: '',
      tags: [],
      allowOverwrite: true,
    })
    expect(handleClose).toHaveBeenCalled()
  })

  it('renders Cartesian crosshairs (+) and no repeated character loops in header', () => {
    render(<SavePresetModal isOpen={true} onClose={() => {}} />)

    const crosshairs = screen.getAllByText('+')
    expect(crosshairs.length).toBeGreaterThanOrEqual(4)

    const dialog = screen.getByRole('dialog', { name: 'Save Preset' })
    const headings = dialog.querySelectorAll('h1, h2, h3, header')
    headings.forEach((heading) => {
      expect(heading.textContent).not.toMatch(/─{2,}/)
    })
  })
})
