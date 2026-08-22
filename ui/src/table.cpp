#include "ui/ui/table.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/label.h"
#include "ui/ui/laypage.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/switch.h"
#include "ui/ui/textbox.h"
#include "ui/ui/theme.h"

#include <algorithm>
#include <chrono>

namespace ui {

namespace {
// 下一个 UTF-8 字符起始下标（跳过后续续字节）
size_t next_cp(const std::string& s, size_t i) {
    if (i >= s.size()) return s.size();
    size_t j = i + 1;
    while (j < s.size() && ((unsigned char)s[j] & 0xC0) == 0x80) ++j;
    return j;
}

// layui-table 颜色（layui.css 提取）
constexpr Vec4 kThBg{0.980f, 0.980f, 0.980f, 1.0f};  // #FAFAFA 表头/奇数行灰底
constexpr Vec4 kLine{0.933f, 0.933f, 0.933f, 1.0f};  // #eee 分隔线/外框
constexpr Vec4 kEvenBg{0.980f, 0.980f, 0.980f, 1.0f};  // #FAFAFA 偶数行
constexpr Vec4 kAccent{0.09f, 0.73f, 0.67f, 1.0f};   // #16baaa 全选/复选选中
// UI-124：行 hover / 单击选中高亮。此前三分支同画 #FAFAFA，hover 与选中零视觉反馈；
// hover 用浅灰加深一档，选中用主题色半透明叠加（layui 行悬停 #f8f8f8 + 选中态惯例）
constexpr Vec4 kHoverBg{0.937f, 0.937f, 0.937f, 1.0f};   // ~#EFEFEF
constexpr Vec4 kSelBg{0.09f, 0.73f, 0.67f, 0.18f};       // #16baaa α0.18 叠加白底
constexpr Vec4 kCheckBorder{0.42f, 0.42f, 0.42f, 1.0f};  // 未选中复选框边框
constexpr float kBox = 18.f;    // 复选/单选框边长
constexpr float kBoxR = 4.f;    // 复选方框圆角
}  // namespace

// 单个数据行：按列宽排布文本/复选/单选/表单控件，hover/选中行高亮 #f8f8f8
class TableRow : public Widget {
public:
    TableRow(Table* owner, int index) : owner_(owner), index_(index) {}

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }

    float preferred_height() const override { return owner_->row_h(); }

    void on_draw(Renderer& renderer) override {
        static const Theme kDefaultTheme;
        const Theme* t = theme_ ? theme_ : &kDefaultTheme;
        (void)t;
        // 所有行都绘制白底，防止父控件深色背景透出
        renderer.draw_quad(bounds_, kEvenBg);
        if (owner_->selected_row() == index_) {
            renderer.draw_quad(bounds_, kSelBg);
        } else if (hovered()) {
            renderer.draw_quad(bounds_, kHoverBg);
        }
        // 行分隔线：Default（网格）/ Line（仅行线）画行底；Row/Nob 不画行线
        if (owner_->has_row_line()) {
            renderer.draw_quad({bounds_.x, bounds_.y + bounds_.h - 1.f, bounds_.w, 1.f}, kLine);
        }
        // 列分隔线：Default（网格）/ Row（仅列线）画每列右竖线
        const int ncol = owner_->col_count();
        if (owner_->has_col_line()) {
            float lx = bounds_.x;
            for (int c = 0; c < ncol; ++c) {
                lx += owner_->col_width(c);
                renderer.draw_quad({lx - 0.5f, bounds_.y, 1.f, bounds_.h}, kLine);
            }
        }

        const std::vector<std::string>& cells = owner_->row(index_);
        const float cy = bounds_.y + bounds_.h * 0.5f;
        float x = bounds_.x;
        for (int c = 0; c < ncol; ++c) {
            const float w = owner_->col_width(c);
            const std::string& text = (int)cells.size() > c ? cells[c] : "";
            const bool is_sel = (c == owner_->selection_col());
            if (is_sel && !owner_->is_radio_column()) {
                draw_checkbox(renderer, x, cy, w, owner_->row_checked(index_));
            } else if (is_sel && owner_->is_radio_column()) {
                draw_radio(renderer, x, cy, w, owner_->checked_radio() == index_);
            } else if (owner_->is_button(c)) {
                draw_buttons(renderer, x, cy, w);
            } else if (owner_->is_cell_form(c)) {
                draw_cell_form(renderer, x, cy, w, c);
            } else if (owner_->is_numbers(c)) {
                if (font_)
                    font_->draw_text(renderer, std::to_string(index_ + 1).c_str(),
                                     x + owner_->cell_pad_x(), cy - font_->line_height() * 0.5f,
                                     Vec4{0.2f, 0.2f, 0.2f, 1.0f});
            } else if (!text.empty() && font_) {
                const char* drawText = text.c_str();
                std::string truncated;
                // 树形首列：层级缩进 + 展开/折叠三角（layui treeTable 风格）
                float tx = x + owner_->cell_pad_x();
                if (c == 0 && owner_->is_tree()) {
                    const int data = owner_->data_row_of(index_);
                    tx += owner_->row_depth(data) * owner_->tree_indent();
                    if (owner_->row_has_children(data)) {
                        const bool exp = owner_->row_expanded(data);
                        // 三角：展开 ▼ / 折叠 ▶（#c2c2c2 灰，hover 行加深）
                        const Vec4 tri = hovered() ? Vec4{0.2f, 0.2f, 0.2f, 1.f}
                                                   : Vec4{0.76f, 0.76f, 0.76f, 1.f};
                        const float cx = tx + 5.f;
                        Vec2 pts[3];
                        if (exp) {
                            pts[0] = {cx - 4, cy - 2};
                            pts[1] = {cx + 4, cy - 2};
                            pts[2] = {cx, cy + 3};
                        } else {
                            pts[0] = {cx - 2, cy - 4};
                            pts[1] = {cx - 2, cy + 4};
                            pts[2] = {cx + 3, cy};
                        }
                        renderer.draw_polygon(pts, 3, tri);
                    }
                    tx += 16.f;  // 叶子行同样占位，文字与父级对齐
                }
                if (owner_->is_measure_text_col(c)) {
                    const float maxW = w - owner_->cell_pad_x() * 2.f;
                    const float fullW = font_->measure_text(text.c_str());
                    const float dotsW = font_->measure_text("...");
                    if (fullW > maxW && maxW > dotsW) {
                        float width = 0.f;
                        size_t n = 0;
                        const char* p = text.c_str();
                        while (*p) {
                            const char* prev = p;
                            // inline UTF-8 decode
                            unsigned char b = static_cast<unsigned char>(*p);
                            unsigned cp = b;
                            if (b >= 0x80) {
                                int seq = (b < 0xE0) ? 2 : (b < 0xF0) ? 3 : 4;
                                cp = b & (0x7F >> seq);
                                for (int k = 1; k < seq; ++k) {
                                    unsigned char nb = static_cast<unsigned char>(p[k]);
                                    if ((nb & 0xC0) != 0x80) { cp = 0; break; }
                                    cp = (cp << 6) | (nb & 0x3F);
                                }
                                p += (cp != 0) ? seq : 1;
                            } else {
                                ++p;
                            }
                            if (cp == 0) break;
                            float cw = font_->char_width(cp);
                            if (width + cw + dotsW > maxW) break;
                            width += cw;
                            n = prev - text.c_str();
                        }
                        if (n > 0) {
                            truncated = text.substr(0, n) + "...";
                            drawText = truncated.c_str();
                        } else {
                            truncated = "...";
                            drawText = truncated.c_str();
                        }
                    }
                }
                font_->draw_text(renderer, drawText, tx,
                                 cy - font_->line_height() * 0.5f, Vec4{0.2f, 0.2f, 0.2f, 1.0f});
            }
            x += w;
        }
    }

private:
    Table* owner_;
    int index_;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;

