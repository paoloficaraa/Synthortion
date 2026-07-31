# TASK
Fix issue {{TASK_ID}}: {{ISSUE_TITLE}} on branch {{BRANCH}}.
Fetch the full issue with `gh issue view {{TASK_ID}}` (plus any parent PRD).
Work ONLY on this single issue.

# CONTEXT
Recent commits:
<recent-commits>
{{RECENT_COMMITS}}
</recent-commits>

# SKILLS

1. Fetch the issue with `gh issue view {{TASK_ID}}` FIRST — you need its
   **labels** to pick the right skills.
2. Read `.sandcastle/skills/SKILLS.md` with the Read tool and pick the skills
   that match the issue's labels/title (mapping table inside).
3. Read the `SKILL.md` of each skill you picked (e.g.
   `.sandcastle/skills/tdd/SKILL.md`) and apply it.
4. Always load `tdd` + `implement` for implementation work.

# RULES
1. Explore the repo, then work TDD: RED (one failing test) → GREEN (implement) → REFACTOR.
2. New DSP/processor behaviour requires a JUCE unit test in `plugin/tests`.
3. Verify before every commit (JUCE submodule is already initialized):
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build --target SynthortionTests
   ./build/plugin/SynthortionTests
4. Commit message: `RALPH:` prefix, task completed + PRD reference, key
   decisions, files changed, blockers/notes. Concise.
5. If not complete, comment on the issue with progress. Do NOT close it.

Once complete, output <promise>COMPLETE</promise>.
