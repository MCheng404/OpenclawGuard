#pragma once
#include <QObject>
#include <QProcess>
#include "portmonitor.h"
#include "config.h"

class GatewayManager : public QObject
{
    Q_OBJECT
public:
    explicit GatewayManager(QObject *parent = nullptr);

    void startGateway();
    void stopGateway();
    void restartGateway();  // openclaw gateway restart（原子操作）

    bool isGatewayRunning() const;
    PortMonitor* monitor() const;

    int  port() const;
    void setPort(int port);

signals:
    void gatewayStarted();
    void gatewayStopped();
    void gatewayCrashed();

private slots:
    void onCommandFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    enum class CommandType { Start, Stop, Restart };

    void runCommand(const QStringList &args, CommandType type);

    PortMonitor *m_monitor = nullptr;
    QProcess    *m_proc = nullptr;
    CommandType  m_pendingType = CommandType::Start;
    bool         m_autoRestart = true;
    bool         m_restartPending = false;
};