    void draw_buttons(Renderer& r, float x, float cy, float w) {
        const int n = owner_->button_count();
        if (n == 0 || !font_) return;
        // 行内小按钮：layui-btn layui-btn-xs 风格，蓝底白字 or 青正常/红危险/白描边
        const float gap = 4.f;
        const float btn_h = 22.f;
        const float by = cy - btn_h * 0.5f;
        // 计算总宽（文本宽 + 内边距）
        float total = 0.f;
        for (int i = 0; i < n; ++i) {
            total += font_->measure_text(owner_->button_label(i).c_str()) + 20.f;
            if (i) total += gap;
        }
        float bx = x + (w - total) * 0.5f;
        if (total < w) bx = x + (w - total) * 0.5f; else bx = x + 2.f;
        // 颜色表：0=正常(index=0 查看)，1=正常(编辑)，2=危险(删除)
        const Vec4 btns[3] = {
            {0.09f, 0.73f, 0.67f, 1.0f},   // 查看 #16baaa
            {0.12f, 0.62f, 1.0f, 1.0f},    // 编辑 #1e9fff
            {1.0f, 0.34f, 0.13f, 1.0f},    // 删除 #ff5722
        };
        for (int i = 0; i < n; ++i) {
            const float bw = font_->measure_text(owner_->button_label(i).c_str()) + 20.f;
            const Vec4 col = btns[std::min(i, 2)];
            r.draw_quad({bx, by, bw, btn_h}, col);
            font_->draw_text(r, owner_->button_label(i).c_str(),
                             bx + (bw - font_->measure_text(owner_->button_label(i).c_str())) * 0.5f,
                             cy - font_->line_height() * 0.5f, {1, 1, 1, 1});
            bx += bw + gap;
        }
    }

    void draw_checkbox(Renderer& r, float x, float cy, float w, bool checked) {
        const float bx = x + (w - kBox) * 0.5f;
        const float by = cy - kBox * 0.5f;
        const Rect box{bx, by, kBox, kBox};
        if (checked) {
            r.draw_rounded_rect(box, kBoxR, kAccent);
            // 对勾：白色对勾图标（layui-icon-right）
            if (font_) font_->draw_icon(r, layui_icon::right, bx + kBox * 0.5f, cy, kBox * 0.7f, {1, 1, 1, 1});
        } else {
            r.draw_rounded_rect_outline(box, kBoxR, 1.f, kCheckBorder);
        }
    }

    void draw_radio(Renderer& r, float x, float cy, float w, bool checked) {
        const float d = kBox;
        const float bx = x + (w - d) * 0.5f;
        const float by = cy - d * 0.5f;
        const Vec2 c{bx + d * 0.5f, by + d * 0.5f};
        // 外圈
        r.draw_rounded_rect({bx, by, d, d}, d * 0.5f, Vec4{1, 1, 1, 1});
        r.draw_rounded_rect_outline({bx, by, d, d}, d * 0.5f, 1.f, kCheckBorder);
        // 内点（选中时）
        if (checked) {
            const float pd = d * 0.5f;
            r.draw_rounded_rect({c.x - pd * 0.5f, c.y - pd * 0.5f, pd, pd}, pd * 0.5f, kAccent);
        }
    }

