#include "liquidglasscard.h"
#include "settings.h"

#include <QPainter>
#include <QPainterPath>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QApplication>
#include <QtMath>
#include <QTimer>

LiquidGlassCard::LiquidGlassCard(QWidget *parent)
    : QFrame(parent)
{
    setContentsMargins(20, 20, 20, 20);
    setAttribute(Qt::WA_TranslucentBackground);

    m_shadow = new QGraphicsDropShadowEffect(this);
    m_shadow->setBlurRadius(28);
    m_shadow->setOffset(0, 6);
    m_shadow->setColor(QColor(0, 0, 0, 50));
    setGraphicsEffect(m_shadow);

    m_hoverAnimObj = new QPropertyAnimation(this, "glassHover", this);
    m_hoverAnimObj->setDuration(200);
    m_hoverAnimObj->setEasingCurve(QEasingCurve::OutCubic);
}

void LiquidGlassCard::setShadowIntensity(int s)
{
    m_shadow->setBlurRadius(qBound(8, s * 56 / 100, 56));
    m_shadow->setColor(QColor(0, 0, 0, s * 80 / 100));
}

void LiquidGlassCard::refreshStyle()
{
    m_radius = AppSettings.cardRadius();
    m_opacity = AppSettings.cardOpacity();
    setShadowIntensity(AppSettings.shadowIntensity());
    invalidateCache();
}

void LiquidGlassCard::invalidateCache()
{
    m_glassReady = false;
    m_glassResult = QImage();
    update();
}

void LiquidGlassCard::showEvent(QShowEvent *) { invalidateCache(); }
void LiquidGlassCard::enterEvent(QEnterEvent *)
{
    m_hoverAnimObj->stop();
    m_hoverAnimObj->setStartValue(m_hoverAnim);
    m_hoverAnimObj->setEndValue(1.0);
    m_hoverAnimObj->start();
}
void LiquidGlassCard::leaveEvent(QEvent *)
{
    m_hoverAnimObj->stop();
    m_hoverAnimObj->setStartValue(m_hoverAnim);
    m_hoverAnimObj->setEndValue(0.0);
    m_hoverAnimObj->start();
}
void LiquidGlassCard::moveEvent(QMoveEvent *) { if (m_enabled) invalidateCache(); }
void LiquidGlassCard::resizeEvent(QResizeEvent *) { invalidateCache(); }

void LiquidGlassCard::scheduleRebuild()
{
    if (m_rebuildPending) return;
    m_rebuildPending = true;
    QTimer::singleShot(50, this, [this]() {
        m_rebuildPending = false;
        rebuildGlass();
        update();
    });
}

void LiquidGlassCard::rebuildGlass()
{
    // 找到顶层父窗口
    QWidget *root = parentWidget();
    while (root && !root->isWindow())
        root = root->parentWidget();
    if (!root) root = window();
    if (!root || !root->isVisible()) return;

    // ① 抓取整个窗口
    QPixmap full = root->grab();
    if (full.isNull()) return;

    // ② 裁剪卡片区域
    QPoint offset = mapTo(root, QPoint(0, 0));
    int ox = qBound(0, offset.x(), full.width() - 1);
    int oy = qBound(0, offset.y(), full.height() - 1);
    int ow = qMin(width(), full.width() - ox);
    int oh = qMin(height(), full.height() - oy);
    if (ow <= 4 || oh <= 4) return;
    QPixmap cropped = full.copy(ox, oy, ow, oh);

    // ③ 降采样 → 模糊 → 上采样
    int sw = qMax(1, cropped.width() / 2);
    int sh = qMax(1, cropped.height() / 2);
    QPixmap small = cropped.scaled(sw, sh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect();
    blur->setBlurRadius(qMax(1, m_blurRadius / 2));
    blur->setBlurHints(QGraphicsBlurEffect::PerformanceHint);

    QGraphicsScene scene;
    QGraphicsPixmapItem *item = scene.addPixmap(small);
    item->setGraphicsEffect(blur);

    QPixmap blurredSmall(small.size());
    blurredSmall.fill(Qt::transparent);
    {
        QPainter p(&blurredSmall);
        scene.render(&p, QRectF(), QRectF(0, 0, small.width(), small.height()));
        p.end();
    }

    // 放大回原尺寸
    QImage result = blurredSmall.toImage().scaled(
        cropped.width(), cropped.height(),
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // ④ 叠加一层半透明色调（根据当前主题）
    bool isDark = palette().color(QPalette::Base).value() < 128;
    QPainter gp(&result);
    gp.setRenderHint(QPainter::Antialiasing, true);
    QColor tint = isDark ? QColor(30, 35, 60) : QColor(255, 255, 255);
    tint.setAlpha(m_tintOpacity * 255 / 100);
    gp.fillRect(result.rect(), tint);
    gp.end();

    m_glassResult = result;
    m_glassReady = true;
}

void LiquidGlassCard::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const int m = 10;
    QRect cardRect(m, m, width() - m * 2, height() - m * 2);

    QPainterPath clip;
    clip.addRoundedRect(cardRect, m_radius, m_radius);
    p.setClipPath(clip);

    if (m_enabled) {
        if (m_glassReady && !m_glassResult.isNull()) {
            paintGlass(p, cardRect);
        } else {
            paintNormal(p, cardRect);
            scheduleRebuild();
        }
    } else {
        paintNormal(p, cardRect);
    }

    // 顶部高光线
    QLinearGradient hl(cardRect.topLeft(), cardRect.topRight());
    hl.setColorAt(0, QColor(255, 255, 255, 0));
    hl.setColorAt(0.5, QColor(255, 255, 255, (int)(15 + m_hoverAnim * 15)));
    hl.setColorAt(1, QColor(255, 255, 255, 0));
    p.setPen(QPen(QBrush(hl), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(cardRect).adjusted(0.5, 0.5, -0.5, -0.5), m_radius, m_radius);

    p.end();
}

void LiquidGlassCard::paintGlass(QPainter &p, const QRect &rect)
{
    p.drawImage(rect, m_glassResult);

    // 玻璃边框（hover 时更亮）
    int borderAlpha = (int)(30 + m_hoverAnim * 40);
    p.setPen(QPen(QColor(255, 255, 255, borderAlpha), 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(rect).adjusted(0.75, 0.75, -0.75, -0.75), m_radius, m_radius);
}

void LiquidGlassCard::paintNormal(QPainter &p, const QRect &rect)
{
    QColor bg = palette().color(QPalette::Base);
    bg.setAlpha(m_opacity * 255 / 100);
    QColor border = palette().color(QPalette::Text);
    border.setAlphaF(0.08);
    p.setBrush(bg);
    p.setPen(QPen(border, 1));
    p.drawRoundedRect(rect, m_radius, m_radius);
}
