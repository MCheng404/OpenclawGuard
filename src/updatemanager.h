#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QList>
#include <QProcess>

struct UpdateInfo {
    QString version;
    QString tagName;
    QString name;
    QString downloadUrl;
    QString body;       // release notes
    QString publishedAt;
    bool    prerelease = false;  // true = beta
};

class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = nullptr);

    void fetchReleases();  // 拉取所有release
    void downloadAndInstall(const UpdateInfo &info);

    // Openclaw 更新相关功能（通过 npm）
    void performOpenclawUpdate(const QString &channel = QString());  // npm install -g openclaw@latest
    QString getCurrentVersion();   // openclaw --version
    QString getCurrentChannel();
    void getUpdateStatus();

    const QList<UpdateInfo>& allReleases() const { return m_allReleases; }
    const QList<UpdateInfo>& stableReleases() const { return m_stable; }
    const QList<UpdateInfo>& betaReleases() const { return m_beta; }

signals:
    void releasesReady();  // 拉取完成
    void fetchError(const QString &msg);
    void downloadProgress(int percent);
    void installReady(const QString &filePath);
    
    // Openclaw 更新信号
    void openclawUpdateAvailable(const QString &current, const QString &latest, bool available);
    void openclawUpdateStatus(const QString &statusJson);
    void openclawUpdateProgress(const QString &msg);
    void openclawUpdateFinished(bool success, const QString &msg);
    void openclawVersionReady(const QString &version);

private slots:
    void onReleasesReply(QNetworkReply *reply);
    void onDownloadReply(QNetworkReply *reply);
    void onOpenclawProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void classify();

    QNetworkAccessManager *m_nam = nullptr;
    QList<UpdateInfo> m_allReleases;
    QList<UpdateInfo> m_stable;
    QList<UpdateInfo> m_beta;
    QProcess *m_openclawProcess = nullptr;
};