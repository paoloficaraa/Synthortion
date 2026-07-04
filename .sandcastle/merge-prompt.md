# TASK

Merge the following branches into the current branch:

{{BRANCHES}}

For each branch:

1. Run `git merge <branch> --no-edit`
2. If there are merge conflicts, resolve them intelligently by reading both sides and choosing the correct resolution
3. If the merged `plugin/CMakeLists.txt` changed, run `npm run configure` once
4. Run `npm run typecheck` and `npm run test` to verify everything compiles (ccache makes subsequent builds ~30s). **Do NOT delete the build directory (`rm -rf build`)** — it wastes ccache and forces a clean rebuild.
5. If the build fails, fix the issues before proceeding to the next branch

After all branches are merged, make a single commit summarizing the merge.

# CLOSE ISSUES

For each branch that was merged, close its issue using the following command:

`gh issue close <ID> --comment "Completed by Sandcastle"`

Here are all the issues:

{{ISSUES}}

Once you've merged everything you can, output <promise>COMPLETE</promise>.
