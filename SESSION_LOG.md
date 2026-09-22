# UE Production Pipeline Toolkit — 会话日志 / Session Log

> 中文部分为项目协作的主要版本，用于记录过程、纠错与补充。
> English section is the GitHub-facing companion version.
> 中文优先；英文完整追加于文档后半部分。

---

# 中文版

> 按时间顺序记录开发过程。本文以追加为主；旧记录如过长，可在不丢失关键决策的前提下归档总结。

## 2026-09-07 — 项目 / UE 环境建立

### 计划
建立一个面向游戏行业 Tools / Pipeline / Systems 岗位的 UE Editor 工具型作品集项目。

### 实际
- 确认 Unreal Engine `5.8.2` 环境。
- 确认 Windows 开发环境。
- 创建 `PipelineToolkit`：Game → Blank → C++ 项目。
- 确认工具型项目不需要 Starter Content。
- 创建 `ProductionPipelineToolkit` Plugin。
- 建立基础 Plugin / Module 结构。

### 关键决策
采用 Editor Tool 项目，而不是传统 gameplay Demo。目的是把已有的 C++ / 工具 / 进程 / 可观测性经验迁移到 Unreal 生产上下文中。

### 结果
UE 项目与 Plugin 骨架成功建立。

---

## 2026-09-07 — V0 Plugin Bootstrap

### 计划
通过最小可运行的 Editor Plugin 建立 UE 工程直觉。

### 实际
已实现并验证：

- Module 生命周期
- `StartupModule()` / `ShutdownModule()`
- Nomad Tab
- ToolMenus 集成
- Slate Widget
- Status 显示
- Scan 交互

Editor 验证结果：

```text
Production Pipeline Toolkit
Status: Ready
```

点击 Scan 后：

```text
Status: Scan Requested
```

### 关键决策
V0 有意几乎不包含业务逻辑，主要验证：

```text
Plugin → Module → Editor integration → Slate → interaction
```

### 结果
V0 Bootstrap 完成。

---

## 2026-09-07 → 2026-09-17 — 项目架构收敛

### 计划
把项目定义为“生产流程工具原型”，而不是孤立的 Asset Checker。

### 决策
- Rule Engine 使用 UE 原生能力，不设计自定义 DSL。
- Editor UI 与后期 Commandlet 共享 Validation Core。
- Commandlet 优先于独立进程。
- Observability 初期从简单表格 / timing 开始，后期再用 SQLite。
- Python 作为 CI 编排 / glue，而不是 Validation Core。
- 跨平台 Policy / Configuration 用来展示 Policy 与 Execution 分离。
- Safe Auto-Fix 应有限且可撤销。
- Material Complexity 估算在有可靠指标前暂时延后。
- Asset Validation 最终应支持定向 / dirty-asset 验证，而不是每次变化都全项目扫描。

### 结果
最终方向收敛为 Tools + Pipeline + Systems / Performance 思维，同时避免不必要的 Framework 复杂度。

---

## 2026-09-17 — Git 仓库清理

### 计划
为 GitHub 面试复现准备项目仓库。

### 实际
- 初始化 Git 仓库。
- 区分 Unreal 生成内容与应纳入版本控制的项目内容。
- 排除生成目录和 IDE 工程文件，例如 `*.slnx`。
- 保留 `Config/`、`.uproject`、`.uplugin`、源码和项目自有资源。

### 第三方依赖决策
保留 `Plugins/VisualStudioTools/` 作为第三方依赖，以提高项目复现性。

其源码 / 文档 / License 与项目自身实现及许可证保持边界分离。

### License 决策
为项目自有原创代码加入 `LICENSE`，采用 MIT。
加入 `THIRD_PARTY_NOTICES.md`，记录依赖及许可证边界。

### 结果
仓库内容被有意识地控制为小而以源码为中心的结构。

---

## 2026-09-17 — GitHub 上传 / 仓库验证

### 实际
- GitHub 仓库上传完成。
- 观察到仓库约 1.2 MB。
- 使用 `git ls-files` 检查实际被 Git 跟踪的内容。
- 确认本地 UE 工作目录较大主要来自生成物 / 缓存 / 开发数据，不应整体镜像到 GitHub。

### 关键结论
作品集仓库应优化为：

```text
Clone → 使用已安装的 UE 5.8.2 → Rebuild → 打开项目
```

而不是打包整个本地 Unreal 工作区快照。

---

## 2026-09-20 — 项目连续性文件体系建立

### 问题
长对话会不断增加上下文成本，也更难干净地恢复工作现场。

### 决策
使用三个职责不重叠的文件维护项目连续性：

