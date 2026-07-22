# SKILLS

**Task-specific skill.** After reading the diff, invoke `find-skills` to discover
the best matching review skill (e.g. `code-review` for general review,
`impeccable` for UI polish, `systematic-debugging` for bugs). Load and follow.

# TASK

Review code changes on branch `{{BRANCH}}` and improve clarity, consistency,
and maintainability while preserving exact functionality.

# CONTEXT

## Branch diff

!`git diff {{TARGET_BRANCH}}...{{BRANCH}}`

## Commits on this branch

!`git log {{TARGET_BRANCH}}..{{BRANCH}} --oneline`

# PROJECT

@.sandcastle/project-context.md

# CODING STANDARDS

@.sandcastle/CODING_STANDARDS.md

# REVIEW PROCESS

1. **Understand the change**: Read the diff and commits to understand intent.

2. **Analyze for improvements**:
   - Reduce unnecessary complexity and nesting
   - Eliminate redundant code and abstractions
   - Improve readability through clear names
   - Consolidate related logic; remove obvious comments
   - Avoid nested ternaries — prefer `switch`/`if-else`
   - Choose clarity over brevity

3. **Check correctness**:
   - Does implementation match intent? Edge cases handled?
   - Are new/changed behaviours covered by tests?
   - Unsafe casts, `any` types, unchecked assumptions?
   - Injection vulnerabilities, credential leaks, security issues?

4. **Check coding standards**: Verify compliance with @.sandcastle/CODING_STANDARDS.md

5. **Maintain balance**: Avoid over-simplification that reduces clarity,
   maintainability, or debuggability. Preserve helpful abstractions.

6. **Preserve functionality**: Never change what the code does — only how.

# EXECUTION

If improvements to make:
1. Apply changes directly on this branch
2. If `plugin/CMakeLists.txt` changed: `npm run configure` first
3. Run `npm run typecheck` and `npm run test` (ccache makes subsequent builds ~30s). **Never `rm -rf build`.**
4. Commit describing the refinements

If code is already clean: do nothing.

Once complete, output <promise>COMPLETE</promise>.