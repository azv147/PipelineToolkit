# AGENTS.md — PipelineToolkit Agent / Codex Guide

> Repository-level rules for Codex and other coding agents.
> 本文件是 Codex / Agent 在仓库内开发时必须遵守的长期规则。
>
> Last updated / 最后更新: 2026-10-11

---

# 中文版

## 1. 开始工作前

先读取：

1. `PROJECT_STATE.md` — 当前真实状态
2. `ARCHITECTURE.md` — 架构边界 / 设计理由
3. `SESSION_LOG.md` — 仅在需要最近开发历史时读取

如果文档与实际代码 / Git / Build 输出冲突：

> 以实际仓库、编译结果、UE Log 和用户最新明确指令为准，并更新文档。

不要仅凭旧文档声称：

- build 成功
- test 成功
- commit 成功
- push 成功

必须实际验证。

---

## 2. 项目目标

这是 Unreal Engine 5.8.2 的 Editor Tools / Pipeline Engineering 作品集项目。

目标是展示：

- C++ / UE 工程能力
- Editor Tool Workflow
- Asset Validation
- Commandlet / Automation / CI 思维
- 简单、可解释、可维护的架构判断

项目原则：

> 30 秒讲清价值；30 分钟讲清实现。

不要把作品集 Demo 扩张成通用 Framework 项目。

---

## 3. 核心开发原则

### 非必要不抽象

当前禁止无真实需求新增：

- `IRule`
- `RuleManager`
- `RuleFactory`
- generic Rule Registry
- 大型 Service / Manager 层
- custom DSL

如要增加抽象，必须先指出：

- 存在哪两个以上真实重复用例
- 当前具体实现为什么已经产生明确成本
- 新抽象如何降低而不是增加复杂度

### 优先 UE 原生机制

优先使用：

- Asset Registry
- UE Editor Subsystems
- Commandlet
- UE Module / Plugin 边界
- UE 原生 Asset API

不要创建平行 Asset Index 或独立 Validation executable。

### 保持职责边界

目标：

```text
Widget = Input + Presentation + Editor Interaction
Runner = Validation Execution
Rule   = Validation Semantics
Run    = Result State
Writer = Serialization
```

### Auto-Fix 边界

- Auto-Fix / Explicit Revert 当前属于 Editor Workflow
- 不依赖 Ctrl+Z 作为 Rename Fix 的合同
- Commandlet 第一版不 Auto-Fix
- 不在 V3.0-A 创建通用 Fix Framework

---

## 4. 当前 V3 工作任务

### V3.0-A — 已完成

Runner 重构已实现，构建与原生 Automation 测试通过；用户于 2026-10-02 确认人工功能验证与代码评审完成。保留现有 Widget / Runner / Rule / Run / Writer 边界。

### V3.0-B — Editor Commandlet（已完成）

已实现 `UValidateAssetsCommandlet`，入口 `-run=ValidateAssets`；构建和 9 个真实进程场景通过，普通 Editor 启动下的 Runner Automation 回归通过，资产哈希不变：

```text
CLI parameters → FValidationConfig → FValidationRunner
                                      ↓
                                FValidationRun
                                  ├── existing JSON Writer
                                  └── process exit code
```

- 输入：`-Scope=`、`-BlueprintPrefix=`、`-ReportPath=`。
- 默认值：`/Game/ValidationTest`、`BP_`、`Saved/ValidationReports/ValidationReport.json`。
- 相对报告路径以项目目录为基准；不把 ReportPath 加进 `FValidationConfig`。
- 退出码：`0` 无规则 FAIL 且报告成功；`1` 有规则 FAIL 且报告成功；`2` 参数 / 执行 / 报告写入失败。
- 规则 FAIL 时仍导出完整 JSON；执行失败不导出无效 Run。
- 保持已有 Runner 查询与输入语义、JSON schema；不复制验证执行逻辑。
- 模块在 `IsRunningCommandlet()` 时跳过 Tab / ToolMenus 注册和清理，普通 Editor 路径保持原行为。

2026-10-03 用户确认 V3.0-B 验收完毕，人工验收来自用户确认，本次未重跑 Editor / build / Automation。此前资产哈希一致仅描述自动化运行前后，不代表当前工作区无资产改动。

### Rule / Writer 合同测试 — 已完成（2026-10-03）

