# UE Production Pipeline Toolkit — 项目状态 / Project State

> 中文部分为项目协作的主要版本，用于对话、纠错和校正。
> English section is the GitHub-facing companion version.
> 中文优先；英文完整追加于文档后半部分。

---

# 中文版

> 只记录当前真实状态。项目事实发生变化时更新此文件。
> 最后验证：2026-09-20（Asia/Singapore）
> 本轮核对：Desktop `AGENTS.md` 基线 + 本地仓库 `git` / 插件源码。不重新设计。

## 1. 项目身份

- 项目：`PipelineToolkit`
- 插件：`ProductionPipelineToolkit`
- Unreal Engine：`5.8.2`
- 平台：Windows
- 项目类型：Game → Blank → C++
- 方向：面向游戏行业 Tools / Systems / Pipeline Engineering 的 UE 作品集项目

## 2. 项目目标

制作一个小型、面向生产流程的 Unreal Editor 工具，用于展示：

- C++ / Unreal Engine 工程能力
- Editor 工具与工作流设计能力
- 规则驱动的资产验证
- 自动化 / Commandlet 思维
- 基础可观测性
- CI / Pipeline 集成思维
- 工程判断力：简单、可解释、可扩展

作品集原则：

> 30 秒讲清价值；30 分钟讲清实现。

这是一个作品集原型，而不是可直接商用的生产插件。

## 3. 当前里程碑

**V0 已完成。当前进入 V1。**

### V0 — 已完成

已验证：

- UE C++ 项目创建成功
- `ProductionPipelineToolkit` 插件创建成功
- Plugin Module 编译成功
- Plugin 能被 UE Editor 正常加载
- `StartupModule()` / `ShutdownModule()` 生命周期正常
- 注册 Nomad Tab
- 注册 Window 菜单项
- Slate UI 能在 Editor 中显示
- `Status: Ready` 正常显示
- 点击 `Scan` 后状态变为 `Status: Scan Requested`

当前插件入口与 Editor 集成已经可运行；下一步是构建真实功能，而不是继续做 Plugin 启动骨架。Scan 仍是占位交互（只改 Status 文本），尚未接入 Asset Registry 或任何 Rule。

### V1 — 当前目标

实现最小的真实验证数据流：

```text
Asset
  ↓
Rule
  ↓
Evaluate
  ↓
Validation Result / Issue
  ↓
Editor 展示
```

第一步的概念重点：

- 理解 `Content Browser Asset` 与 `Level Actor` 的区别
- 然后使用 `Asset Registry` 发现真实项目资产

当前不要跳到 SQLite、Commandlet、CI、Blender 或 AI。

第一步优先讨论设计（Asset 语义与 Content Browser Asset vs Level Actor），再写代码。不要直接写完整 Rule Engine。

## 4. 当前仓库状态

GitHub 仓库已经创建并完成上传。

远程：`https://github.com/azv147/PipelineToolkit.git`

当前仓库约 1.2 MB，这是因为 UE 生成的构建产物和缓存文件被排除在版本控制之外。

已追踪的项目内容包括：

- `PipelineToolkit.uproject`
- `Config/`
- `Source/`
- `Plugins/ProductionPipelineToolkit/`
- `Plugins/VisualStudioTools/` 中选定的源码 / 文档内容
- `README.md`
- `LICENSE`
- `THIRD_PARTY_NOTICES.md`

未追踪：

- `Binaries/`
- `Intermediate/`
- `Saved/`
- `DerivedDataCache/`
- `.vs/`
- `*.slnx`
- 其他生成的构建 / 缓存输出

### 第三方依赖

`Plugins/VisualStudioTools/` 被有意保留为第三方依赖，用于提高项目复现性。其原始源码、文档和许可证与本项目自己的代码及 MIT License 保持边界分离。

## 5. 当前工作树 / 分支状态

2026-09-20 本地验证：

- 分支：`main`（跟踪 `origin/main`，已同步）
- HEAD：`0637f81` — `V0: Bootstrap UE Editor toolkit`
- Working tree：clean
- GitHub 远程已连接；不要重新讨论本地生成物 / 缓存是否应整体上传
- 修改文件前仍应再跑 `git status`

## 6. 当前重要文件

自有 Plugin 源码：

