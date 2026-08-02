// Parallel Planner with Review — four-phase orchestration loop
//
// This template drives a multi-phase workflow:
//   Phase 1 (Plan):             A smart-routed agent analyzes open issues, builds a
//                               dependency graph, and outputs a <plan> JSON
//                               listing unblocked issues with branch names.
//   Phase 2 (Execute + Review): For each issue, a sandbox is created via
//                               createSandbox(). The implementer runs first
//                               (100 iterations). If it produces commits, a
//                               reviewer runs in the same sandbox on the same
//                               branch (1 iteration). All issue pipelines run
//                               concurrently via Promise.allSettled().
//   Phase 3 (Merge):            A single agent merges all completed branches
//                               into the current branch.
//
// The outer loop repeats up to MAX_ITERATIONS times so that newly unblocked
// issues are picked up after each round of merges.
//
// Usage:
//   npx tsx .sandcastle/main.mts
// Or add to package.json:
//   "scripts": { "sandcastle": "npx tsx .sandcastle/main.mts" }

import * as sandcastle from "@ai-hero/sandcastle";
import { docker } from "@ai-hero/sandcastle/sandboxes/docker";
import { execSync } from "node:child_process";
import { z } from "zod";

const REPO_ROOT = execSync("git rev-parse --show-toplevel", {
  encoding: "utf8",
}).trim();
process.chdir(REPO_ROOT);

const SHELL = process.platform === "win32" ? undefined : "bash";

function sh(cmd: string, cwd?: string): string {
  try {
    return execSync(cmd, {
      encoding: "utf8",
      cwd: cwd ?? REPO_ROOT,
      shell: SHELL,
      maxBuffer: 16 * 1024 * 1024,
    }).trim();
  } catch (error) {
    return `(command failed: ${String(error)})`;
  }
}

const baseBranch = sh("git branch --show-current", REPO_ROOT) || "main";

// The planner emits its plan as JSON inside <plan> tags; Output.object extracts
// and validates it against this schema. We use Zod here, but any Standard
// Schema validator works just as well — Valibot, ArkType, etc. See
// https://standardschema.dev.
const planSchema = z.object({
  issues: z.array(
    z.object({ id: z.string(), title: z.string(), branch: z.string() }),
  ),
});

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

// Maximum number of plan→execute→merge cycles before stopping.
// Raise this if your backlog is large; lower it for a quick smoke-test run.
const MAX_ITERATIONS = 10;

// Hooks run inside the sandbox before the agent starts each iteration.
// NOTE: do NOT run `npm install` here, not even for `ui/` — it is the
// corruption vector we hit: planner and merger run in HEAD mode (host repo
// mounted read-write), so an npm install executed inside the container
// rewrites the HOST node_modules (root or ui/) with Linux binaries (e.g.
// @esbuild/linux-x64), breaking host `tsx` or `vite` on Windows.
// Implementer worktrees get node_modules via copyToWorktree below;
// the git submodule hook stays so fresh worktrees get JUCE.
const hooks = {
  sandbox: {
    onSandboxReady: [
      // JUCE is a git submodule — populate it in every fresh worktree before agents build.
      { command: "git submodule update --init --recursive" },
    ],
  },
};

// Copy node_modules from the host into the worktree before each sandbox
// starts. Avoids a full npm install from scratch; the hook above handles
// platform-specific binaries and any packages added since the last copy.
// Skills are NOT copied here — they are bind-mounted read-only from the host
// by sandboxMounts below.
const copyToWorktree = ["node_modules"];

// Agent env: shrink Claude Code's system prompt (skill/agent listings are
// large). This keeps total input below the threshold where Claude Code's CCR
// content-retrieval mangles prompt text for third-party (Omniroute-routed)
// models, and cuts token cost per run.
const agentEnv = {
  CLAUDE_CODE_SIMPLE_SYSTEM_PROMPT: "1",
  CLAUDE_CODE_DISABLE_BUNDLED_SKILLS: "1",
};

