# UE Production Pipeline Toolkit — 架构 / Architecture

> 中文部分为项目协作的主要版本，用于讨论、纠错和架构校正。
> English section is the GitHub-facing companion version.
> 中文优先；英文完整追加于文档后半部分。

---

# 中文版

> 设计护栏与设计理由。本文回答“为什么”和“边界在哪里”，而不是记录完整实现。
> 最后核对：2026-09-20。设计护栏未变；继续以已收敛架构为基线，不重新设计。

## 1. 架构意图

这是一个小型 Unreal Editor 生产工具原型。架构需要体现生产工具工程判断力，但不能把作品集 Demo 变成一个“为了展示架构而存在的框架项目”。

主要数据流：

```text
UE Asset / Project Data
        ↓
   Rule Engine
        ↓
   Issue Model
        ↓
Validation Result
        ↓
 Editor / Report
        ↓
 Commandlet
        ↓
 Python / CI Gate
```

只有在真实用例需要时才扩展新的层。

## 2. 核心边界

### Validation Core

负责真正的验证语义：

- Rule 执行
- Issue / Result 创建
- 确定性的验证行为

不应依赖 Slate UI。

### Editor UI

负责展示与交互：

- 触发验证
- 展示结果
- 在可行时定位 / 选中相关 Asset
- 后期提供 Safe Fix

UI 应消费验证结果，而不是承载验证逻辑。

### Asset Discovery

使用 Unreal 原生 Asset Registry 发现项目内容，不再发明一个平行的索引系统。

重要概念区分：

- Content Browser Asset = 项目 / Package 中的数据对象
- Level Actor = World / Level 中的对象实例

V1 首先关注 Asset。

### Automation Boundary

后期让同一套 Validation Core 可以由 Editor Commandlet 调用。

优先使用 Commandlet，而不是独立可执行程序，因为 Unreal 本身已经是必要的执行环境。

### CI Boundary

后期采用：

```text
Python wrapper
    ↓
UE Commandlet
    ↓
JSON / exit code
    ↓
GitHub Actions
```

Python 是编排 / 胶水层，而不是业务规则的归属地。

## 3. Rule Engine 决策

### 使用 UE 原生的 Reflection / 轻量数据结构

在确实有帮助时使用适当的 `USTRUCT`、`UCLASS`、`UPROPERTY`、Reflection 以及普通 C++ 类型，让 Editor / Config 等能力自然接入。

### 不构建自定义 DSL

原因：

- 增加实现成本
- 增加解析 / 测试复杂度
- 对当前项目规模价值很低
- 会把精力从真正的 Tool / Pipeline 工程问题上移开

### 不提前构建 Registry Framework

先用最小表示执行一个真实 Rule。只有当第二、第三个 Rule 证明某种抽象确实有价值时，才引入 Rule 注册 / 配置等机制。

## 4. Validation Result / Issue Model

一个 Rule 在概念上应该回答：

```text
Input asset
   ↓
Evaluate
   ↓
Pass / Fail / Warning
   ↓
Issue information
```

Result 应同时对人类和未来的自动化有用。至少能够识别：

- Asset
- Rule / Check identity
- Severity 或 Pass/Fail 状态
- 面向人的可读消息

只有在真实用例出现时才添加额外元数据。

## 5. Observability

工具本身最终也应该是可观测的。

初期可观测性：

- 检查的 Asset 总数
- Issue 数量
- 整体扫描耗时
- 必要时记录 Rule / Run 级别的耗时

后期可以使用 SQLite 持久化。

不要建立可视化平台。一个表格加可查询的历史记录已经足够支撑作品集目标。

## 6. 增量验证方向

保存时或 Editor 触发的验证不应从“每次变化都扫描整个项目”开始。

推荐演进：

```text
当前 Level / Selected Scope
        ↓
Dirty / Changed Assets
        ↓
Targeted Validation
```

这样不需要真正做到生产规模，也可以展示性能意识和 Systems 思维。

## 7. Safe Auto-Fix

Auto-Fix 的优先级低于诊断。

只有满足以下条件的修复才值得加入：

- 确定性
- 可逆
- 低风险
- 容易解释

使用 UE Transaction / Undo 支持（在合适的位置使用 `FScopedTransaction`），保证 Editor 中可以安全演示。

