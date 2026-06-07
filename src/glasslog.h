#pragma once
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QCoreApplication>
#include <cstdarg>

inline void glassLog(const char *fmt, ...)
{
    static QFile f(QCoreApplication::applicationDirPath() + "/glass-debug.log");
    if (!f.isOpen()) f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Truncate);
    va_list args;
    va_start(args, fmt);
    QString msg = QString::vasprintf(fmt, args);
    va_end(args);
    QTextStream ts(&f);
    ts << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ") << msg << "\n";
    f.flush();
}
