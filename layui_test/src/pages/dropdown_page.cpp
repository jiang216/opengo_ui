#include "../page_registry.h"

#include "ui/ui/code.h"
#include "ui/ui/dropdown.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/panel.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/table.h"
#include "ui/ui/theme.h"

#include <memory>
#include <string>
#include <vector>

namespace {

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

ui::HBox* add_row(PageHost& host) {
    ui::HBox* row = new ui::HBox(0, 0, 800, 48.f);
    row->set_padding(0.f);
    row->set_spacing(12.f);
    row->set_theme(host.theme);
    host.content->add_child(row);
    return row;
}

ui::Label* add_status(PageHost& host, const char* init) {
    ui::Label* status = new ui::Label(0, 0, 800, 0, init);
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    return status;
}

ui::Dropdown* make_drop(PageHost& host, ui::DropdownStyle style, const char* title, float w = 150.f,
                        float h = 38.f) {
    ui::Dropdown* d = new ui::Dropdown(0, 0, w, h, title);
    d->set_font(host.font);
    d->set_theme(host.theme);
    d->set_trigger_style(style);
    return d;
}

// 初演示：绿色 / 主色 按钮 + 输入框 + 文字链接
void build_basic(PageHost& host) {
    add_section(host, "初演示");
    ui::Label* status = add_status(host, "未点击");

    ui::HBox* row = add_row(host);
    ui::Dropdown* d1 = make_drop(host, ui::DropdownStyle::Button, "下拉菜单", 150.f, 38.f);
    d1->add_item("menu item11", [status]() { status->set_text("点击了：menu item11"); });
    d1->add_item("menu item22", [status]() { status->set_text("点击了：menu item22"); });
    d1->add_item("menu item33", [status]() { status->set_text("点击了：menu item33"); });
    d1->set_on_select([status](const char* t) { status->set_text((std::string("点击了：") + t).c_str()); });
    row->add_child(d1);

    ui::Dropdown* d1b = make_drop(host, ui::DropdownStyle::PrimaryButton, "下拉菜单", 150.f, 38.f);
    d1b->add_item("menu item11");
    d1b->add_item("menu item22");
    d1b->add_item("menu item33");
    d1b->set_on_select([status](const char* t) { status->set_text((std::string("点击了：") + t).c_str()); });
    row->add_child(d1b);

    // 输入框：选择后回填输入框
    ui::HBox* row2 = add_row(host);
    ui::Dropdown* d2 = make_drop(host, ui::DropdownStyle::Input, "在输入框提供一些常用的选项", 235.f, 38.f);
    for (int i = 1; i <= 6; ++i) {
        const std::string t = std::string("menu item ") + char('0' + i);
        d2->add_item(t.c_str());
    }
    row2->add_child(d2);

    ui::Label* word = new ui::Label(0, 0, 800, 32.f,
                                    "可以绑定任意元素，" /* + 链接由下面的 Dropdown 呈现 */);
    word->set_font(host.font);
    word->set_theme(host.theme);
    word->set_color(host.theme->text_secondary);
    row2->add_child(word);
    // 文字链接式触发器
    ui::Dropdown* d3 = make_drop(host, ui::DropdownStyle::Text, "比如这段文字", 160.f, 30.f);
    d3->add_item("menu item 1");
    d3->add_item("menu item 2", []() {});
    d3->add_parent("menu item 2 子菜单");
    d3->add_item("menu item 2-1");
    d3->add_item("menu item 2-2");
    d3->end_group();
    d3->add_separator();
    d3->add_group("menu group");
    d3->add_item("menu item 4-1");
    d3->add_item("menu item 4-2");
    d3->end_group();
    d3->add_separator();
    d3->add_item("menu item 5");
    d3->set_on_select([status](const char* t) { status->set_text((std::string("文字链接选择：") + t).c_str()); });
    row2->add_child(d3);
}

// 高级演示：层级 / 跳转 / 事件
void build_advance(PageHost& host) {
    add_section(host, "高级演示");
    ui::Label* status = add_status(host, "未点击");

    ui::HBox* row = add_row(host);
    ui::Dropdown* d100 = make_drop(host, ui::DropdownStyle::Button, "无限层级 + 跳转 + 事件", 260.f, 38.f);
    d100->add_item("menu item 1");
    d100->add_item("menu item 2");
    d100->add_separator();
    d100->add_group("menu item 3");
    d100->add_item("menu item 3-1");
    d100->add_parent("menu item 3-2");
    d100->add_item("menu item 3-2-1");
    d100->add_item("menu item 3-2-2");
    d100->add_item("menu item 3-2-3");
    d100->add_parent("menu item 3-3");
    d100->add_item("menu item 3-3-1");
    d100->add_item("menu item 3-3-2");
    d100->add_item("menu item 3-3-3");
    d100->end_group();
    d100->end_group();
    d100->add_separator();
    d100->add_item("menu item 4");
    d100->add_parent("menu item 5");
    d100->add_item("menu item 5-1");
    d100->add_item("menu item 5-2");
    d100->add_item("menu item 5-3");
    d100->end_group();
    d100->add_separator();
    d100->add_item("menu item 6");
    d100->set_on_select([status](const char* t) { status->set_text((std::string("点击了：") + t).c_str()); });
    row->add_child(d100);
}

// 自定义事件：hover / mousedown / dblclick
void build_events(PageHost& host) {
    add_section(host, "自定义事件");
    ui::HBox* row = add_row(host);
    const struct {
        const char* label;
        ui::DropdownTrigger trig;
    } trigs[] = {{"hover", ui::DropdownTrigger::Hover},
                 {"mousedown", ui::DropdownTrigger::MouseDown},
                 {"dblclick - 双击", ui::DropdownTrigger::DblClick}};
    for (auto& tr : trigs) {
        ui::Dropdown* d = make_drop(host, ui::DropdownStyle::PrimaryButton, tr.label, 180.f, 38.f);
        d->add_item("menu item 1");
        d->add_item("menu item 2");
        d->add_item("menu item 3");
        d->set_trigger_event(tr.trig);
        row->add_child(d);
    }
}

// 弹出位置：左 / 居中 / 右
void build_align(PageHost& host) {
    add_section(host, "弹出位置");
    ui::HBox* row = add_row(host);
    const struct {
        const char* label;
        ui::DropdownAlign align;
    } aligns[] = {{"左对齐", ui::DropdownAlign::Left},
                  {"居中对齐", ui::DropdownAlign::Center},
                  {"右对齐", ui::DropdownAlign::Right}};
    for (auto& a : aligns) {
        ui::Dropdown* d = make_drop(host, ui::DropdownStyle::PrimaryButton, a.label, 160.f, 38.f);
        for (int i = 1; i <= 3; ++i) d->add_item("menu item test");
        d->set_align(a.align);
        row->add_child(d);
    }
}

// 重定义风格：黑色弹板 + 重定义内容（自定义 Tab 面板）
void build_restyle(PageHost& host) {
    add_section(host, "重定义风格");
    ui::HBox* row = add_row(host);

    ui::Dropdown* d8 = make_drop(host, ui::DropdownStyle::Button, "重定义样式", 180.f, 38.f);
    d8->add_item("menu item 1");
    d8->add_item("menu item 2");
    d8->add_item("menu item 3");
    d8->add_item("menu item 4");
    d8->add_item("menu item 5");
    d8->add_item("menu item 6");
    d8->set_popup_style(ui::Vec4{0.f, 0.f, 0.f, 1.f}, ui::Vec4{0.2f, 0.2f, 0.2f, 1.f},
                        ui::Vec4{1.f, 1.f, 1.f, 1.f});  // 黑底 / 灰 hover / 白字
    d8->set_menu_width(120.f);
    row->add_child(d8);

    ui::Dropdown* d9 = make_drop(host, ui::DropdownStyle::Button, "重定义内容", 180.f, 38.f);
    ui::Panel* panel = new ui::Panel(0, 0, 380.f, 190.f);
    // 顶栏 Tab 结构（简化为标题行）+ 内容文本
    ui::Label* tabrow = new ui::Label(0, 16.f, 380.f, 34.f, "Tab header 1    Tab header 2    Tab header 3");
    tabrow->set_font(host.font);
    tabrow->set_theme(host.theme);
    tabrow->set_color(ui::Vec4{0.2f, 0.45f, 0.9f, 1.f});
    panel->add_child(tabrow);
    ui::Label* line = new ui::Label(0, 54.f, 380.f, 2.f, "");
    line->set_font(host.font);
    line->set_theme(host.theme);
    panel->add_child(line);
    ui::Label* p1 = new ui::Label(10.f, 70.f, 360.f, 40.f,
                                  "在 content 参数中插入任意的内容，可替代默认的下拉菜单，实现更多有趣的弹出内容。");
    p1->set_font(host.font);
    p1->set_theme(host.theme);
    p1->set_color(host.theme->text);
    p1->set_wrap(true);
    panel->add_child(p1);
    ui::Label* p2 = new ui::Label(10.f, 130.f, 360.f, 20.f,
                                  "dropdown 组件不仅是一个下拉菜单或右键菜单，它能被赋予许多想象可能。");
    p2->set_font(host.font);
    p2->set_theme(host.theme);
    p2->set_color(host.theme->text_secondary);
    p2->set_wrap(true);
    panel->add_child(p2);
    d9->set_custom_content(panel);
    row->add_child(d9);
}

// 右键菜单：灰色区域，此区域单击右键弹出
void build_context(PageHost& host) {
    add_section(host, "右键菜单");
    ui::Label* tip = add_status(host, "在此区域单击鼠标右键（灰色）");

    auto* ctx = new ui::Dropdown(0, 0, 800.f, 150.f, "");
    ctx->set_font(host.font);
    ctx->set_theme(host.theme);
    ctx->set_menu_width(200.f);
    ctx->set_region("在此区域单击鼠标右键");
    ctx->set_on_select([tip](const char* t) { tip->set_text((std::string("右键点击了：") + t).c_str()); });
    ctx->add_item("menu item 1");
    ctx->add_item("Printing");
    ctx->add_item("Reload");
    ctx->add_separator();
    ctx->add_item("menu item 3");
    ctx->add_parent("menu item 3 子菜单");
    ctx->add_item("menu item 3-1");
    ctx->add_item("menu item 3-2");
    ctx->add_item("menu item 3-3");
    ctx->end_group();
    ctx->add_separator();
    ctx->add_item("menu item 4");
    ctx->add_item("menu item 5");
    ctx->add_item("menu item 6");
    host.content->add_child(ctx);

    ui::HBox* row = add_row(host);
    ui::Label* toggle_lbl = new ui::Label(0, 0, 260, 28, "开启全局右键菜单");
    toggle_lbl->set_font(host.font);
    toggle_lbl->set_theme(host.theme);
    toggle_lbl->set_color(host.theme->text);
    row->add_child(toggle_lbl);
}

// 表格中的应用：操作列"更多"弹出右对齐下拉
void build_table(PageHost& host) {
    add_section(host, "在表格中的应用");
    ui::Label* status = add_status(host, "表格行操作：");

    ui::Table* tb = new ui::Table(0, 0, 728.f, 0);
    tb->set_font(host.font);
    tb->set_theme(host.theme);
    tb->add_column("ID", 60.f, true);
    tb->add_column("用户名", 120.f, false);
    tb->add_column("邮箱", 200.f, false);
    tb->add_button_column("操作", 160.f, {"编辑", "更多"}, {"edit", "more"});
    const char* rows[][3] = {{"001", "贤心", "xianxin@layui.com"},
                             {"002", "张三", "zhangsan@layui.com"},
                             {"003", "李四", "lisi@layui.com"}};
    for (auto& r : rows) tb->add_row({r[0], r[1], r[2]});
    host.content->add_child(tb);

    auto* share = new ui::Dropdown(0, 0, 0, 0, "更多");
    share->set_font(host.font);
    share->set_theme(host.theme);
    share->set_trigger_style(ui::DropdownStyle::Text);
    share->set_trigger_event(ui::DropdownTrigger::Manual);
    share->set_align(ui::DropdownAlign::Right);
    share->set_menu_width(120.f);
    share->add_item("item 1");
    share->add_item("item 2");
    share->add_separator();
    share->add_item("删除");
    share->set_on_select([status](const char* t) { status->set_text((std::string("表格行操作：") + t).c_str()); });
    host.content->add_child(share);

    tb->set_on_action([tb, share](int row, const std::string& ev) {
        if (ev == "edit") return;
        if (ev == "more") {
            const float header_h = 38.f;
            const float row_h = 38.f;
            const float cx = tb->bounds().x + tb->bounds().w - 12.f;
            const float cy = tb->bounds().y + header_h + row * row_h + row_h * 0.5f;
            share->show_at(cx - 120.f, cy);
        }
    });
}

}  // namespace