    void draw_cell_form(Renderer& r, float x, float cy, float w, int col) {
        const bool is_switch = owner_->is_cell_switch(col);
        const std::vector<std::string>& cells = owner_->row(index_);
        const bool on = cells.size() > (size_t)col &&
                        (cells[col] == "1" || cells[col] == "true");
        const float h = is_switch ? 24.f : kBox;
        const float bw = is_switch ? 42.f : kBox;
        const float bx = x + (w - bw) * 0.5f;
        const float by = cy - h * 0.5f;
        if (is_switch) {
            // UI-130：与项目统一圆角开关皮肤（ui::Switch 同源渲染），禁止自绘矩形开关
            static const Theme kCellThemeDefault;
            draw_switch_skin(r, Rect{bx, by, bw, h}, theme_ ? theme_ : &kCellThemeDefault, on,
                             true);
        } else {
            if (on) {
                r.draw_rounded_rect({bx, by, bw, h}, 4.f, kAccent);
            } else {
                r.draw_rounded_rect_outline({bx, by, bw, h}, 4.f, 1.f, kCheckBorder);
            }
        }
    }
};

Table::Table(float x, float y, float w, float h) : Widget(x, y, w, h) {
    add_class("layui-table");
    body_ = new ScrollBox(0, 0, w, h);
    body_->set_padding(0.f);
    body_->set_spacing(0.f);
    add_child(body_);
}

void Table::add_column(const char* title, float width, bool sortable) {
    cols_.push_back({title ? title : "", width, sortable, true, ColKind::Data});
}

void Table::add_row(const std::vector<std::string>& cells) {
    rows_.push_back(cells);
    row_checked_.push_back(false);
    row_depth_.push_back(0);
    row_expanded_.push_back(true);
    rebuild_rows();
}

void Table::add_tree_row(int depth, const std::vector<std::string>& cells) {
    rows_.push_back(cells);
    row_checked_.push_back(false);
    row_depth_.push_back(std::max(0, depth));
    row_expanded_.push_back(true);
    rebuild_rows();
}

bool Table::is_tree() const {
    for (int d : row_depth_) if (d > 0) return true;
    return false;
}

int Table::row_depth(int data_row) const {
    return (data_row >= 0 && data_row < (int)row_depth_.size()) ? row_depth_[data_row] : 0;
}

bool Table::row_has_children(int data_row) const {
    const int n = (int)rows_.size();
    if (data_row < 0 || data_row >= n - 1) return false;
    return row_depth(data_row + 1) > row_depth(data_row);
}

bool Table::row_expanded(int data_row) const {
    return (data_row >= 0 && data_row < (int)row_expanded_.size()) ? row_expanded_[data_row] : true;
}

void Table::set_row_expanded(int data_row, bool on) {
    if (data_row < 0 || data_row >= (int)row_expanded_.size()) return;
    row_expanded_[data_row] = on;
    rebuild_rows();
    on_layout();
}

void Table::toggle_row(int visible_row) {
    const int data = data_row_of(visible_row);
    if (!row_has_children(data)) return;
    row_expanded_[data] = !row_expanded_[data];
    rebuild_rows();
    on_layout();
    if (on_expand_) on_expand_(data, row_expanded_[data]);
}

void Table::clear_rows() {
    rows_.clear();
    row_checked_.clear();
    row_depth_.clear();
    row_expanded_.clear();
    tree_visible_.clear();
    radio_sel_ = -1;
    sel_row_ = -1;  // UI-124
    rebuild_rows();
}

void Table::set_font(Font* font) {
    font_ = font;
    for (Widget* child : body_->children()) {
        if (auto* row = dynamic_cast<TableRow*>(child)) row->set_font(font);
    }
    if (page_bar_) page_bar_->set_font(font);
}

void Table::set_theme(const Theme* theme) {
    theme_ = theme;
    body_->set_theme(theme);
    for (Widget* child : body_->children()) {
        if (auto* row = dynamic_cast<TableRow*>(child)) row->set_theme(theme);
    }
    if (page_bar_) page_bar_->set_theme(theme);
}

void Table::set_paginated(bool on, int page_size) {
    if (on && !page_bar_) {
        page_bar_ = new Laypage(0, 0, bounds_.w, 28.f);
        page_bar_->set_font(font_);
        page_bar_->set_theme(theme_);
        page_bar_->set_on_change([this](int p) { set_page(p); });
        add_child(page_bar_);
    }
    paginated_ = on;
    page_size_ = std::max(1, page_size);
    // UI-123：页码保持——窗口缩放等触发重设 page_size 时不再无条件跳回首页，
    // 仅 clamp 到新总页数范围（数据重载 reload() 仍显式归零，属 layui reload 语义）
    page_ = std::min(std::max(0, page_), std::max(0, page_count() - 1));
    if (page_bar_) {
        page_bar_->visible = on;
        page_bar_->set_total(page_count());
        page_bar_->set_page(page_);
    }
    // UI-126：流加载钩子仅非分页模式持有——防御「先挂回调后开分页」顺序，
    // 切入分页即摘除 body_ 触底回调，避免翻页滚动到底误触续灌
    if (body_) body_->set_on_scroll_to_bottom((on || !on_load_more_) ? nullptr : on_load_more_);
    rebuild_rows();
    on_layout();
}

// —— 流加载（UI-126）——

void Table::set_on_load_more(std::function<void()> cb) {
    on_load_more_ = std::move(cb);
    // 仅非分页模式生效：分页数据切片由 Laypage 页条控制，无流加载语义
    if (body_ && !paginated_) body_->set_on_scroll_to_bottom(on_load_more_);
}

void Table::set_end_note(const char* text) {
    std::string t = text ? text : "";
    if (t == end_note_) return;  // 文本未变化不重建（续灌路径每次 reload 后调用）
    end_note_ = std::move(t);
    if (body_) rebuild_rows();
}

void Table::reset_scroll() {
    if (body_) body_->reset_scroll();
}

