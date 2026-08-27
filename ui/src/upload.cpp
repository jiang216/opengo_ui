#include "ui/ui/upload.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <cstdio>

namespace ui {

Upload::Upload(float x, float y, float w, float h) : Widget(x, y, w, h) {}

void Upload::add_file(const char* name, int size_bytes) {
    mark_dirty();
        files_.push_back({name ? name : "", size_bytes, 0.f, false});
}

void Upload::advance(float dt) {
    for (auto& f : files_) {
        if (!f.done) {
            f.progress += dt * 12.f;  // 12%/s
            if (f.progress >= 100.f) { f.progress = 100.f; f.done = true; }
        }
    }
}

void Upload::remove_done() {
    files_.erase(std::remove_if(files_.begin(), files_.end(), [](const File& f) { return f.done; }),
                 files_.end());
}

bool Upload::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    // "选择文件"按钮
    const Rect btn{bounds_.x, bounds_.y, 90.f, 28.f};
    if (x >= btn.x && x <= btn.x + btn.w && y >= btn.y && y <= btn.y + btn.h) {
        if (on_pick) on_pick();
        else add_file("demo_file.bin", 1024 * 512);
        return true;
    }
    // 清理已完成项
    const Rect clr{bounds_.x, bounds_.y + 32.f, 70.f, 24.f};
    if (x >= clr.x && x <= clr.x + clr.w && y >= clr.y && y <= clr.y + clr.h) {
        remove_done();
        return true;
    }
    return false;
}

void Upload::on_draw(Renderer& renderer) {
    const Vec4 blue{0.12f, 0.62f, 1.0f, 1.0f};
    const Vec4 border{0.85f, 0.85f, 0.85f, 1.0f};
    const Vec4 dark{0.3f, 0.3f, 0.3f, 1.0f};
    const Vec4 light{0.6f, 0.6f, 0.6f, 1.0f};
    const Vec4 green{0.09f, 0.73f, 0.67f, 1.0f};
    // 按钮
    renderer.draw_rounded_rect(Rect{bounds_.x, bounds_.y, 90.f, 28.f}, 3.f, blue);
    if (font_) font_->draw_text(renderer, "选择文件", bounds_.x + 14.f, bounds_.y + 5.f, Vec4{1, 1, 1, 1});
    renderer.draw_rounded_rect(Rect{bounds_.x, bounds_.y + 32.f, 70.f, 24.f}, 3.f, Vec4{0.95f, 0.95f, 0.95f, 1.0f});
    renderer.draw_rect_outline(Rect{bounds_.x, bounds_.y + 32.f, 70.f, 24.f}, 1.f, border);
    if (font_) font_->draw_text(renderer, "清理", bounds_.x + 16.f, bounds_.y + 36.f, dark);
    // 文件列表
    if (!font_) return;
    float y = bounds_.y + 64.f;
    for (const File& f : files_) {
        if (y + 46.f > bounds_.y + bounds_.h) break;
        // 文件行：圆角背景
        renderer.draw_rounded_rect(Rect{bounds_.x, y, bounds_.w, 42.f}, 3.f, Vec4{0.97f, 0.97f, 0.97f, 1.0f});
        // 文件名 + 大小
        char meta[128];
        std::snprintf(meta, sizeof(meta), "%s  (%d KB)", f.name.c_str(), f.size / 1024);
        font_->draw_text(renderer, meta, bounds_.x + 8.f, y + 2.f, dark);
        // 进度条
        const float pw = bounds_.w - 16.f;
        renderer.draw_rounded_rect(Rect{bounds_.x + 8.f, y + 20.f, pw, 8.f}, 4.f, Vec4{0.9f, 0.9f, 0.9f, 1.0f});
        const float fw = pw * f.progress / 100.f;
        if (fw > 0.f)
            renderer.draw_rounded_rect(Rect{bounds_.x + 8.f, y + 20.f, fw, 8.f}, 4.f,
                                       f.done ? green : blue);
        // 状态文本
        const char* status = f.done ? "完成" : "上传中";
        const float tw = font_->measure_text(status);
        font_->draw_text(renderer, status, bounds_.x + bounds_.w - tw - 8.f, y + 2.f,
                         f.done ? green : light);
        y += 48.f;
    }
}

}  // namespace ui
