#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVersionNumber>

struct EnvInfo {
    QString name;            //   "Node.js", "Python", "Git"
    QString exePath;         //   "C:\\Program Files\\nodejs\\node.exe"
    QString version;         //   "v20.11.0"
    bool    installed = false;
    QString latestVersion;   //   联网查到的
    bool    updateAvailable = false;
    QString updateCmd;       //   升级命令
};

class EnvironmentManager : public QObject
{
    Q_OBJECT
public:
    explicit EnvironmentManager(QObject *parent = nullptr);

    void detectAll();
    void checkLatestVersions();

    //   单项检测
    EnvInfo detectNode();
    EnvInfo detectNpm();
    EnvInfo detectPython();
    EnvInfo detectGit();
    EnvInfo detectDotNet();
    EnvInfo detectJava();
    EnvInfo detectPowerShell();
    EnvInfo detectCmake();

    const QList<EnvInfo>& allEnvs() const { return m_envs; }

    void updateEnv(const QString &name);
    bool isUpdating() const { return m_updateProc && m_updateProc->state() != QProcess::NotRunning; }

signals:
    void detectionFinished();
    void latestVersionChecked(const QString &name, const QString &latest, bool newer);
    void allLatestChecked();
    void updateStarted(const QString &name);
    void updateProgress(const QString &name, const QString &line);  //   实时输出
    void updateFinished(const QString &name, bool success, const QString &msg);

private slots:
    void onUpdateReadyRead();
    void onUpdateFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString runAndGetOutput(const QString &exe, const QStringList &args);
    QString findExe(const QString &name);

    static QVersionNumber extractVersion(const QString &raw);
    static bool isNewer(const QVersionNumber &latest, const QVersionNumber &current);

    QList<EnvInfo> m_envs;
    QNetworkAccessManager *m_nam = nullptr;
    int m_pendingChecks = 0;

    //   异步更新
    QProcess *m_updateProc = nullptr;
    QString m_updateTarget;
};
