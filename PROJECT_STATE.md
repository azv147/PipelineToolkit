# UE Production Pipeline Toolkit — 项目状态 / Project State

> 当前真实状态文档。项目事实发生变化时更新。
> Current-truth document. Update when verified project facts change.
>
> 最后更新 / Last updated: 2026-10-03

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

V3.0-A 与 V3.0-B 已完成验收；当前准备 V3.1 本地 Python 编排，尚未实现。

#### V3.0-A — 已完成：Validation Runner Refactor

已实现 `FValidationConfig + FValidationRunner`，`OnScanClicked()` 只读取输入、调用 Runner 和刷新展示。

- UE 5.8.2 `PipelineToolkitEditor Win64 Development` 构建成功。
- 插件已在无渲染 Editor 自动化进程中加载。
- `PipelineToolkit.Validation.Runner` 原生 Automation 测试成功：真实 Fixture 为 4 项 / PASS 2 / FAIL 1 / SKIP 1；同时验证输入去空白、可配置前缀、重复执行替换结果、失败清空状态与可选错误输出。
- 测试报告：`Saved/Automation/V3Runner/index.json`；日志：`Saved/Logs/V3RunnerAutomation.log`。UE 初始化时出现测试开始前的 `LogAutomationTest: Error: Condition failed`，来源未定位；本次 Runner 测试报告为 0 错误 / 0 警告。
- 2026-10-02 用户确认人工功能验证与代码评审完成，Editor 展示、Scan、Fix/Revert、JSON 导出及配置错误后的状态清理通过验收。该结论来自用户确认，并非本次 Agent 重跑 Editor。V3.0-A 已完成。

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

第一阶段验收标准（已完成）：

```text
Editor behavior unchanged
+
same fixture results
+
validation execution no longer owned by Slate
```

V3.0-A 验收门槛已满足，允许进入 Commandlet；Python wrapper、GitHub Actions CI 与通用 Rule Framework 仍不属于 V3.0-B。

#### V3.0-B — 已完成：Editor Commandlet

已实现 `UValidateAssetsCommandlet`，复用 Runner / Writer，并在 Commandlet 环境跳过模块的 Tab / ToolMenus 注册与清理。

- UE Editor 构建成功；9 个实际进程场景通过：默认 1、Characters 0、自定义 Prefix 1、空白 Scope/Prefix/ReportPath 2、报告写入失败 2、缺少参数值 2、不存在的 Scope 0（空结果）。
- 默认 JSON schema 1，4 项 / PASS 2 / FAIL 1 / SKIP 1；带空格相对路径与绝对路径报告通过。
- 原有 Runner Automation 在普通 Editor 启动路径下通过，报告 1 项成功 / 0 错误 / 0 警告；全部 `.uasset` 运行前后 SHA-256 一致。
- 证据：`Saved/Automation/V3Commandlet/Results.json`（完整进程参数和退出码）、`Build.log`、`EditorRegression/index.json` 及各场景日志。
- 2026-10-03 用户确认本切片验收完毕，关闭 V3.0-B。人工验收结论来自用户确认，本次 Agent 未重跑 Editor / build / Automation。初始化 `Condition failed` 仍未定位，不声称整个启动日志无错误。

当前执行边界：

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

#### Rule / Writer 合同测试 — 已完成（2026-10-03）

- 新增原生 Automation `PipelineToolkit.Validation.BlueprintNamingRule` 和 `PipelineToolkit.Validation.ReportWriter`。Rule 使用构造的资产元数据；Writer 在独立 GUID 临时目录写入并解析报告，不依赖或修改 Fixture。
- 覆盖规则 PASS / FAIL / SKIP、自定义前缀、结果身份和修复建议；覆盖 schema 1、字段与计数、三种状态、可选建议名称、中文 / 引号 / 反斜杠 / 换行往返、空 Run 和写入失败诊断。当前 Writer 通过错误字符串而非日志报告写入失败，测试明确断言该诊断。
- 实际 Editor 构建成功；整个 Validation 组 3 项成功，新增测试重跑 2 项成功；两轮进程退出码均为 0，各测试 0 错误 / 0 警告。四个 `.uasset` SHA-256 不变，临时目录已清理；生产接口、实现和 schema 未修改。
- 证据：`Saved/Automation/ValidationContractsEvidence/` 的 `Build.log`、`AllRetry/index.json`、`Repeat/index.json`、各轮日志 / 退出码和 `Verification.json`。
- 首次带 `-nosound -NoShaderCompile -nop4` 的 Automation 启动在测试开始前崩溃（`Null assigned to TNotNull`，进程 -1073741571）。改用此前成功的 `-unattended -NullRHI -nosplash` 参数后两轮成功；未定位崩溃原因，不声明参数因果关系。启动 `Condition failed` 仍存在且未解决。
- 下一节点仍为 V3.1 Python 编排，准备中，尚未实现。本次没有重新执行 Commandlet 场景或人工 UI 回归，没有 commit / push。

