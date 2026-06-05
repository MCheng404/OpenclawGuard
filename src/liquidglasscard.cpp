#include "liquidglasscard.h"
#include "settings.h"

#include <QPainter>
#include <QPainterPath>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QApplication>
#include <QScreen>
#include <QtMath>
#include <QRandomGenerator>
#include <QTimer>

LiquidGlassCard::LiquidGlassCard(QWidget *parent)
    : QFrame(parent)
{
    setContentsMargins(20, 20, 20, 20);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

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
    int si = AppSettings.shadowIntensity();
    setShadowIntensity(si);
    invalidateCache();
}

void LiquidGlassCard::invalidateCache()
{
    m_bgCacheValid = false;
    m_glassReady = false;
    m_glassResult = QImage();
    update();
}

void LiquidGlassCard::enterEvent(QEnterEvent *)
{
    m_hoverAnimObj->stop();
    m_hoverAnimObj->setStartValue(m_hoverAnim);
    m_hoverAnimObj->setEndValue(1.0);
    m_hoverAnimObj->start();
    if (m_enabled) invalidateCache();
}

void LiquidGlassCard::leaveEvent(QEvent *)
{
    m_hoverAnimObj->stop();
    m_hoverAnimObj->setStartValue(m_hoverAnim);
    m_hoverAnimObj->setEndValue(0.0);
    m_hoverAnimObj->start();
    if (m_enabled) invalidateCache();
}

void LiquidGlassCard::moveEvent(QMoveEvent *)
{
    if (m_enabled) invalidateCache();
}

void LiquidGlassCard::resizeEvent(QResizeEvent *)
{
    invalidateCache();
}

