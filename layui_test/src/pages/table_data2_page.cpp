#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/table.h"
#include "ui/ui/theme.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {

// 小节标题：带 legend 横线（layui-elem-field layui-field-title）
void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

// 生成镜像 /demo/table/user/ 同构的本地数据（9 列 × 50 行），供分页/复选/单选 demo 使用
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

// —— demo 1：开启分页（镜像 table/page.html，page:true）——
void demo_page(PageHost& host) {
    field_title(host, "开启分页（镜像 demo/table/page.html）");
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    add_user_columns(t);
    for (const auto& r : make_user_data(50)) t->add_row(r);
    t->set_paginated(true, 10);
    host.content->add_child(t);
}

// —— demo 2：自定义分页（镜像 demo/table/resetPage.html，page:{groups:1,first:false,last:false}）——
void demo_reset_page(PageHost& host) {
    field_title(host, "自定义分页（镜像 demo/table/resetPage.html：每页 6 条）");
    ui::Label* status = new ui::Label(0, 0, 800, 0, "当前页：1（共 50 条数据）");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    ui::Table* t = new ui::Table(0, 0, 800, 300.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    add_user_columns(t);
    for (const auto& r : make_user_data(50)) t->add_row(r);
    t->set_paginated(true, 6);
    host.content->add_child(t);
}

// —— demo 3：开启复选框（镜像 demo/table/checkbox.html，type:'checkbox' + page:true）——
void demo_checkbox(PageHost& host) {
    field_title(host, "开启复选框（镜像 demo/table/checkbox.html，type:'checkbox'）");
    ui::Label* status = new ui::Label(0, 0, 800, 0, "当前选中：0 行");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_checkbox_column();
    add_user_columns(t);
    for (const auto& r : make_user_data(50)) t->add_row(r);
    t->set_paginated(true, 10);
    t->set_on_selection([status, t](int, bool) {
        char buf[48];
        snprintf(buf, sizeof(buf), "当前选中：%d 行", t->checked_count());
        status->set_text(buf);
    });
    host.content->add_child(t);
}

// —— demo 4：开启单选框（镜像 demo/table/radio.html，type:'radio' + page:true + toolbar）——
void demo_radio(PageHost& host) {
    field_title(host, "开启单选框（镜像 demo/table/radio.html，type:'radio'）");
    ui::Label* status = new ui::Label(0, 0, 800, 0, "当前选中：第 - 行");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_radio_column();
    add_user_columns(t);
    for (const auto& r : make_user_data(50)) t->add_row(r);
    t->set_paginated(true, 10);
    t->set_on_selection([status, t](int row, bool) {
        char buf[64];
        int id = 0;
        const auto& cells = t->row(row);
        if (!cells.empty()) id = atoi(cells[0].c_str());
        snprintf(buf, sizeof(buf), "当前选中：第 %d 行（ID %d）", row + 1, id);
        status->set_text(buf);
    });
    host.content->add_child(t);
}

// —— demo 5：加入表单元素（镜像 demo/table/form.html：序号 + 复选框 + switch + checkbox）——
void demo_form(PageHost& host) {
    field_title(host, "加入表单元素（镜像 demo/table/form.html：序号/复选/switch/checkbox）");
    ui::Label* status = new ui::Label(0, 0, 800, 0, "切换单元格表单控件观察状态变化");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_numbers_column();
    t->add_checkbox_column();
    t->add_column("ID", 90.f, true);
    t->add_column("用户名", 110.f);
    t->add_column("城市", 100.f);
    t->add_column("财富", 130.f, true);
    t->add_column("性别", 100.f);
    t->set_cell_switch(6);      // 性别列渲染 Switch（文本 1=开/0=关）
    t->add_column("是否锁定", 120.f);
    t->set_cell_checkbox(7);    // 锁定列渲染 CheckBox
    for (int i = 0; i < 25; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", 10000 + i);
        char wealth[32];
        snprintf(wealth, sizeof(wealth), "%d", 2000 + (i * 911) % 25000);
        std::string sex = (i % 2 == 0) ? "1" : "0";
        std::string lock = (i % 5 == 0) ? "1" : "0";
        t->add_row({buf, kNames[i % 12], kCities[i % 12], wealth, sex, lock});
    }
    t->set_paginated(true, 10);
    host.content->add_child(t);
    ui::Label* note = new ui::Label(0, 0, 800, 0,
        "序号列跨页连续；性别 = Switch（开/关），是否锁定 = CheckBox（勾选/取消）。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

}  // namespace

// 拆分：每个镜像子页一个独立 board 子页（第二列列出，seg 分组，order 决定板内次序）

static void build_page_page(PageHost& host) { demo_page(host); }
static void build_reset_page(PageHost& host) { demo_reset_page(host); }
static void build_checkbox_page(PageHost& host) { demo_checkbox(host); }
static void build_radio_page(PageHost& host) { demo_radio(host); }
static void build_form_page(PageHost& host) { demo_form(host); }

static const PageDef kPagePage{110, "组件示例", "开启分页",
    "镜像 demo/table/page.html：page:true，10/页", build_page_page, nullptr, false, "数据表格", 2};
static const PageDef kResetPagePage{111, "组件示例", "自定义分页",
    "镜像 demo/table/resetPage.html：page:{...}，6/页", build_reset_page, nullptr, false, "数据表格", 2};
static const PageDef kCheckboxPage{112, "组件示例", "开启复选框",
    "镜像 demo/table/checkbox.html：type:'checkbox' + page", build_checkbox_page, nullptr, false, "数据表格", 2};
static const PageDef kRadioPage{113, "组件示例", "开启单选框",
    "镜像 demo/table/radio.html：type:'radio' + page", build_radio_page, nullptr, false, "数据表格", 2};
static const PageDef kFormPage{120, "组件示例", "加入表单元素",
    "镜像 demo/table/form.html：序号 + 复选 + switch + checkbox 单元格", build_form_page, nullptr, false, "数据表格", 3};

static const bool kRegistered = []() {
    page::register_page(kPagePage);
    page::register_page(kResetPagePage);
    page::register_page(kCheckboxPage);
    page::register_page(kRadioPage);
    page::register_page(kFormPage);
    return true;
}();