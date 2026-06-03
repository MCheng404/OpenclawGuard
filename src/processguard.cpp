#include "processguard.h"
#include "config.h"
#include <QFileInfo>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#endif

// ── 进程存活检测 ──
static bool isProcessRunningByName(const QString &exePath)
{
#ifdef Q_OS_WIN
    QString fileName = QFileInfo(exePath).fileName().toLower();
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    bool found = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (fileName == QString::fromWCharArray(pe.szExeFile).toLower()) {
                found = true; break;
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
#else
    Q_UNUSED(exePath) return false;
#endif
}

ProcessGuard::ProcessGuard(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)), m_retryTimer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &ProcessGuard::check);
    connect(m_retryTimer, &QTimer::timeout, this, &ProcessGuard::check);
    m_retryTimer->setInterval(10000); // 等待空闲时每 10s 检查
}

void ProcessGuard::setGuardItems(const QList<GuardItem> &items)
{
    m_items = items;
    m_status.clear();
    for (auto &i : items) m_status[i.name] = true;
}

void ProcessGuard::setInterval(int ms) { m_timer->setInterval(ms); }
void ProcessGuard::start() { m_timer->start(); }
void ProcessGuard::stop()  { m_timer->stop(); m_retryTimer->stop(); }

void ProcessGuard::addItem(const GuardItem &item)
{
    m_items.append(item);
    m_status[item.name] = true;
}
void ProcessGuard::removeItem(const QString &name)
{
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
        [&](const GuardItem &i) { return i.name == name; }), m_items.end());
    m_status.remove(name);
    m_pendingRestarts.removeAll(name);
}
void ProcessGuard::setItemEnabled(const QString &name, bool enabled)
{
    for (auto &i : m_items) {
        if (i.name == name) { i.enabled = enabled; break; }
    }
}

// ── Windows 系统负载检测 ──
#ifdef Q_OS_WIN

static quint64 fileTimeToUint64(const FILETIME &ft)
{
    return (static_cast<quint64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

int ProcessGuard::getCpuUsage()
{
    // 采样两次，间隔 250ms
    FILETIME idle1, kernel1, user1, idle2, kernel2, user2;
    GetSystemTimes(&idle1, &kernel1, &user1);
    QThread::msleep(250);
    GetSystemTimes(&idle2, &kernel2, &user2);

    quint64 idle = fileTimeToUint64(idle2) - fileTimeToUint64(idle1);
    quint64 kernel = fileTimeToUint64(kernel2) - fileTimeToUint64(kernel1);
    quint64 user = fileTimeToUint64(user2) - fileTimeToUint64(user1);
    quint64 total = kernel + user;

    if (total == 0) return 0;
    return static_cast<int>(100 - (idle * 100 / total));
}

int ProcessGuard::getMemUsage()
{
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if (!GlobalMemoryStatusEx(&mem)) return 0;
    return static_cast<int>(mem.dwMemoryLoad); // 0-100
}

#else
int ProcessGuard::getCpuUsage() { return 0; }
int ProcessGuard::getMemUsage() { return 0; }
#endif

bool ProcessGuard::isSystemBusy()
{
    int cpu = getCpuUsage();
    int mem = getMemUsage();
    if (cpu >= m_cpuThreshold) return true;
    if (mem >= m_memThreshold) return true;
    return false;
}

// ── 主检测循环 ──
void ProcessGuard::check()
{
    // 1) 先检查等待重试的进程是否已存活
    for (int i = m_pendingRestarts.size() - 1; i >= 0; --i) {
        const QString &name = m_pendingRestarts[i];
        auto it = std::find_if(m_items.begin(), m_items.end(),
                               [&](const GuardItem &g) { return g.name == name; });
        if (it == m_items.end() || !it->enabled) {
            m_pendingRestarts.removeAt(i); continue;
        }
        if (isProcessRunningByName(it->exePath)) {
            m_pendingRestarts.removeAt(i);
            DBG_LOG(QString("进程已自行恢复: %1").arg(name));
            emit statusChanged(name, true);
            continue;
        }
        // 2) 检查现在是否可以拉起了
        if (!isSystemBusy()) {
            DBG_LOG(QString("系统空闲，延迟拉起: %1").arg(name));
            QFileInfo fi(it->exePath);
            bool ok = QProcess::startDetached(it->exePath, {}, fi.absolutePath());
            if (ok) {
                DBG_LOG(QString("延迟拉起成功: %1").arg(name));
                emit processRestarted(name);
            }
            m_pendingRestarts.removeAt(i);
        }
    }
    // 没有待处理的了就停掉 retry timer
    if (m_pendingRestarts.isEmpty())
        m_retryTimer->stop();

    // 3) 常规检测
    for (auto &item : m_items) {
        if (!item.enabled) continue;
        bool running = isProcessRunningByName(item.exePath);
        m_status[item.name] = running;
        emit statusChanged(item.name, running);

        if (!running) {
            DBG_LOG(QString("进程掉线: %1").arg(item.name));
            emit processWentDown(item.name);

            if (isSystemBusy()) {
                int cpu = getCpuUsage(), mem = getMemUsage();
                QString reason = QString("CPU %1% / 内存 %2% 过高，延迟拉起").arg(cpu).arg(mem);
                DBG_LOG(QString("%1: %2").arg(item.name, reason));
                emit restartDeferred(item.name, reason);
                if (!m_pendingRestarts.contains(item.name))
                    m_pendingRestarts.append(item.name);
                if (!m_retryTimer->isActive())
                    m_retryTimer->start();
            } else {
                QFileInfo fi(item.exePath);
                bool ok = QProcess::startDetached(item.exePath, {}, fi.absolutePath());
                if (ok) {
                    DBG_LOG(QString("进程重启成功: %1").arg(item.name));
                    emit processRestarted(item.name);
                } else {
                    DBG_LOG(QString("进程重启失败: %1").arg(item.name));
                }
            }
        }
    }
}

QList<QPair<QString, QString>> ProcessGuard::listRunningProcesses()
{
    QList<QPair<QString, QString>> result;
#ifdef Q_OS_WIN
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return result;
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            QString name = QString::fromWCharArray(pe.szExeFile);
            QString path;
            HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
            if (hProc) {
                WCHAR buf[MAX_PATH];
                DWORD len = MAX_PATH;
                if (QueryFullProcessImageNameW(hProc, 0, buf, &len))
                    path = QString::fromWCharArray(buf);
                CloseHandle(hProc);
            }
            if (!path.isEmpty()) result.append({name, path});
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    std::sort(result.begin(), result.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });
    result.erase(std::unique(result.begin(), result.end(),
                  [](const auto &a, const auto &b) { return a.first == b.first && a.second == b.second; }),
                 result.end());
#endif
    return result;
}
