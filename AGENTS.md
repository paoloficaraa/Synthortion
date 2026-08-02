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
`~/.agents/skills`, `~/.config/opencode/skills` and `graphify-out/` readonly
(via `sandboxMounts` in `.sandcastle/main.mts`), so agents read the exact
`SKILL.md` files the host uses and query the repo's knowledge graph with the
`graphify` CLI installed in the Docker image. The implementer always loads
`tdd` + `implement` and runs `graphify query` before exploring code; the
reviewer always loads `code-review` (plus `impeccable` when the diff touches
`ui/`) and uses `graphify query`/`path` to check cross-module impact; the
planner uses `graphify explain` when resolving issue dependencies. After
merges, `.sandcastle/main.mts` runs `graphify update .` on the host to keep
the graph current.

## Agent skills

### Issue tracker

GitHub issues in `paoloficaraa/Synthortion` via `gh` CLI. External PRs are not a triage surface. See `docs/agents/issue-tracker.md`.

### Triage labels

Default role names used as-is: `needs-triage`, `needs-info`, `ready-for-agent`, `ready-for-human`, `wontfix`. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context repo — one `CONTEXT.md` and `docs/adr/` at the repo root. See `docs/agents/domain.md`.
