#pragma once

#include <QAbstractButton>
#include <QPropertyAnimation>
#include <QPainter>

class ToggleSwitch : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(qreal thumbPos READ thumbPos WRITE setThumbPos)

public:
    explicit ToggleSwitch(QWidget *parent = nullptr);

    QSize sizeHint() const override { return QSize(44, 24); }
    QSize minimumSizeHint() const override { return QSize(44, 24); }

    qreal thumbPos() const { return m_thumbPos; }
    void setThumbPos(qreal pos);
    void setTrackColors(const QColor &onBase, const QColor &onHover,
                        const QColor &offBase, const QColor &offHover);

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void enterEvent(QEnterEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void nextCheckState() override;

private:
    qreal m_thumbPos = 4.0;
    bool m_hovered = false;
    QPropertyAnimation *m_anim = nullptr;
    QColor m_trackOnBase  = QColor("#4f8cff");
    QColor m_trackOnHover = QColor("#6ba0ff");
    QColor m_trackOffBase  = QColor("#2d2e45");
    QColor m_trackOffHover = QColor("#4a4d66");

    void animateToggle(bool checked);
};