// ═══ 在 paint 之外执行抓取和处理，避免递归 ═══
void LiquidGlassCard::prepareGlass()
{
    if (m_glassReady && !m_glassResult.isNull()) return;

    // ── 1. 抓取父窗口背景 ──
    if (!m_bgCacheValid || m_bgCache.isNull()) {
        QWidget *p = parentWidget();
        while (p && !p->isWindow())
            p = p->parentWidget();
        if (!p) p = window();
        if (!p || !p->isVisible()) return;

        // grab 在 paint 外调用，不会递归
        QPixmap fullBg = p->grab();
        QPoint offset = mapTo(p, QPoint(0, 0));
        int ox = qMax(0, offset.x());
        int oy = qMax(0, offset.y());
        int ow = qMin(width(), fullBg.width() - ox);
        int oh = qMin(height(), fullBg.height() - oy);
        if (ow <= 0 || oh <= 0) return;
        m_bgCache = fullBg.copy(ox, oy, ow, oh);
        m_bgCacheValid = true;
    }

    if (m_bgCache.isNull()) return;

    // ── 2. 降采样（1/2）提升性能 ──
    const int srcW = m_bgCache.width();
    const int srcH = m_bgCache.height();
    const int dsW = srcW / 2;
    const int dsH = srcH / 2;
    if (dsW <= 0 || dsH <= 0) return;

    QPixmap small = m_bgCache.scaled(dsW, dsH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // ── 3. 高斯模糊 ──
    QPixmap blurred;
    if (m_blurRadius > 0) {
        QGraphicsBlurEffect *blur = new QGraphicsBlurEffect();
        blur->setBlurRadius(m_blurRadius / 2);
        blur->setBlurHints(QGraphicsBlurEffect::PerformanceHint);

        QGraphicsScene scene;
        QGraphicsPixmapItem *item = scene.addPixmap(small);
        item->setGraphicsEffect(blur);

        blurred = QPixmap(small.size());
        blurred.fill(Qt::transparent);
        QPainter p(&blurred);
        scene.render(&p, QRectF(), QRectF(0, 0, small.width(), small.height()));
        p.end();
    } else {
        blurred = small;
    }

    // ── 4. 折射 + 色散 + 辉光 + 噪点（在小图上操作） ──
    QImage src = blurred.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const int w = src.width();
    const int h = src.height();

    QImage result(w, h, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    const float cx = w / 2.0f;
    const float cy = h / 2.0f;
    const float ea = qMax(1.0f, cx - 2.0f);
    const float eb = qMax(1.0f, cy - 2.0f);
    const float en = 2.5f;
    const float hover = (float)m_hoverAnim;
    const float refr = m_refraction * (1.0f + hover * 0.5f);
    const int alpha = m_opacity * 255 / 100;
    const bool doGlow = m_glowIntensity > 0.01f;
    const bool doNoise = m_noiseAmount > 0.01f;
    QRandomGenerator *rng = QRandomGenerator::global();

    for (int y = 0; y < h; ++y) {
        const QRgb *srcLine = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        QRgb *dstLine = reinterpret_cast<QRgb *>(result.scanLine(y));
        float ny = (y - cy) / eb;
        float nyPow = qPow(qAbs(ny), en);

        for (int x = 0; x < w; ++x) {
            float nx = (x - cx) / ea;
            float sdf = qPow(qPow(qAbs(nx), en) + nyPow, 1.0f / en) - 1.0f;
            float edgeDist = qBound(0.0f, -sdf * 3.0f, 1.0f);

            float ox = nx * refr * edgeDist * 8.0f;
            float oy = ny * refr * edgeDist * 8.0f;

            int sx  = qBound(0, (int)(x + ox), w - 1);
            int sxR = qBound(0, (int)(x + ox * 1.02f), w - 1);
            int sxB = qBound(0, (int)(x + ox * 0.98f), w - 1);
            int sy  = qBound(0, (int)(y + oy), h - 1);

            int r  = qRed(srcLine[sxR]);
            int g  = qGreen(srcLine[sx]);
            int bl = qBlue(srcLine[sxB]);

            if (doGlow && sdf > -0.15f && sdf < 0.05f) {
                float glow = (1.0f - qAbs(sdf + 0.05f) / 0.10f) * m_glowIntensity;
                r  = qBound(0, (int)(r  + glow * 80), 255);
                g  = qBound(0, (int)(g  + glow * 90), 255);
                bl = qBound(0, (int)(bl + glow * 100), 255);
            }

            if (doNoise) {
                float noise = ((int)rng->generate() % 100 - 50) * m_noiseAmount * 0.6f;
                r  = qBound(0, (int)(r  + noise), 255);
                g  = qBound(0, (int)(g  + noise), 255);
                bl = qBound(0, (int)(bl + noise), 255);
            }

            dstLine[x] = qRgba(r, g, bl, alpha);
        }
    }

    // ── 5. 放大回原尺寸 ──
    m_glassResult = result.scaled(srcW, srcH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_glassReady = true;
}

void LiquidGlassCard::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const int m = 10;
    QRect cardRect(m, m, width() - m * 2, height() - m * 2);

    QPainterPath clipPath;
    clipPath.addRoundedRect(cardRect, m_radius, m_radius);
    p.setClipPath(clipPath);

    if (m_enabled) {
        if (!m_glassReady || m_glassResult.isNull()) {
            // 兜底：先画普通背景
            paintNormal(p, cardRect);
            // 异步重建，不在 paint 里 grab
            QTimer::singleShot(0, this, [this]() {
                m_glassReady = false;
                prepareGlass();
                update();
            });
        } else {
            p.drawImage(cardRect, m_glassResult);
            // 玻璃边框
            QColor bc(255, 255, 255, (int)(40 + m_hoverAnim * 30));
            p.setPen(QPen(bc, 1.5));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(QRectF(cardRect).adjusted(0.75, 0.75, -0.75, -0.75), m_radius, m_radius);
        }
    } else {
        paintNormal(p, cardRect);
    }

    // 顶部高光线
    QLinearGradient hl(cardRect.topLeft(), cardRect.topRight());
    hl.setColorAt(0, QColor(255, 255, 255, 0));
    hl.setColorAt(0.5, QColor(255, 255, 255, (int)(12 + m_hoverAnim * 10)));
    hl.setColorAt(1, QColor(255, 255, 255, 0));
    p.setPen(QPen(QBrush(hl), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(cardRect).adjusted(0.5, 0.5, -0.5, -0.5), m_radius, m_radius);

    p.end();
}

void LiquidGlassCard::paintNormal(QPainter &p, const QRect &rect)
{
    QColor cardBg = palette().color(QPalette::Base);
    cardBg.setAlpha(m_opacity * 255 / 100);
    QColor borderColor = palette().color(QPalette::Text);
    borderColor.setAlphaF(0.08);

    p.setBrush(cardBg);
    p.setPen(QPen(borderColor, 1));
    p.drawRoundedRect(rect, m_radius, m_radius);
}
