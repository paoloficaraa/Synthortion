# SKILLS

**Task-specific skill.** After reading the diff, invoke `find-skills` to discover
the best matching review skill. Common matches for this repo:
- `code-review` → General logic, architecture, and safety
- `impeccable` → Visual fidelity, CSS/Tailwind precision, and accessibility
- `graphify` → Module dependencies and file structure consistency

Load and follow the selected skill.

# TASK

Review code changes on branch `{{BRANCH}}` and improve clarity, consistency,
and maintainability while preserving exact functionality.

# CONTEXT

## Branch diff — summary

!`git diff {{TARGET_BRANCH}}...{{BRANCH}} --stat`

Do NOT run `git diff {{TARGET_BRANCH}}...{{BRANCH}}` wholesale — on large
cleanups the full diff can exceed the OS argv limit at sandbox spawn and
crash the reviewer with `spawn E2BIG`. Instead, inspect the change one file at a time:

!`git diff --name-only {{TARGET_BRANCH}}...{{BRANCH}}`

For each path listed above, run on demand:
    `git diff {{TARGET_BRANCH}}...{{BRANCH}} -- <path>`

## Commits on this branch

!`git log {{TARGET_BRANCH}}..{{BRANCH}} --oneline`

# PROJECT
@.sandcastle/project-context.md

# UBIQUITOUS LANGUAGE
Always ensure terms in code and comments align with [UBIQUITOUS_LANGUAGE.md](../UBIQUITOUS_LANGUAGE.md).

# CODING STANDARDS
@.sandcastle/CODING_STANDARDS.md

# REVIEW PROCESS

1. **Understand the change**: Read the `--stat` summary, the changed-file list, and the commit subject lines to get scope and intent. Load hunks selectively to keep the prompt bounded.

2. **Analyze for improvements**:
   - **Visual Fidelity**: Does it adhere to **Glitch Brutalism**? (Sharp corners, hard shadows, `#0f0e0e`/`#f6f6f6`/`#c7c3ba`).
   - **Performance**: Are React components avoiding unnecessary re-renders? Is Three.js usage optimized?
   - **Accessibility**: Are `focus-visible` and keyboard tab-orders correct for the new controls?
   - **Readability**: Reduce nesting, eliminate redundant abstractions, and prefer clarity over brevity.

3. **Check correctness**:
   - Does implementation match the issue spec?
   - Are parameter ranges and IDs aligned with the **APVTS**?
   - Is state hoisted correctly to `App.tsx`?

# EXECUTION

If you find improvements to make:
1. Apply changes directly on this branch
2. If `Source/CMakeLists.txt` changed: `npm run configure` first
3. Run `npm run typecheck` and `npm run test` (ccache makes subsequent builds ~30s). **Never `rm -rf build`.**
4. Commit describing refinements using [Conventional Commits](https://www.conventionalcommits.org/) (e.g., `refactor(ui): simplify Knob drag logic`)

If code is already clean: do nothing.

Once complete, output `<promise>COMPLETE</promise>`.
