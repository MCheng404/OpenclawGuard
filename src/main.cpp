#include <QApplication>
#include <QMessageBox>
#include <QSurfaceFormat>
#include <QThread>
#include "mainwindow.h"
#include "theme.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
    // GPU 加速渲染：使用 OpenGL
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(2);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName("OpenclawGuard");
    app.setOrganizationName("OpenclawGuard");
    app.setApplicationVersion(APP_VERSION);
    // DPI 缩放兼容
    app.setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    app.setStyle("windows11");

    // ── 单实例检测（Windows Named Mutex）──
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"OpenclawGuard_Instance");
    bool isRestart = qEnvironmentVariableIsSet("OPENCLAWGUARD_RESTART");
    if (isRestart) {
        // 主题切换重启：旧实例正在退出，等它释放 Mutex
        qunsetenv("OPENCLAWGUARD_RESTART");
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            for (int i = 0; i < 30; ++i) {
                QThread::msleep(100);
                ReleaseMutex(hMutex);
                CloseHandle(hMutex);
                hMutex = CreateMutexW(nullptr, TRUE, L"OpenclawGuard_Instance");
                if (GetLastError() != ERROR_ALREADY_EXISTS)
                    goto mutex_ok;
            }
            // 3 秒还没释放，直接接管
        }
    } else if (GetLastError() == ERROR_ALREADY_EXISTS) {
        QMessageBox::information(nullptr, "进程管理控制台",
                                 "程序已在运行中。");
        if (hMutex) { ReleaseMutex(hMutex); CloseHandle(hMutex); }
        return 0;
    }
    mutex_ok:

    Theme::applyTheme("dark");

    MainWindow w;
    w.show();
    int ret = app.exec();

    // 清理 Mutex
    if (hMutex) { ReleaseMutex(hMutex); CloseHandle(hMutex); }
    return ret;
}
