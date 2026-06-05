#include "updatemanager.h"
#include <QVersionNumber>
#include "config.h"
#include "settings.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QRegularExpression>

namespace {
// 剥离 ANSI 转义序列（颜色、光标控制等）
QString cleanOutput(const QByteArray &raw)
{
    QString s = QString::fromUtf8(raw).trimmed();
    static QRegularExpression ansiRe(QStringLiteral("\\x1b\\[[0-9;]*[a-zA-Z]"));
    s.remove(ansiRe);
    return s.trimmed();
}

QString extractJsonObject(const QString &text)
{
    const int start = text.indexOf('{');
    const int end = text.lastIndexOf('}');
    if (start < 0 || end < start)
        return QString();
    return text.mid(start, end - start + 1).trimmed();
}

// 通过 cmd /c 运行 openclaw 命令（.cmd 脚本需要 cmd.exe 包装）
void startOpenclaw(QProcess *proc, const QStringList &args)
{
    QStringList cmdArgs = {"/c", "openclaw"};
    cmdArgs.append(args);
    proc->start("cmd", cmdArgs);
}

// 同步运行 openclaw 命令，返回清理后的 stdout
QString runOpenclawSync(const QStringList &args, int timeoutMs = 8000)
{
    QProcess proc;
    startOpenclaw(&proc, args);
    proc.waitForFinished(timeoutMs);
    QString output = cleanOutput(proc.readAllStandardOutput());
    if (output.isEmpty())
        output = cleanOutput(proc.readAllStandardError());
    return output;
}
}

UpdateManager::UpdateManager(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_openclawProcess(new QProcess(this))
{
    connect(m_openclawProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &UpdateManager::onOpenclawProcessFinished);
}

void UpdateManager::fetchReleases()
{
    DBG_LOG("开始获取更新列表...");
    QNetworkRequest req(QUrl(Config::UPDATE_API_BASE + "/releases?per_page=30"));
    req.setRawHeader("Accept", "application/vnd.github+json");
    req.setRawHeader("User-Agent", "OpenclawGuard/1.0");

    QString token = AppSettings.githubToken();
    if (token.isEmpty()) {
        token = qEnvironmentVariable("GITHUB_TOKEN");
        if (token.isEmpty())
            token = qEnvironmentVariable("GH_TOKEN");
    }
    if (!token.isEmpty())
        req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    auto *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReleasesReply(reply);
    });
}

void UpdateManager::onReleasesReply(QNetworkReply *reply)
{
    reply->deleteLater();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 403) {
        QString rateRemaining = reply->rawHeader("X-RateLimit-Remaining");
        if (rateRemaining == "0") {
            QString resetTime = reply->rawHeader("X-RateLimit-Reset");
            qint64 epoch = resetTime.toLongLong();
            QDateTime dt = QDateTime::fromSecsSinceEpoch(epoch);
            emit fetchError(QString("GitHub API 限流，重置时间: %1。\n可设置 GITHUB_TOKEN 环境变量解除限制。")
                                .arg(dt.toString("yyyy-MM-dd hh:mm:ss")));
        } else {
            QString token = AppSettings.githubToken();
            if (token.isEmpty()) {
                token = qEnvironmentVariable("GITHUB_TOKEN");
                if (token.isEmpty())
                    token = qEnvironmentVariable("GH_TOKEN");
            }
            if (token.isEmpty()) {
                emit fetchError("GitHub API 403 禁止访问。\n请在设置页面输入 GitHub Token（需具有 repo 权限的 Personal Access Token）。\n仓库 'Openclaw/Openclaw' 可能为私有仓库，需要有效 token 才能访问。");
            } else {
                emit fetchError(QString("GitHub API 403 禁止访问。\n当前 GitHub Token 可能无效或权限不足（需要 repo 权限）。\nToken 前缀: %1...\n请检查 Token 是否在设置页面正确保存。").arg(token.left(6)));
            }
        }
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit fetchError(QString("HTTP %1: %2").arg(statusCode).arg(reply->errorString()));
        return;
    }
    QByteArray data = reply->readAll();
    QJsonArray arr = QJsonDocument::fromJson(data).array();

    m_allReleases.clear();
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        UpdateInfo info;
        info.tagName     = obj["tag_name"].toString();
        info.name        = obj["name"].toString();
        info.version     = info.tagName.startsWith('v')
                            ? info.tagName.mid(1) : info.tagName;
        info.body        = obj["body"].toString();
        info.prerelease  = obj["prerelease"].toBool();
        info.publishedAt = obj["published_at"].toString();

        QJsonArray assets = obj["assets"].toArray();
        QString bestUrl;
        for (const QJsonValue &av : assets) {
            QJsonObject ao = av.toObject();
            QString name = ao["name"].toString().toLower();
            QString url = ao["browser_download_url"].toString();
            if (url.isEmpty()) continue;
            if (bestUrl.isEmpty() || name.endsWith(".exe") || name.endsWith(".msi"))
                bestUrl = url;
            if (name.endsWith(".exe") || name.endsWith(".msi"))
                break;
        }
        info.downloadUrl = bestUrl;

        if (!info.downloadUrl.isEmpty())
            m_allReleases.append(info);
    }

    classify();
    emit releasesReady();
}

