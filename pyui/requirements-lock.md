# OpenclawGuard PyUI 依赖锁定说明

## 当前最小依赖

来自 `requirements.txt`：

- PyQt6>=6.7
- PyQt-Fluent-Widgets>=1.5.5

## 生成锁定文件（推荐在测试机执行）

```bash
cd D:\Open\OpenclawGuard\pyui
python -m venv .venv
.venv\Scripts\activate
python -m pip install -r requirements.txt
python -m pip freeze > requirements-lock.txt
```

> 说明：`requirements-lock.txt` 会包含当前机器解析出的完整依赖树版本，适合用于回归和一致性部署。

## 使用锁定文件安装

```bash
cd D:\Open\OpenclawGuard\pyui
python -m venv .venv
.venv\Scripts\activate
python -m pip install -r requirements-lock.txt
```
