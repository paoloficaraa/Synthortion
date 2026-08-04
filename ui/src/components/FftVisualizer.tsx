import { useRef } from 'react'
import { Canvas } from '@react-three/fiber'
import { PerspectiveCamera } from '@react-three/drei'
import { createFftSignal, type FftSignal } from '../lib/fftSignal'
import { FftBars } from './FftBars'
import { FftParticles } from './FftParticles'
import { FftFloor } from './FftFloor'

/** Frequency labels preserved from the prototype's FFT band. */
const FREQUENCY_LABELS = ['20Hz', '200Hz', '2kHz', '20kHz']

interface FftVisualizerProps {
  /** Master bypass; mirrors the prototype's engineActive toggle. */
  active: boolean
  /**
   * Injectable signal source so tests can feed mock frames. Defaults to the
   * structural-noise generator — visual logic stays isolated from DSP.
   */
  signal?: FftSignal
}

/**
 * FftVisualizer — the 3D spectrum band above the faceplate.
 *
 * A React Three Fiber canvas (perspective, no orthographic) renders sharp-
 * cornered 3D bars with hard `BasicShadowMap` shadowing plus a custom-GLSL
 * glitchy particle layer on the `#0f0e0e` backdrop. It is driven purely by
 * the simulated structural-noise signal generator — no WebAudio FFT.
 */
export function FftVisualizer({ active, signal }: FftVisualizerProps) {
  const fallbackRef = useRef<FftSignal | null>(null)
  const resolvedSignal = signal ?? (fallbackRef.current ??= createFftSignal())

  return (
    <div
      data-testid="fft-visualizer"
      data-active={active}
      className="relative w-full h-[240px] bg-bg overflow-hidden border-b border-border shadow-[inset_0_-1px_3px_rgba(0,0,0,0.5)]"
    >
      <Canvas
        shadows="basic"
        dpr={[1, 2]}
        gl={{ antialias: false, powerPreference: 'high-performance' }}
      >
        <color attach="background" args={['#0f0e0e']} />
        <PerspectiveCamera makeDefault position={[0, 2.6, 9]} fov={40} />
        <ambientLight intensity={0.18} />
        <directionalLight
          castShadow
          position={[6, 8, 4]}
          intensity={1.35}
          shadow-mapSize={[1024, 1024]}
          shadow-camera-near={1}
          shadow-camera-far={30}
          shadow-camera-left={-6}
          shadow-camera-right={6}
          shadow-camera-top={6}
          shadow-camera-bottom={-6}
          shadow-bias={-0.0005}
        />
        <FftBars signal={resolvedSignal} active={active} />
        <FftParticles active={active} />
        <FftFloor />
      </Canvas>

      {/* Hardware bezel shadow */}
      <div className="absolute inset-0 pointer-events-none shadow-[inset_0_12px_24px_rgba(0,0,0,0.9)]" />

      {/* Frequency labels (kept from the prototype) */}
      <div
        className="absolute bottom-3 right-4 flex gap-6 opacity-30"
        aria-hidden="true"
      >
        {FREQUENCY_LABELS.map((label) => (
          <span key={label} className="font-mono text-[9px] text-fg">
            {label}
          </span>
        ))}
      </div>
    </div>
  )
}
