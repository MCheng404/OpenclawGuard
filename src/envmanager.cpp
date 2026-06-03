#include "envmanager.h"
#include "config.h"
#include <QDesktopServices>
#include <QRegularExpression>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

EnvironmentManager::EnvironmentManager(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    connect(m_nam, &QNetworkAccessManager::finished, this,
            [this](QNetworkReply *reply) {
        reply->deleteLater();
        QString url = reply->url().toString();
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        //   重定向: GitHub /releases/latest → 真实 URL 含版本号
        if (statusCode == 302 || statusCode == 301) {
            QString loc = reply->rawHeader("Location");
            if (!loc.isEmpty()) {
                auto redirectReq = QNetworkRequest{QUrl(loc)};
                redirectReq.setRawHeader("User-Agent", "OpenclawGuard/1.0");
                m_nam->get(redirectReq, {});
                return; //   不 count down，等 redirect reply
            }
        }

        if (statusCode == 403) {
            m_pendingChecks--;
            if (m_pendingChecks <= 0) emit allLatestChecked();
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            m_pendingChecks--;
            if (m_pendingChecks <= 0) emit allLatestChecked();
            return;
        }

        QString body = QString::fromUtf8(reply->readAll());
        QString finalUrl = reply->url().toString();

        // ── Node.js: https://nodejs.org/en/download/current → 页面标题含 v26.2.0 ──
        if (url.contains("nodejs.org/en/download/current") ||
            finalUrl.contains("nodejs.org/en/download/")) {
            static QRegularExpression re(R"(v(\d+\.\d+\.\d+))");
            QRegularExpressionMatch m = re.match(body);
            QString latest = m.hasMatch() ? m.captured(1) : QString();
            if (!latest.isEmpty()) {
                for (auto &env : m_envs) {
                    if (env.name == "Node.js") {
                        env.latestVersion = latest;
                        env.updateAvailable = isNewer(extractVersion(latest), extractVersion(env.version));
                        emit latestVersionChecked(env.name, latest, env.updateAvailable);
                    }
                }
            }
            m_pendingChecks--;
        }
        // ── npm: https://github.com/npm/cli/releases/latest → 302 → 含 v11.16.0 ──
        else if (url.contains("github.com/npm/cli") ||
                 finalUrl.contains("github.com/npm/cli/releases/tag")) {
            static QRegularExpression re(R"(v?(\d+\.\d+\.\d+))");
            //   从 302 Location 或 body 提取
            QString latest;
            QRegularExpressionMatch m = re.match(finalUrl);
            if (m.hasMatch()) {
                latest = m.captured(1);
            } else {
                m = re.match(body);
                if (m.hasMatch()) latest = m.captured(1);
            }
            if (!latest.isEmpty()) {
                for (auto &env : m_envs) {
                    if (env.name == "npm") {
                        env.latestVersion = latest;
                        env.updateAvailable = isNewer(extractVersion(latest), extractVersion(env.version));
                        emit latestVersionChecked(env.name, latest, env.updateAvailable);
                    }
                }
            }
            m_pendingChecks--;
        }
        // ── Python: https://www.python.org/api/v2/downloads/release/ ──
        else if (url.contains("python.org/api")) {
            QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8());
            QString latest;
            if (doc.isArray()) {
                QJsonArray arr = doc.array();
                for (const auto &val : arr) {
                    QJsonObject obj = val.toObject();
                    QString name = obj["name"].toString();
                    if (!name.contains('a') && !name.contains('b') && !name.contains("rc")
                        && name.startsWith("3.")) {
                        latest = name;
                        break;
                    }
                }
            }
            for (auto &env : m_envs) {
                if (env.name == "Python" && !latest.isEmpty()) {
                    env.latestVersion = latest;
                    env.updateAvailable = isNewer(extractVersion(latest), extractVersion(env.version));
                    emit latestVersionChecked(env.name, latest, env.updateAvailable);
                }
            }
            m_pendingChecks--;
        }
        // ── Git: https://github.com/git-for-windows/git/releases/latest ──
        else if (url.contains("git-for-windows/git") ||
                 finalUrl.contains("git-for-windows/git/releases/tag")) {
            static QRegularExpression re(R"(v?(\d+\.\d+\.\d+))");
            QString latest;
            QRegularExpressionMatch m = re.match(finalUrl);
            if (m.hasMatch()) {
                latest = m.captured(1);
            } else {
                m = re.match(body);
                if (m.hasMatch()) latest = m.captured(1);
            }
            for (auto &env : m_envs) {
                if (env.name == "Git" && !latest.isEmpty()) {
                    env.latestVersion = latest;
                    env.updateAvailable = isNewer(extractVersion(latest), extractVersion(env.version));
                    emit latestVersionChecked(env.name, latest, env.updateAvailable);
                }
            }
            m_pendingChecks--;
        }
        // ── .NET SDK: https://dotnet.microsoft.com/en-us/download ──
        else if (url.contains("dotnet.microsoft.com")) {
            static QRegularExpression re(R"(SDK\s+version\s+(\d+\.\d+\.\d+))");
            QRegularExpressionMatch m = re.match(body);
            QString latest = m.hasMatch() ? m.captured(1) : QString();
            if (!latest.isEmpty()) {
                for (auto &env : m_envs) {
                    if (env.name == ".NET SDK") {
                        env.latestVersion = latest;
                        env.updateAvailable = isNewer(extractVersion(latest), extractVersion(env.version));
                        emit latestVersionChecked(env.name, latest, env.updateAvailable);
                    }
                }
            }
            m_pendingChecks--;
        }
        // ── PowerShell: https://github.com/PowerShell/PowerShell/releases/latest ──
        else if (url.contains("github.com/PowerShell/PowerShell") ||
                 finalUrl.contains("PowerShell/PowerShell/releases/tag")) {
            static QRegularExpression re(R"(v?(\d+\.\d+\.\d+))");
            QString latest;
            QRegularExpressionMatch m = re.match(finalUrl);
            if (m.hasMatch()) {
                latest = m.captured(1);
            } else {
                m = re.match(body);
                if (m.hasMatch()) latest = m.captured(1);
            }
            for (auto &env : m_envs) {
                if (env.name == "PowerShell" && !latest.isEmpty()) {
                    env.latestVersion = latest;
                    env.updateAvailable = isNewer(extractVersion(latest), extractVersion(env.version));
                    emit latestVersionChecked(env.name, latest, env.updateAvailable);
                }
            }
            m_pendingChecks--;
        }
        // ── CMake: https://github.com/Kitware/CMake/releases/latest ──
        else if (url.contains("github.com/Kitware/CMake") ||
                 finalUrl.contains("Kitware/CMake/releases/tag")) {
            static QRegularExpression re(R"(v?(\d+\.\d+\.\d+))");
            QString latest;
            QRegularExpressionMatch m = re.match(finalUrl);
            if (m.hasMatch()) {
                latest = m.captured(1);
            } else {
                m = re.match(body);
                if (m.hasMatch()) latest = m.captured(1);
            }
            for (auto &env : m_envs) {
                if (env.name == "CMake" && !latest.isEmpty()) {
                    env.latestVersion = latest;
                    env.updateAvailable = isNewer(extractVersion(latest), extractVersion(env.version));
                    emit latestVersionChecked(env.name, latest, env.updateAvailable);
                }
            }
            m_pendingChecks--;
        }
        else {
            m_pendingChecks--;
        }

        if (m_pendingChecks <= 0)
            emit allLatestChecked();
    });
}

