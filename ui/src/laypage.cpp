#include "ui/ui/laypage.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/button.h"
#include "ui/ui/combobox.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/textbox.h"
#include "ui/ui/theme.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace ui {

namespace {
constexpr float kItemH = 28.f;    // 分页条高
constexpr float kPageW = 30.f;    // 页码按钮宽
constexpr float kGap = 8.f;       // 项间距
constexpr float kTextPad = 14.f;  // 文本型项左右内边距（首页/尾页/上/下一页）
constexpr float kLimitW = 110.f;  // limit 下拉宽
constexpr float kRefreshW = 32.f;  // refresh 按钮宽
constexpr float kSkipBoxW = 46.f;  // skip 输入宽
constexpr float kSkipBtnW = 54.f;  // skip 确定按钮宽

const char* kCountFmt = "共 %d 条";
constexpr float kCountPad = 6.f;
constexpr float kMaxPages = 7;  // 可见页码上限（含省略号区）
// layout 含 limit 但未显式 set_limits 时的默认条数候选（镜像 layui 默认 limits）
const std::vector<int>& default_limits() {
    static const std::vector<int> kDefault = {10, 20, 30, 50, 100};
    return kDefault;
}
}  // namespace

Laypage::Laypage(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-laypage");
    layout_ = {Item::Prev, Item::Page, Item::Next};
}

Laypage::~Laypage() {}

void Laypage::set_total(int total) {
    count_ = 0;  // 直接设页数，不再用 count 重算
    total_ = std::max(0, total);
    if (page_ > std::max(0, total_ - 1)) page_ = std::max(0, total_ - 1);
    if (skip_box_) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d", page_ + 1);
        skip_box_->set_text(buf);
    }
}

void Laypage::set_page(int page) {
    page_ = total_ > 0 ? std::min(std::max(0, page), total_ - 1) : std::max(0, page);
    if (skip_box_) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d", page_ + 1);
        skip_box_->set_text(buf);
    }
}

void Laypage::set_count(int count) {
    count_ = std::max(0, count);
    recompute_total();
}

void Laypage::set_limit(int limit) {
    limit_ = std::max(1, limit);
    recompute_total();
}

void Laypage::set_limits(std::vector<int> limits) {
    limits_ = std::move(limits);
    if (!limits_.empty() && std::find(limits_.begin(), limits_.end(), limit_) == limits_.end())
        limit_ = limits_[0];
    recompute_total();
    sync_children();
}

void Laypage::recompute_total() {
    if (count_ > 0) total_ = std::max(1, (count_ + limit_ - 1) / limit_);
    if (page_ > std::max(0, total_ - 1)) page_ = std::max(0, total_ - 1);
    if (skip_box_) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d", page_ + 1);
        skip_box_->set_text(buf);
    }
}

void Laypage::set_layout_str(const std::vector<std::string>& items) {
    explicit_layout_ = true;
    layout_.clear();
    for (const std::string& s : items) {
        if (s == "count") layout_.push_back(Item::Count);
        else if (s == "prev") layout_.push_back(Item::Prev);
        else if (s == "page") layout_.push_back(Item::Page);
        else if (s == "next") layout_.push_back(Item::Next);
        else if (s == "limit") layout_.push_back(Item::Limit);
        else if (s == "refresh") layout_.push_back(Item::Refresh);
        else if (s == "skip") layout_.push_back(Item::Skip);
        else if (s == "first") layout_.push_back(Item::First);
        else if (s == "last") layout_.push_back(Item::Last);
    }
    sync_children();
}

void Laypage::set_font(Font* font) {
    font_ = font;
    sync_children();
}

void Laypage::set_theme(const Theme* theme) {
    theme_ = theme;
    sync_children();
}

bool Laypage::has_item(Item it) const {
    for (Item i : layout_) if (i == it) return true;
    return false;
}

