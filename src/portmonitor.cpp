#include "portmonitor.h"
#include "config.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

PortMonitor::PortMonitor(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &PortMonitor::check);
}

void PortMonitor::setPort(int port)
{
    if (m_port != port) {
        m_port = port;
        m_online = false;
        m_failCount = 0;
    }
}

int PortMonitor::port() const
{
    return m_port;
}

void PortMonitor::setInterval(int ms)
{
    m_timer->setInterval(ms);
}

void PortMonitor::start()
{
    if (!m_timer->isActive()) {
        m_timer->start();
        check();
    }
}

void PortMonitor::stop()
{
    m_timer->stop();
    m_failCount = 0;
    if (m_online) {
        m_online = false;
        emit onlineChanged(false);
        emit wentOffline();
    }
}

bool PortMonitor::isOnline() const
{
    return m_online;
}

void PortMonitor::check()
{
    if (m_port <= 0) return;

    QUrl url(QString("http://127.0.0.1:%1/").arg(m_port));
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setHeader(QNetworkRequest::UserAgentHeader, "OpenclawGuard/1.0");

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished,
            this, [this, reply]() { onHealthReply(reply); });
}

void PortMonitor::onHealthReply(QNetworkReply *reply)
{
    reply->deleteLater();

    bool ok = false;
    if (reply->error() == QNetworkReply::NoError) {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == 200) {
            ok = true;
        }
    }

    if (ok) {
        m_failCount = 0;
        if (!m_online) {
            m_online = true;
            emit onlineChanged(true);
            emit cameOnline();
        }
    } else {
        m_failCount++;
        if (m_failCount >= 3 && m_online) {
            m_online = false;
            emit onlineChanged(false);
            emit wentOffline();
        }
    }
}