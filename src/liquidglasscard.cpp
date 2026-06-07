#include "liquidglasscard.h"
#include "fastblur.h"
#include "glasslog.h"
#include "settings.h"

#include <QPainter>
#include <QPainterPath>
#include <QGraphicsEffect>
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

    // 延迟到下一个事件循环，避免在 paintEvent 内递归重建
    QTimer::singleShot(0, this, &LiquidGlassCard::doRebuild);
}

void LiquidGlassCard::doRebuild()
{
    QWidget *root = parentWidget();
    while (root && !root->isWindow()) root = root->parentWidget();
    if (!root) root = window();
    if (!root || !root->isVisible()) { m_rebuildPending = false; return; }

    QPixmap full = root->grab();
    if (full.isNull()) { m_rebuildPending = false; return; }

    QPoint offset = mapTo(root, QPoint(0, 0));
    int ox = qBound(0, offset.x(), full.width() - 1);
    int oy = qBound(0, offset.y(), full.height() - 1);
    int ow = qMin(width(), full.width() - ox);
    int oh = qMin(height(), full.height() - oy);
    if (ow <= 4 || oh <= 4) { m_rebuildPending = false; return; }

    QImage cropped = full.copy(ox, oy, ow, oh).toImage();
    if (cropped.format() != QImage::Format_RGBA8888)
        cropped = cropped.convertToFormat(QImage::Format_RGBA8888);

    // ── 高性能模糊（CPU 多 pass box blur） ──
    QImage blurred = FastBlur::blur(cropped, m_blurRadius, 2, 2);
    if (blurred.isNull()) { m_rebuildPending = false; return; }

    // ── 色调叠加 ──
    bool isDark = palette().color(QPalette::Base).value() < 128;
    QPainter gp(&blurred);
    QColor tint = isDark ? QColor(30, 35, 60) : QColor(255, 255, 255);
    tint.setAlpha(m_tintOpacity * 255 / 100);
    gp.fillRect(blurred.rect(), tint);
    gp.end();

    // 先设结果，再放行重建，再刷新
    m_glassResult = blurred;
    m_glassReady = true;
    m_rebuildPending = false;  // 此刻起 paintEvent 可以正常绘制 glass
    update();
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
