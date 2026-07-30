# PROJECT

@.sandcastle/project-context.md

# ISSUES

Open issues (pre-filtered to **ready-for-agent** or **ready-for-work**):

<issues-json>
!`gh issue list --state open --limit 100 --json number,title,body,labels,comments --jq '[.[] | select(.labels[].name == "ready-for-agent" or .labels[].name == "ready-for-work") | {number, title, body, labels: [.labels[].name], comments: [.comments[].body]}]'`
</issues-json>

# TASK

Build a dependency graph. Issue B is **blocked by** A if:
- B requires code/infrastructure A introduces (e.g., WebView UI must exist before wiring IPC bridge)
- A and B modify overlapping files/modules (likely merge conflicts)
- B depends on a decision/API shape A establishes (e.g., parameter IDs, layout structure, or APVTS state shape)

An issue is **unblocked** if it has zero blocking dependencies on other open issues.

For each unblocked issue, assign branch name `sandcastle/issue-{id}` (deterministic, no slug).

# OUTPUT

JSON in `<plan>` tags only — do NOT write any file to disk.

<plan>
{"issues": [{"id": "42", "title": "Implement WebView UI layout", "branch": "sandcastle/issue-42"}]}
</plan>

Include only unblocked issues. If all blocked, include the single highest-priority candidate.
If no issues at all: `<plan>{}{"issues": []}</plan>`. Always emit `<plan>` tags.