// UI-127：预热表头/可见行/尾注全部文本字形，消除流加载续灌与表格首次显示
// 瞬间的 UI-125 配额限流渐进补齐闪烁。prebake_text 内部跳过 ASCII（已在图集）、
// 图标与已烘焙码点，重复调用仅余字符串遍历开销。
void Table::prebake_visible_text() {
    if (!font_) return;
    for (const auto& c : cols_) font_->prebake_text(c.title.c_str());
    for (int r : visible_rows_) {
        if (r < 0 || r >= static_cast<int>(rows_.size())) continue;
        for (const auto& cell : rows_[r]) font_->prebake_text(cell.c_str());
    }
    if (!end_note_.empty()) font_->prebake_text(end_note_.c_str());
}

int Table::auto_page_size() const {
    // UI-123：可用高度须扣除表头与内部分页条（28px）——本接口的约定用途是
    // 为 set_paginated(true, n) 提供条数，分页条必然占位，恒定预留
    if (bounds_.h <= 0.f) return 1;
    const float avail = bounds_.h - header_h() - 28.f;
    if (avail <= 0.f) return 1;
    return std::max(1, static_cast<int>(avail / row_h()));
}

void Table::set_page(int page) {
    const int pc = page_count();
    page_ = std::min(std::max(0, page), std::max(0, pc - 1));
    sel_row_ = -1;  // UI-124：翻页后原可见行号失效
    if (page_bar_) {
        page_bar_->set_total(pc);
        page_bar_->set_page(page_);
    }
    rebuild_rows();
    on_layout();
}

int Table::page_count() const {
    if (!paginated_) return 1;
    // 树形模式按树形过滤后的行数分页（折叠子树不计入）
    const size_t n = is_tree() ? tree_visible_.size() : rows_.size();
    if (n == 0) return 1;
    return (int)((n + page_size_ - 1) / page_size_);
}

int Table::visible_row_count() const { return (int)visible_rows_.size(); }

int Table::data_row_of(int visible) const {
    if (visible < 0 || visible >= (int)visible_rows_.size()) return -1;
    return visible_rows_[visible];
}

void Table::add_checkbox_column() {
    // 前置插入复选列
    cols_.insert(cols_.begin(), {"", 46.f, false, true, ColKind::Checkbox});
    selection_col_ = 0;
    radio_mode_ = false;
    // 已有数据行补齐复选位
    row_checked_.resize(rows_.size(), false);
    rebuild_rows();
}

void Table::add_radio_column() {
    cols_.insert(cols_.begin(), {"", 46.f, false, true, ColKind::Radio});
    selection_col_ = 0;
    radio_mode_ = true;
    row_checked_.resize(rows_.size(), false);
    rebuild_rows();
}

void Table::add_numbers_column() {
    cols_.insert(cols_.begin(), {"", 50.f, false, true, ColKind::Numbers});
    rebuild_rows();
}

bool Table::row_checked(int visible_row) const {
    const int data = data_row_of(visible_row);
    if (data < 0 || data >= (int)row_checked_.size()) return false;
    return row_checked_[data];
}

int Table::checked_radio() const {
    if (!radio_mode_) return -1;
    // 返回可见行号
    const int data = radio_sel_;
    for (int v = 0; v < (int)visible_rows_.size(); ++v)
        if (visible_rows_[v] == data) return v;
    return -1;
}

int Table::checked_count() const {
    int n = 0;
    for (int v = 0; v < (int)visible_rows_.size(); ++v)
        if (row_checked(v)) ++n;
    return n;
}

int Table::first_checked() const {
    for (int v = 0; v < (int)visible_rows_.size(); ++v)
        if (row_checked(v)) return v;
    return -1;
}

void Table::set_cell_switch(int col) {
    if (col >= 0 && col < (int)cols_.size()) cols_[col].kind = ColKind::CellSwitch;
}

void Table::set_cell_checkbox(int col) {
    if (col >= 0 && col < (int)cols_.size()) cols_[col].kind = ColKind::CellCheckbox;
}

void Table::add_button_column(const char* title, float width,
                              const std::vector<std::string>& labels,
                              const std::vector<std::string>& events) {
    cols_.push_back({title ? title : "", width, false, true, ColKind::Button});
    button_col_ = (int)cols_.size() - 1;
    btn_labels_ = labels;
    btn_events_ = (events.size() == labels.size()) ? events : std::vector<std::string>(labels.size());
    rebuild_rows();
}

bool Table::is_cell_form(int col) const {
    if (col < 0 || col >= (int)cols_.size()) return false;
    return cols_[col].kind == ColKind::CellSwitch || cols_[col].kind == ColKind::CellCheckbox;
}

bool Table::is_cell_switch(int col) const {
    return col >= 0 && col < (int)cols_.size() && cols_[col].kind == ColKind::CellSwitch;
}

bool Table::is_numbers(int col) const {
    return col >= 0 && col < (int)cols_.size() && cols_[col].kind == ColKind::Numbers;
}

int Table::col_count() const { return (int)cols_.size(); }

float Table::col_width(int c) const {
    if (c < 0 || c >= (int)cols_.size()) return 0.f;
    if (c < (int)eff_widths_.size()) return eff_widths_[c];
    return cols_[c].width;
}

const std::vector<std::string>& Table::row(int r) const {
    static const std::vector<std::string> kEmpty;
    const int data = data_row_of(r);
    if (data < 0 || data >= (int)rows_.size()) return kEmpty;
    return rows_[data];
}

int Table::selected_row() const {
    if (radio_mode_) {
        return checked_radio();
    }
    return sel_row_;  // UI-124：单击选中行（可见行号）
}

float Table::row_h() const {
    switch (size_) {
        case TableSize::Lg: return 50.f;
        case TableSize::Sm: return 30.f;
        default: return 38.f;
    }
}