void Laypage::sync_children() {
    if (has_item(Item::Limit)) {
        if (!limit_box_) {
limit_box_ = new ComboBox(0, 0, kLimitW, kItemH);
            add_child(limit_box_);
            {
                const std::vector<int>* src =
                    limits_.empty() ? &default_limits() : &limits_;
                for (int l : *src) {
                    char buf[32];
                    std::snprintf(buf, sizeof(buf), "%d 条/页", l);
                    limit_box_->add_option(buf);
                }
            }
const std::vector<int>& src =
                limits_.empty() ? default_limits() : limits_;
            limit_box_->set_on_change([this, &src](int idx) {
                if (idx >= 0 && idx < (int)src.size()) set_limit(src[idx]);
            });
        }
        limit_box_->set_font(font_);
        limit_box_->set_theme(theme_);
    }
    if (limit_box_) limit_box_->visible = has_item(Item::Limit);

    if (has_item(Item::Skip)) {
        if (!skip_box_) {
            skip_box_ = new TextBox(0, 0, kSkipBoxW, kItemH);
            skip_box_->set_text("1");
            skip_box_->set_placeholder("1");
            add_child(skip_box_);
            skip_btn_ = new Button(0, 0, kSkipBtnW, kItemH, "确定");
            add_child(skip_btn_);
            skip_btn_->set_on_click([this]() {
                const int target = std::atoi(skip_box_->text().c_str());
                jump_to(target - 1);
            });
        }
        skip_box_->set_font(font_);
        skip_box_->set_theme(theme_);
        skip_btn_->set_font(font_);
        skip_btn_->set_theme(theme_);
    }
    if (skip_box_) skip_box_->visible = has_item(Item::Skip);
    if (skip_btn_) skip_btn_->visible = has_item(Item::Skip);

    if (has_item(Item::Refresh)) {
        if (!refresh_btn_) {
            refresh_btn_ = new Button(0, 0, kRefreshW, kItemH, "");
            refresh_btn_->set_icon("refresh");
            add_child(refresh_btn_);
            refresh_btn_->set_on_click([this]() {
                if (on_refresh_) on_refresh_();
                else jump_to(page_);
            });
        }
        refresh_btn_->set_font(font_);
        refresh_btn_->set_theme(theme_);
    }
    if (refresh_btn_) refresh_btn_->visible = has_item(Item::Refresh);
}

void Laypage::visible_pages(std::vector<int>& out) const {
    out.clear();
    if (total_ <= 0) { out.push_back(0); return; }
    const int last = total_ - 1;
    if (total_ <= (int)kMaxPages) {
        for (int i = 0; i < total_; ++i) out.push_back(i);
        return;
    }
    out.push_back(0);
    if (page_ - 1 > 1) out.push_back(-1);  // 首页后省略号
    for (int i = std::max(0, page_ - 1); i <= std::min(last, page_ + 1); ++i)
        if (i != 0 && i != last) out.push_back(i);
    if (page_ + 1 < last - 1) out.push_back(-1);  // 末页前省略号
    if (last > 0) out.push_back(last);
}

