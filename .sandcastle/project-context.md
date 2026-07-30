# PROJECT

**Hybrid Architecture**: C++20 JUCE DSP Engine + WebView UI (React/Vite)

## DSP Engine (C++20 / JUCE 8.0.8)

- Headers: `plugin/include/Synthortion/`
- Sources: `plugin/src/`
- Tests: `plugin/tests/`
- Dependencies: `libs/juce/` and `modules/gin/` (git submodules)
- Build: `cmake -B build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && cmake --build build`
- ccache enabled — first build ~8 min, cached ~30s
- **Never `rm -rf build`** — wastes ccache

## WebView UI (React / Vite / TypeScript)

- Sources: `ui/`
- Entry: `ui/src/App.tsx`
- Styling: Tailwind CSS + CSS Modules
- 3D: React Three Fiber (`@react-three/fiber`)
- Animation: Framer Motion
- Build: `npm run build` (Vite)
- Dev server: `npm run dev`
- Lint: `npm run lint` (ESLint)

## Test target rule (C++)

When adding `SynthortionTests` to `Source/CMakeLists.txt`:
- Include **ALL** `Source/src/*.cpp` in test target sources, not just modified files
- Build only the test target, not the full VST3/AU plugin

## Feedback loop

Before committing:
- If `Source/CMakeLists.txt` changed: `npm run configure` first
- `npm run typecheck` — must pass with zero errors
- `npm run lint` — must pass
- `npm run test` — must pass
- **Never delete `build/`** — wastes ccache
