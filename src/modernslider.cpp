#include "modernslider.h"
#include <QPainter>
#include <QPainterPath>
#include <QLabel>
#include <QEnterEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QTimer>

ModernSlider::ModernSlider(QWidget *parent)
    : QSlider(Qt::Horizontal, parent)
{
    setMouseTracking(true);
    setFixedHeight(32);
    // 隐藏默认 QSlider 绘制，全部自绘
    setStyleSheet("QSlider { background: transparent; }"
                  "QSlider::groove:horizontal { background: transparent; height: 0; }"
                  "QSlider::handle:horizontal { background: transparent; width: 0; height: 0; margin: 0; }");

    m_anim = new QPropertyAnimation(this, "handleScale", this);
    m_anim->setDuration(150);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
}

void ModernSlider::setColorTempStyle(bool on) { m_colorTemp = on; update(); }
void ModernSlider::setAccentColor(const QColor &c) { m_accent = c; update(); }
void ModernSlider::setDarkMode(bool dark) { m_dark = dark; update(); }

qreal ModernSlider::handleScale() const { return m_scale; }
void ModernSlider::setHandleScale(qreal s) { m_scale = s; update(); }

// 色温 Kelvin → RGB（Tanner Helland 算法）
QColor ModernSlider::kelvinToRGB(int kelvin)
{
    qreal temp = kelvin / 100.0;
    qreal r, g, b;

    // Red
    if (temp <= 66) r = 255;
    else { r = temp - 60; r = 329.698727446 * qPow(r, -0.1332047592); r = qBound(0.0, r, 255.0); }

    // Green
    if (temp <= 66) { g = temp; g = 99.4708025861 * qLn(g) - 161.1195681661; }
    else { g = temp - 60; g = 288.1221695283 * qPow(g, -0.0755148492); }
    g = qBound(0.0, g, 255.0);

    // Blue
    if (temp >= 66) b = 255;
    else if (temp <= 19) b = 0;
    else { b = temp - 10; b = 138.5177312231 * qLn(b) - 305.0447927307; b = qBound(0.0, b, 255.0); }

    return QColor((int)r, (int)g, (int)b);
}

int ModernSlider::valueFromX(int x) const
{
    const int handleR = 9;
    const int margin = handleR + 2;
    const qreal trackL = margin;
    const qreal trackR = width() - margin;
    const qreal trackW = trackR - trackL;
    if (trackW <= 0) return minimum();
    qreal ratio = (qreal)(x - trackL) / trackW;
    ratio = qBound(0.0, ratio, 1.0);
    return minimum() + qRound(ratio * (maximum() - minimum()));
}

void ModernSlider::showValueTooltip()
{
    const int margin = 11;
    const qreal trackW = width() - margin * 2;
    const qreal ratio = (qreal)(value() - minimum()) / qMax(1, maximum() - minimum());
    const qreal handleX = margin + trackW * ratio;
    QString tip = m_colorTemp ? QString("%1 K").arg(value()) : QString("%1%").arg(value());

    if (!m_tooltipBubble) {
        m_tooltipBubble = new QWidget(window(), Qt::ToolTip | Qt::FramelessWindowHint);
        m_tooltipBubble->setAttribute(Qt::WA_TranslucentBackground, true);
        m_tooltipBubble->setFixedSize(60, 28);
        auto *lbl = new QLabel(m_tooltipBubble);
        lbl->setObjectName("tipLabel");
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setGeometry(0, 0, 60, 28);
    }
    auto *lbl = m_tooltipBubble->findChild<QLabel*>("tipLabel");
    if (lbl) {
        lbl->setText(tip);
        lbl->setStyleSheet(m_dark
            ? "QLabel#tipLabel { background: #2a2d4a; color: #e2e4f0; border-radius: 8px; font-size: 12px; font-weight: 600; }"
            : "QLabel#tipLabel { background: #ffffff; color: #1e293b; border: 1px solid #e2e8f0; border-radius: 8px; font-size: 12px; font-weight: 600; }");
    }
    QPoint globalPos = mapToGlobal(QPoint((int)handleX - 30, -36));
    m_tooltipBubble->move(globalPos);
    m_tooltipBubble->show();
    m_tooltipBubble->raise();
}

