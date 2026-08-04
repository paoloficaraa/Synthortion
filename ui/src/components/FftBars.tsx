import { useRef } from 'react'
import { useFrame } from '@react-three/fiber'
import type { Mesh } from 'three'
import { BAR_COUNT, type FftSignal } from '../lib/fftSignal'

/** Total width the bar row spans in world units. */
const SPREAD = 5.4

/** Depth of each bar (z-axis), giving the spectrum physical thickness. */
const BAR_DEPTH = 0.65

/** Base height of a bar before per-bin scaling (unit box). */
const BASE_HEIGHT = 1

/**
 * Maps a prototype bin level (~0..100) to a world-space bar height.
 *
 * The floor keeps bypassed bars as small stubs instead of vanishing.
 */
function barHeight(binLevel: number): number {
  return Math.max(0.05, (binLevel / 28) * BASE_HEIGHT)
}

/** Centres bar `index` across the row so the spectrum hugs the origin. */
function barX(index: number): number {
  return ((index / (BAR_COUNT - 1)) - 0.5) * SPREAD
}

interface FftBarsProps {
  /** Structural-noise signal the bars are driven by. */
  signal: FftSignal
  /** Master bypass; bypassed engines decay the bars to idle stubs. */
  active: boolean
}

/**
 * The sharp-cornered 3D spectrum bars.
 *
 * One box per prototype bar, scaled on the Y axis each frame from the signal's
 * bin levels. `flatShading` + hard `BasicShadowMap` shadows keep the brutalist
 * no-soft-shading look. The boxes are never remounted — only their scale and
 * position mutate — so the loop is allocation-free after mount.
 */
export function FftBars({ signal, active }: FftBarsProps) {
  const meshesRef = useRef<Array<Mesh | null>>([])

  useFrame(() => {
    signal.step(active)
    const bins = signal.bins
    for (let i = 0; i < BAR_COUNT; i++) {
      const mesh = meshesRef.current[i]
      if (!mesh) continue
      const height = barHeight(bins[i] ?? 0)
      mesh.scale.y = height
      mesh.position.y = height / 2
    }
  })

  const barWidth = SPREAD / BAR_COUNT - 0.045

  return (
    <group>
      {Array.from({ length: BAR_COUNT }, (_, i) => (
        <mesh
          key={i}
          ref={(el) => {
            meshesRef.current[i] = el
          }}
          position={[barX(i), 0.02, 0]}
          castShadow
          receiveShadow
        >
          <boxGeometry args={[barWidth, BASE_HEIGHT, BAR_DEPTH]} />
          <meshStandardMaterial color="#f6f6f6" roughness={1} metalness={0.08} flatShading />
        </mesh>
      ))}
    </group>
  )
}
