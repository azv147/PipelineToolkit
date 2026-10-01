# AGENTS.md — PipelineToolkit Agent / Codex Guide

> Repository-level rules for Codex and other coding agents.
> 本文件是 Codex / Agent 在仓库内开发时必须遵守的长期规则。
>
> Last updated / 最后更新: 2026-10-01

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

### V3.0-A — Validation Runner Refactor

当前第一任务：

```text
Refactor the existing Editor validation execution into a reusable
FValidationRunner driven by FValidationConfig.
```

新增的最小 Config：

```cpp
struct FValidationConfig
{
    FString Scope;
    FString BlueprintPrefix;
};
```

Runner 应拥有：

- config validation
- Asset Registry loading / discovery
- recursive `GetAssetsByPath`
- Redirector filtering
- `FBlueprintNamingRule::Evaluate`
- `FValidationRun` construction
- `RecalculateSummary`
- Validation execution log

Slate 应保留：

- `SEditableTextBox` input
- status / error presentation
- SummaryText
- ResultsContainer / Slate rows
- Fix button
- Revert button
- `UEditorAssetSubsystem` Auto-Fix / Revert interaction
- `FValidationReportWriter` trigger can remain UI-side for V3.0-A

推荐最小 Runner 接口：

```cpp
static bool Run(
    const FValidationConfig& Config,
    FValidationRun& OutRun,
    FString* OutErrorMessage = nullptr);
```

可以根据实际编译需要做小幅调整，但不要扩张为通用 Framework。

### 当前禁止在同一切片中实现

除非用户明确改变任务范围，否则不要加入：

- Commandlet
- Python wrapper
- GitHub Actions
- SQLite
- Blender
- AI / RAG
- second validation engine
- generic Rule Framework
- JSON schema redesign

---

## 5. V3.0-A 验收

重构后必须保持 Editor 行为。

默认 Fixture：

```text
Scope  = /Game/ValidationTest
Prefix = BP_
```

预期：

```text
BP_PlayerTest → PASS
Test_Player   → FAIL
M_Test        → SKIP
BP_EnemyTest  → PASS
```

验证：

- 项目能够编译
- Plugin 能正常加载
- Scan 正常
- Summary counts 正常
- Result rows 正常
- Fix / Revert 行为未被破坏
- Export JSON 行为与 schema 未被无故改变

如无法自动验证 UE Editor 行为：

- 明确列出已完成的 compile/static verification
- 明确列出需要用户在 Editor 中人工验证的步骤
- 不得把未执行的 Editor 验证写成 PASS

---

## 6. Commandlet 何时开始

只有 V3.0-A 完成并验证后，才进入 V3.0-B：

```text
UE Editor Commandlet
    ↓
FValidationRunner
    ↓
FValidationRun
    ├── JSON
    └── Process Exit Code
```

第一版 Commandlet：

```text
Validate only
No automatic asset mutation
```

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

### V3.0-A — Validation Runner Refactor

First task:

```text
Refactor the existing Editor validation execution into a reusable
FValidationRunner driven by FValidationConfig.
```

Minimal Config:

```cpp
struct FValidationConfig
{
    FString Scope;
    FString BlueprintPrefix;
};
```

Runner owns:

- config validation
- Asset Registry loading / discovery
- recursive `GetAssetsByPath`
- Redirector filtering
- `FBlueprintNamingRule::Evaluate`
- `FValidationRun` construction
- `RecalculateSummary`
- validation execution logging

Slate keeps:

- `SEditableTextBox` input
- status / error presentation
- SummaryText
- ResultsContainer / Slate rows
- Fix button
- Revert button
- `UEditorAssetSubsystem` Auto-Fix / Revert interaction
- the JSON export trigger may remain UI-side for V3.0-A

Recommended minimal Runner interface:

```cpp
static bool Run(
    const FValidationConfig& Config,
    FValidationRun& OutRun,
    FString* OutErrorMessage = nullptr);
```

Small changes are allowed when compile requirements prove necessary, but do not expand this into a generic framework.

### Do not implement in the same slice unless the user changes scope

- Commandlet
- Python wrapper
- GitHub Actions
- SQLite
- Blender
- AI / RAG
- second validation engine
- generic Rule Framework
- JSON schema redesign

---

## 5. V3.0-A Acceptance

Default fixture:

```text
Scope  = /Game/ValidationTest
Prefix = BP_
```

Expected:

```text
BP_PlayerTest → PASS
Test_Player   → FAIL
M_Test        → SKIP
BP_EnemyTest  → PASS
```

Verify:

- project compiles
- Plugin loads
- Scan works
- summary counts remain correct
- result rows remain correct
- Fix / Revert is not broken
- Export JSON behavior/schema is not changed without reason

If UE Editor behavior cannot be automatically verified:

- report exactly which compile/static checks were run
- list the manual Editor checks the user must run
- do not mark unexecuted Editor verification as PASS

---

## 6. When to Start Commandlet

Only after V3.0-A is verified:

```text
UE Editor Commandlet
    ↓
FValidationRunner
    ↓
FValidationRun
    ├── JSON
    └── Process Exit Code
```

First Commandlet:

```text
Validate only
No automatic asset mutation
```

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