// Host skill directories, mounted read-only into every sandbox so agents can
// read the exact SKILL.md files the host uses. This replaces the old
// `.sandcastle/skills` copy (which was gitignored and never populated). The
// full skill catalog (~/.agents/skills on this host) is exposed to every
// agent, who picks the right skills per issue (see the prompt files).
//
// The mount stays READ-ONLY on purpose: for domains the local catalog does
// not cover, agents use the find-skills skill in fallback mode — they READ
// external skills via `npx skills use <owner/repo@skill>` (streams the full
// SKILL.md to stdout, no install) instead of `skills add` (which would try
// to write into this mount and fail). See the SKILLS sections in
// implement-prompt.md / review-prompt.md / planner-instructions.md.
//
// NOTE: do NOT add mount host paths that don't exist on the host — sandcastle
// 0.12 validates every mount via resolveUserMounts() and THROWS
// `Mount hostPath does not exist` at startup, aborting the whole run.
// (`~/.config/opencode/skills` was previously listed here but does not exist
// on this host; agents read skills from ~/.agents/skills instead.)
//
// NOTE: sandboxPath MUST be an absolute POSIX path. The lib resolves relative
// sandbox paths with `path.resolve(SANDBOX_REPO_DIR, …)`, which on Windows
// mangles them to `C:\home\agent\workspace\…` and makes docker fail with
// "too many colons" (the drive colon is parsed as a mount separator).
// SANDBOX_REPO_DIR mirrors the lib's constant — the worktree always lives at
// /home/agent/workspace in the container (see Dockerfile WORKDIR).
const SANDBOX_REPO_DIR = "/home/agent/workspace";
const SANDBOX_HOME_DIR = "/home/agent";
const sandboxMounts = [
  {
    hostPath: "~/.agents/skills",
    sandboxPath: `${SANDBOX_HOME_DIR}/.agents/skills`,
    readonly: true,
  },
];

// Shortcut for the claudeCode provider with our shared env.
// effort: "max" forces maximum reasoning effort (model_reasoning_effort=max)
// on every agent — planner, implementer, reviewer and merger.
const cc = (model: string) =>
  sandcastle.claudeCode(model, { env: agentEnv, effort: "max" });

// Docker provider that always mounts the host skills read-only.
const sandboxWithSkills = () => docker({ mounts: sandboxMounts });

// ---------------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------------

