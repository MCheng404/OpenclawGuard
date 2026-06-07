#include "glblurhelper.h"
#include "glasslog.h"

#include <QOpenGLFramebufferObjectFormat>
#include <QSurfaceFormat>
#include <QDebug>

// ── GLSL 着色器 ─────────────────────────────────────────────────────────

static const char *kBlurVert = R"(#version 300 es
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

static const char *kBlurFrag = R"(#version 300 es
precision mediump float;
in vec2 vUV;
out vec4 fragColor;

uniform sampler2D uTexture;
uniform vec2 uDirection;   // (1/w, 0) 水平  or  (0, 1/h) 垂直
uniform float uRadius;

void main() {
    vec2 texel = uDirection;
    float sigma = max(uRadius * 0.4, 0.5);
    float twoSigma2 = 2.0 * sigma * sigma;

    // 采样范围限制为 radius，步长 1.5 以平衡质量与性能
    float step = max(1.0, uRadius / 16.0);
    int taps = int(ceil(uRadius / step));

    vec4 sum = texture(uTexture, vUV);
    float totalWeight = 1.0;

    for (int i = 1; i <= 64; ++i) {
        if (i > taps) break;
        float fi = float(i);
        float weight = exp(-fi * fi / twoSigma2);
        vec2 off = texel * fi * step;
        sum += texture(uTexture, vUV + off) * weight;
        sum += texture(uTexture, vUV - off) * weight;
        totalWeight += 2.0 * weight;
    }

    fragColor = sum / totalWeight;
}
)";

// ── 全屏四边形顶点 ──────────────────────────────────────────────────────

static const float kQuadVerts[] = {
    // pos       // uv (翻转 Y，匹配 Qt 坐标系)
    -1.f, -1.f,  0.f, 1.f,
     1.f, -1.f,  1.f, 1.f,
    -1.f,  1.f,  0.f, 0.f,
     1.f,  1.f,  1.f, 0.f,
};

// ── 单例 ────────────────────────────────────────────────────────────────

GLBlurHelper *GLBlurHelper::instance()
{
    static GLBlurHelper s_instance;
    return &s_instance;
}

GLBlurHelper::GLBlurHelper() = default;

GLBlurHelper::~GLBlurHelper()
{
    cleanup();
}

// ── 确保 OpenGL 上下文就绪 ───────────────────────────────────────────────

bool GLBlurHelper::ensureContext()
{
    if (m_context && m_context->isValid()) return true;

    glassLog("[GL] ensureContext: creating new context...");
    QMutexLocker lock(&m_mutex);
    if (m_context && m_context->isValid()) return true; // double-check

    // 离屏 Surface
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setVersion(3, 2);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(0);
    fmt.setStencilBufferSize(0);
    fmt.setSamples(0);

    m_surface = new QOffscreenSurface();
    m_surface->setFormat(fmt);
    m_surface->create();

    m_context = new QOpenGLContext();
    m_context->setFormat(fmt);
    if (!m_context->create()) {
        qWarning() << "[GLBlurHelper] Failed to create OpenGL context";
        return false;
    }

    if (!m_context->makeCurrent(m_surface)) {
        qWarning() << "[GLBlurHelper] Failed to make GL context current";
        return false;
    }

    initializeOpenGLFunctions();

    // VAO + VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVerts), kQuadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<void *>(2 * sizeof(float)));
    glBindVertexArray(0);

    m_initialized = true;
    return true;
}

// ── 编译链接着色器 ──────────────────────────────────────────────────────

bool GLBlurHelper::ensureShaders()
{
    if (m_blurShader && m_blurShader->isLinked()) return true;

    m_blurShader = new QOpenGLShaderProgram();
    if (!m_blurShader->addShaderFromSourceCode(QOpenGLShader::Vertex, kBlurVert)) {
        qWarning() << "[GLBlurHelper] Vertex shader:" << m_blurShader->log();
        return false;
    }
    if (!m_blurShader->addShaderFromSourceCode(QOpenGLShader::Fragment, kBlurFrag)) {
        qWarning() << "[GLBlurHelper] Fragment shader:" << m_blurShader->log();
        return false;
    }
    if (!m_blurShader->link()) {
        qWarning() << "[GLBlurHelper] Link:" << m_blurShader->log();
        return false;
    }
    return true;
}

// ── 确保 FBO 尺寸匹配 ──────────────────────────────────────────────────

