/* eslint-disable react-hooks/refs */
import { useRef } from 'react'
import { useFrame } from '@react-three/fiber'
import * as THREE from 'three'

/** Particle count — kept low so the glitch layer stays lightweight. */
const PARTICLE_COUNT = 240

/**
 * Square-sprite vertex shader.
 *
 * A custom GLSL layer for the "glitchy light particles": each particle is a
 * hard-cornered square (no soft round sprites), sized per-instance so depth
 * feels physical.
 */
const vertexShader = /* glsl */ `
  attribute float aScale;
  uniform float uSize;
  varying float vGlow;
  void main() {
    vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);
    gl_PointSize = uSize * aScale * (260.0 / -mvPosition.z);
    gl_Position = projectionMatrix * mvPosition;
    vGlow = aScale;
  }
`

/** Square-sprited fragment shader with a hard, aliased edge. */
const fragmentShader = /* glsl */ `
  uniform vec3 uColor;
  uniform float uOpacity;
  varying float vGlow;
  void main() {
    vec2 c = gl_PointCoord - 0.5;
    float edge = 1.0 - smoothstep(0.40, 0.50, max(abs(c.x), abs(c.y)));
    if (edge < 0.02) discard;
    gl_FragColor = vec4(uColor * (0.6 + 0.4 * vGlow), edge * uOpacity);
  }
`

/** Lazily-built mutable particle state (base positions, working positions, GPU attributes). */
function createParticleState() {
  const base = new Float32Array(PARTICLE_COUNT * 3)
  const positions = new Float32Array(PARTICLE_COUNT * 3)
  const scales = new Float32Array(PARTICLE_COUNT)

  for (let i = 0; i < PARTICLE_COUNT; i++) {
    base[i * 3] = (Math.random() - 0.5) * 8
    base[i * 3 + 1] = Math.random() * 3.6 - 0.4
    base[i * 3 + 2] = (Math.random() - 0.5) * 5 - 1
    positions[i * 3] = base[i * 3]
    positions[i * 3 + 1] = base[i * 3 + 1]
    positions[i * 3 + 2] = base[i * 3 + 2]
    scales[i] = 0.4 + Math.random() * 1.6
  }

  return {
    base,
    positions,
    positionAttr: new THREE.BufferAttribute(positions, 3),
    scaleAttr: new THREE.BufferAttribute(scales, 1),
    uniforms: {
      uColor: { value: new THREE.Color('#c7c3ba') }, // mirrors --accent
      uOpacity: { value: 0.55 },
      uSize: { value: 0.9 },
    },
  }
}

interface FftParticlesProps {
  /** Master bypass; bypassed particles stop glitching and settle. */
  active: boolean
}

/**
 * Lightweight glitchy particle field floating around the spectrum.
 *
 * Each frame the particles re-compute from a fixed base position, adding a
 * rare large "glitch" jump plus a slow sway. Position updates happen in place
 * (no allocation) and only the position attribute is flagged dirty.
 */
export function FftParticles({ active }: FftParticlesProps) {
  const pointsRef = useRef<THREE.Points>(null)
  const stateRef = useRef(createParticleState())
  const { base, positions, positionAttr, scaleAttr, uniforms } = stateRef.current

  useFrame((state) => {
    const points = pointsRef.current
    if (!points) return

    const t = state.clock.elapsedTime
    for (let i = 0; i < PARTICLE_COUNT; i++) {
      const i3 = i * 3
      const glitch = active && Math.random() < 0.035 ? (Math.random() - 0.5) * 0.9 : 0
      const swayY = Math.sin(t * 0.5 + i * 0.7) * 0.05
      const jitterY = active ? (Math.random() - 0.5) * 0.02 : 0

      positions[i3] = base[i3] + glitch
      positions[i3 + 1] = base[i3 + 1] + swayY + jitterY
      positions[i3 + 2] = base[i3 + 2]
    }

    positionAttr.needsUpdate = true
  })

  return (
    <points ref={pointsRef}>
      <bufferGeometry>
        <primitive object={positionAttr} attach="attributes-position" />
        <primitive object={scaleAttr} attach="attributes-aScale" />
      </bufferGeometry>
      <shaderMaterial
        uniforms={uniforms}
        vertexShader={vertexShader}
        fragmentShader={fragmentShader}
        transparent
        depthWrite={false}
        blending={THREE.AdditiveBlending}
      />
    </points>
  )
}
