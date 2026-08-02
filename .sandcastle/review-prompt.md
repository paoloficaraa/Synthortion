# TASK
Review branch {{BRANCH}} against {{TARGET_BRANCH}}: improve clarity,
consistency and maintainability while preserving exact functionality.

# CONTEXT
Fetch and read the outputs of:
- `git diff {{TARGET_BRANCH}}...{{BRANCH}}`
- `git log {{TARGET_BRANCH}}..{{BRANCH}} --oneline`

# GRAPHIFY (architecture awareness)
A knowledge graph is mounted read-only at `graphify-out/` and the `graphify`
tools/skill are installed in the sandbox. Use them to ask natural-language
questions about concepts in the diff and about the modules the change
touches — they return scoped subgraphs showing cross-module relationships, so
you can check whether the change affects other parts of the codebase it
should stay consistent with.

# SKILLS

1. Load `~/.agents/skills/code-review/SKILL.md` (always).
2. If the diff touches the webview (`ui/`), also load
   `~/.agents/skills/impeccable/SKILL.md`.

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
