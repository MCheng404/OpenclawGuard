#include "theme.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#ifndef DWMSBT_MAINWINDOW
#define DWMSBT_MAINWINDOW 2
#endif
#ifndef DWMSBT_ACRYLIC
#define DWMSBT_ACRYLIC 4
#endif

void Theme::enableMica(WId winId)
{
    HWND hwnd = reinterpret_cast<HWND>(winId);

    // Step 1: Extend DWM frame into entire client area so backdrop shows through
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    // Step 2: Dark title bar (must set BEFORE backdrop on some Win 11 builds)
    BOOL darkMode = (currentTheme == "dark");
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE,
                          &darkMode, sizeof(darkMode));

    // Step 3: Set Acrylic backdrop (more visible than Mica on Qt Widgets)
    DWM_SYSTEMBACKDROP_TYPE backdrop = static_cast<DWM_SYSTEMBACKDROP_TYPE>(DWMSBT_ACRYLIC);
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE,
                                        &backdrop, sizeof(backdrop));
    if (FAILED(hr)) {
        // Fallback: Mica
        backdrop = static_cast<DWM_SYSTEMBACKDROP_TYPE>(DWMSBT_MAINWINDOW);
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE,
                              &backdrop, sizeof(backdrop));
    }
}

void Theme::disableMica(WId winId)
{
    HWND hwnd = reinterpret_cast<HWND>(winId);
    DWM_SYSTEMBACKDROP_TYPE none = static_cast<DWM_SYSTEMBACKDROP_TYPE>(0);
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &none, sizeof(none));
}
#else
void Theme::enableMica(WId) {}
void Theme::disableMica(WId) {}
#endif
