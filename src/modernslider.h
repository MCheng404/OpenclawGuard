#pragma once

#include <QSlider>
#include <QPropertyAnimation>
#include <QColor>

class ModernSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(qreal handleScale READ handleScale WRITE setHandleScale)

public:
    explicit ModernSlider(QWidget *parent = nullptr);

    void setColorTempStyle(bool on);
    void setAccentColor(const QColor &c);
    void setDarkMode(bool dark);

    qreal handleScale() const;
    void setHandleScale(qreal s);

    // 色温 → RGB 映射
    static QColor kelvinToRGB(int kelvin);

protected:
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private:
    int valueFromX(int x) const;
    void showValueTooltip();
    QWidget *m_tooltipBubble = nullptr;

    QPropertyAnimation *m_anim = nullptr;
    qreal m_scale = 1.0;
    bool m_pressed = false;
    bool m_colorTemp = false;
    bool m_dark = true;
    QColor m_accent = QColor(79, 140, 255);
};
