#include "theme.h"
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>
#include <QPalette>
#include <QWindow>
#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_MICA
#define DWMWA_MICA 1029
#endif
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#ifndef DWMSBT_MAINWINDOW
#define DWMSBT_MAINWINDOW 2
#endif
#endif // Q_OS_WIN

ColorScheme Theme::darkColors()
{
    return {
        .sidebarBg    = QColor("#20233a"),   // 稍亮，模拟 Mica 透过
        .mainBg       = QColor("#141425"),   // 稍亮
        .cardBg       = QColor("#222540"),   // 稍亮
        .accent       = QColor("#4f8cff"),
        .accentHover  = QColor("#6ba0ff"),
        .success      = QColor("#34d399"),
        .warning      = QColor("#f59e0b"),
        .danger       = QColor("#ef4444"),
        .textPrimary  = QColor("#e2e4f0"),
        .textSecondary= QColor("#8b8fa3"),
        .borderColor  = QColor(255, 255, 255, 20),
        .navIndicator = QColor("#4f8cff"),
    };
}

ColorScheme Theme::lightColors()
{
    return {
        .sidebarBg    = QColor("#f0f1f5"),
        .mainBg       = QColor("#fafafa"),
        .cardBg       = QColor("#ffffff"),
        .accent       = QColor("#3b82f6"),
        .accentHover  = QColor("#60a5fa"),
        .success      = QColor("#10b981"),
        .warning      = QColor("#f59e0b"),
        .danger       = QColor("#ef4444"),
        .textPrimary  = QColor("#1e293b"),
        .textSecondary= QColor("#64748b"),
        .borderColor  = QColor(0, 0, 0, 20),
        .navIndicator = QColor("#3b82f6"),
    };
}

ColorScheme Theme::currentColors()
{
    if (currentTheme == "dark") return darkColors();
    return lightColors();
}

QString Theme::detectSystemTheme()
{
#ifdef Q_OS_WIN
    QSettings reg(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)",
                  QSettings::NativeFormat);
    int appsUseLight = reg.value("AppsUseLightTheme", 1).toInt();
    return appsUseLight == 1 ? "light" : "dark";
#else
    return "light";
#endif
}

void Theme::applyTheme(const QString &themeName)
{
    QString actual = themeName;
    if (actual == "system") {
        actual = detectSystemTheme();
    }
    currentTheme = actual;

    auto *app = qApp;
    if (!app) return;

    auto cs = currentColors();

    if (actual == "dark") {
        app->setStyle(QStyleFactory::create("Fusion"));
        QPalette p;
        p.setColor(QPalette::Window,          QColor(0, 0, 0, 0));  // transparent for Mica
        p.setColor(QPalette::WindowText,      cs.textPrimary);
        p.setColor(QPalette::Base,            cs.cardBg);
        p.setColor(QPalette::AlternateBase,   cs.mainBg.lighter(120));
        p.setColor(QPalette::ToolTipBase,     cs.cardBg);
        p.setColor(QPalette::ToolTipText,     cs.textPrimary);
        p.setColor(QPalette::Text,            cs.textPrimary);
        p.setColor(QPalette::Button,          cs.cardBg);
        p.setColor(QPalette::ButtonText,      cs.textPrimary);
        p.setColor(QPalette::BrightText,      cs.danger);
        p.setColor(QPalette::Link,            cs.accent);
        p.setColor(QPalette::Highlight,       cs.accent);
        p.setColor(QPalette::HighlightedText, Qt::white);
        p.setColor(QPalette::Disabled, QPalette::Text,       QColor(100, 100, 100));
        p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(100, 100, 100));
        app->setPalette(p);
        app->setStyleSheet(darkStyleSheet());
    } else {
        app->setStyle(QStyleFactory::create("Fusion"));
        QPalette p;
        p.setColor(QPalette::Window,          QColor(0, 0, 0, 0));  // transparent for Mica
        p.setColor(QPalette::WindowText,      cs.textPrimary);
        p.setColor(QPalette::Base,            cs.cardBg);
        p.setColor(QPalette::AlternateBase,   QColor("#f5f5f5"));
        p.setColor(QPalette::ToolTipBase,     cs.cardBg);
        p.setColor(QPalette::ToolTipText,     cs.textPrimary);
        p.setColor(QPalette::Text,            cs.textPrimary);
        p.setColor(QPalette::Button,          cs.cardBg);
        p.setColor(QPalette::ButtonText,      cs.textPrimary);
        p.setColor(QPalette::BrightText,      cs.danger);
        p.setColor(QPalette::Link,            cs.accent);
        p.setColor(QPalette::Highlight,       cs.accent);
        p.setColor(QPalette::HighlightedText, Qt::white);
        p.setColor(QPalette::Disabled, QPalette::Text,       QColor(180, 180, 180));
        p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(180, 180, 180));
        app->setPalette(p);
        app->setStyleSheet(lightStyleSheet());
    }
}

