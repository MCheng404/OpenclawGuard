#pragma once
#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

class MainWindow;

class TrayManager : public QObject
{
    Q_OBJECT
public:
    explicit TrayManager(MainWindow *window, QObject *parent = nullptr);

    void show();
    void showMessage(const QString &title, const QString &msg);

signals:
    void showWindowRequested();
    void quitRequested();
    void restartGatewayRequested();

private slots:
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void buildMenu();

    QSystemTrayIcon *m_tray = nullptr;
    QMenu           *m_menu = nullptr;
    MainWindow      *m_window = nullptr;
    QAction *m_showAction = nullptr;
    QAction *m_restartAction = nullptr;
    QAction *m_quitAction = nullptr;
};