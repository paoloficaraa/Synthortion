# PROJECT

@.sandcastle/project-context.md

# TASK

Merge the following branches into the current branch:

{{BRANCHES}}

For each branch:
1. Run `git merge <branch> --no-edit`
2. Resolve merge conflicts intelligently by reading both sides
3. If merged `plugin/CMakeLists.txt` changed, run `npm run configure` once
4. Run `npm run typecheck` and `npm run test` to verify. **Never `rm -rf build`.** (ccache)
5. If build fails, fix before next branch

After all branches merged, make a single commit summarizing the merge.

# CLOSE ISSUES

Close each merged branch's issue:

`gh issue close <ID> --comment "Completed by Sandcastle"`

Issues:

{{ISSUES}}

Once you've merged everything, output <promise>COMPLETE</promise>.