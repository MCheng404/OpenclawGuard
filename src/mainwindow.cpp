#include "mainwindow.h"
#include "config.h"
#include "settings.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QHeaderView>
#include <QFileDialog>
#include <QCloseEvent>
#include <QShowEvent>
#include <QMessageBox>
#include <QProcess>
#include <QDateTime>
#include <QStyle>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextEdit>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QLineEdit>
#include <QApplication>
#include <QClipboard>
#include <QStatusBar>
#include <QScrollArea>
#include <QDesktopServices>
#include <QGridLayout>
#include <QSvgRenderer>
#include <QPainter>
#include <QPainterPath>
#include <QIcon>
#include <QStyledItemDelegate>
#include <QMouseEvent>
#include <functional>

// ═══ 自绘阴影卡片（偏移圆角矩形，天然无直角） ═══
class ShadowCard : public QFrame {
public:
    explicit ShadowCard(QWidget *parent = nullptr) : QFrame(parent) {
        setContentsMargins(10, 10, 10, 10);
        setAttribute(Qt::WA_TranslucentBackground);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        const int m = 10;
        QRect cardRect(m, m, width() - m*2, height() - m*2);
        const int cr = 14;
        auto cs = Theme::currentColors();

        // 45° 偏移圆角矩形阴影（3 层，从外到内）
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 8));
        p.drawRoundedRect(cardRect.adjusted(-2, -1, 4, 5), cr+2, cr+2);
        p.setBrush(QColor(0, 0, 0, 12));
        p.drawRoundedRect(cardRect.adjusted(-1, 0, 3, 4), cr+1, cr+1);
        p.setBrush(QColor(0, 0, 0, 16));
        p.drawRoundedRect(cardRect.adjusted(0, 1, 2, 3), cr, cr);

        // 卡片背景
        p.setBrush(cs.cardBg);
        p.setPen(QPen(cs.borderColor, 1));
        p.drawRoundedRect(cardRect, cr, cr);

        p.end();
    }
};

// 图标加载辅助
static QIcon loadSvgIcon(const QString &name) {
    return QIcon(QString(":/icons/icons/%1.svg").arg(name));
}

static void setStatusBadgeStyle(QLabel *label, const QColor &baseColor)
{
    if (!label) return;
    label->setStyleSheet(QString(
        "background: rgba(%1,%2,%3,0.14); color: %4; border: 1px solid rgba(%1,%2,%3,0.26);"
        "border-radius: 999px; padding: 6px 12px; font-size: 12px; font-weight: 600;"
    ).arg(baseColor.red()).arg(baseColor.green()).arg(baseColor.blue()).arg(baseColor.name()));
}

static void setSemanticItemStyle(QTableWidgetItem *item, const QString &tone)
{
    if (!item) return;

    QColor fg;
    QColor bg;
    QColor border;

    if (tone == "success") {
        fg = QColor("#34d399");
        bg = QColor(52, 211, 153, 36);
        border = QColor(52, 211, 153, 72);
    } else if (tone == "danger") {
        fg = QColor("#ef4444");
        bg = QColor(239, 68, 68, 36);
        border = QColor(239, 68, 68, 72);
    } else if (tone == "warning") {
        fg = QColor("#f59e0b");
        bg = QColor(245, 158, 11, 36);
        border = QColor(245, 158, 11, 72);
    } else {
        fg = QColor("#4f8cff");
        bg = QColor(79, 140, 255, 30);
        border = QColor(79, 140, 255, 64);
    }

    item->setTextAlignment(Qt::AlignCenter);
    item->setForeground(fg);
    item->setBackground(bg);
    item->setData(Qt::UserRole, border);
}

class TableActionDelegate final : public QStyledItemDelegate
{
public:
    explicit TableActionDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem baseOption(option);
        initStyleOption(&baseOption, index);
        baseOption.text.clear();
        QStyledItemDelegate::paint(painter, baseOption, index);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setRenderHint(QPainter::TextAntialiasing, true);

        const bool enabled = index.data(Qt::UserRole + 1).toBool();
        const QString text = index.data(Qt::DisplayRole).toString();
        const auto cs = Theme::currentColors();
        const bool dark = cs.mainBg.value() < 128;

        QRectF buttonRect = option.rect.adjusted(16, 8, -16, -8);
        buttonRect.setHeight(32.0);
        buttonRect.moveTop(option.rect.top() + (option.rect.height() - buttonRect.height()) / 2.0);

        QColor fill = dark ? QColor(79, 140, 255, 26) : QColor(59, 130, 246, 16);
        QColor border = dark ? QColor(79, 140, 255, 46) : QColor(59, 130, 246, 40);
        QColor textColor = dark ? QColor("#8fb4ff") : QColor("#2563eb");

        if (!enabled) {
            fill = dark ? QColor(255, 255, 255, 6) : QColor(15, 23, 42, 6);
            border = dark ? QColor(255, 255, 255, 10) : QColor(15, 23, 42, 14);
            textColor = dark ? QColor("#5e6278") : QColor("#c3c7d0");
        } else if (option.state & QStyle::State_MouseOver) {
            fill = dark ? QColor(79, 140, 255, 40) : QColor(59, 130, 246, 28);
            border = dark ? QColor(79, 140, 255, 68) : QColor(59, 130, 246, 56);
            textColor = dark ? QColor("#d8e7ff") : QColor("#1d4ed8");
        }

        painter->setPen(QPen(border, 1.0));
        painter->setBrush(fill);
        painter->drawRoundedRect(buttonRect, 9, 9);

        QFont font = option.font;
        font.setWeight(QFont::DemiBold);
        painter->setFont(font);
        painter->setPen(textColor);
        painter->drawText(buttonRect, Qt::AlignCenter, text);
        painter->restore();
    }
};

class GuardToggleDelegate final : public QStyledItemDelegate
{
public:
    explicit GuardToggleDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionViewItem baseOption(option);
        initStyleOption(&baseOption, index);
        baseOption.text.clear();
        QStyledItemDelegate::paint(painter, baseOption, index);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setRenderHint(QPainter::TextAntialiasing, true);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

        const bool checked = index.data(Qt::UserRole + 1).toBool();
        const auto cs = Theme::currentColors();
        const bool dark = cs.mainBg.value() < 128;

        QRectF trackRect(option.rect.center().x() - 22.0, option.rect.center().y() - 12.0, 44.0, 24.0);
        const qreal radius = trackRect.height() / 2.0;
        const qreal knobSize = 18.0;
        const qreal knobY = trackRect.top() + (trackRect.height() - knobSize) / 2.0;
        const qreal knobX = checked ? (trackRect.right() - knobSize - 3.0) : (trackRect.left() + 3.0);

        QColor trackFill = checked
            ? (dark ? QColor(cs.accent.red(), cs.accent.green(), cs.accent.blue(), 190)
                    : QColor(cs.accent.red(), cs.accent.green(), cs.accent.blue(), 170))
            : (dark ? QColor("#2d2e45") : QColor("#d4d4d8"));
        QColor trackBorder = checked
            ? (dark ? QColor(cs.accentHover.red(), cs.accentHover.green(), cs.accentHover.blue(), 220)
                    : QColor(cs.accent.red(), cs.accent.green(), cs.accent.blue(), 200))
            : (dark ? QColor(255, 255, 255, 24) : QColor(15, 23, 42, 24));

        painter->setPen(QPen(trackBorder, 1.0));
        painter->setBrush(trackFill);
        painter->drawRoundedRect(trackRect, radius, radius);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 0, 0, checked ? 44 : 24));
        painter->drawEllipse(QRectF(knobX + 0.5, knobY + 1.5, knobSize, knobSize));

        painter->setPen(QPen(QColor(255, 255, 255, 210), 1.0));
        painter->setBrush(Qt::white);
        painter->drawEllipse(QRectF(knobX, knobY, knobSize, knobSize));
        painter->restore();
    }
};
// ═══ 活动列表卡片代理 ═══
class ActivityItemDelegate final : public QStyledItemDelegate {
public:
    explicit ActivityItemDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        bool isPlaceholder = index.data(Qt::UserRole).toString() == "placeholder";
        QRect r = option.rect.adjusted(4, 2, -4, -2);

        // 卡片背景
        QColor bg = isPlaceholder
            ? QColor(255, 255, 255, 8)
            : (option.state & QStyle::State_MouseOver
                ? QColor(79, 140, 255, 18)
                : QColor(255, 255, 255, 10));
        painter->setPen(Qt::NoPen);
        painter->setBrush(bg);
        painter->drawRoundedRect(r, 10, 10);

        // 左侧色条
        if (!isPlaceholder) {
            painter->setBrush(QColor(79, 140, 255, 60));
            painter->drawRoundedRect(QRect(r.left(), r.top() + 4, 3, r.height() - 8), 1, 1);
        }

        // 文字
        painter->setPen(isPlaceholder ? QColor(120, 130, 160) : QColor(207, 211, 232));
        QFont f = option.font;
        f.setPixelSize(12);
        painter->setFont(f);
        painter->drawText(r.adjusted(12, 6, -8, -6), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
                          index.data(Qt::DisplayRole).toString());

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QString text = index.data(Qt::DisplayRole).toString();
        QFontMetrics fm(option.font);
        int w = option.rect.width() - 24;  // margins
        QRect bound = fm.boundingRect(QRect(0, 0, w, 1000), Qt::TextWordWrap, text);
        return QSize(option.rect.width(), qMax(36, bound.height() + 16));
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("进程管理控制台");
    resize(1000, 680);
    setMinimumSize(800, 540);

    m_gateway = new GatewayManager(this);
    m_guard   = new ProcessGuard(this);
    m_updater = new UpdateManager(this);
    m_envMgr  = new EnvironmentManager(this);

    setupUI();
    setupConnections();
    loadSettings();
    applyTheme();

    // 网关状态脉冲动画
    m_pulseOpacity = new QGraphicsOpacityEffect(this);
    m_pulseAnim = new QPropertyAnimation(m_pulseOpacity, "opacity", this);
    m_pulseAnim->setDuration(1200);
    m_pulseAnim->setStartValue(1.0);
    m_pulseAnim->setEndValue(0.25);
    m_pulseAnim->setEasingCurve(QEasingCurve::InOutSine);

    m_dashPulseOpacity = new QGraphicsOpacityEffect(this);
    m_dashPulseAnim = new QPropertyAnimation(m_dashPulseOpacity, "opacity", this);
    m_dashPulseAnim->setDuration(1400);
    m_dashPulseAnim->setStartValue(1.0);
    m_dashPulseAnim->setEndValue(0.2);
    m_dashPulseAnim->setEasingCurve(QEasingCurve::InOutSine);

    // 更新进度脉冲
    m_updatePulseTimer = new QTimer(this);
    m_updatePulseTimer->setInterval(20);
    m_updatePulseCounter = 0;
    connect(m_updatePulseTimer, &QTimer::timeout, this, [this]() {
        m_updatePulseCounter = (m_updatePulseCounter + 1) % 100;
        m_downloadProgress->setValue(m_updatePulseCounter);
    });

    m_guard->setInterval(Config::PROCESS_CHECK_INTERVAL);
    m_guard->start();