## 3. 当前重要文件

```text
Plugins/ProductionPipelineToolkit/
└─ Source/ProductionPipelineToolkit/
   ├─ Private/
   │  ├─ ProductionPipelineToolkit.cpp
   │  ├─ Commandlets/
   │  │  ├─ ValidateAssetsCommandlet.h
   │  │  └─ ValidateAssetsCommandlet.cpp
   │  ├─ UI/
   │  │  └─ SProductionPipelineToolkit.cpp
   │  ├─ Tests/
   │  │  ├─ BlueprintNamingRuleTests.cpp
   │  │  ├─ ValidationReportWriterTests.cpp
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

## 6. Codex 当前交接任务 — V3.1（准备中，尚未实现）

V3.0-B 已验收关闭。下一节点计划新增 `Scripts/run_validation.py`，仅使用 Python 标准库，将手工调用变为可重复执行的本地自动化入口。以下为开发合同，不代表已实现：

- CLI：必填 `--ue-cmd`、`--project`；可选 `--scope`、`--blueprint-prefix`、`--report-path`，默认值沿用 Commandlet，相对报告路径以项目目录为基准。
- 使用参数列表启动现有 Commandlet，不通过 shell 拼接命令；支持路径中的空格并转发 UE 日志。
- UE 返回 0 / 1 时读取本次 schema 1 报告，显示资产总数和 PASS / FAIL / SKIP；返回 2 时不消费可能遗留的旧报告。
- 保留 0 / 1 / 2 合同；脚本参数错误、启动失败、异常进程退出、报告缺失 / 无法解析 / schema 不支持统一返回 2，并说明原因。
- Python 只编排进程与消费报告，不实现 Rule / Discovery、不修改资产或 C++ 验证语义、不重设计 JSON。此切片不自动构建 UE、不加入 CI 或通用框架。
- 验收：通过脚本实际运行默认 Fixture 得到 4 项 / PASS 2 / FAIL 1 / SKIP 1、退出码 1；Characters 得到 PASS 1、退出码 0；空配置、不可写报告目标及无效 UE 路径返回 2；带空格路径通过，失败不误读旧报告，资产无修改。
- 顺序：实现脚本 → 实际进程验证 → 人工验收 → 再单独确定 CI 执行环境与报告归档。

历史启动日志中的 `Condition failed` 仍为未定位观察，不因本次人工验收被标记为已修复。

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

V3.0-A and V3.0-B acceptance is complete. V3.1 local Python orchestration is being prepared, not implemented.

#### V3.0-A — complete: Validation Runner Refactor

`FValidationConfig + FValidationRunner` are implemented. `OnScanClicked()` now only reads input, calls the Runner, and refreshes presentation.

- UE 5.8.2 `PipelineToolkitEditor Win64 Development` build succeeded.
- The plugin loaded in the Editor automation process with rendering disabled.
- The native `PipelineToolkit.Validation.Runner` Automation test passed: the real fixture produced 4 results / PASS 2 / FAIL 1 / SKIP 1. It also verified trimming, configurable prefixes, replacement of results on repeated calls, state clearing on failure, and optional error output.
- Report: `Saved/Automation/V3Runner/index.json`; log: `Saved/Logs/V3RunnerAutomation.log`. UE initialization emitted `LogAutomationTest: Error: Condition failed` before the test started; the source is unresolved. The Runner test report contains 0 errors / 0 warnings.
- On 2026-10-02 the user confirmed completion of manual functional verification and code review, covering Editor presentation, Scan, Fix/Revert, JSON export, and state clearing after configuration errors. This is user-confirmed acceptance, not an Editor rerun by the agent in this session. V3.0-A is complete.

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

Acceptance criteria (completed):

```text
Editor behavior unchanged
+
same fixture results
+
validation execution no longer owned by Slate
```

The V3.0-A acceptance gate is satisfied, so Commandlet work may begin. Python wrappers, GitHub Actions CI, and a generic rule framework remain outside V3.0-B.

#### V3.0-B — complete: Editor Commandlet

`UValidateAssetsCommandlet` reuses the Runner / Writer. Module Tab / ToolMenus registration and cleanup are skipped in Commandlet mode.

- The UE Editor build succeeded. Nine actual process cases passed: default 1, Characters 0, custom Prefix 1, blank Scope/Prefix/ReportPath 2, report-writing failure 2, missing argument value 2, and nonexistent Scope 0 (empty results).
- Default JSON uses schema 1 with 4 assets / PASS 2 / FAIL 1 / SKIP 1. Reports to quoted relative paths containing spaces and absolute paths passed.
- Existing Runner Automation passed under ordinary Editor startup with 1 successful test / 0 errors / 0 warnings. All `.uasset` SHA-256 hashes were unchanged.
- Evidence: `Saved/Automation/V3Commandlet/Results.json` (full process arguments and exit codes), `Build.log`, `EditorRegression/index.json`, and per-case logs.
- On 2026-10-03 the user confirmed acceptance, closing V3.0-B. Manual acceptance is user-confirmed; the agent did not rerun Editor / build / Automation in this session. Initialization `Condition failed` remains unresolved; the entire startup log is not claimed to be error-free.

Current execution boundary:

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

#### Rule / Writer Contract Tests — complete (2026-10-03)

- Added native Automation `PipelineToolkit.Validation.BlueprintNamingRule` and `PipelineToolkit.Validation.ReportWriter`. Rule tests use constructed asset metadata; Writer tests serialize and parse reports in isolated GUID temporary directories without depending on or changing the fixture.
- Covers rule PASS / FAIL / SKIP, custom prefixes, result identity, and fix suggestions; schema 1, fields / counts, three statuses, optional suggestions, Chinese / quote / backslash / newline round trips, empty runs, and write-failure diagnostics. The current Writer returns write failures through an error string, not a log; tests explicitly assert that diagnostic.
- Actual Editor build succeeded. The full Validation group passed 3 tests; repeat execution passed 2 new tests. Both processes exited 0, with 0 errors / 0 warnings per test. All four `.uasset` SHA-256 hashes were unchanged, and temporary directories were cleaned. Production interfaces, implementations, and schema were unchanged.
- Evidence: `Saved/Automation/ValidationContractsEvidence/` contains `Build.log`, `AllRetry/index.json`, `Repeat/index.json`, logs / exit codes, and `Verification.json`.
- The initial Automation launch with `-nosound -NoShaderCompile -nop4` crashed before tests started (`Null assigned to TNotNull`, exit -1073741571). Both runs succeeded using the previously successful `-unattended -NullRHI -nosplash` flags. The crash cause is unresolved; no causal claim about individual flags is made. Startup `Condition failed` remains present and unresolved.
- Next is still V3.1 Python orchestration, preparation only. No Commandlet-case rerun, manual UI regression, commit, or push occurred in this session.

## 3. Important Files

Current validation-related source:

```text
Plugins/ProductionPipelineToolkit/
└─ Source/ProductionPipelineToolkit/
   ├─ Private/
   │  ├─ ProductionPipelineToolkit.cpp
   │  ├─ Commandlets/
   │  │  ├─ ValidateAssetsCommandlet.h
   │  │  └─ ValidateAssetsCommandlet.cpp
   │  ├─ UI/
   │  │  └─ SProductionPipelineToolkit.cpp
   │  ├─ Tests/
   │  │  ├─ BlueprintNamingRuleTests.cpp
   │  │  ├─ ValidationReportWriterTests.cpp
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

