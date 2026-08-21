#include "ui/media/image_view.h"

#include "ui/core/log.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace ui {

ImageView::ImageView() : Widget(0, 0, 320, 240) {
    texture_ = new Texture;
}

bool ImageView::load_from_file(const char* path) {
    cv::Mat img = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        log_error("ImageView: cannot load %s", path ? path : "");
        has_image_ = false;
        return false;
    }

    cv::Mat rgba;
    switch (img.channels()) {
        case 4: cv::cvtColor(img, rgba, cv::COLOR_BGRA2RGBA); break;
        case 3: cv::cvtColor(img, rgba, cv::COLOR_BGR2RGBA); break;
        case 1: cv::cvtColor(img, rgba, cv::COLOR_GRAY2RGBA); break;
        default: rgba = img; break;
    }
    if (!rgba.isContinuous()) rgba = rgba.clone();

    texture_->create(rgba.cols, rgba.rows, rgba.data);
    has_image_ = true;
    log_info("ImageView: loaded %s (%dx%d)", path ? path : "", rgba.cols, rgba.rows);
    return true;
}

int ImageView::image_width() const { return texture_ ? texture_->width() : 0; }
int ImageView::image_height() const { return texture_ ? texture_->height() : 0; }

void ImageView::on_draw(Renderer& renderer) {
    if (!has_image_ || !texture_) return;
    const float iw = (float)texture_->width();
    const float ih = (float)texture_->height();
    if (iw <= 0.f || ih <= 0.f) return;

    // 保持宽高比，居中适配控件矩形
    const float img_aspect = iw / ih;
    const float box_aspect = bounds_.w / bounds_.h;
    Rect r = bounds_;
    if (box_aspect > img_aspect) {
        const float w = bounds_.h * img_aspect;
        r.x = bounds_.x + (bounds_.w - w) * 0.5f;
        r.w = w;
    } else {
        const float h = bounds_.w / img_aspect;
        r.y = bounds_.y + (bounds_.h - h) * 0.5f;
        r.h = h;
    }
    // uv {0,0,1,1}：纹理缓冲首行(图像顶部)对应 v=0，与渲染器 v0=上边一致
    renderer.draw_textured_quad(r, texture_->id(), Rect{0.f, 0.f, 1.f, 1.f},
                                Vec4{1.f, 1.f, 1.f, 1.f});
}

}  // namespace ui