已新增原生 Automation 合同测试，Validation 组 3 项通过，新测试独立重跑 2 项通过，两轮进程均为 0；资产哈希不变，临时目录已清理。首次 Automation 启动在测试前出现 `Null assigned to TNotNull` 崩溃，简化启动参数后成功，原因未定位；历史 `Condition failed` 仍未解决。证据见 `Saved/Automation/ValidationContractsEvidence/`。现有 Runner 测试与生产代码保持不变。

### V3.1 — 本地 Python 编排（已完成）

已新增标准库脚本 `Scripts/run_validation.py` 与 unittest，调用现有 Commandlet、读取 schema 1 报告并输出摘要。11 项单元测试及 8 个实际入口场景通过，资产哈希不变。2026-10-05 用户确认 V3.1 人工验收完成，据此关闭；该人工结论来自用户确认。Python 不承担 Rule / Discovery，不修改资产或 JSON schema，也不自行构建 UE。

### V3.2 — 最小 CI（已完成）

新增 `Scripts/run_ci.ps1` 和 `.github/workflows/validation.yml`。顺序为 Python 测试 → Editor 构建 → UE Automation → 默认 Fixture 烟雾测试（预期 1）→ Characters（预期 0）；失败使流水线退出 1。只手动触发，Windows 自托管 runner 标签为 `ue-5.8`，配置 `UE_ROOT`（UE 5.8.2）和可选 `PYTHON_COMMAND`。逐阶段日志 / 退出码 / JSON / 资产哈希保存在独立 Saved/Automation/CI/GUID 目录，成功失败均归档。2026-10-11 用户确认 CI 人工验收通过，据此关闭 V3.2；远端验收结论来自用户确认，本次 Agent 未重跑 CI 或独立复核成功远端产物。

## 5. V3.0-B 验收基线（已关闭）

- 项目编译，Commandlet 被 UE 发现并可无 UI 运行。
- 默认 Fixture：4 项 / PASS 2 / FAIL 1 / SKIP 1，JSON schema 1，退出码 1。
- Scope `/Game/ValidationTest/Characters`：PASS 1，退出码 0。
- 空白配置、不可写报告目标：退出码 2，日志说明原因。
- 保留现有空目录 / 查询失败语义，不新增特殊资产发现策略。
- 资产无修改，Editor Scan / Fix / Revert / JSON 行为不回归。
- 记录实际命令、报告和进程退出码；未执行的检查不得写为 PASS。

## 6. 当前切片边界

Commandlet 第一版只验证，不 Auto-Fix，不修改资产。V3.2 允许最小 GitHub Actions 和本地 CI 脚本；不得加入 SQLite、Blender、AI/RAG、第二验证引擎、通用 Rule/Fix Framework 或 JSON schema 重设计。Python 仍只负责验证入口，构建 / 测试顺序由 CI 编排。

V3.0-A / V3.0-B / Rule / Writer 合同测试 / V3.1 / V3.2 已完成。历史本地证据见 `Saved/Automation/V3Commandlet/`、`Saved/Automation/ValidationContractsEvidence/`、`Saved/Automation/V3Python/` 和 `Saved/Automation/CI/`；V3.2 人工验收来自用户确认。历史启动 `Condition failed` 与一次 `Null assigned to TNotNull` 崩溃仍未解决，验收完成不代表已修复。

下一切片建议补齐 Python 边界测试并新增 GitHub 托管 Windows 的轻量测试 workflow，随后单独定位启动异常；这些均尚未实施，用户本次仅授权文档同步与交接。具体任务、文件和验收标准见 `PROJECT_STATE.md` 第 6 节，静态排查线索见 `SESSION_LOG.md` 的 2026-10-11 记录。保持现有 UE 自托管完整流水线和生产语义，不新增长期记忆文档。

---

## 7. Git / 变更规则

开始修改前：

```bash
git status
git log --oneline -5
```

完成切片后建议：

- 输出 `git diff --stat`
- 输出关键 diff
- 说明实际执行过的 build/test command
- 说明任何尚未验证的行为

不要自动重写历史提交。

不要自动 push，除非用户明确要求。

不要删除 `.uasset` 或通过 Windows 文件系统模拟 UE Asset 操作。

---

## 8. 文档维护规则

**所有长期项目文档必须输出中文 + English 双语版本。**

需要维护：

- `PROJECT_STATE.md`
- `ARCHITECTURE.md`
- `SESSION_LOG.md`
- `README.md`
- `AGENTS.md`

规则：

### `PROJECT_STATE.md`
只有“当前真实状态”变化时更新。

### `ARCHITECTURE.md`
只有架构边界 / 设计决策变化时更新。

### `SESSION_LOG.md`
追加：