for (let iteration = 1; iteration <= MAX_ITERATIONS; iteration++) {
  console.log(`\n=== Iteration ${iteration}/${MAX_ITERATIONS} ===\n`);

  // -------------------------------------------------------------------------
  // Phase 1: Plan
  //
  // The planning agent (auto/smart routing) reads the open issue list,
  // builds a dependency graph, and selects the issues that can be worked in
  // parallel right now (i.e., no blocking dependencies on other open issues).
  //
  // It outputs a <plan> JSON block — Output.object parses and validates it.
  // -------------------------------------------------------------------------
  const plan = await sandcastle.run({
    hooks,
    sandbox: sandboxWithSkills(),
    name: "planner",
    cwd: REPO_ROOT,
    // One iteration is enough: the planner just needs to read and reason,
    // not write code. (Structured output requires maxIterations: 1.)
    maxIterations: 1,
    // auto/smart: verified to work with Bash tool calls on this Omniroute.
    // (auto/best-reasoning resolves to ambiguous model ids -> "400 Ambiguous
    // model", and non-Claude backends cannot resolve CCR placeholders.)
    agent: cc("auto/reasoning"),
    promptFile: "./.sandcastle/plan-prompt.md",
    // NOTE: we deliberately do NOT inject the issue list or long instructions
    // into the prompt. Claude Code's CCR feature replaces injected content
    // with `[CCR retrieve hash=...]` placeholders that third-party
    // (Omniroute-routed) models cannot resolve — the planner would report
    // "no issues JSON". Instead the planner READS the full instructions from
    // .sandcastle/planner-instructions.md and FETCHES the issue list itself
    // via the Bash tool; tool results are delivered to any model reliably.
    // (gh + GH_TOKEN are baked into the sandbox.)
    //
    // Extract and validate the <plan> JSON into a typed object. Throws
    // StructuredOutputError if the tag is missing, the JSON is malformed, or
    // validation fails — which aborts the loop.
    output: sandcastle.Output.object({
      tag: "plan",
      schema: planSchema,
      // Models often write prose instead of the <plan> block after tool
      // calls. Retrying resumes the same session and asks it to re-emit
      // ONLY the JSON block.
      maxRetries: 2,
    }),
  });

  const issues = plan.output.issues;

  if (issues.length === 0) {
    // No unblocked work — either everything is done or everything is blocked.
    console.log("No unblocked issues to work on. Exiting.");
    break;
  }

  console.log(
    `Planning complete. ${issues.length} issue(s) to work in parallel:`,
  );
  for (const issue of issues) {
    console.log(`  ${issue.id}: ${issue.title} → ${issue.branch}`);
  }

  // -------------------------------------------------------------------------
  // Phase 2: Execute + Review
  //
  // For each issue, create a sandbox via createSandbox() so the implementer
  // and reviewer share the same sandbox instance per branch. The implementer
  // runs first; if it produces commits, the reviewer runs in the same sandbox.
  //
  // Promise.allSettled means one failing pipeline doesn't cancel the others.
  // -------------------------------------------------------------------------

  const settled = await Promise.allSettled(
    issues.map(async (issue) => {
      const sandbox = await sandcastle.createSandbox({
        branch: issue.branch,
        sandbox: sandboxWithSkills(),
        hooks,
        copyToWorktree,
        cwd: REPO_ROOT,
      });

      try {
        // Run the implementer
        const implement = await sandbox.run({
          name: "implementer",
          maxIterations: 100,
          // auto/coding: verified to work with Bash tool calls on this
          // Omniroute. (gemini/gemini-3.1-flash-lite errors with "missing
          // thought_signature" whenever it calls a tool.)
          agent: cc("auto/coding"),
          promptFile: "./.sandcastle/implement-prompt.md",
          cwd: REPO_ROOT,
          promptArgs: {
            TASK_ID: issue.id,
            ISSUE_TITLE: issue.title,
            BRANCH: issue.branch,
            RECENT_COMMITS: sh("git log -n 5 --oneline", REPO_ROOT),
          },
        });

        // Only review if the implementer produced commits
        if (implement.commits.length > 0) {
          const review = await sandbox.run({
            name: "reviewer",
            maxIterations: 1,
            agent: cc("auto/smart"),
            promptFile: "./.sandcastle/review-prompt.md",
            cwd: REPO_ROOT,
            promptArgs: {
              BRANCH: issue.branch,
            },
          });

          // Merge commits from both runs so the merge phase sees all of them.
          // Each sandbox.run() only returns commits from its own run.
          return {
            ...review,
            commits: [...implement.commits, ...review.commits],
          };
        }

        return implement;
      } finally {
        await sandbox.close();
      }
    }),
  );

  // Log any agents that threw (network error, sandbox crash, etc.).
  for (const [i, outcome] of settled.entries()) {
    if (outcome.status === "rejected") {
      console.error(
        `  ✗ ${issues[i]!.id} (${issues[i]!.branch}) failed: ${outcome.reason}`,
      );
    }
  }

  // Only pass branches that actually produced commits to the merge phase.
  // An agent that ran successfully but made no commits has nothing to merge.
  const completedIssues = settled
    .map((outcome, i) => ({ outcome, issue: issues[i]! }))
    .filter(
      (entry) =>
        entry.outcome.status === "fulfilled" &&
        entry.outcome.value.commits.length > 0,
    )
    .map((entry) => entry.issue);

  const completedBranches = completedIssues.map((i) => i.branch);

  console.log(
    `\nExecution complete. ${completedBranches.length} branch(es) with commits:`,
  );
  for (const branch of completedBranches) {
    console.log(`  ${branch}`);
  }

  if (completedBranches.length === 0) {
    // All agents ran but none made commits — nothing to merge this cycle.
    console.log("No commits produced. Nothing to merge.");
    continue;
  }

  // -------------------------------------------------------------------------
  // Phase 3: Merge
  //
  // One agent merges all completed branches into the current branch,
  // resolving any conflicts and running tests to confirm everything works.
  //
  // The {{BRANCHES}} and {{ISSUES}} prompt arguments are lists that the agent
  // uses to know which branches to merge and which issues to close.
  // -------------------------------------------------------------------------
  await sandcastle.run({
    hooks,
    sandbox: sandboxWithSkills(),
    name: "merger",
    maxIterations: 1,
    agent: cc("auto/fast"),
    promptFile: "./.sandcastle/merge-prompt.md",
    cwd: REPO_ROOT,
    promptArgs: {
      // A markdown list of branch names, one per line.
      BRANCHES: completedBranches.map((b) => `- ${b}`).join("\n"),
      // A markdown list of issue IDs and titles, one per line.
      ISSUES: completedIssues.map((i) => `- ${i.id}: ${i.title}`).join("\n"),
    },
  });

  console.log("\nBranches merged.");
}

console.log("\nAll done.");
