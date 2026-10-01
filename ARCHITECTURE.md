# UE Production Pipeline Toolkit — 架构 / Architecture

> 本文记录长期架构护栏、边界和重要设计理由，不记录日常进度。
> This document records durable architectural guardrails, boundaries, and design rationale rather than day-to-day progress.
>
> 最后更新 / Last updated: 2026-10-01

---

# 中文版

## 1. 架构意图

`PipelineToolkit` 是一个小型 Unreal Editor 生产工具原型。

目标不是建立“最通用”的 Validation Framework，而是展示可解释的生产工具工程判断：

```text
UE Asset / Project Data
        ↓
Asset Discovery
        ↓
Validation Rule
        ↓
FValidationResult
        ↓
FValidationRun
        ├── Editor UI
        ├── JSON Report
        └── Commandlet / Exit Code（V3）
```

架构只在真实用例证明需要时扩展。

核心原则：

> 非必要不抽象；让真实重复结构证明抽象边界。

---

## 2. 核心职责边界

### 2.1 Asset Discovery

使用 Unreal Engine 原生 Asset Registry。

```text
Scope
 ↓
IAssetRegistry
 ↓
TArray<FAssetData>
```

理由：

- UE 已提供资产索引
- `FAssetData` 适合元数据级检查
- 避免为了验证而建立平行索引系统
- 在能用元数据时不必加载每个 UObject

### 2.2 Rule

Rule 负责真正的验证语义。

当前唯一真实 Rule：

```text
FBlueprintNamingRule
```

行为：

```text
Blueprint → check prefix
non-Blueprint → SKIP
```

当前不建立：

- `IRule`
- `RuleManager`
- `RuleFactory`
- 通用 Rule Registry

只有第二、第三个真实 Rule 出现稳定重复形态后再重新评估。

### 2.3 `FValidationResult`

单条验证结果应同时对人和自动化有意义。

当前包含的核心语义：

```text
Asset
Object Path
Rule Id
Status
Message
Auto-Fix metadata（存在时）
```

不要为了“未来可能有用”提前堆字段。

### 2.4 `FValidationRun`

一次 Validation 执行的共享状态：

```text
Scope
Asset Count
Results
Passed Count
Failed Count
Skipped Count
```

它是 Editor / Report / Automation 共享的结果边界。

不要为：

- Slate
- JSON
- Commandlet

分别建立互相重复的数据模型。

### 2.5 Editor UI / Slate

Slate 负责：

- 获取用户输入
- 触发 Validation
- 展示 Summary / Results
- Status
- Fix / Revert
- Editor-only interaction

Slate 不应该负责：

- Asset discovery 语义
- Rule evaluation 语义
- ValidationRun construction 语义

目标 mental model：

```text
Widget = Input + Presentation + Editor Interaction
Runner = Validation Execution
Rule   = Validation Semantics
Run    = Result State
Writer = Serialization
```

---

## 3. V3 Validation Execution Boundary

V2 结束时，Result / Run / Writer 已经分离，但完整执行仍大部分位于：

```text
SProductionPipelineToolkit::OnScanClicked()
```

这会使 Commandlet 难以复用而不复制代码。

### 3.1 `FValidationConfig`

V3 第一版共享输入仅包含当前已证明需要共享的字段：

```cpp
struct FValidationConfig
{
    FString Scope;
    FString BlueprintPrefix;
};
```

当前不要加入：

```text
Recursive toggle
OnDiskOnly toggle
ReportPath
AutoFix flag
CI flag
Platform policy
```

除非后续真实调用方确实需要改变它们。

### 3.2 `FValidationRunner`

第一版 Runner 是具体的 orchestration boundary，而不是通用 Framework。

职责：

```text
validate FValidationConfig
        ↓
Asset Registry discovery
        ↓
filter Redirectors
        ↓
FBlueprintNamingRule::Evaluate()
        ↓
FValidationRun
        ↓
RecalculateSummary()
```

推荐的最小接口形态：

```cpp
static bool Run(
    const FValidationConfig& Config,
    FValidationRun& OutRun,
    FString* OutErrorMessage = nullptr);
```

理由：

- 能区分 Validation 执行失败与“成功执行但没有 Asset”
- 与现有 `ValidationReportWriter` 的简单错误处理风格一致
- 不需要提前创建复杂 Execution Result hierarchy

### 3.3 先 Runner，后 Commandlet

V3 的顺序必须是：

```text
Step 1
Extract reusable Runner
        ↓
Step 2
Editor still works exactly as before
        ↓
Step 3
Verify baseline fixture
        ↓
Step 4
Add Commandlet as second caller
```

