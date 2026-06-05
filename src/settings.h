#pragma once
#include <QSettings>
#include <QString>
#include <QStringList>
#include "config.h"

struct GuardItem {
    QString name;
    QString exePath;
    bool    enabled = true;
};

class Settings
{
public:
    static Settings& instance();

    // 网关路径
    QString gatewayPath() const;
    void    setGatewayPath(const QString &path);

    // 网关端口
    int  gatewayPort() const;
    void setGatewayPort(int port);

    // 自动重启网关
    bool autoRestartGateway() const;
    void setAutoRestartGateway(bool on);

    // 启动时最小化
    bool startMinimized() const;
    void setStartMinimized(bool on);

    // 开机自启
    bool autoStart() const;
    void setAutoStart(bool on);

    // 自动检查更新
    bool autoCheckUpdate() const;
    void setAutoCheckUpdate(bool on);

    // GitHub Token
    QString githubToken() const;
    void    setGithubToken(const QString &token);

    // 更新通道 (stable / beta)
    QString updateChannel() const;
    void    setUpdateChannel(const QString &ch);

    // 自定义守护列表
    QList<GuardItem> guardList();
    void             setGuardList(const QList<GuardItem> &list);

    // 主题 (system / light / dark)
    QString theme() const;
    void    setTheme(const QString &t);

    // 智能拉起阈值
    int  smartGuardCpuThreshold() const;
    void setSmartGuardCpuThreshold(int pct);
    int  smartGuardMemThreshold() const;
    void setSmartGuardMemThreshold(int pct);

    // 色温 (Kelvin, 3000-7000, 默认 6500)
    int  colorTemperature() const;
    void setColorTemperature(int kelvin);

    // UI 自定义
    int  cardOpacity() const;        // 0-100, 默认 100
    void setCardOpacity(int pct);
    int  cardRadius() const;         // 4-24, 默认 16
    void setCardRadius(int px);
    int  shadowIntensity() const;    // 0-100, 默认 50
    void setShadowIntensity(int pct);
    QString accentColor() const;     // hex, 默认 #4f8cff
    void setAccentColor(const QString &hex);

    // Liquid Glass
    bool liquidGlassEnabled() const;
    void setLiquidGlassEnabled(bool on);
    int  liquidGlassBlur() const;        // 4-40, 默认 20
    void setLiquidGlassBlur(int r);
    int  liquidGlassTint() const;        // 0-80, 默认 30
    void setLiquidGlassTint(int pct);

    // 窗口状态
    QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray &g);
    bool closeHintShown() const;
    void setCloseHintShown(bool v);

private:
    Settings();
    QSettings s;
};

#define AppSettings Settings::instance()