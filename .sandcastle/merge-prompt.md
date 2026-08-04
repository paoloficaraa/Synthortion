# TASK: Merge branches and close issues

You are the **merger agent**. Your job is to merge completed branches into the current branch AND close the corresponding GitHub issues. Both are required — you have not finished until every issue is closed.

## Input

Branches to merge:
{{BRANCHES}}

Issues to close (one per merged branch, same order):
{{ISSUES}}

---

## Procedure

For **each branch** in the list above, in order:

1. **Merge the branch**
   ```bash
   git merge <branch> --no-edit
   ```
   Resolve any conflicts by reading both sides; choose the correct resolution.

2. **Verify the merge works**
   ```bash
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build --target SynthortionTests
   ./build/plugin/SynthortionTests
   ```
   Fix any failures before proceeding to the next branch.

3. **CLOSE THE CORRESPONDING ISSUE IMMEDIATELY** (MANDATORY)
   The issue ID is the number at the start of the corresponding line in {{ISSUES}}.
   ```bash
   gh issue close <ID> --comment "Completed by Sandcastle"
   ```
   Example: if the issue line is `- 43: T03 — Mappe Input/Output: GainMeter + TRIM`, run:
   ```bash
   gh issue close 43 --comment "Completed by Sandcastle"
   ```
   Verify the closure succeeded before moving to the next branch.

---

## After ALL branches are merged AND all issues are closed:

- Make a single commit summarizing the merge (if there were any merge conflicts resolved)
- Output `<promise>COMPLETE</promise>`

---

## Critical Rules

- **Do NOT output `<promise>COMPLETE</promise>` until ALL issues are closed.**
- **Close each issue immediately after merging its branch**, not at the end.
- If `gh issue close` fails, retry once. If it still fails, report the error but continue.
- The planner will re-schedule any issue that remains open — you MUST close them.