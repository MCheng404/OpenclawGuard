#include "fastblur.h"
#include <QtMath>
#include <cstring>

QImage FastBlur::blur(QImage source, int radius, int passes, int downsample)
{
    if (source.isNull() || radius < 1) return source;

    int srcW = source.width();
    int srcH = source.height();

    // 降采样
    int dsW = qMax(1, srcW / downsample);
    int dsH = qMax(1, srcH / downsample);
    int dsR = qMax(1, radius / downsample);

    QImage img = source.scaled(dsW, dsH, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    if (img.format() != QImage::Format_RGBA8888)
        img = img.convertToFormat(QImage::Format_RGBA8888);

    // 多 pass box blur
    for (int i = 0; i < passes; ++i) {
        boxBlurH(img, dsR);
        boxBlurV(img, dsR);
    }

    // 上采样
    if (dsW != srcW || dsH != srcH) {
        img = img.scaled(srcW, srcH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    return img;
}

void FastBlur::boxBlurH(QImage &img, int radius)
{
    int w = img.width();
    int h = img.height();
    int stride = img.bytesPerLine();
    int ksz = radius * 2 + 1;

    // 临时行缓冲
    QVector<quint32> lineBuf(w);
    QVector<quint32> acc(4);
    quint32 *bits = reinterpret_cast<quint32*>(img.bits());

    for (int y = 0; y < h; ++y) {
        quint32 *row = bits + y * (stride / 4);
        acc[0] = acc[1] = acc[2] = acc[3] = 0;

        // 初始化滑动窗口：左边缘填充
        quint32 left = row[0];
        for (int i = -radius; i <= radius; ++i) {
            int idx = qBound(0, i, w - 1);
            quint32 px = row[idx];
            acc[0] += (px      ) & 0xFF;
            acc[1] += (px >>  8) & 0xFF;
            acc[2] += (px >> 16) & 0xFF;
            acc[3] += (px >> 24) & 0xFF;
        }

        for (int x = 0; x < w; ++x) {
            lineBuf[x] = ((acc[0] / ksz)      ) |
                         ((acc[1] / ksz) <<  8) |
                         ((acc[2] / ksz) << 16) |
                         ((acc[3] / ksz) << 24);

            // 滑出左侧
            int removeIdx = qBound(0, x - radius, w - 1);
            quint32 oldPx = row[removeIdx];
            acc[0] -= (oldPx      ) & 0xFF;
            acc[1] -= (oldPx >>  8) & 0xFF;
            acc[2] -= (oldPx >> 16) & 0xFF;
            acc[3] -= (oldPx >> 24) & 0xFF;

            // 滑入右侧
            int addIdx = qBound(0, x + radius + 1, w - 1);
            quint32 newPx = row[addIdx];
            acc[0] += (newPx      ) & 0xFF;
            acc[1] += (newPx >>  8) & 0xFF;
            acc[2] += (newPx >> 16) & 0xFF;
            acc[3] += (newPx >> 24) & 0xFF;
        }

        std::memcpy(row, lineBuf.constData(), w * sizeof(quint32));
    }
}

void FastBlur::boxBlurV(QImage &img, int radius)
{
    int w = img.width();
    int h = img.height();
    int stride = img.bytesPerLine();
    int ksz = radius * 2 + 1;

    QVector<quint32> colBuf(h);
    QVector<quint32> acc(4);
    quint32 *bits = reinterpret_cast<quint32*>(img.bits());

    for (int x = 0; x < w; ++x) {
        acc[0] = acc[1] = acc[2] = acc[3] = 0;

        // 初始化滑动窗口
        for (int i = -radius; i <= radius; ++i) {
            int idx = qBound(0, i, h - 1);
            quint32 px = bits[idx * (stride / 4) + x];
            acc[0] += (px      ) & 0xFF;
            acc[1] += (px >>  8) & 0xFF;
            acc[2] += (px >> 16) & 0xFF;
            acc[3] += (px >> 24) & 0xFF;
        }

        for (int y = 0; y < h; ++y) {
            colBuf[y] = ((acc[0] / ksz)      ) |
                        ((acc[1] / ksz) <<  8) |
                        ((acc[2] / ksz) << 16) |
                        ((acc[3] / ksz) << 24);

            int removeIdx = qBound(0, y - radius, h - 1);
            quint32 oldPx = bits[removeIdx * (stride / 4) + x];
            acc[0] -= (oldPx      ) & 0xFF;
            acc[1] -= (oldPx >>  8) & 0xFF;
            acc[2] -= (oldPx >> 16) & 0xFF;
            acc[3] -= (oldPx >> 24) & 0xFF;

            int addIdx = qBound(0, y + radius + 1, h - 1);
            quint32 newPx = bits[addIdx * (stride / 4) + x];
            acc[0] += (newPx      ) & 0xFF;
            acc[1] += (newPx >>  8) & 0xFF;
            acc[2] += (newPx >> 16) & 0xFF;
            acc[3] += (newPx >> 24) & 0xFF;
        }

        for (int y = 0; y < h; ++y) {
            bits[y * (stride / 4) + x] = colBuf[y];
        }
    }
}
