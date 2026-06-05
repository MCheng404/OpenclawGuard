#pragma once
#include <QString>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>

// 版本号
#ifndef APP_VERSION
#define APP_VERSION "1.7.0"
#endif

// Debug 日志宏
#if OPENCLAWGUARD_DEBUG
#define DBG_FILE() ([]() -> QFile* { \
    static QFile* f = nullptr; \
    if (!f) { \
        QString _logDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation); \
        QDir().mkpath(_logDir); \
        f = new QFile(_logDir + "/debug.log"); \
        (void)f->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text); \
    } \
    return f; \
}())
#define DBG_LOG(msg) do { \
    QString _ts = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"); \
    QString _line = QString("[%1] %2\n").arg(_ts, msg); \
    QTextStream _dbg_stream(DBG_FILE()); \
    _dbg_stream << _line; \
    DBG_FILE()->flush(); \
    qDebug() << msg; \
} while(0)
#define DBG_ENTER()  DBG_LOG(QString("→ %1").arg(__FUNCTION__))
#define DBG_EXIT()   DBG_LOG(QString("← %1").arg(__FUNCTION__))
#else
#define DBG_LOG(msg)  ((void)0)
#define DBG_ENTER()   ((void)0)
#define DBG_EXIT()    ((void)0)
#endif

namespace Config {

// 网关默认端口
constexpr int  GATEWAY_DEFAULT_PORT    = 18789;
// 端口检测间隔 (ms)
constexpr int  PORT_CHECK_INTERVAL     = 5000;
// 端口检测连续失败多少次认定离线
constexpr int  PORT_FAIL_THRESHOLD     = 3;
// 进程检测间隔 (ms)
constexpr int  PROCESS_CHECK_INTERVAL  = 3000;
// 更新检查间隔 (h)
constexpr int  UPDATE_CHECK_INTERVAL_H = 6;
// 健康检查超时 (ms)
constexpr int  HEALTHCHECK_TIMEOUT_MS  = 3000;

// Openclaw CLI 默认命令
const QString  OPENCLAW_CLI           = "openclaw";
// 网关名
const QString  GATEWAY_NAME            = "Openclaw Gateway";

// 更新 API 基址
const QString  UPDATE_API_BASE     = "https://api.github.com/repos/openclaw/openclaw";

namespace Reg {
const QString ORG  = "OpenclawGuard";
const QString APP  = "OpenclawGuard";
} // namespace Reg

} // namespace Config