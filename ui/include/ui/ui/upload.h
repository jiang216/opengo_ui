#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 上传组件（M17d，layui-upload 克隆）：选择文件按钮 + 文件列表（名称/大小/进度条/状态）。
// 应用层调 add_file() 添加、advance(dt) 推进进度、remove 完成项。
class Upload : public Widget {
public:
    Upload(float x, float y, float w, float h);

    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }
    // 添加一个模拟文件（文件名 + 大小字节），开始上传
    void add_file(const char* name, int size_bytes);
    void advance(float dt);
    bool on_mouse_down(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

    std::function<void()> on_pick;  // 应用层可覆盖"选择文件"行为

private:
    struct File {
        std::string name;
        int size = 0;
        float progress = 0.f;
        bool done = false;
    };
    void remove_done();

    std::vector<File> files_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
};

}  // namespace ui
