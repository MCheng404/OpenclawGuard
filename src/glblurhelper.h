#pragma once

#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMutex>

class GLBlurHelper : public QOpenGLExtraFunctions
{
public:
    static GLBlurHelper *instance();

    /// 对 QImage 做 GPU 高斯模糊（两 pass 可分离），返回模糊后的 QImage
    /// @param source   源图像（ARGB32 或 RGBA8888）
    /// @param radius   模糊半径（像素，越大越模糊）
    /// @param downsample 降采样倍率（2 = 宽高各减半，速度 ↑ 质量 ↓）
    QImage blur(const QImage &source, int radius, int downsample = 2);

    /// 释放所有 GL 资源（析构前调用或需要时手动释放）
    void cleanup();

private:
    GLBlurHelper();
    ~GLBlurHelper();

    bool ensureContext();
    bool ensureShaders();
    bool ensureFBO(const QSize &size);
    GLuint uploadTexture(const QImage &img);
    void drawFullscreenQuad();

    QOffscreenSurface  *m_surface = nullptr;
    QOpenGLContext      *m_context = nullptr;
    QOpenGLShaderProgram *m_blurShader = nullptr;
    QOpenGLFramebufferObject *m_fboA = nullptr;
    QOpenGLFramebufferObject *m_fboB = nullptr;
    QSize               m_fboSize;
    GLuint              m_vao = 0;
    GLuint              m_vbo = 0;
    bool                m_initialized = false;
    QMutex              m_mutex;
};
