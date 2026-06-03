# 更新日志

## v1.0.0 (2026-06-03)

首个正式版本。

### 功能
- **仪表盘**：网关状态 + 环境概览 + 快捷操作
- **网关管理**：内置 `openclaw gateway start/stop/restart`，无需外部路径配置
- **进程守护**：CPU/内存智能阈值 + 崩溃自动拉起
- **更新管理**：通过 npm 执行 Openclaw 更新，支持 stable/beta 通道
- **环境检测**：8 种工具链扫描 + 对比最新版本 + 一键更新
- **系统托盘**：后台常驻 + 开机自启

### 界面
- Windows 11 Mica / Acrylic 毛玻璃特效
- 深色 / 浅色 / 跟随系统主题
- 现代化渐变按钮 + 圆角卡片
- DPI 缩放自适应
- GPU 加速渲染 (OpenGL)

### 技术
- Qt 6.11.1 + C++17
- CMake + Ninja 构建
- Clang 17 (LLVM MinGW) 编译
- 异步 QProcess（环境更新不阻塞 UI）
- ANSI 转义码清洗（CLI 输出兼容）
