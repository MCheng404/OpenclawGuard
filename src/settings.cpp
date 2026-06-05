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

int Settings::colorTemperature() const
{
    return s.value("ui/colorTemperature", 6500).toInt();
}

void Settings::setColorTemperature(int kelvin)
{
    s.setValue("ui/colorTemperature", kelvin);
}

int Settings::cardOpacity() const { return s.value("ui/cardOpacity", 100).toInt(); }
void Settings::setCardOpacity(int pct) { s.setValue("ui/cardOpacity", pct); }
int Settings::cardRadius() const { return s.value("ui/cardRadius", 16).toInt(); }
void Settings::setCardRadius(int px) { s.setValue("ui/cardRadius", px); }
int Settings::shadowIntensity() const { return s.value("ui/shadowIntensity", 50).toInt(); }
void Settings::setShadowIntensity(int pct) { s.setValue("ui/shadowIntensity", pct); }
QString Settings::accentColor() const { return s.value("ui/accentColor", "#4f8cff").toString(); }
void Settings::setAccentColor(const QString &hex) { s.setValue("ui/accentColor", hex); }

bool Settings::liquidGlassEnabled() const { return s.value("ui/liquidGlass", false).toBool(); }
void Settings::setLiquidGlassEnabled(bool on) { s.setValue("ui/liquidGlass", on); }
int Settings::liquidGlassBlur() const { return s.value("ui/glassBlur", 18).toInt(); }
void Settings::setLiquidGlassBlur(int r) { s.setValue("ui/glassBlur", r); }
int Settings::liquidGlassRefraction() const { return s.value("ui/glassRefraction", 45).toInt(); }
void Settings::setLiquidGlassRefraction(int pct) { s.setValue("ui/glassRefraction", pct); }
int Settings::liquidGlassGlow() const { return s.value("ui/glassGlow", 35).toInt(); }
void Settings::setLiquidGlassGlow(int pct) { s.setValue("ui/glassGlow", pct); }
int Settings::liquidGlassNoise() const { return s.value("ui/glassNoise", 4).toInt(); }
void Settings::setLiquidGlassNoise(int pct) { s.setValue("ui/glassNoise", pct); }

QByteArray Settings::windowGeometry() const
{
    return s.value("ui/geometry").toByteArray();
}

void Settings::setWindowGeometry(const QByteArray &g)
{
    s.setValue("ui/geometry", g);
}

bool Settings::closeHintShown() const
{
    return s.value("ui/closeHintShown", false).toBool();
}

void Settings::setCloseHintShown(bool v)
{
    s.setValue("ui/closeHintShown", v);
}