#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/table.h"
#include "ui/ui/theme.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {

void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

void add_status(PageHost& host, const char* text, ui::Label** out) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text);
    host.content->add_child(label);
    if (out) *out = label;
}

const char* kNames[] = {"贤心", "张三", "李四", "王五", "赵六", "孙七",
                        "周八", "吴九", "郑十", "钱十一", "陈十二", "刘十三"};
const char* kSex[] = {"男", "女"};
const char* kCities[] = {"海南", "浙江", "北京", "上海", "广州", "深圳",
                         "成都", "杭州", "武汉", "南京", "西安", "重庆"};
const char* kClassify[] = {"前端", "后端", "产品", "设计", "测试", "运营"};

std::vector<std::vector<std::string>> make_user_data(int n) {
    std::vector<std::vector<std::string>> rows;
    rows.reserve(n);
    for (int i = 0; i < n; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", 10000 + i);
        char exp[32];
        snprintf(exp, sizeof(exp), "%d", 20 + (i * 37) % 180);
        char score[32];
        snprintf(score, sizeof(score), "%.1f", 3.0 + (i % 20) * 0.1);
        char wealth[32];
        snprintf(wealth, sizeof(wealth), "%d", 2000 + (i * 911) % 25000);
        rows.push_back({buf,
                        kNames[i % 12],
                        kSex[i % 2],
                        kCities[i % 12],
                        "签名 " + std::to_string(i + 1),
                        exp,
                        score,
                        kClassify[i % 6],
                        wealth});
    }
    return rows;
}

void add_user_columns(ui::Table* t) {
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 80.f);
    t->add_column("性别", 80.f, true);
    t->add_column("城市", 80.f);
    t->add_column("签名", 200.f);
    t->add_column("积分", 80.f, true);
    t->add_column("评分", 80.f, true);
    t->add_column("职业", 80.f);
    t->add_column("财富", 100.f, true);
}

