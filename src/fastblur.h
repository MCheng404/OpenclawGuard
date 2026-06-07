#pragma once

#include <QImage>

/// GPU 模糊的简单替代：CPU 多 pass box blur（两次 box ≈ 高斯）
/// 对 1080p 降采样后的图像（~500×300），三次 pass < 3ms
class FastBlur
{
public:
    /// 对 source 做 box blur
    /// @param source 源图像（RGBA8888 或 ARGB32）
    /// @param radius 模糊半径（越大越模糊）
    /// @param passes box blur 次数（2≈高斯，3 更平滑）
    /// @param downsample 降采样倍率（2 = 宽高各减半）
    static QImage blur(QImage source, int radius, int passes = 2, int downsample = 2);

private:
    static void boxBlurH(QImage &img, int radius);
    static void boxBlurV(QImage &img, int radius);
};
