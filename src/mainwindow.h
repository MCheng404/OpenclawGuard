#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
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
    QCheckBox    *m_autoStartCheck = nullptr;
    QSpinBox     *m_cpuSpin = nullptr;
    QSpinBox     *m_memSpin = nullptr;

    // GitHub Token
    QLineEdit    *m_githubTokenEdit = nullptr;

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