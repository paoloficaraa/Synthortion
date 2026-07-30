# SKILLS

**Tone.** Load the `caveman` skill at `wenyan-ultra` intensity BEFORE any action.
It stays active for every response (code, tests, and error strings remain normal;
the skill auto-drops for security warnings). Cuts output tokens by 65%+.

**Task-specific skill.** After reading the issue, select the best matching skill(s)
using the mapping table below. Check the issue's **labels** first, then **title**.

| When issue …                                          | Load these skills                                        |
|-------------------------------------------------------|----------------------------------------------------------|
| Label: `ui`, `frontend`, `visual`, `component`        | `impeccable`, `ui-ux-pro-max`                            |
| Title contains: `UI`, `visualizer`, `layout`          | `impeccable`, `ui-ux-pro-max`                            |
| Label: `bug`, `fix`, `debug`                          | `systematic-debugging`, `tdd`                            |
| Label: `test`, `testing`                              | `tdd`, `webapp-testing`                                  |
| Label: `architecture`, `refactor`                     | `codebase-design`, `graphify`                            |
| Label: `spec`, `prd`, `documentation`                 | `ubiquitous-language` (write specs), also check title    |
| Title contains: `spec`, `PRD` **AND** `UI`, `component` | `impeccable`, `ui-ux-pro-max` (spec → implementation)   |
| Label: `dsp`, `audio`, `engine`                       | `tdd`, `codebase-design`                                 |
| None of the above                                     | `impeccable` (UI default), `graphify` (exploration)      |

Load and follow the selected skill(s) alongside `caveman`.

> ⚠️ Do NOT invoke `find-skills`. It searches the public ecosystem and misses locally
> installed skills. Use the direct mapping above.

# TASK

Resolve issue {{TASK_ID}}: {{ISSUE_TITLE}}

1. Pull the issue using `gh issue view {{TASK_ID}}`.
2. If the issue references a parent PRD or spec, pull that in too.
3. Work exclusively on the issue specified. Do not expand scope.
4. Work on branch `{{BRANCH}}`.
5. Make atomic commits with messages following [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) (e.g., `feat(ui): add GainMeter component`).
6. Verify the build and linting before pushing.

# CONTEXT

## Recent commits

<recent-commits>
!`git log -n 10 --format="%H%n%ad%n%B---" --date=short`
</recent-commits>

## Project
@.sandcastle/project-context.md

## Ubiquitous Language
Always use terms from [UBIQUITOUS_LANGUAGE.md](../UBIQUITOUS_LANGUAGE.md). Examples:
- **WebView UI** (not "web UI" or "HTML editor")
- **Value arc** (not "circus" or "active arc")
- **Glitch Brutalism** (not "dark theme" or "industrial theme")
- **IPC bridge** (not "message bus" or "JS bridge")

## Coding Standards
- **File structure**: All WebView UI code lives in `ui/` (React/Vite). C++ DSP code lives in `Source/`.
- **Styling**: Use Tailwind CSS for layout/spacing. Use CSS Modules (`.module.css`) for component-scoped styles.
- **State**: Hoist all parameter state to `App.tsx` (controlled components).
- **3D**: Use React Three Fiber (`@react-three/fiber`) exclusively for `FftVisualizer`.
- **Colors**: Use the **Vintage Industrial palette** (`#0f0e0e`, `#f6f6f6`, `#c7c3ba`).
- **Accessibility**: Every interactive control must support `focus-visible` and keyboard navigation.

## Build Commands
```bash
npm install       # Install dependencies
npm run dev       # Start Vite dev server
npm run build     # Build for production
npm run lint      # Run ESLint
npm run test      # Run unit tests
```

## Out of Scope
- Implementing the C++ IPC bridge (handled by separate issue).
- Real FFT analysis (simulated noise is acceptable for now).
- DAW-specific integration (VST/AU/AAX).

# EXECUTION

If applicable, use RGR:
1. RED: write one test (e.g. Vitest/Playwright)
2. GREEN: implement to pass that test
3. REPEAT until done
4. REFACTOR

# FEEDBACK

Before committing:
- `npm run typecheck` — must pass, zero errors
- `npm run lint` — must pass
- Verify visual fidelity against the **Glitch Brutalism** spec.

# COMMIT

Commit message format:
`type(scope): description`
Example: `feat(ui): implement 3-column layout for VstLayout`

# FINAL RULES

If task not complete, leave a detailed summary of:
1. What was achieved.
2. Where exactly you stopped (file/line).
3. What is the immediate next step.
