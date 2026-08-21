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

// 小节标题：带 legend 横线（layui-elem-field layui-field-title）
void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

// 生成镜像 /test/table/demo1.json 同构的本地数据（12 列 × 50 行），供合计/工具栏/单元格事件/操作列使用
const char* kU[] = {"贤心", "张三", "李四", "王五", "赵六", "孙七",
                    "周八", "吴九", "郑十", "钱十一", "陈十二", "刘十三"};
const char* kSexA[] = {"男", "女"};
const char* kCityA[] = {"海南", "浙江", "北京", "上海", "广州", "深圳",
                        "成都", "杭州", "武汉", "南京", "西安", "重庆"};

std::vector<std::vector<std::string>> make_demo1_data(int n) {
    std::vector<std::vector<std::string>> rows;
    rows.reserve(n);
    for (int i = 0; i < n; ++i) {
        char id[32], logins[32], join[32], exp[32], score[32], wealth[32];
        snprintf(id, sizeof(id), "%d", 10000 + i);
        snprintf(logins, sizeof(logins), "%d", 1 + (i * 13) % 90);
        snprintf(join, sizeof(join), "%04d-%02d-%02d", 2017 + i % 4, 1 + i % 12, 1 + i % 28);
        snprintf(exp, sizeof(exp), "%d", 20 + (i * 37) % 180);
        snprintf(score, sizeof(score), "%.1f", 3.0 + (i % 20) * 0.1);
        snprintf(wealth, sizeof(wealth), "%d", 2000 + (i * 911) % 25000);
        rows.push_back({id,
                        kU[i % 12],
                        kSexA[i % 2],
                        kCityA[i % 12],
                        "签名 " + std::to_string(i + 1),
                        logins,
                        join,
                        kSexA[i % 2],
                        exp,
                        "职业 " + std::to_string(1 + i % 6),
                        score,
                        wealth});
    }
    return rows;
}

// —— demo 1：开启合计行（镜像 demo/table/totalRow.html：top:true + 合计列 + toolbar 图标）——
void demo_total_row(PageHost& host) {
    field_title(host, "开启合计行（镜像 demo/table/totalRow.html：积分/登入次数列求和）");
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->set_total_row(true);
    t->set_total_row_text("合计");
    t->set_total_col(3);   // 对第 3 列（登入次数）求和展示在表尾
    t->add_checkbox_column();
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 100.f);
    t->add_column("性别", 70.f);
    t->add_column("登入次数", 90.f, true);
    t->add_column("邮箱", 200.f);
    for (const auto& r : make_demo1_data(30)) t->add_row(r);
    t->set_paginated(true, 8);
    host.content->add_child(t);
    ui::Label* note = new ui::Label(0, 0, 800, 0,
        "表尾固定合计行：首列显示“合计”，登入次数列求和（合计当前页行数据）。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

// —— demo 2：头部工具栏 + 行操作列（镜像 demo/table/toolbar.html：头部按钮 + defaultToolbar 图标 + 编辑/删除）——
void demo_toolbar(PageHost& host) {
    field_title(host, "头部工具栏 + 行操作列（镜像 demo/table/toolbar.html）");
    ui::Label* status = new ui::Label(0, 0, 800, 0, "选中 0 行");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    ui::Button* bt = new ui::Button(0, 0, 120, 30, "获取选中行数据");
    bt->set_font(host.font);
    bt->set_theme(host.theme);
    host.content->add_child(status);
    host.content->add_child(bt);
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_checkbox_column();
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 100.f);
    t->add_column("性别", 70.f);
    t->add_column("城市", 80.f);
    t->add_column("签名", 180.f);
    t->add_column("积分", 80.f, true);
    t->set_on_selection([status, t](int, bool) {
        char buf[48];
        snprintf(buf, sizeof(buf), "选中 %d 行", t->checked_count());
        status->set_text(buf);
    });
    bt->set_on_click([t, status]() {
        const int c = t->checked_count();
        char buf[64];
        if (c == 0) { status->set_text("请先勾选需要操作的行"); return; }
        const int r = t->first_checked();
        const auto& cells = t->row(r);
        snprintf(buf, sizeof(buf), "选中 %d 行，首个 ID=%s", c, cells.empty() ? "?" : cells[1].c_str());
        status->set_text(buf);
    });
    t->add_button_column("操作", 170.f, {"编辑", "删除"}, {"edit", "del"});
    t->set_on_action([t, status](int row, const std::string& ev) {
        const auto& cells = t->row(row);
        char buf[96];
        snprintf(buf, sizeof(buf), "[%s] 行 %d：ID=%s", ev.c_str(), row + 1,
                 cells.empty() ? "?" : cells[1].c_str());
        status->set_text(buf);
        if (ev == "del") {
            // 删除当前页可见行 → 反映到回调用户提示（不真正移除底层数据）
            std::string full = std::string(buf) + "（示例：触发删除操作）";
            status->set_text(full.c_str());
        }
    });
    for (const auto& r : make_demo1_data(30)) t->add_row(r);
    t->set_paginated(true, 8);
    host.content->add_child(t);
}