- `PROJECT_STATE.md` — 当前真实状态
- `ARCHITECTURE.md` — 设计理由与护栏
- `SESSION_LOG.md` — 时间顺序的历史记录

### 补充建议
如果以后 AI 协作规范越来越多，增加单独的 `AGENTS.md`（或等价文件），不要继续把操作规范塞进 `PROJECT_STATE.md`。

### 当前状态
V0 已完成，V1 为当前里程碑。

### 下一会话重点
从 UE 的概念边界开始：

```text
Content Browser Asset
vs
Level Actor
```

然后实现：

```text
Asset Registry
→ Rule
→ Evaluate
→ Result
→ Editor display
```

在 V1 验证数据流真正存在并可演示之前，不进入 SQLite、Commandlet、CI、Blender 或 AI。

---

## 2026-09-20 — 按 AGENTS.md 核对连续性文件并恢复现场

### 计划
读取 Desktop `AGENTS.md` 作为项目基线，检查并更新 `PROJECT_STATE.md`、`ARCHITECTURE.md`、`SESSION_LOG.md`，在当前设计与进度上继续，不重新设计。

### 实际
- 以 Desktop `AGENTS.md` 为协作基线：V0 已完成，当前里程碑是 V1；先讨论 Asset / Level Actor，再写代码。
- 本地验证 Git：分支 `main`，跟踪 `origin/main`，working tree clean，HEAD `0637f81`（`V0: Bootstrap UE Editor toolkit`），远程 `https://github.com/azv147/PipelineToolkit.git`。
- 核对插件源码：Nomad Tab、Window 菜单、Slate Status/Scan 占位交互与 V0 描述一致；Scan 尚未接入 Asset Registry。
- 纠正 `PROJECT_STATE.md` 中已过时或不准确的事实：分支已确认是 `main`；仓库中没有 `Resources/Icon128.png`；记录当前 `.uplugin` Module 类型与 `.uproject` 插件发现方式。
- `ARCHITECTURE.md` 护栏未改，只追加“在已接受架构上继续”的决策记录。
- 协作规范仍放在 Desktop `AGENTS.md`，不写入 `PROJECT_STATE.md`。

### 结果
连续性文件已与仓库真实状态对齐。下一工作仍是 V1 第一步的设计讨论，而不是写 Rule Engine。

---

## 当前交接检查表

以后每次结束会话前：

- `PROJECT_STATE.md`：只有“当前真实状态”发生变化时才更新
- `ARCHITECTURE.md`：只有设计决策 / 约束发生变化时才更新
- `SESSION_LOG.md`：追加本次实际做了什么、为什么这样做、剩余什么

每次新会话开始时，建议按这个顺序读取：

1. `PROJECT_STATE.md`
2. 只有需要设计背景时再读 `ARCHITECTURE.md`
3. 需要追溯最近进度 / 上次停在哪里时再读 `SESSION_LOG.md`

任何涉及构建状态的结论，都应以实际项目结果为依据，例如 `git status`、UE Build 结果、Log 或对应工具输出，不要仅凭记忆假设。

---

# English Version

> Chronological development record. Keep this file append-oriented; summarize older history rather than rewriting it into project state.

## 2026-09-07 — Project / UE Environment Established

### Plan
Create a UE Editor-tool portfolio project aimed at game-industry Tools / Pipeline / Systems roles.

### Actual
- Confirmed Unreal Engine `5.8.2` environment.
- Confirmed Windows development environment.
- Created `PipelineToolkit` as a Game → Blank → C++ project.
- Confirmed Starter Content was not required for the tool-focused project.
- Created `ProductionPipelineToolkit` plugin.
- Established the basic plugin/module structure.

### Key decision
Use an Editor-tool project rather than a conventional gameplay demo. The purpose is to transfer existing C++ / tooling / process / observability experience into Unreal production context.

### Result
The UE project and plugin skeleton were successfully created.

---

## 2026-09-07 — V0 Plugin Bootstrap

### Plan
First establish UE project intuition through a minimal end-to-end Editor plugin.

### Actual
Implemented and verified:

- module lifecycle
- `StartupModule()` / `ShutdownModule()`
- Nomad tab
- ToolMenus integration
- Slate widget
- status display
- Scan interaction

Editor verification reached:

```text
Production Pipeline Toolkit
Status: Ready
```

After clicking Scan:

```text
Status: Scan Requested
```

### Key decision
V0 intentionally contains almost no business logic. It proves the path:

```text
Plugin → Module → Editor integration → Slate → interaction
```

### Result
V0 bootstrap complete.

---

## 2026-09-07 → 2026-09-17 — Project Architecture Consolidated

### Plan
Define a production-pipeline tool prototype rather than an isolated asset checker.

