# TASK
Review branch {{BRANCH}} against {{TARGET_BRANCH}}: improve clarity,
consistency and maintainability while preserving exact functionality.

# CONTEXT
Fetch and read the outputs of:
- `git diff {{TARGET_BRANCH}}...{{BRANCH}}`
- `git log {{TARGET_BRANCH}}..{{BRANCH}} --oneline`

# SKILLS

1. Read `.sandcastle/skills/SKILLS.md` with the Read tool.
2. Load `.sandcastle/skills/code-review/SKILL.md` (always).
3. If the diff touches the webview (`ui/`), also load
   `.sandcastle/skills/impeccable/SKILL.md`.

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