## 8. Configuration Strategy

长期设计应把 **Policy** 与 **Execution** 分离。

示意：

```text
Policy: PC
Policy: Mobile
Policy: Console
        ↓
Same validation engine
        ↓
Different thresholds / requirements
```

跨平台配置用于展示“策略与执行分离”的工程思维，而不是要求真正搭建多套平台流水线。

## 9. DCC / Blender 集成

DCC 集成属于后期扩展，不是 MVP 依赖。

未来可能形成：

```text
DCC
 ↓
Python precheck
 ↓
Export
 ↓
UE validation
 ↓
CI
```

即使没有 Blender，UE Validation Core 也必须独立有价值。

## 10. AI 边界

AI 是可选增强层，不应成为项目基础。

未来可以考虑：

```text
Natural-language request
        ↓
AI
        ↓
Tool API
        ↓
Rule / Validation system
```

不要为了“证明 AI 能力”而加入 RAG / Agent 架构。

## 11. 依赖方向

推荐方向：

```text
UI
 ↓
Validation Core
 ↓
UE data / Asset Registry
```

自动化同样应该调用同一份 Core，而不是复制业务逻辑：

```text
Editor UI ─┐
           ├→ Validation Core
Commandlet ┘
```

Python / CI 保持在 Core 外部。

## 12. 明确的非目标

除非新的证据证明有必要，否则不要加入：

- 自定义 DSL
- 独立 Validation 可执行程序
- 大型 Plugin Framework
- 在真正需要之前堆叠 Factory / Strategy / Registry 层
- Dashboard / 图表平台
- 大型 AI / RAG 架构
- 大型展示用游戏场景
- 大量投机性的 Rule

## 13. 决策记录格式

当重要设计发生变化时，在这里添加一条轻量记录：

```text
### Decision: <topic>
Status: Accepted | Superseded
Date: YYYY-MM-DD

Context:
...

Decision:
...

Reason:
...

Trade-off:
...
```

### Decision: 在已接受架构上继续，不重新设计
Status: Accepted
Date: 2026-09-20

Context:
新会话从 Desktop `AGENTS.md` 与既有连续性文件恢复。V0 Editor 集成已经验证。

Decision:
保持 V0–V4 方向，以及 Validation Core / Editor UI / Commandlet / CI 边界不变。不重新设计项目。

Reason:
没有新的证据要求换架构。下一步是 V1 真实数据流，从 Asset vs Level Actor 开始，而不是新框架。

Trade-off:
V0 插件 Module 仍保持已交付的 `Type: Runtime`，直到出现真实的加载类型问题。V1 仍从一个真实 Rule 开始，而不是 Rule Registry。

---

# English Version

> Decision guardrails and design rationale. This file explains **why** and **where**, not the full implementation.
> Last reviewed: 2026-09-20. Guardrails unchanged; continue from the accepted architecture without redesign.

## 1. Architectural Intent

The project is a small Unreal Editor production-tool prototype. Its architecture should expose production-tool engineering judgment without turning the portfolio demo into a framework project.

Primary flow:

```text
UE Asset / Project Data
        ↓
   Rule Engine
        ↓
   Issue Model
        ↓
Validation Result
        ↓
 Editor / Report
        ↓
 Commandlet
        ↓
 Python / CI Gate
```

The layers should grow only when a real use case requires them.

## 2. Core Boundaries

### Validation Core

Owns the actual validation semantics:

- rule evaluation
- issue/result creation
- deterministic validation behavior

Should not depend on Slate UI.

### Editor UI

Owns presentation and interaction:

- trigger validation
- show results
- locate/select relevant assets where practical
- later expose safe fixes

The UI should consume validation results rather than contain validation logic.

### Asset Discovery

Use Unreal's native Asset Registry for project-content discovery rather than inventing a parallel indexing system.

Important conceptual distinction:

- Content Browser asset = project/package data object
- Level Actor = object instance in a world/level

The first V1 implementation focuses on assets.

### Automation Boundary

Later, the same validation core should be callable from an Editor Commandlet.

The Commandlet is preferred over a separate executable because Unreal is already the required execution environment.

### CI Boundary

Later:

```text
Python wrapper
    ↓
UE Commandlet
    ↓
JSON / exit code
    ↓
GitHub Actions
```

