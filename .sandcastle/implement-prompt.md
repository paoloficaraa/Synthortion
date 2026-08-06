# TASK
Fix issue {{TASK_ID}}: {{ISSUE_TITLE}} on branch {{BRANCH}}.
Fetch the full issue with `gh issue view {{TASK_ID}}` (plus any parent PRD).
Work ONLY on this single issue.

# CONTEXT
Recent commits:
<recent-commits>
{{RECENT_COMMITS}}
</recent-commits>

# STEP 0 — LOAD SKILLS (MANDATORY, FIRST)

Do this BEFORE exploring the code or running any git command. The only
command allowed before Step 0 is `gh issue view {{TASK_ID}}` (you need the
issue body to pick context-specific skills).

1. `ls ~/.agents/skills/` to see the installed catalog.
2. Read `~/.agents/skills/tdd/SKILL.md` and
   `~/.agents/skills/implement/SKILL.md` — always required for
   implementation work.
3. Read any context-specific skills from the map in SKILLS below that match
   the issue (labels/title/body). Not every skill applies — pick what
   genuinely helps.
4. In your FIRST message, list the skills you loaded and the key rules you
   will apply from each. This makes skill usage visible in the log and
   forces you to internalize the guidance before touching code.

Do NOT skip Step 0 even when the branch already looks implemented — you
still need the skills to judge the existing work.

# SKILLS

The full host skill catalog is mounted read-only at `~/.agents/skills`.
These are the exact same skills the host uses. Pick the right ones from this
map (label/title/body-driven):

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

Not every skill applies to every issue — pick what genuinely helps and load
only those SKILL.md files. Do not load all of them blindly.

## External skills (fallback only)

If, after the catalog, a domain the issue needs is NOT covered (e.g. a Vercel
deploy with no Vercel skill installed), use `find-skills` to READ an external
skill WITHOUT installing (the host catalog is read-only by design):

1. Read `~/.agents/skills/find-skills/SKILL.md`.
2. Search non-interactively:
   ```bash
   echo "" | npx skills find "<domain keywords>" | sed 's/\x1b\[[0-9;]*m//g'
   ```
   (`skills find` opens a TUI on a tty, but with stdin closed it prints the
   ranked text results to stdout instead; `sed` strips ANSI colour codes.)
3. Vet candidates: prefer install count ≥1K and authoritative sources
   (`vercel-labs`, `anthropics`, `microsoft`, official orgs). Skip anything
   sketchy (<100 installs, unknown author).
4. READ the chosen skill's instructions WITHOUT installing:
   ```bash
   npx skills use <owner/repo@skill> > /tmp/skill.md 2>/dev/null
   # then open /tmp/skill.md with the Read tool
   ```
   Do NOT run `skills add` — it would try to write into the read-only mount
   and fail. `skills use` is read-only and is the supported substitute.
5. If `npx skills find` returns nothing relevant, fall back to your own
   engineering judgement — find-skills is a help, not a dependency.

## Apply

Apply the loaded skills' instructions to your work — both local (Step 0) and,
if used, the external one. They are not a checklist — they are guidance for
better implementation.

# EXPLORING THE CODE

There is no knowledge graph in the sandbox. Before editing, map the parts
you will touch yourself:
- `ls`/`find` to see the directory shape, then read the relevant files.
- Start from the entry points named in the issue (e.g. `ui/src/App.tsx`,
  `plugin/`, `lib/`) and follow imports.
- Keep a mental (or scratch) note of what calls what — it is cheaper than
  reading the whole repo.

# RULES
1. Explore the repo, then work TDD: RED (one failing test) → GREEN (implement) → REFACTOR.
2. New DSP/processor behaviour requires a JUCE unit test in `plugin/tests`.
3. Verify before every commit (JUCE submodule is already initialized):
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build --target SynthortionTests
   ./build/plugin/SynthortionTests
4. Commit message: `RALPH:` prefix, task completed + PRD reference, key
   decisions, files changed, blockers/notes. Concise.
5. NEVER run `gh issue close` or any other `gh issue ...` write command.
   Closing issues is the merger's job, AFTER the branch is merged. If you
   need to record progress, comment on the issue instead:
   `gh issue comment {{TASK_ID}} --body "..."`
6. STOP CONDITION: once the implementation is complete and verified (tests,
   build, lint, dev-server as applicable), signal
   <promise>COMPLETE</promise> immediately. Do NOT re-verify the same checks
   across multiple iterations and do NOT keep exploring once every acceptance
   criterion is green. If the branch already contains the full implementation,
   verify ONCE, report it, and signal COMPLETE.
7. NO INTERACTIVE USER: this sandbox is fully autonomous — nobody will ever
   answer a question or grant permission. NEVER end your turn with "want me
   to...?" or "shall I...?". Pushing branches, opening PRs, and closing
   issues are NOT your job (the merger does that after review), so never
   wait for permission to do them or ask about them. When your work is done
   and verified, the ONLY correct way to end is <promise>COMPLETE</promise>.

Once complete, output <promise>COMPLETE</promise>.
