#include "../page_registry.h"

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

// demo onrow: 行事件（镜像 demo/table/onrow.html）
void demo_onrow(PageHost& host) {
    field_title(host, "行事件（镜像 demo/table/onrow.html：行单击 + 行双击）");
    ui::Label* status = nullptr;
    add_status(host, "点击或双击表格行查看事件回调", &status);
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 120.f);
    t->add_column("邮箱", 150.f);
    t->add_column("性别", 80.f, true);
    t->add_column("城市", 100.f);
    t->add_column("签名", 160.f);
    t->add_column("积分", 80.f, true);
    t->add_column("IP", 120.f);
    t->add_column("登入次数", 100.f, true);
    t->add_column("加入时间", 120.f);
    for (const auto& r : make_user_data(20)) t->add_row(r);
    t->set_paginated(true, 10);
    t->set_on_row_click([status, t](int row) {
        const auto& cells = t->row(row);
        char buf[96];
        snprintf(buf, sizeof(buf), "单击第 %d 行：ID=%s, 用户名=%s",
                 row + 1,
                 cells.size() > 0 ? cells[0].c_str() : "?",
                 cells.size() > 1 ? cells[1].c_str() : "?");
        status->set_text(buf);
    });
    t->set_on_row_double_click([status, t](int row) {
        const auto& cells = t->row(row);
        char buf[96];
        snprintf(buf, sizeof(buf), "双击第 %d 行：ID=%s, 用户名=%s",
                 row + 1,
                 cells.size() > 0 ? cells[0].c_str() : "?",
                 cells.size() > 1 ? cells[1].c_str() : "?");
        status->set_text(buf);
    });
    host.content->add_child(t);
}

// demo parseData: 解析任意数据格式（镜像 demo/table/parseData.html）
void demo_parse_data(PageHost& host) {
    field_title(host, "解析任意数据格式（镜像 demo/table/parseData.html）");
    ui::Label* note = new ui::Label(0, 0, 800, 0,
        "官方示例通过 parseData 回调将非标准 JSON 转为 table 规定格式。"
        "此处用本地数据模拟相同效果：直接加载并展示。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 120.f);
    t->add_column("性别", 80.f, true);
    t->add_column("城市", 100.f);
    t->add_column("签名", 180.f);
    t->add_column("积分", 80.f, true);
    t->add_column("评分", 80.f, true);
    t->add_column("职业", 80.f);
    t->add_column("财富", 100.f, true);
    for (const auto& r : make_user_data(25)) t->add_row(r);
    t->set_total_row(true);
    t->set_total_col(5);
    t->set_paginated(true, 10);
    host.content->add_child(t);
}

// demo reload: 数据表格的重载（镜像 demo/table/reload.html）
void demo_reload(PageHost& host) {
    field_title(host, "数据表格的重载（镜像 demo/table/reload.html）");
    ui::Label* status = nullptr;
    add_status(host, "点击按钮重载表格数据", &status);
    ui::Table* t = new ui::Table(0, 0, 800, 310.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_checkbox_column();
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 80.f);
    t->add_column("性别", 80.f, true);
    t->add_column("城市", 80.f);
    t->add_column("签名", 160.f);
    t->add_column("积分", 80.f, true);
    t->add_column("评分", 80.f, true);
    t->add_column("职业", 80.f);
    t->add_column("财富", 135.f, true);
    for (const auto& r : make_user_data(30)) t->add_row(r);
    t->set_paginated(true, 10);

    ui::Button* reload_btn = new ui::Button(0, 0, 180, 30, "重载为随机数据");
    reload_btn->set_font(host.font);
    reload_btn->set_theme(host.theme);
    reload_btn->set_on_click([t, status]() {
        std::vector<std::vector<std::string>> new_rows;
        for (int i = 0; i < 20; ++i) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", 20000 + i);
            int idx = i % 12;
            char exp[32];
            snprintf(exp, sizeof(exp), "%d", 10 + (i * 43) % 200);
            char wealth[32];
            snprintf(wealth, sizeof(wealth), "%d", 1000 + (i * 777) % 30000);
            new_rows.push_back({buf, kNames[idx], kSex[i % 2], kCities[idx],
                                "重载数据 " + std::to_string(i + 1), exp,
                                "4.0", kClassify[i % 6], wealth});
        }
        t->reload(new_rows);
        status->set_text("表格已重载（20 条随机数据）");
    });
    host.content->add_child(reload_btn);
    host.content->add_child(t);
}

// demo thead: 复杂表头（镜像 demo/table/thead.html，简化版：单级表头 + 操作列）
void demo_thead(PageHost& host) {
    field_title(host, "复杂表头（镜像 demo/table/thead.html，简化演示）");
    ui::Label* note = new ui::Label(0, 0, 800, 0,
        "官方示例使用 rowspan/colspan 多级表头，需要 Table 大幅重构。"
        "此处简化为标准单级表头 + 操作列。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_checkbox_column();
    t->add_column("联系人", 150.f);
    t->add_column("省", 120.f);
    t->add_column("市", 120.f);
    t->add_column("区", 200.f);
    t->add_column("金额", 120.f, true);
    t->add_button_column("操作", 160.f, {"按钮1", "按钮2"}, {"detail", "edit"});
    for (const auto& r : make_user_data(15)) t->add_row(r);
    t->set_paginated(true, 6);
    host.content->add_child(t);
}

}  // namespace

static void build_onrow_page(PageHost& host) { demo_onrow(host); }
static void build_parse_data_page(PageHost& host) { demo_parse_data(host); }
static void build_reload_page(PageHost& host) { demo_reload(host); }
static void build_thead_page(PageHost& host) { demo_thead(host); }

static const PageDef kOnrowPage{141, "组件示例", "行事件",
    "镜像 demo/table/onrow.html：行单击 + 行双击事件", build_onrow_page, nullptr, false, "数据表格", 4};
static const PageDef kParseDataPage{142, "组件示例", "解析任意数据格式",
    "镜像 demo/table/parseData.html：非标准数据解析", build_parse_data_page, nullptr, false, "数据表格", 4};
static const PageDef kReloadPage{143, "组件示例", "数据表格的重载",
    "镜像 demo/table/reload.html：按钮触发 reload", build_reload_page, nullptr, false, "数据表格", 4};
static const PageDef kTheadPage{163, "组件示例", "复杂表头",
    "镜像 demo/table/thead.html：简化版（单级表头+操作列）", build_thead_page, nullptr, false, "数据表格", 5};

static const bool kRegistered = []() {
    page::register_page(kOnrowPage);
    page::register_page(kParseDataPage);
    page::register_page(kReloadPage);
    page::register_page(kTheadPage);
    return true;
}();
