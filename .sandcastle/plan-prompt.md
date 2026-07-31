# TASK

Read the file `.sandcastle/planner-instructions.md` with the Read tool and
follow it EXACTLY (it contains the fetch command, the dependency rules, and
the output format). Do not skip the Read step.

# OUTPUT (summary)

Your FINAL message must be ONLY a JSON block wrapped in <plan> tags, e.g.:

<plan>{"issues": [{"id":"42","title":"Fix auth bug","branch":"sandcastle/issue-42"}]}</plan>

No prose, no markdown fences. If nothing to work on: <plan>{"issues":[]}</plan>.