- 实际做了什么
- 为什么
- 实际验证结果
- 未完成项

不要删除旧历史来“纠正”过去；用新记录说明后来事实。

### `README.md`
只有对外项目状态明显变化时更新。

### 双语准则
中文部分用于学习 / 协作；English 部分用于 GitHub / 面试 / Agent handoff。

两部分必须表达同一事实，不得长期漂移。

---

# English Version

## 1. Before Starting Work

Read:

1. `PROJECT_STATE.md` — current verified truth
2. `ARCHITECTURE.md` — architectural boundaries / rationale
3. `SESSION_LOG.md` — only when recent history is needed

If documentation conflicts with code, Git, build output, UE logs, or the user's latest explicit instruction:

> Treat the real repository/runtime evidence and latest user instruction as authoritative, then update the docs.

Never claim build/test/commit/push success from documentation alone.

---

## 2. Project Goal

This is an Unreal Engine 5.8.2 Editor Tools / Pipeline Engineering portfolio project.

It demonstrates:

- C++ / UE engineering
- Editor Tool workflows
- Asset Validation
- Commandlet / Automation / CI thinking
- simple, explainable, maintainable engineering judgment

Portfolio principle:

> Explain the value in 30 seconds; explain the implementation in 30 minutes.

Do not turn the portfolio demo into a generic framework project.

---

## 3. Core Engineering Rules

### Do not abstract without evidence

Do not add without a real demonstrated need:

- `IRule`
- `RuleManager`
- `RuleFactory`
- generic Rule Registry
- large Service / Manager layers
- custom DSL

Before introducing an abstraction, identify:

- at least two real repeated use cases
- the concrete cost of the current implementation
- how the new abstraction reduces rather than increases complexity

### Prefer UE-native mechanisms

Prefer:

- Asset Registry
- UE Editor Subsystems
- Commandlet
- UE Module / Plugin boundaries
- native UE Asset APIs

Do not build a parallel asset index or separate validation executable.

### Preserve responsibility boundaries

Target model:

```text
Widget = Input + Presentation + Editor Interaction
Runner = Validation Execution
Rule   = Validation Semantics
Run    = Result State
Writer = Serialization
```

### Auto-Fix boundary

- Auto-Fix / Explicit Revert remains an Editor workflow
- generic Ctrl+Z is not the Rename Fix contract
- first Commandlet does not Auto-Fix
- do not build a generic Fix Framework during V3.0-A

---

## 4. Current V3 Task

### V3.0-A — complete

The Runner refactor is implemented; the build and native Automation test passed. On 2026-10-02 the user confirmed completion of manual functional verification and code review. Preserve the existing Widget / Runner / Rule / Run / Writer boundaries.

### V3.0-B — Editor Commandlet (complete)

`UValidateAssetsCommandlet` is implemented and invoked with `-run=ValidateAssets`. The build and 9 actual process cases passed, Runner Automation passed under ordinary Editor startup, and asset hashes are unchanged:

```text
CLI parameters → FValidationConfig → FValidationRunner
                                      ↓
                                FValidationRun
                                  ├── existing JSON Writer
                                  └── process exit code
```

- Inputs: `-Scope=`, `-BlueprintPrefix=`, and `-ReportPath=`.
- Defaults: `/Game/ValidationTest`, `BP_`, and `Saved/ValidationReports/ValidationReport.json`.
- Resolve relative report paths from the project directory; do not add ReportPath to `FValidationConfig`.
- Exit codes: `0` = no rule FAIL and report success; `1` = rule FAIL and report success; `2` = argument / execution / report-writing failure.
- Rule FAIL still produces the complete JSON report; execution failures do not export an invalid Run.
- Preserve existing Runner query/input semantics and the JSON schema; do not duplicate validation execution.
- The module skips Tab / ToolMenus registration and cleanup when `IsRunningCommandlet()` is true; the ordinary Editor path retains its behavior.

On 2026-10-03 the user confirmed V3.0-B acceptance. Manual acceptance is user-confirmed; no Editor / build / Automation rerun occurred in this session. Earlier unchanged asset hashes describe the automated runs only, not an asset-clean current worktree.

### Rule / Writer Contract Tests — complete (2026-10-03)

Native Automation contract tests were added: the Validation group passed 3 tests, and an independent repeat passed 2 new tests, both processes exiting 0. Asset hashes were unchanged and temporary directories cleaned. The initial Automation startup crashed before tests with `Null assigned to TNotNull`; simplified startup flags succeeded, but the cause is unresolved. Historical `Condition failed` is also unresolved. Evidence: `Saved/Automation/ValidationContractsEvidence/`. Existing Runner tests and production code are unchanged.

