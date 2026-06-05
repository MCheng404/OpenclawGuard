# 更新日志

## v1.6.0 (2026-06-06)

### 新增
- 全局色温调节（3000K–7000K，Tanner Helland 算法实时 RGB 映射）
- 色温叠加层渲染（半透明覆盖，色温偏移越大效果越明显）
- ModernSlider 自定义绘制控件（渐变轨道 + 圆形 handle + 悬浮动画 + 自绘圆角 tooltip）
- 设置页滑动条支持鼠标拖动 + 滚轮调整

### 优化
- 主题切换改为动态应用，无需重启程序
- 滑动条 tooltip 自绘，避免 Windows 原生 tooltip 直角问题
- 进程表格移除“启用”列标题

### 修复
- 修复启动时因 loadSettings 触发 onThemeChanged 导致无限重启
- 修复 QSharedMemory 竞态弹出“程序已在运行中”（改用 Windows Named Mutex）
- 修复滑动条鼠标无法拖动（重写 mousePressEvent/mouseMoveEvent）
- 版本号升级至 v1.6.0

## v1.5.0 (2026-06-05)

### 新增
- 侧边栏滑动指示器（页面切换时蓝色条平滑滑动，320ms OutQuart）
- 页面卡片交错入场动画（递减延迟 + 22px 上移 + OutQuart 缓动）
- 统计卡片顶部渐变装饰条（蓝→紫，亚克力质感）
- ShadowCard 高斯模糊阴影（QGraphicsDropShadowEffect，blurRadius=28）
- 卡片顶部高光线（亚克力质感渐变）

### 优化
- 导航按钮 hover 文字微右移 2px（物理反馈）
- 主按钮 hover/pressed padding 微移（按下感）
- 仪表盘卡片 hover 增强边框 + 背景色变化
- 活动列表 hover 文字颜色变化
- 滚动条更纤细（10px → 8px）
- 缓动曲线全面升级为 OutQuart（更有「着陆感」）

### 清理
- 移除废弃的 pyui/（Python 原型）
- 移除废弃的 qml/（QML 原型）
- 移除废弃的 winui/（WinUI 3 原型）
- 移除 docs/superpowers/（过期设计文档）

## v1.2.0 (2026-06-03)

### 优化
- 现代化按钮重设计

## v1.1.3 (2026-06-03)

### 优化
- 主题感知委托、键盘快捷键

## v1.0.3 (2026-06-03)

### 修复
- 修复卡片阴影直角问题（改用偏移圆角矩形，天然跟随圆角）
- 优化阴影为最小存在感
- 优化 EXE 启动速度（环境检测延迟异步 + 检测超时 3s→1.5s + MSAA 4x→2x）
- 活动列表自适应高度（delegate sizeHint 按文字计算）

## v1.0.2 (2026-06-03)

### 修复
- 修复仪表盘卡片阴影错位和渲染不全（改用自绘 ShadowCard，不受 widget 边界裁剪）
- 补全 Mica/Acrylic 毛玻璃背景（主窗口调色板透明化）
- 侧边栏透明度优化（减少与标题栏的色差）

### 优化
- 更新页：状态概览横排迷你卡片 + 紧凑操作区 + 图标标题
- 设置页：GitHub 项目链接 + Token 显示/隐藏 + 分区图标
- 卡片 45° 阴影（自绘多层高斯模拟）
- 迷你状态卡片文字不再被截断

## v1.0.1 (2026-06-03)

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