float Table::cell_pad_x() const {
    switch (size_) {
        case TableSize::Lg: return 30.f;
        case TableSize::Sm: return 10.f;
        default: return 15.f;
    }
}

void Table::on_layout() {
    const Rect& b = bounds_;
    // 列宽自适应：容器宽 > 各列宽之和 → 剩余空间按比例分配给所有列
    // 每列至少保持 cell_min_width_（如设置了）或其原始 width
    const int nc = (int)cols_.size();
    eff_widths_.resize(nc);
    float sum = 0.f;
    for (int i = 0; i < nc; ++i) {
        float w = cols_[i].width;
        if (cell_min_width_ > 0.f && w < cell_min_width_) w = cell_min_width_;
        eff_widths_[i] = w;
        sum += w;
    }
    if (nc > 0 && b.w > sum) {
        const float extra = b.w - sum;
        for (int i = 0; i < nc; ++i) {
            eff_widths_[i] += extra * (eff_widths_[i] / sum);
        }
    }

    int vis = (int)visible_rows_.size();
    if (visible_rows_.empty()) vis = (int)rows_.size();
    const float pb_h = (paginated_ && page_bar_) ? 28.f : 0.f;
    const float body_h = (b.h <= 0.f) ? (float)vis * row_h()
                                       : std::max(0.f, b.h - header_h() - pb_h);
    body_->set_bounds(b.x, b.y + header_h(), b.w, body_h);
    if (page_bar_) {
        page_bar_->set_bounds(b.x, b.y + header_h() + body_h, b.w, 28.f);
    }
}

float Table::preferred_height() const {
    // UI-131 高度语义修正：旧实现首行「bounds_.h > 0 即返回」使首次分配值永久冻结——
    // 流式容器（Card/ScrollBox/VBox…）在空表期分到表头高后，add_row 增长永不反映，
    // 行区 body_h≈0 全部裁剪（audio_text 声纹编辑面板样本列表被按钮遮挡 SPK-005 根因）。
    // 改为默认内容跟随（每次调用按当前可见行数重算）；固定高场景显式 set_fixed_height，
    // 流式容器防溢出用 set_max_content_height 钳制。
    if (fixed_h_ > 0.f) return fixed_h_;
    const int vis = (int)visible_rows_.size();
    float h = header_h() + (float)vis * row_h() + (paginated_ ? 28.f : 0.f);
    if (max_content_h_ > 0.f && h > max_content_h_) h = max_content_h_;
    return h;
}

void Table::rebuild_rows() {
    // 树形可见性：先序遍历，祖先任一折叠则整棵子树隐藏
    tree_visible_.clear();
    {
        std::vector<int> path;  // path[d] = 当前深度 d 的祖先数据行下标
        for (int i = 0; i < (int)rows_.size(); ++i) {
            const int d = row_depth(i);
            if ((int)path.size() > d) path.resize(d);
            bool vis = true;
            for (int anc : path) {
                if (!row_expanded_[anc]) { vis = false; break; }
            }
            if (vis) tree_visible_.push_back(i);
            path.push_back(i);
        }
    }
    // 计算当前页可见行映射（分页在树形过滤结果上切片）
    visible_rows_.clear();
    std::vector<int> base;
    if (is_tree()) {
        base = tree_visible_;
    } else {
        base.resize(rows_.size());
        for (int i = 0; i < (int)rows_.size(); ++i) base[i] = i;
    }
    if (paginated_) {
        const int start = page_ * page_size_;
        for (int i = start; i < (int)base.size() && i < start + page_size_; ++i)
            visible_rows_.push_back(base[i]);
    } else {
        visible_rows_ = base;
    }
    // 清掉旧行控件（UI-040：递归释放整批旧行，此前仅移除不 delete 造成排序/编辑后 TableRow 泄漏）
    body_->clear_children();
    for (int i = 0; i < (int)visible_rows_.size(); ++i) {
        TableRow* row = new TableRow(this, i);
        row->set_font(font_);
        row->set_theme(theme_);
        body_->add_child(row);
    }
    // UI-126 流加载尾注：只读 Label 排在行后随内容滚动；ScrollBox 流式布局按
    // preferred_height 排布、宽度强制 content_w。非数据行，不参与行号映射/选中/编辑
    if (!paginated_ && !end_note_.empty()) {
        auto* note = new Label(0, 0, bounds_.w, row_h(), end_note_.c_str());
        note->set_font(font_);
        note->set_align(1);  // 居中
        note->set_color(Vec4{0.45f, 0.45f, 0.45f, 1.0f});
        body_->add_child(note);
    }
    // UI-126 触底重检：ScrollBox 触发仅在滚动事件时机（set_scroll），小批次不足以
    // 把内容底推出触发区且用户已在底部时将停摆；此处补齐 layui flow.load「DOM 插入
    // 后再查一次」语义。同步回调可再入 add_row→rebuild_rows→rearm 递归，深度以填满
    // 一屏所需批次数为界（有限收敛）；数据耗尽方停止 append 即断链。
    // 注意：必须在尾注追加之后调用（先定稿内容再判触底）。
    if (!paginated_ && on_load_more_ && body_) body_->rearm_bottom();
}

void Table::truncate_rows() {
    if (truncate_cols_.empty()) return;
    // 纯字符估算截断，不调用 measure_text；measure_text_cols 由 on_draw 渲染时精确处理
    for (auto& row : rows_) {
        for (int c : truncate_cols_) {
            if (c < 0 || c >= (int)row.size()) continue;
            if (is_measure_text_col(c)) continue;
            float maxW = col_width(c) - cell_pad_x() * 2.f;
            if (maxW <= 0.f) continue;
            const std::string& text = row[c];
            if (text.empty()) continue;
            // 粗略估算：每 ASCII 字符约 8px，每 CJK 字符约 16px
            int budget = static_cast<int>(maxW);
            int px = 0;
            size_t n = 0;
            while (n < text.size() && px < budget) {
                unsigned char c2 = static_cast<unsigned char>(text[n]);
                if (c2 < 0x80) { px += 8; n += 1; }
                else if (c2 < 0xE0) { px += 16; n += 2; }
                else if (c2 < 0xF0) { px += 16; n += 3; }
                else { px += 16; n += 4; }
            }
            if (n < text.size()) row[c] = text.substr(0, n) + "...";
        }
    }
}