QString Theme::darkStyleSheet()
{
    return QStringLiteral(
        "QMainWindow { background: transparent; }\n"
        "QWidget { font-family: \"LXGW Neo XiHei Plus\", \"Inter\", \"Microsoft YaHei\", \"Segoe UI\", sans-serif; font-size: 13px; }\n"
        "QLabel#logoLabel { font-size: 18px; font-weight: 800; color: #f4f7ff; padding: 0; }\n"
        "QLabel#sidebarTagline { color: #8f97b5; font-size: 11px; padding: 0; }\n"
        "QLabel#navSectionLabel { color: #69708d; font-size: 11px; font-weight: 700; padding: 10px 10px 6px 10px; text-transform: uppercase; letter-spacing: 0.8px; }\n"
        "QLabel#sidebarPageLabel { color: #f4f7ff; font-size: 15px; font-weight: 700; }\n"
        "QLabel#sidebarPageHint { color: #9298b1; font-size: 11px; line-height: 1.45; }\n"
        "QLabel#sidebarMetaLabel { color: #7d84a1; font-size: 10px; font-weight: 700; text-transform: uppercase; letter-spacing: 0.6px; }\n"
        "QLabel#sidebarMetaValue { color: #dfe4f6; font-size: 12px; font-weight: 600; }\n"
        "QLabel#versionLabel { color: #7d84a1; font-size: 11px; padding-bottom: 2px; }\n"
        "QWidget#sidebar { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 rgba(23,25,43,0.65), stop:1 rgba(23,25,43,0.92)); border-right: none; }\n"
        "QFrame#sidebarBrandCard { background: rgba(79,140,255,0.06); border: 1px solid rgba(79,140,255,0.12); border-radius: 16px; }\n"
        "QFrame#sidebarBrandCard[compact=\"true\"] { border-radius: 14px; }\n"
        "QFrame#sidebarContextCard { background: rgba(255,255,255,0.02); border: 1px solid rgba(255,255,255,0.04); border-radius: 16px; }\n"
        "QFrame#sidebarContextCard[compact=\"true\"] { border-radius: 14px; }\n"
        "QPushButton#navBtn { text-align: left; padding: 11px 14px; border: 1px solid transparent; border-radius: 12px; background: transparent; color: #9ca3bd; font-size: 13px; font-weight: 600; margin: 2px 0; outline: none; }\n"
        "QPushButton#navBtn:hover { background: rgba(255,255,255,0.07); border: 1px solid rgba(255,255,255,0.06); color: #eef2ff; }\n"
        "QPushButton#navBtn:checked { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 rgba(79,140,255,0.18), stop:1 rgba(79,140,255,0.08)); border: 1px solid rgba(79,140,255,0.28); color: #8fb4ff; font-weight: 700; }\n"
        "QPushButton#navBtn:focus { border: 1px solid rgba(143,180,255,0.30); }\n"
        "QFrame#separator { background: rgba(255,255,255,0.06); max-height: 1px; }\n"
        "QLabel#pageTitle { font-size: 24px; font-weight: 700; color: #e2e4f0; letter-spacing: -0.5px; }\n"
        "QLabel#pageDesc { color: #8b8fa3; font-size: 12px; margin-top: 2px; }\n"
        "QLabel#sectionTitle { font-size: 12px; font-weight: 700; color: #8b8fa3; padding: 0 0 2px 2px; text-transform: uppercase; letter-spacing: 0.5px; }\n"
        "QFrame#card { background: transparent; border: none; }\n"
        "QFrame#statCard { background: transparent; border: none; }\n"
        "QFrame#statCard:hover { border: none; }\n"
        "QFrame[dashboardCard=\"true\"] { border: 1px solid rgba(79,140,255,0.10); }\n"
        "QFrame[dashboardCard=\"true\"]:hover { border: 1px solid rgba(79,140,255,0.18); }\n"
        "QFrame[pageTopCard=\"true\"] { border: 1px solid rgba(79,140,255,0.14); background: rgba(79,140,255,0.03); }\n"
        "QFrame[pageTopCard=\"true\"]:hover { border: 1px solid rgba(79,140,255,0.22); background: rgba(79,140,255,0.05); }\n"
        "QLabel#statValue { font-size: 30px; font-weight: 300; color: #e2e4f0; font-family: \"JetBrains Mono\", \"Consolas\", \"Courier New\", monospace; }\n"
        "QLabel#miniStatValue { font-size: 14px; font-weight: 600; color: #e2e4f0; }\n"
        "QLabel#statLabel { font-size: 11px; color: #8b8fa3; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; }\n"
        "QLabel#statusPill { color: #8fb4ff; background: rgba(79,140,255,0.12); border: 1px solid rgba(79,140,255,0.18); border-radius: 999px; padding: 6px 12px; font-size: 11px; font-weight: 600; }\n"
        "QLabel#statusBadge { border-radius: 999px; padding: 6px 12px; font-size: 12px; font-weight: 600; }\n"
        "QLabel#fieldLabel { color: #c9cde0; font-size: 12px; font-weight: 600; min-width: 72px; }\n"
        "QPushButton#primaryBtn { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #5a9aff, stop:1 #4080f0); color: white; border: 1px solid rgba(140,183,255,0.16); border-radius: 10px; padding: 9px 20px; min-width: 96px; min-height: 38px; font-weight: 700; font-size: 13px; outline: none; }\n"
        "QPushButton#primaryBtn:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #6ea8ff, stop:1 #5090ff); border: 1px solid rgba(140,183,255,0.30); }\n"
        "QPushButton#primaryBtn:pressed { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #3870d8, stop:1 #3068c8); padding-top: 10px; padding-bottom: 8px; }\n"
        "QPushButton#primaryBtn:focus { border: 1px solid rgba(191,219,254,0.40); }\n"
        "QPushButton#primaryBtn:disabled { background: #3a3a5c; border: 1px solid rgba(255,255,255,0.04); color: #6b6b8a; }\n"
        "QPushButton#secondaryBtn { background: rgba(79,140,255,0.06); color: #8fb4ff; border: 1px solid rgba(79,140,255,0.22); border-radius: 10px; padding: 9px 18px; min-width: 96px; min-height: 38px; font-size: 13px; font-weight: 600; outline: none; }\n"
        "QPushButton#secondaryBtn:hover { background: rgba(79,140,255,0.14); border: 1px solid rgba(79,140,255,0.36); color: #c6dcff; }\n"
        "QPushButton#secondaryBtn:pressed { background: rgba(79,140,255,0.20); padding-top: 10px; padding-bottom: 8px; }\n"
        "QPushButton#secondaryBtn:focus { border: 1px solid rgba(191,219,254,0.30); }\n"
        "QPushButton#secondaryBtn:disabled { background: rgba(255,255,255,0.02); border: 1px solid rgba(255,255,255,0.04); color: #6b6b8a; }\n"
        "QPushButton#dangerBtn { background: rgba(239,68,68,0.04); color: #ff8f8f; border: 1px solid rgba(239,68,68,0.24); border-radius: 10px; padding: 9px 18px; min-width: 96px; min-height: 38px; font-size: 13px; font-weight: 600; outline: none; }\n"
        "QPushButton#dangerBtn:hover { background: rgba(239,68,68,0.10); border: 1px solid rgba(239,68,68,0.34); color: #ffc0c0; }\n"
        "QPushButton#dangerBtn:pressed { background: rgba(239,68,68,0.16); padding-top: 10px; padding-bottom: 8px; }\n"
        "QPushButton#dangerBtn:focus { border: 1px solid rgba(254,202,202,0.26); }\n"
        "QPushButton#dangerBtn:disabled { background: rgba(255,255,255,0.02); border: 1px solid rgba(255,255,255,0.04); color: #6b6b8a; }\n"
        "QPushButton#iconBtn { background: rgba(255,255,255,0.02); border: 1px solid transparent; border-radius: 8px; padding: 6px; min-width: 34px; min-height: 34px; outline: none; }\n"
        "QPushButton#iconBtn:hover { background: rgba(255,255,255,0.06); border: 1px solid rgba(255,255,255,0.06); }\n"
        "QPushButton#iconBtn:focus { border: 1px solid rgba(191,219,254,0.20); }\n"
        "QLineEdit, QSpinBox { background: #15162a; color: #e2e4f0; border: 1px solid rgba(255,255,255,0.10); border-radius: 10px; padding: 8px 12px; font-size: 13px; }\n"
        "QLineEdit:hover, QSpinBox:hover { border: 1px solid rgba(255,255,255,0.16); }\n"
        "QLineEdit:focus, QSpinBox:focus { border: 1px solid #4f8cff; background: #181a30; }\n"
        "QComboBox { background: #15162a; color: #e2e4f0; border: 1px solid rgba(255,255,255,0.10); border-radius: 10px; padding: 8px 12px; min-height: 32px; font-size: 13px; }\n"
        "QComboBox:hover { border: 1px solid rgba(255,255,255,0.16); }\n"
        "QComboBox:focus { border: 1px solid #4f8cff; }\n"
        "QComboBox::drop-down { border: none; width: 24px; }\n"
        "QComboBox QAbstractItemView { background: #1c1d33; color: #e2e4f0; border: 1px solid rgba(255,255,255,0.08); border-radius: 6px; padding: 4px; selection-background-color: rgba(79,140,255,0.15); outline: none; }\n"
        "QComboBox QAbstractItemView::item { padding: 6px 12px; border-radius: 4px; }\n"
        "QTableWidget { background: transparent; color: #e2e4f0; border: 1px solid rgba(255,255,255,0.04); border-radius: 12px; gridline-color: transparent; alternate-background-color: rgba(255,255,255,0.015); selection-background-color: rgba(79,140,255,0.14); selection-color: #ffffff; outline: none; }\n"
        "QTableWidget::item { padding: 9px 12px; border-bottom: 1px solid rgba(255,255,255,0.03); }\n"
        "QHeaderView::section { background: rgba(255,255,255,0.03); color: #8b8fa3; border: none; border-bottom: 1px solid rgba(255,255,255,0.08); padding: 10px 12px; font-size: 11px; font-weight: 700; text-transform: uppercase; letter-spacing: 0.3px; }\n"
        "QTableWidget::item:hover { background: rgba(79,140,255,0.06); }\n"
        "QPushButton#tableBtn { background: rgba(79,140,255,0.10); color: #8fb4ff; border: 1px solid rgba(79,140,255,0.18); border-radius: 9px; padding: 6px 14px; font-size: 12px; font-weight: 700; min-width: 74px; min-height: 30px; outline: none; }\n"
        "QPushButton#tableBtn:hover { background: rgba(79,140,255,0.18); border: 1px solid rgba(79,140,255,0.28); color: #d8e7ff; }\n"
        "QPushButton#tableBtn:pressed { background: rgba(79,140,255,0.28); padding-top: 7px; padding-bottom: 5px; }\n"
        "QPushButton#tableBtn:focus { border: 1px solid rgba(191,219,254,0.24); }\n"
        "QPushButton#tableBtn:disabled { background: rgba(255,255,255,0.02); border: 1px solid rgba(255,255,255,0.04); color: #5e6278; }\n"
        "QListWidget#activityList { background: rgba(255,255,255,0.02); border: 1px solid rgba(255,255,255,0.05); border-radius: 12px; padding: 6px; outline: none; }\n"
        "QListWidget#activityList::item { color: #cfd3e8; padding: 10px 12px; border-radius: 8px; border-bottom: 1px solid rgba(255,255,255,0.04); }\n"
        "QListWidget#activityList::item:hover { background: rgba(79,140,255,0.05); }\n"
        "QTextEdit#dryRunOutput { background: #0d0e1e; color: #a0bcd8; border: 1px solid rgba(79,140,255,0.12); border-radius: 12px; padding: 14px; font-family: 'JetBrains Mono', 'Consolas', monospace; font-size: 12px; selection-background-color: rgba(79,140,255,0.25); }\n"
        "QStatusBar { background: #1a1b2e; color: #8b8fa3; border-top: 1px solid rgba(255,255,255,0.04); font-size: 12px; padding: 3px 14px; }\n"
        "QProgressBar { background: #15162a; border: none; border-radius: 4px; height: 6px; text-align: center; font-size: 10px; color: transparent; }\n"
        "QProgressBar::chunk { background: #4f8cff; border-radius: 4px; }\n"
        "QScrollBar:vertical { background: rgba(255,255,255,0.03); width: 10px; margin: 2px; border-radius: 5px; }\n"
        "QScrollBar::handle:vertical { background: rgba(255,255,255,0.18); border-radius: 5px; min-height: 30px; }\n"
        "QScrollBar::handle:vertical:hover { background: rgba(255,255,255,0.30); }\n"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }\n"
        "QScrollBar:horizontal { background: rgba(255,255,255,0.03); height: 10px; margin: 2px; border-radius: 5px; }\n"
        "QScrollBar::handle:horizontal { background: rgba(255,255,255,0.18); border-radius: 5px; min-width: 30px; }\n"
        "QScrollBar::handle:horizontal:hover { background: rgba(255,255,255,0.30); }\n"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }\n"
        "QCheckBox { color: #e2e4f0; spacing: 10px; font-size: 13px; }\n"
        "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; border: 1.5px solid rgba(255,255,255,0.18); background: transparent; }\n"
        "QCheckBox::indicator:checked { background: #4f8cff; border-color: #4f8cff; }\n"
        "QCheckBox::indicator:hover { border-color: rgba(255,255,255,0.30); }\n"
        "QTabWidget::pane { border: none; background: transparent; }\n"
        "QTabBar::tab { background: transparent; color: #8b8fa3; padding: 8px 16px; border: none; border-bottom: 2px solid transparent; }\n"
        "QTabBar::tab:selected { color: #4f8cff; border-bottom: 2px solid #4f8cff; }\n"
        "QTabBar::tab:hover:!selected { color: #e2e4f0; }\n"
        "QGroupBox { color: #8b8fa3; border: 1px solid rgba(255,255,255,0.06); border-radius: 10px; margin-top: 8px; padding: 16px 14px 14px 14px; font-weight: 600; }\n"
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 4px; }\n"
        "QToolTip { background: #1e2040; color: #e2e4f0; border: 1px solid rgba(255,255,255,0.08); border-radius: 8px; padding: 6px 10px; font-size: 12px; }\n"
        "QLabel#helperText { color: #99a0ba; font-size: 11px; }\n"
        "QLabel#emptyState { color: #8b8fa3; font-size: 12px; padding: 36px 24px; border: 1px dashed rgba(79,140,255,0.18); border-radius: 12px; background: rgba(255,255,255,0.01); }\n"
        "QLabel#pageDesc { color: #8b8fa3; font-size: 12px; }\n"
    );
}

