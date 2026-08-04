interface RackScrewProps {
  /** Corner placement inside the panel. */
  className: string
}

/**
 * RackScrew — decorative hardware screw for the faceplate corners.
 *
 * Pure chrome: a dark metal head with a cross-slot cut, laid over the panel
 * corners. `pointer-events-none` keeps it from stealing focus from controls,
 * and `aria-hidden` removes it from the accessibility tree.
 */
export function RackScrew({ className }: RackScrewProps) {
  return (
    <div
      data-testid="rack-screw"
      aria-hidden="true"
      className={`absolute w-[13px] h-[13px] rounded-full pointer-events-none ${className}`}
      style={{
        background:
          'radial-gradient(circle at 38% 34%, #3a3a3a 0%, #1a1a1a 45%, #0c0c0c 100%)',
        border: '1px solid #000',
        boxShadow:
          'inset 0 1px 1px rgba(255,255,255,0.22), 0 1px 2px rgba(0,0,0,0.9)',
      }}
    >
      <span
        className="absolute inset-[3px] rounded-full"
        style={{
          background:
            'linear-gradient(135deg, #000 0%, #000 42%, #555 50%, #000 58%, #000 100%)',
        }}
      />
    </div>
  )
}