    // 应用启动即自动检测网关状态
    m_gateway->monitor()->setInterval(Config::PORT_CHECK_INTERVAL);
    m_gateway->monitor()->start();

    // 启动时自动检查更新
    m_updater->fetchReleases();

    // 默认显示 Dashboard
    m_navGroup->button(0)->setChecked(true);
    m_pages->setCurrentIndex(0);
    updateSidebarContext(0);
    updateResponsiveLayout();
    updateGuardSelectionDetails();

    // 启动后自动获取 Openclaw CLI 状态
    QTimer::singleShot(500, this, [this]() {
        m_updater->getUpdateStatus();
    });
}

MainWindow::~MainWindow() {}

// ====================== UI 构建 ======================

QWidget* MainWindow::createNavButton(const QString &iconName, const QString &text, int id)
{
    auto *btn = new QPushButton();
    btn->setObjectName("navBtn");
    btn->setCheckable(true);
    btn->setIcon(loadSvgIcon(iconName));
    btn->setIconSize(QSize(17, 17));
    btn->setText(text);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setMinimumHeight(44);
    btn->setIconSize(QSize(18, 18));
    btn->setAutoDefault(false);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_navGroup->addButton(btn, id);
    return btn;
}

QFrame* MainWindow::createSeparator()
{
    auto *sep = new QFrame();
    sep->setObjectName("separator");
    sep->setFrameShape(QFrame::HLine);
    return sep;
}

QWidget* MainWindow::createPageHeader(const QString &title, const QString &desc)
{
    auto *w = new QWidget();
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 16);
    lay->setSpacing(4);
    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName("pageTitle");
    auto *descLabel = new QLabel(desc);
    descLabel->setObjectName("pageDesc");
    lay->addWidget(titleLabel);
    lay->addWidget(descLabel);
    return w;
}

QFrame* MainWindow::createCard(QWidget *content)
{
    auto *card = new ShadowCard();
    card->setObjectName("card");
    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(20, 20, 20, 20);  // 内容不被阴影遮挡
    lay->setSpacing(12);
    if (content) lay->addWidget(content);
    return card;
}

QFrame* MainWindow::createStatCard(const QString &iconName, const QString &labelText,
                                    QLabel *&valueLabel, const QString &initialValue,
                                    QLabel **statusDot)
{
    auto *card = new ShadowCard();
    card->setObjectName("statCard");
    card->setMinimumHeight(130);
    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(24, 24, 24, 24);
    lay->setSpacing(8);

    auto *topRow = new QHBoxLayout();
    auto *iconLbl = new QLabel();
    iconLbl->setPixmap(loadSvgIcon(iconName).pixmap(28, 28));
    topRow->addWidget(iconLbl);
    topRow->addStretch();

    if (statusDot) {
        auto *dot = new QLabel();
        dot->setFixedSize(10, 10);
        dot->setStyleSheet(QString("background: %1; border-radius: 5px;")
            .arg(Theme::currentColors().textSecondary.name()));
        *statusDot = dot;
        topRow->addWidget(dot);
    }

    lay->addLayout(topRow);

    valueLabel = new QLabel(initialValue);
    valueLabel->setObjectName("statValue");
    lay->addWidget(valueLabel);

    auto *label = new QLabel(labelText);
    label->setObjectName("statLabel");
    lay->addWidget(label);

    return card;
}

void MainWindow::setupSidebar(QHBoxLayout *mainLayout)
{
    m_sidebar = new QWidget();
    m_sidebar->setObjectName("sidebar");
    m_sidebar->setFixedWidth(236);
    auto *sideLayout = new QVBoxLayout(m_sidebar);
    sideLayout->setContentsMargins(14, 16, 14, 16);
    sideLayout->setSpacing(0);

    m_sidebarBrandCard = new QFrame();
    m_sidebarBrandCard->setObjectName("sidebarBrandCard");
    auto *brandLayout = new QVBoxLayout(m_sidebarBrandCard);
    brandLayout->setContentsMargins(14, 14, 14, 14);
    brandLayout->setSpacing(4);

    m_logoLabel = new QLabel("进程管理控制台");
    m_logoLabel->setObjectName("logoLabel");
    m_sidebarTagline = new QLabel("进程管理控制台");
    m_sidebarTagline->setObjectName("sidebarTagline");
    brandLayout->addWidget(m_logoLabel);
    brandLayout->addWidget(m_sidebarTagline);
    sideLayout->addWidget(m_sidebarBrandCard);
    sideLayout->addSpacing(14);

    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);

    auto *overviewLabel = new QLabel("总览");
    overviewLabel->setObjectName("navSectionLabel");
    sideLayout->addWidget(overviewLabel);
    sideLayout->addWidget(createNavButton("layout-dashboard", "仪表盘", 0));
    sideLayout->addSpacing(8);

    auto *runtimeLabel = new QLabel("运行与维护");
    runtimeLabel->setObjectName("navSectionLabel");
    sideLayout->addWidget(runtimeLabel);
    sideLayout->addWidget(createNavButton("server", "网关管理", 1));
    sideLayout->addWidget(createNavButton("shield-check", "进程", 2));
    sideLayout->addWidget(createNavButton("download-cloud", "更新", 3));
    sideLayout->addWidget(createNavButton("cpu", "环境", 4));
    sideLayout->addSpacing(8);

    auto *prefLabel = new QLabel("偏好与系统");
    prefLabel->setObjectName("navSectionLabel");
    sideLayout->addWidget(prefLabel);
    sideLayout->addWidget(createNavButton("settings", "设置中心", 5));

    sideLayout->addStretch();

    m_sidebarContextCard = new QFrame();
    m_sidebarContextCard->setObjectName("sidebarContextCard");
    auto *contextLayout = new QVBoxLayout(m_sidebarContextCard);
    contextLayout->setContentsMargins(14, 14, 14, 14);
    contextLayout->setSpacing(8);

    auto *contextTitle = new QLabel("当前工作区");
    contextTitle->setObjectName("navSectionLabel");
    m_sidebarPageLabel = new QLabel("仪表盘");
    m_sidebarPageLabel->setObjectName("sidebarPageLabel");
    m_sidebarPageHint = new QLabel("查看系统概览、最近事件与关键状态摘要");
    m_sidebarPageHint->setObjectName("sidebarPageHint");
    m_sidebarPageHint->setWordWrap(true);
    contextLayout->addWidget(contextTitle);
    contextLayout->addWidget(m_sidebarPageLabel);
    contextLayout->addWidget(m_sidebarPageHint);
    contextLayout->addWidget(createSeparator());

    auto *bottomRow = new QHBoxLayout();
    bottomRow->setContentsMargins(0, 2, 0, 0);
    bottomRow->setSpacing(8);
    m_themeBtn = new QPushButton();
    m_themeBtn->setObjectName("iconBtn");
    m_themeBtn->setIcon(loadSvgIcon("moon"));
    m_themeBtn->setIconSize(QSize(16, 16));
    m_themeBtn->setToolTip("切换主题");
    m_themeBtn->setCursor(Qt::PointingHandCursor);

    auto *themeMeta = new QVBoxLayout();
    themeMeta->setContentsMargins(0, 0, 0, 0);
    themeMeta->setSpacing(2);
    auto *themeTitle = new QLabel("外观主题");
    themeTitle->setObjectName("sidebarMetaLabel");
    m_sidebarThemeLabel = new QLabel("跟随系统");
    m_sidebarThemeLabel->setObjectName("sidebarMetaValue");
    themeMeta->addWidget(themeTitle);
    themeMeta->addWidget(m_sidebarThemeLabel);

    m_versionLabel = new QLabel("v1.0.0");
    m_versionLabel->setObjectName("versionLabel");

    bottomRow->addWidget(m_themeBtn, 0, Qt::AlignTop);
    bottomRow->addLayout(themeMeta, 1);
    bottomRow->addWidget(m_versionLabel, 0, Qt::AlignBottom | Qt::AlignRight);
    contextLayout->addLayout(bottomRow);

    sideLayout->addWidget(m_sidebarContextCard);
    mainLayout->addWidget(m_sidebar);
}