### V3.1 — Local Python Orchestration (complete)

Added standard-library `Scripts/run_validation.py` and unittest coverage to invoke the existing Commandlet, consume schema 1 reports, and print a summary. Eleven unit tests and eight actual entry-point cases passed, with unchanged asset hashes. On 2026-10-05 the user confirmed manual V3.1 acceptance, closing the slice; manual acceptance is user-confirmed. Python does not own rules / discovery, mutate assets, redesign JSON, or build UE itself.

### V3.2 — Minimal CI (complete)

Added `Scripts/run_ci.ps1` and `.github/workflows/validation.yml`. Sequence: Python tests → Editor build → UE Automation → default-fixture smoke (expected 1) → Characters (expected 0). Failures exit the pipeline with 1. Manual trigger only; Windows self-hosted runner label `ue-5.8`, `UE_ROOT` pointing to UE 5.8.2, and optional `PYTHON_COMMAND`. Stage logs / exits / JSON / asset hashes live in isolated Saved/Automation/CI/GUID directories and are archived on success or failure. On 2026-10-11 the user confirmed manual CI acceptance, closing V3.2. Remote acceptance is user-confirmed; the agent did not rerun CI or independently verify successful remote artifacts in this session.

## 5. V3.0-B Acceptance Baseline (closed)

- The project compiles; UE discovers and runs the Commandlet without UI.
- Default fixture: 4 assets / PASS 2 / FAIL 1 / SKIP 1, JSON schema 1, exit code 1.
- Scope `/Game/ValidationTest/Characters`: PASS 1, exit code 0.
- Blank configuration and an unwritable report destination: exit code 2 and a diagnostic log.
- Preserve existing empty-folder/query-failure semantics without introducing a new discovery strategy.
- Assets remain unchanged; Editor Scan / Fix / Revert / JSON behavior does not regress.
- Record actual commands, reports, and process exit codes. Never mark unexecuted checks as PASS.

## 6. Current Slice Boundary

The first Commandlet validates only: no Auto-Fix or asset mutation. V3.2 may add minimal GitHub Actions and local CI orchestration, without SQLite, Blender, AI/RAG, a second validation engine, generic Rule/Fix Framework, or JSON schema redesign. Python remains the validation entry; CI owns build / test sequencing.

V3.0-A / V3.0-B / Rule / Writer contract tests / V3.1 / V3.2 are complete. Historical local evidence is under `Saved/Automation/V3Commandlet/`, `Saved/Automation/ValidationContractsEvidence/`, `Saved/Automation/V3Python/`, and `Saved/Automation/CI/`; V3.2 manual acceptance is user-confirmed. Historical startup `Condition failed` and one `Null assigned to TNotNull` crash remain unresolved; acceptance does not establish a fix.

The suggested next slice adds Python boundary tests and a lightweight GitHub-hosted Windows test workflow, followed by a separate startup investigation. None is implemented; the user's current authorization covers documentation synchronization and handoff only. Tasks, files, and acceptance criteria are in section 6 of `PROJECT_STATE.md`; static investigation leads are in the 2026-10-11 `SESSION_LOG.md` entry. Preserve the full self-hosted UE pipeline and production semantics; do not add a long-lived memory document.

---

## 7. Git / Change Rules

Before changes:

```bash
git status
git log --oneline -5
```

After the slice:

- show `git diff --stat`
- summarize key diff
- state the actual build/test commands executed
- state anything not yet verified

Do not rewrite Git history automatically.

Do not push unless the user explicitly asks.

Do not delete `.uasset` files or simulate UE Asset operations through Windows filesystem manipulation.

---

## 8. Documentation Rules

**All long-lived project documents must be maintained in both Chinese and English.**

Maintain:

- `PROJECT_STATE.md`
- `ARCHITECTURE.md`
- `SESSION_LOG.md`
- `README.md`
- `AGENTS.md`

### `PROJECT_STATE.md`
Update only when current truth changes.

### `ARCHITECTURE.md`
Update when architectural boundaries or design decisions change.

### `SESSION_LOG.md`
Append:

- what was actually done
- why
- actual verification
- remaining work

Do not rewrite old history to make it look current; add a later entry that records the new truth.

### `README.md`
Update when externally visible project status meaningfully changes.

### Bilingual rule
Chinese supports learning/collaboration; English supports GitHub/interviews/agent handoff.

Both sections must describe the same facts and must not drift.
