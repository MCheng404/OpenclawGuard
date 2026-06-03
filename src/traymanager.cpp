#include "traymanager.h"
#include "mainwindow.h"
#include "config.h"
#include <QApplication>

TrayManager::TrayManager(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    m_tray = new QSystemTrayIcon(window);
    m_tray->setIcon(QIcon(":/icons/icons/tray.svg"));
    m_tray->setToolTip("进程管理控制台");

    buildMenu();

    connect(m_tray, &QSystemTrayIcon::activated,
            this, &TrayManager::onTrayActivated);

    m_tray->show();
}

void TrayManager::buildMenu()
{
    m_menu = new QMenu();

    m_showAction = m_menu->addAction("显示主窗口");
    connect(m_showAction, &QAction::triggered, this, &TrayManager::showWindowRequested);

    m_menu->addSeparator();

    m_statusAction = m_menu->addAction("网关: 未知");
    m_statusAction->setEnabled(false);
    m_statusAction->setIcon(QIcon(":/icons/icons/circle-dot.svg"));

    m_restartAction = m_menu->addAction("重启网关");
    m_restartAction->setIcon(QIcon(":/icons/icons/rotate-cw.svg"));
    connect(m_restartAction, &QAction::triggered, this, &TrayManager::restartGatewayRequested);

    m_menu->addSeparator();

    m_quitAction = m_menu->addAction("退出");
    connect(m_quitAction, &QAction::triggered, this, &TrayManager::quitRequested);

    m_tray->setContextMenu(m_menu);
}

void TrayManager::show()
{
    m_tray->show();
}

void TrayManager::showMessage(const QString &title, const QString &msg)
{
    m_tray->showMessage(title, msg, QSystemTrayIcon::Information, 3000);
}

void TrayManager::updateGatewayStatus(bool online, const QString &port)
{
    if (!m_statusAction) return;
    if (online) {
        QString text = port.isEmpty() ? "网关: 在线" : QString("网关: 在线 · 端口 %1").arg(port);
        m_statusAction->setText(text);
        m_statusAction->setIcon(QIcon(":/icons/icons/check-circle.svg"));
        m_tray->setToolTip(QString("进程管理控制台 — 网关在线") + (port.isEmpty() ? "" : " · 端口 " + port));
    } else {
        m_statusAction->setText("网关: 离线");
        m_statusAction->setIcon(QIcon(":/icons/icons/x-circle.svg"));
        m_tray->setToolTip("进程管理控制台 — 网关离线");
    }
}

void TrayManager::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        emit showWindowRequested();
    }
}