void MainWindow::setupPages()
{
    m_pages = new QStackedWidget();
    m_pages->setContentsMargins(0, 0, 0, 0);

    // ====== Page 0: Dashboard ======
    auto *dashPage = new QWidget();
    auto *dashLayout = new QVBoxLayout(dashPage);
    dashLayout->setContentsMargins(24, 24, 24, 24);
    dashLayout->setSpacing(16);
    dashLayout->addWidget(createPageHeader("仪表盘", "系统概览、运行状态与最近事件"));

    auto *overviewRow = new QHBoxLayout();
    overviewRow->setSpacing(16);

    auto *statsPanel = new QWidget();
    statsPanel->setMinimumWidth(400);
    auto *statsGrid = new QGridLayout(statsPanel);
    statsGrid->setContentsMargins(0, 0, 0, 0);
    statsGrid->setHorizontalSpacing(16);
    statsGrid->setVerticalSpacing(16);
    statsGrid->addWidget(createStatCard("server", "网关状态", m_dashGwValue, "未启动", &m_dashGwStatus), 0, 0);
    statsGrid->addWidget(createStatCard("shield-check", "守护进程数", m_dashGuardCount, "0"), 0, 1);
    statsGrid->addWidget(createStatCard("download-cloud", "可用更新", m_dashUpdateCount, "0"), 1, 0);
    statsGrid->addWidget(createStatCard("cpu", "环境项数", m_dashEnvCount, "0"), 1, 1);
    overviewRow->addWidget(statsPanel, 2);

    auto *activityCard = createCard();
    activityCard->setProperty("dashboardCard", true);
    activityCard->setMinimumWidth(280);
    activityCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *activityInner = static_cast<QVBoxLayout*>(activityCard->layout());
    auto *activityTitle = new QLabel("最近事件");
    activityTitle->setObjectName("sectionTitle");
    auto *activityDesc = new QLabel("保留最近 50 条关键状态与操作反馈");
    activityDesc->setObjectName("helperText");
    m_dashPortValue = new QLabel("监听端口 --");
    m_dashPortValue->setObjectName("statusPill");
    m_activityList = new QListWidget();
    m_activityList->setObjectName("activityList");
    m_activityList->setSelectionMode(QAbstractItemView::NoSelection);
    m_activityList->setFocusPolicy(Qt::NoFocus);
    m_activityList->setFrameShape(QFrame::NoFrame);
    m_activityList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_activityList->setUniformItemSizes(false);
    m_activityList->setWordWrap(true);
    m_activityList->setSpacing(4);
    m_activityList->setMinimumHeight(160);
    m_activityList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_activityList->setItemDelegate(new ActivityItemDelegate(m_activityList));
    auto *idleItem = new QListWidgetItem("等待系统事件...");
    idleItem->setData(Qt::UserRole, "placeholder");
    idleItem->setFlags(idleItem->flags() & ~Qt::ItemIsSelectable);
    m_activityList->addItem(idleItem);
    activityInner->addWidget(activityTitle);
    activityInner->addWidget(activityDesc);
    activityInner->addWidget(m_dashPortValue, 0, Qt::AlignLeft);
    activityInner->addWidget(m_activityList, 1);
    overviewRow->addWidget(activityCard, 1);
    dashLayout->addLayout(overviewRow);

    auto *quickCard = createCard();
    quickCard->setProperty("dashboardCard", true);
    auto *quickCardLayout = static_cast<QVBoxLayout*>(quickCard->layout());
    auto *quickTitle = new QLabel("快捷操作");
    quickTitle->setObjectName("sectionTitle");
    auto *quickDesc = new QLabel("高频维护动作会直接作用于当前配置与更新通道");
    quickDesc->setObjectName("helperText");
    quickCardLayout->addWidget(quickTitle);
    quickCardLayout->addWidget(quickDesc);
    auto *quickBtns = new QHBoxLayout();
    quickBtns->setSpacing(8);
    auto *startGwBtn = new QPushButton();
    startGwBtn->setObjectName("primaryBtn");
    startGwBtn->setText("启动网关");
    startGwBtn->setIcon(loadSvgIcon("play"));
    auto *stopGwBtn = new QPushButton();
    stopGwBtn->setObjectName("secondaryBtn");
    stopGwBtn->setText("停止网关");
    stopGwBtn->setIcon(loadSvgIcon("square"));
    auto *checkUpBtn = new QPushButton();
    checkUpBtn->setObjectName("secondaryBtn");
    checkUpBtn->setText("检查更新");
    checkUpBtn->setIcon(loadSvgIcon("refresh-cw"));
    quickBtns->addWidget(startGwBtn);
    quickBtns->addWidget(stopGwBtn);
    quickBtns->addWidget(checkUpBtn);
    quickBtns->addStretch();
    quickCardLayout->addLayout(quickBtns);

    connect(startGwBtn, &QPushButton::clicked, this, [this]() {
        if (!m_gateway->isGatewayRunning()) onStartStopGateway();
    });
    connect(stopGwBtn, &QPushButton::clicked, this, [this]() {
        if (m_gateway->isGatewayRunning()) onStartStopGateway();
    });
    connect(checkUpBtn, &QPushButton::clicked, this, &MainWindow::onFetchUpdates);

    dashLayout->addWidget(quickCard);

    dashLayout->addStretch();
    m_pages->addWidget(dashPage);

    // ====== Page 1: 网关管理 ======
    auto *gwPage = new QWidget();
    auto *gwLayout = new QVBoxLayout(gwPage);
    gwLayout->setContentsMargins(24, 24, 24, 24);
    gwLayout->setSpacing(16);
    gwLayout->addWidget(createPageHeader("网关管理", "配置网关路径、监听端口与运行控制"));

    auto *gwHeroCard = createCard();
    gwHeroCard->setProperty("pageTopCard", true);
    auto *gwHeroInner = static_cast<QVBoxLayout*>(gwHeroCard->layout());
    auto *gwHeroTitle = new QLabel("网关控制台");
    gwHeroTitle->setObjectName("sectionTitle");
    auto *gwHeroDesc = new QLabel("内置 openclaw gateway 命令，启动/停止/重启一站式管理");
    gwHeroDesc->setObjectName("helperText");
    gwHeroInner->addWidget(gwHeroTitle);
    gwHeroInner->addWidget(gwHeroDesc);

    auto *gwBadgeRow = new QHBoxLayout();
    gwBadgeRow->setSpacing(8);
    m_gatewayStatusLabel = new QLabel("网关未启动");
    m_gatewayStatusLabel->setObjectName("statusBadge");
    setStatusBadgeStyle(m_gatewayStatusLabel, QColor("#ef4444"));
    m_portStatusLabel = new QLabel("监听端口 --");
    m_portStatusLabel->setObjectName("statusBadge");
    setStatusBadgeStyle(m_portStatusLabel, Theme::currentColors().accent);
    gwBadgeRow->addWidget(m_gatewayStatusLabel, 0, Qt::AlignLeft);
    gwBadgeRow->addWidget(m_portStatusLabel, 0, Qt::AlignLeft);
    gwBadgeRow->addStretch();
    gwHeroInner->addLayout(gwBadgeRow);

    auto *gwActionRow = new QHBoxLayout();
    gwActionRow->setSpacing(8);
    m_startStopBtn = new QPushButton("启动网关");
    m_startStopBtn->setObjectName("primaryBtn");
    m_startStopBtn->setIcon(loadSvgIcon("play"));
    m_restartBtn = new QPushButton("重启");
    m_restartBtn->setObjectName("secondaryBtn");
    m_restartBtn->setIcon(loadSvgIcon("rotate-cw"));
    gwActionRow->addWidget(m_startStopBtn);
    gwActionRow->addWidget(m_restartBtn);
    gwActionRow->addStretch();
    gwHeroInner->addLayout(gwActionRow);
    gwLayout->addWidget(gwHeroCard);

    // 端口监控卡片
    auto *cfgCard = createCard();
    auto *cfgInner = static_cast<QVBoxLayout*>(cfgCard->layout());
    auto *cfgTitle = new QLabel("端口监控");
    cfgTitle->setObjectName("sectionTitle");
    auto *cfgDesc = new QLabel("监控 OpenClaw 网关端口状态，端口由 OpenClaw 自身配置管理");
    cfgDesc->setObjectName("helperText");
    cfgInner->addWidget(cfgTitle);
    cfgInner->addWidget(cfgDesc);

    auto *portRow = new QHBoxLayout();
    portRow->addWidget(new QLabel("监听端口"));
    m_portSpin = new QSpinBox();
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(Config::GATEWAY_DEFAULT_PORT);
    portRow->addWidget(m_portSpin, 1);
    portRow->addStretch(3);
    cfgInner->addLayout(portRow);
    gwLayout->addWidget(cfgCard);
    gwLayout->addStretch();
    m_pages->addWidget(gwPage);

    // ====== Page 2: 进程 ======
    auto *gdPage = new QWidget();
    auto *gdLayout = new QVBoxLayout(gdPage);
    gdLayout->setContentsMargins(24, 24, 24, 24);
    gdLayout->setSpacing(16);
    gdLayout->addWidget(createPageHeader("进程", "监控关键进程，崩溃自动拉起"));

    auto *gdTopCard = createCard();
    gdTopCard->setProperty("pageTopCard", true);
    auto *gdTopInner = static_cast<QVBoxLayout*>(gdTopCard->layout());
    auto *gdTopTitle = new QLabel("守护操作");
    gdTopTitle->setObjectName("sectionTitle");
    auto *gdTopDesc = new QLabel("添加、启停和移除受监控进程，列表状态会实时同步");
    gdTopDesc->setObjectName("helperText");
    gdTopInner->addWidget(gdTopTitle);
    gdTopInner->addWidget(gdTopDesc);
    auto *gdBtnRow = new QHBoxLayout();
    m_addGuardBtn = new QPushButton("浏览添加");
    m_addGuardBtn->setObjectName("primaryBtn");
    m_addGuardBtn->setIcon(loadSvgIcon("folder-open"));
    m_pickProcessBtn = new QPushButton("从进程选取");
    m_pickProcessBtn->setObjectName("secondaryBtn");
    m_pickProcessBtn->setIcon(loadSvgIcon("list"));
    m_removeGuardBtn = new QPushButton("移除选中");
    m_removeGuardBtn->setObjectName("dangerBtn");
    m_removeGuardBtn->setIcon(loadSvgIcon("trash-2"));
    m_removeGuardBtn->setEnabled(false);
    gdBtnRow->addWidget(m_addGuardBtn);
    gdBtnRow->addWidget(m_pickProcessBtn);
    gdBtnRow->addWidget(m_removeGuardBtn);
    gdBtnRow->addStretch();
    gdTopInner->addLayout(gdBtnRow);
    gdLayout->addWidget(gdTopCard);

    auto *gdTableCard = createCard();
    auto *gdTblInner = static_cast<QVBoxLayout*>(gdTableCard->layout());

    auto *gdInfoWrap = new QWidget();
    auto *gdInfoRow = new QHBoxLayout(gdInfoWrap);
    gdInfoRow->setContentsMargins(0, 0, 0, 0);
    gdInfoRow->setSpacing(10);
    auto *gdInfoLabel = new QLabel("当前选中路径");
    gdInfoLabel->setObjectName("fieldLabel");
    m_guardSelectionPath = new QLabel("未选择守护项");
    m_guardSelectionPath->setObjectName("helperText");
    m_guardSelectionPath->setWordWrap(true);
    m_guardSelectionPath->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_guardCopyPathBtn = new QPushButton("复制路径");
    m_guardCopyPathBtn->setObjectName("tableBtn");
    m_guardCopyPathBtn->setEnabled(false);
    gdInfoRow->addWidget(gdInfoLabel, 0, Qt::AlignTop);
    gdInfoRow->addWidget(m_guardSelectionPath, 1);
    gdInfoRow->addWidget(m_guardCopyPathBtn, 0, Qt::AlignTop);
    gdTblInner->addWidget(gdInfoWrap);

    m_guardEmptyState = new QLabel("暂无守护进程\n可通过“浏览添加”或“从进程选取”开始");
    m_guardEmptyState->setObjectName("emptyState");
    m_guardEmptyState->setAlignment(Qt::AlignCenter);
    m_guardEmptyState->setVisible(false);
    gdTblInner->addWidget(m_guardEmptyState);
    m_guardTable = new QTableWidget();
    m_guardTable->setColumnCount(4);
    m_guardTable->setHorizontalHeaderLabels({"名称", "路径", "状态", "启用"});
    m_guardTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_guardTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_guardTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_guardTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_guardTable->horizontalHeader()->setMinimumSectionSize(72);
    m_guardTable->setColumnWidth(0, 148);
    m_guardTable->setColumnWidth(3, 112);
    m_guardTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_guardTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_guardTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_guardTable->setWordWrap(false);
    m_guardTable->setTextElideMode(Qt::ElideMiddle);
    m_guardTable->setMouseTracking(true);
    m_guardTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_guardTable->horizontalHeader()->setHighlightSections(false);
    m_guardTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_guardTable->verticalHeader()->setVisible(false);
    m_guardTable->verticalHeader()->setDefaultSectionSize(40);
    m_guardTable->setShowGrid(false);
    m_guardTable->setAlternatingRowColors(true);
    m_guardTable->setItemDelegateForColumn(3, new GuardToggleDelegate(m_guardTable));
    gdTblInner->addWidget(m_guardTable);
    gdLayout->addWidget(gdTableCard, 1);
    m_pages->addWidget(gdPage);

    // ====== Page 3: 更新 ======
    auto *upPage = new QWidget();
    auto *upOuterLayout = new QVBoxLayout(upPage);
    upOuterLayout->setContentsMargins(0, 0, 0, 0);

    auto *upScrollArea = new QScrollArea();
    upScrollArea->setWidgetResizable(true);
    upScrollArea->setFrameShape(QFrame::NoFrame);
    upScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    upScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto *upScrollContent = new QWidget();
    upScrollContent->setMinimumWidth(600);
    auto *upLayout = new QVBoxLayout(upScrollContent);
    upLayout->setContentsMargins(24, 24, 24, 24);
    upLayout->setSpacing(16);
    upLayout->addWidget(createPageHeader("更新", "通过 npm 检查与安装 Openclaw 更新"));

    // ------ 状态概览（横排 4 格迷你卡片） ------
    auto *statusGrid = new QHBoxLayout();
    statusGrid->setSpacing(12);

    m_ocVersionLabel  = new QLabel(QStringLiteral("—"));
    m_ocChannelLabel  = new QLabel(QStringLiteral("—"));
    m_ocInstallLabel  = new QLabel(QStringLiteral("—"));
    m_ocAvailableLabel = new QLabel(QStringLiteral("—"));

    auto makeStatusMini = [&](const QString &iconName, const QString &title, QLabel *valLabel) -> QFrame * {
        auto *c = createCard();
        c->setProperty("statCard", true);
        auto *inner = static_cast<QVBoxLayout*>(c->layout());
        inner->setSpacing(4);
        inner->setContentsMargins(14, 12, 14, 12);
        auto *hdr = new QHBoxLayout();
        hdr->setSpacing(6);
        auto *ic = new QLabel();
        ic->setPixmap(loadSvgIcon(iconName).pixmap(16, 16));
        ic->setFixedSize(20, 20);
        auto *ttl = new QLabel(title);
        ttl->setObjectName("helperText");
        ttl->setStyleSheet("font-size: 11px;");
        hdr->addWidget(ic);
        hdr->addWidget(ttl);
        hdr->addStretch();
        inner->addLayout(hdr);
        valLabel->setObjectName("miniStatValue");
        valLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        valLabel->setWordWrap(true);
        inner->addWidget(valLabel);
        return c;
    };

    statusGrid->addWidget(makeStatusMini("server",         "当前版本", m_ocVersionLabel));
    statusGrid->addWidget(makeStatusMini("download-cloud",  "更新通道", m_ocChannelLabel));
    statusGrid->addWidget(makeStatusMini("settings",        "安装方式", m_ocInstallLabel));
    statusGrid->addWidget(makeStatusMini("refresh-cw",      "可用更新", m_ocAvailableLabel));
    upLayout->addLayout(statusGrid);

    // ------ 操作区（紧凑：标题 + 按钮同一行） ------
    auto *cliCard = createCard();
    auto *cliInner = static_cast<QVBoxLayout*>(cliCard->layout());
    cliInner->setSpacing(10);

    auto *cliTopRow = new QHBoxLayout();
    cliTopRow->setSpacing(8);
    auto *cliIcon = new QLabel();
    cliIcon->setPixmap(loadSvgIcon("download-cloud").pixmap(18, 18));
    cliIcon->setFixedSize(22, 22);
    auto *cliTitle = new QLabel("更新操作");
    cliTitle->setObjectName("sectionTitle");
    m_channelCombo = new QComboBox();
    m_channelCombo->addItems({"当前通道", "stable", "beta"});
    m_channelCombo->setFixedWidth(120);
    m_channelCombo->setObjectName("channelCombo");
    m_fetchUpdateBtn = new QPushButton("检查状态");
    m_fetchUpdateBtn->setObjectName("secondaryBtn");
    m_fetchUpdateBtn->setIcon(loadSvgIcon("refresh-cw"));
    m_cliUpdateBtn = new QPushButton("执行更新");
    m_cliUpdateBtn->setObjectName("primaryBtn");
    m_cliUpdateBtn->setIcon(loadSvgIcon("download-cloud"));
    cliTopRow->addWidget(cliIcon);
    cliTopRow->addWidget(cliTitle);
    cliTopRow->addStretch();
    cliTopRow->addWidget(new QLabel("通道"));
    cliTopRow->addWidget(m_channelCombo);
    cliTopRow->addWidget(m_fetchUpdateBtn);
    cliTopRow->addWidget(m_cliUpdateBtn);
    cliInner->addLayout(cliTopRow);

    m_ocDryRunOutput = new QTextEdit();
    m_ocDryRunOutput->setObjectName("dryRunOutput");
    m_ocDryRunOutput->setReadOnly(true);
    m_ocDryRunOutput->setPlaceholderText("点击「检查状态」查看 Openclaw 更新状态...");
    m_ocDryRunOutput->setMinimumHeight(100);
    m_ocDryRunOutput->setMaximumHeight(240);
    m_ocDryRunOutput->setFont(QFont("JetBrains Mono, Consolas, monospace", 10));
    m_ocDryRunOutput->setFrameShape(QFrame::NoFrame);
    cliInner->addWidget(m_ocDryRunOutput);
    upLayout->addWidget(cliCard);

    m_downloadProgress = new QProgressBar();
    m_downloadProgress->setVisible(false);
    m_downloadProgress->setFixedHeight(6);
    m_downloadProgress->setObjectName("downloadProgress");
    upLayout->addWidget(m_downloadProgress);

    // ------ Releases 列表 ------
    auto *upTableCard = createCard();
    auto *upTblInner = static_cast<QVBoxLayout*>(upTableCard->layout());
    upTblInner->setSpacing(8);

    auto *tblHeader = new QHBoxLayout();
    tblHeader->setSpacing(6);
    auto *tblIcon = new QLabel();
    tblIcon->setPixmap(loadSvgIcon("list").pixmap(16, 16));
    tblIcon->setFixedSize(20, 20);
    auto *tblTitle = new QLabel("GitHub Releases");
    tblTitle->setObjectName("sectionTitle");
    tblHeader->addWidget(tblIcon);
    tblHeader->addWidget(tblTitle);
    tblHeader->addStretch();
    upTblInner->addLayout(tblHeader);

    m_updateEmptyState = new QLabel("暂无更新记录\n点击\"检查更新\"拉取 GitHub Releases");
    m_updateEmptyState->setObjectName("emptyState");
    m_updateEmptyState->setAlignment(Qt::AlignCenter);
    m_updateEmptyState->setVisible(false);
    upTblInner->addWidget(m_updateEmptyState);
    m_updateTable = new QTableWidget();
    m_updateTable->setMinimumHeight(200);
    m_updateTable->setColumnCount(4);
    m_updateTable->setHorizontalHeaderLabels({"版本", "名称", "类型", "日期"});
    m_updateTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_updateTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_updateTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_updateTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_updateTable->horizontalHeader()->setMinimumSectionSize(72);
    m_updateTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_updateTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_updateTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_updateTable->setWordWrap(false);
    m_updateTable->setTextElideMode(Qt::ElideRight);
    m_updateTable->setMouseTracking(true);
    m_updateTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_updateTable->horizontalHeader()->setHighlightSections(false);
    m_updateTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_updateTable->verticalHeader()->setVisible(false);
    m_updateTable->verticalHeader()->setDefaultSectionSize(44);
    m_updateTable->setShowGrid(false);
    m_updateTable->setAlternatingRowColors(true);
    upTblInner->addWidget(m_updateTable);
    upLayout->addWidget(upTableCard, 1);
    upLayout->addStretch();

    upScrollArea->setWidget(upScrollContent);
    upOuterLayout->addWidget(upScrollArea);
    m_pages->addWidget(upPage);
    // ====== Page 4: 环境 ======
    auto *envPage = new QWidget();
    auto *envLayout = new QVBoxLayout(envPage);
    envLayout->setContentsMargins(24, 24, 24, 24);
    envLayout->setSpacing(16);
    envLayout->addWidget(createPageHeader("环境", "检测与更新系统开发环境"));

    auto *envTopCard = createCard();
    envTopCard->setProperty("pageTopCard", true);
    auto *envTopInner = static_cast<QVBoxLayout*>(envTopCard->layout());
    auto *envTopTitle = new QLabel("环境维护");
    envTopTitle->setObjectName("sectionTitle");
    auto *envTopDesc = new QLabel("扫描本机工具链并对比最新版本，适合做一键巡检与按项更新");
    envTopDesc->setObjectName("helperText");
    envTopInner->addWidget(envTopTitle);
    envTopInner->addWidget(envTopDesc);
    auto *envTopRow = new QHBoxLayout();
    m_detectEnvBtn = new QPushButton("检测环境");
    m_detectEnvBtn->setObjectName("primaryBtn");
    m_detectEnvBtn->setIcon(loadSvgIcon("search"));
    m_checkLatestBtn = new QPushButton("检查最新版本");
    m_checkLatestBtn->setObjectName("secondaryBtn");
    m_checkLatestBtn->setIcon(loadSvgIcon("refresh-cw"));
    m_updateEnvBtn = new QPushButton("更新选中");
    m_updateEnvBtn->setObjectName("secondaryBtn");
    m_updateEnvBtn->setIcon(loadSvgIcon("download-cloud"));
    m_updateEnvBtn->setEnabled(false);
    envTopRow->addWidget(m_detectEnvBtn);
    envTopRow->addWidget(m_checkLatestBtn);
    envTopRow->addWidget(m_updateEnvBtn);
    envTopRow->addStretch();
    envTopInner->addLayout(envTopRow);
    envLayout->addWidget(envTopCard);

    auto *envTableCard = createCard();
    auto *envTblInner = static_cast<QVBoxLayout*>(envTableCard->layout());
    m_envEmptyState = new QLabel("尚未检测到环境信息\n点击“检测环境”开始扫描");
    m_envEmptyState->setObjectName("emptyState");
    m_envEmptyState->setAlignment(Qt::AlignCenter);
    m_envEmptyState->setVisible(false);
    envTblInner->addWidget(m_envEmptyState);
    m_envTable = new QTableWidget();
    m_envTable->setColumnCount(6);
    m_envTable->setHorizontalHeaderLabels({"环境", "路径", "当前版本", "最新版本", "状态", "操作"});
    m_envTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_envTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_envTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_envTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_envTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_envTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    m_envTable->horizontalHeader()->setMinimumSectionSize(72);
    m_envTable->setColumnWidth(5, 132);
    m_envTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_envTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_envTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_envTable->setWordWrap(false);
    m_envTable->setTextElideMode(Qt::ElideMiddle);
    m_envTable->setMouseTracking(true);
    m_envTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_envTable->horizontalHeader()->setHighlightSections(false);
    m_envTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_envTable->verticalHeader()->setVisible(false);
    m_envTable->verticalHeader()->setDefaultSectionSize(48);
    m_envTable->setShowGrid(false);
    m_envTable->setAlternatingRowColors(true);
    m_envTable->setColumnWidth(5, 132);
    m_envTable->setItemDelegateForColumn(5, new TableActionDelegate(m_envTable));
    envTblInner->addWidget(m_envTable);
    envLayout->addWidget(envTableCard, 1);
    m_pages->addWidget(envPage);

    // ====== Page 5: 设置 ======
    auto *stPage = new QWidget();
    auto *stOuterLayout = new QVBoxLayout(stPage);
    stOuterLayout->setContentsMargins(0, 0, 0, 0);
    auto *stScroll = new QScrollArea();
    stScroll->setWidgetResizable(true);
    stScroll->setFrameShape(QFrame::NoFrame);
    auto *stInner = new QWidget();
    stInner->setMinimumWidth(500);
    auto *stLayout = new QVBoxLayout(stInner);
    stLayout->setContentsMargins(24, 24, 24, 24);
    stLayout->setSpacing(16);
    stLayout->addWidget(createPageHeader("设置", "应用偏好与行为配置"));

    // 基础偏好
    auto *baseCard = createCard();
    baseCard->setProperty("pageTopCard", true);
    auto *baseInner = static_cast<QVBoxLayout*>(baseCard->layout());
    baseInner->setSpacing(12);
    auto *baseHeader = new QHBoxLayout();
    baseHeader->setSpacing(8);
    auto *baseIcon = new QLabel();
    baseIcon->setPixmap(loadSvgIcon("sun").pixmap(18, 18));
    baseIcon->setFixedSize(22, 22);
    auto *baseTitle = new QLabel("基础偏好");
    baseTitle->setObjectName("sectionTitle");
    baseHeader->addWidget(baseIcon);
    baseHeader->addWidget(baseTitle);
    baseHeader->addStretch();
    baseInner->addLayout(baseHeader);
    auto *baseDesc = new QLabel("统一管理主题、启动行为与常用应用偏好");
    baseDesc->setObjectName("helperText");
    baseInner->addWidget(baseDesc);

    auto *themeRow = new QHBoxLayout();
    themeRow->setSpacing(12);
    auto *themeLabel = new QLabel("界面主题");
    themeLabel->setObjectName("fieldLabel");
    themeLabel->setFixedWidth(80);
    m_themeCombo = new QComboBox();
    m_themeCombo->addItems({"跟随系统", "浅色", "深色"});
    m_themeCombo->setMinimumWidth(160);
    themeRow->addWidget(themeLabel);
    themeRow->addWidget(m_themeCombo);
    themeRow->addStretch();
    baseInner->addLayout(themeRow);

    auto *autoRow = new QHBoxLayout();
    autoRow->setSpacing(12);
    auto *autoLabel = new QLabel("开机自启");
    autoLabel->setObjectName("fieldLabel");
    autoLabel->setFixedWidth(80);
    m_autoStartCheck = new QCheckBox("登录 Windows 时自动启动");
    autoRow->addWidget(autoLabel);
    autoRow->addWidget(m_autoStartCheck);
    autoRow->addStretch();
    baseInner->addLayout(autoRow);
    stLayout->addWidget(baseCard);

    // 智能拉起
    auto *smartCard = createCard();
    auto *smartInner = static_cast<QVBoxLayout*>(smartCard->layout());
    smartInner->setSpacing(12);
    auto *smartHeader = new QHBoxLayout();
    smartHeader->setSpacing(8);
    auto *smartIcon = new QLabel();
    smartIcon->setPixmap(loadSvgIcon("shield-check").pixmap(18, 18));
    smartIcon->setFixedSize(22, 22);
    auto *smartTitle = new QLabel("智能拉起策略");
    smartTitle->setObjectName("sectionTitle");
    smartHeader->addWidget(smartIcon);
    smartHeader->addWidget(smartTitle);
    smartHeader->addStretch();
    smartInner->addLayout(smartHeader);
    auto *smartDesc = new QLabel("当 CPU 或内存占用超过阈值时，守护进程会延迟拉起，待系统空闲后自动恢复");
    smartDesc->setObjectName("helperText");
    smartInner->addWidget(smartDesc);

    auto *thresholdRow = new QHBoxLayout();
    thresholdRow->setSpacing(24);
    auto *cpuGroup = new QHBoxLayout();
    cpuGroup->setSpacing(8);
    auto *cpuLabel = new QLabel("CPU 阈值");
    cpuLabel->setObjectName("fieldLabel");
    m_cpuSpin = new QSpinBox();
    m_cpuSpin->setSuffix("%");
    m_cpuSpin->setRange(10, 100);
    m_cpuSpin->setFixedWidth(90);
    m_cpuSpin->setValue(AppSettings.smartGuardCpuThreshold());
    cpuGroup->addWidget(cpuLabel);
    cpuGroup->addWidget(m_cpuSpin);
    thresholdRow->addLayout(cpuGroup);
    auto *memGroup = new QHBoxLayout();
    memGroup->setSpacing(8);
    auto *memLabel = new QLabel("内存阈值");
    memLabel->setObjectName("fieldLabel");
    m_memSpin = new QSpinBox();
    m_memSpin->setSuffix("%");
    m_memSpin->setRange(10, 100);
    m_memSpin->setFixedWidth(90);
    m_memSpin->setValue(AppSettings.smartGuardMemThreshold());
    memGroup->addWidget(memLabel);
    memGroup->addWidget(m_memSpin);
    thresholdRow->addLayout(memGroup);
    thresholdRow->addStretch();
    smartInner->addLayout(thresholdRow);
    stLayout->addWidget(smartCard);

    // 更新与网络
    auto *tokenCard = createCard();
    auto *tokenInner = static_cast<QVBoxLayout*>(tokenCard->layout());
    tokenInner->setSpacing(12);
    auto *tokenHeader = new QHBoxLayout();
    tokenHeader->setSpacing(8);
    auto *tokenIcon = new QLabel();
    tokenIcon->setPixmap(loadSvgIcon("settings").pixmap(18, 18));
    tokenIcon->setFixedSize(22, 22);
    auto *tokenTitle = new QLabel("更新与网络");
    tokenTitle->setObjectName("sectionTitle");
    tokenHeader->addWidget(tokenIcon);
    tokenHeader->addWidget(tokenTitle);
    tokenHeader->addStretch();
    tokenInner->addLayout(tokenHeader);
    auto *tokenDesc = new QLabel("GitHub Token 用于缓解 API 限流，仅在检查与拉取更新时使用");
    tokenDesc->setObjectName("helperText");
    tokenInner->addWidget(tokenDesc);

    auto *tokenInputRow = new QHBoxLayout();
    tokenInputRow->setSpacing(8);
    m_githubTokenEdit = new QLineEdit();
    m_githubTokenEdit->setPlaceholderText("ghp_... 或 ghs_...");
    m_githubTokenEdit->setEchoMode(QLineEdit::Password);
    auto *toggleEchoBtn = new QPushButton();
    toggleEchoBtn->setObjectName("secondaryBtn");
    toggleEchoBtn->setIcon(loadSvgIcon("circle-dot"));
    toggleEchoBtn->setFixedSize(36, 36);
    toggleEchoBtn->setToolTip("显示 / 隐藏 Token");
    connect(toggleEchoBtn, &QPushButton::clicked, this, [this, toggleEchoBtn]() {
        if (m_githubTokenEdit->echoMode() == QLineEdit::Password) {
            m_githubTokenEdit->setEchoMode(QLineEdit::Normal);
            toggleEchoBtn->setIcon(loadSvgIcon("circle"));
        } else {
            m_githubTokenEdit->setEchoMode(QLineEdit::Password);
            toggleEchoBtn->setIcon(loadSvgIcon("circle-dot"));
        }
    });
    tokenInputRow->addWidget(m_githubTokenEdit, 1);
    tokenInputRow->addWidget(toggleEchoBtn);
    tokenInner->addLayout(tokenInputRow);

    auto *tokenBtnRow = new QHBoxLayout();
    tokenBtnRow->setSpacing(8);
    auto *saveTokenBtn = new QPushButton("保存 Token");
    saveTokenBtn->setObjectName("primaryBtn");
    auto *testTokenBtn = new QPushButton("测试连接");
    testTokenBtn->setObjectName("secondaryBtn");
    testTokenBtn->setIcon(loadSvgIcon("refresh-cw"));
    tokenBtnRow->addWidget(saveTokenBtn);
    tokenBtnRow->addWidget(testTokenBtn);
    tokenBtnRow->addStretch();
    tokenInner->addLayout(tokenBtnRow);

    connect(saveTokenBtn, &QPushButton::clicked, this, [this]() {
        QString token = m_githubTokenEdit->text().trimmed();
        if (token.isEmpty()) {
            QMessageBox::warning(this, "提示", "请输入有效的 GitHub Token");
            return;
        }
        AppSettings.setGithubToken(token);
        updateStatusBar("GitHub Token 已保存");
        QMessageBox::information(this, "成功", "GitHub Token 已保存到应用设置。");
    });

    connect(testTokenBtn, &QPushButton::clicked, this, [this]() {
        updateStatusBar("正在测试 GitHub Token...");
        m_updater->fetchReleases();
    });

    stLayout->addWidget(tokenCard);

    // 关于
    auto *aboutCard = createCard();
    auto *aboutInner = static_cast<QVBoxLayout*>(aboutCard->layout());
    aboutInner->setSpacing(10);
    auto *aboutHeader = new QHBoxLayout();
    aboutHeader->setSpacing(8);
    auto *aboutIcon = new QLabel();
    aboutIcon->setPixmap(loadSvgIcon("list").pixmap(18, 18));
    aboutIcon->setFixedSize(22, 22);
    auto *aboutTitle = new QLabel("关于");
    aboutTitle->setObjectName("sectionTitle");
    aboutHeader->addWidget(aboutIcon);
    aboutHeader->addWidget(aboutTitle);
    aboutHeader->addStretch();
    aboutInner->addLayout(aboutHeader);

    auto *aboutRow = new QHBoxLayout();
    aboutRow->setSpacing(12);
    auto *verLabel = new QLabel(QStringLiteral("版本 1.0.0"));
    verLabel->setObjectName("helperText");
    aboutRow->addWidget(verLabel);
    aboutRow->addStretch();
    auto *githubLink = new QPushButton("GitHub 项目");
    githubLink->setObjectName("secondaryBtn");
    githubLink->setIcon(loadSvgIcon("external-link"));
    connect(githubLink, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/MCheng404/OpenclawGuard"));
    });
    aboutRow->addWidget(githubLink);
    aboutInner->addLayout(aboutRow);
    stLayout->addWidget(aboutCard);

    stLayout->addStretch();
    stScroll->setWidget(stInner);
    stOuterLayout->addWidget(stScroll);
    m_pages->addWidget(stPage);
}