不要把“重构 + Commandlet + CLI 参数 + JSON 改造 + CI”一次性完成。

这样可以把：

- Refactor regression
- Headless behavior bug
- CLI bug
- CI bug

分开定位。

---

## 4. Automation Boundary

Runner 验证后，目标结构：

```text
Editor UI ─────┐
               ├── FValidationRunner
Commandlet ────┘
                       ↓
                FValidationRun
                  ├── JSON
                  └── Exit Code
```

Commandlet 第一版：

```text
Validate only
Do not Auto-Fix
Do not mutate assets
```

理由：

- CI 应默认诊断而不是修改项目
- Mutation 会引入 Source Control / Redirector / Asset lifecycle 复杂度
- 第一目标是证明共享 Validation Semantics

Python / CI 位于 Core 外部：

```text
Python wrapper
    ↓
UE Commandlet
    ↓
JSON / Exit Code
    ↓
CI
```

Python 不承担 Rule 语义。

---

## 5. JSON Report Boundary

JSON 是现有 `FValidationRun` 的序列化输出：

```text
FValidationRun
      ↓
FValidationReportWriter
      ↓
JSON
```

它不是第二个 Validation Domain Model。

V3 Runner 重构不得无故修改 JSON schema。

只有 Commandlet / CI 的真实需要证明缺字段时才演进 schema。

---

## 6. Safe Auto-Fix / Revert Boundary

Auto-Fix 的优先级低于诊断。

当前已证明的 Fix：

```text
Failed Blueprint Naming
        ↓
Suggested Asset Name
        ↓
UEditorAssetSubsystem::RenameAsset()
```

### 6.1 为什么不依赖 Ctrl+Z

测试表明：

```text
RenameAsset()
≠
可以直接假设为普通 UObject Ctrl+Z Transaction
```

外层 `FScopedTransaction` 曾造成不稳定 / Crash。

移除额外 transaction 后：

- Crash 消失
- Ctrl+Z 仍不能可靠撤销 Rename

### 6.2 当前明确采用 Explicit Revert

当前 Editor Workflow：

```text
Fix
 ↓
record Original Path
record Fixed Path
 ↓
Revert Last Fix
 ↓
RenameAsset(Fixed → Original)
```

因此：

- Explicit Revert 已经取代“依赖 Ctrl+Z”的方案
- Revert 不是通用 Undo 系统
- 当前不创建 Fix Framework
- Runner / Commandlet 第一版不承担 Auto-Fix

---

## 7. UE Asset Boundary

### Content Browser Asset vs Level Actor

本项目验证的是 Project Asset，而不是 World 中的 Actor Instance。

```text
Content Browser Asset
→ Asset Registry

Level Actor
→ World / Level instance
```

因此 V1 从 Asset Registry 开始是正确边界。

### Package Path / Package Name / Object Path

必须区分：

```text
Package Path
/Game/ValidationTest

Package Name
/Game/ValidationTest/Test_Player

Object Path
/Game/ValidationTest/Test_Player.Test_Player
```

Auto-Fix 曾因混淆这些语义构造错误 Rename 目标。

### Redirector

Asset Rename / Move 可能产生 Redirector。

Validation 不能把 Redirector 当作普通业务 Asset。

### 文件系统边界

`.uasset` 不应在正在运行的 Editor Workflow 中被当作普通 Windows 文件直接删除 / 移动。

正确做法优先通过：

- Unreal Editor
- Content Browser
- UE Asset API

---

## 8. Module Boundary

项目当前以 Editor Tool 为主。

长期原则：

```text
Runtime must not depend on Editor
```

如果未来出现 Runtime 功能：

```text
Plugin
├─ Runtime Module
└─ Editor Module
```

Editor-only 依赖放 Editor Module。

当前项目没有必要为了展示而提前拆出 Runtime Module。

---

## 9. Configuration Strategy

长期设计倾向：

```text
Policy
  ↓
Same validation execution
  ↓
Different thresholds / requirements
```

但当前只有 Blueprint Prefix / Scope 已被实际证明。

不要为了未来的 PC / Mobile / Console Policy 提前构建配置系统。

---

## 10. Observability Direction

可观测性是后期增强层。

优先级：

```text
asset count
issue count
run duration
rule/run timing（有需要时）
```

SQLite / history / chart 不属于当前 V3 第一任务。

---

## 11. 文档与 Agent 协作边界

长期文档职责：