// ══════════════ 本地检测 ══════════════

void EnvironmentManager::detectAll()
{
    m_envs.clear();
    m_envs.append(detectNode());
    m_envs.append(detectNpm());
    m_envs.append(detectPython());
    m_envs.append(detectGit());
    m_envs.append(detectDotNet());
    m_envs.append(detectJava());
    m_envs.append(detectPowerShell());
    m_envs.append(detectCmake());
    emit detectionFinished();
}

// ── 版本号提取 ──
QVersionNumber EnvironmentManager::extractVersion(const QString &raw)
{
    static QRegularExpression re(R"((\d+)\.(\d+)\.(\d+))");
    QRegularExpressionMatch m = re.match(raw);
    if (m.hasMatch())
        return QVersionNumber(m.captured(1).toInt(), m.captured(2).toInt(), m.captured(3).toInt());
    return {};
}

bool EnvironmentManager::isNewer(const QVersionNumber &latest, const QVersionNumber &current)
{
    if (latest.isNull() || current.isNull()) return false;
    return latest > current;
}

// ── 联网查最新版本 ──
void EnvironmentManager::checkLatestVersions()
{
    QNetworkRequest req;
    req.setRawHeader("User-Agent", "OpenclawGuard/1.0");

    // 1) Node.js — 页面标题直接写版本
    req.setUrl(QUrl("https://nodejs.org/en/download/current"));
    m_nam->get(req);

    // 2) npm — GitHub /releases/latest → 302 重定向
    QNetworkRequest npmReq(QUrl("https://github.com/npm/cli/releases/latest"));
    npmReq.setRawHeader("User-Agent", "OpenclawGuard/1.0");
    m_nam->get(npmReq);

    // 3) Python — 官方 JSON API
    QNetworkRequest pyReq(QUrl("https://www.python.org/api/v2/downloads/release/?is_published=true&page_size=5"));
    pyReq.setRawHeader("User-Agent", "OpenclawGuard/1.0");
    m_nam->get(pyReq);

    // 4) Git — GitHub /releases/latest
    QNetworkRequest gitReq(QUrl("https://github.com/git-for-windows/git/releases/latest"));
    gitReq.setRawHeader("User-Agent", "OpenclawGuard/1.0");
    m_nam->get(gitReq);

    // 5) .NET SDK — 官方下载页
    QNetworkRequest dotReq(QUrl("https://dotnet.microsoft.com/en-us/download"));
    dotReq.setRawHeader("User-Agent", "OpenclawGuard/1.0");
    m_nam->get(dotReq);

    // 6) PowerShell — GitHub /releases/latest
    QNetworkRequest psReq(QUrl("https://github.com/PowerShell/PowerShell/releases/latest"));
    psReq.setRawHeader("User-Agent", "OpenclawGuard/1.0");
    m_nam->get(psReq);

    // 7) CMake — GitHub /releases/latest
    QNetworkRequest cmReq(QUrl("https://github.com/Kitware/CMake/releases/latest"));
    cmReq.setRawHeader("User-Agent", "OpenclawGuard/1.0");
    m_nam->get(cmReq);

    m_pendingChecks = 7;
}

