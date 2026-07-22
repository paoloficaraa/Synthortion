# PROJECT

**C++20 JUCE 8.0.8 VST3/AU plugin**, built with **CMake + Ninja**.

- Headers: `plugin/include/Synthortion/`
- Sources: `plugin/src/`
- Tests: `plugin/tests/`
- Dependencies: `libs/juce/` and `modules/gin/` (git submodules)
- Build: `cmake -B build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && cmake --build build`
- ccache enabled — first build ~8 min, cached ~30s
- **Never `rm -rf build`** — wastes ccache

## Test target rule

When adding `SynthortionTests` to `plugin/CMakeLists.txt`:
- Include **ALL** `plugin/src/*.cpp` in test target sources, not just modified files
- Build only the test target, not the full VST3/AU plugin

## Feedback loop

Before committing:
- If `plugin/CMakeLists.txt` changed: `npm run configure` first
- `npm run typecheck` — must pass with zero errors
- `npm run test` — must pass
- **Never delete `build/`**