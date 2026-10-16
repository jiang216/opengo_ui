#pragma once

#include "ui/ui/widget.h"

#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace ui {
class ComboBox;
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

    void set_skin(TableSkin skin) { skin_ = skin; mark_dirty(); }
    void set_size(TableSize size) { size_ = size; mark_dirty(); }
    void set_even(bool even) { even_ = even; mark_dirty(); }
    void set_cell_min_width(float w) { cell_min_width_ = w; mark_dirty(); }
    // 指定哪些列启用自动截断：文字超出列宽时以 "..." 截断
    void set_truncate_cols(const std::vector<int>& cols) { truncate_cols_.assign(cols.begin(), cols.end()); mark_dirty(); }
    bool is_truncate_col(int col) const { return std::find(truncate_cols_.begin(), truncate_cols_.end(), col) != truncate_cols_.end(); }
    // 指定哪些列用 measure_text 精确截断（渲染时逐行测量，替代纯字符估算）
    void set_measure_text_cols(const std::vector<int>& cols) { measure_text_cols_.assign(cols.begin(), cols.end()); mark_dirty(); }
    bool is_measure_text_col(int col) const { return std::find(measure_text_cols_.begin(), measure_text_cols_.end(), col) != measure_text_cols_.end(); }

    // —— 单元格 tooltip（悬停显示完整文本，UI-177）——
    // 命中可截断列时，行控件按悬停列查询完整文本并置为行 tooltip；返回空串则不显示。
    // 参数：数据行号（rows_ 下标）, 列号。应用层借此提供"省略后的完整内容"。
    void set_cell_tooltip(std::function<std::string(int data_row, int col)> cb) { cell_tooltip_ = std::move(cb); mark_dirty(); }
    // 行控件调用：可视行号 + 根空间横坐标 → 计算命中列的 tooltip 文本（未注册回调返回空）
    std::string cell_tooltip_at(int visible_row, float x) const;

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
    // 按当前 bounds 高度自适应每页行数（UI-123）：已扣除表头与内部分页条高度
    // （28px）。调用方此前按 (h-表头)/行高 自行估算，末行被挤进分页条下方盲区。
    int auto_page_size() const;
    // 当前页可见数据行数（未分页 = 全量行数）
    int visible_row_count() const;
    // 可视行号 → 数据行下标（rows_ 下标；跨分页/树形过滤的稳定行标识）
    int data_row_of(int visible) const;

    // —— 流加载（layui flow 信息流滚动自动加载，UI-126）—— 仅非分页模式生效：
    // body_ 滚动近底部（mb 临界距离，UI-035/UI-094 语义）时回调；配合 add_row/reload
    // 续灌数据。rebuild_rows 尾部自动 rearm 重检：追加批次后若内容底仍未推出触发区，
    // 将继续回调直至填满一屏或数据耗尽（耗尽时回调方停止 append 即可，链自然断）。
    void set_on_load_more(std::function<void()> cb);
    // 流加载尾注（如「没有更多了」）：非分页且非空时在最后一行之后渲染只读提示行
    // （灰字居中跨列，随内容滚动）；不参与 data_row_of/选中/编辑/命中回调。
    // 文本变化才触发重建，空串清除。
    void set_end_note(const char* text);
    // 滚动回顶部（过滤条件变化重灌数据后调用）
    void reset_scroll();
    // 预热表头 + 当前可见行全部单元格 + 尾注的 CJK 字形（UI-127）：一次性烘焙消除
    // UI-125 逐帧配额限流的方块宽渐进补齐闪烁（流加载续灌瞬间/表格首次显示瞬间）。
    // prebake_text 不受配额限制且自动跳过已烘焙码点，重复调用开销可忽略。
    void prebake_visible_text();

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
    // 单元格表单切换回调（UI-129）：点击 Switch/CheckBox 格翻转后触发
    // （参数：可见行号, 列号, 切换后的选中态）；此前仅翻转单元格文本不通知，调用方无法感知
    void set_on_cell_form(std::function<void(int visible_row, int col, bool checked)> cb) { on_cell_form_ = std::move(cb); }

    // —— 单元格文本编辑提交回调（UI-134）——
    // 双击文本格进入行内编辑，Enter/失焦 commit_edit 写回 rows_ 后触发；
    // 参数：数据行号（rows_ 下标，跨分页/排序稳定）, 列号, 新文本。
    // 此前仅写控件私有副本不通知调用方，应用层无法感知编辑结果
    // （audio_text 模式映射编辑丢失缺陷根源）。
    void set_on_cell_edited(std::function<void(int data_row, int col, const std::string& text)> cb) { on_cell_edited_ = std::move(cb); }
    // 列级可编辑开关（UI-134）：false 则该列双击不进入行内编辑（默认 true 可编辑）
    void set_cell_editable(int col, bool editable);

    // —— 下拉单元格列（UI-143：表格行内下拉选择，复用 ComboBox layui-form-select 皮肤）——
    // 该列每格渲染文本 + 右侧下拉三角，单击单元格展开行内 ComboBox；
    // 选项选中后写回该格文本并触发选中回调。
    void set_cell_combobox(int col);
    bool is_cell_combobox(int col) const;
    // 指定下拉列的可选项（与列下标平行；未设置视为空选项集）
    void set_combobox_options(int col, const std::vector<std::string>& options);
    // 单元格（数据行, 列）级下拉选项覆盖（M20：每后端一行，各行语种独立声明）。
    // row 为数据行下标（rows_ 下标，跨分页/排序稳定）；此后该格展开时优先用此覆盖集，
    // 缺省（未设置/空集）回退列级 set_combobox_options。clear_rows/reload 后覆盖失效。
    void set_cell_combobox_options(int row, int col, const std::vector<std::string>& options);
    // 单元格级下拉开关（M20-WP8）：标记某特定单元格为下拉型，独立于列级 set_cell_combobox；
    // 命中时展开行内 ComboBox（复用 combo_options_for 行级覆盖/列级回退）。
    // row=数据行下标，col=列号；clear_rows/reload 后标记失效。
    void set_cell_combo(int row, int col, bool on = true);
    bool is_cell_combo(int data_row, int col) const;
    // 下拉选中回调（参数：可见行号, 列号, 选项下标, 选中文本）
    void set_on_cell_combobox(std::function<void(int visible_row, int col, int index, const std::string& text)> cb) { on_cell_combo_ = std::move(cb); }

    // —— 合计行（layui totalRow，totalRow.html / parseData.html）——
    void set_total_row(bool on) { total_row_ = on; mark_dirty(); }
    bool total_row() const { return total_row_; }
    void set_total_row_text(const char* s) { total_row_text_ = s ? s : "合计"; mark_dirty(); }
    // 标记某列为合计列（表尾对当前页该列数值求和）；total-row-text 仅放首合计列
    void set_total_col(int col) { total_col_ = col; mark_dirty(); }
    int total_col() const { return total_col_; }

    // —— 列点击事件（layui 单元格事件 cellEvent.html，如签名列 event:'setSign'）——
    void set_cell_event_col(int col) { cell_event_col_ = col; mark_dirty(); }
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

    // —— 树形层级（layui treeTable）——
    // 追加带层级的行：depth 0=根，>0 为上一 depth-1 行的子级（须按先序次序追加）。
    // 有子级的行首列渲染展开三角（▼ 展开 / ▶ 折叠），点击切换；折叠时整棵子树隐藏。
    void add_tree_row(int depth, const std::vector<std::string>& cells);
    bool is_tree() const;                       // 存在 depth>0 的行
    int row_depth(int data_row) const;          // 层级深度（add_row 的行恒为 0）
    bool row_has_children(int data_row) const;  // 下一行深度更深即有子级
    bool row_expanded(int data_row) const;      // 默认展开
    void set_row_expanded(int data_row, bool on);
    void toggle_row(int visible_row);           // 点击展开图标切换并重建
    float tree_indent() const { return 20.f; }  // 每级缩进像素
    void set_on_expand(std::function<void(int data_row, bool expanded)> cb) { on_expand_ = std::move(cb); }

    // 供内部行控件查询列/数据（TableRow）
    int col_count() const;
    float col_width(int c) const;
    const std::vector<std::string>& row(int r) const;
    int selected_row() const;
    // 行选中态（UI-124）：单击数据行高亮定位；参数为可见行号，-1 清除。
    // 与复选/单选列无关，纯视觉反馈；reload/set_page/排序时自动清除。
    void set_selected_row(int visible_row) { sel_row_ = visible_row; mark_dirty(); }
    bool even() const { return even_; }
    bool has_row_line() const { return skin_ == TableSkin::Default || skin_ == TableSkin::Line; }
    bool has_col_line() const { return skin_ == TableSkin::Default || skin_ == TableSkin::Row; }
    float header_h() const { return row_h(); }
    float row_h() const;
    float cell_pad_x() const;

    // —— 列宽拖动调节（UI-211，资源管理器式表头分隔线拖拽）——
    // set_column_resizable(true) 后，表头相邻列分隔线（±kDividerHit px）可拖动调宽；
    // 按下返回 true 即被框架自动捕获鼠标（widget dispatch），随后 on_mouse_move/on_mouse_up
    // 接管拖动，调用方无需自行管理捕获。
    // set_flex_col：弹性列——拖动时它按需伸缩，使「总表宽恒定」（资源管理器语义）；
    //   默认 -1 保持原有「余量按比例分摊给所有列」的布局行为，兼容既有页面。
    // set_column_min_width：该列拖动下限（与 cols_ 平行，未设=0）。
    // set_on_column_resize：拖动结束回调（列, 最终宽度），用于持久化。
    void set_column_resizable(bool on) { column_resizable_ = on; mark_dirty(); }
    void set_flex_col(int col) { flex_col_ = col; mark_dirty(); }
    void set_column_min_width(int col, float w) {
        if (col < 0) return;
        if ((int)col_min_w_.size() <= col) col_min_w_.resize(col + 1, 0.f);
        col_min_w_.at(col) = w;
        mark_dirty();
    }
    void set_column_width(int col, float w) {
        if (col >= 0 && col < (int)cols_.size() && w > 0.f) { cols_[col].width = w; mark_dirty(); }
    }
    void set_on_column_resize(std::function<void(int, float)> cb) { on_col_resize_ = std::move(cb); }
    // 命中分隔线则进入拖动状态并返回 true（供 on_mouse_down 顶部先判；列下标为「左列」）
    bool column_resize_begin(float x, float y);
    // 表头分隔线悬停查询（左侧列下标，-1 无）——供光标反馈
    int column_divider_at(float x, float y) const;
    // 是否正在拖动列宽（供宿主在拖动中维持「左右箭头」光标）
    bool column_resizing() const { return drag_col_ >= 0; }
    // 表头 x 所在列下标（col_hit 的公开包装；应用层用它替代自算列边界，避免漂移）
    int col_at_x(float x) const { return col_hit(x); }

    void on_layout() override;
    // 高度语义（UI-131）：默认内容跟随——每次调用按表头 + 可见行数×行高 [+ 分页条] 重算，
    // 不再被首次分配的 bounds_.h 冻结（旧「h>0 即冻结」导致流式容器内行数增长永不反映，
    // 行区被裁剪成 0，见 audio_text SPK-005）。
    // set_fixed_height(h>0)：显式锁定高度（固定视口 + 内部纵向滚动的摆放），h<=0 解除回内容跟随；
    // set_max_content_height(h>0)：内容跟随模式的上限钳制（流式容器防溢出），超出部分由内部滚动区承载。
    float preferred_height() const override;
    void set_fixed_height(float h) { fixed_h_ = h; mark_dirty(); }
    void set_max_content_height(float h) { max_content_h_ = h; mark_dirty(); }
    bool on_mouse_down(float x, float y, int button) override;
    // 列宽拖动（UI-211）：按下分隔线被自动捕获后，move 更新宽度、up 结束并回调
    bool on_mouse_move(float x, float y) override;
    bool on_mouse_up(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    // 列内容类型
    enum class ColKind { Data, Checkbox, Radio, Numbers, CellSwitch, CellCheckbox, Button, ComboBox };
    struct Column {
        std::string title;
        float width = 0.f;
        bool sortable = false;
        bool ascending = true;
        bool editable = true;  // UI-134：false 则双击不进入行内编辑
        ColKind kind = ColKind::Data;
    };

    void layout_columns();   // 表头/行共用列起始 x（缓存）
    void rebuild_rows();     // 重建 body 内行控件
    void truncate_rows();    // 按列宽截断 truncate_cols_ 中的文本
    void sort_rows(int col);
    void begin_edit(int row, int col, float click_x = -1.f);
    void commit_edit();
    // 下拉单元格列：单击展开行内 ComboBox（UI-143）
    void begin_combo(int row, int col);
    void close_combo();
    bool in_header(float x, float y) const;
    int header_col_at(float x) const;
    int col_hit(float x) const;
    // UI-211：表头列分隔线命中（±kDividerHit px）→ 右侧列下标，未命中 -1
    int divider_index(float x, float y) const;
    // 第 c 列起始 x（布局后有效）
    float col_x(int c) const;

    std::vector<Column> cols_;
    std::vector<std::vector<std::string>> rows_;
    std::vector<int> row_depth_;     // 与 rows_ 平行；add_row 恒为 0，add_tree_row 指定
    std::vector<bool> row_expanded_; // 与 rows_ 平行；默认 true
    std::vector<int> tree_visible_;  // 树形过滤后的数据行（祖先全展开），分页在其上切片
    std::vector<int> visible_rows_;  // 当前页可见行在 rows_ 中的下标
    int hover_row_ = -1;
    int sel_row_ = -1;   // 单击选中行（可见行号，UI-124；-1 无）
    // 高度语义（UI-131）：fixed_h_>0 显式锁定 preferred_height；max_content_h_>0 内容跟随上限钳制
    float fixed_h_ = 0.f;
    float max_content_h_ = 0.f;
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

    // 单元格 tooltip（UI-177）：悬停命中列的完整文本查询回调
    std::function<std::string(int, int)> cell_tooltip_;

    // 单元格表单切换（UI-129）
    std::function<void(int, int, bool)> on_cell_form_;

    // 单元格文本编辑提交（UI-134）
    std::function<void(int, int, const std::string&)> on_cell_edited_;

    // 操作列（行内按钮）
    int button_col_ = -1;
    std::vector<std::string> btn_labels_;
    std::vector<std::string> btn_events_;
    std::function<void(int, const std::string&)> on_action_;

    // 树形层级（treeTable）
    std::function<void(int, bool)> on_expand_;  // 展开/折叠回调（数据行下标, 展开态）

    // 流加载（UI-126）：触底续灌回调 + 尾注文本（仅非分页模式参与）
    std::function<void()> on_load_more_;
    std::string end_note_;

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

    // 下拉单元格状态（UI-143）
    ComboBox* combo_box_ = nullptr;
    int combo_row_ = -1;   // 展开的可视行号
    int combo_col_ = -1;
    int combo_data_row_ = -1;  // 展开对应的数据行下标
    // 下拉列选项集（与 cols_ 下标平行；未设置列留空）
    std::vector<std::vector<std::string>> combo_options_;
    // 单元格级下拉选项覆盖（M20）：外层=数据行下标，中层=列下标，内层=选项集；
    // 空集表示未覆盖，回退列级 combo_options_
    std::vector<std::vector<std::vector<std::string>>> combo_row_options_;
    // 单元格级下拉开关（M20-WP8）：per-cell combo 标记，结构同 combo_row_options_（外层=行，中层=列）
    std::vector<std::vector<bool>> cell_combo_;
    const std::vector<std::string>& combo_options_for(int data, int col) const;
    std::function<void(int, int, int, const std::string&)> on_cell_combo_;

    // 列宽自适应：容器宽 > 各列宽之和时，剩余空间按比例分配给所有列
    std::vector<float> eff_widths_;  // 每列实际渲染宽度（on_layout 计算）
    float cell_min_width_ = 0.f;     // cellMinWidth：每列最小宽度（>0 时启用）

    // —— 列宽拖动（UI-211）——
    bool  column_resizable_ = false;  // 开关（默认关，既有页面零影响）
    int   flex_col_ = -1;             // 弹性列（-1 = 维持旧版「余量比例分摊」）
    int   drag_col_ = -1;             // 拖动中的左列下标（-1 = 未在拖）
    float drag_x0_ = 0.f;             // 拖动起点 x（累积位移基线）
    float drag_w0_ = 0.f;             // 起点宽度（避免逐帧累积浮点漂移）
    float drag_fw0_ = 0.f;            // 起点弹性列宽
    int   hover_div_ = -1;            // 悬停分隔线（左侧列下标，-1 无）→ 高亮/光标
    std::vector<float> col_min_w_;    // 各列最小宽度（与 cols_ 平行，未设=0）
    std::function<void(int, float)> on_col_resize_;
};

}  // namespace ui
