# AGENTS.md — PipelineToolkit Agent / Codex Guide

> Repository-level rules for Codex and other coding agents.
> 本文件是 Codex / Agent 在仓库内开发时必须遵守的长期规则。
>
> Last updated / 最后更新: 2026-10-03

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

### V3.1 — 本地 Python 编排（准备中，尚未实现）

下一节点新增最小标准库脚本 `Scripts/run_validation.py`，调用现有 Commandlet、读取 schema 1 报告并输出摘要。CLI、退出码和验收合同见 `PROJECT_STATE.md` 第 6 节。Python 不承担 Rule / Discovery，不修改资产或 JSON schema，不自动构建 UE；CI 留给后续独立切片。

## 5. V3.0-B 验收基线（已关闭）

- 项目编译，Commandlet 被 UE 发现并可无 UI 运行。
- 默认 Fixture：4 项 / PASS 2 / FAIL 1 / SKIP 1，JSON schema 1，退出码 1。
- Scope `/Game/ValidationTest/Characters`：PASS 1，退出码 0。
- 空白配置、不可写报告目标：退出码 2，日志说明原因。
- 保留现有空目录 / 查询失败语义，不新增特殊资产发现策略。
- 资产无修改，Editor Scan / Fix / Revert / JSON 行为不回归。
- 记录实际命令、报告和进程退出码；未执行的检查不得写为 PASS。

## 6. 当前切片边界

Commandlet 第一版只验证，不 Auto-Fix，不修改资产。V3.1 允许实现最小 Python wrapper；不得加入 GitHub Actions、SQLite、Blender、AI/RAG、第二验证引擎、通用 Rule/Fix Framework 或 JSON schema 重设计。

V3.0-B 已验收关闭，运行证据见 `Saved/Automation/V3Commandlet/`。V3.1 当前仅准备，不得记为已实现或测试通过。历史启动 `Condition failed` 仍未定位。

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

### V3.1 — Local Python Orchestration (preparation, not implemented)

The next node adds the minimal standard-library `Scripts/run_validation.py` to invoke the existing Commandlet, consume schema 1 reports, and print a summary. See section 6 of `PROJECT_STATE.md` for CLI, exit-code, and acceptance contracts. Python does not own rules / discovery, mutate assets, redesign JSON, or build UE automatically. CI remains a later separate slice.

## 5. V3.0-B Acceptance Baseline (closed)

- The project compiles; UE discovers and runs the Commandlet without UI.
- Default fixture: 4 assets / PASS 2 / FAIL 1 / SKIP 1, JSON schema 1, exit code 1.
- Scope `/Game/ValidationTest/Characters`: PASS 1, exit code 0.
- Blank configuration and an unwritable report destination: exit code 2 and a diagnostic log.
- Preserve existing empty-folder/query-failure semantics without introducing a new discovery strategy.
- Assets remain unchanged; Editor Scan / Fix / Revert / JSON behavior does not regress.
- Record actual commands, reports, and process exit codes. Never mark unexecuted checks as PASS.

## 6. Current Slice Boundary

The first Commandlet validates only: no Auto-Fix or asset mutation. V3.1 may implement the minimal Python wrapper. Do not add GitHub Actions, SQLite, Blender, AI/RAG, a second validation engine, a generic Rule/Fix Framework, or a JSON schema redesign.

V3.0-B acceptance is closed; evidence is under `Saved/Automation/V3Commandlet/`. V3.1 is preparation only; do not record it as implemented or tested. Historical startup `Condition failed` remains unresolved.

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
