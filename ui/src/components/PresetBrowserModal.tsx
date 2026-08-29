import { useEffect, useState, useMemo, useCallback, type KeyboardEvent } from 'react'
import { parameterStore, CANONICAL_CATEGORIES, type PresetHeader } from '../lib/parameterStore'

export interface PresetBrowserModalProps {
  /** Whether the modal dialog is open. */
  isOpen: boolean
  /** Callback to close the modal dialog. */
  onClose: () => void
  /** Optional bridge override for testing or custom invocation. */
  bridge?: {
    loadPreset?: (id: string) => void
    deletePreset?: (id: string) => void
  }
}

/** Category list prefix with ALL. */
const CATEGORY_NAMES = ['ALL', ...CANONICAL_CATEGORIES.map((c) => c.toUpperCase())] as const

/**
 * PresetBrowserModal — High-contrast monochrome Cartesian ASCII preset catalog browser.
 *
 * Provides category sidebar filtering, instant 0ms multi-field search, monospaced
 * tabular preset list with selection cursor, detailed metadata inspection, double-click/Enter
 * direct recall, and safe deletion confirmation with factory preset protection.
 */
export function PresetBrowserModal({
  isOpen,
  onClose,
  bridge,
}: PresetBrowserModalProps) {
  const [catalog, setCatalog] = useState<PresetHeader[]>(() =>
    parameterStore.getPresetCatalog()
  )
  const [activePresetId, setActivePresetId] = useState<string | null>(() =>
    parameterStore.getActivePresetId()
  )
  const [selectedCategory, setSelectedCategory] = useState<string>('ALL')
  const [searchQuery, setSearchQuery] = useState<string>('')
  const [selectedPresetId, setSelectedPresetId] = useState<string | null>(null)
  const [confirmingDelete, setConfirmingDelete] = useState<boolean>(false)

  // Sync catalog and active preset from parameterStore
  useEffect(() => {
    const sync = () => {
      const currentCatalog = parameterStore.getPresetCatalog()
      setCatalog(currentCatalog)
      const currentActiveId = parameterStore.getActivePresetId()
      setActivePresetId(currentActiveId)
    }
    sync()
    return parameterStore.subscribePresets(sync)
  }, [])

  // Derive unique categories from catalog combined with canonical list
  const categories = useMemo(() => {
    const set = new Set<string>(CATEGORY_NAMES)
    catalog.forEach((p) => {
      if (p.category) {
        set.add(p.category.toUpperCase())
      }
    })
    return Array.from(set)
  }, [catalog])

  // Count presets per category
  const categoryCounts = useMemo(() => {
    const counts: Record<string, number> = { ALL: catalog.length }
    catalog.forEach((p) => {
      const cat = p.category ? p.category.toUpperCase() : 'USER'
      counts[cat] = (counts[cat] || 0) + 1
    })
    return counts
  }, [catalog])

  // Instant 0ms multi-field search & category filter
  const filteredPresets = useMemo(() => {
    const query = searchQuery.trim().toLowerCase()
    return catalog.filter((preset) => {
      // Category check
      if (selectedCategory !== 'ALL') {
        const catUpper = (preset.category || '').toUpperCase()
        if (catUpper !== selectedCategory) {
          return false
        }
      }

      // Search query check across name, description, author, and tags
      if (query.length > 0) {
        const nameMatch = (preset.name || '').toLowerCase().includes(query)
        const descMatch = (preset.description || '').toLowerCase().includes(query)
        const authorMatch = (preset.author || '').toLowerCase().includes(query)
        const tagsMatch = (preset.tags || []).some((t) =>
          t.toLowerCase().includes(query)
        )
        if (!nameMatch && !descMatch && !authorMatch && !tagsMatch) {
          return false
        }
      }

      return true
    })
  }, [catalog, selectedCategory, searchQuery])

  // Keep a valid selected preset when list or filters change
  useEffect(() => {
    if (!isOpen) {
      setConfirmingDelete(false)
      return
    }
    if (filteredPresets.length > 0) {
      if (!selectedPresetId || !filteredPresets.some((p) => p.id === selectedPresetId)) {
        // Default to active preset if present in filtered list, else first
        const activeInList = filteredPresets.find((p) => p.id === activePresetId)
        setSelectedPresetId(activeInList ? activeInList.id : filteredPresets[0].id)
      }
    } else {
      setSelectedPresetId(null)
    }
    setConfirmingDelete(false)
  }, [filteredPresets, isOpen, activePresetId, selectedPresetId])

  // Selected preset object
  const selectedPreset = useMemo(() => {
    if (!selectedPresetId) return null
    return catalog.find((p) => p.id === selectedPresetId) || null
  }, [catalog, selectedPresetId])

  // Handle loading a preset
  const handleLoadPreset = useCallback(
    (presetId: string) => {
      parameterStore.loadPreset(presetId, bridge)
      onClose()
    },
    [bridge, onClose]
  )

  // Handle deleting a user preset
  const handleDeletePreset = useCallback(
    (presetId: string) => {
      parameterStore.deletePreset(presetId, bridge)
      setConfirmingDelete(false)
    },
    [bridge]
  )

  // Keyboard navigation across presets & modal shortcuts
  const handleKeyDown = useCallback(
    (e: KeyboardEvent<HTMLDivElement> | React.KeyboardEvent) => {
      if (e.key === 'Escape') {
        e.preventDefault()
        onClose()
        return
      }

      if (filteredPresets.length === 0) return

      const currentIndex = filteredPresets.findIndex((p) => p.id === selectedPresetId)

      if (e.key === 'ArrowDown') {
        e.preventDefault()
        const nextIndex = currentIndex < filteredPresets.length - 1 ? currentIndex + 1 : 0
        setSelectedPresetId(filteredPresets[nextIndex].id)
        setConfirmingDelete(false)
      } else if (e.key === 'ArrowUp') {
        e.preventDefault()
        const prevIndex = currentIndex > 0 ? currentIndex - 1 : filteredPresets.length - 1
        setSelectedPresetId(filteredPresets[prevIndex].id)
        setConfirmingDelete(false)
      } else if (e.key === 'Enter') {
        // Allow Enter to recall preset if valid preset is selected and not confirming delete
        if (selectedPreset && !confirmingDelete) {
          e.preventDefault()
          handleLoadPreset(selectedPreset.id)
        }
      }
    },
    [filteredPresets, selectedPresetId, selectedPreset, confirmingDelete, handleLoadPreset, onClose]
  )

  // Global escape key handler
  useEffect(() => {
    if (!isOpen) return
    const handleGlobalKeyDown = (e: globalThis.KeyboardEvent) => {
      if (e.key === 'Escape') {
        onClose()
      }
    }
    window.addEventListener('keydown', handleGlobalKeyDown)
    return () => window.removeEventListener('keydown', handleGlobalKeyDown)
  }, [isOpen, onClose])

  if (!isOpen) return null

  return (
    <div
      role="dialog"
      aria-modal="true"
      aria-label="Preset Browser"
      onKeyDown={handleKeyDown}
      tabIndex={-1}
      className="fixed inset-0 z-50 flex items-center justify-center bg-black/80 backdrop-blur-xs p-4 outline-none select-none"
    >
      {/* Modal Dialog Chassis */}
      <div
        className="w-[880px] max-w-[96vw] h-[540px] max-h-[92vh] bg-elev-1 border border-grid-rule flex flex-col font-mono text-fg relative overflow-hidden shadow-2xl"
        onClick={(e) => e.stopPropagation()}
      >
        {/* Cartesian Coordinate Corner Marks */}
        <div
          className="absolute top-0 left-0 -mt-[4px] -ml-[3px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none"
          aria-hidden="true"
        >
          +
        </div>
        <div
          className="absolute top-0 right-0 -mt-[4px] -mr-[3px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none"
          aria-hidden="true"
        >
          +
        </div>
        <div
          className="absolute bottom-0 left-0 -mb-[4px] -ml-[3px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none"
          aria-hidden="true"
        >
          +
        </div>
        <div
          className="absolute bottom-0 right-0 -mb-[4px] -mr-[3px] font-ascii text-[8px] text-ink-3 leading-none pointer-events-none select-none"
          aria-hidden="true"
        >
          +
        </div>

        {/* Modal Header */}
        <header className="h-[44px] bg-elev-0 border-b border-grid-rule px-4 flex items-center justify-between shrink-0 relative">
          <div className="flex items-center gap-2">
            <span aria-hidden="true" className="text-ink-3 font-ascii text-[10px]">
              [ + ]
            </span>
            <h2 className="font-mono text-[12px] uppercase-tracked text-fg font-bold tracking-wider">
              PRESET BROWSER
            </h2>
            <span className="font-mono text-[9px] text-muted uppercase-tracked ml-2 hidden sm:inline" aria-hidden="true">
              [ CARTESIAN CATALOG v1.0 ]
            </span>
          </div>

          <button
            type="button"
            onClick={onClose}
            aria-label="Close"
            className="px-2.5 py-1 border border-border font-mono text-[9px] uppercase-tracked text-muted hover:bg-fg hover:text-bg hover:border-fg transition-all cursor-pointer outline-none focus-visible:ring-2 focus-visible:ring-accent"
          >
            <span aria-hidden="true">[ </span>
            ESC / CLOSE
            <span aria-hidden="true"> ]</span>
          </button>
        </header>

        {/* Modal Body: 3 Columns (Categories, Table, Inspector) */}
        <div className="flex flex-1 min-h-0 divide-x divide-grid-rule overflow-hidden">
          {/* Left Panel: Category Sidebar */}
          <aside className="w-[180px] shrink-0 flex flex-col bg-elev-0/40 overflow-hidden">
            <div className="px-3 py-2 border-b border-grid-rule text-[9px] font-mono uppercase-tracked text-ink-3 flex items-center justify-between shrink-0">
              <span>[ CATEGORIES ]</span>
              <span>{catalog.length}</span>
            </div>

            <div className="flex-1 overflow-y-auto p-2 flex flex-col gap-1">
              {categories.map((cat) => {
                const count = categoryCounts[cat] || 0
                const isSelected = selectedCategory === cat
                return (
                  <button
                    key={cat}
                    type="button"
                    onClick={() => {
                      setSelectedCategory(cat)
                      setConfirmingDelete(false)
                    }}
                    aria-label={`Category ${cat}, ${count} presets`}
                    aria-pressed={isSelected}
                    className={`w-full px-2.5 py-1 text-left font-mono text-[10px] uppercase-tracked transition-all flex items-center justify-between cursor-pointer border outline-none focus-visible:ring-1 focus-visible:ring-accent ${
                      isSelected
                        ? 'bg-fg text-bg font-bold border-fg'
                        : 'text-muted hover:text-fg hover:bg-elev-2 border-transparent'
                    }`}
                  >
                    <span>
                      {isSelected ? `> [ ${cat} ]` : `  [ ${cat} ]`}
                    </span>
                    <span
                      className={`text-[9px] ${
                        isSelected ? 'text-bg font-normal' : 'text-ink-3'
                      }`}
                    >
                      {count}
                    </span>
                  </button>
                )
              })}
            </div>
          </aside>

          {/* Center Panel: Preset Table & Instant 0ms Search Filter */}
          <main className="flex-1 flex flex-col min-w-0 bg-elev-1/30 overflow-hidden">
            {/* Search Input Bar */}
            <div className="p-2.5 border-b border-grid-rule flex items-center gap-2 bg-elev-0/40 shrink-0">
              <span
                className="font-mono text-[10px] text-ink-3 uppercase-tracked select-none shrink-0"
                aria-hidden="true"
              >
                [ SEARCH ]
              </span>
              <input
                type="text"
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                placeholder="SEARCH PRESETS (NAME, AUTHOR, TAGS)..."
                aria-label="Search presets"
                className="flex-1 bg-elev-0 border border-border px-2.5 py-1 text-[11px] font-mono text-fg placeholder:text-ink-2 outline-none focus:border-fg transition-colors"
              />
              {searchQuery.length > 0 && (
                <button
                  type="button"
                  onClick={() => setSearchQuery('')}
                  aria-label="Clear search"
                  className="px-2 py-0.5 border border-border font-mono text-[9px] uppercase-tracked text-muted hover:text-fg hover:border-fg cursor-pointer"
                >
                  <span aria-hidden="true">[ </span>
                  CLEAR
                  <span aria-hidden="true"> ]</span>
                </button>
              )}
            </div>

            {/* Table Column Headers */}
            <div className="grid grid-cols-[1fr_80px_110px_70px] gap-2 px-3 py-1.5 border-b border-grid-rule text-[9px] font-mono uppercase-tracked text-ink-3 bg-elev-0 shrink-0 select-none">
              <div>NAME</div>
              <div>CATEGORY</div>
              <div>AUTHOR</div>
              <div className="text-right">TYPE</div>
            </div>

            {/* Table Rows List */}
            <div
              role="listbox"
              aria-label="Preset list"
              className="flex-1 overflow-y-auto p-1 divide-y divide-elev-6"
            >
              {filteredPresets.length === 0 ? (
                <div className="p-8 text-center font-mono text-[11px] text-muted flex flex-col items-center justify-center gap-1.5">
                  <span>[ NO PRESETS FOUND ]</span>
                  <span className="text-[9px] text-ink-3">
                    Try adjusting search query or category filter
                  </span>
                </div>
              ) : (
                filteredPresets.map((preset) => {
                  const isSelected = preset.id === selectedPresetId
                  const isActive = preset.id === activePresetId
                  return (
                    <div
                      key={preset.id}
                      onClick={() => {
                        setSelectedPresetId(preset.id)
                        setConfirmingDelete(false)
                      }}
                      onDoubleClick={() => handleLoadPreset(preset.id)}
                      role="option"
                      aria-selected={isSelected}
                      className={`grid grid-cols-[1fr_80px_110px_70px] gap-2 px-2.5 py-1.5 items-center text-[10px] font-mono cursor-pointer border transition-colors ${
                        isSelected
                          ? 'bg-elev-3 border-grid-rule text-fg shadow-sm'
                          : 'border-transparent text-muted hover:bg-elev-2 hover:text-fg'
                      }`}
                    >
                      {/* Name Column */}
                      <div className="flex items-center gap-1.5 truncate font-medium">
                        {isSelected ? (
                          <span className="text-fg font-bold">
                            &gt; [ {preset.name.toUpperCase()} ] &lt;
                          </span>
                        ) : (
                          <span>  {preset.name}</span>
                        )}
                        {isActive && (
                          <span
                            className="text-[9px] text-ink-4 px-1 border border-border"
                            title="Currently loaded active preset"
                          >
                            ACTIVE
                          </span>
                        )}
                        {preset.favorite && (
                          <span className="text-fg text-[9px]" title="Favorite">
                            ★
                          </span>
                        )}
                      </div>

                      {/* Category Column */}
                      <div className="truncate text-ink-3">
                        [{preset.category.toUpperCase()}]
                      </div>

                      {/* Author Column */}
                      <div className="truncate text-ink-3">
                        {preset.author || 'Synthortion'}
                      </div>

                      {/* Type Column */}
                      <div className="text-right text-[9px] truncate">
                        <span
                          className={`px-1 py-0.5 ${
                            preset.isFactory
                              ? 'text-ink-3'
                              : 'text-fg border border-border'
                          }`}
                        >
                          {preset.isFactory ? 'FACTORY' : 'USER'}
                        </span>
                      </div>
                    </div>
                  )
                })
              )}
            </div>
          </main>

          {/* Right Panel: Metadata & Action Inspector */}
          <aside className="w-[260px] shrink-0 flex flex-col bg-elev-0/40 p-3 justify-between overflow-y-auto">
            {selectedPreset ? (
              <>
                {/* Metadata Details */}
                <div className="flex flex-col gap-3">
                  <div className="pb-2 border-b border-grid-rule text-[9px] font-mono uppercase-tracked text-ink-3 flex items-center justify-between shrink-0">
                    <span>[ METADATA INSPECTOR ]</span>
                    <span className="text-ink-2">
                      {selectedPreset.isFactory ? 'IMMUTABLE' : 'USER WRITABLE'}
                    </span>
                  </div>

                  {/* Preset Title & Category */}
                  <div>
                    <div className="font-mono text-[13px] font-bold text-fg uppercase tracking-wide leading-tight">
                      {selectedPreset.name}
                    </div>
                    <div className="font-mono text-[9px] text-muted uppercase-tracked mt-0.5">
                      [ {selectedPreset.category.toUpperCase()} ]
                    </div>
                  </div>

                  {/* Author & Creation */}
                  <div className="flex flex-col gap-1 text-[10px] text-ink-3 border-y border-elev-6 py-2">
                    <div className="flex justify-between">
                      <span>AUTHOR:</span>
                      <span className="text-fg truncate max-w-[140px]">
                        {selectedPreset.author || 'Synthortion Core'}
                      </span>
                    </div>
                    <div className="flex justify-between">
                      <span>TYPE:</span>
                      <span className="text-fg">
                        {selectedPreset.isFactory ? 'FACTORY (BINARY)' : 'USER FILE'}
                      </span>
                    </div>
                    {selectedPreset.createdAt && (
                      <div className="flex justify-between text-[9px]">
                        <span>CREATED:</span>
                        <span className="text-ink-4 truncate max-w-[140px]">
                          {selectedPreset.createdAt.split('T')[0] || selectedPreset.createdAt}
                        </span>
                      </div>
                    )}
                  </div>

                  {/* Description */}
                  <div className="flex flex-col gap-1">
                    <span className="text-[9px] text-ink-3 uppercase-tracked">
                      DESCRIPTION:
                    </span>
                    <p className="text-[10px] text-fg leading-relaxed bg-elev-1/60 p-2 border border-elev-6 min-h-[48px]">
                      {selectedPreset.description || 'No description provided.'}
                    </p>
                  </div>

                  {/* Tags */}
                  {selectedPreset.tags && selectedPreset.tags.length > 0 && (
                    <div className="flex flex-col gap-1">
                      <span className="text-[9px] text-ink-3 uppercase-tracked">
                        TAGS:
                      </span>
                      <div className="flex flex-wrap gap-1">
                        {selectedPreset.tags.map((tag) => (
                          <span
                            key={tag}
                            className="px-1.5 py-0.5 bg-elev-2 border border-border text-[9px] text-muted font-mono"
                          >
                            #{tag}
                          </span>
                        ))}
                      </div>
                    </div>
                  )}

                  {/* File Path */}
                  {selectedPreset.filePath && (
                    <div className="flex flex-col gap-1">
                      <span className="text-[9px] text-ink-3 uppercase-tracked">
                        PATH:
                      </span>
                      <span className="text-[9px] text-ink-4 font-mono break-all bg-elev-1/40 p-1 border border-elev-6">
                        {selectedPreset.filePath}
                      </span>
                    </div>
                  )}
                </div>

                {/* Inspector Bottom Actions: [ LOAD ] and [ DELETE ] */}
                <div className="pt-3 border-t border-grid-rule flex flex-col gap-2 mt-4 shrink-0">
                  {/* Load Button */}
                  <button
                    type="button"
                    onClick={() => handleLoadPreset(selectedPreset.id)}
                    aria-label={`Load preset ${selectedPreset.name}`}
                    className="w-full py-1.5 border border-border bg-elev-0 font-mono text-[10px] uppercase-tracked text-fg hover:bg-fg hover:text-bg hover:border-fg transition-all cursor-pointer outline-none focus-visible:ring-2 focus-visible:ring-accent active:scale-[0.98] flex items-center justify-center gap-1"
                  >
                    <span aria-hidden="true">[ </span>
                    LOAD PRESET
                    <span aria-hidden="true"> ]</span>
                  </button>

                  {/* Delete Section / Protection */}
                  {selectedPreset.isFactory ? (
                    <div className="flex flex-col items-center gap-0.5">
                      <button
                        type="button"
                        disabled
                        aria-label="Delete preset disabled for factory preset"
                        className="w-full py-1 border border-border/30 font-mono text-[9px] uppercase-tracked text-muted/40 cursor-not-allowed flex items-center justify-center"
                      >
                        <span aria-hidden="true">[ </span>
                        DELETE
                        <span aria-hidden="true"> ]</span>
                      </button>
                      <span className="text-[8px] text-ink-3 font-mono uppercase-tracked" aria-hidden="true">
                        [ IMMUTABLE FACTORY ]
                      </span>
                    </div>
                  ) : confirmingDelete ? (
                    <div className="flex flex-col gap-1.5 p-2 bg-elev-2 border border-grid-rule text-center">
                      <span className="text-[9px] font-mono text-fg font-bold">
                        [ CONFIRM DELETE? Y / N ]
                      </span>
                      <div className="flex gap-1.5 justify-center mt-1">
                        <button
                          type="button"
                          onClick={() => handleDeletePreset(selectedPreset.id)}
                          aria-label="Confirm delete"
                          className="px-2 py-0.5 border border-fg bg-fg text-bg font-mono text-[9px] uppercase-tracked hover:opacity-90 cursor-pointer"
                        >
                          <span aria-hidden="true">[ </span>
                          YES / CONFIRM
                          <span aria-hidden="true"> ]</span>
                        </button>
                        <button
                          type="button"
                          onClick={() => setConfirmingDelete(false)}
                          aria-label="Cancel delete"
                          className="px-2 py-0.5 border border-border font-mono text-[9px] uppercase-tracked text-muted hover:text-fg hover:border-fg cursor-pointer"
                        >
                          <span aria-hidden="true">[ </span>
                          NO / CANCEL
                          <span aria-hidden="true"> ]</span>
                        </button>
                      </div>
                    </div>
                  ) : (
                    <button
                      type="button"
                      onClick={() => setConfirmingDelete(true)}
                      aria-label={`Delete preset ${selectedPreset.name}`}
                      className="w-full py-1 border border-border font-mono text-[9px] uppercase-tracked text-muted hover:text-fg hover:border-fg transition-all cursor-pointer flex items-center justify-center"
                    >
                      <span aria-hidden="true">[ </span>
                      DELETE USER PRESET
                      <span aria-hidden="true"> ]</span>
                    </button>
                  )}
                </div>
              </>
            ) : (
              <div className="flex-1 flex items-center justify-center text-center font-mono text-[10px] text-ink-3">
                [ NO PRESET SELECTED ]
              </div>
            )}
          </aside>
        </div>
      </div>
    </div>
  )
}
