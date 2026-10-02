# UE Production Pipeline Toolkit — 项目状态 / Project State

> 当前真实状态文档。项目事实发生变化时更新。
> Current-truth document. Update when verified project facts change.
>
> 最后更新 / Last updated: 2026-10-01

---

# 中文版

## 1. 项目身份

- 项目：`PipelineToolkit`
- 插件：`ProductionPipelineToolkit`
- Unreal Engine：`5.8.2`
- 平台：Windows
- 项目类型：Game → Blank → C++
- 目标岗位方向：游戏行业 Tools / Systems / Pipeline Engineer
- 项目性质：作品集原型，不以商用通用插件为目标

作品集原则：

> 30 秒讲清价值；30 分钟讲清实现。

项目希望展示：

- Unreal Engine C++ 工程能力
- Editor Tool / Slate 工作流设计
- Rule-driven Asset Validation
- Automation / Commandlet / CI 思维
- 基础可观测性与性能意识
- 模块边界与工程判断力
- “先证明真实需求，再支付抽象成本”的设计习惯

---

## 2. 里程碑状态

```text
V0  Editor Plugin + Slate Toolkit        ✅
V1  Asset Registry + Validation Core     ✅
V2  Production Validation Workflow       ✅
V3  Headless Validation & Automation     ▶ Active
```

### V0 — 已完成

已验证：

```text
Plugin
→ Module
→ Editor Integration
→ Slate UI
→ User Interaction
```

包括：

- UE C++ 项目与 Editor Plugin
- Module 生命周期
- Nomad Tab
- ToolMenus 菜单入口
- Slate UI
- Scan 交互入口

### V1 — 已完成

已跑通真实 Asset Validation 垂直切片：

```text
Asset Registry
    ↓
FAssetData
    ↓
FBlueprintNamingRule
    ↓
FValidationResult
    ↓
Slate Editor Display
```

测试 Fixture：

```text
/Game/ValidationTest/BP_PlayerTest
/Game/ValidationTest/Test_Player
/Game/ValidationTest/M_Test
/Game/ValidationTest/Characters/BP_EnemyTest
```

基线结果：

```text
BP_PlayerTest → PASS
Test_Player   → FAIL
M_Test        → SKIP
BP_EnemyTest  → PASS
```

### V2 — 已完成并同步 GitHub

V2 将“能做一次验证”组织成完整 Editor Validation Workflow：

```text
Asset Registry
      ↓
Rule Evaluation
      ↓
FValidationResult
      ↓
FValidationRun
      ├── Editor Summary / Details
      ├── Rule Configuration
      ├── Safe Auto-Fix / Explicit Revert
      └── JSON Report
```

已完成并验证：

- `FValidationRun`
  - Scope
  - Asset Count
  - Results
  - Passed / Failed / Skipped Count
- Scan Scope 可配置
- Blueprint Naming Prefix 可配置
- Invalid Scope / Rule Configuration 会清空过期 UI 结果
- Safe Rename Auto-Fix
- 独立 `Revert Last Fix` 控件恢复前一个资产名
- JSON Report 落盘到：
  - `Saved/ValidationReports/ValidationReport.json`

### Auto-Fix 当前事实

最初尝试依赖 `Ctrl+Z` 撤销 Rename，但测试表明：

- `RenameAsset()` 不能简单视作普通 UObject Transaction
- 外层 `FScopedTransaction` 曾导致不稳定 / Crash
- 移除外层 Transaction 后，Ctrl+Z 仍不能可靠恢复 Rename

当前方案已经改为：

```text
Fix
 ↓
UEditorAssetSubsystem::RenameAsset()
 ↓
记录 Original / Fixed Path
 ↓
Revert Last Fix
 ↓
显式 Rename 回原资产名
```

因此：

- 不再把 Ctrl+Z Undo 作为 Auto-Fix 目标
- Revert 是明确的 Editor Workflow
- 当前不建立通用 Fix Framework

### V3 — 当前阶段