// ══════════════ 各工具检测 ══════════════

QString EnvironmentManager::runAndGetOutput(const QString &exe, const QStringList &args)
{
    QProcess proc;
    proc.start(exe, args);
    proc.waitForFinished(1500);
    return QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
}

QString EnvironmentManager::findExe(const QString &name)
{
    QString found = QStandardPaths::findExecutable(name);
    if (!found.isEmpty()) return QDir::toNativeSeparators(found);
    return {};
}

EnvInfo EnvironmentManager::detectNode()
{
    EnvInfo info;
    info.name = "Node.js";
    info.exePath = findExe("node.exe");
    if (!info.exePath.isEmpty()) {
        info.installed = true;
        info.version = runAndGetOutput(info.exePath, {"-v"}).remove('\r').remove("v");
        int major = extractVersion(info.version).majorVersion();
        info.updateCmd = "winget upgrade OpenJS.NodeJS --accept-source-agreements --accept-package-agreements";
    }
    return info;
}

EnvInfo EnvironmentManager::detectNpm()
{
    EnvInfo info;
    info.name = "npm";
    info.exePath = findExe("npm.cmd");
    if (info.exePath.isEmpty()) info.exePath = findExe("npm");
    if (!info.exePath.isEmpty()) {
        info.installed = true;
        info.version = runAndGetOutput(info.exePath, {"-v"}).remove('\r');
        info.updateCmd = "npm install -g npm@latest --yes";
    }
    return info;
}

EnvInfo EnvironmentManager::detectPython()
{
    EnvInfo info;
    info.name = "Python";
    info.exePath = findExe("python.exe");
    if (info.exePath.isEmpty()) info.exePath = findExe("python3.exe");
    if (!info.exePath.isEmpty()) {
        info.installed = true;
        info.version = runAndGetOutput(info.exePath, {"--version"}).remove('\r');
        if (info.version.isEmpty())
            info.version = runAndGetOutput(info.exePath, {"-V"}).remove('\r');
        info.version = info.version.remove("Python ");
        int major = extractVersion(info.version).majorVersion();
        if (major == 3) {
            int minor = extractVersion(info.version).minorVersion();
            info.updateCmd = QString("winget upgrade Python.Python.3.%1 --accept-source-agreements --accept-package-agreements").arg(minor);
        } else {
            info.updateCmd = "winget upgrade Python.Python.3 --accept-source-agreements --accept-package-agreements";
        }
    }
    return info;
}

EnvInfo EnvironmentManager::detectGit()
{
    EnvInfo info;
    info.name = "Git";
    info.exePath = findExe("git.exe");
    if (!info.exePath.isEmpty()) {
        info.installed = true;
        QString raw = runAndGetOutput(info.exePath, {"--version"}).remove('\r');
        // "git version 2.54.0.windows.1" →  "2.54.0"
        info.version = raw.remove("git version ").remove(".windows.1");
        info.updateCmd = "winget upgrade Git.Git --accept-source-agreements --accept-package-agreements";
    }
    return info;
}