QString Theme::lightStyleSheet()
{
    return QStringLiteral(
        "QMainWindow { background: transparent; }\n"
        "QWidget { font-family: \"LXGW Neo XiHei Plus\", \"Inter\", \"Microsoft YaHei\", \"Segoe UI\", sans-serif; font-size: 13px; }\n"
        "QLabel#logoLabel { font-size: 18px; font-weight: 800; color: #1e40af; padding: 0; }\n"
        "QLabel#sidebarTagline { color: #64748b; font-size: 11px; padding: 0; }\n"
        "QLabel#navSectionLabel { color: #64748b; font-size: 11px; font-weight: 700; padding: 10px 10px 6px 10px; text-transform: uppercase; letter-spacing: 0.8px; }\n"
        "QLabel#sidebarPageLabel { color: #0f172a; font-size: 15px; font-weight: 700; }\n"
        "QLabel#sidebarPageHint { color: #64748b; font-size: 11px; line-height: 1.45; }\n"
        "QLabel#sidebarMetaLabel { color: #64748b; font-size: 10px; font-weight: 700; text-transform: uppercase; letter-spacing: 0.6px; }\n"
        "QLabel#sidebarMetaValue { color: #1e293b; font-size: 12px; font-weight: 600; }\n"
        "QLabel#versionLabel { color: #64748b; font-size: 11px; padding-bottom: 2px; }\n"
        "QWidget#sidebar { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 rgba(240,241,245,0.60), stop:1 rgba(240,241,245,0.95)); border-right: none; }\n"
        "QFrame#sidebarBrandCard { background: rgba(59,130,246,0.05); border: 1px solid rgba(59,130,246,0.10); border-radius: 16px; }\n"
        "QFrame#sidebarBrandCard[compact=\"true\"] { border-radius: 14px; }\n"
        "QFrame#sidebarContextCard { background: rgba(255,255,255,0.55); border: 1px solid rgba(59,130,246,0.06); border-radius: 16px; }\n"
        "QFrame#sidebarContextCard[compact=\"true\"] { border-radius: 14px; }\n"
        "QPushButton#navBtn { text-align: left; padding: 11px 14px; border: 1px solid transparent; border-radius: 12px; background: transparent; color: #64748b; font-size: 13px; font-weight: 600; margin: 2px 0; outline: none; }\n"
        "QPushButton#navBtn:hover { background: rgba(59,130,246,0.05); border: 1px solid rgba(59,130,246,0.08); color: #0f172a; }\n"
        "QPushButton#navBtn:checked { background: rgba(59,130,246,0.10); border: 1px solid rgba(59,130,246,0.18); color: #2563eb; font-weight: 700; }\n"
        "QPushButton#navBtn:focus { border: 1px solid rgba(59,130,246,0.22); }\n"
        "QFrame#separator { background: rgba(0,0,0,0.06); max-height: 1px; }\n"
        "QLabel#pageTitle { font-size: 24px; font-weight: 700; color: #1e293b; letter-spacing: -0.5px; }\n"
        "QLabel#pageDesc { color: #64748b; font-size: 12px; margin-top: 2px; }\n"
        "QLabel#sectionTitle { font-size: 12px; font-weight: 700; color: #64748b; padding: 0 0 2px 2px; text-transform: uppercase; letter-spacing: 0.5px; }\n"
        "QFrame#card { background: transparent; border: none; }\n"
        "QFrame#statCard { background: transparent; border: none; }\n"
        "QFrame#statCard:hover { border: none; }\n"
        "QFrame[dashboardCard=\"true\"] { border: 1px solid rgba(59,130,246,0.10); }\n"
        "QFrame[dashboardCard=\"true\"]:hover { border: 1px solid rgba(59,130,246,0.18); }\n"
        "QFrame[pageTopCard=\"true\"] { border: 1px solid rgba(59,130,246,0.14); background: rgba(59,130,246,0.03); }\n"
        "QFrame[pageTopCard=\"true\"]:hover { border: 1px solid rgba(59,130,246,0.22); background: rgba(59,130,246,0.05); }\n"
        "QLabel#statValue { font-size: 30px; font-weight: 300; color: #1e293b; font-family: \"JetBrains Mono\", \"Consolas\", \"Courier New\", monospace; }\n"
        "QLabel#miniStatValue { font-size: 14px; font-weight: 600; color: #1e293b; }\n"
        "QLabel#statLabel { font-size: 11px; color: #64748b; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; }\n"
        "QLabel#statusPill { color: #2563eb; background: rgba(59,130,246,0.10); border: 1px solid rgba(59,130,246,0.16); border-radius: 999px; padding: 6px 12px; font-size: 11px; font-weight: 600; }\n"
        "QLabel#statusBadge { border-radius: 999px; padding: 6px 12px; font-size: 12px; font-weight: 600; }\n"
        "QLabel#fieldLabel { color: #334155; font-size: 12px; font-weight: 600; min-width: 72px; }\n"
        "QPushButton#primaryBtn { background: #3b82f6; color: white; border: 1px solid rgba(59,130,246,0.10); border-radius: 10px; padding: 9px 20px; min-width: 96px; min-height: 38px; font-weight: 700; font-size: 13px; outline: none; }\n"
        "QPushButton#primaryBtn:hover { background: #60a5fa; border: 1px solid rgba(59,130,246,0.20); }\n"
        "QPushButton#primaryBtn:pressed { background: #2563eb; padding-top: 10px; padding-bottom: 8px; }\n"
        "QPushButton#primaryBtn:focus { border: 1px solid rgba(147,197,253,0.34); }\n"
        "QPushButton#primaryBtn:disabled { background: #e2e2ea; border: 1px solid rgba(0,0,0,0.04); color: #999; }\n"
        "QPushButton#secondaryBtn { background: rgba(59,130,246,0.04); color: #2563eb; border: 1px solid rgba(59,130,246,0.22); border-radius: 10px; padding: 9px 18px; min-width: 96px; min-height: 38px; font-size: 13px; font-weight: 600; outline: none; }\n"
        "QPushButton#secondaryBtn:hover { background: rgba(59,130,246,0.08); border: 1px solid rgba(59,130,246,0.30); color: #1d4ed8; }\n"
        "QPushButton#secondaryBtn:pressed { background: rgba(59,130,246,0.14); padding-top: 10px; padding-bottom: 8px; }\n"
        "QPushButton#secondaryBtn:focus { border: 1px solid rgba(96,165,250,0.28); }\n"
        "QPushButton#secondaryBtn:disabled { background: rgba(0,0,0,0.02); border: 1px solid rgba(0,0,0,0.06); color: #b6bcc8; }\n"
        "QPushButton#dangerBtn { background: rgba(239,68,68,0.03); color: #dc2626; border: 1px solid rgba(239,68,68,0.20); border-radius: 10px; padding: 9px 18px; min-width: 96px; min-height: 38px; font-size: 13px; font-weight: 600; outline: none; }\n"
        "QPushButton#dangerBtn:hover { background: rgba(239,68,68,0.08); border: 1px solid rgba(239,68,68,0.28); color: #b91c1c; }\n"
        "QPushButton#dangerBtn:pressed { background: rgba(239,68,68,0.14); padding-top: 10px; padding-bottom: 8px; }\n"
        "QPushButton#dangerBtn:focus { border: 1px solid rgba(252,165,165,0.26); }\n"
        "QPushButton#dangerBtn:disabled { background: rgba(0,0,0,0.02); border: 1px solid rgba(0,0,0,0.05); color: #c8c8c8; }\n"
        "QPushButton#iconBtn { background: rgba(15,23,42,0.02); border: 1px solid transparent; border-radius: 8px; padding: 6px; min-width: 34px; min-height: 34px; outline: none; }\n"
        "QPushButton#iconBtn:hover { background: rgba(59,130,246,0.06); border: 1px solid rgba(59,130,246,0.08); }\n"
        "QPushButton#iconBtn:focus { border: 1px solid rgba(96,165,250,0.22); }\n"
        "QLineEdit, QSpinBox { background: #ffffff; color: #1e293b; border: 1px solid rgba(0,0,0,0.10); border-radius: 10px; padding: 8px 12px; font-size: 13px; }\n"
        "QLineEdit:hover, QSpinBox:hover { border: 1px solid rgba(0,0,0,0.16); }\n"
        "QLineEdit:focus, QSpinBox:focus { border: 1px solid #3b82f6; background: #fafafe; }\n"
        "QComboBox { background: #ffffff; color: #1e293b; border: 1px solid rgba(0,0,0,0.10); border-radius: 10px; padding: 8px 12px; min-height: 32px; font-size: 13px; }\n"
        "QComboBox:hover { border: 1px solid rgba(0,0,0,0.18); }\n"
        "QComboBox:focus { border: 1px solid #3b82f6; }\n"
        "QComboBox::drop-down { border: none; width: 24px; }\n"
        "QComboBox QAbstractItemView { background: #ffffff; color: #1e293b; border: 1px solid rgba(0,0,0,0.08); border-radius: 6px; padding: 4px; selection-background-color: rgba(59,130,246,0.08); outline: none; }\n"
        "QComboBox QAbstractItemView::item { padding: 6px 12px; border-radius: 4px; }\n"
        "QTableWidget { background: transparent; color: #1e293b; border: 1px solid rgba(0,0,0,0.05); border-radius: 12px; gridline-color: transparent; alternate-background-color: rgba(0,0,0,0.015); selection-background-color: rgba(59,130,246,0.10); selection-color: #0f172a; outline: none; }\n"
        "QTableWidget::item { padding: 9px 12px; border-bottom: 1px solid rgba(0,0,0,0.04); }\n"
        "QHeaderView::section { background: rgba(15,23,42,0.02); color: #64748b; border: none; border-bottom: 1px solid rgba(0,0,0,0.07); padding: 10px 12px; font-size: 11px; font-weight: 700; text-transform: uppercase; letter-spacing: 0.3px; }\n"
        "QTableWidget::item:hover { background: rgba(59,130,246,0.04); }\n"
        "QPushButton#tableBtn { background: rgba(59,130,246,0.06); color: #2563eb; border: 1px solid rgba(59,130,246,0.16); border-radius: 9px; padding: 6px 14px; font-size: 12px; font-weight: 700; min-width: 74px; min-height: 30px; outline: none; }\n"
        "QPushButton#tableBtn:hover { background: rgba(59,130,246,0.12); border: 1px solid rgba(59,130,246,0.24); color: #1d4ed8; }\n"
        "QPushButton#tableBtn:pressed { background: rgba(59,130,246,0.18); padding-top: 7px; padding-bottom: 5px; }\n"
        "QPushButton#tableBtn:focus { border: 1px solid rgba(96,165,250,0.22); }\n"
        "QPushButton#tableBtn:disabled { background: rgba(0,0,0,0.02); border: 1px solid rgba(0,0,0,0.05); color: #c3c7d0; }\n"
        "QListWidget#activityList { background: rgba(59,130,246,0.03); border: 1px solid rgba(59,130,246,0.10); border-radius: 12px; padding: 6px; outline: none; }\n"
        "QListWidget#activityList::item { color: #334155; padding: 10px 12px; border-radius: 8px; border-bottom: 1px solid rgba(0,0,0,0.04); }\n"
        "QListWidget#activityList::item:hover { background: rgba(59,130,246,0.05); }\n"
        "QTextEdit#dryRunOutput { background: #f0f4ff; color: #334155; border: 1px solid rgba(59,130,246,0.12); border-radius: 12px; padding: 14px; font-family: 'JetBrains Mono', 'Consolas', monospace; font-size: 12px; selection-background-color: rgba(59,130,246,0.15); }\n"
        "QStatusBar { background: #ededf5; color: #64748b; border-top: 1px solid rgba(0,0,0,0.04); font-size: 12px; padding: 3px 14px; }\n"
        "QProgressBar { background: #ededf5; border: none; border-radius: 4px; height: 6px; text-align: center; font-size: 10px; color: transparent; }\n"
        "QProgressBar::chunk { background: #3b82f6; border-radius: 4px; }\n"
        "QScrollBar:vertical { background: rgba(0,0,0,0.03); width: 10px; margin: 2px; border-radius: 5px; }\n"
        "QScrollBar::handle:vertical { background: rgba(0,0,0,0.18); border-radius: 5px; min-height: 30px; }\n"
        "QScrollBar::handle:vertical:hover { background: rgba(0,0,0,0.30); }\n"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }\n"
        "QScrollBar:horizontal { background: rgba(0,0,0,0.03); height: 10px; margin: 2px; border-radius: 5px; }\n"
        "QScrollBar::handle:horizontal { background: rgba(0,0,0,0.18); border-radius: 5px; min-width: 30px; }\n"
        "QScrollBar::handle:horizontal:hover { background: rgba(0,0,0,0.30); }\n"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }\n"
        "QCheckBox { color: #1e293b; spacing: 10px; font-size: 13px; }\n"
        "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; border: 1.5px solid rgba(0,0,0,0.18); background: transparent; }\n"
        "QCheckBox::indicator:checked { background: #3b82f6; border-color: #3b82f6; }\n"
        "QCheckBox::indicator:hover { border-color: rgba(0,0,0,0.30); }\n"
        "QTabWidget::pane { border: none; background: transparent; }\n"
        "QTabBar::tab { background: transparent; color: #64748b; padding: 8px 16px; border: none; border-bottom: 2px solid transparent; }\n"
        "QTabBar::tab:selected { color: #3b82f6; border-bottom: 2px solid #3b82f6; }\n"
        "QTabBar::tab:hover:!selected { color: #1e293b; }\n"
        "QGroupBox { color: #64748b; border: 1px solid rgba(0,0,0,0.06); border-radius: 10px; margin-top: 8px; padding: 16px 14px 14px 14px; font-weight: 600; }\n"
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 4px; }\n"
        "QToolTip { background: #ffffff; color: #1e293b; border: 1px solid rgba(0,0,0,0.08); border-radius: 8px; padding: 6px 10px; font-size: 12px; }\n"
        "QLabel#helperText { color: #64748b; font-size: 11px; }\n"
        "QLabel#emptyState { color: #64748b; font-size: 12px; padding: 36px 24px; border: 1px dashed rgba(59,130,246,0.20); border-radius: 12px; background: rgba(59,130,246,0.03); }\n"
    );
}