```text
Plugins/ProductionPipelineToolkit/
├─ ProductionPipelineToolkit.uplugin
└─ Source/ProductionPipelineToolkit/
   ├─ Private/ProductionPipelineToolkit.cpp
   ├─ Private/UI/SProductionPipelineToolkit.cpp
   ├─ Private/UI/SProductionPipelineToolkit.h
   ├─ Public/ProductionPipelineToolkit.h
   └─ ProductionPipelineToolkit.Build.cs
```

补充事实（不是新设计）：

- 当前仓库中没有 `Resources/Icon128.png`
- 插件 Module 在 `.uplugin` 中为 `Type: Runtime` / `LoadingPhase: Default`，并依赖 Editor 模块（`UnrealEd`、`LevelEditor`、`ToolMenus`）
- `PipelineToolkit.uproject` 的 Plugins 列表未显式列出本插件；它作为项目 `Plugins/` 目录插件被发现
- 连续性文档已放入项目根目录（`PROJECT_STATE.md` / `ARCHITECTURE.md` / `SESSION_LOG.md`），当前尚未纳入 Git 跟踪

## 7. 当前约束

- 保持项目小而且易于展示。
- 优先使用 UE 原生机制。
- 不设计自定义 Validation DSL。
- 不引入独立 CLI 进程；后期使用 UE Editor Commandlet。
- 不为了外观而构建 Dashboard / 图表系统。
- 在真实验证数据路径出现之前，不要过度抽象。
- 重要设计应从生产价值出发解释，而不是堆砌技术名词。

## 8. 当前已确定的设计方向

初期规则应优先选择可靠且容易展示的检查，例如：

- 命名 / 路径规则
- 三角形数量限制
- LOD 存在性 / 数量限制
- 纹理分辨率 / 大小 / Mipmap 相关检查

Material Complexity 估算暂时延后，因为它可能产生误导，而且并不是证明核心架构所必需的。

Safe Auto-Fix 应限制为一到两个可逆案例，并使用 UE 的事务 / Undo 支持。

## 9. 下一步具体任务

从 V1 开始，仅推进第一小段真实数据流：

1. 明确这个工具中的 Unreal `Asset` 是什么
2. 理解 `Content Browser` Asset 与 `Level Actor` 的区别
3. 理解为什么 `Asset Registry` 适合作为资产发现机制
4. 设计最小 Rule / Evaluate / Result 模型
5. 实现一个真实 Rule，并在 Editor 中显示结果

本会话就绪点：停在第 1–2 步的设计讨论之前；尚未开始写 V1 代码。

在这条数据流跑通之前，不实现后续 Pipeline 层。

---

# English Version

> Current truth only. Update this file when a fact about the project changes.
> Last verified: 2026-09-20 (Asia/Singapore)
> This pass checked Desktop `AGENTS.md` plus local `git` / plugin source. No redesign.

## 1. Project Identity

- Project: `PipelineToolkit`
- Plugin: `ProductionPipelineToolkit`
- Engine: Unreal Engine `5.8.2`
- Platform: Windows
- Project type: Game → Blank → C++
- Direction: Unreal Engine Tools / Systems / Pipeline Engineering portfolio project

## 2. Goal

Build a small production-oriented Unreal Editor tool that demonstrates:

- C++ / Unreal Engine engineering ability
- Editor tooling and workflow design
- Rule-driven asset validation
- Automation / Commandlet thinking
- Basic observability
- CI / pipeline integration thinking
- Engineering judgment: simple, explainable, extensible

Portfolio principle:

> 30 seconds to explain the value; 30 minutes to explain the implementation.

This is a portfolio prototype, not a production-ready commercial plugin.

## 3. Current Milestone

**V0 is complete. V1 is next.**

### V0 — complete

Verified:

- UE C++ project created successfully
- `ProductionPipelineToolkit` plugin created
- plugin module builds successfully
- plugin loads in UE Editor
- `StartupModule()` / `ShutdownModule()` lifecycle works
- Nomad tab registered
- Window menu entry registered
- Slate UI appears in Editor
- `Status: Ready` displays
- `Scan` button changes status to `Status: Scan Requested`

