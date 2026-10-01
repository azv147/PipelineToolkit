# UE5 Production Pipeline Toolkit

> Unreal Engine 5.8.2 · Windows · C++ · Editor Tool / Asset Validation / Pipeline Automation

---

# 中文版

## 项目简介

`PipelineToolkit` 是一个面向游戏行业 Tools / Systems / Pipeline Engineering 岗位的 Unreal Engine Editor 工具作品集项目。

它不是商业级通用插件，而是一个小型、可解释、可复现的生产流程原型。

核心原则：

> 30 秒讲清价值；30 分钟讲清实现。

## 当前状态

```text
V0  Editor Plugin + Slate Toolkit        ✅
V1  Asset Registry + Validation Core     ✅
V2  Production Validation Workflow       ✅
V3  Headless Validation & Automation     ▶ Active
```

V2 已完成并同步 GitHub。

当前已经具备：

- Unreal Editor Plugin / Module lifecycle
- Dockable Slate Tool
- Asset Registry recursive scan
- `FAssetData`-based validation
- Blueprint naming rule
- `FValidationResult`
- `FValidationRun` summary
- configurable Scan Scope / Naming Prefix
- deterministic Rename Auto-Fix
- explicit `Revert Last Fix`
- JSON Validation Report

当前 V3 第一任务不是直接实现 Commandlet，而是先把 Validation Execution 从 Slate 中抽离：

```text
Editor UI
   ↓
FValidationConfig
   ↓
FValidationRunner
   ↓
FValidationRun
```

验证这一共享边界后，再加入：

```text
Editor UI ─────┐
               ├── FValidationRunner
Commandlet ────┘
                       ↓
                  JSON + Exit Code
```

## 设计原则

- 优先使用 UE 原生机制
- 非必要不抽象
- 当前不建立 `IRule / RuleManager / RuleFactory`
- UI 不承载 Validation 语义
- JSON 不复制 Validation Model
- Auto-Fix / Revert 保持 Editor-only Workflow
- 第一版 Commandlet 只 Validation，不自动修改 Asset
- Python / CI 属于后续 orchestration 层

## 文档

- `PROJECT_STATE.md` — 当前真实状态
- `ARCHITECTURE.md` — 架构边界与重要设计决策
- `SESSION_LOG.md` — 按时间记录开发历史
- `AGENTS.md` — Codex / Agent 开发规则与当前交接任务

项目长期文档统一维护中文 + English 双语版本。

## License

项目原创代码使用 MIT License。

第三方软件及许可证边界见：

- `LICENSE`
- `THIRD_PARTY_NOTICES.md`

---

# English Version

## Overview

`PipelineToolkit` is an Unreal Engine Editor-tool portfolio project aimed at game-industry Tools / Systems / Pipeline Engineering roles.

It is intentionally a small, explainable, reproducible production-workflow prototype rather than a commercial general-purpose plugin.

Portfolio principle:

> Explain the value in 30 seconds; explain the implementation in 30 minutes.

## Current Status

```text
V0  Editor Plugin + Slate Toolkit        ✅
V1  Asset Registry + Validation Core     ✅
V2  Production Validation Workflow       ✅
V3  Headless Validation & Automation     ▶ Active
```

V2 is complete and synced to GitHub.

Implemented:

- Unreal Editor Plugin / Module lifecycle
- dockable Slate tool
- recursive Asset Registry scan
- `FAssetData`-based validation
- Blueprint naming rule
- `FValidationResult`
- `FValidationRun` summary
- configurable Scan Scope / Naming Prefix
- deterministic Rename Auto-Fix
- explicit `Revert Last Fix`
- JSON Validation Report

The first V3 task is not Commandlet implementation. It first separates reusable validation execution from Slate:

```text
Editor UI
   ↓
FValidationConfig
   ↓
FValidationRunner
   ↓
FValidationRun
```

After that boundary is verified:

```text
Editor UI ─────┐
               ├── FValidationRunner
Commandlet ────┘
                       ↓
                  JSON + Exit Code
```

## Engineering Principles

- prefer UE-native mechanisms
- abstract only when evidence justifies it
- do not add `IRule / RuleManager / RuleFactory` yet
- keep validation semantics out of Slate
- do not duplicate the validation model for JSON
- keep Auto-Fix/Revert as an Editor-only workflow
- first Commandlet validates only; it does not mutate assets
- Python / CI remain later orchestration layers

## Documentation

- `PROJECT_STATE.md` — current verified truth
- `ARCHITECTURE.md` — architectural boundaries and decisions
- `SESSION_LOG.md` — chronological development history
- `AGENTS.md` — Codex/agent development rules and current handoff

All long-lived project documents are maintained in both Chinese and English.

## License

Original project code is licensed under the MIT License.

See:

- `LICENSE`
- `THIRD_PARTY_NOTICES.md`

for project and third-party licensing boundaries.
