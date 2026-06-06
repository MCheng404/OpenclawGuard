#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QSlider>
#include "modernslider.h"
#include <QPushButton>
#include <QCheckBox>
#include "toggle_switch.h"
#include <QComboBox>
#include <QTableWidget>
#include <QProgressBar>
#include <QTimer>
#include <QStackedWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QFrame>
#include <QSvgRenderer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QResizeEvent>
#include <QTextEdit>
#include "gatewaymanager.h"
#include "processguard.h"
#include "updatemanager.h"
#include "envmanager.h"
#include "traymanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    GatewayManager* gateway() const { return m_gateway; }
    ProcessGuard*   guard()   const { return m_guard; }

signals:
    void gatewayRestartRequested();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onNavigate(int pageIndex);
    void onToggleTheme();

    // 网关
    void onStartStopGateway();
    void onRestartGateway();
    void onPortChanged(int port);
    void onGatewayOnlineChanged(bool online);
    void onGatewayCrashed();

    // 守护
    void onAddGuardItem();
    void onPickFromProcess();
    void onRemoveGuardItem();
    void onGuardItemChanged();
    void onGuardStatusChanged(const QString &name, bool running);

    // 更新
    void onFetchUpdates();
    void onInstallUpdate(const UpdateInfo &info);
    void onUpdateChannelChanged(int idx);

    // 环境
    void onEnvDetect();
    void onEnvUpdate(const QString &name);
    void onEnvDetectionFinished();
    void onEnvUpdateFinished(const QString &name, bool success, const QString &msg);
    void onLatestVersionChecked(const QString &name, const QString &latest, bool newer);

    // 主题
    void onThemeChanged(int idx);

