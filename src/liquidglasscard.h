#pragma once

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QImage>
#include <QPropertyAnimation>
#include <QFutureWatcher>

class LiquidGlassCard : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(qreal glassHover READ glassHover WRITE setGlassHover)

public:
    explicit LiquidGlassCard(QWidget *parent = nullptr);

    void setBlurRadius(int r)       { m_blurRadius = r; invalidateCache(); }
    void setTintOpacity(int v)      { m_tintOpacity = v; invalidateCache(); }
    void setGlassEnabled(bool on)   { m_enabled = on; invalidateCache(); }
    void setCardRadius(int r)       { m_radius = r; update(); }
    void setCardOpacity(int o)      { m_opacity = o; update(); }
    void setShadowIntensity(int s);

    bool isGlassEnabled() const { return m_enabled; }

    qreal glassHover() const { return m_hoverAnim; }
    void setGlassHover(qreal v) { m_hoverAnim = v; update(); }

    void refreshStyle();
    void invalidateCache();

protected:
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
    void moveEvent(QMoveEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void showEvent(QShowEvent *) override;

private:
    void scheduleRebuild();
    void paintNormal(QPainter &p, const QRect &rect);
    void paintGlass(QPainter &p, const QRect &rect);

    // 参数
    bool m_enabled = false;
    int  m_blurRadius = 20;
    int  m_tintOpacity = 30;  // 色调叠加透明度 0-100
    int  m_radius = 16;
    int  m_opacity = 100;

    // 缓存
    QGraphicsDropShadowEffect *m_shadow = nullptr;
    QImage m_glassResult;
    bool m_glassReady = false;
    bool m_rebuildPending = false;
    qreal m_hoverAnim = 0.0;
    QPropertyAnimation *m_hoverAnimObj = nullptr;
    QFutureWatcher<QImage> *m_watcher = nullptr;
};