// demo auto: 列宽自动分配（镜像 demo/table/auto.html）
void demo_auto(PageHost& host) {
    field_title(host, "列宽自动分配（镜像 demo/table/auto.html）");
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    add_user_columns(t);
    for (const auto& r : make_user_data(20)) t->add_row(r);
    host.content->add_child(t);
    ui::Label* note = new ui::Label(0, 0, 800, 0,
        "列宽自动分配：各列按指定宽度排列，排序、行点击交互完整。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

// demo cellEdit: 开启单元格编辑（镜像 demo/table/cellEdit.html）
void demo_cell_edit(PageHost& host) {
    field_title(host, "开启单元格编辑（镜像 demo/table/cellEdit.html）");
    ui::Label* status = nullptr;
    add_status(host, "双击任意单元格即可编辑，按 Enter 或失焦提交", &status);
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_checkbox_column();
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 120.f, true);
    t->add_column("邮箱", 180.f);
    t->add_column("性别", 80.f, true);
    t->add_column("城市", 120.f);
    t->add_column("积分", 80.f, true);
    for (const auto& r : make_user_data(15)) t->add_row(r);
    t->set_paginated(true, 10);
    t->set_on_row_double_click([status, t](int row) {
        const auto& cells = t->row(row);
        char buf[64];
        snprintf(buf, sizeof(buf), "编辑第 %d 行（ID=%s）", row + 1,
                 cells.empty() ? "?" : cells[0].c_str());
        status->set_text(buf);
    });
    host.content->add_child(t);
}

// demo data: 赋值已知数据（镜像 demo/table/data.html）
void demo_data(PageHost& host) {
    field_title(host, "赋值已知数据（镜像 demo/table/data.html：静态本地数据）");
    ui::Table* t = new ui::Table(0, 0, 800, 300.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 120.f);
    t->add_column("邮箱", 180.f);
    t->add_column("签名", 180.f);
    t->add_column("性别", 80.f);
    t->add_column("城市", 100.f);
    t->add_column("积分", 80.f, true);
    t->add_row({"10001", "杜甫", "test@email.com", "人生恰似一场修行", "男", "浙江杭州", "116"});
    t->add_row({"10002", "李白", "test@email.com", "人生恰似一场修行", "男", "浙江杭州", "12"});
    t->add_row({"10003", "王勃", "test@email.com", "人生恰似一场修行", "男", "浙江杭州", "65"});
    t->add_row({"10004", "贤心", "test@email.com", "人生恰似一场修行", "男", "浙江杭州", "888"});
    t->add_row({"10005", "苏轼", "test@email.com", "人生恰似一场修行", "男", "浙江杭州", "256"});
    host.content->add_child(t);
}

// demo fixed: 固定列（镜像 demo/table/fixed.html，简化版）
void demo_fixed(PageHost& host) {
    field_title(host, "固定列（镜像 demo/table/fixed.html，简化演示）");
    ui::Label* note = new ui::Label(0, 0, 800, 0,
        "固定列需要横向滚动支持，当前简化演示基本表格 + 操作列。"
        "完整实现需支持 fixed:'left'/'right' 的列冻结布局。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 100.f);
    t->add_column("性别", 80.f, true);
    t->add_column("城市", 100.f);
    t->add_column("签名", 160.f);
    t->add_column("积分", 80.f, true);
    t->add_column("评分", 80.f, true);
    t->add_column("职业", 80.f);
    t->add_column("财富", 100.f, true);
    t->add_button_column("操作", 160.f, {"查看", "编辑", "删除"}, {"detail", "edit", "del"});
    for (const auto& r : make_user_data(20)) t->add_row(r);
    t->set_paginated(true, 10);
    host.content->add_child(t);
}

// demo height: 高度最大化适应（镜像 demo/table/height.html）
void demo_height(PageHost& host) {
    field_title(host, "高度最大化适应（镜像 demo/table/height.html）");
    ui::Table* t = new ui::Table(0, 0, 800, 400.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_checkbox_column();
    t->add_column("ID", 100.f, true);
    t->add_column("用户名", 100.f);
    t->add_column("性别", 100.f, true);
    t->add_column("签名", 200.f);
    t->add_column("积分", 100.f, true);
    t->add_column("评分", 100.f, true);
    for (const auto& r : make_user_data(30)) t->add_row(r);
    t->set_paginated(true, 30);
    host.content->add_child(t);
    ui::Label* note = new ui::Label(0, 0, 800, 0,
        "高度固定 400px，行数多时纵向滚动（纵向滚动已内置支持）。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

// demo initSort: 设置初始排序（镜像 demo/table/initSort.html）
void demo_init_sort(PageHost& host) {
    field_title(host, "设置初始排序（镜像 demo/table/initSort.html：财富列降序）");
    ui::Table* t = new ui::Table(0, 0, 800, 308.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_column("ID", 80.f);
    t->add_column("用户名", 80.f);
    t->add_column("评分", 80.f, true);
    t->add_column("财富", 150.f, true);
    for (const auto& r : make_user_data(20)) t->add_row(r);
    t->set_init_sort(3, false);  // 财富列降序
    host.content->add_child(t);
}

// demo style: 设置单元格样式（镜像 demo/table/style.html，简化版）
void demo_style(PageHost& host) {
    field_title(host, "设置单元格样式（镜像 demo/table/style.html：skin row + even）");
    ui::Table* t = new ui::Table(0, 0, 800, 305.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->set_skin(ui::TableSkin::Row);
    t->set_even(true);
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 80.f);
    t->add_column("性别", 80.f, true);
    t->add_column("城市", 80.f);
    t->add_column("签名", 160.f);
    t->add_column("积分", 80.f, true);
    t->add_column("评分", 80.f, true);
    t->add_column("职业", 80.f);
    t->add_column("财富", 135.f, true);
    for (const auto& r : make_user_data(20)) t->add_row(r);
    t->set_paginated(true, 10);
    host.content->add_child(t);
    ui::Label* note = new ui::Label(0, 0, 800, 0,
        "skin='row' 仅行分隔线 + even 斑马纹交替行背景色。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

// demo static: 转化静态表格（镜像 demo/table/static.html）
void demo_static(PageHost& host) {
    field_title(host, "转化静态表格（镜像 demo/table/static.html：静态表格 → 数据表格）");

    ui::Label* status = new ui::Label(0, 0, 800, 0,
        "当前为静态表格（不可交互），点击下方按钮转化为数据表格");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);

    ui::Button* btn = new ui::Button(0, 0, 180, 36, "立即转化为数据表格");
    btn->set_font(host.font);
    btn->set_theme(host.theme);
    btn->add_class("layui-btn");
    host.content->add_child(btn);

    ui::Table* static_table = new ui::Table(0, 0, 800, 200.f);
    static_table->set_font(host.font);
    static_table->set_theme(host.theme);
    static_table->add_column("昵称", 200.f);
    static_table->add_column("加入时间", 150.f);
    static_table->add_column("签名", 280.f);
    static_table->add_row({"贤心1", "2016-11-28", "人生就像是一场修行 A"});
    static_table->add_row({"贤心2", "2016-11-29", "人生就像是一场修行 B"});
    static_table->add_row({"贤心3", "2016-11-30", "人生就像是一场修行 C"});
    host.content->add_child(static_table);

    ui::Table* dynamic_table = new ui::Table(0, 0, 800, 200.f);
    dynamic_table->set_font(host.font);
    dynamic_table->set_theme(host.theme);
    dynamic_table->add_column("昵称", 200.f, true);
    dynamic_table->add_column("加入时间", 150.f);
    dynamic_table->add_column("签名", 280.f);
    dynamic_table->add_row({"贤心1", "2016-11-28", "人生就像是一场修行 A"});
    dynamic_table->add_row({"贤心2", "2016-11-29", "人生就像是一场修行 B"});
    dynamic_table->add_row({"贤心3", "2016-11-30", "人生就像是一场修行 C"});
    dynamic_table->set_on_row_click([status](int row) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "已转化：点击第 %d 行（数据表格模式）", row + 1);
        status->set_text(buf);
    });
    dynamic_table->visible = false;
    host.content->add_child(dynamic_table);

    btn->set_on_click([status, btn, static_table, dynamic_table]() {
        static_table->visible = false;
        dynamic_table->visible = true;
        btn->set_text("已转化为数据表格");
        btn->set_enabled(false);
        status->set_text("已转化：昵称列可排序，双击单元格可编辑，行点击可交互");
    });

    ui::Label* note = new ui::Label(0, 0, 800, 0,
        "静态表格不支持排序/编辑/行事件；点击按钮后转化为可交互的数据表格。"
        "官方 demo 用 table.init() 将 <table> 标签原地转化为数据表格，"
        "本页用控件切换模拟相同效果。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

}  // namespace

static void build_auto_page(PageHost& host) { demo_auto(host); }
static void build_cell_edit_page(PageHost& host) { demo_cell_edit(host); }
static void build_data_page(PageHost& host) { demo_data(host); }
static void build_fixed_page(PageHost& host) { demo_fixed(host); }
static void build_height_page(PageHost& host) { demo_height(host); }
static void build_init_sort_page(PageHost& host) { demo_init_sort(host); }
static void build_static_page(PageHost& host) { demo_static(host); }
static void build_style_page(PageHost& host) { demo_style(host); }

static const PageDef kAutoPage{101, "组件示例", "列宽自动分配",
    "镜像 demo/table/auto.html：列宽自适应 + 排序", build_auto_page, nullptr, false, "数据表格", 1};
static const PageDef kDataPage{102, "组件示例", "赋值已知数据",
    "镜像 demo/table/data.html：静态本地数据", build_data_page, nullptr, false, "数据表格", 1};
static const PageDef kCellEditPage{121, "组件示例", "开启单元格编辑",
    "镜像 demo/table/cellEdit.html：双击编辑单元格", build_cell_edit_page, nullptr, false, "数据表格", 2};
static const PageDef kFixedPage{122, "组件示例", "固定列",
    "镜像 demo/table/fixed.html：简化版", build_fixed_page, nullptr, false, "数据表格", 3};
static const PageDef kStylePage{123, "组件示例", "设置单元格样式",
    "镜像 demo/table/style.html：skin row + even", build_style_page, nullptr, false, "数据表格", 3};
static const PageDef kHeightPage{161, "组件示例", "高度最大化适应",
    "镜像 demo/table/height.html：固定高度 + 纵向滚动", build_height_page, nullptr, false, "数据表格", 5};
static const PageDef kInitSortPage{162, "组件示例", "设置初始排序",
    "镜像 demo/table/initSort.html：财富列降序", build_init_sort_page, nullptr, false, "数据表格", 5};
static const PageDef kStaticPage{103, "组件示例", "转化静态表格",
    "镜像 demo/table/static.html：静态表格转化为数据表格", build_static_page, nullptr, false, "数据表格", 1};

static const bool kRegistered = []() {
    page::register_page(kAutoPage);
    page::register_page(kDataPage);
    page::register_page(kStaticPage);
    page::register_page(kCellEditPage);
    page::register_page(kFixedPage);
    page::register_page(kStylePage);
    page::register_page(kHeightPage);
    page::register_page(kInitSortPage);
    return true;
}();