void Table::sort_rows(int col) {
    if (is_tree()) return;  // 树形模式禁排序：扁平重排会破坏层级先序
    if (col < 0 || col >= (int)cols_.size()) return;
    Column& c = cols_[col];
    c.ascending = !c.ascending;
    std::stable_sort(rows_.begin(), rows_.end(), [col, &c](const std::vector<std::string>& a,
                                                           const std::vector<std::string>& b) {
        const std::string& sa = col < (int)a.size() ? a[col] : "";
        const std::string& sb = col < (int)b.size() ? b[col] : "";
        const int cmp = sa.compare(sb);
        return c.ascending ? cmp < 0 : cmp > 0;
    });
    sort_col_ = col;
    sel_row_ = -1;  // UI-124：排序重排后可见行号失效
    rebuild_rows();
    if (on_sort_) on_sort_(col, c.ascending);
}

void Table::set_init_sort(int col, bool asc) {
    if (is_tree()) return;  // 树形模式禁排序
    if (col < 0 || col >= (int)cols_.size()) return;
    cols_[col].ascending = asc;
    std::stable_sort(rows_.begin(), rows_.end(), [col, asc](const std::vector<std::string>& a,
                                                            const std::vector<std::string>& b) {
        const std::string& sa = col < (int)a.size() ? a[col] : "";
        const std::string& sb = col < (int)b.size() ? b[col] : "";
        const int cmp = sa.compare(sb);
        return asc ? cmp < 0 : cmp > 0;
    });
    sort_col_ = col;
    rebuild_rows();
}

void Table::reload(const std::vector<std::vector<std::string>>& new_rows) {
    rows_ = new_rows;
    row_checked_.resize(rows_.size(), false);
    row_depth_.assign(rows_.size(), 0);
    row_expanded_.assign(rows_.size(), true);
    radio_sel_ = -1;
    sel_row_ = -1;  // UI-124：reload 后选中态失效
    page_ = 0;
    rebuild_rows();
    on_layout();
    truncate_rows();  // 仅在数据加载时截断一次，不在每帧 on_layout 中重复
}

void Table::begin_edit(int row, int col, float click_x) {
    if (row < 0 || row >= (int)visible_rows_.size() || col < 0 || col >= (int)cols_.size()) return;
    commit_edit();

    // 定位单元格：表头高之下 + 列偏移（可视行内的位置）
    float cx = bounds_.x;
    for (int i = 0; i < col; ++i) cx += col_width(i);
    const float cy = bounds_.y + header_h() + row * row_h() - body_->scroll_offset();
    const float cw = col_width(col);

    const int data = data_row_of(row);
    edit_row_ = row;
    edit_col_ = col;
    edit_data_row_ = data;
    edit_box_ = new TextBox(cx, cy, cw, row_h());
    edit_box_->set_font(font_);
    edit_box_->set_theme(theme_);
    if (data >= 0 && data < (int)rows_.size() && col < (int)rows_[data].size())
        edit_box_->set_text(rows_[data][col].c_str());
    // 初始光标定位：双击时根据鼠标 x 坐标落在文本的哪个字符之后
    if (click_x >= 0.f && font_ && !edit_box_->text().empty()) {
        const float pad = 8.f;
        const float local_x = click_x - cx - pad;
        if (local_x <= 0.f) {
            edit_box_->set_caret_pos(0);
        } else {
            const std::string& txt = edit_box_->text();
            size_t best = txt.size();
            float best_dist = local_x;
            size_t i = 0;
            while (i < txt.size()) {
                const size_t next = next_cp(txt, i);
                if (next == i) break;
                const float w = font_->measure_text(txt.substr(0, next).c_str());
                const float dist = std::abs(local_x - w);
                if (dist < best_dist) { best_dist = dist; best = next; }
                i = next;
            }
            edit_box_->set_caret_pos(best);
        }
    }
    edit_box_->set_on_enter([this]() { commit_edit(); });
    add_child(edit_box_);
    edit_box_->focus();
}

void Table::commit_edit() {
    if (!edit_box_) return;
    if (edit_data_row_ >= 0 && edit_data_row_ < (int)rows_.size() && edit_col_ >= 0 &&
        edit_col_ < (int)rows_[edit_data_row_].size()) {
        rows_[edit_data_row_][edit_col_] = edit_box_->text();
    }
    remove_child(edit_box_);
    delete edit_box_;
    edit_box_ = nullptr;
    edit_row_ = -1;
    edit_col_ = -1;
    edit_data_row_ = -1;
    rebuild_rows();
}

int Table::col_hit(float x) const {
    float cx = bounds_.x;
    for (int i = 0; i < (int)cols_.size(); ++i) {
        const float w = col_width(i);
        if (x >= cx && x < cx + w) return i;
        cx += w;
    }
    return -1;
}

float Table::col_x(int c) const {
    float cx = bounds_.x;
    for (int i = 0; i < c && i < (int)cols_.size(); ++i) cx += col_width(i);
    return cx;
}

