#pragma once
#include <QObject>
#include <QTimer>
#include <QMap>
#include <QProcess>
#include "settings.h"

class ProcessGuard : public QObject
{
    Q_OBJECT
public:
    explicit ProcessGuard(QObject *parent = nullptr);

    void setGuardItems(const QList<GuardItem> &items);
    void setInterval(int ms);
    void start();
    void stop();

    void addItem(const GuardItem &item);
    void removeItem(const QString &name);
    void setItemEnabled(const QString &name, bool enabled);

    int itemCount() const { return m_items.size(); }

    // 智能拉起阈值
    void setCpuThreshold(int pct)  { m_cpuThreshold = pct; }
    void setMemThreshold(int pct)  { m_memThreshold = pct; }
    int  cpuThreshold() const      { return m_cpuThreshold; }
    int  memThreshold() const      { return m_memThreshold; }

    // 系统资源查询（仪表盘用）
    int  getCpuUsage();     // 0-100
    int  getMemUsage();     // 0-100

    static QList<QPair<QString, QString>> listRunningProcesses();

signals:
    void processWentDown(const QString &name);
    void processRestarted(const QString &name);
    void statusChanged(const QString &name, bool running);
    void restartDeferred(const QString &name, const QString &reason);

private slots:
    void check();

private:
    bool isSystemBusy();    // CPU 或内存是否超阈

    QTimer              *m_timer = nullptr;
    QTimer              *m_retryTimer = nullptr;  // 等待系统空闲后重试
    QList<GuardItem>     m_items;
    QMap<QString, bool>  m_status;
    QStringList          m_pendingRestarts;        // 等待空闲后拉起的进程

    int m_cpuThreshold = 80;
    int m_memThreshold = 90;
};