EnvInfo EnvironmentManager::detectDotNet()
{
    EnvInfo info;
    info.name = ".NET SDK";
    info.exePath = findExe("dotnet.exe");
    if (!info.exePath.isEmpty()) {
        info.installed = true;
        info.version = runAndGetOutput(info.exePath, {"--version"}).remove('\r');
        info.updateCmd = "winget upgrade Microsoft.DotNet.SDK.10 --accept-source-agreements --accept-package-agreements";
    }
    return info;
}

EnvInfo EnvironmentManager::detectJava()
{
    EnvInfo info;
    info.name = "Java";
    info.exePath = findExe("java.exe");
    if (!info.exePath.isEmpty()) {
        info.installed = true;
        QString raw = runAndGetOutput(info.exePath, {"--version"}).remove('\r');
        // "openjdk 26.0.1 2026-..." →  "26.0.1"
        static QRegularExpression re(R"((\d+\.\d+\.\d+))");
        QRegularExpressionMatch m = re.match(raw);
        info.version = m.hasMatch() ? m.captured(1) : raw;
        info.updateCmd = "winget upgrade Microsoft.OpenJDK.21 --accept-source-agreements --accept-package-agreements";
    }
    return info;
}

EnvInfo EnvironmentManager::detectPowerShell()
{
    EnvInfo info;
    info.name = "PowerShell";
    info.exePath = findExe("pwsh.exe");
    if (!info.exePath.isEmpty()) {
        info.installed = true;
        info.version = runAndGetOutput(info.exePath, {"-NoProfile", "-Command", "$PSVersionTable.PSVersion.ToString()"});
        info.updateCmd = "winget upgrade Microsoft.PowerShell --accept-source-agreements --accept-package-agreements";
    } else {
        //   只有 Windows 内置 5.1，不算"已安装"
        info.version = "5.1 (内置)";
        info.installed = false;
    }
    return info;
}

EnvInfo EnvironmentManager::detectCmake()
{
    EnvInfo info;
    info.name = "CMake";
    info.exePath = findExe("cmake.exe");
    if (!info.exePath.isEmpty()) {
        info.installed = true;
        QString raw = runAndGetOutput(info.exePath, {"--version"}).remove('\r');
        // "cmake version 4.2.1" →  "4.2.1"
        info.version = raw.remove("cmake version ").section('\n', 0, 0);
        info.updateCmd = "winget upgrade Kitware.CMake --accept-source-agreements --accept-package-agreements";
    }
    return info;
}

// ══════════════ 更新（异步） ══════════════

void EnvironmentManager::updateEnv(const QString &name)
{
    if (isUpdating()) {
        emit updateFinished(name, false, "已有更新任务正在运行，请等待完成");
        return;
    }

    QString cmd;
    for (const auto &e : m_envs) {
        if (e.name == name) { cmd = e.updateCmd; break; }
    }
    if (cmd.isEmpty()) {
        emit updateFinished(name, false, "无可用更新命令");
        return;
    }

    if (cmd.startsWith("start ")) {
        QDesktopServices::openUrl(QUrl(cmd.mid(6)));
        emit updateFinished(name, true, "已打开下载页面");
        return;
    }

    //   异步执行
    m_updateTarget = name;
    m_updateProc = new QProcess(this);
    m_updateProc->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_updateProc, &QProcess::readyRead, this, &EnvironmentManager::onUpdateReadyRead);
    connect(m_updateProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &EnvironmentManager::onUpdateFinished);

    emit updateStarted(name);
    m_updateProc->start("cmd", {"/c", cmd});
}

void EnvironmentManager::onUpdateReadyRead()
{
    if (!m_updateProc) return;
    QString line = QString::fromUtf8(m_updateProc->readAllStandardOutput()).trimmed();
    if (!line.isEmpty())
        emit updateProgress(m_updateTarget, line);
}

void EnvironmentManager::onUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!m_updateProc) return;

    QString remaining = QString::fromUtf8(m_updateProc->readAllStandardOutput()).trimmed();
    bool ok = (exitStatus == QProcess::NormalExit && exitCode == 0);
    QString msg = ok ? "更新成功，请重启终端"
                     : (remaining.isEmpty() ? QString("更新失败 (exit %1)").arg(exitCode) : remaining);

    emit updateFinished(m_updateTarget, ok, msg);

    m_updateProc->deleteLater();
    m_updateProc = nullptr;
    m_updateTarget.clear();
}
