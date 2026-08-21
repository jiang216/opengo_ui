#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
struct Theme;

// 日期/时间选择器（layui laydate 一比一克隆）：
// 支持 type: year / month / date / time / datetime
// 支持 range 选择、format 自定义、min/max 限定、公历节日/标注、
// 主题（默认 / molv / grid / 自定义颜色）、position:static 内嵌显示、
// 底部栏（今天/清空/确定）、done/change/ready 回调、lang:'en' 国际版。
class Laydate : public Widget {
public:
    enum Type { TypeDate, TypeYear, TypeMonth, TypeTime, TypeDateTime };

    Laydate(float x, float y, float w, float h);

    // ---- 配置（参考 layui laydate.render({...}) 参数） ----
    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    // 类型：year / month / date(默认) / time / datetime
    void set_type(Type t) { type_ = t; }
    void set_type(const char* t);
    Type type() const { return type_; }

    // 初始日期/时间
    void set_date(int y, int m, int d) { year_ = y; month_ = m; day_ = d; }
    void set_time(int h, int mi, int s) { hour_ = h; minute_ = mi; second_ = s; }
    void set_value(const char* v);  // 解析 "yyyy-MM-dd" / "HH:mm:ss" 等

    // 范围选择
    void set_range(bool v) { range_ = v; }
    bool range() const { return range_; }
    void set_range_separator(const char* s) { range_sep_ = s ? s : " - "; }

    // 自定义格式（默认根据 type 自动）
    void set_format(const char* f) { format_ = f ? f : ""; }

    // 限定范围
    void set_min(const char* v) { min_ = v ? v : ""; }
    void set_max(const char* v) { max_ = v ? v : ""; }
    // 相对今天偏移（天）：等效 min/max 的 -7/+7 形式，与 set_min/set_max 互斥时后者优先
    void set_min_offset(int days) { min_offset_set_ = true; min_offset_ = days; }
    void set_max_offset(int days) { max_offset_set_ = true; max_offset_ = days; }

    // 公历节日
    void set_calendar(bool v) { calendar_ = v; }

    // 自定义标注（"MM-DD": "文字" / "YYYY-MM-DD": "文字"）
    struct Mark { std::string key; std::string text; };
    void set_marks(const std::vector<Mark>& m) { marks_ = m; }

    // 主题：""(默认) / "molv"(墨绿) / "grid"(格子) / "#rrggbb"(自定义)
    void set_theme_style(const char* t) { theme_style_ = t ? t : ""; }

    // 底部栏
    void set_show_bottom(bool v) { show_bottom_ = v; }
    void set_btns(const std::vector<std::string>& b) { btns_ = b; }

    // 国际化
    void set_lang(const char* l) { lang_ = l ? l : ""; }

    // ---- 回调 ----
    void set_on_change(std::function<void(int, int, int)> cb) { on_change_ = std::move(cb); }
    void set_on_done(std::function<void(const char*, int, int, int, int, int, int)> cb) { on_done_ = std::move(cb); }
    void set_on_ready(std::function<void(int, int, int)> cb) { on_ready_ = std::move(cb); }

    // ---- 便捷 API ----
    void next_month();
    void prev_month();
    void next_year();
    void prev_year();
    void select_today();

    // 当前选中值的格式化字符串（用于写回输入框）
    std::string value() const { return format_value(); }

    // ---- Widget 覆写 ----
    bool on_mouse_down(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    void month_info(int* first_weekday, int* days) const;
    bool is_leap(int y) const { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }
    int days_in_month(int y, int m) const;

    // 当前视图绘制
    void draw_header(Renderer& r);
    void draw_weekday_row(Renderer& r);
    void draw_date_grid(Renderer& r);
    void draw_month_grid(Renderer& r);
    void draw_year_grid(Renderer& r);
    void draw_time_panel(Renderer& r);
    void draw_footer(Renderer& r);

    // 命中测试
    bool hit_header(float x, float y) const;
    bool hit_grid(float x, float y, int& out_row, int& out_col) const;
    bool hit_footer(float x, float y, int& out_btn) const;
    bool hit_time(float x, float y, int& out_col, int& out_idx) const;

    // 主题色
    struct Colors {
        Vec4 bg, border, header_bg, header_text;
        Vec4 weekday_text, weekend_text;
        Vec4 day_text, day_hover_bg, day_selected_bg, day_selected_text;
        Vec4 today_border, mark_bg, mark_text;
        Vec4 footer_bg, footer_btn_text, footer_divider;
        Vec4 range_bg;
        Vec4 time_col_bg, time_item_text, time_item_selected_bg;
    };
    Colors colors() const;

    // 是否可选中：受 min/max（含相对今天偏移）限制
    bool date_selectable(int y, int m, int d) const;

    // 格式化当前选中为字符串
    std::string format_value() const;

    Type type_ = TypeDate;
    int year_ = 2026, month_ = 8, day_ = 1;
    int hour_ = 0, minute_ = 0, second_ = 0;
    // 范围选择
    bool range_ = false;
    int range_y2_ = 0, range_m2_ = 0, range_d2_ = 0;
    int range_h2_ = 0, range_mi2_ = 0, range_s2_ = 0;
    bool picking_end_ = false;  // 正在选结束日期
    std::string range_sep_ = " - ";
    // format / min / max
    std::string format_;
    std::string min_, max_;
    bool min_offset_set_ = false, max_offset_set_ = false;  // 用相对今天偏移
    int min_offset_ = 0, max_offset_ = 0;                   // 相对今天的天数
    // 公历节日 / 标注
    bool calendar_ = false;
    std::vector<Mark> marks_;
    // 主题
    std::string theme_style_;
    // 底部栏
    bool show_bottom_ = true;
    std::vector<std::string> btns_ = {"clear", "today", "confirm"};
    // 国际化
    std::string lang_;
    // 回调
    std::function<void(int, int, int)> on_change_;
    std::function<void(const char*, int, int, int, int, int, int)> on_done_;
    std::function<void(int, int, int)> on_ready_;
    // year grid state
    int year_grid_base_ = 0;  // 年网格起始年
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
};

}  // namespace ui
