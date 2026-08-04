/**
 * Ground plane that catches the bars' hard shadows.
 *
 * A single dark plane below the spectrum so the directional light has
 * something to carve a shadow onto — keeping the "hard, non-soft" brutalist
 * look the spec asks for.
 */
export function FftFloor() {
  return (
    <mesh rotation-x={-Math.PI / 2} position={[0, -0.03, 0]} receiveShadow>
      <planeGeometry args={[24, 12]} />
      <meshStandardMaterial color="#0a0a0a" roughness={1} metalness={0} />
    </mesh>
  )
}
