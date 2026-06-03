#pragma once
#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class PortMonitor : public QObject
{
    Q_OBJECT
public:
    explicit PortMonitor(QObject *parent = nullptr);

    void setPort(int port);
    int  port() const;
    void setInterval(int ms);
    void start();
    void stop();
    bool isOnline() const;
    void check();

signals:
    void onlineChanged(bool online);
    void wentOffline();
    void cameOnline();

private:
    void onHealthReply(QNetworkReply *reply);

    QNetworkAccessManager *m_nam = nullptr;
    QTimer     *m_timer  = nullptr;
    int         m_port   = 0;
    int         m_failCount = 0;
    bool        m_online = false;
};