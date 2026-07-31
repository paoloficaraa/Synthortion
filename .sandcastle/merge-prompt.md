# TASK
Merge the following branches into the current branch:

{{BRANCHES}}

For each branch:
1. `git merge <branch> --no-edit`
2. Resolve conflicts by reading both sides; choose the correct resolution.
3. Verify everything works:
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build --target SynthortionTests
   ./build/plugin/SynthortionTests
4. Fix failures before the next branch.

After all merges, make a single commit summarizing the merge.

# CLOSE ISSUES
Close each merged issue:
`gh issue close <ID> --comment "Completed by Sandcastle"`

Issues:
{{ISSUES}}

Once done, output <promise>COMPLETE</promise>.
