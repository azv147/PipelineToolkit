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

V3.0-A 已完成：Validation Execution 已从 Slate 中抽离，Editor 构建与真实 Fixture 原生 Automation 测试通过；用户于 2026-10-02 确认人工功能验收和代码评审完成：

```text
Editor UI
   ↓
FValidationConfig
   ↓
FValidationRunner
   ↓
FValidationRun
```

V3.0-B Commandlet 已完成：此前实际 CLI 验证通过，用户于 2026-10-03 确认人工验收完毕。复用现有 JSON schema，只验证、不修改资产：

```text
Editor UI ─────┐
               ├── FValidationRunner
Commandlet ────┘
                       ↓
                  JSON + Exit Code
```

下一节点为 V3.1 本地 Python 编排（准备中，尚未实现）：调用 Commandlet、读取 JSON 摘要并保留退出码；CI 后续独立接入。开发合同与验收标准见 `PROJECT_STATE.md`。

## Commandlet 使用

```powershell
& 'D:/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' 'D:/Epic Games/UEProjects/PipelineToolkit/PipelineToolkit.uproject' -run=ValidateAssets -unattended -NullRHI
```

可选参数：`-Scope=/Game/ValidationTest`、`-BlueprintPrefix=BP_`、`-ReportPath="Saved/ValidationReports/ValidationReport.json"`。默认值如上；相对报告路径以项目目录为基准，含空格值需引号。

退出码：`0` 无规则失败且报告成功，`1` 有规则失败且报告成功，`2` 参数 / 执行 / 写入失败。默认 Fixture 返回 1 并输出完整 JSON；Scope `/Game/ValidationTest/Characters` 返回 0。不存在的 Scope 沿用 Runner 的空结果语义并返回 0。退出码 2 时不要将旧报告视为本次结果。

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

V3.0-A is complete: reusable validation execution is separated from Slate, the Editor build and native Automation test passed, and the user confirmed manual functional acceptance and code review on 2026-10-02:

```text
Editor UI
   ↓
FValidationConfig
   ↓
FValidationRunner
   ↓
FValidationRun
```

V3.0-B Commandlet is complete: actual CLI checks passed previously, and the user confirmed manual acceptance on 2026-10-03. It reuses the existing JSON schema and validates without asset mutation:

```text
Editor UI ─────┐
               ├── FValidationRunner
Commandlet ────┘
                       ↓
                  JSON + Exit Code
```

Next is V3.1 local Python orchestration (preparation, not implemented): invoke the Commandlet, summarize JSON, and preserve exit codes. CI follows separately. See `PROJECT_STATE.md` for the development contract and acceptance criteria.

## Commandlet Usage

```powershell
& 'D:/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' 'D:/Epic Games/UEProjects/PipelineToolkit/PipelineToolkit.uproject' -run=ValidateAssets -unattended -NullRHI
```

Optional parameters: `-Scope=/Game/ValidationTest`, `-BlueprintPrefix=BP_`, and `-ReportPath="Saved/ValidationReports/ValidationReport.json"`. These are also the defaults. Relative report paths start from the project directory; quote values containing spaces.

Exit codes: `0` means no rule failures and report success; `1` means rule failures and report success; `2` means argument / execution / writing failure. The default fixture returns 1 with a complete JSON report; Scope `/Game/ValidationTest/Characters` returns 0. A nonexistent Scope preserves the Runner's empty-result semantics and returns 0. On exit code 2, do not treat an old report as the current result.

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
