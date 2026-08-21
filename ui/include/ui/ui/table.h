#pragma once

#include "ui/ui/widget.h"

#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Laypage;
class Renderer;
class ScrollBox;
class TextBox;
struct Theme;

// 表格皮肤（layui-table lay-skin，镜像静态表格 table-element.html 四态）
enum class TableSkin {
    Default,  // 默认：单元格完整网格（行+列分隔线）+ 外框
    Line,     // lay-skin="line"：仅行分隔线（无列竖线）
    Row,      // lay-skin="row"：仅列分隔线（无行线）
    Nob,      // lay-skin="nob"：无内部分隔线（仅外框）
};

// 表格尺寸（layui-table lay-size）
enum class TableSize {
    Normal,  // 默认：行高 38
    Lg,      // lay-size="lg"：大尺寸（行高 50，左右内边距 30）
    Sm,      // lay-size="sm"：小尺寸（行高 30，字号 12，左右内边距 10）
};

// 数据表格（M17c，layui-table 克隆）：表头（可排序）+ 数据行（hover 高亮）+ 纵向滚动。
// 双击单元格进入行内编辑（TextBox 覆盖，Enter / 失焦提交写回）。
//
// 分阶段扩展（layui table 模块 21 子页 一比一克隆）：
//   - 分页：set_paginated() → 表底挂 Laypage，仅显示当前页可见行（visible_rows 映射）。
//   - 复选/单选列：add_checkbox_column()/add_radio_column()，表头全选 + 逐行勾选位 + 选中回调。
//   - 序号列：add_numbers_column()，每行前置递增序号。
//   - 单元格表单：set_cell_switch()/set_cell_checkbox() 令某列每格渲染 Switch/CheckBox 并回调。
class Table : public Widget {
public:
    Table(float x, float y, float w, float h);

    // 添加列：title 表头文本、width 列宽（像素）、sortable 是否可排序
    void add_column(const char* title, float width, bool sortable = false);
    void add_row(const std::vector<std::string>& cells);
    void clear_rows();

    void set_skin(TableSkin skin) { skin_ = skin; }
    void set_size(TableSize size) { size_ = size; }
    void set_even(bool even) { even_ = even; }
    void set_cell_min_width(float w) { cell_min_width_ = w; }
    // 指定哪些列启用自动截断：文字超出列宽时以 "..." 截断
    void set_truncate_cols(const std::vector<int>& cols) { truncate_cols_.assign(cols.begin(), cols.end()); }
    bool is_truncate_col(int col) const { return std::find(truncate_cols_.begin(), truncate_cols_.end(), col) != truncate_cols_.end(); }
    // 指定哪些列用 measure_text 精确截断（渲染时逐行测量，替代纯字符估算）
    void set_measure_text_cols(const std::vector<int>& cols) { measure_text_cols_.assign(cols.begin(), cols.end()); }
    bool is_measure_text_col(int col) const { return std::find(measure_text_cols_.begin(), measure_text_cols_.end(), col) != measure_text_cols_.end(); }

    void set_font(Font* font);
    void set_theme(const Theme* theme);
    void set_on_sort(std::function<void(int col, bool asc)> cb) { on_sort_ = std::move(cb); }
    void set_on_row_click(std::function<void(int row)> cb) { on_row_click_ = std::move(cb); }

    // —— 分页（layui page:true / resetPage）—— 分页时表底显示 Laypage，仅渲染当前页
    void set_paginated(bool on, int page_size = 10);
    bool paginated() const { return paginated_; }
    void set_page(int page);  // 0 基，立即跳页
    int page() const { return page_; }
    int page_count() const;
    int page_size() const { return page_size_; }
    // 当前页可见数据行数（未分页 = 全量行数）
    int visible_row_count() const;

    // —— 复选列 / 单选列（layui type:'checkbox' / type:'radio'）——
    void add_checkbox_column();  // 表头全选 + 各行复选位
    void add_radio_column();     // 表头占位 + 各行单选位
    void add_numbers_column();   // 序号列（从 1 起，跨页连续）
    bool has_selection() const { return selection_col_ >= 0; }
    int selection_col() const { return selection_col_; }
    bool is_radio_column() const { return selection_col_ >= 0 && radio_mode_; }
    bool is_cell_form(int col) const;
    bool is_cell_switch(int col) const;
    bool is_numbers(int col) const;
    bool is_button(int col) const { return col >= 0 && col < (int)cols_.size() && cols_[col].kind == ColKind::Button; }
    bool row_checked(int visible_row) const;   // 复选位
    int checked_radio() const;                 // 单选选中行（可见行，-1 无）
    int checked_count() const;                 // 当前选中行数（复选）
    int first_checked() const;                 // 第一个复选行（可见行，-1 无）
    // 选中 / 单选变化回调（参数：可见行号, 最新选中态）
    void set_on_selection(std::function<void(int visible_row, bool checked)> cb) { on_selection_ = std::move(cb); }

    // —— 单元格表单列（layui templet 内嵌 switch/checkbox，form.html）——
    void set_cell_switch(int col);    // 该列每格渲染 Switch（基于该行第 col 列文本非空与否预置开）
    void set_cell_checkbox(int col);  // 该列每格渲染 CheckBox（默认勾选：文本为 "1"/"true"）

    // —— 合计行（layui totalRow，totalRow.html / parseData.html）——
    void set_total_row(bool on) { total_row_ = on; }
    bool total_row() const { return total_row_; }
    void set_total_row_text(const char* s) { total_row_text_ = s ? s : "合计"; }
    // 标记某列为合计列（表尾对当前页该列数值求和）；total-row-text 仅放首合计列
    void set_total_col(int col) { total_col_ = col; }
    int total_col() const { return total_col_; }

