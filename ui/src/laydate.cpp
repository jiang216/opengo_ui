#include "ui/ui/laydate.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

namespace ui {

namespace {
const char* kWd[] = {"日", "一", "二", "三", "四", "五", "六"};
const char* kWdEn[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
const char* kMonthNames[] = {"一月", "二月", "三月", "四月", "五月", "六月",
                              "七月", "八月", "九月", "十月", "十一月", "十二月"};
const char* kMonthNamesEn[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const int kDaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 简易公历节日（月-日 → 名称）
struct Holiday { int m, d; const char* name; };
const Holiday kHolidays[] = {
    {1, 1, "元旦"}, {2, 14, "情人节"}, {3, 8, "妇女节"}, {3, 12, "植树节"},
    {4, 1, "愚人节"}, {5, 1, "劳动节"}, {5, 4, "青年节"}, {6, 1, "儿童节"},
    {7, 1, "建党节"}, {8, 1, "建军节"}, {9, 10, "教师节"}, {10, 1, "国庆节"},
    {12, 25, "圣诞节"},
};

bool is_en(const std::string& lang) { return lang == "en"; }
const char* wd_name(int i, bool en) { return en ? kWdEn[i] : kWd[i]; }
const char* month_name(int m, bool en) { return en ? kMonthNamesEn[m - 1] : kMonthNames[m - 1]; }

// 解析 "yyyy-MM-dd" 或 "HH:mm:ss"
bool parse_date_str(const char* s, int& y, int& m, int& d) {
    if (!s || !*s) return false;
    return std::sscanf(s, "%d-%d-%d", &y, &m, &d) == 3;
}
bool parse_time_str(const char* s, int& h, int& mi, int& s2) {
    if (!s || !*s) return false;
    return std::sscanf(s, "%d:%d:%d", &h, &mi, &s2) == 3;
}
bool parse_datetime_str(const char* s, int& y, int& m, int& d, int& h, int& mi, int& s2) {
    if (!s || !*s) return false;
    return std::sscanf(s, "%d-%d-%d %d:%d:%d", &y, &m, &d, &h, &mi, &s2) >= 3;
}

bool in_range(int v, int lo, int hi) { return v >= lo && v <= hi; }

// 公历日期 → 天数序数（用于 min/max 比较）+ 反向
long days_from_civil(int y, int m, int d) {
    if (m <= 2) { y -= 1; m += 12; }
    long era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = (unsigned)(y - era * 400);
    unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (long)doe - 719468;
}
void civil_from_days(long z, int& y, int& m, int& d) {
    z += 719468;
    long era = (z >= 0 ? z : z - 146096) / 146097;
    unsigned doe = (unsigned)(z - era * 146097);
    unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long yr = (long)yoe + era * 400;
    unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    unsigned mp = (5 * doy + 2) / 153;
    d = (int)(doy - (153 * mp + 2) / 5 + 1);
    m = (int)(mp + (mp < 10 ? 3 : -9));
    y = (int)(yr + (m <= 2 ? 1 : 0));
}
void today_ymd(int& y, int& m, int& d) {
    std::time_t t = std::time(nullptr);
    std::tm lm;
#if defined(_WIN32)
    localtime_s(&lm, &t);
#else
    localtime_r(&t, &lm);
#endif
    y = lm.tm_year + 1900;
    m = lm.tm_mon + 1;
    d = lm.tm_mday;
}
}  // namespace

// ---- 构造 / set_type ----

Laydate::Laydate(float x, float y, float w, float h) : Widget(x, y, w, h) {}

void Laydate::set_type(const char* t) {
    if (!t) return;
    if (std::strcmp(t, "year") == 0) type_ = TypeYear;
    else if (std::strcmp(t, "month") == 0) type_ = TypeMonth;
    else if (std::strcmp(t, "time") == 0) type_ = TypeTime;
    else if (std::strcmp(t, "datetime") == 0) type_ = TypeDateTime;
    else type_ = TypeDate;
}

void Laydate::set_value(const char* v) {
    if (!v || !*v) return;
    int y = 0, m = 0, d = 0, h = 0, mi = 0, s = 0;
    if (type_ == TypeTime) {
        if (parse_time_str(v, h, mi, s)) { hour_ = h; minute_ = mi; second_ = s; }
    } else if (type_ == TypeDateTime) {
        if (parse_datetime_str(v, y, m, d, h, mi, s)) {
            year_ = y; month_ = m; day_ = d; hour_ = h; minute_ = mi; second_ = s;
        }
    } else {
        if (parse_date_str(v, y, m, d)) { year_ = y; month_ = m; day_ = d; }
    }
}

// ---- 月份工具 ----

int Laydate::days_in_month(int y, int m) const {
    int d = kDaysInMonth[m - 1];
    if (m == 2 && is_leap(y)) d = 29;
    return d;
}

void Laydate::month_info(int* first_weekday, int* days) const {
    int y = year_, m = month_;
    if (m < 3) { m += 12; --y; }
    const int k = y % 100, j = y / 100;
    int h = (1 + 13 * (m + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    *first_weekday = (h + 6) % 7;
    *days = days_in_month(year_, month_);
}

void Laydate::next_month() { if (++month_ > 12) { month_ = 1; ++year_; } }
void Laydate::prev_month() { if (--month_ < 1) { month_ = 12; --year_; } }
void Laydate::next_year() { ++year_; }
void Laydate::prev_year() { --year_; }
void Laydate::select_today() {
    // 简化：使用当前显示的年月的1号
    day_ = 1;
    if (on_change_) on_change_(year_, month_, day_);
}

bool Laydate::date_selectable(int y, int m, int d) const {
    const bool has_min = min_offset_set_ || !min_.empty();
    const bool has_max = max_offset_set_ || !max_.empty();
    if (!has_min && !has_max) return true;
    long val = days_from_civil(y, m, d);
    if (min_offset_set_) {
        int ty, tm, td;
        today_ymd(ty, tm, td);
        if (val < days_from_civil(ty, tm, td) + min_offset_) return false;
    } else if (!min_.empty()) {
        int my = 0, mm = 0, md = 0;
        if (parse_date_str(min_.c_str(), my, mm, md) && val < days_from_civil(my, mm, md)) return false;
    }
    if (max_offset_set_) {
        int ty, tm, td;
        today_ymd(ty, tm, td);
        if (val > days_from_civil(ty, tm, td) + max_offset_) return false;
    } else if (!max_.empty()) {
        int my = 0, mm = 0, md = 0;
        if (parse_date_str(max_.c_str(), my, mm, md) && val > days_from_civil(my, mm, md)) return false;
    }
    return true;
}

// ---- 主题色 ----

Laydate::Colors Laydate::colors() const {
    Colors c;
    if (theme_style_ == "molv") {
        c = {{0.16f,0.20f,0.18f,1}, {0.12f,0.16f,0.14f,1}, {0.16f,0.22f,0.18f,1}, {0.85f,0.92f,0.87f,1},
             {0.55f,0.65f,0.58f,1}, {0.85f,0.4f,0.4f,1},
             {0.2f,0.2f,0.2f,1}, {0.88f,0.94f,0.90f,1}, {0.27f,0.63f,0.38f,1}, {1,1,1,1},
             {0.27f,0.63f,0.38f,1}, {0.88f,0.94f,0.90f,0.3f}, {0.27f,0.63f,0.38f,1},
             {0.16f,0.20f,0.18f,1}, {0.85f,0.92f,0.87f,1}, {0.20f,0.28f,0.22f,1},
             {0.88f,0.94f,0.90f,0.25f},
             {0.96f,0.96f,0.96f,1}, {0.2f,0.2f,0.2f,1}, {0.27f,0.63f,0.38f,1}};
    } else if (theme_style_ == "grid") {
        c = {{1,1,1,1}, {0.85f,0.85f,0.85f,1}, {1,1,1,1}, {0.2f,0.2f,0.2f,1},
             {0.5f,0.5f,0.5f,1}, {0.8f,0.3f,0.3f,1},
             {0.2f,0.2f,0.2f,1}, {0.92f,0.92f,0.92f,1}, {0.12f,0.62f,1.f,1}, {1,1,1,1},
             {0.12f,0.62f,1.f,1}, {0.95f,0.95f,0.95f,1}, {0.8f,0.3f,0.3f,1},
             {0.97f,0.97f,0.97f,1}, {0.12f,0.62f,1.f,1}, {0.90f,0.90f,0.90f,1},
             {0.85f,0.92f,1.f,0.2f},
             {1,1,1,1}, {0.2f,0.2f,0.2f,1}, {0.12f,0.62f,1.f,1}};
    } else if (!theme_style_.empty() && theme_style_[0] == '#') {
        // 自定义颜色：从 theme_style_ 解析主色调
        float r = 0.12f, g = 0.62f, b = 1.f;
        if (theme_style_.size() >= 7) {
            auto hex = [](const char* p) -> float {
                unsigned int v = 0;
                for (int i = 0; i < 2; ++i) {
                    char ch = p[i];
                    if (ch >= '0' && ch <= '9') v = v * 16 + (ch - '0');
                    else if (ch >= 'a' && ch <= 'f') v = v * 16 + (ch - 'a' + 10);
                    else if (ch >= 'A' && ch <= 'F') v = v * 16 + (ch - 'A' + 10);
                }
                return v / 255.f;
            };
            r = hex(&theme_style_[1]); g = hex(&theme_style_[3]); b = hex(&theme_style_[5]);
        }
        c = {{1,1,1,1}, {0.85f,0.85f,0.85f,1}, {1,1,1,1}, {0.2f,0.2f,0.2f,1},
             {0.5f,0.5f,0.5f,1}, {0.8f,0.3f,0.3f,1},
             {0.2f,0.2f,0.2f,1}, {0.92f,0.92f,0.92f,1}, {r,g,b,1}, {1,1,1,1},
             {r,g,b,1}, {0.95f,0.95f,0.95f,1}, {0.8f,0.3f,0.3f,1},
             {0.97f,0.97f,0.97f,1}, {r,g,b,1}, {0.90f,0.90f,0.90f,1},
             {r,g,b,0.15f},
             {1,1,1,1}, {0.2f,0.2f,0.2f,1}, {r,g,b,1}};
    } else {
        // 默认蓝色主题
        c = {{1,1,1,1}, {0.85f,0.85f,0.85f,1}, {0.96f,0.96f,0.96f,1}, {0.3f,0.3f,0.3f,1},
             {0.5f,0.5f,0.5f,1}, {0.8f,0.3f,0.3f,1},
             {0.3f,0.3f,0.3f,1}, {0.92f,0.92f,0.92f,1}, {0.12f,0.62f,1.f,1}, {1,1,1,1},
             {0.12f,0.62f,1.f,1}, {0.95f,0.95f,0.95f,1}, {0.8f,0.3f,0.3f,1},
             {0.97f,0.97f,0.97f,1}, {0.12f,0.62f,1.f,1}, {0.90f,0.90f,0.90f,1},
             {0.85f,0.92f,1.f,0.2f},
             {1,1,1,1}, {0.3f,0.3f,0.3f,1}, {0.12f,0.62f,1.f,1}};
    }
    return c;
}

// ---- 格式化 ----

std::string Laydate::format_value() const {
    char buf[64];
    if (!format_.empty()) {
        // 简易 format 解析
        std::string result;
        const char* p = format_.c_str();
        while (*p) {
            if (*p == 'y' || *p == 'M' || *p == 'd' || *p == 'H' || *p == 'm' || *p == 's') {
                const char* start = p;
                while (*p == *start) ++p;
                int len = (int)(p - start);
                if (*start == 'y') { std::snprintf(buf, sizeof(buf), "%0*d", len, year_); result += buf; }
                else if (*start == 'M') {
                    if (len <= 2) { std::snprintf(buf, sizeof(buf), "%0*d", len, month_); result += buf; }
                    else { result += month_name(month_, is_en(lang_)); }
                }
                else if (*start == 'd') { std::snprintf(buf, sizeof(buf), "%0*d", len, day_); result += buf; }
                else if (*start == 'H') { std::snprintf(buf, sizeof(buf), "%0*d", len, hour_); result += buf; }
                else if (*start == 'm') { std::snprintf(buf, sizeof(buf), "%0*d", len, minute_); result += buf; }
                else if (*start == 's') { std::snprintf(buf, sizeof(buf), "%0*d", len, second_); result += buf; }
            } else {
                result += *p;
                ++p;
            }
        }
        return result;
    }
    // 默认格式
    switch (type_) {
        case TypeYear: std::snprintf(buf, sizeof(buf), "%04d", year_); break;
        case TypeMonth: std::snprintf(buf, sizeof(buf), "%04d-%02d", year_, month_); break;
        case TypeTime: std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hour_, minute_, second_); break;
        case TypeDateTime:
            std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d", year_, month_, day_, hour_, minute_, second_);
            break;
        default: std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year_, month_, day_); break;
    }
    return buf;
}

// ---- 布局常量 ----

static constexpr float kHeaderH = 36.f;
static constexpr float kWeekdayH = 24.f;
static constexpr float kFooterH = 38.f;
static constexpr float kCellPad = 2.f;

// ---- 绘制 ----

void Laydate::on_draw(Renderer& renderer) {
    Colors col = colors();
    // 背景 + 边框
    renderer.draw_quad(bounds_, col.bg);
    renderer.draw_rect_outline(bounds_, 1.f, col.border);

    if (type_ == TypeTime) {
        draw_time_panel(renderer);
        if (show_bottom_) draw_footer(renderer);
        return;
    }
    draw_header(renderer);
    if (type_ == TypeDate || type_ == TypeDateTime) {
        draw_weekday_row(renderer);
        draw_date_grid(renderer);
    } else if (type_ == TypeMonth) {
        draw_month_grid(renderer);
    } else if (type_ == TypeYear) {
        draw_year_grid(renderer);
    }
    if (type_ == TypeDateTime) {
        // datetime 模式底部追加时间选择区域
        draw_time_panel(renderer);
    }
    if (show_bottom_) draw_footer(renderer);
}

void Laydate::draw_header(Renderer& r) {
    Colors col = colors();
    const bool en = is_en(lang_);
    Rect hdr{bounds_.x, bounds_.y, bounds_.w, kHeaderH};
    r.draw_quad(hdr, col.header_bg);

    if (!font_) return;
    const float ax = bounds_.x + bounds_.w / 2.f;
    const float aw = 18.f;
    // 左箭头
    font_->draw_text(r, "<", bounds_.x + 8.f, bounds_.y + 9.f, col.header_text);
    // 标题
    char title[64];
    if (type_ == TypeYear) {
        int base = year_grid_base_ > 0 ? year_grid_base_ : (year_ / 10) * 10;
        std::snprintf(title, sizeof(title), "%d - %d", base, base + 9);
    } else if (type_ == TypeMonth) {
        std::snprintf(title, sizeof(title), "%d", year_);
    } else {
        std::snprintf(title, sizeof(title), "%d年 %s", year_, month_name(month_, en));
    }
    const float tw = font_->measure_text(title);
    font_->draw_text(r, title, ax - tw / 2.f, bounds_.y + 9.f, col.header_text);
    // 右箭头
    font_->draw_text(r, ">", bounds_.x + bounds_.w - 20.f, bounds_.y + 9.f, col.header_text);
}

void Laydate::draw_weekday_row(Renderer& r) {
    Colors col = colors();
    const bool en = is_en(lang_);
    const float y0 = bounds_.y + kHeaderH;
    const float cw = bounds_.w / 7.f;
    for (int i = 0; i < 7; ++i) {
        const Vec4 tc = (i == 0 || i == 6) ? col.weekend_text : col.weekday_text;
        const char* txt = wd_name(i, en);
        const float tw = font_ ? font_->measure_text(txt) : 0;
        font_->draw_text(r, txt, bounds_.x + i * cw + (cw - tw) / 2.f, y0 + 5.f, tc);
    }
}

void Laydate::draw_date_grid(Renderer& r) {
    Colors col = colors();
    int fw, dim;
    month_info(&fw, &dim);
    const float grid_top = bounds_.y + kHeaderH + kWeekdayH;
    const float cell_w = bounds_.w / 7.f;
    float avail_h = bounds_.h - kWeekdayH - kHeaderH;
    if (show_bottom_) avail_h -= kFooterH;
    if (type_ == TypeDateTime) avail_h -= 80.f;  // 时间区域
    const float cell_h = avail_h / 6.f;

    const bool en = is_en(lang_);
    char buf[16];
    for (int d = 1; d <= dim; ++d) {
        const int cell = fw + d - 1;
        const int row = cell / 7, c = cell % 7;
        const float cx = bounds_.x + c * cell_w;
        const float cy = grid_top + row * cell_h;

        const bool is_today = (d == day_);
        const bool is_weekend = (c == 0 || c == 6);

        if (is_today) {
            r.draw_rounded_rect(Rect{cx + kCellPad, cy + kCellPad, cell_w - kCellPad * 2, cell_h - kCellPad * 2},
                                   3.f, col.day_selected_bg);
        }
        std::snprintf(buf, sizeof(buf), "%d", d);
        Vec4 tc = is_today ? col.day_selected_text : (is_weekend ? col.weekend_text : col.day_text);
        const float tw = font_ ? font_->measure_text(buf) : 0;
        font_->draw_text(r, buf, cx + (cell_w - tw) / 2.f, cy + kCellPad + 2.f, tc);

        // 标注
        if (calendar_) {
            char mk_key[16];
            std::snprintf(mk_key, sizeof(mk_key), "%d-%02d-%02d", 0, month_, d);
            for (auto& mk : marks_) {
                if (mk.key == mk_key || mk.key == std::to_string(d)) {
                    font_->draw_text(r, mk.text.c_str(), cx + 2.f, cy + cell_h - 10.f, col.mark_text);
                }
            }
        }
    }
}

void Laydate::draw_month_grid(Renderer& r) {
    Colors col = colors();
    const bool en = is_en(lang_);
    const float grid_top = bounds_.y + kHeaderH + 8.f;
    const float cw = bounds_.w / 4.f;
    float avail_h = bounds_.h - kHeaderH - 16.f;
    if (show_bottom_) avail_h -= kFooterH;
    const float ch = avail_h / 3.f;

    for (int i = 0; i < 12; ++i) {
        const int row = i / 4, c = i % 4;
        const float cx = bounds_.x + c * cw;
        const float cy = grid_top + row * ch;
        const bool is_sel = (i + 1 == month_);
        if (is_sel) {
            r.draw_rounded_rect(Rect{cx + kCellPad, cy + kCellPad, cw - kCellPad * 2, ch - kCellPad * 2},
                                   3.f, col.day_selected_bg);
        }
        const char* txt = month_name(i + 1, en);
        const float tw = font_ ? font_->measure_text(txt) : 0;
        font_->draw_text(r, txt, cx + (cw - tw) / 2.f, cy + (ch - 14.f) / 2.f,
                         is_sel ? col.day_selected_text : col.day_text);
    }
}

void Laydate::draw_year_grid(Renderer& r) {
    Colors col = colors();
    int base = year_grid_base_ > 0 ? year_grid_base_ : (year_ / 10) * 10;
    const float grid_top = bounds_.y + kHeaderH + 8.f;
    const float cw = bounds_.w / 4.f;
    float avail_h = bounds_.h - kHeaderH - 16.f;
    if (show_bottom_) avail_h -= kFooterH;
    const float ch = avail_h / 3.f;

    for (int i = 0; i < 12; ++i) {
        const int yr = base + i - 1;  // 第一行第一格是 base-1
        if (i == 0) {
            // 占位格（base-1，灰色显示上一个年代的年）
            char buf[8];
            std::snprintf(buf, sizeof(buf), "%d", yr);
            const int row = i / 4, c = i % 4;
            const float cx = bounds_.x + c * cw;
            const float cy = grid_top + row * ch;
            const float tw = font_ ? font_->measure_text(buf) : 0;
            font_->draw_text(r, buf, cx + (cw - tw) / 2.f, cy + (ch - 14.f) / 2.f, col.weekday_text);
            continue;
        }
        const int yr2 = base + i;
        char buf[8];
        std::snprintf(buf, sizeof(buf), "%d", yr2);
        const int row = i / 4, c = i % 4;
        const float cx = bounds_.x + c * cw;
        const float cy = grid_top + row * ch;
        const bool is_sel = (yr2 == year_);
        if (is_sel) {
            r.draw_rounded_rect(Rect{cx + kCellPad, cy + kCellPad, cw - kCellPad * 2, ch - kCellPad * 2},
                                   3.f, col.day_selected_bg);
        }
        const float tw = font_ ? font_->measure_text(buf) : 0;
        font_->draw_text(r, buf, cx + (cw - tw) / 2.f, cy + (ch - 14.f) / 2.f,
                         is_sel ? col.day_selected_text : col.day_text);
    }
}

void Laydate::draw_time_panel(Renderer& r) {
    Colors col = colors();
    const float y0 = (type_ == TypeDateTime)
        ? bounds_.y + kHeaderH + kWeekdayH + (bounds_.h - kHeaderH - kWeekdayH - (show_bottom_ ? kFooterH : 0.f)) * 0.6f
        : bounds_.y;
    const float panel_h = (type_ == TypeDateTime)
        ? (bounds_.h - kHeaderH - kWeekdayH - (show_bottom_ ? kFooterH : 0.f)) * 0.4f
        : bounds_.h - (show_bottom_ ? kFooterH : 0.f);
    const float col_w = bounds_.w / 3.f;

    // 背景
    r.draw_quad(Rect{bounds_.x, y0, bounds_.w, panel_h}, col.time_col_bg);
    // 分隔线
    r.draw_quad(Rect{bounds_.x + col_w, y0, 1.f, panel_h}, col.border);
    r.draw_quad(Rect{bounds_.x + col_w * 2, y0, 1.f, panel_h}, col.border);

    if (!font_) return;
    // 三列：时 / 分 / 秒
    const int vals[3] = {hour_, minute_, second_};
    for (int col_idx = 0; col_idx < 3; ++col_idx) {
        const float x0 = bounds_.x + col_idx * col_w;
        const int max_val = (col_idx == 0) ? 23 : 59;
        const int cur = vals[col_idx];
        // 显示当前值及上下各2个
        for (int offset = -2; offset <= 2; ++offset) {
            int v = (cur + offset + max_val + 1) % (max_val + 1);
            char buf[8];
            std::snprintf(buf, sizeof(buf), "%02d", v);
            const float iy = y0 + panel_h / 2.f + offset * 18.f - 7.f;
            const bool is_sel = (offset == 0);
            if (is_sel) {
                r.draw_rounded_rect(Rect{x0 + 4.f, iy - 2.f, col_w - 8.f, 20.f}, 3.f, col.time_item_selected_bg);
            }
            const float tw = font_->measure_text(buf);
            font_->draw_text(r, buf, x0 + (col_w - tw) / 2.f, iy,
                             is_sel ? col.day_selected_text : col.time_item_text);
        }
    }
}

void Laydate::draw_footer(Renderer& r) {
    Colors col = colors();
    const float fy = bounds_.y + bounds_.h - kFooterH;
    r.draw_quad(Rect{bounds_.x, fy, bounds_.w, kFooterH}, col.footer_bg);
    r.draw_quad(Rect{bounds_.x, fy, bounds_.w, 1.f}, col.footer_divider);

    if (!font_ || btns_.empty()) return;
    const bool en = is_en(lang_);
    const char* labels_en[] = {"clear", "today", "confirm"};
    const char* labels_cn[] = {"清空", "今天", "确定"};
    float xo = bounds_.x + 12.f;
    for (size_t i = 0; i < btns_.size(); ++i) {
        const char* txt = btns_[i].c_str();
        // 匹配预定义按钮名
        for (int k = 0; k < 3; ++k) {
            if (btns_[i] == labels_en[k]) { txt = en ? labels_en[k] : labels_cn[k]; break; }
        }
        const float tw = font_->measure_text(txt);
        font_->draw_text(r, txt, xo, fy + 12.f, col.footer_btn_text);
        xo += tw + 16.f;
    }
}

// ---- 命中测试 ----

bool Laydate::hit_header(float x, float y) const {
    return x >= bounds_.x && x <= bounds_.x + bounds_.w && y >= bounds_.y && y <= bounds_.y + kHeaderH;
}

bool Laydate::hit_grid(float x, float y, int& out_row, int& out_col) const {
    const float grid_top = bounds_.y + kHeaderH + kWeekdayH;
    float avail_h = bounds_.h - kHeaderH - kWeekdayH;
    if (show_bottom_) avail_h -= kFooterH;
    if (type_ == TypeDateTime) avail_h -= 80.f;
    const float cell_w = bounds_.w / 7.f;
    const float cell_h = avail_h / 6.f;
    if (x < bounds_.x || x > bounds_.x + bounds_.w || y < grid_top || y > grid_top + cell_h * 6)
        return false;
    out_row = (int)((y - grid_top) / cell_h);
    out_col = (int)((x - bounds_.x) / cell_w);
    return true;
}

bool Laydate::hit_footer(float x, float y, int& out_btn) const {
    if (!show_bottom_ || btns_.empty()) return false;
    const float fy = bounds_.y + bounds_.h - kFooterH;
    if (y < fy || y > fy + kFooterH || x < bounds_.x || x > bounds_.x + bounds_.w)
        return false;
    // 简化：按位置比例分配按钮
    float xo = bounds_.x + 12.f;
    for (size_t i = 0; i < btns_.size(); ++i) {
        float tw = font_ ? font_->measure_text(btns_[i].c_str()) : 40.f;
        if (x >= xo && x <= xo + tw + 8.f) { out_btn = (int)i; return true; }
        xo += tw + 16.f;
    }
    return false;
}

bool Laydate::hit_time(float x, float y, int& out_col, int& out_idx) const {
    const float y0 = (type_ == TypeDateTime)
        ? bounds_.y + kHeaderH + kWeekdayH + (bounds_.h - kHeaderH - kWeekdayH - (show_bottom_ ? kFooterH : 0.f)) * 0.6f
        : bounds_.y;
    const float panel_h = (type_ == TypeDateTime)
        ? (bounds_.h - kHeaderH - kWeekdayH - (show_bottom_ ? kFooterH : 0.f)) * 0.4f
        : bounds_.h - (show_bottom_ ? kFooterH : 0.f);
    const float col_w = bounds_.w / 3.f;
    if (y < y0 || y > y0 + panel_h) return false;
    out_col = (int)((x - bounds_.x) / col_w);
    if (out_col < 0 || out_col > 2) return false;
    int cur = (out_col == 0) ? hour_ : (out_col == 1) ? minute_ : second_;
    float cy = y0 + panel_h / 2.f - 7.f;
    float dy = y - cy;
    out_idx = (int)(dy / 18.f + 0.5f);
    if (out_idx < -2 || out_idx > 2) return false;
    return true;
}

// ---- 事件 ----

bool Laydate::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    Colors col = colors();
    const bool en = is_en(lang_);

    // 头部箭头
    if (hit_header(x, y)) {
        if (x < bounds_.x + bounds_.w / 2.f - 20.f) {
            if (type_ == TypeYear) { year_grid_base_ -= 10; return true; }
            if (type_ == TypeMonth) { --year_; return true; }
            prev_month();
        } else if (x > bounds_.x + bounds_.w / 2.f + 20.f) {
            if (type_ == TypeYear) { year_grid_base_ += 10; return true; }
            if (type_ == TypeMonth) { ++year_; return true; }
            next_month();
        } else {
            // 点击标题：降级视图
            if (type_ == TypeDate) { type_ = TypeMonth; return true; }
            if (type_ == TypeMonth) { type_ = TypeYear; year_grid_base_ = (year_ / 10) * 10; return true; }
        }
        return true;
    }

    // 日期网格
    if ((type_ == TypeDate || type_ == TypeDateTime) && !(type_ == TypeDateTime && y > bounds_.y + bounds_.h * 0.7f)) {
        int row = 0, col = 0;
        if (hit_grid(x, y, row, col)) {
            int fw, dim;
            month_info(&fw, &dim);
            int day_num = row * 7 + col - fw + 1;
            if (day_num >= 1 && day_num <= dim && date_selectable(year_, month_, day_num)) {
                if (range_) {
                    if (!picking_end_) {
                        year_ = year_; month_ = month_; day_ = day_num;
                        picking_end_ = true;
                    } else {
                        range_y2_ = year_; range_m2_ = month_; range_d2_ = day_num;
                        picking_end_ = false;
                    }
                } else {
                    day_ = day_num;
                }
                if (on_change_) on_change_(year_, month_, day_);
                return true;
            }
        }
    }

    // 月份网格
    if (type_ == TypeMonth) {
        const float grid_top = bounds_.y + kHeaderH + 8.f;
        float avail_h = bounds_.h - kHeaderH - 16.f;
        if (show_bottom_) avail_h -= kFooterH;
        const float cw = bounds_.w / 4.f;
        const float ch = avail_h / 3.f;
        if (x >= bounds_.x && x <= bounds_.x + bounds_.w && y >= grid_top && y <= grid_top + ch * 3) {
            int c = (int)((x - bounds_.x) / cw);
            int row = (int)((y - grid_top) / ch);
            int m = row * 4 + c + 1;
            if (m >= 1 && m <= 12) {
                month_ = m;
                type_ = TypeDate;  // 选中月份后回到日期视图
                if (on_change_) on_change_(year_, month_, day_);
                return true;
            }
        }
    }

    // 年份网格
    if (type_ == TypeYear) {
        int base = year_grid_base_ > 0 ? year_grid_base_ : (year_ / 10) * 10;
        const float grid_top = bounds_.y + kHeaderH + 8.f;
        float avail_h = bounds_.h - kHeaderH - 16.f;
        if (show_bottom_) avail_h -= kFooterH;
        const float cw = bounds_.w / 4.f;
        const float ch = avail_h / 3.f;
        if (x >= bounds_.x && x <= bounds_.x + bounds_.w && y >= grid_top && y <= grid_top + ch * 3) {
            int c = (int)((x - bounds_.x) / cw);
            int row = (int)((y - grid_top) / ch);
            int idx = row * 4 + c;
            if (idx >= 1 && idx <= 10) {
                year_ = base + idx;
                type_ = TypeMonth;  // 选中年份后进入月份视图
                if (on_change_) on_change_(year_, month_, day_);
                return true;
            }
        }
    }

    // 时间面板
    if (type_ == TypeTime || type_ == TypeDateTime) {
        int tc = 0, ti = 0;
        if (hit_time(x, y, tc, ti)) {
            int cur = (tc == 0) ? hour_ : (tc == 1) ? minute_ : second_;
            int max_val = (tc == 0) ? 23 : 59;
            int new_val = (cur + ti + max_val + 1) % (max_val + 1);
            if (tc == 0) hour_ = new_val;
            else if (tc == 1) minute_ = new_val;
            else second_ = new_val;
            if (on_change_) on_change_(year_, month_, day_);
            return true;
        }
    }

    // 底部按钮
    int btn = -1;
    if (hit_footer(x, y, btn)) {
        const char* labels[] = {"clear", "today", "confirm"};
        if (btn < 3) {
            if (std::strcmp(labels[btn], "clear") == 0) {
                day_ = 1; hour_ = 0; minute_ = 0; second_ = 0;
            } else if (std::strcmp(labels[btn], "today") == 0) {
                select_today();
            } else if (std::strcmp(labels[btn], "confirm") == 0) {
                if (on_done_) {
                    std::string val = format_value();
                    on_done_(val.c_str(), year_, month_, day_, hour_, minute_, second_);
                }
            }
        }
        return true;
    }

    return false;
}

}  // namespace ui