V3 的总体目标：

> 将现有 Editor Validation Workflow 推进到可无 UI 执行的 Automation Boundary，同时保证 Editor UI 与未来 Commandlet 共享同一套验证语义。

目标架构：

```text
Editor UI ─────┐
               ├── FValidationRunner
Commandlet ────┘
                       ↓
                FValidationRun
                  ├── JSON
                  └── Exit Code
```

但 **当前第一步不是直接写 Commandlet**。

#### V3.0-A — 当前开发切片：Validation Runner Refactor

已实现 `FValidationConfig + FValidationRunner`，`OnScanClicked()` 只读取输入、调用 Runner 和刷新展示。

- UE 5.8.2 `PipelineToolkitEditor Win64 Development` 构建成功。
- 插件已在无渲染 Editor 自动化进程中加载。
- `PipelineToolkit.Validation.Runner` 原生 Automation 测试成功：真实 Fixture 为 4 项 / PASS 2 / FAIL 1 / SKIP 1；同时验证输入去空白、可配置前缀、重复执行替换结果、失败清空状态与可选错误输出。
- 测试报告：`Saved/Automation/V3Runner/index.json`；日志：`Saved/Logs/V3RunnerAutomation.log`。UE 初始化时出现测试开始前的 `LogAutomationTest: Error: Condition failed`，来源未定位；本次 Runner 测试报告为 0 错误 / 0 警告。
- Slate 界面、Fix/Revert 与 UI JSON 导出尚待人工验证；V3.0-A 尚未完成全部验收，不进入 V3.0-B。

当前共享边界：

```text
FValidationConfig
├─ Scope
└─ BlueprintPrefix

FValidationRunner
├─ validate config
├─ Asset Registry discovery
├─ redirector filtering
├─ rule evaluation
├─ build FValidationRun
└─ recalculate summary
```

Slate 保留：

- TextBox 输入
- Status
- Summary / Result Rows
- Fix / Revert
- Editor-only interaction

第一阶段完成标准：

```text
Editor behavior unchanged
+
same fixture results
+
validation execution no longer owned by Slate
```

在此验证完成之前：

- 不实现 Commandlet
- 不实现 Python wrapper
- 不实现 GitHub Actions CI
- 不扩展 Rule Framework

#### V3.0-B — 后续：Editor Commandlet

Runner 重构验证后再进入：

```text
UE Editor Commandlet
    ↓
FValidationRunner
    ↓
FValidationRun
    ├── JSON Report
    └── Process Exit Code
```

---

## 3. 当前重要文件

```text
Plugins/ProductionPipelineToolkit/
└─ Source/ProductionPipelineToolkit/
   ├─ Private/
   │  ├─ ProductionPipelineToolkit.cpp
   │  ├─ UI/
   │  │  └─ SProductionPipelineToolkit.cpp
   │  ├─ Tests/
   │  │  └─ ValidationRunnerTests.cpp
   │  └─ Validation/
   │     ├─ BlueprintNamingRule.h
   │     ├─ BlueprintNamingRule.cpp
   │     ├─ ValidationRunner.cpp
   │     └─ ValidationReportWriter.cpp
   ├─ Public/
   │  └─ Validation/
   │     ├─ ValidationConfig.h
   │     ├─ ValidationRunner.h
   │     ├─ ValidationResult.h
   │     ├─ ValidationRun.h
   │     └─ ValidationReportWriter.h
   └─ ProductionPipelineToolkit.Build.cs
```

插件模块已声明为 `Editor`，保留 Editor API 依赖；未增加 Runtime 模块。

---

## 4. Git / 仓库状态

当前项目事实：

- GitHub 仓库已建立
- V2.0 已完成并同步上传 GitHub
- 主分支此前确认为 `main`，跟踪 `origin/main`
- UE 生成产物继续排除在版本控制之外
- `Saved/ValidationReports/` 为运行产物，不应作为源码提交

开始 Codex 修改前应实际执行：

```bash
git status
git log --oneline -5
```