static void build(PageHost& host) {
    build_basic(host);
    build_advance(host);
    build_events(host);
    build_align(host);
    build_restyle(host);
    build_context(host);
    build_table(host);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/dropdown_page.cpp：克隆 layui dropdown.html 全示例\n"
            "\n"
            "ui::Dropdown* d = new ui::Dropdown(0, 0, 150.f, 38.f, \"下拉菜单\");\n"
            "d->set_font(host.font);\n"
            "d->set_theme(host.theme);\n"
            "d->set_trigger_style(ui::DropdownStyle::Button);   // Button / PrimaryButton / Input / Text\n"
            "d->set_trigger_event(ui::DropdownTrigger::Click);  // Click / Hover / MouseDown / DblClick / ContextMenu\n"
            "d->set_align(ui::DropdownAlign::Left);            // Left / Center / Right\n"
            "d->add_item(\"menu item 1\");\n"
            "d->add_separator();                                // 分隔线\n"
            "d->add_group(\"menu group\");                      // 纵向菜单组\n"
            "d->add_item(\"menu item 4-1\");\n"
            "d->end_group();\n"
            "d->add_parent(\"子菜单\");                          // 父级子面板（悬停展开）\n"
            "d->add_item(\"menu item 2-1\");\n"
            "d->end_group();\n"
            "d->set_popup_style(黑底, 灰hover, 白字);          // 重定义风格\n"
            "d->set_custom_content(panel);                      // 重定义内容\n"
            "row->add_child(d);");
    }
    if (host.help) {
        const char* lines[] = {
            "下拉菜单（Dropdown）API",
            "  set_trigger_style(s)：Button(绿键)/PrimaryButton(描边键)/Input(输入框)/Text(文字链接)",
            "  set_trigger_event(t)：Click/Hover/MouseDown/DblClick/ContextMenu/Manual",
            "  set_align(a)：Left(左对齐)/Center(居中)/Right(右对齐，默认 left)",
            "  add_item(title, cb)：菜单项；add_separator()：分隔线",
            "  add_group(title)/end_group()：纵向菜单组；add_parent(title)/end_group()：子面板",
            "  set_popup_style(bg,hover,text)：重定义弹板配色",
            "  set_custom_content(widget)：以任意控件替代默认菜单",
            "  set_menu_width(w)：弹板宽度；show()/hide()/show_at(x,y)：手动显隐",
            "交互：点击/悬停/双击/右键触发；点菜单项自动收起并回调 on_select(title)",
            "限制：父级子面板仅支持一层；深层递归子菜单暂不展开",
        };
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            label->set_font(host.font);
            label->set_theme(host.theme);
            label->set_color(host.theme->text);
            host.help->add_child(label);
        }
    }
}

static const PageDef kDropdownPage{54, "交互", "下拉菜单",
                                   "下拉菜单：触发器/菜单项/层级/分组/右键/对齐/风格", build, nullptr, false, nullptr, 0, "Dropdown"};

static const bool kRegistered = []() {
    page::register_page(kDropdownPage);
    return true;
}();