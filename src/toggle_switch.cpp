#include "toggle_switch.h"
#include <QMouseEvent>
#include <QEasingCurve>
#include <QPainterPath>

ToggleSwitch::ToggleSwitch(QWidget *parent)
    : QAbstractButton(parent)
{
    setCheckable(true);
    setChecked(false);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(44, 24);

    m_anim = new QPropertyAnimation(this, "thumbPos");
    m_anim->setEasingCurve(QEasingCurve::OutElastic);
    m_anim->setDuration(450);
}

void ToggleSwitch::setTrackColors(const QColor &onBase, const QColor &onHover,
                                   const QColor &offBase, const QColor &offHover)
{
    m_trackOnBase  = onBase;
    m_trackOnHover = onHover;
    m_trackOffBase  = offBase;
    m_trackOffHover = offHover;
    update();
}

void ToggleSwitch::setThumbPos(qreal pos)
{
    m_thumbPos = pos;
    update();
}

void ToggleSwitch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const bool on = isChecked();
    const int w = width();
    const int h = height();
    const qreal trackRadius = h / 2.0;
    const QRectF trackRect(0.5, 0.5, w - 1.0, h - 1.0);

    QColor trackColor = on
        ? (m_hovered ? m_trackOnHover : m_trackOnBase)
        : (m_hovered ? m_trackOffHover : m_trackOffBase);
    QColor borderColor = on
        ? trackColor.lighter(122)
        : QColor(255, 255, 255, 26);
    if (!on && trackColor.lightness() > 170)
        borderColor = QColor(15, 23, 42, 28);

    p.setPen(QPen(borderColor, 1.0));
    p.setBrush(trackColor);
    p.drawRoundedRect(trackRect, trackRadius, trackRadius);

    const qreal thumbD = 18.0;
    const qreal thumbY = (h - thumbD) / 2.0;
    const QRectF shadowRect(m_thumbPos + 0.5, thumbY + 1.5, thumbD, thumbD);
    const QRectF thumbRect(m_thumbPos, thumbY, thumbD, thumbD);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, on ? 44 : 30));
    p.drawEllipse(shadowRect);

    p.setPen(QPen(QColor(255, 255, 255, 210), 1.0));
    p.setBrush(QColor("#ffffff"));
    p.drawEllipse(thumbRect);
}

void ToggleSwitch::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && rect().contains(e->pos()))
    {
        toggle();
        animateToggle(isChecked());
        emit clicked();
    }
}

void ToggleSwitch::enterEvent(QEnterEvent *)
{
    m_hovered = true;
    update();
}

void ToggleSwitch::leaveEvent(QEvent *)
{
    m_hovered = false;
    update();
}

void ToggleSwitch::nextCheckState()
{
    // called by QAbstractButton when toggled
    animateToggle(!isChecked());
    QAbstractButton::nextCheckState();
}

void ToggleSwitch::animateToggle(bool checked)
{
    qreal from = checked ? 4.0 : 22.0;
    qreal to   = checked ? 22.0 : 4.0;

    m_anim->stop();
    m_anim->setStartValue(from);
    m_anim->setEndValue(to);
    m_anim->start();
}