## 6. Current Codex Handoff — V3.1 (preparation, not implemented)

V3.0-B acceptance is closed. Plan `Scripts/run_validation.py`, using only the Python standard library, to turn manual invocation into a repeatable local automation entry point. The following is a development contract, not implemented behavior:

- CLI: require `--ue-cmd` and `--project`; offer `--scope`, `--blueprint-prefix`, and `--report-path` with Commandlet defaults. Resolve relative report paths from the project directory.
- Launch the existing Commandlet with an argument list without shell concatenation; support spaces in paths and forward UE logs.
- On UE exit 0 / 1, read the current schema 1 report and print asset / PASS / FAIL / SKIP counts. On exit 2, do not consume a possible old report.
- Preserve the 0 / 1 / 2 contract. Script argument errors, launch failures, abnormal process exits, missing / unreadable reports, and unsupported schemas return 2 with diagnostics.
- Python only orchestrates processes and consumes reports: no rule / discovery implementation, asset mutation, C++ validation semantics changes, or JSON redesign. No automatic UE build, CI, or generic framework in this slice.
- Acceptance: actual script runs produce default 4 assets / PASS 2 / FAIL 1 / SKIP 1 with exit 1, and Characters PASS 1 with exit 0. Blank configuration, unwritable report targets, and invalid UE paths return 2. Paths containing spaces work; failures do not consume stale reports; assets remain unchanged.
- Sequence: implement script → verify actual processes → manual acceptance → separately choose CI execution infrastructure and report archiving.

The historical startup `Condition failed` observation remains unresolved; this manual acceptance does not establish a fix.
