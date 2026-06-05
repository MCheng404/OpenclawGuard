#pragma once

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QTimer>

class LiquidGlassCard : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(qreal glassHover READ glassHover WRITE setGlassHover)

public:
    explicit LiquidGlassCard(QWidget *parent = nullptr);

    // 参数
    void setBlurRadius(int r)       { m_blurRadius = r; update(); }
    void setRefraction(float f)     { m_refraction = f; update(); }
    void setGlowIntensity(float f)  { m_glowIntensity = f; update(); }
    void setNoiseAmount(float f)    { m_noiseAmount = f; update(); }
    void setGlassEnabled(bool on)   { m_enabled = on; update(); }
    void setCardRadius(int r)       { m_radius = r; update(); }
    void setCardOpacity(int o)      { m_opacity = o; update(); }
    void setShadowIntensity(int s);

    bool isGlassEnabled() const { return m_enabled; }

    qreal glassHover() const { return m_hoverAnim; }
    void setGlassHover(qreal v) { m_hoverAnim = v; update(); }

    void refreshStyle();
    void invalidateCache() { m_bgCacheValid = false; update(); }

protected:
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
    void moveEvent(QMoveEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    QPixmap grabBackground();
    QPixmap applyBlur(const QPixmap &src, int radius);
    QPoint globalBackgroundOffset() const;
    void paintGlass(QPainter &p, const QRect &rect);
    void paintNormal(QPainter &p, const QRect &rect);

    // 参数
    bool  m_enabled = false;
    int   m_blurRadius = 18;
    float m_refraction = 0.45f;
    float m_glowIntensity = 0.35f;
    float m_noiseAmount = 0.04f;
    int   m_radius = 16;
    int   m_opacity = 100;

    // 内部
    QGraphicsDropShadowEffect *m_shadow = nullptr;
    QPixmap m_bgCache;
    bool m_bgCacheValid = false;
    qreal m_hoverAnim = 0.0;
    QPropertyAnimation *m_hoverAnimObj = nullptr;
};
