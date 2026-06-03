#include "settings.h"
#include <QStandardPaths>

Settings::Settings()
    : s(Config::Reg::ORG, Config::Reg::APP)
{
}

Settings& Settings::instance()
{
    static Settings inst;
    return inst;
}

QString Settings::gatewayPath() const
{
    return s.value("gateway/path", "").toString();
}

void Settings::setGatewayPath(const QString &path)
{
    s.setValue("gateway/path", path);
}

int Settings::gatewayPort() const
{
    return s.value("gateway/port", Config::GATEWAY_DEFAULT_PORT).toInt();
}

void Settings::setGatewayPort(int port)
{
    s.setValue("gateway/port", port);
}

bool Settings::autoRestartGateway() const
{
    return s.value("gateway/autoRestart", true).toBool();
}

void Settings::setAutoRestartGateway(bool on)
{
    s.setValue("gateway/autoRestart", on);
}

bool Settings::startMinimized() const
{
    return s.value("ui/startMinimized", true).toBool();
}

void Settings::setStartMinimized(bool on)
{
    s.setValue("ui/startMinimized", on);
}

bool Settings::autoStart() const
{
    return s.value("ui/autoStart", false).toBool();
}

void Settings::setAutoStart(bool on)
{
    s.setValue("ui/autoStart", on);
}

QString Settings::githubToken() const
{
    return s.value("github/token", "").toString();
}

void Settings::setGithubToken(const QString &token)
{
    s.setValue("github/token", token);
}

bool Settings::autoCheckUpdate() const
{
    return s.value("update/autoCheck", true).toBool();
}

void Settings::setAutoCheckUpdate(bool on)
{
    s.setValue("update/autoCheck", on);
}

QString Settings::updateChannel() const
{
    return s.value("update/channel", "stable").toString();
}

void Settings::setUpdateChannel(const QString &ch)
{
    s.setValue("update/channel", ch);
}

QList<GuardItem> Settings::guardList()
{
    QList<GuardItem> list;
    int size = s.beginReadArray("guardList");
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);
        GuardItem item;
        item.name    = s.value("name").toString();
        item.exePath = s.value("exePath").toString();
        item.enabled = s.value("enabled", true).toBool();
        list.append(item);
    }
    s.endArray();
    return list;
}

void Settings::setGuardList(const QList<GuardItem> &list)
{
    s.beginWriteArray("guardList", list.size());
    for (int i = 0; i < list.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("name",    list[i].name);
        s.setValue("exePath", list[i].exePath);
        s.setValue("enabled", list[i].enabled);
    }
    s.endArray();
}

QString Settings::theme() const
{
    return s.value("ui/theme", "system").toString();
}

void Settings::setTheme(const QString &t)
{
    s.setValue("ui/theme", t);
}

int Settings::smartGuardCpuThreshold() const
{
    return s.value("guard/cpuThreshold", 80).toInt();
}

void Settings::setSmartGuardCpuThreshold(int pct)
{
    s.setValue("guard/cpuThreshold", pct);
}

int Settings::smartGuardMemThreshold() const
{
    return s.value("guard/memThreshold", 90).toInt();
}

void Settings::setSmartGuardMemThreshold(int pct)
{
    s.setValue("guard/memThreshold", pct);
}