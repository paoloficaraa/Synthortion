# SKILLS

**Tone.** Load the `caveman` skill at `wenyan-ultra` intensity BEFORE any action.
It stays active every response (code, tests, and error strings remain normal;
the skill auto-drops for security warnings). Cuts output tokens by 65%+.

**Task-specific skill.** After reading the issue, invoke `find-skills` to
discover the best matching skill for this task (e.g. `impeccable` for UI,
`systematic-debugging` for bugs, `tdd` for test-first features). Load and follow
that skill alongside `caveman`.

# TASK

Fix issue {{TASK_ID}}: {{ISSUE_TITLE}}

Pull in the issue using `gh issue view <ID>`. If it has a parent PRD, pull that in too. Only work on the issue specified.

Work on branch {{BRANCH}}. Make commits and verify the build.

# CONTEXT

Last 10 commits:

<recent-commits>

!`git log -n 10 --format="%H%n%ad%n%B---" --date=short`

</recent-commits>

# PROJECT

@.sandcastle/project-context.md

# CODING STANDARDS

@.sandcastle/CODING_STANDARDS.md

# EXPLORATION

Explore the repo and fill your context window. Pay extra attention to:
- Existing test infrastructure in `plugin/tests/`
- The gin library patterns in `modules/gin/` (uses `juce::UnitTest`)
- Audio thread safety constraints (see CODING_STANDARDS.md)

# EXECUTION

If applicable, use RGR:
1. RED: write one test
2. GREEN: implement to pass that test
3. REPEAT until done
4. REFACTOR

For C++ changes:
- RAII for all resources (no raw `new`/`delete`)
- `override` on virtual functions, `noexcept` where applicable
- `std::atomic<float>*` cached from APVTS for audio thread param access

# FEEDBACK

Before committing:
- If `plugin/CMakeLists.txt` changed: `npm run configure` first
- `npm run typecheck` — must pass, zero errors
- `npm run test` — must pass
- **Never `rm -rf build`** — wastes ccache

# COMMIT

Commit message format:
1. Start with `RALPH:` prefix
2. Task completed + PRD reference
3. Key decisions made
4. Files changed
5. Blockers or notes for next iteration

# FINAL RULES

If task not complete, leave a comment on the issue with what was done. Do not close the issue — done later.
Once complete, output <promise>COMPLETE</promise>.
ONLY WORK ON A SINGLE TASK.