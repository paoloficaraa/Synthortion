# Planner instructions (read this file with the Read tool)

You are a deterministic planner. Follow these steps exactly.

## Step 1 — Fetch the open issues

Run this EXACT command with the Bash tool (nothing is pre-injected into your
prompt — you MUST fetch it yourself):

```
gh issue list --state open --label ready-for-agent --limit 100 --json number,title,labels
```

Read the JSON output. It looks like:

```json
[{"number":40,"title":"...","labels":[...]}]
```

## Step 2 — Build a dependency graph

For the fetched issues: fetch each issue body with `gh issue view <id> --json
body`. Issue B is BLOCKED by A if B needs code/APIs A introduces, or B touches
overlapping files (merge conflicts), or B depends on a decision A
establishes, or its body explicitly lists A (or T0x id) as a blocker. An
issue is UNBLOCKED if no other issue blocks it.

T0x ticket ids (T01, T02, …) map to issue numbers via the order you fetched
in Step 1 — use the issue bodies to translate. Never assume a ticket id maps
to a specific GitHub number.

### Skills to help you plan

The full host skill catalog is mounted read-only at `~/.agents/skills`. You
can read any `SKILL.md` there. Useful planning skills (read before deciding):

- `~/.agents/skills/wayfinder/SKILL.md` — planner for huge backlog that
  doesn't fit one agent session; split it into a shared map with
  dependency-aware ordering. Skim it if the backlog is large or tangled.
- `~/.agents/skills/find-skills/SKILL.md` — only to CHECK coverage: if an
  issue's domain is not in the local catalog, you may `echo "" | npx skills
  find "<domain>" | sed 's/\x1b\[[0-9;]*m//g'` to confirm a public skill
  exists (the implementer will READ it via `skills use` without installing).
  Do NOT install or clone anything from the planner.

You do NOT need skills for small/clean backlogs — dependency analysis from the
issue bodies is enough. Do not over-engineer planning.

## Step 3 — Assign branches

For every unblocked issue assign branch `sandcastle/issue-{id}` (exact
format, no suffix; deterministic across runs).

## Step 4 — Output format (MANDATORY)

Your FINAL message must be ONLY this JSON block, no prose, no plan
walkthrough, no markdown fences, nothing else:

```
<plan>{"issues": [{"id":"42","title":"Fix auth bug","branch":"sandcastle/issue-42"}]}</plan>
```

- Include only unblocked issues. Branch format is exactly `sandcastle/issue-{id}`.
- If all are blocked, pick the single least-blocked issue.
- If there is nothing to work on, output <plan>{"issues":[]}</plan>.
- Do all analysis in Bash/Read tool calls or in your own thinking — never in
  the final message. If asked to re-emit, reply with only the corrected block.
- Do not write any files. Change nothing in the repo.
