#include "backend.h"
#include "config.h"
#include <QWindow>
#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#ifndef DWMSBT_MAINWINDOW
#define DWMSBT_MAINWINDOW 2
#endif
#endif

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_gateway(new GatewayManager(this))
    , m_guard(new ProcessGuard(this))
    , m_updater(new UpdateManager(this))
    , m_envMgr(new EnvironmentManager(this))
{
    m_guard->setInterval(Config::PROCESS_CHECK_INTERVAL);
    m_guard->start();
    m_gateway->monitor()->setInterval(Config::PORT_CHECK_INTERVAL);
    m_gateway->monitor()->start();
    m_updater->fetchReleases();
    m_envMgr->detectAll();
    m_envMgr->checkLatestVersions();
}

void Backend::setTheme(const QString &name)
{
    Theme::applyTheme(name);
    m_themeName = (name == "dark") ? "dark" : "light";
    m_accent = Theme::currentColors().accent;
    emit themeChanged();
}

void Backend::enableMica(QWindow *window)
{
#ifdef Q_OS_WIN
    if (!window) return;
    HWND hwnd = reinterpret_cast<HWND>(window->winId());
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);
    DWM_SYSTEMBACKDROP_TYPE backdrop = static_cast<DWM_SYSTEMBACKDROP_TYPE>(DWMSBT_MAINWINDOW);
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));
#endif
}