### Decisions
- Rule Engine uses UE-native capabilities; no custom DSL.
- Editor UI and later Commandlet share a validation core.
- Commandlet is preferred over a separate process.
- Observability should begin with simple tables/timing and later use SQLite.
- Python is orchestration glue for CI, not the validation core.
- Cross-platform policy/configuration can demonstrate separation of policy and execution.
- Safe auto-fix should be limited and undoable.
- Material-complexity estimation is deferred until a reliable metric is justified.
- Asset validation should eventually support targeted/dirty-asset validation rather than constant full-project rescans.

### Result
The target architecture stabilized around Tools + Pipeline + Systems/Performance thinking without unnecessary framework complexity.

---

## 2026-09-17 — Git Repository Cleanup

### Plan
Prepare the project for GitHub-based interview reproduction.

### Actual
- Initialized Git repository.
- Reviewed generated UE content versus source-controlled project content.
- Excluded generated directories and IDE project files such as `*.slnx`.
- Kept `Config/`, `.uproject`, `.uplugin`, source code and project-owned resources.

### Third-party dependency decision
Kept `Plugins/VisualStudioTools/` in the repository as a third-party dependency to improve reproducibility.

Its source/documentation/license remain separate from the project's own implementation and license.

### Licensing decision
Added project `LICENSE` using MIT for the project's own original code.
Added `THIRD_PARTY_NOTICES.md` to document dependency/license boundaries.

### Result
Repository content was made intentionally small and source-focused.

---

## 2026-09-17 — GitHub Upload / Repository Validation

### Actual
- GitHub repository upload completed.
- Repository size observed at approximately 1.2 MB.
- `git ls-files` reviewed to verify tracked content.
- Confirmed large local UE workspace size is primarily generated/cache/development data and should not be mirrored to GitHub.

### Key conclusion
The portfolio repository should optimize for:

```text
Clone → use installed UE 5.8.2 → rebuild → open project
```

rather than shipping a snapshot of the entire local UE workspace.

---

## 2026-09-20 — Project Continuity System Introduced

### Problem
Long conversations increase context cost and make clean continuation harder.

### Decision
Maintain project continuity through three files with non-overlapping responsibilities:

- `PROJECT_STATE.md` — current truth
- `ARCHITECTURE.md` — design rationale and guardrails
- `SESSION_LOG.md` — chronological history

### Additional recommendation
If agent-specific working instructions become substantial, add a separate `AGENTS.md` (or equivalent) rather than putting procedural instructions into `PROJECT_STATE.md`.

### Current state
V0 is complete. V1 is the active milestone.

### Next session focus
Start from the UE concept boundary:

```text
Content Browser Asset
vs
Level Actor
```

Then implement:

```text
Asset Registry
→ Rule
→ Evaluate
→ Result
→ Editor display
```

Do not jump to SQLite, Commandlet, CI, Blender, or AI before the V1 validation path is real and demonstrable.

---

## 2026-09-20 — Continuity files verified against AGENTS.md

### Plan
Treat Desktop `AGENTS.md` as the baseline, check and update the three continuity files, then continue from the current design and progress without redesigning.

### Actual
- Used Desktop `AGENTS.md` as the collaboration baseline: V0 complete, V1 current; discuss Asset / Level Actor before writing code.
- Verified Git locally: branch `main`, tracking `origin/main`, working tree clean, HEAD `0637f81` (`V0: Bootstrap UE Editor toolkit`), remote `https://github.com/azv147/PipelineToolkit.git`.
- Confirmed plugin source matches V0: Nomad tab, Window menu, Slate Status/Scan placeholder; Scan is not connected to Asset Registry.
- Corrected stale facts in `PROJECT_STATE.md`: branch is `main`; `Resources/Icon128.png` is absent; recorded current `.uplugin` module type and `.uproject` plugin discovery.
- `ARCHITECTURE.md` guardrails unchanged; added a “continue from accepted architecture” decision record.
- Collaboration instructions remain in Desktop `AGENTS.md`, not in `PROJECT_STATE.md`.

### Result
Continuity files now match repository truth. Next work remains the V1 design discussion, not a Rule Engine implementation.

---

## Current Handoff Checklist

Before ending each future session, update:

- `PROJECT_STATE.md`: only if the current truth changed
- `ARCHITECTURE.md`: only if a design decision/constraint changed
- `SESSION_LOG.md`: append what happened and what remains

At the start of a new session, read in this order:

1. `PROJECT_STATE.md`
2. `ARCHITECTURE.md` only when design context is needed
3. `SESSION_LOG.md` for recent history / previous stopping point

Before making claims about build state, verify with the actual project (`git status`, UE build result, logs, or the relevant tool output).
