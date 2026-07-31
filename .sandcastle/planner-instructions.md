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

For the fetched issues: issue B is BLOCKED by A if B needs code/APIs A
introduces, or B touches overlapping files (merge conflicts), or B depends on
a decision A establishes. An issue is UNBLOCKED if no other issue blocks it.

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
