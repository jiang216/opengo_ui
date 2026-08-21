#pragma once

#include "ui/ui/widget.h"

#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 代码块（M17d，layui-code 克隆）：标题条 + 行号列 + 浅色文字，
// 只读；超高内容纵向滚动（滚轮）。用 ASCII 文本逐行渲染。
// UI-###：新增 title / skin(notepad) / max_height(固定高度滚动) / 内容自适应高度 / 嵌套子 Code。
class Code : public Widget {
public:
    enum class Skin { kDefault, kNotepad };

    Code(float x, float y, float w, float h);

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }
    // 标题条左侧文本（lay-title；空则不画）
    void set_title(const char* title) { title_ = title ? title : ""; }
    // 标题条右侧语言标签（lay-lang）
    void set_lang(const char* lang) { lang_ = lang ? lang : ""; }
    void set_skin(Skin skin) { skin_ = skin; }
    // 固定最大高度（lay-height）：内容超高时内部滚动；0 表示自适应内容高度
    void set_max_height(float h) { max_height_ = h; }
    // 多行代码（\n 分隔）
    void set_code(const char* code);
    void clear() { lines_.clear(); }
    void add_line(const char* line) { lines_.push_back(line ? line : ""); }
    bool empty() const { return lines_.empty(); }

    bool on_mouse_scroll(float x, float y, float dx, float dy) override;
    float preferred_height() const override;
    void on_layout() override;
    void on_draw(Renderer& renderer) override;
    void draw(Renderer& renderer) override;

private:
    std::vector<std::string> lines_;
    std::string title_;
    std::string lang_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    float scroll_ = 0.f;
    float line_h_ = 18.f;
    Skin skin_ = Skin::kDefault;
    float max_height_ = 0.f;
};

}  // namespace ui