涉及“已 build / 已 commit / 已 push”的结论必须以命令输出或 UE 实际结果为依据，不凭文档推断。

---

## 5. 工程约束

- 保持项目小、真实、易展示。
- 优先使用 Unreal Engine 原生机制。
- 非必要不抽象。
- 当前不引入 `IRule` / `RuleManager` / `RuleFactory` / 通用 Registry。
- JSON 是 `FValidationRun` 的输出边界，不创建第二套 Validation Model。
- Auto-Fix / Revert 当前属于 Editor Workflow。
- Commandlet 第一版只做 Validation，不自动修改资产。
- Python 未来只做 orchestration / glue，不承载业务 Rule。
- 不引入独立 Validation 可执行程序。
- 不为了展示而构建 Dashboard / 大型 Framework / AI-RAG 架构。
- 重要设计应以生产问题解释，而不是以技术名词解释。

### 文档开发准则

**所有项目长期文档必须输出中文 + English 双语版本。**

默认结构：

```text
# 中文版
...
# English Version
...
```

文档职责：

- `README.md`：GitHub / 对外展示
- `PROJECT_STATE.md`：当前真实状态
- `ARCHITECTURE.md`：架构理由、边界、重要决策
- `SESSION_LOG.md`：按时间追加实际开发记录
- `AGENTS.md`：Codex / Agent 的仓库级开发规则与当前工作边界

文档不得把“计划”写成“已完成”。

---

## 6. Codex 当前交接任务

已实现的切片目标（仍待 Editor 人工验收）：

```text
Refactor the existing Editor validation execution into a reusable
FValidationRunner driven by FValidationConfig.
```

范围：

- 把 Asset Registry discovery 移出 Slate
- 把 Redirector filtering 移出 Slate
- 把 BlueprintNamingRule evaluation 移出 Slate
- 把 `FValidationRun` construction / summary 移出 Slate
- 保留 Slate presentation
- 保留 Fix / Revert 在 Editor UI
- 保持 JSON schema 不变
- 不实现 Commandlet
- 不引入 Rule Framework

验收：

```text
BP_PlayerTest → PASS
Test_Player   → FAIL
M_Test        → SKIP
BP_EnemyTest  → PASS
```

且 Editor UI 的现有行为保持一致。

下一步人工验收：默认 Scope `/Game/ValidationTest`、Prefix `BP_` 扫描并检查 Summary / Result Rows；Fix `Test_Player` 后检查更名及重新扫描；Revert 后恢复基线；导出 JSON 检查 `schema_version = 1` 和结果字段；空白 Scope / Prefix 后确认旧结果清空。完成后才能进入 Commandlet。

---

# English Version

## 1. Project Identity

- Project: `PipelineToolkit`
- Plugin: `ProductionPipelineToolkit`
- Unreal Engine: `5.8.2`
- Platform: Windows
- Project type: Game → Blank → C++
- Target roles: game-industry Tools / Systems / Pipeline Engineer
- Project type: portfolio prototype, not a commercial general-purpose plugin

Portfolio principle:

> Explain the value in 30 seconds; explain the implementation in 30 minutes.

The project is intended to demonstrate:

- Unreal Engine C++ engineering
- Editor tooling / Slate workflow design
- rule-driven asset validation
- Automation / Commandlet / CI thinking
- basic observability and performance awareness
- module boundaries and engineering judgment
- evidence-driven abstraction

---

## 2. Milestone Status

```text
V0  Editor Plugin + Slate Toolkit        ✅
V1  Asset Registry + Validation Core     ✅
V2  Production Validation Workflow       ✅
V3  Headless Validation & Automation     ▶ Active
```

### V0 — complete

Verified:

```text
Plugin
→ Module
→ Editor Integration
→ Slate UI
→ User Interaction
```

Includes the UE C++ project and plugin, module lifecycle, Nomad Tab, ToolMenus entry, Slate UI, and Scan interaction.

### V1 — complete

Verified vertical slice:

```text
Asset Registry
    ↓
FAssetData
    ↓
FBlueprintNamingRule
    ↓
FValidationResult
    ↓
Slate Editor Display
```

Fixture:

```text
/Game/ValidationTest/BP_PlayerTest
/Game/ValidationTest/Test_Player
/Game/ValidationTest/M_Test
/Game/ValidationTest/Characters/BP_EnemyTest
```

Baseline:

```text
BP_PlayerTest → PASS
Test_Player   → FAIL
M_Test        → SKIP
BP_EnemyTest  → PASS
```

### V2 — complete and synced to GitHub

V2 turned validation into a coherent Editor workflow:

```text
Asset Registry
      ↓
Rule Evaluation
      ↓
FValidationResult
      ↓
FValidationRun
      ├── Editor Summary / Details
      ├── Rule Configuration
      ├── Safe Auto-Fix / Explicit Revert
      └── JSON Report
```

Verified:

- `FValidationRun` summary state
- configurable scan scope
- configurable Blueprint naming prefix
- stale-result clearing for invalid input
- deterministic Rename Auto-Fix
- explicit `Revert Last Fix`
- JSON output under `Saved/ValidationReports/ValidationReport.json`

### Current Auto-Fix truth

Generic Ctrl+Z Undo is no longer a project goal for Rename Auto-Fix.

Testing showed:

- `RenameAsset()` cannot simply be assumed to behave like a normal UObject transaction
- an outer `FScopedTransaction` caused instability / crash
- removing it stopped the crash but did not provide reliable Ctrl+Z undo

Current workflow:

```text
Fix
 ↓
UEditorAssetSubsystem::RenameAsset()
 ↓
remember Original / Fixed Path
 ↓
Revert Last Fix
 ↓
explicit rename back
```

Auto-Fix/Revert remains an Editor workflow; no generic Fix Framework is being introduced.

### V3 — active

V3 aims to move the validation workflow across the automation boundary while keeping one validation semantics path shared by Editor UI and Commandlet.

Target shape:

```text
Editor UI ─────┐
               ├── FValidationRunner
Commandlet ────┘
                       ↓
                FValidationRun
                  ├── JSON
                  └── Exit Code
```

The current first step is **not** Commandlet implementation.

#### V3.0-A — current slice: Validation Runner Refactor

`FValidationConfig + FValidationRunner` are implemented. `OnScanClicked()` now only reads input, calls the Runner, and refreshes presentation.

- UE 5.8.2 `PipelineToolkitEditor Win64 Development` build succeeded.
- The plugin loaded in the Editor automation process with rendering disabled.
- The native `PipelineToolkit.Validation.Runner` Automation test passed: the real fixture produced 4 results / PASS 2 / FAIL 1 / SKIP 1. It also verified trimming, configurable prefixes, replacement of results on repeated calls, state clearing on failure, and optional error output.
- Report: `Saved/Automation/V3Runner/index.json`; log: `Saved/Logs/V3RunnerAutomation.log`. UE initialization emitted `LogAutomationTest: Error: Condition failed` before the test started; the source is unresolved. The Runner test report contains 0 errors / 0 warnings.
- Slate presentation, Fix/Revert, and UI JSON export still require manual verification. Full V3.0-A acceptance is pending; do not start V3.0-B yet.

Current shared boundary:

```text
FValidationConfig
├─ Scope
└─ BlueprintPrefix

FValidationRunner
├─ validate config
├─ Asset Registry discovery
├─ redirector filtering
├─ rule evaluation
├─ build FValidationRun
└─ recalculate summary
```

Slate keeps:

- UI input
- status
- summary/result presentation
- Fix/Revert
- Editor-only interaction

Acceptance:

```text
Editor behavior unchanged
+
same fixture results
+
validation execution no longer owned by Slate
```

Do not implement Commandlet, Python, CI, or a larger Rule Framework before this slice is verified.

#### V3.0-B — next: Editor Commandlet

After the Runner refactor is verified:

