import { render, screen, fireEvent, within } from '@testing-library/react'
import { describe, it, expect, vi, beforeEach } from 'vitest'
import { PresetBrowserModal } from '../components/PresetBrowserModal'
import { parameterStore, type PresetHeader } from '../lib/parameterStore'

const MOCK_PRESETS: PresetHeader[] = [
  {
    id: 'factory://Init/00_Default_Init',
    name: 'Default Init',
    category: 'Init',
    author: 'Synthortion Core',
    description: 'Clean default starting point with neutral parameters.',
    tags: ['Init', 'Default', 'Clean'],
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
    description: 'Massive saturated sub bass with heavy bitcrushing.',
    tags: ['Bass', 'Sub', 'Crush', 'Heavy'],
    isFactory: true,
    filePath: 'factory://Bass/01_Sub_Destroyer.synthortionpreset',
    favorite: true,
    createdAt: '2026-08-28T12:00:00Z',
    modifiedAt: '2026-08-28T12:00:00Z',
  },
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
    tags: ['Bass', 'Dark', 'Custom'],
    isFactory: false,
    filePath: '%APPDATA%/Synthortion/Presets/Bass/Dark_Matter.synthortionpreset',
    favorite: false,
    createdAt: '2026-08-29T10:00:00Z',
    modifiedAt: '2026-08-29T10:00:00Z',
  },
]