void MainWindow::setupUI()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);
    central->setContentsMargins(0, 0, 0, 0);

    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // 主体: 侧边栏 + 页面
    auto *bodyRow = new QHBoxLayout();
    bodyRow->setContentsMargins(0, 0, 0, 0);
    bodyRow->setSpacing(0);

    setupSidebar(bodyRow);
    setupPages();
    bodyRow->addWidget(m_pages, 1);
    root->addLayout(bodyRow, 1);

    // 状态栏
    auto *statusBar = new QStatusBar();
    m_statusMsg = new QLabel("就绪");
    m_statusIndicator = new QLabel();
    m_statusIndicator->setFixedSize(10, 10);
    m_statusIndicator->setStyleSheet(QString("background: %1; border-radius: 5px;")
        .arg(Theme::currentColors().textSecondary.name()));
    m_statusPort = new QLabel("");
    m_statusPort->setStyleSheet("color: #8b8fa3; font-size: 11px;");
    statusBar->addWidget(m_statusIndicator);
    statusBar->addWidget(m_statusMsg, 1);
    statusBar->addPermanentWidget(m_statusPort);
    root->addWidget(statusBar);
}

void MainWindow::setupConnections()
{
    m_tray = new TrayManager(this, this);
    connect(m_tray, &TrayManager::showWindowRequested, this, [this]() {
        show(); raise(); activateWindow();
    });
    connect(m_tray, &TrayManager::quitRequested, qApp, &QApplication::quit);
    connect(m_tray, &TrayManager::restartGatewayRequested,
            this, &MainWindow::onRestartGateway);

    // 导航
    connect(m_navGroup, &QButtonGroup::idClicked, this, &MainWindow::onNavigate);

    // 主题切换
    connect(m_themeBtn, &QPushButton::clicked, this, &MainWindow::onToggleTheme);

    // 网关
    connect(m_startStopBtn, &QPushButton::clicked, this, &MainWindow::onStartStopGateway);
    connect(m_restartBtn, &QPushButton::clicked, this, &MainWindow::onRestartGateway);
    connect(m_portSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onPortChanged);
    connect(m_gateway->monitor(), &PortMonitor::onlineChanged,
            this, &MainWindow::onGatewayOnlineChanged);
    connect(m_gateway, &GatewayManager::gatewayStarted, this, [this]() {
        updateStatusBar("网关已启动");
        m_tray->updateGatewayStatus(true, QString::number(m_gateway->port()));
    });
    connect(m_gateway, &GatewayManager::gatewayStopped, this, [this]() {
        updateStatusBar("网关已停止");
        m_tray->updateGatewayStatus(false);
    });
    connect(m_gateway, &GatewayManager::gatewayCrashed,
            this, &MainWindow::onGatewayCrashed);

    // 守护
    connect(m_addGuardBtn, &QPushButton::clicked, this, &MainWindow::onAddGuardItem);
    connect(m_pickProcessBtn, &QPushButton::clicked, this, &MainWindow::onPickFromProcess);
    connect(m_removeGuardBtn, &QPushButton::clicked, this, &MainWindow::onRemoveGuardItem);
    connect(m_guardTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        m_removeGuardBtn->setEnabled(m_guardTable->currentRow() >= 0);
        updateGuardSelectionDetails();
    });
    connect(m_guardTable, &QTableWidget::cellClicked, this, [this](int row, int column) {
        if (column != 3) return;
        auto list = AppSettings.guardList();
        if (row < 0 || row >= list.size()) return;
        list[row].enabled = !list[row].enabled;
        AppSettings.setGuardList(list);
        m_guard->setItemEnabled(list[row].name, list[row].enabled);
        auto *statusItem = m_guardTable->item(row, 2);
        if (statusItem) {
            statusItem->setText(list[row].enabled ? "待监控" : "已停用");
            setSemanticItemStyle(statusItem, list[row].enabled ? "info" : "danger");
        }
        auto *actionItem = m_guardTable->item(row, 3);
        if (actionItem) {
            actionItem->setData(Qt::UserRole + 1, list[row].enabled);
            actionItem->setToolTip(list[row].enabled ? "点击停用守护" : "点击启用守护");
        }
        m_guardTable->viewport()->update();
    });
    connect(m_guardCopyPathBtn, &QPushButton::clicked, this, [this]() {
        const int row = m_guardTable->currentRow();
        if (row < 0 || !m_guardTable->item(row, 1)) return;
        QGuiApplication::clipboard()->setText(m_guardTable->item(row, 1)->text());
        updateStatusBar("已复制守护路径");
    });
    connect(m_envTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        const int row = m_envTable->currentRow();
        m_updateEnvBtn->setEnabled(row >= 0);
    });
    connect(m_envTable, &QTableWidget::cellClicked, this, [this](int row, int column) {
        if (column != 5) return;
        if (row < 0 || row >= m_envTable->rowCount()) return;
        auto *item = m_envTable->item(row, 0);
        auto *actionItem = m_envTable->item(row, 5);
        if (!item || !actionItem || !actionItem->data(Qt::UserRole + 1).toBool()) return;
        onEnvUpdate(item->text());
    });
    connect(m_guard, &ProcessGuard::statusChanged,
            this, &MainWindow::onGuardStatusChanged);

    // 更新
    connect(m_fetchUpdateBtn, &QPushButton::clicked, this, &MainWindow::onFetchUpdates);
    connect(m_cliUpdateBtn, &QPushButton::clicked, this, [this]() {
        QString ch = m_channelCombo->currentText();
        if (ch == "当前通道") ch = "latest";
        m_ocDryRunOutput->setPlainText("正在通过 npm 执行 Openclaw 更新...");
        updateStatusBar("正在通过 npm 更新 Openclaw...");
        m_updater->performOpenclawUpdate(ch);
    });
    connect(m_channelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUpdateChannelChanged);
    connect(m_updater, &UpdateManager::releasesReady, this, [this]() {
        refreshUpdateTable();
        updateStatusBar("Release 列表已刷新");
    });
    connect(m_updater, &UpdateManager::fetchError, this, [this](const QString &msg) {
        updateStatusBar("更新检查失败: " + msg);
        m_ocDryRunOutput->setPlainText(QString("错误:\n%1").arg(msg));
    });
    // CLI 更新信号
    connect(m_updater, &UpdateManager::openclawUpdateStatus, this, [this](const QString &statusJson) {
        QJsonParseError parseError{};
        QJsonDocument doc = QJsonDocument::fromJson(statusJson.toUtf8(), &parseError);
        if (!doc.isObject()) {
            m_ocDryRunOutput->setPlainText(QString("状态 JSON 解析失败: %1\n\n%2")
                .arg(parseError.errorString(), statusJson));
            return;
        }

        QJsonObject root = doc.object();
        QJsonObject upd = root["update"].toObject();
        QJsonObject chObj = root["channel"].toObject();
        QJsonObject avail = root["availability"].toObject();

        const QString version = m_updater->getCurrentVersion();
        const QString channelValue = chObj["value"].toString("unknown");
        const QString channelSource = chObj["source"].toString("unknown");
        const QString installKind = upd["installKind"].toString("unknown");
        const QString packageManager = upd["packageManager"].toString("unknown");
        const bool available = avail["available"].toBool(false);

        m_ocVersionLabel->setText(version.isEmpty() ? "unknown" : version);
        m_ocChannelLabel->setText(QString("%1 (%2)").arg(channelValue, channelSource));
        m_ocInstallLabel->setText(QString("%1 (%2)").arg(installKind, packageManager));
        m_ocAvailableLabel->setText(available ? "有可用更新" : "已是最新");
        m_ocAvailableLabel->setStyleSheet(available
            ? "color: #f59e0b; font-weight: 600;"
            : "color: #34d399; font-weight: 600;");
    });
    connect(m_updater, &UpdateManager::openclawUpdateFinished, this,
            [this](bool success, const QString &msg) {
        if (success) {
            m_ocDryRunOutput->setPlainText(QString("npm 更新完成:\n%1").arg(msg));
            updateStatusBar("Openclaw 更新完成");
            // 更新后刷新状态
            m_updater->getUpdateStatus();
        } else {
            m_ocDryRunOutput->setPlainText(QString("更新失败:\n%1").arg(msg));
            updateStatusBar("Openclaw 更新失败");
        }
    });
    connect(m_updater, &UpdateManager::openclawUpdateProgress, this,
            [this](const QString &msg) {
        updateStatusBar(msg);
    });

    // 环境
    connect(m_detectEnvBtn, &QPushButton::clicked, this, &MainWindow::onEnvDetect);
    connect(m_checkLatestBtn, &QPushButton::clicked, this, [this]() {
        m_checkLatestBtn->setEnabled(false);
        m_checkLatestBtn->setText("检查中...");
        updateStatusBar("正在联网检查最新版本...");
        m_envMgr->checkLatestVersions();
    });
    connect(m_updateEnvBtn, &QPushButton::clicked, this, [this]() {
        int row = m_envTable->currentRow();
        if (row < 0) return;
        QString name = m_envTable->item(row, 0)->text();
        onEnvUpdate(name);
    });
    connect(m_envMgr, &EnvironmentManager::detectionFinished,
            this, &MainWindow::onEnvDetectionFinished);
    connect(m_envMgr, &EnvironmentManager::latestVersionChecked,
            this, &MainWindow::onLatestVersionChecked);
    connect(m_envMgr, &EnvironmentManager::allLatestChecked, this, [this]() {
        m_checkLatestBtn->setEnabled(true);
        m_checkLatestBtn->setText("检查最新版本");
        updateStatusBar("最新版本检查完成");
    });
    connect(m_envMgr, &EnvironmentManager::updateFinished,
            this, &MainWindow::onEnvUpdateFinished);
    connect(m_envMgr, &EnvironmentManager::updateStarted, this, [this](const QString &name) {
        m_updateEnvBtn->setEnabled(false);
        m_updateEnvBtn->setText("更新中...");
    });
    connect(m_envMgr, &EnvironmentManager::updateProgress, this, [this](const QString &name, const QString &line) {
        updateStatusBar(QString("[%1] %2").arg(name, line.left(80)));
    });

    // 主题下拉框
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onThemeChanged);

    // 开机自启
    connect(m_autoStartCheck, &QCheckBox::toggled, this, [this](bool on) {
        AppSettings.setAutoStart(on);
#ifdef Q_OS_WIN
        QSettings reg(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)",
                      QSettings::NativeFormat);
        if (on) reg.setValue("OpenclawGuard",
                             QDir::toNativeSeparators(QApplication::applicationFilePath()));
        else    reg.remove("OpenclawGuard");
#endif
    });

    // 智能拉起阈值
    connect(m_cpuSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
        AppSettings.setSmartGuardCpuThreshold(v);
        m_guard->setCpuThreshold(v);
    });
    connect(m_memSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
        AppSettings.setSmartGuardMemThreshold(v);
        m_guard->setMemThreshold(v);
    });

    // 延迟拉起通知
    connect(m_guard, &ProcessGuard::restartDeferred, this, [this](const QString &name, const QString &reason) {
        updateStatusBar(QString("%1: %2").arg(name, reason));
    });
}

