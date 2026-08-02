# TASK
Fix issue {{TASK_ID}}: {{ISSUE_TITLE}} on branch {{BRANCH}}.
Fetch the full issue with `gh issue view {{TASK_ID}}` (plus any parent PRD).
Work ONLY on this single issue.

# CONTEXT
Recent commits:
<recent-commits>
{{RECENT_COMMITS}}
</recent-commits>

# GRAPH-IFY (architecture awareness)
The repo has a knowledge graph mounted read-only at `graphify-out/` and the
`graphify` CLI is installed in the sandbox. Before exploring the code, run
`graphify query "<your question about the code>"` (e.g. "WebView UI", "DSP
pipeline", "parameter state") and `graphify explain "<concept>"` to get a
scoped subgraph of the relevant modules and their cross-file relationships.
Use this as your map before you start editing — it is much cheaper than
reading the whole repo.

# SKILLS

1. Fetch the issue with `gh issue view {{TASK_ID}}` FIRST — you need its
   **labels** to pick the right skills.
2. Skills are mounted read-only from the host at `~/.agents/skills` and
   `~/.config/opencode/skills`. List them, read the `SKILL.md` of the ones that
   match the issue's labels/title, and apply them.
3. Always load `tdd` (`~/.agents/skills/tdd/SKILL.md`) and `implement`
   (`~/.agents/skills/implement/SKILL.md`) for implementation work.

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