```text
README.md
→ public overview

PROJECT_STATE.md
→ current verified truth

ARCHITECTURE.md
→ rationale / boundaries / decisions

SESSION_LOG.md
→ chronological actual work

AGENTS.md
→ Codex execution guardrails
```

开发准则：

> 所有长期项目文档必须维护中文 + English 双语版本。

任何 Agent 都不得：

- 把计划写成已完成
- 根据文档推断 build / commit / push 成功
- 在没有真实需求的情况下扩张架构

---

## 12. 明确非目标

除非真实用例证明必要，否则不要加入：

- 自定义 Validation DSL
- 独立 Validation executable
- 大型 Rule Framework
- 大量 Factory / Strategy / Registry
- Dashboard / chart platform
- 大型 AI / RAG / Agent 架构
- 大量投机性 Rule
- 大型展示游戏场景
- Commandlet 自动修改资产

---

## 13. 重要决策记录

### Decision: Evidence-driven abstraction for rules
Status: Accepted
Date: 2026-09-23

Context: 当前只有一个真实 `BlueprintNamingRule`。

Decision: 不创建 `IRule` / `RuleManager` / `RuleFactory` / 通用 Registry。

Reason: 尚无足够重复结构证明 Framework 的收益。

Trade-off: 等第二、第三个真实 Rule 出现后再重新评估。

### Decision: `FValidationRun` is the V2 execution-state boundary
Status: Accepted
Date: 2026-09-23 → 2026-09-27

Decision: 将 Scope / Results / Summary Counts 组织为一次完整 Run。

Reason: UI / Report / Automation 需要共享同一执行状态。

### Decision: JSON Report is an output boundary
Status: Accepted
Date: 2026-09-27

Decision: 直接序列化 `FValidationRun`，不创建第二套 Report Domain Model。

### Decision: Generic Ctrl+Z is not the Rename Auto-Fix contract
Status: Accepted
Date: 2026-09-27

Decision: 不声称当前 Rename Auto-Fix 支持通用 Ctrl+Z。

### Decision: Explicit Revert replaces the Ctrl+Z approach
Status: Accepted
Date: 2026-09-29

Decision: 使用独立 `Revert Last Fix` 控件，通过保存前后 Asset Path 显式恢复前一个资产名。

Reason: 比依赖不可靠 Transaction 行为更确定、更容易解释。

Trade-off: 只恢复最后一次已记录 Rename，不是通用 Undo Stack。

### Decision: Extract Validation Runner before adding Commandlet
Status: Accepted
Date: 2026-10-01

Context: `OnScanClicked()` 仍混合 UI、Discovery、Rule Execution 与 Presentation。

Decision: 先引入最小 `FValidationConfig + FValidationRunner`，验证 Editor 行为不变后，再让 Commandlet 成为第二调用方。

Reason: 避免复制 Validation 逻辑，也让重构问题与 Headless 问题可分开定位。

Trade-off: Commandlet 延后一个小切片，但 V3 的共享边界更清晰。

### Decision: Long-lived project documents are bilingual
Status: Accepted
Date: 2026-10-01

Decision: `README / PROJECT_STATE / ARCHITECTURE / SESSION_LOG / AGENTS` 均维护中文 + English。

Reason: 中文用于学习和协作，英文用于 GitHub / 面试 / Agent handoff，并避免两套文档长期漂移。

---

# English Version

## 1. Architectural Intent

`PipelineToolkit` is a small Unreal Editor production-tool prototype.

The goal is not to build the most generic validation framework. The architecture should demonstrate explainable production-tool engineering:

```text
UE Asset / Project Data
        ↓
Asset Discovery
        ↓
Validation Rule
        ↓
FValidationResult
        ↓
FValidationRun
        ├── Editor UI
        ├── JSON Report
        └── Commandlet / Exit Code (V3)
```

Extend the architecture only when a real use case proves the need.

Core principle:

> Do not abstract without evidence; let repeated real structure prove the boundary.

---

## 2. Core Responsibility Boundaries

### 2.1 Asset Discovery

Use Unreal Engine's native Asset Registry:

```text
Scope
 ↓
IAssetRegistry
 ↓
TArray<FAssetData>
```

Reasons:

- UE already owns the asset index
- `FAssetData` supports metadata-oriented validation
- no parallel indexing system is needed
- avoid loading every UObject when metadata is sufficient

### 2.2 Rule

Rules own validation semantics.

Current real rule:

```text
FBlueprintNamingRule
```

Current behavior:

```text
Blueprint → check prefix
non-Blueprint → SKIP
```

Do not introduce yet:

- `IRule`
- `RuleManager`
- `RuleFactory`
- generic rule registry

Revisit only after multiple real rules reveal a stable common shape.

### 2.3 `FValidationResult`

One result should be useful to both humans and automation.

Current core semantics:

```text
Asset
Object Path
Rule Id
Status
Message
Auto-Fix metadata (when applicable)
```

Do not add speculative metadata.

### 2.4 `FValidationRun`

A shared representation of one validation execution:

```text
Scope
Asset Count
Results
Passed Count
Failed Count
Skipped Count
```

It is the shared output boundary for Editor, report, and automation.

Do not create separate duplicate models for Slate, JSON, and Commandlet.

### 2.5 Editor UI / Slate

Slate owns:

- user input
- triggering validation
- summary/result presentation
- status
- Fix/Revert
- Editor-only interaction

Slate should not own:

- asset-discovery semantics
- rule-evaluation semantics
- ValidationRun-construction semantics

Mental model:

```text
Widget = Input + Presentation + Editor Interaction
Runner = Validation Execution
Rule   = Validation Semantics
Run    = Result State
Writer = Serialization
```

---

## 3. V3 Validation Execution Boundary

At the end of V2, Result/Run/Writer are separated, but complete execution still largely lives in:

```text
SProductionPipelineToolkit::OnScanClicked()
```

That makes Commandlet reuse difficult without duplication.

### 3.1 `FValidationConfig`

The first shared input contains only fields already proven necessary:

```cpp
struct FValidationConfig
{
    FString Scope;
    FString BlueprintPrefix;
};
```

Do not prematurely add:

```text
Recursive toggle
OnDiskOnly toggle
ReportPath
AutoFix flag
CI flag
Platform policy
```

### 3.2 `FValidationRunner`

The first Runner is a concrete orchestration boundary, not a generic framework.

Responsibilities:

```text
validate FValidationConfig
        ↓
Asset Registry discovery
        ↓
filter Redirectors
        ↓
FBlueprintNamingRule::Evaluate()
        ↓
FValidationRun
        ↓
RecalculateSummary()
```

Recommended minimal interface:

```cpp
static bool Run(
    const FValidationConfig& Config,
    FValidationRun& OutRun,
    FString* OutErrorMessage = nullptr);
```

This distinguishes execution failure from a successful run containing zero assets without inventing a complex execution-result hierarchy.

### 3.3 Runner before Commandlet

Required V3 order:

```text
Step 1
Extract reusable Runner
        ↓
Step 2
Editor still works exactly as before
        ↓
Step 3
Verify baseline fixture
        ↓
Step 4
Add Commandlet as the second caller
```

Do not combine refactor, Commandlet, CLI parsing, JSON redesign, and CI in one change.

---

## 4. Automation Boundary

After Runner verification:

```text
Editor UI ─────┐
               ├── FValidationRunner
Commandlet ────┘
                       ↓
                FValidationRun
                  ├── JSON
                  └── Exit Code
```

First Commandlet:

```text
Validate only
Do not Auto-Fix
Do not mutate assets
```

Reasons:

- CI should diagnose by default rather than mutate
- mutation adds source-control, redirector, and asset-lifecycle complexity
- the first goal is shared validation semantics

Python / CI remain outside the core:

```text
Python wrapper
    ↓
UE Commandlet
    ↓
JSON / Exit Code
    ↓
CI
```

Python does not own rule semantics.

---

## 5. JSON Report Boundary

JSON serializes the existing run:

```text
FValidationRun
      ↓
FValidationReportWriter
      ↓
JSON
```

It is not a second validation domain model.

The V3 Runner refactor must not change the JSON schema without a demonstrated automation requirement.

---

## 6. Safe Auto-Fix / Revert Boundary

Diagnosis has higher priority than Auto-Fix.

Proven fix:

```text
Failed Blueprint Naming
        ↓
Suggested Asset Name
        ↓
UEditorAssetSubsystem::RenameAsset()
```

### 6.1 Why Ctrl+Z is not the contract

Testing showed:

```text
RenameAsset()
≠
something that can simply be assumed to behave like a normal UObject transaction
```

An outer `FScopedTransaction` caused instability / crash.

Removing it stopped the crash but did not provide reliable Ctrl+Z undo.

### 6.2 Explicit Revert is the accepted design

Current Editor workflow:

```text
Fix
 ↓
record Original Path
record Fixed Path
 ↓
Revert Last Fix
 ↓
RenameAsset(Fixed → Original)
```

Therefore:

- Explicit Revert replaces the Ctrl+Z approach
- Revert is not a generic Undo system
- no generic Fix Framework is introduced
- Runner / first Commandlet do not own Auto-Fix

---

## 7. UE Asset Boundary

### Content Browser Asset vs Level Actor

This project validates project assets, not actor instances in a World.

```text
Content Browser Asset
→ Asset Registry

Level Actor
→ World / Level instance
```

### Package Path / Package Name / Object Path

Keep these distinct:

```text
Package Path
/Game/ValidationTest

Package Name
/Game/ValidationTest/Test_Player

Object Path
/Game/ValidationTest/Test_Player.Test_Player
```

Confusing them caused a real Rename destination bug.

### Redirector

Asset Rename/Move may create redirectors.

Redirectors should not be treated as normal business assets during validation.

### Filesystem boundary

Do not treat `.uasset` files as ordinary Windows files during an active Editor workflow.

Prefer:

- Unreal Editor
- Content Browser
- UE Asset APIs

---

## 8. Module Boundary

The project is currently an Editor tool.

Long-term rule:

```text
Runtime must not depend on Editor
```

If runtime functionality appears later:

```text
Plugin
├─ Runtime Module
└─ Editor Module
```

Do not split modules merely for appearance before a runtime use case exists.

---

## 9. Configuration Strategy

Long-term direction:

```text
Policy
  ↓
Same validation execution
  ↓
Different thresholds / requirements
```

At present, only Scope and Blueprint Prefix are proven configuration inputs.

Do not build a speculative platform-policy system yet.

---

## 10. Observability Direction

Observability is a later enhancement.

Priority order:

```text
asset count
issue count
run duration
rule/run timing (when useful)
```

SQLite/history/charting are not part of the current V3 slice.

---

## 11. Documentation and Agent Boundary

Long-lived documents:

```text
README.md
→ public overview

PROJECT_STATE.md
→ current verified truth

ARCHITECTURE.md
→ rationale / boundaries / decisions

SESSION_LOG.md
→ chronological actual work

AGENTS.md
→ Codex execution guardrails
```

Development rule:

> All long-lived project documents must be maintained in both Chinese and English.

Agents must not:

- record planned work as complete
- infer build/commit/push success from docs
- expand architecture without a real requirement

---

## 12. Explicit Non-Goals

Unless a real use case proves otherwise, do not add:

- custom validation DSL
- separate validation executable
- large rule framework
- broad Factory / Strategy / Registry layers
- dashboard/charting platform
- large AI/RAG/agent architecture
- many speculative rules
- large showcase game scene
- automatic asset mutation from the first Commandlet

---

## 13. Decision Records

### Decision: Evidence-driven abstraction for rules
Status: Accepted
Date: 2026-09-23

Decision: Do not add `IRule`, `RuleManager`, `RuleFactory`, or a generic registry with only one real rule.

### Decision: `FValidationRun` is the V2 execution-state boundary
Status: Accepted
Date: 2026-09-23 → 2026-09-27

Decision: One run owns scope, results, and summary counts so UI/report/automation can share the same state.

### Decision: JSON Report is an output boundary
Status: Accepted
Date: 2026-09-27

Decision: Serialize `FValidationRun`; do not create a second report-domain model.

### Decision: Generic Ctrl+Z is not the Rename Auto-Fix contract
Status: Accepted
Date: 2026-09-27

Decision: Do not claim generic Ctrl+Z support for Rename Auto-Fix.

### Decision: Explicit Revert replaces the Ctrl+Z approach
Status: Accepted
Date: 2026-09-29

Decision: Use a dedicated `Revert Last Fix` control and stored asset paths to rename the asset explicitly back to its previous name.

Reason: This is deterministic and easier to explain than relying on fragile transaction behavior.

Trade-off: It restores the recorded last Rename; it is not a generic Undo stack.

### Decision: Extract Validation Runner before adding Commandlet
Status: Accepted
Date: 2026-10-01

Context: `OnScanClicked()` still mixes UI, discovery, rule execution, and presentation.

Decision: Introduce the minimal `FValidationConfig + FValidationRunner` first, verify unchanged Editor behavior, then add Commandlet as the second caller.

Reason: Prevent duplicated validation logic and isolate refactor regressions from headless-execution issues.

### Decision: Long-lived project documents are bilingual
Status: Accepted
Date: 2026-10-01

Decision: Maintain `README / PROJECT_STATE / ARCHITECTURE / SESSION_LOG / AGENTS` in Chinese and English.

Reason: Chinese supports learning/collaboration; English supports GitHub/interviews/agent handoff without allowing separate documents to drift.
