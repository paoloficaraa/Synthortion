/**
 * CartesianCornerMarks — Decorative coordinate corner crosses (`+`) for modal and frame overlays.
 */
export function CartesianCornerMarks() {
  return (
    <>
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
    </>
  )
}
