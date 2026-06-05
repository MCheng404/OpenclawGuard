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
    m_bgCacheValid = false;
    update();
}

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

void LiquidGlassCard::moveEvent(QMoveEvent *)
{
    m_bgCacheValid = false;
}

void LiquidGlassCard::resizeEvent(QResizeEvent *)
{
    m_bgCacheValid = false;
}

QPoint LiquidGlassCard::globalBackgroundOffset() const
{
    // 找到最近的可抓取父窗口
    QWidget *p = parentWidget();
    while (p && !p->isWindow())
        p = p->parentWidget();
    if (!p) p = window();
    QPoint global = mapTo(p, QPoint(0, 0));
    return global;
}

QPixmap LiquidGlassCard::grabBackground()
{
    if (m_bgCacheValid && !m_bgCache.isNull())
        return m_bgCache;

    QWidget *p = parentWidget();
    while (p && !p->isWindow())
        p = p->parentWidget();
    if (!p) p = window();

    // 抓取父窗口内容
    QPixmap bg = p->grab();
    QPoint offset = mapTo(p, QPoint(0, 0));

    // 裁剪卡片区域
    m_bgCache = bg.copy(offset.x(), offset.y(), width(), height());
    m_bgCacheValid = true;
    return m_bgCache;
}

QPixmap LiquidGlassCard::applyBlur(const QPixmap &src, int radius)
{
    if (radius <= 0 || src.isNull()) return src;

    // 用 QGraphicsBlurEffect 做模糊
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect();
    blur->setBlurRadius(radius);
    blur->setBlurHints(QGraphicsBlurEffect::PerformanceHint);

    // 通过 QGraphicsScene 应用模糊效果
    QGraphicsScene scene;
    QGraphicsPixmapItem *item = scene.addPixmap(src);
    item->setGraphicsEffect(blur);

    QPixmap result(src.size());
    result.fill(Qt::transparent);
    QPainter p(&result);
    scene.render(&p, QRectF(), QRectF(0, 0, src.width(), src.height()));
    p.end();

    return result;
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
        paintGlass(p, cardRect);
    } else {
        paintNormal(p, cardRect);
    }

    // 顶部高光线
    QLinearGradient topHighlight(cardRect.topLeft(), cardRect.topRight());
    topHighlight.setColorAt(0, QColor(255, 255, 255, 0));
    topHighlight.setColorAt(0.5, QColor(255, 255, 255, (int)(12 + m_hoverAnim * 10)));
    topHighlight.setColorAt(1, QColor(255, 255, 255, 0));
    p.setPen(QPen(QBrush(topHighlight), 1));
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

void LiquidGlassCard::paintGlass(QPainter &p, const QRect &rect)
{
    // 1. 抓取背景
    QPixmap bg = grabBackground();
    if (bg.isNull()) { paintNormal(p, rect); return; }

    // 2. 模糊
    QPixmap blurred = applyBlur(bg, m_blurRadius);

    // 3. 折射位移 — 边缘偏移大，中心偏移小（模拟凸透镜折射）
    QImage blurredImg = blurred.toImage();
    QImage resultImg(blurredImg.size(), QImage::Format_ARGB32_Premultiplied);
    resultImg.fill(Qt::transparent);

    const int w = blurredImg.width();
    const int h = blurredImg.height();
    const float cx = w / 2.0f;
    const float cy = h / 2.0f;
    const float maxDist = qSqrt(cx * cx + cy * cy);

    // SDF 超椭圆参数
    const float a = w / 2.0f - 2.0f;
    const float b = h / 2.0f - 2.0f;
    const float n = 2.5f; // 超椭圆指数

    for (int y = 0; y < h; ++y) {
        const QRgb *srcLine = reinterpret_cast<const QRgb *>(blurredImg.constScanLine(y));
        QRgb *dstLine = reinterpret_cast<QRgb *>(resultImg.scanLine(y));

        for (int x = 0; x < w; ++x) {
            // 归一化坐标到 [-1, 1]
            float nx = (x - cx) / a;
            float ny = (y - cy) / b;

            // SDF: 超椭圆距离（负值 = 内部）
            float sdf = qPow(qPow(qAbs(nx), n) + qPow(qAbs(ny), n), 1.0f / n) - 1.0f;

            // 距离边缘的比例（内部越深 = 折射越强）
            float edgeDist = qBound(0.0f, -sdf * 3.0f, 1.0f);

            // 折射偏移：边缘大，中心小
            float offsetX = nx * m_refraction * edgeDist * 8.0f;
            float offsetY = ny * m_refraction * edgeDist * 8.0f;

            // 悬浮时折射增强
            offsetX *= (1.0f + m_hoverAnim * 0.5f);
            offsetY *= (1.0f + m_hoverAnim * 0.5f);

            // 采样模糊纹理（带位移）
            int sx = qBound(0, (int)(x + offsetX), w - 1);
            int sy = qBound(0, (int)(y + offsetY), h - 1);
            QRgb sample = srcLine[sx];

            // 色散：RGB 三通道分别偏移
            int sxR = qBound(0, (int)(x + offsetX * 1.02f), w - 1);
            int sxB = qBound(0, (int)(x + offsetX * 0.98f), w - 1);
            int r = qRed(blurredImg.pixel(sxR, sy));
            int g = qGreen(sample);
            int b_ch = qBlue(blurredImg.pixel(sxB, sy));

            // 辉光：边缘处加亮
            float glow = 0.0f;
            if (sdf > -0.15f && sdf < 0.05f) {
                glow = (1.0f - qAbs(sdf + 0.05f) / 0.10f) * m_glowIntensity;
            }
            r = qBound(0, (int)(r + glow * 80), 255);
            g = qBound(0, (int)(g + glow * 90), 255);
            b_ch = qBound(0, (int)(b_ch + glow * 100), 255);

            // 噪点
            float noise = (QRandomGenerator::global()->generateDouble() - 0.5) * m_noiseAmount * 60;
            r = qBound(0, (int)(r + noise), 255);
            g = qBound(0, (int)(g + noise), 255);
            b_ch = qBound(0, (int)(b_ch + noise), 255);

            // 卡片透明度
            int alpha = m_opacity * 255 / 100;

            dstLine[x] = qRgba(r, g, b_ch, alpha);
        }
    }

    // 4. 绘制结果
    p.drawImage(rect, resultImg);

    // 5. 边框（玻璃边缘高光）
    QColor borderColor(255, 255, 255, (int)(40 + m_hoverAnim * 30));
    p.setPen(QPen(borderColor, 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect, m_radius, m_radius);
}