float Laypage::place_items(float x) {
    item_rects_.clear();
    page_nums_.clear();
    const float cy = bounds_.y;

    std::vector<Item> seq;
    if (explicit_layout_) {
        seq = layout_;
    } else {
        if (show_first_) seq.push_back(Item::First);
        for (Item it : layout_) seq.push_back(it);
        if (show_last_) seq.push_back(Item::Last);
    }

    visible_pages(page_nums_);
    float cursor = x;

    for (Item it : seq) {
        switch (it) {
            case Item::Limit: {
                if (!limit_box_) sync_children();
                if (!limit_box_) break;
                limit_box_->set_bounds(cursor, cy, kLimitW, kItemH);
                item_rects_.push_back({Item::Limit, cursor, kLimitW});
                cursor += kLimitW + kGap;
                continue;
            }
            case Item::Refresh: {
                if (!refresh_btn_) sync_children();
                if (!refresh_btn_) break;
                refresh_btn_->set_bounds(cursor, cy, kRefreshW, kItemH);
                item_rects_.push_back({Item::Refresh, cursor, kRefreshW});
                cursor += kRefreshW + kGap;
                continue;
            }
            case Item::Skip: {
                if (!skip_box_) sync_children();
                if (!skip_box_) break;
                const float labelw = font_ ? font_->measure_text("跳至") : 24.f;
                skip_box_->set_bounds(cursor + labelw + kGap, cy, kSkipBoxW, kItemH);
                skip_btn_->set_bounds(cursor + labelw + kGap + kSkipBoxW + kGap, cy,
                                      kSkipBtnW, kItemH);
                const float w = labelw + kGap + kSkipBoxW + kGap + kSkipBtnW;
                item_rects_.push_back({Item::Skip, cursor, w});
                cursor += w + kGap;
                continue;
            }
            case Item::Page: {
                for (int p : page_nums_) {
                    const float pw = (p < 0) ? 24.f : kPageW;
                    item_rects_.push_back({Item::Page, cursor, pw});
                    cursor += pw + kGap;
                }
                continue;
            }
            case Item::Count: {
                char buf[32];
                std::snprintf(buf, sizeof(buf), kCountFmt, count_ > 0 ? count_ : limit_ * std::max(total_, 0));
                const float w = (font_ ? font_->measure_text(buf) : 20.f) + kCountPad * 2;
                item_rects_.push_back({Item::Count, cursor, w});
                cursor += w + kGap;
                continue;
            }
            default: break;
        }
        // 文本型项：First / Last / Prev / Next
        const char* txt =
            it == Item::First ? first_text_.c_str() :
            it == Item::Last  ? last_text_.c_str()  :
            it == Item::Prev  ? prev_text_.c_str()  : next_text_.c_str();
        const float w = (font_ ? font_->measure_text(txt) : 20.f) + kTextPad * 2;
        item_rects_.push_back({it, cursor, w});
        cursor += w + kGap;
    }
    return cursor;
}

bool Laypage::hit_rect(Item type, float x) const {
    for (const ItemRect& r : item_rects_)
        if (r.type == type && x >= r.x && x <= r.x + r.w) return true;
    return false;
}

void Laypage::on_layout() {
    place_items(bounds_.x);
}

void Laypage::jump_to(int page) {
    if (total_ <= 0) return;
    const int p = std::min(std::max(0, page), total_ - 1);
    if (p == page_) return;
    page_ = p;
    if (skip_box_) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d", page_ + 1);
        skip_box_->set_text(buf);
    }
    if (on_change_) on_change_(page_);
    if (on_jump_) on_jump_(page_ + 1, limit_);
}

bool Laypage::on_mouse_down(float x, float y, int button) {
    (void)y;
    if (button != 0) return false;
    for (size_t i = 0; i < item_rects_.size(); ++i) {
        const ItemRect& r = item_rects_[i];
        if (x < r.x || x > r.x + r.w) continue;
        switch (r.type) {
            case Item::Prev:
                if (page_ > 0) jump_to(page_ - 1);
                return true;
            case Item::Next:
                if (page_ < total_ - 1) jump_to(page_ + 1);
                return true;
            case Item::First:
                jump_to(0);
                return true;
            case Item::Last:
                jump_to(total_ - 1);
                return true;
            case Item::Page: {
                int idx = 0;
                for (size_t k = 0; k < item_rects_.size(); ++k) {
                    if (item_rects_[k].type == Item::Page) {
                        if (k == i) break;
                        ++idx;
                    }
                }
                if (idx < (int)page_nums_.size() && page_nums_[idx] >= 0)
                    jump_to(page_nums_[idx]);
                return true;
            }
            case Item::Limit:
            case Item::Refresh:
            case Item::Skip:
                // 子控件（limit/skip 输入框等）在 dispatch 阶段先于父节点获得事件；
                // 走到这里说明子控件未命中/未消费，本区域属于分页条自身 chrome，
                // 必须消费，防止点击穿透到下层控件（UI-122）
                return true;
            default:
                return true;
        }
    }
    // 条目间空隙 / 前后空白：同属分页条自身区域，一律消费（UI-122，
    // 与 Button::on_mouse_down、ScrollBox 面板消费同一原则），否则点击会
    // 穿透到表格行或更下层控件（规则管理页翻页穿透缺陷根源之一）。
    return true;
}

