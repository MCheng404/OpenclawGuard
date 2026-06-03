# OpenclawGuard UI 重构迁移报告（PyQt-Fluent-Widgets）

## 1. 目标与范围

- 原项目：`D:\Open\OpenclawGuard`
- 重构目标：基于 `PyQt6 + PyQt-Fluent-Widgets` 重构 UI 层，保留并逐步对接原有业务逻辑。
- 当前成果目录：`D:\Open\OpenclawGuard\pyui`

## 2. 页面映射

- 仪表盘（Dashboard）
  - 原：QWidget/QML 概览卡片 + 快捷按钮
  - 新：Fluent Card + 操作按钮（启动/停止网关、检查更新、刷新状态）

- 网关管理（Gateway）
  - 原：路径、端口、启停、重启
  - 新：
    - 控制入口保留（启停/重启/刷新）
    - 配置入口统一迁移到“设置页”

- 进程守护（Guard）
  - 原：表格 + 增删 + 启用控制
  - 新：
    - 新增守护项
    - 删除选中守护项
    - 启用/禁用切换
    - 表格可编辑并保存

- 更新管理（Updates）
  - 原：检查更新 + 表格展示 + 更新动作
  - 新：
    - dry-run 结果表格化展示（当前版本/通道/目标规格/结果）
    - 执行更新按钮 + 二次确认弹窗

- 环境检测（Env）
  - 原：检测工具链版本
  - 新：检测 Node/npm/Python/Git/Openclaw/qmake 并展示状态

- 设置（Settings）
  - 原：分散配置项
  - 新：统一配置入口：CLI 路径、网关端口、GitHub Token

## 3. 已实现对接点

### 3.1 Openclaw CLI 对接

- 网关：
  - `openclaw gateway status`
  - `openclaw gateway start`
  - `openclaw gateway stop`
  - `openclaw gateway restart`

- 更新：
  - `openclaw update --dry-run`
  - `openclaw update [--channel stable|beta]`

### 3.2 配置持久化

- 使用 ini：`D:\Open\OpenclawGuard\build-release\OpenclawGuard.ini`
- 已读写字段：
  - `gateway/path`
  - `gateway/port`
  - `github/token`
  - `guardList\N\name`
  - `guardList\N\exePath`
  - `guardList\N\enabled`

### 3.3 守护状态检测

- 通过 `tasklist /FO CSV /NH` 判断守护进程是否运行。

## 4. 当前差异与未完成项

1) Guard “运行状态”当前由外部进程枚举判断，尚未完全复用 C++ `ProcessGuard` 的“高负载延迟拉起策略”。
2) Env 页面当前实现“本机命令探测”，尚未接入原 C++ `EnvironmentManager` 的“线上 latest 版本对比”。
3) 更新页当前为 CLI 输出解析，不是结构化 API 回填。
4) 未接入原项目托盘管理（TrayManager）与主题细节（完整 token 级样式映射）。

## 5. 可运行与验证

- 语法验证：`python -m py_compile` 已通过。
- 运行入口：`D:\Open\OpenclawGuard\pyui\main.py`

## 6. 建议上线步骤

1. 在测试机跑一轮完整流程：网关启停/重启、守护增删改、更新 dry-run/执行、环境检测。
2. 导出一次配置快照（现有 ini）。
3. 若要替换主 UI，先并行灰度（保留原 Qt C++ UI 作为回退）。
4. 收集 1-2 天使用反馈后再做默认入口切换。

## 7. 结论

当前阶段已完成“可用 UI + 核心操作可用 + 配置可持久化”的重构目标。后续建议聚焦：
- Guard 策略与 C++ 行为一致化
- Env 最新版本联网对比
- 主题与交互细节抛光
