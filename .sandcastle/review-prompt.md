# TASK
Review branch {{BRANCH}} against {{TARGET_BRANCH}}: improve clarity,
consistency and maintainability while preserving exact functionality.

# CONTEXT
Fetch and read the outputs of:
- `git diff {{TARGET_BRANCH}}...{{BRANCH}}`
- `git log {{TARGET_BRANCH}}..{{BRANCH}} --oneline`

# EXPLORING THE CHANGE

Fetch and read the outputs of:
- `git diff {{TARGET_BRANCH}}...{{BRANCH}}`
- `git log {{TARGET_BRANCH}}..{{BRANCH}} --oneline`

To check whether the change affects other parts of the codebase, follow the
imports of the changed files and read the callers yourself — there is no
knowledge graph. Keep the blast-radius check focused on the actual symbols
the diff touches.

# SKILLS

The full host skill catalog is mounted read-only at `~/.agents/skills`.
Use it to raise review quality:

1. Always load `~/.agents/skills/code-review/SKILL.md` — it runs the review
   along two axes (Standards + Spec) in parallel.
2. If the diff touches the webview (`ui/`), also load these from
   `~/.agents/skills/` as relevant and apply them:
   - `frontend-design` — visual design / hierarchy
   - `impeccable` — broad frontend audit (UX, a11y, motion, edge cases)
   - `ui-ux-pro-max` — searchable styles/palettes/fonts/UX guidelines
   - `web-design-guidelines` — Web Interface Guidelines compliance
   - `webapp-testing` — Playwright-based verification of UI behaviour
   Pick only the ones that match what actually changed; do not load all of
   them for a one-line cosmetic edit.
3. For a backend/C++ change, in addition to `code-review` consider
   `codebase-design` (deep-module vocabulary) and `diagnosing-bugs` if the
   diff looks like a bug fix.

## Fallback — external skill via find-skills (only if needed)

If the diff touches a domain the local catalog above does NOT cover (e.g.
a framework/dep with no matching installed skill), discover and READ an
external skill WITHOUT installing — the host catalog is read-only by design.
1. Read `~/.agents/skills/find-skills/SKILL.md`.
2. Search non-interactively:
   `echo "" | npx skills find "<domain keywords>" | sed 's/\x1b\[[0-9;]*m//g'`
3. Vet: prefer install count ≥1K and authoritative sources; skip anything
   <100 installs or from unknown authors.
4. READ the skill's instructions without installing:
   `npx skills use <owner/repo@skill> > /tmp/skill.md` then open it with
   the Read tool. Do NOT run `skills add` (it writes to the read-only
   mount and fails) — `skills use` is the read-only substitute.
5. If nothing relevant comes back, review with `code-review` alone.

# CHECK
- Unnecessary complexity/nesting, redundant code, unclear names, nested
  ternaries (prefer switch/if-else), unneeded comments; clarity over brevity.
- Correctness: matches intent, edge cases, tests for new behaviour, unsafe
  casts / `any`, injection or credential leaks.
- Follow @.sandcastle/CODING_STANDARDS.md.

# EXECUTE
If improvements: edit on this branch, verify, then commit:
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target SynthortionTests
./build/plugin/SynthortionTests

If already clean, do nothing. Never change behaviour — only how.
Output <promise>COMPLETE</promise>.
