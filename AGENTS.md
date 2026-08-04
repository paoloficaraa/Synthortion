## graphify

This project has a knowledge graph at graphify-out/ with god nodes, community structure, and cross-file relationships.

When the user types `/graphify`, invoke the `skill` tool with `skill: "graphify"` before doing anything else.

Rules:
- For codebase questions, first run `graphify query "<question>"` when graphify-out/graph.json exists. Use `graphify path "<A>" "<B>"` for relationships and `graphify explain "<concept>"` for focused concepts. These return a scoped subgraph, usually much smaller than GRAPH_REPORT.md or raw grep output.
- Dirty graphify-out/ files are expected after hooks or incremental updates; dirty graph files are not a reason to skip graphify. Only skip graphify if the task is about stale or incorrect graph output, or the user explicitly says not to use it.
- If graphify-out/wiki/index.md exists, use it for broad navigation instead of raw source browsing.
- Read graphify-out/GRAPH_REPORT.md only for broad architecture review or when query/path/explain do not surface enough context.
- After modifying code, run `graphify update .` to keep the graph current (AST-only, no API cost).

## Sandcastle skills

`.sandcastle/` orchestrates parallel agent pipelines. Every sandbox bind-mounts
`~/.agents/skills` readonly (via `sandboxMounts` in `.sandcastle/main.mts`), so
agents read the exact `SKILL.md` files the host uses. The knowledge graph is
HOST-ONLY: `graphify` is not installed in the Docker image and `graphify-out/`
is not mounted into sandboxes (removed in commit 502cd44), so sandbox agents
map the repo with `ls`/`find` and import-following instead of graph queries
(see implement-prompt.md / review-prompt.md).

The skill contract per agent:
- **implementer** — MUST load `tdd` + `implement` first (Step 0 in
  implement-prompt.md), plus context-specific skills from the catalog map;
  must NEVER close issues (the merger closes them after merging); must signal
  COMPLETE as soon as the work is done and verified.
- **reviewer** — always loads `code-review` (plus `impeccable` when the diff
  touches `ui/`); runs under an idle + wall-clock watchdog so a stalled
  review cannot hold the loop hostage.
- **planner** — follows `.sandcastle/planner-instructions.md`; no graphify.

After merges, run `graphify update .` on the host to keep the local graph
current — the pipeline does not do this automatically.

## Agent skills

### Issue tracker

GitHub issues in `paoloficaraa/Synthortion` via `gh` CLI. External PRs are not a triage surface. See `docs/agents/issue-tracker.md`.

### Triage labels

Default role names used as-is: `needs-triage`, `needs-info`, `ready-for-agent`, `ready-for-human`, `wontfix`. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context repo — one `CONTEXT.md` and `docs/adr/` at the repo root. See `docs/agents/domain.md`.
