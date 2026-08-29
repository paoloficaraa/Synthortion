import { useState, useEffect, useMemo, useCallback, type FormEvent, type KeyboardEvent } from 'react'
import {
  parameterStore,
  CANONICAL_CATEGORIES,
  type SavePresetData,
  type PresetHeader,
} from '../lib/parameterStore'

export interface SavePresetModalProps {
  /** Whether the modal dialog is open. */
  isOpen: boolean
  /** Callback to close the modal dialog. */
  onClose: () => void
  /** Initial preset name. */
  initialName?: string
  /** Initial preset category. */
  initialCategory?: string
  /** Optional bridge override for testing or custom invocation. */
  bridge?: {
    savePreset?: (data: SavePresetData) => void
  }
}
/** Regex to detect illegal filename characters on all supported operating systems. */
const ILLEGAL_FILENAME_CHARS = /[\\/:*?"<>|]/

/**
 * SavePresetModal — High-contrast monochrome Cartesian ASCII save dialog.
 *
 * Enforces filename sanitization, character limits, category management, metadata entry,
 * and safe overwrite confirmation before dispatching the save IPC bridge event.
 */
export function SavePresetModal({
  isOpen,
  onClose,
  initialName,
  initialCategory,
  bridge,
}: SavePresetModalProps) {
  const [catalog, setCatalog] = useState<PresetHeader[]>(() =>
    parameterStore.getPresetCatalog()
  )
  const [name, setName] = useState<string>('')
  const [category, setCategory] = useState<string>('User')
  const [author, setAuthor] = useState<string>('User')
  const [description, setDescription] = useState<string>('')
  const [tags, setTags] = useState<string>('')
  const [confirmingOverwrite, setConfirmingOverwrite] = useState<boolean>(false)

  // Sync catalog when opened
  useEffect(() => {
    if (!isOpen) {
      setConfirmingOverwrite(false)
      return
    }
    const currentCatalog = parameterStore.getPresetCatalog()
    setCatalog(currentCatalog)

    const fallbackName = initialName || parameterStore.getActivePresetName() || 'My Preset'
    const fallbackCategory = initialCategory || parameterStore.getActivePresetCategory() || 'User'
    setName(fallbackName)
    setCategory(fallbackCategory)
    setAuthor('User')
    setDescription('')
    setTags('')
    setConfirmingOverwrite(false)
  }, [isOpen, initialName, initialCategory])

  // Real-time validation checks
  const validationError = useMemo(() => {
    const trimmed = name.trim()
    if (trimmed.length === 0) {
      return 'NAME REQUIRED (1-32 CHARS)'
    }
    if (trimmed.length > 32) {
      return 'MAX 32 CHARACTERS'
    }
    if (ILLEGAL_FILENAME_CHARS.test(name)) {
      return 'ILLEGAL CHARACTERS: \\ / : * ? " < > |'
    }
    if (ILLEGAL_FILENAME_CHARS.test(category)) {
      return 'CATEGORY ILLEGAL CHARACTERS: \\ / : * ? " < > |'
    }
    return null
  }, [name, category])
  const isNameValid = validationError === null

  // Check if preset already exists in the target category (duplicate)
  const isDuplicate = useMemo(() => {
    const trimmedName = name.trim().toLowerCase()
    const trimmedCat = category.trim().toLowerCase()
    return catalog.some(
      (p) =>
        p.name.toLowerCase() === trimmedName &&
        p.category.toLowerCase() === trimmedCat
    )
  }, [catalog, name, category])

  // Sanitized tags array
  const parsedTags = useMemo(() => {
    return tags
      .split(',')
      .map((t) => t.trim())
      .filter((t) => t.length > 0)
      .slice(0, 8)
  }, [tags])

  // Submit save action
  const handleSave = useCallback(
    (allowOverwrite: boolean) => {
      if (!isNameValid) return

      // If duplicate and not yet confirmed, trigger confirmation box
      if (isDuplicate && !allowOverwrite) {
        setConfirmingOverwrite(true)
        return
      }

      const saveData: SavePresetData = {
        name: name.trim(),
        category: category.trim() || 'User',
        author: author.trim() || 'User',
        description: description.trim(),
        tags: parsedTags,
        allowOverwrite,
      }

      parameterStore.savePreset(saveData, bridge)
      onClose()
    },
    [isNameValid, isDuplicate, name, category, author, description, parsedTags, bridge, onClose]
  )

  const handleSubmit = (e: FormEvent) => {
    e.preventDefault()
    handleSave(confirmingOverwrite)
  }

  // Keyboard navigation
  const handleKeyDown = (e: KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Escape') {
      e.preventDefault()
      onClose()
    }
  }

  // Global Escape key handler
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
      aria-label="Save Preset"
      onKeyDown={handleKeyDown}
      tabIndex={-1}
      className="fixed inset-0 z-50 flex items-center justify-center bg-black/80 backdrop-blur-xs p-4 outline-none select-none"
    >
      {/* Modal Chassis */}
      <div
        className="w-[520px] max-w-[96vw] bg-elev-1 border border-grid-rule flex flex-col font-mono text-fg relative overflow-hidden shadow-2xl"
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
              SAVE PRESET
            </h2>
            <span className="font-mono text-[9px] text-muted uppercase-tracked ml-2 hidden sm:inline" aria-hidden="true">
              [ CARTESIAN WRITER ]
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

        {/* Modal Body / Form */}
        <form onSubmit={handleSubmit} className="p-5 flex flex-col gap-4 bg-elev-1/40">
          {/* Preset Name Field */}
          <div className="flex flex-col gap-1.5">
            <div className="flex justify-between items-center text-[10px] uppercase-tracked">
              <label htmlFor="preset-name" className="text-fg font-medium">
                PRESET NAME <span className="text-ink-3 font-normal">*</span>
              </label>
              <span className="text-[9px] text-ink-3">
                {name.trim().length}/32
              </span>
            </div>
            <input
              id="preset-name"
              type="text"
              aria-label="Preset Name"
              value={name}
              onChange={(e) => {
                setName(e.target.value)
                setConfirmingOverwrite(false)
              }}
              placeholder="e.g. Acid Bass V2"
              autoFocus
              className={`w-full bg-elev-0 border px-3 py-1.5 text-[11px] font-mono text-fg outline-none transition-colors ${
                validationError
                  ? 'border-border text-fg'
                  : 'border-border focus:border-fg'
              }`}
            />
            {validationError && (
              <span className="text-[9px] text-fg font-mono uppercase-tracked bg-elev-3 px-2 py-0.5 border border-grid-rule">
                [ {validationError} ]
              </span>
            )}
          </div>

          {/* Category Selection */}
          <div className="flex flex-col gap-1.5">
            <div className="flex justify-between items-center text-[10px] uppercase-tracked">
              <label htmlFor="preset-category" className="text-fg font-medium">
                CATEGORY <span className="text-ink-3 font-normal">*</span>
              </label>
              <span className="text-[9px] text-ink-3">[ 1-LEVEL SUBFOLDER ]</span>
            </div>
            <div className="flex gap-2">
              <select
                id="preset-category"
                aria-label="Category"
                value={category}
                onChange={(e) => {
                  setCategory(e.target.value)
                  setConfirmingOverwrite(false)
                }}
                className="flex-1 bg-elev-0 border border-border px-2.5 py-1.5 text-[11px] font-mono text-fg outline-none focus:border-fg cursor-pointer"
              >
                {CANONICAL_CATEGORIES.map((cat) => (
                  <option key={cat} value={cat} className="bg-elev-1 text-fg">
                    {cat.toUpperCase()}
                  </option>
                ))}
              </select>
              <input
                type="text"
                aria-label="Custom category"
                placeholder="OR TYPE CUSTOM..."
                value={category}
                onChange={(e) => {
                  setCategory(e.target.value)
                  setConfirmingOverwrite(false)
                }}
                className="flex-1 bg-elev-0 border border-border px-2.5 py-1.5 text-[11px] font-mono text-fg placeholder:text-ink-2 outline-none focus:border-fg"
              />
            </div>
          </div>

          {/* Author Field */}
          <div className="flex flex-col gap-1.5">
            <label htmlFor="preset-author" className="text-[10px] text-fg font-medium uppercase-tracked">
              AUTHOR
            </label>
            <input
              id="preset-author"
              type="text"
              aria-label="Author"
              value={author}
              onChange={(e) => setAuthor(e.target.value)}
              placeholder="User"
              className="w-full bg-elev-0 border border-border px-3 py-1.5 text-[11px] font-mono text-fg placeholder:text-ink-2 outline-none focus:border-fg"
            />
          </div>

          {/* Description Field */}
          <div className="flex flex-col gap-1.5">
            <label htmlFor="preset-description" className="text-[10px] text-fg font-medium uppercase-tracked">
              DESCRIPTION <span className="text-ink-3 font-normal">(OPTIONAL)</span>
            </label>
            <textarea
              id="preset-description"
              aria-label="Description"
              rows={2}
              value={description}
              onChange={(e) => setDescription(e.target.value)}
              placeholder="Sound characteristics, modulation routing, or usage tips..."
              className="w-full bg-elev-0 border border-border p-2.5 text-[11px] font-mono text-fg placeholder:text-ink-2 outline-none focus:border-fg resize-none"
            />
          </div>

          {/* Tags Field */}
          <div className="flex flex-col gap-1.5">
            <div className="flex justify-between items-center text-[10px] uppercase-tracked">
              <label htmlFor="preset-tags" className="text-fg font-medium">
                TAGS <span className="text-ink-3 font-normal">(COMMA-SEPARATED, MAX 8)</span>
              </label>
              {parsedTags.length > 0 && (
                <span className="text-[9px] text-ink-3">
                  {parsedTags.length}/8 TAGS
                </span>
              )}
            </div>
            <input
              id="preset-tags"
              type="text"
              aria-label="Tags"
              value={tags}
              onChange={(e) => setTags(e.target.value)}
              placeholder="Lead, Acid, Cyberpunk, Distortion"
              className="w-full bg-elev-0 border border-border px-3 py-1.5 text-[11px] font-mono text-fg placeholder:text-ink-2 outline-none focus:border-fg"
            />
          </div>

          {/* Overwrite Warning Box */}
          {confirmingOverwrite && (
            <div className="p-3 bg-elev-2 border border-grid-rule flex flex-col gap-2 text-center animate-vst-enter">
              <span className="text-[10px] font-mono text-fg font-bold uppercase-tracked">
                [ OVERWRITE EXISTING PRESET? ]
              </span>
              <p className="text-[9px] text-muted font-mono leading-tight">
                A preset named "{name.trim()}" already exists in category "{category.trim()}".
                Overwriting will permanently replace the file.
              </p>
              <div className="flex justify-center gap-2 mt-1">
                <button
                  type="button"
                  onClick={() => handleSave(true)}
                  aria-label="Confirm overwrite"
                  className="px-3 py-1 bg-fg text-bg font-mono text-[9px] uppercase-tracked font-bold hover:opacity-90 cursor-pointer"
                >
                  <span aria-hidden="true">[ </span>
                  YES, OVERWRITE
                  <span aria-hidden="true"> ]</span>
                </button>
                <button
                  type="button"
                  onClick={() => setConfirmingOverwrite(false)}
                  aria-label="Cancel overwrite"
                  className="px-3 py-1 border border-border font-mono text-[9px] uppercase-tracked text-muted hover:text-fg hover:border-fg cursor-pointer"
                >
                  <span aria-hidden="true">[ </span>
                  CANCEL
                  <span aria-hidden="true"> ]</span>
                </button>
              </div>
            </div>
          )}

          {/* Action Footer */}
          {!confirmingOverwrite && (
            <div className="pt-3 border-t border-grid-rule flex justify-end gap-2 shrink-0 mt-2">
              <button
                type="button"
                onClick={onClose}
                aria-label="Cancel"
                className="px-4 py-1.5 border border-border font-mono text-[10px] uppercase-tracked text-muted hover:text-fg hover:border-fg transition-all cursor-pointer"
              >
                <span aria-hidden="true">[ </span>
                CANCEL
                <span aria-hidden="true"> ]</span>
              </button>

              <button
                type="submit"
                disabled={!isNameValid}
                aria-label="Save Preset"
                className={`px-5 py-1.5 border font-mono text-[10px] uppercase-tracked font-bold transition-all ${
                  isNameValid
                    ? 'border-border bg-elev-0 text-fg hover:bg-fg hover:text-bg hover:border-fg cursor-pointer active:scale-[0.98]'
                    : 'border-border/40 text-muted/40 cursor-not-allowed'
                }`}
              >
                <span aria-hidden="true">[ </span>
                SAVE PRESET
                <span aria-hidden="true"> ]</span>
              </button>
            </div>
          )}
        </form>
      </div>
    </div>
  )
}
