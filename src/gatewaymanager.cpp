#include "gatewaymanager.h"
#include "settings.h"
#include <QStandardPaths>

GatewayManager::GatewayManager(QObject *parent)
    : QObject(parent)
    , m_monitor(new PortMonitor(this))
    , m_proc(new QProcess(this))
{
    m_monitor->setPort(AppSettings.gatewayPort());

    connect(m_monitor, &PortMonitor::onlineChanged, this, [this](bool online) {
        if (!online && m_autoRestart && !m_restartPending) {
            m_restartPending = true;
            emit gatewayCrashed();
            restartGateway();
        }
        if (online) m_restartPending = false;
    });

    connect(m_proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &GatewayManager::onCommandFinished);
}

void GatewayManager::startGateway()
{
    runCommand({"gateway", "start"}, CommandType::Start);
}

void GatewayManager::stopGateway()
{
    m_autoRestart = false;
    runCommand({"gateway", "stop"}, CommandType::Stop);
}

void GatewayManager::restartGateway()
{
    runCommand({"gateway", "restart"}, CommandType::Restart);
}

bool GatewayManager::isGatewayRunning() const
{
    return m_monitor->isOnline();
}

PortMonitor* GatewayManager::monitor() const
{
    return m_monitor;
}

int GatewayManager::port() const
{
    return m_monitor->port();
}

void GatewayManager::setPort(int port)
{
    m_monitor->setPort(port);
}

void GatewayManager::runCommand(const QStringList &args, CommandType type)
{
    if (m_proc->state() != QProcess::NotRunning) {
        m_proc->kill();
        m_proc->waitForFinished(3000);
    }

    m_pendingType = type;
    // 通过 cmd /c 运行 openclaw（.cmd 脚本需要 cmd.exe 包装）
    QStringList cmdArgs = {"/c", "openclaw"};
    cmdArgs.append(args);
    m_proc->start("cmd", cmdArgs);

    if (type == CommandType::Stop)
        m_autoRestart = false;
}

void GatewayManager::onCommandFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    switch (m_pendingType) {
    case CommandType::Start:
        m_autoRestart = true;
        emit gatewayStarted();
        break;
    case CommandType::Stop:
        emit gatewayStopped();
        break;
    case CommandType::Restart:
        m_autoRestart = true;
        emit gatewayStarted();
        break;
    }
}