bool GLBlurHelper::ensureFBO(const QSize &size)
{
    if (m_fboA && m_fboB && m_fboSize == size) return true;

    delete m_fboA;
    delete m_fboB;
    m_fboA = nullptr;
    m_fboB = nullptr;

    QOpenGLFramebufferObjectFormat fboFmt;
    fboFmt.setInternalTextureFormat(GL_RGBA8);
    fboFmt.setSamples(0);
    fboFmt.setMipmap(false);

    m_fboA = new QOpenGLFramebufferObject(size, fboFmt);
    m_fboB = new QOpenGLFramebufferObject(size, fboFmt);
    m_fboSize = size;

    return (m_fboA->isValid() && m_fboB->isValid());
}

// ── 上传 QImage 到 GL 纹理 ─────────────────────────────────────────────

GLuint GLBlurHelper::uploadTexture(const QImage &img)
{
    // 统一转为 RGBA8888（避免 ARGB32/BGRA 格式混乱）
    QImage glImg = img.convertToFormat(QImage::Format_RGBA8888);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, glImg.width(), glImg.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, glImg.constBits());
    return tex;
}

// ── 绘制全屏四边形 ─────────────────────────────────────────────────────

void GLBlurHelper::drawFullscreenQuad()
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// ── 公开接口：GPU 高斯模糊 ─────────────────────────────────────────────

QImage GLBlurHelper::blur(const QImage &source, int radius, int downsample)
{
    if (source.isNull() || radius < 1) return source;

    if (!ensureContext() || !ensureShaders()) {
        qWarning() << "[GLBlurHelper] init failed, returning unblurred";
        return source;
    }

    m_context->makeCurrent(m_surface);
    qDebug("[GL] blur: src=%dx%d, radius=%d, ds=%d", source.width(), source.height(), radius, downsample);

    // ── 1. 降采样 ──
    int srcW = source.width();
    int srcH = source.height();
    int dsW = qMax(1, srcW / downsample);
    int dsH = qMax(1, srcH / downsample);
    int blurR = qMax(1, radius / downsample);

    QImage small = source.scaled(dsW, dsH, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    if (small.format() != QImage::Format_RGBA8888 && small.format() != QImage::Format_ARGB32)
        small = small.convertToFormat(QImage::Format_RGBA8888);

    // ── 2. 上传纹理 ──
    GLuint srcTex = uploadTexture(small);

    // ── 3. 确保 FBO ──
    QSize fboSize(dsW, dsH);
    if (!ensureFBO(fboSize)) {
        qWarning() << "[GL] FBO creation failed for size" << fboSize;
        glDeleteTextures(1, &srcTex);
        m_context->doneCurrent();
        return source;
    }
    qDebug("[GL] FBO OK: %dx%d, shader=%p", dsW, dsH, m_blurShader);

    // ── 4. Pass 1: 水平模糊 ──
    m_blurShader->bind();
    m_blurShader->setUniformValue("uTexture", 0);
    m_blurShader->setUniformValue("uDirection", QVector2D(1.0f / dsW, 0.0f));
    m_blurShader->setUniformValue("uRadius", static_cast<float>(blurR));

    m_fboA->bind();
    glViewport(0, 0, dsW, dsH);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    drawFullscreenQuad();
    m_fboA->release();

    // ── 5. Pass 2: 垂直模糊 ──
    m_blurShader->setUniformValue("uDirection", QVector2D(0.0f, 1.0f / dsH));

    m_fboB->bind();
    glViewport(0, 0, dsW, dsH);
    glBindTexture(GL_TEXTURE_2D, m_fboA->texture());
    drawFullscreenQuad();
    // toImage() 必须在 FBO 绑定状态下调用（内部用 glReadPixels）
    QImage result = m_fboB->toImage(true);
    m_fboB->release();

    // ── 7. 清理临时资源 ──
    glDeleteTextures(1, &srcTex);
    glFinish();
    m_context->doneCurrent();

    // ── 8. 上采样到原始尺寸 ──
    if (result.size() != source.size()) {
        result = result.scaled(srcW, srcH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    qDebug("[GL] blur done: result=%dx%d, null=%d", result.width(), result.height(), (int)result.isNull());
    return result;
}

// ── 释放资源 ────────────────────────────────────────────────────────────

void GLBlurHelper::cleanup()
{
    QMutexLocker lock(&m_mutex);

    if (m_context) {
        m_context->makeCurrent(m_surface);

        if (m_fboA) { delete m_fboA; m_fboA = nullptr; }
        if (m_fboB) { delete m_fboB; m_fboB = nullptr; }
        if (m_blurShader) { delete m_blurShader; m_blurShader = nullptr; }
        if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
        if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }

        m_context->doneCurrent();
        delete m_context;
        m_context = nullptr;
    }

    if (m_surface) {
        delete m_surface;
        m_surface = nullptr;
    }

    m_initialized = false;
}