describe('PresetBrowserModal', () => {
  beforeEach(() => {
    parameterStore.reset()
    parameterStore.setPresetCatalog(MOCK_PRESETS, 'factory://Init/00_Default_Init')
  })

  it('renders nothing when isOpen is false', () => {
    const { container } = render(
      <PresetBrowserModal isOpen={false} onClose={() => {}} />
    )
    expect(container).toBeEmptyDOMElement()
  })

  it('renders Cartesian modal frame and close button when isOpen is true', () => {
    render(<PresetBrowserModal isOpen={true} onClose={() => {}} />)

    const dialog = screen.getByRole('dialog', { name: 'Preset Browser' })
    expect(dialog).toBeInTheDocument()
    expect(screen.getByText(/PRESET BROWSER/i)).toBeInTheDocument()
    expect(screen.getByRole('button', { name: /close/i })).toBeInTheDocument()
  })

  it('calls onClose when close button is clicked or Escape key is pressed', () => {
    const handleClose = vi.fn()
    render(<PresetBrowserModal isOpen={true} onClose={handleClose} />)

    const closeBtn = screen.getByRole('button', { name: /close/i })
    fireEvent.click(closeBtn)
    expect(handleClose).toHaveBeenCalledTimes(1)

    fireEvent.keyDown(window, { key: 'Escape' })
    expect(handleClose).toHaveBeenCalledTimes(2)
  })

  it('renders category navigation list with preset counts', () => {
    render(<PresetBrowserModal isOpen={true} onClose={() => {}} />)

    // Standard categories
    expect(screen.getByRole('button', { name: /^Category ALL/i })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: /^Category INIT/i })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: /^Category BASS/i })).toBeInTheDocument()
    expect(screen.getByRole('button', { name: /^Category LEAD/i })).toBeInTheDocument()

    // Counts: ALL = 4, BASS = 2, LEAD = 1, INIT = 1
    const allCategoryBtn = screen.getByRole('button', { name: /^Category ALL/i })
    expect(allCategoryBtn).toHaveTextContent('4')

    const bassCategoryBtn = screen.getByRole('button', { name: /^Category BASS/i })
    expect(bassCategoryBtn).toHaveTextContent('2')
  })

  it('filters presets by category when a category button is clicked', () => {
    render(<PresetBrowserModal isOpen={true} onClose={() => {}} />)
    const mainTable = screen.getByRole('main')

    // Initial ALL shows all 4 presets
    expect(within(mainTable).getByText(/Default Init/i)).toBeInTheDocument()
    expect(within(mainTable).getByText(/Sub Destroyer/i)).toBeInTheDocument()
    expect(within(mainTable).getByText(/Cyber Neon/i)).toBeInTheDocument()
    expect(within(mainTable).getByText(/Dark Matter/i)).toBeInTheDocument()

    // Click BASS category
    fireEvent.click(screen.getByRole('button', { name: /^Category BASS/i }))

    // Only Bass presets visible
    expect(within(mainTable).getByText(/Sub Destroyer/i)).toBeInTheDocument()
    expect(within(mainTable).getByText(/Dark Matter/i)).toBeInTheDocument()
    expect(within(mainTable).queryByText(/Default Init/i)).not.toBeInTheDocument()
    expect(within(mainTable).queryByText(/Cyber Neon/i)).not.toBeInTheDocument()
  })

  it('filters presets with instant 0ms search across name, description, author, and tags', () => {
    render(<PresetBrowserModal isOpen={true} onClose={() => {}} />)
    const mainTable = screen.getByRole('main')
    const searchInput = screen.getByPlaceholderText(/search/i)

    // Search by author
    fireEvent.change(searchInput, { target: { value: 'User Producer' } })
    expect(within(mainTable).getByText(/Dark Matter/i)).toBeInTheDocument()
    expect(within(mainTable).queryByText(/Sub Destroyer/i)).not.toBeInTheDocument()

    // Search by tag
    fireEvent.change(searchInput, { target: { value: 'Neon' } })
    expect(within(mainTable).getByText(/Cyber Neon/i)).toBeInTheDocument()
    expect(within(mainTable).queryByText(/Dark Matter/i)).not.toBeInTheDocument()

    // Search by description keyword
    fireEvent.change(searchInput, { target: { value: 'saturated' } })
    expect(within(mainTable).getByText(/Sub Destroyer/i)).toBeInTheDocument()
    expect(within(mainTable).queryByText(/Cyber Neon/i)).not.toBeInTheDocument()

    // Non-matching query
    fireEvent.change(searchInput, { target: { value: 'xyz123nonexistent' } })
    expect(screen.getByText(/no presets/i)).toBeInTheDocument()
  })

  it('selects preset on row click and updates metadata inspector', () => {
    render(<PresetBrowserModal isOpen={true} onClose={() => {}} />)
    const mainTable = screen.getByRole('main')

    // Click on Cyber Neon preset row in main table
    const leadRow = within(mainTable).getByText(/Cyber Neon/i)
    fireEvent.click(leadRow)

    // Check right panel metadata inspection
    expect(screen.getByText('Bright resonant lead with sync delay and wide chorus.')).toBeInTheDocument()
    expect(screen.getByText('factory://Lead/03_Cyber_Neon.synthortionpreset')).toBeInTheDocument()
  })

  it('double clicking a preset row or clicking [ LOAD ] invokes loadPreset and closes', () => {
    const loadSpy = vi.fn()
    parameterStore.setBridge({ setParameter: () => {}, loadPreset: loadSpy })
    const handleClose = vi.fn()

    render(<PresetBrowserModal isOpen={true} onClose={handleClose} />)
    const mainTable = screen.getByRole('main')

    // Select and double-click Sub Destroyer
    const bassRow = within(mainTable).getByText(/Sub Destroyer/i)
    fireEvent.doubleClick(bassRow)

    expect(loadSpy).toHaveBeenCalledWith('factory://Bass/01_Sub_Destroyer')
    expect(handleClose).toHaveBeenCalled()
  })

  it('pressing Enter loads the currently selected preset', () => {
    const loadSpy = vi.fn()
    parameterStore.setBridge({ setParameter: () => {}, loadPreset: loadSpy })
    const handleClose = vi.fn()

    render(<PresetBrowserModal isOpen={true} onClose={handleClose} />)
    const mainTable = screen.getByRole('main')

    // Focus / select row and press Enter
    const leadRow = within(mainTable).getByText(/Cyber Neon/i)
    fireEvent.click(leadRow)

    const dialog = screen.getByRole('dialog', { name: 'Preset Browser' })
    fireEvent.keyDown(dialog, { key: 'Enter' })

    expect(loadSpy).toHaveBeenCalledWith('factory://Lead/03_Cyber_Neon')
    expect(handleClose).toHaveBeenCalled()
  })

  it('keyboard ArrowDown and ArrowUp navigates preset selection', () => {
    render(<PresetBrowserModal isOpen={true} onClose={() => {}} />)

    const dialog = screen.getByRole('dialog', { name: 'Preset Browser' })

    // Initially first item Default Init is selected
    expect(screen.getByText('Clean default starting point with neutral parameters.')).toBeInTheDocument()

    // Press ArrowDown -> moves to Sub Destroyer
    fireEvent.keyDown(dialog, { key: 'ArrowDown' })
    expect(screen.getByText('Massive saturated sub bass with heavy bitcrushing.')).toBeInTheDocument()

    // Press ArrowDown again -> moves to Cyber Neon
    fireEvent.keyDown(dialog, { key: 'ArrowDown' })
    expect(screen.getByText('Bright resonant lead with sync delay and wide chorus.')).toBeInTheDocument()

    // Press ArrowUp -> moves back to Sub Destroyer
    fireEvent.keyDown(dialog, { key: 'ArrowUp' })
    expect(screen.getByText('Massive saturated sub bass with heavy bitcrushing.')).toBeInTheDocument()
  })

  it('disables [ DELETE ] action for immutable factory presets with tooltip note', () => {
    render(<PresetBrowserModal isOpen={true} onClose={() => {}} />)
    const mainTable = screen.getByRole('main')

    // Select factory preset (Default Init)
    fireEvent.click(within(mainTable).getByText(/Default Init/i))

    const deleteBtn = screen.getByRole('button', { name: /disabled for factory preset/i })
    expect(deleteBtn).toBeDisabled()
    expect(screen.getByText(/IMMUTABLE FACTORY/i)).toBeInTheDocument()
  })

  it('enables [ DELETE ] for user preset and requires confirmation before deletion', () => {
    const deleteSpy = vi.fn()
    parameterStore.setBridge({ setParameter: () => {}, deletePreset: deleteSpy })

    render(<PresetBrowserModal isOpen={true} onClose={() => {}} />)
    const mainTable = screen.getByRole('main')

    // Select user preset (Dark Matter)
    fireEvent.click(within(mainTable).getByText(/Dark Matter/i))

    const deleteBtn = screen.getByRole('button', { name: 'Delete preset Dark Matter' })
    expect(deleteBtn).not.toBeDisabled()

    // Click delete -> shows confirmation prompt
    fireEvent.click(deleteBtn)
    expect(screen.getByText(/CONFIRM DELETE\?/i)).toBeInTheDocument()

    // Cancel deletion
    const cancelBtn = screen.getByRole('button', { name: 'Cancel delete' })
    fireEvent.click(cancelBtn)
    expect(deleteSpy).not.toHaveBeenCalled()
    expect(screen.queryByText(/CONFIRM DELETE\?/i)).not.toBeInTheDocument()

    // Click delete again and confirm
    fireEvent.click(screen.getByRole('button', { name: 'Delete preset Dark Matter' }))
    const confirmBtn = screen.getByRole('button', { name: 'Confirm delete' })
    fireEvent.click(confirmBtn)

    expect(deleteSpy).toHaveBeenCalledWith('user://Bass/Dark_Matter')
  })

  it('renders Cartesian coordinate crosshairs (+) and no repeated rule loops in headers', () => {
    render(<PresetBrowserModal isOpen={true} onClose={() => {}} />)

    const crosshairs = screen.getAllByText('+')
    expect(crosshairs.length).toBeGreaterThanOrEqual(4)

    const dialog = screen.getByRole('dialog', { name: 'Preset Browser' })
    const headings = dialog.querySelectorAll('h1, h2, h3, header')
    headings.forEach((heading) => {
      expect(heading.textContent).not.toMatch(/─{2,}/)
    })
  })
})
