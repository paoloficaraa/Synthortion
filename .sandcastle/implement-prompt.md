# TASK

Fix issue {{TASK_ID}}: {{ISSUE_TITLE}}

Pull in the issue using `gh issue view <ID>`. If it has a parent PRD, pull that in too.

Only work on the issue specified.

Work on branch {{BRANCH}}. Make commits and verify the build.

# CONTEXT

Here are the last 10 commits:

<recent-commits>

!`git log -n 10 --format="%H%n%ad%n%B---" --date=short`

</recent-commits>

# PROJECT

This is a **C++20 JUCE 8.0.8 VST3/AU plugin** built with **CMake + Ninja**.

- Headers: `plugin/include/Synthortion/`
- Sources: `plugin/src/`
- Tests: `plugin/tests/`
- Dependencies: `libs/juce/` and `modules/gin/` (git submodules)
- Build: `cmake -B build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache && cmake --build build`
- Coding standards: `@.sandcastle/CODING_STANDARDS.md`
- Build cache: ccache is enabled — first build ~8 min, subsequent builds ~30s

# TEST TARGET RULES

When you add a test target (`SynthortionTests`) to `plugin/CMakeLists.txt`:
- Include **ALL** `plugin/src/*.cpp` files in the target sources, not just the files you're modifying
- This ensures `npm run typecheck` catches errors across the entire codebase
- Only build the test target (not the full VST3/AU plugin) to save time

# EXPLORATION

Explore the repo and fill your context window with relevant information that will allow you to complete the task.

Pay extra attention to:
- Existing test infrastructure in `plugin/tests/`
- The gin library patterns in `modules/gin/` (uses `juce::UnitTest`)
- Audio thread safety constraints: no heap allocation, mutexes, I/O, or string formatting inside `processBlock`

# EXECUTION

If applicable, use RGR to complete the task.

1. RED: write one test
2. GREEN: write the implementation to pass that test
3. REPEAT until done
4. REFACTOR the code

For C++ changes, ensure:
- RAII for all resources (no raw `new`/`delete`)
- `override` on virtual functions, `noexcept` where applicable
- `std::atomic<float>*` cached pointers from APVTS for audio thread parameter access

# FEEDBACK LOOPS

Before committing:
- If you modified `plugin/CMakeLists.txt`, run `npm run configure` first
- Run `npm run typecheck` — builds the project with ccache; first build ~8 min, subsequent ~30s. Must pass with zero errors.
- Run `npm run test` — builds and runs unit tests; must pass.
- **Never delete the build directory** (`rm -rf build`) — it wastes ccache and forces a clean rebuild.

# COMMIT

Make a git commit. The commit message must:

1. Start with `RALPH:` prefix
2. Include task completed + PRD reference
3. Key decisions made
4. Files changed
5. Blockers or notes for next iteration

Keep it concise.

# THE ISSUE

If the task is not complete, leave a comment on the issue with what was done.

Do not close the issue - this will be done later.

Once complete, output <promise>COMPLETE</promise>.

# FINAL RULES

ONLY WORK ON A SINGLE TASK.