private:
    void showEvent(QShowEvent *event) override;
    void setupUI();
    void setupSidebar(QHBoxLayout *mainLayout);
    void setupPages();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void refreshGuardTable();
    void refreshUpdateTable();
    void applyTheme();
    void updateSidebarContext(int pageIndex);
    void animatePageCards(QWidget *page);
    void updateResponsiveLayout();
    void updateStatusBar(const QString &msg);
    void appendLog(const QString &msg);
    void updateGuardSelectionDetails();

    // 侧边栏
    QWidget     *m_sidebar = nullptr;
    QButtonGroup *m_navGroup = nullptr;
    QFrame      *m_navIndicator = nullptr;
    QLabel      *m_versionLabel = nullptr;
    QPushButton *m_themeBtn = nullptr;
    QLabel      *m_logoLabel = nullptr;
    QLabel      *m_sidebarTagline = nullptr;
    QLabel      *m_sidebarPageLabel = nullptr;
    QLabel      *m_sidebarPageHint = nullptr;
    QLabel      *m_sidebarThemeLabel = nullptr;
    QFrame      *m_sidebarBrandCard = nullptr;
    QFrame      *m_sidebarContextCard = nullptr;
    bool         m_sidebarCompact = false;
    bool         m_loading = false;  // loadSettings 期间阻止 onThemeChanged

    // 动画
    QPropertyAnimation       *m_pulseAnim = nullptr;
    QGraphicsOpacityEffect   *m_pulseOpacity = nullptr;
    QPropertyAnimation       *m_dashPulseAnim = nullptr;
    QGraphicsOpacityEffect   *m_dashPulseOpacity = nullptr;
    QTimer                   *m_updatePulseTimer = nullptr;
    int                       m_updatePulseCounter = 0;

    // 页面容器
    QStackedWidget *m_pages = nullptr;

    // 主题 (保留兼容)
    QComboBox    *m_themeCombo = nullptr;

    // 开机自启
    ToggleSwitch *m_autoStartCheck = nullptr;
    ModernSlider *m_cpuSlider = nullptr;
    QLabel       *m_cpuValueLabel = nullptr;
    ModernSlider *m_memSlider = nullptr;
    QLabel       *m_memValueLabel = nullptr;

    // 色温
    ModernSlider *m_colorTempSlider = nullptr;
    QLabel       *m_colorTempValueLabel = nullptr;
    QWidget      *m_tempCard = nullptr;   // 色温卡片（浅色主题才显示）

    // UI 自定义
    ModernSlider *m_cardOpacitySlider = nullptr;
    QLabel       *m_cardOpacityLabel = nullptr;
    ModernSlider *m_cardRadiusSlider = nullptr;
    QLabel       *m_cardRadiusLabel = nullptr;
    ModernSlider *m_shadowSlider = nullptr;
    QLabel       *m_shadowLabel = nullptr;
    QWidget      *m_uiCustomCard = nullptr;
    QVector<QFrame*> m_allShadowCards;  // 所有 ShadowCard 引用

    // Liquid Glass
    ToggleSwitch *m_glassToggle = nullptr;
    ModernSlider *m_glassBlurSlider = nullptr;
    QLabel       *m_glassBlurLabel = nullptr;
    ModernSlider *m_glassTintSlider = nullptr;
    QLabel       *m_glassTintLabel = nullptr;
    QWidget      *m_glassSlidersContainer = nullptr;

    // 毛玻璃适配的非卡片组件
    QFrame       *m_sidebarGlassBrand = nullptr;
    QFrame       *m_sidebarGlassContext = nullptr;
    QWidget      *m_activityCard = nullptr;  // activityList 的卡片容器

    void applyColorTemperature(int kelvin);
    void updateColorTempVisibility();
    void applyUiCustomization();

    // 仪表盘自适应缩放
    void updateDashboardScale();
    QWidget     *m_dashPage = nullptr;
    QWidget     *m_dashHeader = nullptr;
    QWidget     *m_overviewCard = nullptr;
    QWidget     *m_resCard = nullptr;
    QLabel      *m_dashOvTitle = nullptr;
    QLabel      *m_dashResTitle = nullptr;
    QLabel      *m_dashActTitle = nullptr;
    QLabel      *m_dashActDesc = nullptr;
    QPushButton *m_dashStartBtn = nullptr;
    QPushButton *m_dashStopBtn = nullptr;
    QPushButton *m_dashCheckBtn = nullptr;
    QVector<QLabel*> m_dashStatValues;
    QVector<QLabel*> m_dashHelperLabels;
    QVector<QLabel*> m_dashIcons;
    QVector<QProgressBar*> m_dashResBars;
    class ActivityItemDelegate *m_activityDelegate = nullptr;

    // GitHub Token
    QLineEdit    *m_githubTokenEdit = nullptr;

    // OpenClaw 计划任务设置
    QLabel       *m_taskLogonLabel = nullptr;
    QLabel       *m_taskStatusIcon = nullptr;
    QPushButton  *m_taskSwitchBtn = nullptr;
    void refreshTaskLogonType();
    void setTaskLogonType(const QString &logonType);

    // 网关管理
    GatewayManager *m_gateway = nullptr;
    QSpinBox       *m_portSpin = nullptr;
    QPushButton    *m_startStopBtn = nullptr;
    QPushButton    *m_restartBtn = nullptr;
    QLabel         *m_gatewayStatusLabel = nullptr;
    QLabel         *m_portStatusLabel = nullptr;

    // 进程守护
    ProcessGuard   *m_guard = nullptr;
    QTableWidget   *m_guardTable = nullptr;
    QPushButton    *m_addGuardBtn = nullptr;
    QPushButton    *m_pickProcessBtn = nullptr;
    QPushButton    *m_removeGuardBtn = nullptr;

    // 更新管理
    UpdateManager  *m_updater = nullptr;
    QComboBox      *m_channelCombo = nullptr;
    QTableWidget   *m_updateTable = nullptr;
    QPushButton    *m_fetchUpdateBtn = nullptr;
    QPushButton    *m_cliUpdateBtn = nullptr;
    QLabel         *m_ocVersionLabel = nullptr;
    QLabel         *m_ocChannelLabel = nullptr;
    QLabel         *m_ocInstallLabel = nullptr;
    QLabel         *m_ocAvailableLabel = nullptr;
    QLabel         *m_ocLatestVersionLabel = nullptr;
    QTextEdit      *m_ocDryRunOutput = nullptr;
    QProgressBar   *m_downloadProgress = nullptr;

    // 环境检测
    EnvironmentManager *m_envMgr = nullptr;
    QTableWidget   *m_envTable = nullptr;
    QPushButton    *m_detectEnvBtn = nullptr;
    QPushButton    *m_checkLatestBtn = nullptr;
    QPushButton    *m_updateEnvBtn = nullptr;

    // 托盘
    TrayManager    *m_tray = nullptr;

    // 状态栏
    QLabel         *m_statusMsg = nullptr;
    QLabel         *m_statusIndicator = nullptr;
    QLabel         *m_statusPort = nullptr;

    // 日志缓存
    struct LogEntry { QString time; QString msg; };
    QList<LogEntry> m_logEntries;

    // Dashboard 元素
    QLabel *m_dashGwStatus = nullptr;
    QLabel *m_dashGwValue = nullptr;
    QLabel *m_dashGuardCount = nullptr;
    QLabel *m_dashUpdateCount = nullptr;
    QLabel *m_dashEnvCount = nullptr;
    QLabel *m_dashPortValue = nullptr;
    QLabel *m_dashCpuValue = nullptr;
    QLabel *m_dashMemValue = nullptr;
    QProgressBar *m_dashCpuBar = nullptr;
    QProgressBar *m_dashMemBar = nullptr;
    QTimer *m_resourceTimer = nullptr;
    QListWidget *m_activityList = nullptr;
    QLabel *m_guardEmptyState = nullptr;
    QLabel *m_updateEmptyState = nullptr;
    QLabel *m_envEmptyState = nullptr;
    QLabel *m_guardSelectionPath = nullptr;
    QPushButton *m_guardCopyPathBtn = nullptr;

    // 工具方法
    QWidget* createNavButton(const QString &icon, const QString &text, int id);
    QFrame*  createSeparator();
    QWidget* createPageHeader(const QString &title, const QString &desc, const QString &iconName = QString());
    QFrame*  createCard(QWidget *content = nullptr);
    QFrame*  createStatCard(const QString &iconPath, const QString &labelText,
                            QLabel *&valueLabel, const QString &initialValue,
                            QLabel **statusDot = nullptr);
};