The current plugin entry point and Editor integration are functional; the next work is feature construction rather than plugin bootstrap. Scan is still a placeholder (status text only) and has not reached Asset Registry or any rule.

### V1 — current target

Build the smallest real validation data flow:

```text
Asset
  ↓
Rule
  ↓
Evaluate
  ↓
Validation Result / Issue
  ↓
Editor display
```

First conceptual step:

- understand `Content Browser Asset` vs `Level Actor`
- then use `Asset Registry` to discover real assets

Do not jump to SQLite, Commandlet, CI, Blender, or AI yet.

Discuss design first (asset semantics and Content Browser Asset vs Level Actor) before writing code. Do not write a full Rule Engine yet.

## 4. Current Repository State

The GitHub repository has been created and uploaded.

Remote: `https://github.com/azv147/PipelineToolkit.git`

The repository is intentionally small (~1.2 MB) because UE-generated build and cache artifacts are excluded from version control.

Tracked project content includes:

- `PipelineToolkit.uproject`
- `Config/`
- `Source/`
- `Plugins/ProductionPipelineToolkit/`
- selected source/documentation for `Plugins/VisualStudioTools/`
- `README.md`
- `LICENSE`
- `THIRD_PARTY_NOTICES.md`

Not tracked:

- `Binaries/`
- `Intermediate/`
- `Saved/`
- `DerivedDataCache/`
- `.vs/`
- `*.slnx`
- other generated build/cache output

### Third-party dependency

`Plugins/VisualStudioTools/` is intentionally kept as a third-party dependency to improve project reproducibility. Its original source, documentation, and license remain separate from the project's own code and MIT license.

## 5. Current Working Tree / Branch

Verified locally on 2026-09-20:

- branch: `main` (tracking `origin/main`, up to date)
- HEAD: `0637f81` — `V0: Bootstrap UE Editor toolkit`
- working tree: clean
- GitHub remote connected; do not reopen whether generated/cache data should be uploaded wholesale
- still run `git status` before modifying files

## 6. Important Current Files

Own plugin source:

```text
Plugins/ProductionPipelineToolkit/
├─ ProductionPipelineToolkit.uplugin
└─ Source/ProductionPipelineToolkit/
   ├─ Private/ProductionPipelineToolkit.cpp
   ├─ Private/UI/SProductionPipelineToolkit.cpp
   ├─ Private/UI/SProductionPipelineToolkit.h
   ├─ Public/ProductionPipelineToolkit.h
   └─ ProductionPipelineToolkit.Build.cs
```

Additional facts (not a redesign):

- `Resources/Icon128.png` is not present in the current repository
- the plugin module is `Type: Runtime` / `LoadingPhase: Default` in `.uplugin`, with Editor module dependencies (`UnrealEd`, `LevelEditor`, `ToolMenus`)
- `PipelineToolkit.uproject` does not list this plugin explicitly; it is discovered from the project `Plugins/` folder
- continuity documents now live at the project root (`PROJECT_STATE.md` / `ARCHITECTURE.md` / `SESSION_LOG.md`) and are not Git-tracked yet

## 7. Current Constraints

- Keep the project small and demonstrable.
- Prefer UE-native mechanisms.
- Do not create a custom validation DSL.
- Do not introduce a separate CLI process; use UE Editor Commandlet later.
- Do not build dashboards/charts just for appearance.
- Do not over-abstract before a real validation data path exists.
- Explain important design decisions in terms of production value, not buzzwords.

## 8. Known Design Direction

Initial rules should favor reliable and easy-to-demonstrate checks, such as:

- naming/path rules
- triangle-count limits
- LOD presence / limits
- texture resolution / size / mipmap-related checks

Material-complexity estimation is intentionally deferred because it can be misleading and is not necessary to prove the core architecture.

Safe auto-fix should be limited to one or two reversible cases and use UE transaction/undo support.

## 9. Next Concrete Step

Start V1 by implementing only the first conceptual slice:

1. What an Unreal `Asset` is for this tool
2. `Content Browser` asset vs `Level Actor`
3. Why `Asset Registry` is the right discovery mechanism
4. Minimal Rule / Evaluate / Result model
5. Implement one real rule and display its result in the Editor

Ready point for this session: before the design discussion for steps 1–2; V1 code has not started.

Do not implement later pipeline layers until this data path works.
