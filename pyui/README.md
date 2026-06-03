# OpenclawGuard Fluent UI（PyQt-Fluent-Widgets）

基于 `PyQt6 + PyQt-Fluent-Widgets` 的 UI 重构骨架，目标是替换现有 QWidget/QML 的表现层，先保留后端逻辑。

## 目录

- `main.py`：FluentWindow 主入口
- `services/controller.py`：UI 控制器（当前为示例逻辑）
- `models.py`：页面共享状态
- `pages/`：各页面 UI 组件

## 运行

```bash
cd D:\Open\OpenclawGuard\pyui
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
python main.py
```

## 下一步（建议）

1. 用 `QProcess` 对接现有 `openclaw` 命令调用逻辑（替换 controller 示例方法）
2. 将 `src/gatewaymanager.* / updatemanager.* / processguard.* / envmanager.*` 抽象成 JSON-RPC/CLI 适配层
3. 补充 UI 一致性清单（配色、间距、状态文案）并做逐页对照
