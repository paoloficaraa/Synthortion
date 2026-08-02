# TASK
Fix issue {{TASK_ID}}: {{ISSUE_TITLE}} on branch {{BRANCH}}.
Fetch the full issue with `gh issue view {{TASK_ID}}` (plus any parent PRD).
Work ONLY on this single issue.

# CONTEXT
Recent commits:
<recent-commits>
{{RECENT_COMMITS}}
</recent-commits>

# EXPLORING THE CODE

There is no knowledge graph. Before editing, map the parts you will touch
yourself:
- `ls`/`find` to see the directory shape, then read the relevant files.
- Start from the entry points named in the issue (e.g. `ui/src/App.tsx`,
  `plugin/`, `lib/`) and follow imports.
- Keep a mental (or scratch) note of what calls what — it is cheaper than
  reading the whole repo.

# SKILLS

The full host skill catalog is mounted read-only at `~/.agents/skills`.
These are the exact same skills the host uses. Pick the right ones in two
stages. Start by fetching the issue: `gh issue view {{TASK_ID}}` FIRST — you
need its **labels**, title and body to judge what skills apply.

## Stage 1 — Select from the local catalog (always)

1. `ls ~/.agents/skills/` to see what's installed, then read the `SKILL.md`
   of the ones that match the issue context (labels/title/body). A non-
   exhaustive map of the catalog:
   - **always load for code work**: `tdd`, `implement`
   - **C++ / DSP / JUCE plugin core**: `code-review`, `codebase-design`,
     `diagnosing-bugs`, `prototype`, `resolving-merge-conflicts`
   - **React / webview UI (`ui/`)**: `frontend-design`, `impeccable`,
     `ui-ux-pro-max`, `webapp-testing`, `web-design-guidelines`,
     `canvas-design`, `vercel-composition-patterns`,
     `vercel-react-best-practices`, `vercel-react-view-transitions`,
     `web-artifacts-builder`
   - **docs / specs / writing**: `doc-coauthoring`, `research`,
     `writing-guidelines`, `domain-modeling`, `ubiquitous-language`
   - **deploy**: `deploy-to-vercel`, `vercel-cli-with-tokens`,
     `vercel-optimize`
   - **other**: `diagnosing-bugs`, `mcp-builder`, `pdf`, `docx`, `xlsx`,
     `pptx`, `slack-gif-creator`, `skill-creator`, `find-skills`
   Not every skill applies to every issue — pick what genuinely helps and
   load only those SKILL.md files. Do not load all of them blindly.
2. Always load `tdd` (`~/.agents/skills/tdd/SKILL.md`) and `implement`
   (`~/.agents/skills/implement/SKILL.md`) for implementation work, on top
   of any context-specific skills you picked.

## Stage 2 — Find external skills when the catalog is insufficient (optional)

If, after Stage 1, a domain the issue needs is NOT covered by the local
catalog (e.g. an issue asks for Vercel deploy but no Vercel skill is
installed, or it asks for a domain the map above doesn't list), use the
`find-skills` skill as a FALLBACK to discover and READ an external skill —
WITHOUT installing it (the host catalog is read-only by design).

1. Read `~/.agents/skills/find-skills/SKILL.md` for the workflow.
2. Search the public registry non-interactively:
   ```bash
   echo "" | npx skills find "<domain keywords>" | sed 's/\x1b\[[0-9;]*m//g'
   ```
   (`skills find` opens a TUI on a tty, but with stdin closed it prints the
   ranked text results to stdout instead; `sed` strips ANSI colour codes.)
3. Vet candidates the way find-skills prescribes: prefer install count
   ≥1K and authoritative sources (`vercel-labs`, `anthropics`, `microsoft`,
   official orgs). Skip anything sketchy (<100 installs, unknown author).
4. READ the chosen skill's instructions WITHOUT installing. `skills use`
   streams the full `SKILL.md` to stdout — add `-g -y` style non-interactive
   flags are not needed; just pipe to a file you Read:
   ```bash
   npx skills use <owner/repo@skill> > /tmp/skill.md 2>/dev/null
   # then open /tmp/skill.md with the Read tool
   ```
   Do NOT run `skills add` — it would try to write into the read-only mount
   and fail. `skills use` is read-only and is the supported substitute.
5. If `npx skills find` returns nothing relevant, fall back to your own
   engineering judgement — find-skills is a help, not a dependency.

## Apply

3. (continuing the numbering) Apply the loaded skills' instructions to your
   work — both local (Stage 1) and, if used, the external one (Stage 2).
   They are not a checklist — they are guidance for better implementation.

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