void UpdateManager::classify()
{
    m_stable.clear();
    m_beta.clear();
    for (auto &r : m_allReleases) {
        if (r.prerelease) m_beta.append(r);
        else              m_stable.append(r);
    }
}

void UpdateManager::downloadAndInstall(const UpdateInfo &info)
{
    if (info.downloadUrl.isEmpty()) return;

    QNetworkRequest req(QUrl(info.downloadUrl));
    req.setRawHeader("Accept", "application/octet-stream");
    req.setRawHeader("User-Agent", "OpenclawGuard/1.0");

    QString token = AppSettings.githubToken();
    if (token.isEmpty()) {
        token = qEnvironmentVariable("GITHUB_TOKEN");
        if (token.isEmpty())
            token = qEnvironmentVariable("GH_TOKEN");
    }
    if (!token.isEmpty())
        req.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    auto *reply = m_nam->get(req);

    connect(reply, &QNetworkReply::downloadProgress, this,
            [this](qint64 rcvd, qint64 total) {
                if (total > 0) emit downloadProgress((rcvd * 100) / total);
            });

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onDownloadReply(reply);
    });
}

void UpdateManager::onDownloadReply(QNetworkReply *reply)
{
    reply->deleteLater();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 302 || statusCode == 301) {
        emit fetchError(QString("下载重定向(%1)，请检查网络").arg(statusCode));
        return;
    }
    if (statusCode == 403 || statusCode == 401) {
        emit fetchError("下载被拒绝(403/401)，可能需要 GitHub Token");
        return;
    }
    if (reply->error() != QNetworkReply::NoError) {
        emit fetchError("下载失败: " + reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    if (data.size() < 1024) {
        QString preview = QString::fromUtf8(data.left(200));
        emit fetchError("下载内容异常（太小，可能为错误页面）: " + preview);
        return;
    }

    QString contentType = reply->rawHeader("Content-Type");
    if (contentType.contains("text/html") || contentType.contains("application/json")) {
        emit fetchError("下载到非二进制内容(" + contentType + ")，可能为错误页面");
        return;
    }

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString fileName = reply->url().fileName();

    if (fileName.isEmpty()) {
        QString cd = QString::fromUtf8(reply->rawHeader("Content-Disposition"));
        QRegularExpression re(R"(filename[^;=\n]*=[\s'"]*([^;\n'"]+))",
                              QRegularExpression::CaseInsensitiveOption);
        auto match = re.match(cd);
        if (match.hasMatch())
            fileName = match.captured(1).trimmed();
    }
    if (fileName.isEmpty())
        fileName = "openclaw_update.exe";

    QString filePath = tempDir + "/" + fileName;

    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(data);
        f.close();
    } else {
        emit fetchError("无法写入文件: " + filePath);
        return;
    }

    QString lower = fileName.toLower();
    if (!lower.endsWith(".exe") && !lower.endsWith(".msi")) {
        emit fetchError(QString("下载完成但非可执行安装包 (%1)。\n请手动打开文件: %2").arg(fileName, filePath));
        return;
    }

    emit installReady(filePath);
}

// ============== Openclaw 更新相关功能（通过 npm） ==============

void UpdateManager::fetchLatestVersionNpm(const QString &channel)
{
    // 同时查 stable 和 beta，取最新版本
    QString stableVer, betaVer;

    QProcess proc1;
    proc1.start("cmd", {"/c", "npm", "view", "openclaw", "version"});
    proc1.waitForFinished(15000);
    stableVer = cleanOutput(proc1.readAllStandardOutput()).trimmed();

    QProcess proc2;
    proc2.start("cmd", {"/c", "npm", "view", "openclaw@beta", "version"});
    proc2.waitForFinished(15000);
    betaVer = cleanOutput(proc2.readAllStandardOutput()).trimmed();

    // 去掉可能的 "error" 响应
    if (stableVer.contains("error", Qt::CaseInsensitive)) stableVer.clear();
    if (betaVer.contains("error", Qt::CaseInsensitive)) betaVer.clear();

    if (stableVer.isEmpty() && betaVer.isEmpty()) {
        emit latestVersionFetched(QString(), "npm 查询失败");
        return;
    }

    // 比较版本，取更高版本
    auto parseVer = [](const QString &v) -> QVersionNumber {
        return QVersionNumber::fromString(v);
    };

    QString best = stableVer;
    if (!betaVer.isEmpty()) {
        if (best.isEmpty() || parseVer(betaVer) > parseVer(best))
            best = betaVer;
    }

    // 同时返回 stable 和 beta 信息
    emit latestVersionFetched(best, QString(), stableVer, betaVer);
}

void UpdateManager::performOpenclawUpdate(const QString &channel)
{
    if (m_openclawProcess->state() != QProcess::NotRunning) {
        emit openclawUpdateFinished(false, "已有更新任务正在运行");
        return;
    }

    QString pkg = "openclaw@latest";
    if (!channel.isEmpty() && channel.toLower() == "beta") {
        pkg = "openclaw@beta";
    }

    emit openclawUpdateProgress(QString("正在通过 npm 安装 %1 ...").arg(pkg));
    // npm 是 .cmd 脚本，需要 cmd /c
    m_openclawProcess->start("cmd", {"/c", "npm", "install", "-g", pkg, "--yes"});
    if (!m_openclawProcess->waitForStarted(5000))
        emit openclawUpdateFinished(false, "无法启动 npm，请确认 Node.js 已安装且 npm 在 PATH 中");
}

QString UpdateManager::getCurrentVersion()
{
    return runOpenclawSync({"--version"}, 5000);
}

QString UpdateManager::getCurrentChannel()
{
    QString output = runOpenclawSync({"update", "status", "--json"}, 8000);
    QString json = extractJsonObject(output);
    if (json.isEmpty()) return "unknown";

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isObject()) {
        const QJsonObject ch = doc.object()["channel"].toObject();
        const QString value = ch["value"].toString();
        if (!value.isEmpty())
            return value;
    }
    return "unknown";
}

