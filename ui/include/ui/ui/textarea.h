#pragma once

#include "ui/ui/scrollbar_style.h"
#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

struct LineSpan {
    size_t start;
    size_t end;
};

// 多行文本输入框（M4 扩展）：Enter 换行、↑↓/Home/End/退格/Delete、按宽度逐字符自动换行，
// 超宽纵向滚动保证光标可见；空文本时显示 placeholder 灰字提示。
//
// 选区扩展：支持鼠标拖拽选择、Ctrl+A 全选、Ctrl+C/X 剪贴板、Ctrl+V 粘贴、
// Shift+方向键扩展选区、选区绘制高亮、只读模式（readonly）。
class TextArea : public Widget {
public:
    TextArea(float x, float y, float w, float h);

    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }
    void set_placeholder(const char* text) {  placeholder_ = text ? text : ""; mark_dirty(); }

    void set_text(const char* text);
    const std::string& text() const { return text_; }
    void set_caret(size_t pos) {  caret_ = (pos <= text_.size()) ? pos : text_.size(); mark_dirty(); }
    // 滚动到底部（日志自动跟踪最新内容）
    void scroll_to_end();
    // 文本变化（含程序 set_text）时回调
    void set_on_change(std::function<void(const char*)> cb) { on_change_ = std::move(cb); }

    // ---- 只读模式（日志显示用）----
    void set_readonly(bool v) {  readonly_ = v; mark_dirty(); }
    bool readonly() const { return readonly_; }

    // ---- 剪贴板回调（可选覆盖；未注入时使用内置 GLFW 剪贴板，UI-103）----
    void set_on_copy(std::function<void(const std::string&)> cb) { on_copy_ = std::move(cb); }
    void set_on_paste(std::function<std::string()> cb) { on_paste_ = std::move(cb); }

    // ---- 文件拖放 ----
    void set_drop_enabled(bool v) {  drop_enabled_ = v; mark_dirty(); }
    bool drop_enabled() const { return drop_enabled_; }
    void set_on_drop(std::function<void(const std::vector<std::string>&)> cb) { on_drop_cb_ = std::move(cb); }
    // 向外部暴露插入能力（供 on_drop_cb_ 回调使用）
    void insert_text(const std::string& s);

    // ---- 选区查询 ----
    bool has_selection() const { return has_selection_; }
    std::string selected_text() const;
    // 选区范围（字节偏移，a <= b）
    void selection_range(size_t& a, size_t& b) const;

    bool on_key_down(int key, int mods) override;
    bool on_char(unsigned int codepoint) override;
    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_move(float x, float y) override;
    bool on_mouse_up(float x, float y, int button) override;
    bool on_mouse_scroll(float x, float y, float dx, float dy) override;
    bool on_drop(float x, float y, const std::vector<std::string>& paths) override;
    void on_draw(Renderer& renderer) override;

    // 多行光标锚定点（UI-202，IME 候选窗跟随，与 TextBox::caret_offset 对称）：
    // x = 光标列像素偏移（控件左边界起算，含 kPad）；
    // y = 光标所在行底部像素（控件顶部起算，含滚动平移）。
    void caret_anchor(float& x, float& y) const;

    // ---- 换行缓存读写（供 wrap_lines 缓存命中用）----
    bool lines_cache_valid() const { return !lines_dirty_; }
    const std::vector<LineSpan>& cached_lines() const { return cached_lines_; }
    float cached_line_h() const { return cached_line_h_; }
    void update_lines_cache(std::vector<LineSpan> lines, float lh) {
        cached_lines_ = std::move(lines);
        cached_line_h_ = lh;
        lines_dirty_ = false;
    }

private:
    void erase_cp(bool backspace);
    void ensure_caret_visible(float line_h, size_t row);
    // 由点击点（控件内坐标）计算 caret 字节下标
    size_t caret_from_point(float local_y, float local_x) const;
    // 由 byte offset 计算 screen 坐标（控件内 y）
    float offset_to_y(size_t offset, float line_h,
                      const std::vector<LineSpan>& lines) const;
    // 选区删除（Backspace/Delete/输入时调用）
    void erase_selection();
    // 替换选区（粘贴或字符输入）
    void replace_selection(const std::string& repl);
    // 将 scroll_ clamp 到有效范围
    void clamp_scroll(float line_h);
    // 文本可用宽度（恒定让出右侧滚动条列，避免滚动条出现/消失时换行跳动）
    float avail_width() const;
    // 内容总高度（换行后行数 * 行高 + 上下 padding）
    float content_height(float line_h) const;

    std::string text_;
    std::string placeholder_;
    size_t caret_ = 0;
    float scroll_ = 0.f;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(const char*)> on_change_;

    // ---- 选区 ----
    size_t sel_start_ = 0, sel_end_ = 0, sel_anchor_ = 0;
    bool selecting_ = false;      // 鼠标拖拽中
    bool has_selection_ = false;

    // ---- 垂直滚动条（共享样式/状态，见 src/scrollbar_style.h）----
    scrollbar::State sb_;
    float mx_ = -1.f, my_ = -1.f;  // 最近鼠标点（控件内坐标，判定悬停 thumb 列）

    // ---- 只读 ----
    bool readonly_ = false;

    // ---- 拖拽自动滚屏 ----
    float scroll_margin_ = 24.f;

    // ---- 剪贴板回调 ----
    std::function<void(const std::string&)> on_copy_;
    std::function<std::string()> on_paste_;

    // ---- 文件拖放 ----
    bool drop_enabled_ = false;
    bool drop_highlight_ = false;
    std::function<void(const std::vector<std::string>&)> on_drop_cb_;

    // ---- 换行缓存（避免每帧 O(n) 重算 wrap_lines）----
    std::vector<LineSpan> cached_lines_;
    float cached_line_h_ = 0.f;
    bool lines_dirty_ = true;
    void invalidate_lines() { lines_dirty_ = true; }
};

}  // namespace ui