// ====================== Show ======================
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    static bool firstShow = true;
    if (firstShow) {
        firstShow = false;
        Theme::enableMica(winId());
        // 延迟执行，让窗口先渲染出来
        QTimer::singleShot(150, this, [this]() {
            m_envMgr->detectAll();
            m_envMgr->checkLatestVersions();
        });
    }
}

// ====================== 导航 ======================

void MainWindow::onNavigate(int pageIndex)
{
    m_pages->setCurrentIndex(pageIndex);
    updateSidebarContext(pageIndex);
}

void MainWindow::onToggleTheme()
{
    int cur = m_themeCombo->currentIndex();
    if (cur == 2) { // 当前深色 → 浅色
        m_themeCombo->setCurrentIndex(1);
        m_themeBtn->setIcon(loadSvgIcon("sun"));
    } else { // 当前浅色或系统 → 深色
        m_themeCombo->setCurrentIndex(2);
        m_themeBtn->setIcon(loadSvgIcon("moon"));
    }
}

// ====================== 网关 Slots ======================

void MainWindow::onStartStopGateway()
{
    if (m_gateway->isGatewayRunning()) {
        m_gateway->stopGateway();
        updateStatusBar("网关已停止");
    } else {
        m_gateway->startGateway();
        updateStatusBar("正在启动网关...");
    }
}

