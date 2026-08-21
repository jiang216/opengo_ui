#pragma once

#include "ui/ui/laydate.h"
#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;
class Laydate;

// 日期/时间选择输入框（layui laydate 弹层交互的一比一克隆）：
// 视觉是一个输入框（显示当前值 / 占位符 / 只读），点击弹出下方日期面板（根级 popup）。
// 面板配置通过 set_* 透传至内部 Laydate；选中/确定后写回输入框文本并回调。
// 对应 laydate.html 中 input + laydate.render({...}) 的弹层模式（test1..test31）。
class LaydateBox : public Widget {
public:
    LaydateBox(float x, float y, float w, float h);
    ~LaydateBox() override;

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    // ---- 配置透传（等价 laydate.render 参数） ----
    void set_type(const char* t);          // year / month / date(默认) / time / datetime
    void set_range(bool v);                // 范围选择
    void set_range_separator(const char* s);
    void set_format(const char* f);
    void set_min(const char* v);
    void set_max(const char* v);
    void set_min_offset(int days);
    void set_max_offset(int days);
    void set_calendar(bool v);
    void set_marks(const std::vector<Laydate::Mark>& m);
    void set_theme_style(const char* t);
    void set_show_bottom(bool v);
    void set_btns(const std::vector<std::string>& b);
    void set_lang(const char* l);

    // ---- 输入框 ----
    void set_text(const char* t);
    const std::string& text() const { return text_; }
    void set_placeholder(const char* p) { placeholder_ = p ? p : ""; }
    void set_readonly(bool v) { readonly_ = v; }
    void set_value(const char* v);                // 初始赋值（内部面板 set_value）
    void set_date(int y, int m, int d) { panel_->set_date(y, m, d); }
    void set_time(int h, int mi, int s) { panel_->set_time(h, mi, s); }
    void set_on_done(std::function<void(const char*, int, int, int, int, int, int)> cb);
    void set_on_change(std::function<void(const char*)> cb);   // 值写回输入框后回调（值为字符串）
    void set_on_ready(std::function<void()> cb) { on_ready_ = std::move(cb); }

    void open();      // 展开面板
    void close();     // 收起面板
    bool is_open() const { return open_; }
    Widget* panel() const { return panel_; }

    // ---- Widget 覆写 ----
    bool on_mouse_down(float x, float y, int button) override;
    void on_layout() override;
    void on_draw(Renderer& renderer) override;

private:
    void sync_panel();
    void apply_value_from_panel();  // 面板选中/确定后写回 text_ + 回调

    std::string text_;
    std::string placeholder_;
    bool readonly_ = false;
    bool open_ = false;
    std::string type_ = "date";
    Laydate* panel_ = nullptr;      // 弹出面板（层级成员，挂 root 时由 close 摘除）
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(const char*)> on_change_;
    std::function<void()> on_ready_;
};

}  // namespace ui