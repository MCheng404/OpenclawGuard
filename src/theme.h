#pragma once
#include <QString>
#include <QColor>
#include <QApplication>
#include <QPalette>

struct ColorScheme {
    QColor sidebarBg;
    QColor mainBg;
    QColor cardBg;
    QColor accent;
    QColor accentHover;
    QColor success;
    QColor warning;
    QColor danger;
    QColor textPrimary;
    QColor textSecondary;
    QColor borderColor;
    QColor navIndicator;
};

class Theme
{
public:
    static ColorScheme darkColors();
    static ColorScheme lightColors();
    static ColorScheme currentColors();
    static QString detectSystemTheme();
    static void applyTheme(const QString &themeName);
    static void enableMica(WId winId);
    static void disableMica(WId winId);
    static QString darkStyleSheet();
    static QString lightStyleSheet();

private:
    static inline QString currentTheme = "dark";
};
