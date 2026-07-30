# PROJECT

@.sandcastle/project-context.md

# CONTEXT

## Branch to merge

`{{BRANCH}}` into `{{TARGET_BRANCH}}`

## Change summary

!`git log {{TARGET_BRANCH}}..{{BRANCH}} --oneline`

# TASK

Merge `{{BRANCH}}` into `{{TARGET_BRANCH}}` using a clean, fast-forward or squash merge depending on the current project state.

1. **Conflict Check**: Ensure there are no unresolved merge conflicts.
2. **Build Verification**: Run `npm run build` and `npm run lint` on the resulting state to ensure no regression.
3. **Visual Smoke Test**: Confirm that the **WebView UI** renders correctly and the **Vintage Industrial palette** is preserved.
After all branches merged, make a single commit summarizing the merge.

# CLOSE ISSUES

Close each merged branch's issue:

`gh issue close <ID> --comment "Completed by Sandcastle"`

Issues:

{{ISSUES}}

Once you've merged everything, output <promise>COMPLETE</promise>.