bool Table::on_mouse_down(float x, float y, int button) {
    if (button != 0) return false;
    if (!contains(x, y)) return false;

    // 如果正在编辑单元格，单击其他位置先提交编辑
    if (edit_box_) {
        commit_edit();
    }

    // 表头：点击可排序列切换排序；点复选头全选当前页
    if (y >= bounds_.y && y < bounds_.y + header_h()) {
        const int hcol = col_hit(x);
        if (hcol >= 0) {
            if (selection_col_ >= 0 && hcol == selection_col_ && !radio_mode_) {
                // 全选/取消全选当前页
                const bool all = (checked_count() == (int)visible_rows_.size());
                for (int v = 0; v < (int)visible_rows_.size(); ++v) {
                    const int data = data_row_of(v);
                    row_checked_[data] = !all;
                    if (on_selection_) on_selection_(v, !all);
                }
                return true;
            }
            if (cols_[hcol].sortable) sort_rows(hcol);
            return true;
        }
        return true;
    }

    // 分页条区域：Laypage 子控件在 dispatch 阶段先于本节点获得事件并消费；
    // 这里兜底消费自身 chrome（UI-122），此前 return false 放行导致点击穿透到
    // 表格下层控件（规则管理页翻页穿透缺陷根源之一）。仅在分页开启时生效。
    if (paginated_ && page_bar_ && page_bar_->visible) {
        const Rect& pb = page_bar_->bounds();
        if (pb.w > 0.f && x >= pb.x && x < pb.x + pb.w && y >= pb.y && y < pb.y + pb.h)
            return true;
    }

    // 数据区：点击行触发回调；点击复选/单选/表单格切换；双击文本格编辑
    const int row = (int)((y - bounds_.y - header_h() + body_->scroll_offset()) / row_h());
    if (row >= 0 && row < (int)visible_rows_.size()) {
        const int data = data_row_of(row);
        const int col = col_hit(x);
        // 树形展开图标命中（首列缩进三角区域）→ 切换展开/折叠
        if (col == 0 && is_tree() && row_has_children(data)) {
            const float ry = bounds_.y + header_h() + (float)row * row_h() - body_->scroll_offset();
            const float ix = col_x(0) + cell_pad_x() + (float)row_depth(data) * tree_indent();
            if (x >= ix && x < ix + 16.f && y >= ry && y < ry + row_h()) {
                toggle_row(row);
                return true;
            }
        }
        const bool in_sel_col = (selection_col_ >= 0 && col == selection_col_);
        if (in_sel_col) {
            if (radio_mode_) {
                // 单选切换
                if (radio_sel_ != data) {
                    radio_sel_ = data;
                    if (on_selection_) on_selection_(row, true);
                }
            } else {
                const bool cur = row_checked_[data];
                row_checked_[data] = !cur;
                if (on_selection_) on_selection_(row, !cur);
            }
            return true;
        }
        // 单元格表单切换
        if (col >= 0 && is_cell_form(col)) {
            // 表单格的原始文本为 "1"/"0"/"true"/"false"，点击翻转
            if (data >= 0) {
                bool on = rows_[data][col] == "1" || rows_[data][col] == "true";
                rows_[data][col] = !on ? "1" : "0";
                // UI-129：翻转后通知调用方（此前仅改文本，调用方无法感知切换）
                if (on_cell_form_) on_cell_form_(row, col, !on);
            }
            return true;
        }

        // 操作列（行内按钮）：命中某个按钮 → 触发 on_action
        if (col >= 0 && is_button(col)) {
            if (btn_labels_.size() && font_) {
                // 反算该行按钮实际中心 x，命中到具体按钮
                const float cy = bounds_.y + header_h() + row * row_h() - body_->scroll_offset() + row_h() * 0.5f;
                const float ry = bounds_.y + header_h() + row * row_h() - body_->scroll_offset();
                const float rh = row_h();
                const float w = col_width(col);
                const float gap = 4.f;
                const float btn_h = 22.f;
                const float by = cy - btn_h * 0.5f;
                float total = 0.f;
                for (int i = 0; i < (int)btn_labels_.size(); ++i) {
                    total += font_->measure_text(btn_labels_[i].c_str()) + 20.f;
                    if (i) total += gap;
                }
                float bx = col_x(col) + (w - total) * 0.5f;
                if (total >= w) bx = col_x(col) + 2.f;
                for (int i = 0; i < (int)btn_labels_.size(); ++i) {
                    const float bw = font_->measure_text(btn_labels_[i].c_str()) + 20.f;
                    const Rect r{bx, by, bw, btn_h};
                    if (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h) {
                        if (on_action_) on_action_(row, btn_events_[i]);
                        break;
                    }
                    bx += bw + gap;
                }
            }
            return true;
        }

        // 单元格事件列：单击该列 → on_cell_event
        if (col >= 0 && cell_event_col_ >= 0 && col == cell_event_col_) {
            if (on_cell_event_) on_cell_event_(row, col);
            return true;
        }

        sel_row_ = row;  // UI-124：单击选中高亮（可见行号）
        if (on_row_click_) on_row_click_(row);
        const auto now = std::chrono::steady_clock::now();
        const double dt = std::chrono::duration<double>(now - last_click_).count();
        const bool dbl = (dt < 0.35 && last_click_row_ == row);
        last_click_ = now;
        last_click_row_ = row;
        if (dbl) {
            if (on_row_dbl_click_) {
                on_row_dbl_click_(row);
            } else {
                begin_edit(row, col < 0 ? 0 : col, x);
            }
        }
        return true;
    }
    return true;
}

