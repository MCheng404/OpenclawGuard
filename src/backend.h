#pragma once
#include <QObject>
#include "gatewaymanager.h"
#include "processguard.h"
#include "updatemanager.h"
#include "envmanager.h"
#include "settings.h"
#include "theme.h"

/// Bridge between C++ managers and QML UI
class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(GatewayManager* gateway READ gateway CONSTANT)
    Q_PROPERTY(ProcessGuard* guard READ guard CONSTANT)
    Q_PROPERTY(UpdateManager* updater READ updater CONSTANT)
    Q_PROPERTY(EnvironmentManager* envMgr READ envMgr CONSTANT)
    Q_PROPERTY(QString themeName READ themeName NOTIFY themeChanged)
    Q_PROPERTY(QColor accent READ accent NOTIFY themeChanged)

public:
    explicit Backend(QObject *parent = nullptr);

    GatewayManager* gateway() const { return m_gateway; }
    ProcessGuard* guard() const { return m_guard; }
    UpdateManager* updater() const { return m_updater; }
    EnvironmentManager* envMgr() const { return m_envMgr; }

    QString themeName() const { return m_themeName; }
    QColor accent() const { return m_accent; }

    Q_INVOKABLE void setTheme(const QString &name);
    Q_INVOKABLE void enableMica(QWindow *window);

signals:
    void themeChanged();

private:
    GatewayManager     *m_gateway;
    ProcessGuard       *m_guard;
    UpdateManager      *m_updater;
    EnvironmentManager *m_envMgr;
    QString             m_themeName = "dark";
    QColor              m_accent = QColor("#4f8cff");
};