void UpdateManager::getUpdateStatus()
{
    QProcess proc;
    startOpenclaw(&proc, {"update", "status", "--json"});
    proc.waitForFinished(10000);

    QString output = cleanOutput(proc.readAllStandardOutput());
    QString err = cleanOutput(proc.readAllStandardError());

    // stdout 为空时尝试 stderr（openclaw 有时混合输出）
    if (output.isEmpty())
        output = err;

    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        emit openclawUpdateFinished(false, output.isEmpty() ? "命令执行失败" : output);
        return;
    }

    QString json = extractJsonObject(output);
    if (json.isEmpty()) {
        // 最后尝试：合并 stdout + stderr 再提取
        json = extractJsonObject(output + "\n" + err);
    }
    if (json.isEmpty()) {
        emit openclawUpdateFinished(false,
            QString("未获取到有效 JSON 输出。\n原始输出:\n%1").arg(output.left(500)));
        return;
    }

    emit openclawUpdateStatus(json);
}

void UpdateManager::onOpenclawProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString stdOut = cleanOutput(m_openclawProcess->readAllStandardOutput());
    QString stdErr = cleanOutput(m_openclawProcess->readAllStandardError());

    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        emit openclawUpdateFinished(false, stdErr.isEmpty() ? stdOut : stdErr);
        return;
    }

    // npm install 输出成功后，刷新状态
    QString full = (stdOut + "\n" + stdErr).trimmed();
    emit openclawUpdateFinished(true, full);
}