void ModernSlider::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int trackH = 4;
    const int handleR = m_pressed ? 9 : (int)(8 * m_scale);
    const int margin = 11; // 固定 margin 避免 handle 缩放时轨道跳动
    const qreal trackY = height() / 2.0;
    const qreal trackL = margin;
    const qreal trackR = width() - margin;
    const qreal trackW = trackR - trackL;
    const qreal ratio = (qreal)(value() - minimum()) / qMax(1, maximum() - minimum());
    const qreal fillR = trackL + trackW * ratio;
    const qreal handleX = fillR;

    // --- 轨道背景 ---
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, m_dark ? 20 : 15));
    p.drawRoundedRect(QRectF(trackL, trackY - trackH / 2.0, trackW, trackH), trackH / 2.0, trackH / 2.0);

    // --- 已填充部分 ---
    if (m_colorTemp) {
        // 色温：根据当前位置对应色温值着色
        int curKelvin = value();
        QColor curColor = kelvinToRGB(curKelvin);
        QLinearGradient grad(trackL, 0, trackR, 0);
        grad.setColorAt(0.0, kelvinToRGB(minimum()));
        grad.setColorAt(ratio, curColor);
        grad.setColorAt(1.0, kelvinToRGB(maximum()));
        p.setBrush(grad);
    } else {
        QLinearGradient grad(trackL, 0, fillR, 0);
        grad.setColorAt(0.0, m_accent.lighter(120));
        grad.setColorAt(1.0, m_accent);
        p.setBrush(grad);
    }
    p.drawRoundedRect(QRectF(trackL, trackY - trackH / 2.0, fillR - trackL, trackH), trackH / 2.0, trackH / 2.0);

    // --- Handle 阴影 ---
    p.setBrush(QColor(0, 0, 0, m_dark ? 50 : 25));
    p.drawEllipse(QPointF(handleX, trackY + 1), handleR + 1.5, handleR + 1.5);

    // --- Handle 主体 ---
    if (m_colorTemp) {
        // 色温 handle 用当前色温颜色填充
        QColor tempColor = kelvinToRGB(value());
        p.setBrush(m_pressed ? tempColor.lighter(130) : tempColor.lighter(160));
        p.setPen(QPen(tempColor.darker(120), 2.0));
    } else {
        p.setBrush(m_pressed ? QColor(230, 240, 255) : QColor(255, 255, 255));
        p.setPen(QPen(m_accent, 2.0));
    }
    p.drawEllipse(QPointF(handleX, trackY), handleR, handleR);

    // --- Handle 内部小圆点（按下时）---
    if (m_pressed) {
        p.setPen(Qt::NoPen);
        p.setBrush(m_colorTemp ? kelvinToRGB(value()).darker(130) : m_accent);
        p.drawEllipse(QPointF(handleX, trackY), 3, 3);
    }
}

void ModernSlider::enterEvent(QEnterEvent *)
{
    m_anim->stop();
    m_anim->setStartValue(m_scale);
    m_anim->setEndValue(1.25);
    m_anim->start();
}

void ModernSlider::leaveEvent(QEvent *)
{
    m_anim->stop();
    m_anim->setStartValue(m_scale);
    m_anim->setEndValue(1.0);
    m_anim->start();
    if (m_tooltipBubble) m_tooltipBubble->hide();
}

void ModernSlider::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        int newVal = valueFromX(e->pos().x());
        setValue(newVal);
        m_pressed = true;
        update();
        showValueTooltip();
        e->accept();
    } else {
        QSlider::mousePressEvent(e);
    }
}

void ModernSlider::mouseReleaseEvent(QMouseEvent *e)
{
    m_pressed = false;
    update();
    if (m_tooltipBubble) m_tooltipBubble->hide();
    QSlider::mouseReleaseEvent(e);
}

void ModernSlider::mouseMoveEvent(QMouseEvent *e)
{
    if (m_pressed) {
        int newVal = valueFromX(e->pos().x());
        setValue(newVal);
        showValueTooltip();
        e->accept();
    } else {
        showValueTooltip();
        QSlider::mouseMoveEvent(e);
    }
}

void ModernSlider::wheelEvent(QWheelEvent *e)
{
    const int step = m_colorTemp ? 100 : 1;
    if (e->angleDelta().y() > 0)
        setValue(qMin(value() + step, maximum()));
    else if (e->angleDelta().y() < 0)
        setValue(qMax(value() - step, minimum()));
    showValueTooltip();
    e->accept();
}