namespace {
void draw_btn(Renderer& r, Font* f, const Rect& box, const char* txt,
              const Theme& t, bool disabled) {
    const Vec4 bg = t.panel;
    const Vec4 border = disabled ? Vec4{0.93f, 0.93f, 0.93f, 1.0f} : t.border;
    const Vec4 color = disabled ? t.text_muted : t.text_secondary;
    r.draw_rounded_rect(box, 2.f, bg);
    r.draw_rect_outline(box, 1.f, border);
    const float tw = f->measure_text(txt);
    f->draw_text(r, txt, box.x + (box.w - tw) * 0.5f,
                 box.y + (box.h - f->line_height()) * 0.5f, color);
}
}  // namespace

void Laypage::on_draw(Renderer& renderer) {
    if (!font_ || item_rects_.empty()) return;
    static const Theme kDefaultTheme;
    const Theme* t = theme_ ? theme_ : &kDefaultTheme;
    const float cy = bounds_.y;
    const float text_y = cy + (kItemH - font_->line_height()) * 0.5f;
    int page_idx = 0;

    for (const ItemRect& r : item_rects_) {
        const Rect box{r.x, cy, r.w, kItemH};
        switch (r.type) {
            case Item::Count: {
                char buf[32];
                std::snprintf(buf, sizeof(buf), kCountFmt,
                              count_ > 0 ? count_ : limit_ * std::max(total_, 0));
                font_->draw_text(renderer, buf, r.x + kCountPad, text_y, t->text_secondary);
                break;
            }
            case Item::First:
                draw_btn(renderer, font_, box, first_text_.c_str(), *t, page_ == 0);
                break;
            case Item::Last:
                draw_btn(renderer, font_, box, last_text_.c_str(), *t, page_ >= total_ - 1);
                break;
            case Item::Prev:
                draw_btn(renderer, font_, box, prev_text_.c_str(), *t, page_ == 0);
                break;
            case Item::Next:
                draw_btn(renderer, font_, box, next_text_.c_str(), *t, page_ >= total_ - 1);
                break;
            case Item::Page:
                if (page_idx < (int)page_nums_.size()) {
                    const int p = page_nums_[page_idx++];
                    if (p < 0) {
                        const char* dots = "\xE2\x80\xA6";  // …
                        const float tw = font_->measure_text(dots);
                        font_->draw_text(renderer, dots, box.x + (box.w - tw) * 0.5f,
                                         text_y, t->text_muted);
                    } else if (p == page_) {
                        renderer.draw_rounded_rect(box, 2.f, theme_color_);
                        char b[8];
                        std::snprintf(b, sizeof(b), "%d", p + 1);
                        const float tw = font_->measure_text(b);
                        font_->draw_text(renderer, b, box.x + (box.w - tw) * 0.5f,
                                         text_y, Vec4{1, 1, 1, 1});
                    } else {
                        renderer.draw_rounded_rect(box, 2.f, t->panel);
                        renderer.draw_rect_outline(box, 1.f, t->border);
                        char b[8];
                        std::snprintf(b, sizeof(b), "%d", p + 1);
                        const float tw = font_->measure_text(b);
                        font_->draw_text(renderer, b, box.x + (box.w - tw) * 0.5f,
                                         text_y, t->text_secondary);
                    }
                }
                break;
            default:
                break;  // Limit/Refresh/Skip 由子控件绘制
        }
    }
}

}  // namespace ui