void Table::on_draw(Renderer& renderer) {
    // 表头：#FAFAFA 灰底 + 列标题（可排序时带箭头）；分隔线按 skin 绘制
    const float hh = header_h();
    const Rect header{bounds_.x, bounds_.y, bounds_.w, hh};
    renderer.draw_quad(header, kThBg);
    // 表头列分隔线：Default（网格）/ Row（仅列线）画列右竖线
    if (has_col_line()) {
        float lx = bounds_.x;
        for (int i = 0; i < (int)cols_.size(); ++i) {
            lx += col_width(i);
            renderer.draw_quad({lx - 0.5f, bounds_.y, 1.f, hh}, kLine);
        }
    }
    // 表头底部行分隔线：Default / Line 画（Row/Nob 不画行线）
    if (has_row_line()) {
        renderer.draw_quad({bounds_.x, bounds_.y + hh - 1.f, bounds_.w, 1.f}, kLine);
    }

    // 表头单元格内容：复选/单选头 / 序号头 / 文本标题
    const float cy = bounds_.y + hh * 0.5f;
    float x = bounds_.x;
    for (int i = 0; i < (int)cols_.size(); ++i) {
        const Column& col = cols_[i];
        const float w = col_width(i);
        if (selection_col_ >= 0 && i == selection_col_) {
            if (!radio_mode_) {
                // 全选框：当前页全选态（选中态 #16baaa，未选中灰边框）
                const bool all = (checked_count() == (int)visible_rows_.size());
                const float bx = x + (w - kBox) * 0.5f;
                const float by = cy - kBox * 0.5f;
                const Rect box{bx, by, kBox, kBox};
                if (all) renderer.draw_rounded_rect(box, kBoxR, kAccent);
                else renderer.draw_rounded_rect_outline(box, kBoxR, 1.f, kCheckBorder);
            }
        } else if (col.kind == ColKind::Numbers) {
            if (font_)
                font_->draw_text(renderer, col.title.c_str(), x + cell_pad_x(),
                                 cy - font_->line_height() * 0.5f, Vec4{0.2f, 0.2f, 0.2f, 1.0f});
        } else if (col.kind == ColKind::CellSwitch || col.kind == ColKind::CellCheckbox) {
            // UI-131：表单列（开关/复选）表头标题居中，与单元格控件对齐
            if (font_ && !col.title.empty()) {
                const float tw = font_->measure_text(col.title.c_str());
                font_->draw_text(renderer, col.title.c_str(), x + (w - tw) * 0.5f,
                                 cy - font_->line_height() * 0.5f, Vec4{0.2f, 0.2f, 0.2f, 1.0f});
            }
        } else if (font_) {
            font_->draw_text(renderer, col.title.c_str(), x + cell_pad_x(),
                             cy - font_->line_height() * 0.5f, Vec4{0.2f, 0.2f, 0.2f, 1.0f});
            if (col.sortable && sort_col_ == i) {
                // 排序箭头：升序向上三角 / 降序向下三角（激活 #16baaa）
                const float ax = x + w - 18.f;
                const float ay = bounds_.y + hh * 0.5f;
                const Vec2 up[3] = {{ax - 4, ay + 3}, {ax + 4, ay + 3}, {ax, ay - 3}};
                const Vec2 dn[3] = {{ax - 4, ay - 3}, {ax + 4, ay - 3}, {ax, ay + 3}};
                const Vec4 arrow{0.09f, 0.73f, 0.67f, 1.0f};  // #16baaa
                renderer.draw_polygon(col.ascending ? up : dn, 3, arrow);
            }
        }
        x += w;
    }

    // 合计行（layui totalRow）：固定于数据区底部的 footer 行，首列显示合计文案，合计列显示当前页求和
    if (total_row_) {
        const float pb_h = (paginated_ && page_bar_) ? 28.f : 0.f;
        const float body_h = (bounds_.h <= 0.f) ? (float)visible_rows_.size() * row_h()
                                                : std::max(0.f, bounds_.h - header_h() - pb_h);
        const float ty = bounds_.y + header_h() + body_h - row_h();
        const Rect trow{bounds_.x, ty, bounds_.w, row_h()};
        renderer.draw_quad(trow, kThBg);
        if (has_row_line())
            renderer.draw_quad({bounds_.x, ty - 1.f, bounds_.w, 1.f}, kLine);
        // 合计 column 分隔线（跟随列宽）
        if (has_col_line()) {
            float lx = bounds_.x;
            for (int c = 0; c < (int)cols_.size(); ++c) {
                lx += col_width(c);
                renderer.draw_quad({lx - 0.5f, trow.y, 1.f, trow.h}, kLine);
            }
        }
        // 求和当前页 total_col 列数值
        double sum = 0;
        for (int v = 0; v < (int)visible_rows_.size(); ++v) {
            const int d = data_row_of(v);
            if (d >= 0 && total_col_ >= 0 && total_col_ < (int)rows_[d].size()) {
                try { sum += std::stod(rows_[d][total_col_]); } catch (...) {}
            }
        }
        const float tcy = ty + row_h() * 0.5f;
        const std::string sum_str = (total_col_ >= 0) ? std::to_string((long long)sum) : "";
        if (font_) {
            float cx = bounds_.x;
            for (int c = 0; c < (int)cols_.size(); ++c) {
                const float w = col_width(c);
                if (c == 0) {
                    font_->draw_text(renderer, total_row_text_.c_str(), cx + cell_pad_x(),
                                     tcy - font_->line_height() * 0.5f, Vec4{0.2f, 0.2f, 0.2f, 1.0f});
                } else if (c == total_col_ && !sum_str.empty()) {
                    font_->draw_text(renderer, sum_str.c_str(), cx + cell_pad_x(),
                                     tcy - font_->line_height() * 0.5f, Vec4{0.2f, 0.2f, 0.2f, 1.0f});
                }
                cx += w;
            }
        }
    }

    // 表格外框 1px #eee（Nob 皮肤也保留外框，镜像 lay-skin=nob 仅去内分隔线）
    renderer.draw_rect_outline(bounds_, 1.f, kLine);
}

}  // namespace ui