    // —— 列点击事件（layui 单元格事件 cellEvent.html，如签名列 event:'setSign'）——
    void set_cell_event_col(int col) { cell_event_col_ = col; }
    int cell_event_col() const { return cell_event_col_; }
    void set_on_cell_event(std::function<void(int visible_row, int col)> cb) { on_cell_event_ = std::move(cb); }

    // —— 初始排序（layui initSort）——
    void set_init_sort(int col, bool asc);

    // —— 数据重载（layui reload）——
    void reload(const std::vector<std::vector<std::string>>& new_rows);

    // —— 行双击回调（layui onrow）——
    void set_on_row_double_click(std::function<void(int visible_row)> cb) { on_row_dbl_click_ = std::move(cb); }

    // —— 操作列（layui toolbar 列：行内按钮 查看/编辑/删除，toolbar/operate/totalRow 等）——
    void add_button_column(const char* title, float width,
                           const std::vector<std::string>& labels,
                           const std::vector<std::string>& events = {});
    int button_col() const { return button_col_; }
    int button_count() const { return (int)btn_labels_.size(); }
    const std::string& button_label(int i) const { return btn_labels_[i]; }
    const std::string& button_event(int i) const { return btn_events_[i]; }
    // 行内工具按钮点击回调（参数：可视行号, 事件名）
    void set_on_action(std::function<void(int visible_row, const std::string& event)> cb) { on_action_ = std::move(cb); }

    // 供内部行控件查询列/数据（TableRow）
    int col_count() const;
    float col_width(int c) const;
    const std::vector<std::string>& row(int r) const;
    int selected_row() const;
    bool even() const { return even_; }
    bool has_row_line() const { return skin_ == TableSkin::Default || skin_ == TableSkin::Line; }
    bool has_col_line() const { return skin_ == TableSkin::Default || skin_ == TableSkin::Row; }
    float header_h() const { return row_h(); }
    float row_h() const;
    float cell_pad_x() const;

    void on_layout() override;
    // 高度自适应：构造时 h=0 → 表头 + 行数 × 行高（静态表格整表展示）；h>0 → 固定 + 纵向滚动
    float preferred_height() const override;
    bool on_mouse_down(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    // 列内容类型
    enum class ColKind { Data, Checkbox, Radio, Numbers, CellSwitch, CellCheckbox, Button };
    struct Column {
        std::string title;
        float width = 0.f;
        bool sortable = false;
        bool ascending = true;
        ColKind kind = ColKind::Data;
    };

    void layout_columns();   // 表头/行共用列起始 x（缓存）
    void rebuild_rows();     // 重建 body 内行控件
    void truncate_rows();    // 按列宽截断 truncate_cols_ 中的文本
    void sort_rows(int col);
    void begin_edit(int row, int col, float click_x = -1.f);
    void commit_edit();
    bool in_header(float x, float y) const;
    int header_col_at(float x) const;
    // 可见行→数据行映射（分页切片）；未分页时索引即数据行号
    int data_row_of(int visible) const;
    int col_hit(float x) const;
    // 第 c 列起始 x（布局后有效）
    float col_x(int c) const;

    std::vector<Column> cols_;
    std::vector<std::vector<std::string>> rows_;
    std::vector<int> visible_rows_;  // 当前页可见行在 rows_ 中的下标
    int hover_row_ = -1;
    int sort_col_ = -1;
    TableSkin skin_ = TableSkin::Default;
    TableSize size_ = TableSize::Normal;
    bool even_ = false;

    // 自动截断列（文字超出列宽时截断为 "..."）
    std::vector<int> truncate_cols_;
    // 渲染时 measure_text 精确截断列（覆盖 truncate_cols 中的同名列）
    std::vector<int> measure_text_cols_;

    // 分页
    bool paginated_ = false;
    int page_size_ = 10;
    int page_ = 0;
    Laypage* page_bar_ = nullptr;

    // 复选/单选
    int selection_col_ = -1;
    bool radio_mode_ = false;
    std::vector<bool> row_checked_;
    int radio_sel_ = -1;   // 单选选中可见行
    std::function<void(int, bool)> on_selection_;

    // 合计行
    bool total_row_ = false;
    int total_col_ = -1;   // 合计列（-1 = 无）
    std::string total_row_text_ = "合计";

    // 列点击事件
    int cell_event_col_ = -1;
    std::function<void(int, int)> on_cell_event_;

    // 操作列（行内按钮）
    int button_col_ = -1;
    std::vector<std::string> btn_labels_;
    std::vector<std::string> btn_events_;
    std::function<void(int, const std::string&)> on_action_;

    ScrollBox* body_ = nullptr;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(int, bool)> on_sort_;
    std::function<void(int)> on_row_click_;
    std::function<void(int)> on_row_dbl_click_;

    // 双击检测（行内编辑）
    std::chrono::steady_clock::time_point last_click_;
    int last_click_row_ = -1;

    // 行内编辑状态
    TextBox* edit_box_ = nullptr;
    int edit_row_ = -1;   // 编辑的可视行号（定位用）
    int edit_col_ = -1;
    int edit_data_row_ = -1;  // 编辑对应的数据行下标（写回用，跨分页）

    // 列宽自适应：容器宽 > 各列宽之和时，剩余空间按比例分配给所有列
    std::vector<float> eff_widths_;  // 每列实际渲染宽度（on_layout 计算）
    float cell_min_width_ = 0.f;     // cellMinWidth：每列最小宽度（>0 时启用）
};

}  // namespace ui
