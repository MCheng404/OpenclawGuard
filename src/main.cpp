#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include <QSurfaceFormat>
#include "mainwindow.h"
#include "theme.h"

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
    app.setApplicationVersion("1.1.1");
    // DPI 缩放兼容
    app.setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    app.setStyle("windows11");

    QSharedMemory shm("OpenclawGuard_Instance");
    if (!shm.create(1)) {
        QMessageBox::information(nullptr, "进程管理控制台",
                                 "程序已在运行中。");
        return 0;
    }

    Theme::applyTheme("dark");

    MainWindow w;
    w.show();
    return app.exec();
}