Python is orchestration/glue, not the home of business rules.

## 3. Rule Engine Decisions

### Use UE-native reflection/lightweight data structures

Use appropriate combinations of `USTRUCT`, `UCLASS`, `UPROPERTY`, reflection and ordinary C++ types where they genuinely help editor/configuration integration.

### Do not build a custom DSL

Reason:

- adds implementation cost
- adds parsing/testing complexity
- provides little value for this scale
- distracts from real tool/pipeline engineering

### Do not build a registry framework prematurely

Start with the smallest representation that can execute one real rule. Introduce rule registration/configuration only when a second or third rule proves that an abstraction is useful.

## 4. Validation Result / Issue Model

A rule should conceptually answer:

```text
Input asset
   ↓
Evaluate
   ↓
Pass / Fail / Warning
   ↓
Issue information
```

A result should be useful to both humans and later automation. At minimum, it should identify:

- the asset
- rule/check identity
- severity or pass/fail state
- human-readable message

Additional metadata can be added when a real use case requires it.

## 5. Observability

The tool itself should eventually be observable.

Initial observability:

- total assets checked
- issue counts
- overall scan duration
- per-rule/per-run timing when useful

Persistence may later use SQLite.

Do not build a visualization system. A table and queryable history are enough for the portfolio objective.

## 6. Incremental Validation Direction

A save-time or editor-triggered validation system should not start by rescanning the whole project on every change.

Preferred evolution:

```text
Current level / selected scope
        ↓
Dirty / changed assets
        ↓
Targeted validation
```

This demonstrates performance and systems thinking without requiring a large production-scale implementation.

## 7. Safe Auto-Fix

Auto-fix is secondary to diagnosis.

Only add fixes that are:

- deterministic
- reversible
- low-risk
- easy to explain

Use UE transaction/undo support (`FScopedTransaction` where appropriate) so a fix can be demonstrated safely in the Editor.

## 8. Configuration Strategy

The long-term design should separate **policy** from **execution**.

Example concept:

```text
Policy: PC
Policy: Mobile
Policy: Console
        ↓
Same validation engine
        ↓
Different thresholds / requirements
```

This cross-platform configuration is an engineering demonstration, not a requirement to build multiple platform pipelines.

## 9. DCC / Blender Integration

DCC integration is a later extension, not an MVP dependency.

Possible later chain:

```text
DCC
 ↓
Python precheck
 ↓
Export
 ↓
UE validation
 ↓
CI
```

The UE validation core must remain useful without Blender.

## 10. AI Boundary

AI is optional and must remain an enhancement layer rather than the project foundation.

A possible future interface is:

```text
Natural-language request
        ↓
AI
        ↓
Tool API
        ↓
Rule / Validation system
```

Do not build RAG/agents merely to claim AI capability.

## 11. Dependency Direction

Preferred dependency direction:

```text
UI
 ↓
Validation Core
 ↓
UE data / Asset Registry
```

Automation should call the same core rather than duplicate business logic:

```text
Editor UI ─┐
           ├→ Validation Core
Commandlet ┘
```

Python/CI stays outside the core.

## 12. Explicit Non-Goals

Do not add these unless new evidence justifies them:

- custom DSL
- separate validation executable
- large plugin framework
- complex factory/strategy/registry layers before needed
- dashboard/charting platform
- broad AI/RAG architecture
- large showcase game scene
- large set of speculative rules

## 13. Decision Record Style

When a significant design choice changes, record a small entry here:

```text
### Decision: <topic>
Status: Accepted | Superseded
Date: YYYY-MM-DD

Context:
...

Decision:
...

Reason:
...

Trade-off:
...
```

### Decision: Continue from accepted architecture
Status: Accepted
Date: 2026-09-20

Context:
New session started from Desktop `AGENTS.md` and existing continuity files. V0 Editor integration is already verified.

Decision:
Keep the V0–V4 direction and Validation Core / Editor UI / Commandlet / CI boundaries unchanged. Do not redesign the project.

Reason:
No new evidence requires a different architecture. The next work is V1’s real data path, starting with Asset vs Level Actor, not a new framework.

Trade-off:
The V0 plugin module remains `Type: Runtime` as shipped until a real loading-type problem appears. V1 still starts with one real rule rather than a rule registry.