// —— demo 3：单元格事件（镜像 demo/table/cellEvent.html：点击签名列 event:'setSign'）——
void demo_cell_event(PageHost& host) {
    field_title(host, "单元格事件（镜像 demo/table/cellEvent.html：点击签名列弹改）");
    ui::Label* status = new ui::Label(0, 0, 800, 0, "点击下表中的“签名”列单元格，演示单元格事件");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_column("ID", 80.f);
    t->add_column("用户名", 100.f);
    t->add_column("签名", 300.f);
    t->add_column("积分", 100.f);
    t->add_column("评分", 100.f);
    t->set_cell_event_col(2);   // 签名列：单击触发单元格事件
    t->set_on_cell_event([status, t](int row, int) {
        const auto& cells = t->row(row);
        char buf[128];
        snprintf(buf, sizeof(buf), "单元格事件 setSign：行 %d 签名 = “%s”",
                 row + 1, cells.size() > 2 ? cells[2].c_str() : "?");
        status->set_text(buf);
    });
    for (const auto& r : make_demo1_data(30)) t->add_row(r);
    t->set_paginated(true, 8);
    host.content->add_child(t);
}

// —— demo 4：数据操作（镜像 demo/table/operate.html：头部按钮组 + 行操作列 查看/编辑/删除 + 复选框）——
void demo_operate(PageHost& host) {
    field_title(host, "数据操作（镜像 demo/table/operate.html：查看/编辑/删除 + 获取选中）");
    ui::Label* status = new ui::Label(0, 0, 800, 0, "选中 0 行");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    // 头部按钮组：获取选中行数据 / 获取选中数目 / 验证是否全选
    const char* labels[] = {"获取选中行数据", "获取选中数目", "验证是否全选"};
    ui::Button* btns[3];
    for (int i = 0; i < 3; ++i) {
        btns[i] = new ui::Button(0, 0, 170, 30, labels[i]);
        btns[i]->set_font(host.font);
        btns[i]->set_theme(host.theme);
    }
    ui::Table* t = new ui::Table(0, 0, 800, 320.f);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->add_checkbox_column();
    t->add_column("ID", 80.f, true);
    t->add_column("用户名", 100.f);
    t->add_column("性别", 70.f);
    t->add_column("城市", 80.f);
    t->add_column("签名", 180.f);
    t->add_column("积分", 80.f, true);
    btns[0]->set_on_click([t, status]() {
        int c = t->checked_count();
        char buf[96];
        snprintf(buf, sizeof(buf), "选中 %d 行，首个 ID=%s", c,
                 c ? t->row(t->first_checked())[1].c_str() : "-");
        status->set_text(buf);
    });
    btns[1]->set_on_click([t, status]() {
        char buf[48];
        snprintf(buf, sizeof(buf), "选中了：%d 个", t->checked_count());
        status->set_text(buf);
    });
    btns[2]->set_on_click([t, status]() {
        const int c = t->checked_count();
        const int total = t->visible_row_count();
        status->set_text((c == total && total > 0) ? "验证：全选" : "验证：未全选");
    });
    t->set_on_selection([status, t](int, bool) {
        char buf[48];
        snprintf(buf, sizeof(buf), "选中 %d 行", t->checked_count());
        status->set_text(buf);
    });
    t->add_button_column("操作", 180.f, {"查看", "编辑", "删除"}, {"detail", "edit", "del"});
    t->set_on_action([status, t](int row, const std::string& ev) {
        const auto& cells = t->row(row);
        char buf[128];
        snprintf(buf, sizeof(buf), "[%s] 行 %d：ID=%s", ev.c_str(), row + 1,
                 cells.empty() ? "?" : cells[1].c_str());
        status->set_text(buf);
    });
    for (const auto& r : make_demo1_data(30)) t->add_row(r);
    t->set_paginated(true, 8);
    host.content->add_child(btns[0]);
    host.content->add_child(btns[1]);
    host.content->add_child(btns[2]);
    host.content->add_child(t);
}

}  // namespace

static void build_total_row_page(PageHost& host) { demo_total_row(host); }
static void build_toolbar_page(PageHost& host) { demo_toolbar(host); }
static void build_cell_event_page(PageHost& host) { demo_cell_event(host); }
static void build_operate_page(PageHost& host) { demo_operate(host); }

static const PageDef kTotalRowPage{130, "组件示例", "开启合计行",
    "镜像 demo/table/totalRow.html：totalRow:true + 合计列求和 + toolbar", build_total_row_page, nullptr, false, "数据表格", 2};
static const PageDef kToolbarPage{131, "组件示例", "开启头部工具栏",
    "镜像 demo/table/toolbar.html：头部按钮 + defaultToolbar + 行操作列", build_toolbar_page, nullptr, false, "数据表格", 2};
static const PageDef kCellEventPage{160, "组件示例", "单元格事件",
    "镜像 demo/table/cellEvent.html：点击签名列触发事件", build_cell_event_page, nullptr, false, "数据表格", 5};
static const PageDef kOperatePage{140, "组件示例", "数据操作",
    "镜像 demo/table/operate.html：查看/编辑/删除 + 获取选中", build_operate_page, nullptr, false, "数据表格", 4};

static const bool kRegistered = []() {
    page::register_page(kTotalRowPage);
    page::register_page(kToolbarPage);
    page::register_page(kCellEventPage);
    page::register_page(kOperatePage);
    return true;
}();