void MainWindow::onRestartGateway()
{
    m_gateway->restartGateway();
    updateStatusBar("网关重启中...");
}

void MainWindow::onPortChanged(int port)
{
    m_gateway->setPort(port);
    AppSettings.setGatewayPort(port);
    m_portStatusLabel->setText(QString("监听端口 %1").arg(port));
    setStatusBadgeStyle(m_portStatusLabel, Theme::currentColors().accent);
    if (m_dashPortValue)
        m_dashPortValue->setText(QString("监听端口 %1").arg(port));
}

void MainWindow::onGatewayOnlineChanged(bool online)
{
    const auto offlineColor = Theme::currentColors().textSecondary.name();
    const QColor badgeColor = online ? QColor("#34d399") : QColor("#ef4444");

    m_portStatusLabel->setText(QString("监听端口 %1 · %2").arg(m_gateway->port()).arg(online ? "在线" : "离线"));
    setStatusBadgeStyle(m_portStatusLabel, Theme::currentColors().accent);
    m_statusPort->setText(QString("端口 %1").arg(m_gateway->port()));
    if (m_dashPortValue)
        m_dashPortValue->setText(QString("监听端口 %1 · %2").arg(m_gateway->port()).arg(online ? "在线" : "离线"));
    m_dashGwValue->setText(online ? "在线" : "离线");
    m_gatewayStatusLabel->setText(online ? "网关在线" : "网关离线");
    setStatusBadgeStyle(m_gatewayStatusLabel, badgeColor);
    m_startStopBtn->setText(online ? "停止网关" : "启动网关");
    m_startStopBtn->setIcon(online ? loadSvgIcon("square") : loadSvgIcon("play"));

    m_statusIndicator->setStyleSheet(QString("background: %1; border-radius: 5px;")
        .arg(online ? "#34d399" : offlineColor));
    m_statusIndicator->setGraphicsEffect(online ? m_pulseOpacity : nullptr);

    if (online) {
        if (m_pulseAnim->state() != QAbstractAnimation::Running) {
            m_pulseAnim->setLoopCount(-1);
            m_pulseAnim->start();
        }
    } else {
        m_pulseAnim->stop();
        m_pulseOpacity->setOpacity(1.0);
    }

    if (m_dashGwStatus) {
        m_dashGwStatus->setStyleSheet(QString("background: %1; border-radius: 5px;")
            .arg(online ? "#34d399" : offlineColor));
        m_dashGwStatus->setGraphicsEffect(online ? m_dashPulseOpacity : nullptr);

        if (online) {
            if (m_dashPulseAnim->state() != QAbstractAnimation::Running) {
                m_dashPulseAnim->setLoopCount(-1);
                m_dashPulseAnim->start();
            }
        } else {
            m_dashPulseAnim->stop();
            m_dashPulseOpacity->setOpacity(1.0);
        }
    }

    // 更新托盘状态
    m_tray->updateGatewayStatus(online, QString::number(m_gateway->port()));
}