```text
UE Editor Commandlet
    ↓
FValidationRunner
    ↓
FValidationRun
    ├── JSON Report
    └── Process Exit Code
```

---

## 3. Important Files

Current validation-related source:

```text
Plugins/ProductionPipelineToolkit/
└─ Source/ProductionPipelineToolkit/
   ├─ Private/
   │  ├─ ProductionPipelineToolkit.cpp
   │  ├─ UI/
   │  │  └─ SProductionPipelineToolkit.cpp
   │  ├─ Tests/
   │  │  └─ ValidationRunnerTests.cpp
   │  └─ Validation/
   │     ├─ BlueprintNamingRule.h
   │     ├─ BlueprintNamingRule.cpp
   │     ├─ ValidationRunner.cpp
   │     └─ ValidationReportWriter.cpp
   ├─ Public/
   │  └─ Validation/
   │     ├─ ValidationConfig.h
   │     ├─ ValidationRunner.h
   │     ├─ ValidationResult.h
   │     ├─ ValidationRun.h
   │     └─ ValidationReportWriter.h
   └─ ProductionPipelineToolkit.Build.cs
```

The plugin module is now declared as `Editor`, retaining its Editor API dependencies. No Runtime module was added.

---

## 4. Git / Repository State

Current project facts:

- GitHub repository exists
- V2.0 is complete and has been synced to GitHub
- the main branch was previously confirmed as `main` tracking `origin/main`
- UE-generated artifacts remain excluded from source control
- `Saved/ValidationReports/` is runtime output, not source

Before Codex changes code, verify:

```bash
git status
git log --oneline -5
```

Never infer build/commit/push state from documentation alone.

---

## 5. Engineering Constraints

- Keep the project small, real, and demonstrable.
- Prefer UE-native mechanisms.
- Do not abstract without evidence.
- Do not add `IRule`, `RuleManager`, `RuleFactory`, or a generic registry yet.
- JSON is an output of `FValidationRun`, not a second validation model.
- Auto-Fix/Revert remains an Editor workflow.
- The first Commandlet validates only; it does not mutate assets.
- Python is future orchestration/glue, not the rule layer.
- Do not introduce a separate validation executable.
- Do not add dashboards, large frameworks, or AI/RAG architecture for appearance.
- Explain design choices through production problems, not buzzwords.

### Documentation Rule

**All long-lived project documents must be maintained in both Chinese and English.**

Default shape:

```text
# 中文版
...
# English Version
...
```

Responsibilities:

- `README.md`: public / GitHub-facing overview
- `PROJECT_STATE.md`: current verified truth
- `ARCHITECTURE.md`: design rationale, boundaries, decisions
- `SESSION_LOG.md`: append-oriented development history
- `AGENTS.md`: repository-level Codex/agent rules and current work boundary

Do not record planned work as completed work.

---

## 6. Current Codex Handoff

Implemented slice objective (manual Editor acceptance is still pending):

```text
Refactor the existing Editor validation execution into a reusable
FValidationRunner driven by FValidationConfig.
```

Scope:

- move Asset Registry discovery out of Slate
- move redirector filtering out of Slate
- move `BlueprintNamingRule` evaluation out of Slate
- move `FValidationRun` construction/summary out of Slate
- keep Slate presentation
- keep Fix/Revert in Editor UI
- preserve JSON schema
- do not implement Commandlet yet
- do not introduce a Rule Framework

Acceptance fixture:

```text
BP_PlayerTest → PASS
Test_Player   → FAIL
M_Test        → SKIP
BP_EnemyTest  → PASS
```

Existing Editor behavior must remain unchanged.

Next manual checks: scan Scope `/Game/ValidationTest` with Prefix `BP_` and inspect Summary / Result Rows; Fix `Test_Player` and check the rename and refreshed scan; Revert and confirm the baseline; export JSON and check `schema_version = 1` and result fields; submit blank Scope / Prefix and confirm stale results are cleared. Complete these before starting Commandlet.
