<div align="center">

# 进程管理控制台 / Process Manager

> OpenClaw 网关守护与系统环境管理桌面工具
>
> OpenClaw gateway daemon & system environment management desktop tool

![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-blue?style=flat-square)
![Qt](https://img.shields.io/badge/Qt-6.11.1-41cd52?style=flat-square&logo=qt)
![C++](https://img.shields.io/badge/C%2B%2B-17-00599c?style=flat-square&logo=cplusplus)
![License](https://img.shields.io/badge/license-CC%20BY--NC%204.0-orange?style=flat-square)

**版本 / Version: v1.6.2** | [更新日志 / Changelog](CHANGELOG.md)

[**English**](#features) | [**中文**](#功能特性)

</div>

---

## Features

### Dashboard
- Real-time gateway status monitoring (online/offline + port + pulse animation)
- Environment overview (Node.js / Python / Git / .NET / PowerShell / CMake / Java)
- Recent events list (card-based rendering, adaptive height)
- Quick actions: one-click gateway start/restart, environment scan

### Gateway Management
- Built-in `openclaw gateway start/stop/restart` commands
- TCP port liveness detection
- Crash auto-restart + CPU/memory intelligent threshold guard

### Update Management
- Update via `npm install -g openclaw@latest` (`--yes` auto-approves scripts)
- Stable / beta channel switching
- GitHub Releases list browsing & download
- Status overview: 4 mini cards (version/channel/install method/available updates)

### Environment Detection
- Auto-scan 8 development toolchains
- Compare current vs. latest versions
- Async updates (winget / npm), non-blocking UI
- Real-time progress in status bar

### System Tray
- Background resident, double-click to show main window
- Tray menu shows real-time gateway status (online/offline + port)
- One-click gateway restart

### UI
- Windows 11 Mica / Acrylic blur effects
- Dark / Light / System theme (dynamic switching, no restart)
- Global color temperature adjustment (3000K–7000K, palette-level tinting)
- Custom-painted ModernSlider (gradient track + circular handle + hover animation + custom tooltip)
- Gaussian blur shadow cards + top gradient decoration bar
- Sidebar sliding indicator (smooth transition on page switch)
- Page card staggered entry animation (OutQuart easing + decremental delay)
- Navigation button hover physical feedback (text shift + background gradient)
- DPI scaling adaptive (PassThrough policy)
- GPU-accelerated rendering (OpenGL + 2x MSAA)

### Settings
- GitHub Token management (show/hide toggle)
- Smart guard threshold configuration (CPU / memory sliders)
- Color temperature adjustment (warm → neutral → cool)
- Auto-start management
- GitHub project link

## Tech Stack

| Layer | Technology |
|---|---|
| UI Framework | Qt 6.11.1 Widgets |
| Language | C++17 |
| Build | CMake + Ninja |
| Compiler | Clang 17 (LLVM MinGW) |
| DWM Effects | Windows DWM API (Mica / Acrylic) |
| GPU Rendering | OpenGL (QSurfaceFormat) |
| Shadows | QGraphicsDropShadowEffect (Gaussian blur) |

## Build

### Prerequisites

- **Qt** 6.8+ (recommended 6.11.1)
- **Compiler** LLVM MinGW 17+ or MSVC 2022
- **CMake** 3.20+
- **Ninja** 1.10+

### Build Steps

```bash
# Debug
mkdir build-debug && cd build-debug
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/llvm-mingw_64" -DCMAKE_CXX_COMPILER="D:/Qt/Tools/llvm-mingw1706_64/bin/clang++.exe"
ninja

# Release
mkdir build-release && cd build-release
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/llvm-mingw_64" -DCMAKE_CXX_COMPILER="D:/Qt/Tools/llvm-mingw1706_64/bin/clang++.exe"
ninja
```

### Deploy Runtime DLLs

```bash
cd build-release
windeployqt6.exe --no-translations OpenclawGuard.exe
```

## Project Structure

```
OpenclawGuard/
├── src/
│   ├── main.cpp              # Entry + OpenGL init + DPI setup
│   ├── mainwindow.cpp/h      # Main window (dashboard/gateway/process/update/env/settings)
│   ├── gatewaymanager.cpp/h  # Gateway management (openclaw gateway commands)
│   ├── processguard.cpp/h    # Process guard (auto-restart + threshold detection)
│   ├── updatemanager.cpp/h   # Update management (npm + GitHub API)
│   ├── envmanager.cpp/h      # Environment detection (async updates)
│   ├── portmonitor.cpp/h     # TCP port monitoring
│   ├── theme.cpp/h           # Theme system + DWM effects
│   ├── mica_helper.cpp       # Windows Mica/Acrylic implementation
│   ├── traymanager.cpp/h     # System tray (real-time gateway status)
│   ├── settings.cpp/h        # Configuration persistence (QSettings)
│   ├── config.h              # Constants
│   ├── toggle_switch.cpp/h   # Custom toggle switch widget
│   └── modernslider.cpp/h    # Custom slider (gradient track + color temp rendering)
├── resources/
│   └── icons/                # SVG icons (Lucide)
├── docs/
│   ├── screenshot-dark.png   # Dark theme screenshot
│   └── screenshot-light.png  # Light theme screenshot
├── CMakeLists.txt
├── CHANGELOG.md
├── .gitignore
├── LICENSE
└── README.md
```

## Screenshots

<div align="center">

| Dark Theme | Light Theme |
|:---:|:---:|
| ![dark](docs/screenshot-dark.png) | ![light](docs/screenshot-light.png) |

</div>

## License

[CC BY-NC 4.0](LICENSE) — No commercial use allowed.

---

<div align="center">

**Process Manager** — OpenClaw management tool 🛠️

</div>

---

## 功能特性

### 仪表盘
- 网关状态实时监控（在线/离线 + 端口 + 脉冲动画）
- 环境概览（Node.js / Python / Git / .NET / PowerShell / CMake / Java）
- 最近事件列表（卡片式渲染，自适应高度）
- 快捷操作：一键启动/重启网关、环境检测

### 网关管理
- 内置 `openclaw gateway start/stop/restart` 命令
- TCP 端口存活检测
- 崩溃自动拉起 + CPU/内存智能阈值守护

### 更新管理
- 通过 `npm install -g openclaw@latest` 执行更新（`--yes` 自动批准脚本）
- 支持 stable / beta 通道切换
- GitHub Releases 列表浏览与下载
- 状态概览横排 4 格迷你卡片（版本/通道/安装方式/可用更新）

### 环境检测
- 自动扫描 8 种开发工具链
- 对比当前版本与最新版本
- 异步更新（winget / npm），不阻塞 UI
- 实时输出进度到状态栏

### 系统托盘
- 后台常驻，双击显示主窗口
- 托盘菜单实时显示网关状态（在线/离线 + 端口）
- 一键重启网关

### 界面
- Windows 11 Mica / Acrylic 毛玻璃特效
- 深色 / 浅色 / 跟随系统主题（动态切换，无需重启）
- 全局色温调节（3000K–7000K，调色板级染色）
- 自定义绘制 ModernSlider（渐变轨道 + 圆形 handle + 悬浮动画 + 自绘 tooltip）
- 高斯模糊阴影卡片 + 顶部渐变装饰条
- 侧边栏滑动指示器（页面切换时平滑过渡）
- 页面卡片交错入场动画（OutQuart 缓动 + 递减延迟）
- 导航按钮 hover 物理反馈（文字位移 + 背景渐变）
- DPI 缩放自适应（PassThrough 策略）
- GPU 加速渲染（OpenGL + 2x MSAA）

### 设置
- GitHub Token 管理（显示/隐藏切换）
- 智能拉起阈值配置（CPU / 内存滑动条）
- 色温调节（暖黄 → 中性 → 冷蓝）
- 开机自启管理
- GitHub 项目链接

## 技术栈

| 层 | 技术 |
|---|---|
| UI 框架 | Qt 6.11.1 Widgets |
| 语言 | C++17 |
| 构建 | CMake + Ninja |
| 编译器 | Clang 17 (LLVM MinGW) |
| DWM 特效 | Windows DWM API (Mica / Acrylic) |
| GPU 渲染 | OpenGL (QSurfaceFormat) |
| 阴影 | QGraphicsDropShadowEffect (高斯模糊) |

## 构建

### 环境要求

- **Qt** 6.8+ (推荐 6.11.1)
- **编译器** LLVM MinGW 17+ 或 MSVC 2022
- **CMake** 3.20+
- **Ninja** 1.10+

### 编译步骤

```bash
# Debug
mkdir build-debug && cd build-debug
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/llvm-mingw_64" -DCMAKE_CXX_COMPILER="D:/Qt/Tools/llvm-mingw1706_64/bin/clang++.exe"
ninja

# Release
mkdir build-release && cd build-release
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/llvm-mingw_64" -DCMAKE_CXX_COMPILER="D:/Qt/Tools/llvm-mingw1706_64/bin/clang++.exe"
ninja
```

### 部署运行时 DLL

```bash
cd build-release
windeployqt6.exe --no-translations OpenclawGuard.exe
```

## 项目结构

```
OpenclawGuard/
├── src/
│   ├── main.cpp              # 入口 + OpenGL 初始化 + DPI 设置
│   ├── mainwindow.cpp/h      # 主窗口（仪表盘/网关/进程/更新/环境/设置）
│   ├── gatewaymanager.cpp/h  # 网关管理（openclaw gateway 命令）
│   ├── processguard.cpp/h    # 进程守护（自动拉起 + 阈值检测）
│   ├── updatemanager.cpp/h   # 更新管理（npm + GitHub API）
│   ├── envmanager.cpp/h      # 环境检测（异步更新）
│   ├── portmonitor.cpp/h     # TCP 端口监控
│   ├── theme.cpp/h           # 主题系统 + DWM 特效
│   ├── mica_helper.cpp       # Windows Mica/Acrylic 实现
│   ├── traymanager.cpp/h     # 系统托盘（实时网关状态）
│   ├── settings.cpp/h        # 配置持久化（QSettings）
│   ├── config.h              # 常量定义
│   ├── toggle_switch.cpp/h   # 自定义开关控件
│   └── modernslider.cpp/h    # 自定义滑动条（渐变轨道 + 色温渲染）
├── resources/
│   └── icons/                # SVG 图标 (Lucide)
├── docs/
│   ├── screenshot-dark.png   # 深色主题截图
│   └── screenshot-light.png  # 浅色主题截图
├── CMakeLists.txt
├── CHANGELOG.md
├── .gitignore
├── LICENSE
└── README.md
```

## 截图

<div align="center">

| 深色主题 | 浅色主题 |
|:---:|:---:|
| ![dark](docs/screenshot-dark.png) | ![light](docs/screenshot-light.png) |

</div>

## 许可证

[CC BY-NC 4.0](LICENSE) — 禁止商业用途

---

<div align="center">

**进程管理控制台** — 旺老板的 OpenClaw 管理利器 🛠️

</div>