void MainWindow::onGatewayCrashed()
{
    updateStatusBar("网关崩溃，正在自动重启...");
    m_tray->showMessage("进程管理控制台", "检测到网关异常，正在自动重启...");
}

// ====================== 守护 Slots ======================

void MainWindow::onAddGuardItem()
{
    QString exePath = QFileDialog::getOpenFileName(this, "选择要守护的程序",
                        QString(), "可执行文件 (*.exe)");
    if (exePath.isEmpty()) return;

    QString name = QFileInfo(exePath).baseName();
    GuardItem item{name, exePath, true};

    auto list = AppSettings.guardList();
    list.append(item);
    AppSettings.setGuardList(list);

    m_guard->addItem(item);
    refreshGuardTable();
    updateStatusBar(QString("已添加守护: %1").arg(name));
    m_dashGuardCount->setText(QString::number(m_guardTable->rowCount()));
}

void MainWindow::onPickFromProcess()
{
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle("从运行进程选取");
    dlg->resize(550, 400);
    auto *dlgLayout = new QVBoxLayout(dlg);

    auto *filterEdit = new QLineEdit();
    filterEdit->setPlaceholderText("输入关键词过滤...");
    dlgLayout->addWidget(filterEdit);

    auto *listWidget = new QListWidget();
    dlgLayout->addWidget(listWidget);

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dlgLayout->addWidget(btnBox);
    connect(btnBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    auto procs = ProcessGuard::listRunningProcesses();
    for (const auto &p : procs) {
        auto *item = new QListWidgetItem(p.first + "  —  " + p.second);
        item->setData(Qt::UserRole, p.second);
        listWidget->addItem(item);
    }

    connect(filterEdit, &QLineEdit::textChanged, this, [listWidget, filterEdit]() {
        QString kw = filterEdit->text().toLower();
        for (int i = 0; i < listWidget->count(); ++i) {
            listWidget->item(i)->setHidden(
                kw.isEmpty() ? false : !listWidget->item(i)->text().toLower().contains(kw));
        }
    });

    if (dlg->exec() == QDialog::Accepted) {
        auto *sel = listWidget->currentItem();
        if (!sel) return;
        QString exePath = sel->data(Qt::UserRole).toString();
        QString name = QFileInfo(exePath).baseName();

        auto list = AppSettings.guardList();
        for (auto &i : list) {
            if (i.exePath == exePath) {
                QMessageBox::information(this, "提示", "该程序已在守护列表中");
                return;
            }
        }

        GuardItem item{name, exePath, true};
        list.append(item);
        AppSettings.setGuardList(list);
        m_guard->addItem(item);
        refreshGuardTable();
        updateStatusBar(QString("已添加守护: %1").arg(name));
        m_dashGuardCount->setText(QString::number(m_guardTable->rowCount()));
    }
    dlg->deleteLater();
}

void MainWindow::onRemoveGuardItem()
{
    int row = m_guardTable->currentRow();
    if (row < 0) return;

    QString name = m_guardTable->item(row, 0)->text();
    m_guard->removeItem(name);

    auto list = AppSettings.guardList();
    list.removeAt(row);
    AppSettings.setGuardList(list);

    refreshGuardTable();
    updateStatusBar(QString("已移除守护: %1").arg(name));
    m_dashGuardCount->setText(QString::number(m_guardTable->rowCount()));
}

void MainWindow::onGuardItemChanged()
{
    auto list = AppSettings.guardList();
    for (int i = 0; i < m_guardTable->rowCount() && i < list.size(); ++i) {
        auto *actionItem = m_guardTable->item(i, 3);
        if (!actionItem)
            continue;
        list[i].enabled = actionItem->data(Qt::UserRole + 1).toBool();
        m_guard->setItemEnabled(list[i].name, list[i].enabled);
    }
    AppSettings.setGuardList(list);
}

void MainWindow::onGuardStatusChanged(const QString &name, bool running)
{
    for (int i = 0; i < m_guardTable->rowCount(); ++i) {
        if (m_guardTable->item(i, 0)->text() == name) {
            auto *statusItem = m_guardTable->item(i, 2);
            statusItem->setText(running ? "运行中" : "已停止");
            setSemanticItemStyle(statusItem, running ? "success" : "danger");
        }
    }
    if (!running)
        updateStatusBar(QString("%1 已停止，尝试自动拉起").arg(name));
}

// ====================== 更新 Slots ======================

void MainWindow::onFetchUpdates()
{
    m_ocDryRunOutput->setPlainText("正在获取 Openclaw 更新状态...");
    updateStatusBar("正在检查 Openclaw 更新状态...");
    m_updater->getUpdateStatus();
    m_updater->fetchReleases();  // 拉取 releases 列表作为参考
}

void MainWindow::onInstallUpdate(const UpdateInfo &info)
{
    auto ret = QMessageBox::question(this, "确认安装",
                QString("确定要下载并安装 %1 吗?").arg(info.version));
    if (ret == QMessageBox::Yes) {
        m_downloadProgress->setVisible(true);
        updateStatusBar(QString("正在下载 %1...").arg(info.version));
        m_updater->downloadAndInstall(info);
    }
}

void MainWindow::onUpdateChannelChanged(int)
{
    refreshUpdateTable();
}

// ====================== 环境 Slots ======================

void MainWindow::onEnvDetect()
{
    updateStatusBar("正在检测系统环境...");
    m_envMgr->detectAll();
}

void MainWindow::onEnvDetectionFinished()
{
    const auto &envs = m_envMgr->allEnvs();
    m_envTable->setRowCount(envs.size());
    m_envTable->verticalHeader()->setDefaultSectionSize(48);
    for (int i = 0; i < envs.size(); ++i) {
        const auto &e = envs[i];
        auto *nameItem = new QTableWidgetItem(e.name);
        auto *pathItem = new QTableWidgetItem(e.exePath);
        pathItem->setToolTip(e.exePath);
        auto *versionItem = new QTableWidgetItem(e.version);
        auto *latestItem = new QTableWidgetItem("待检查");
        auto *statusItem = new QTableWidgetItem(e.installed ? "已安装" : "未安装");
        versionItem->setTextAlignment(Qt::AlignCenter);
        latestItem->setTextAlignment(Qt::AlignCenter);
        setSemanticItemStyle(statusItem, e.installed ? "success" : "danger");
        setSemanticItemStyle(latestItem, "info");
        m_envTable->setItem(i, 0, nameItem);
        m_envTable->setItem(i, 1, pathItem);
        m_envTable->setItem(i, 2, versionItem);
        m_envTable->setItem(i, 3, latestItem);
        m_envTable->setItem(i, 4, statusItem);

        auto *actionItem = new QTableWidgetItem("更新");
        actionItem->setTextAlignment(Qt::AlignCenter);
        actionItem->setData(Qt::UserRole + 1, e.installed && !e.updateCmd.isEmpty());
        actionItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        actionItem->setToolTip(e.installed && !e.updateCmd.isEmpty() ? "更新此环境" : "当前项不可更新");
        m_envTable->setItem(i, 5, actionItem);
    }
    m_envTable->resizeColumnsToContents();
    m_envTable->setColumnWidth(5, 132);
    m_envTable->verticalHeader()->setDefaultSectionSize(48);
    const bool hasRows = !envs.isEmpty();
    m_envTable->setVisible(hasRows);
    if (m_envEmptyState) m_envEmptyState->setVisible(!hasRows);
    m_updateEnvBtn->setEnabled(hasRows && m_envTable->currentRow() >= 0);
    if (m_dashEnvCount)
        m_dashEnvCount->setText(QString::number(envs.size()));
    updateStatusBar("环境检测完成");
}

void MainWindow::onLatestVersionChecked(const QString &name, const QString &latest, bool newer)
{
    for (int i = 0; i < m_envTable->rowCount(); ++i) {
        if (m_envTable->item(i, 0)->text() == name) {
            auto *latestItem = m_envTable->item(i, 3);
            auto *statusItem = m_envTable->item(i, 4);
            latestItem->setText(newer ? latest + " · 可更新" : latest);
            setSemanticItemStyle(latestItem, newer ? "warning" : "success");
            if (statusItem && statusItem->text() != "未安装") {
                statusItem->setText(newer ? "可更新" : "已最新");
                setSemanticItemStyle(statusItem, newer ? "warning" : "success");
            }
            break;
        }
    }
}

void MainWindow::onEnvUpdate(const QString &name)
{
    updateStatusBar(QString("正在更新 %1...").arg(name));
    m_envMgr->updateEnv(name);
}

void MainWindow::onEnvUpdateFinished(const QString &name, bool success, const QString &msg)
{
    m_updateEnvBtn->setEnabled(true);
    m_updateEnvBtn->setText("更新选中");
    if (success) {
        updateStatusBar(QString("%1 更新成功").arg(name));
        QMessageBox::information(this, "更新完成", QString("%1 更新成功").arg(name));
    } else {
        updateStatusBar(QString("%1 更新失败: %2").arg(name, msg));
        QMessageBox::warning(this, "更新失败", msg);
    }
}

// ====================== 主题 ======================

void MainWindow::onThemeChanged(int idx)
{
    QString themes[] = {"system", "light", "dark"};
    QString theme = themes[idx];
    AppSettings.setTheme(theme);

    if (idx == 1)       m_themeBtn->setIcon(loadSvgIcon("sun"));
    else if (idx == 2)  m_themeBtn->setIcon(loadSvgIcon("moon"));
    else {
        QString sys = Theme::detectSystemTheme();
        m_themeBtn->setIcon(sys == "dark" ? loadSvgIcon("moon") : loadSvgIcon("sun"));
    }
    applyTheme();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_WIN
    hide();
    event->ignore();
#else
    event->accept();
#endif
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateResponsiveLayout();
}

// ====================== Helpers ======================

void MainWindow::loadSettings()
{
    int port = AppSettings.gatewayPort();
    m_portSpin->setValue(port);
    m_gateway->setPort(port);
    if (m_dashPortValue)
        m_dashPortValue->setText(QString("监听端口 %1").arg(port));

    QString theme = AppSettings.theme();
    int themeIdx = 0;
    if (theme == "light") themeIdx = 1;
    else if (theme == "dark") themeIdx = 2;
    m_themeCombo->setCurrentIndex(themeIdx);

    m_autoStartCheck->setChecked(AppSettings.autoStart());

    m_guard->setCpuThreshold(AppSettings.smartGuardCpuThreshold());
    m_guard->setMemThreshold(AppSettings.smartGuardMemThreshold());

    auto list = AppSettings.guardList();
    m_guard->setGuardItems(list);
    refreshGuardTable();

    m_dashGuardCount->setText(QString::number(list.size()));
    if (m_dashEnvCount)
        m_dashEnvCount->setText("0");

    if (m_sidebarThemeLabel) {
        if (themeIdx == 0) m_sidebarThemeLabel->setText(QString("跟随系统 · %1").arg(Theme::detectSystemTheme() == "dark" ? "深色" : "浅色"));
        else if (themeIdx == 1) m_sidebarThemeLabel->setText("浅色模式");
        else m_sidebarThemeLabel->setText("深色模式");
    }

    // GitHub Token
    QString savedToken = AppSettings.githubToken();
    if (!savedToken.isEmpty())
        m_githubTokenEdit->setText(savedToken);
}

void MainWindow::saveSettings() {}

void MainWindow::applyTheme()
{
    int idx = m_themeCombo->currentIndex();
    QString themes[] = {"system", "light", "dark"};

    //   切换主题时临时关闭绘制，避免半透明窗口和 DWM 背景冲突
    setUpdatesEnabled(false);

    //   先清除 DWM Acrylic 背景
    Theme::disableMica(winId());

    Theme::applyTheme(themes[idx]);

    //   重新应用 Mica/Acrylic
    Theme::enableMica(winId());

    auto cs = Theme::currentColors();
    if (m_guardTable)
        m_guardTable->viewport()->update();

    setStatusBadgeStyle(m_portStatusLabel, cs.accent);
    const bool gatewayOnline = m_gatewayStatusLabel && m_gatewayStatusLabel->text().contains("在线");
    setStatusBadgeStyle(m_gatewayStatusLabel, gatewayOnline ? QColor("#34d399") : QColor("#ef4444"));

    const QString effectiveTheme = (idx == 0) ? Theme::detectSystemTheme() : themes[idx];
    if (m_themeBtn)
        m_themeBtn->setIcon(loadSvgIcon(effectiveTheme == "dark" ? "moon" : "sun"));
    if (m_sidebarThemeLabel) {
        if (idx == 0) m_sidebarThemeLabel->setText(QString("跟随系统 · %1").arg(effectiveTheme == "dark" ? "深色" : "浅色"));
        else if (idx == 1) m_sidebarThemeLabel->setText("浅色模式");
        else m_sidebarThemeLabel->setText("深色模式");
    }

    //   解冻 + 强制全量重绘
    setUpdatesEnabled(true);
    repaint();
}

void MainWindow::updateSidebarContext(int pageIndex)
{
    if (!m_sidebarPageLabel || !m_sidebarPageHint)
        return;

    struct SidebarPageMeta {
        const char *title;
        const char *hint;
    };

    static const SidebarPageMeta pages[] = {
        {"仪表盘", "查看系统概览、最近事件与关键状态摘要"},
        {"网关管理", "配置 OpenClaw 网关端口与运行控制"},
        {"进程", "维护守护列表并控制异常进程自动拉起"},
        {"更新", "按通道查看发布版本并执行安装更新"},
        {"环境", "扫描本机工具链、对比版本并按项更新"},
        {"设置中心", "调整主题、智能拉起与更新网络策略"}
    };

    const int maxIndex = static_cast<int>(sizeof(pages) / sizeof(pages[0])) - 1;
    const int safeIndex = qBound(0, pageIndex, maxIndex);
    m_sidebarPageLabel->setText(QString::fromUtf8(pages[safeIndex].title));
    m_sidebarPageHint->setText(QString::fromUtf8(pages[safeIndex].hint));
}

void MainWindow::updateResponsiveLayout()
{
    if (!m_sidebar || !m_logoLabel || !m_sidebarTagline || !m_sidebarPageHint || !m_sidebarContextCard || !m_sidebarBrandCard)
        return;

    const bool compact = width() < 1120;
    m_sidebarCompact = compact;
    m_sidebar->setFixedWidth(compact ? 208 : 236);
    m_logoLabel->setText(compact ? "进程管理" : "进程管理控制台");
    m_sidebarTagline->setVisible(!compact);
    m_sidebarPageHint->setVisible(!compact);
    m_sidebarContextCard->setVisible(true);
    m_sidebarBrandCard->setProperty("compact", compact);
    m_sidebarContextCard->setProperty("compact", compact);
    m_sidebarBrandCard->style()->unpolish(m_sidebarBrandCard);
    m_sidebarBrandCard->style()->polish(m_sidebarBrandCard);
    m_sidebarContextCard->style()->unpolish(m_sidebarContextCard);
    m_sidebarContextCard->style()->polish(m_sidebarContextCard);
}

void MainWindow::updateGuardSelectionDetails()
{
    if (!m_guardSelectionPath || !m_guardCopyPathBtn || !m_guardTable)
        return;

    const int row = m_guardTable->currentRow();
    if (row < 0 || !m_guardTable->item(row, 1)) {
        m_guardSelectionPath->setText("未选择守护项");
        m_guardCopyPathBtn->setEnabled(false);
        return;
    }

    const QString path = m_guardTable->item(row, 1)->text();
    m_guardSelectionPath->setText(path);
    m_guardSelectionPath->setToolTip(path);
    m_guardCopyPathBtn->setEnabled(!path.isEmpty());
}

void MainWindow::refreshGuardTable()
{
    m_guardTable->setRowCount(0);
    auto list = AppSettings.guardList();
    m_guardTable->setRowCount(list.size());
    m_guardTable->verticalHeader()->setDefaultSectionSize(48);
    for (int i = 0; i < list.size(); ++i) {
        auto &item = list[i];
        m_guardTable->setItem(i, 0, new QTableWidgetItem(item.name));
        auto *pathItem = new QTableWidgetItem(item.exePath);
        pathItem->setToolTip(item.exePath);
        pathItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_guardTable->setItem(i, 1, pathItem);

        auto *statusItem = new QTableWidgetItem(item.enabled ? "待监控" : "已停用");
        setSemanticItemStyle(statusItem, item.enabled ? "info" : "danger");
        m_guardTable->setItem(i, 2, statusItem);

        auto *actionItem = new QTableWidgetItem(item.enabled ? "已启用" : "已停用");
        actionItem->setTextAlignment(Qt::AlignCenter);
        actionItem->setData(Qt::UserRole + 1, item.enabled);
        actionItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        actionItem->setToolTip(item.enabled ? "点击停用守护" : "点击启用守护");
        m_guardTable->setItem(i, 3, actionItem);
    }
    m_guardTable->resizeColumnsToContents();
    m_guardTable->setColumnWidth(1, qMax(320, m_guardTable->viewport()->width() - 300));
    m_guardTable->setColumnWidth(3, 132);
    m_guardTable->verticalHeader()->setDefaultSectionSize(48);
    const bool hasRows = !list.isEmpty();
    m_guardTable->setVisible(hasRows);
    if (m_guardEmptyState) m_guardEmptyState->setVisible(!hasRows);
    m_removeGuardBtn->setEnabled(hasRows && m_guardTable->currentRow() >= 0);
    m_dashGuardCount->setText(QString::number(list.size()));
    updateGuardSelectionDetails();
}

void MainWindow::refreshUpdateTable()
{
    m_updateTable->setRowCount(0);
    const QList<UpdateInfo> *list = nullptr;
    const QString chText = m_channelCombo->currentText();
    const int chIdx = m_channelCombo->currentIndex();
    if (chText == "stable" || chIdx == 1)
        list = &m_updater->stableReleases();
    else if (chText == "beta" || chIdx == 2)
        list = &m_updater->betaReleases();
    else
        list = &m_updater->allReleases();

    m_updateTable->setRowCount(list->size());
    m_updateTable->verticalHeader()->setDefaultSectionSize(40);
    for (int i = 0; i < list->size(); ++i) {
        auto &info = list->at(i);
        m_updateTable->setItem(i, 0, new QTableWidgetItem(info.version));
        m_updateTable->setItem(i, 1, new QTableWidgetItem(info.name));
        m_updateTable->setItem(i, 2, new QTableWidgetItem(
            info.prerelease ? "Beta" : "Stable"));
        m_updateTable->setItem(i, 3, new QTableWidgetItem(info.publishedAt));
    }
    m_updateTable->resizeColumnsToContents();
    m_updateTable->verticalHeader()->setDefaultSectionSize(40);
    const bool hasRows = !list->isEmpty();
    m_updateTable->setVisible(hasRows);
    if (m_updateEmptyState) m_updateEmptyState->setVisible(!hasRows);
    m_dashUpdateCount->setText(QString::number(list->size()));
}

void MainWindow::updateStatusBar(const QString &msg)
{
    m_statusMsg->setText(msg);
    appendLog(msg);
}

void MainWindow::appendLog(const QString &msg)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logEntries.prepend({time, msg});
    if (m_logEntries.size() > 50) {
        m_logEntries.removeLast();
    }

    if (!m_activityList)
        return;

    if (m_activityList->count() == 1) {
        auto *first = m_activityList->item(0);
        if (first && first->data(Qt::UserRole).toString() == "placeholder") {
            delete m_activityList->takeItem(0);
        }
    }

    auto *item = new QListWidgetItem(QString("%1  %2").arg(time, msg));
    item->setToolTip(msg);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    m_activityList->insertItem(0, item);
    while (m_activityList->count() > 50) {
        delete m_activityList->takeItem(m_activityList->count() - 1);
    }
}
