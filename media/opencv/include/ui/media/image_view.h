#pragma once

#include "ui/ui/widget.h"

namespace ui {
class Renderer;
class Texture;

// 图像控件（M6，OpenCV 处理 → 纹理）：载入图片后按宽高比适配绘制
class ImageView : public Widget {
public:
    ImageView();

    // 载入图像（jpg/png/bmp 等 OpenCV 支持的格式），失败返回 false
    bool load_from_file(const char* path);

    bool has_image() const { return has_image_; }
    int image_width() const;
    int image_height() const;

    void on_draw(Renderer& renderer) override;

private:
    Texture* texture_ = nullptr;
    bool has_image_ = false;
};

}  // namespace ui
