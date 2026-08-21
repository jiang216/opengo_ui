#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/label.h"
#include "ui/ui/menu.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

// 左侧菜单（demo1）：克隆 layui menu.html 第一个面板
void build_left_menu(PageHost& host) {
    ui::Menu* m = new ui::Menu(0, 0, 260, 0);
    m->set_font(host.font);
    m->set_theme(host.theme);

    m->add_item("menu item 1");
    m->add_item("menu item 2");
    m->set_dot_badge();
    m->add_divider();

    // menu group (spread, collapsible)
    m->add_group("menu group", true);
    m->add_item("menu item 3-1");
    // Nested sub-group (no spread)
    m->add_group("menu group 2", false);
    m->set_checked();
    m->add_item("menu item 3-2-1");
    m->add_item("menu item 3-2-2");
    m->end_group();
    m->add_item("menu item 3-3");
    m->end_group();

    m->add_divider();

    m->add_item("menu item 4");
    m->set_badge("1");

    m->add_item("menu item 5");
    m->add_item("menu item 6");

    // Parent item → sub-panel
    m->add_parent("menu item 7");
    m->add_item("menu item 7-2");
    m->add_item("menu item 7-3");

    m->add_divider();

    // menu group 9 (spread)
    m->add_group("menu group 9", true);
    m->add_item("menu item 9-1");
    m->add_parent("menu item 9-2");
    m->add_item("menu item 9-2-1");
    m->add_item("menu item 9-2-2");
    m->add_item("menu item 9-2-3");
    m->add_item("menu item 9-31");
    m->end_group();

    m->add_divider();

    m->add_item("menu item 8");

    m->add_divider();

    m->add_item("menu item 10");

    host.content->add_child(m);
}

// 右侧菜单（docDemoMenu1）：克隆 layui menu.html 第二个面板
void build_right_menu(PageHost& host) {
    ui::Menu* m = new ui::Menu(0, 0, 260, 0);
    m->set_font(host.font);
    m->set_theme(host.theme);

    m->add_item("menu item 1");
    m->add_item("menu item 2");
    m->set_dot_badge();
    m->add_divider();

    // menu group (no spread)
    m->add_group("menu group", false);
    m->add_item("menu item 3-1");
    m->add_item("menu item 3-2");
    m->end_group();

    m->add_divider();

    // menu group 2 (no spread)
    m->add_group("menu group 2", false);
    m->add_item("menu item 4-1");
    m->add_item("menu item 4-2");
    m->end_group();

    m->add_divider();

    // Parent item → sub-panel
    m->add_parent("menu item 5");
    m->add_item("menu item 5-1");
    m->add_item("menu item 5-2");

    m->add_divider();

    m->add_item("menu item 6");

    host.content->add_child(m);
}

void build_menubar(PageHost& host) {
    // Quote
    ui::BlockQuote* q = new ui::BlockQuote(700, 0,
        "以下为「基础菜单」的静态展示，更多操作可见：下拉菜单组件", false);
    q->set_font(host.font);
    q->set_theme(host.theme);
    host.content->add_child(q);

    add_section(host, " ");

    // Two menus side by side
    build_left_menu(host);

    add_section(host, " ");

    build_right_menu(host);

    add_section(host, " ");

    ui::BlockQuote* q2 = new ui::BlockQuote(700, 0,
        "更多我们未能呈现的示例，还有待您在阅读文档、以及不断使用的过程，去深入挖掘。", false);
    q2->set_font(host.font);
    q2->set_theme(host.theme);
    host.content->add_child(q2);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/menubar_page.cpp — 克隆 layui menu.html 基础菜单\n"
            "\n"
            "ui::Menu* m = new ui::Menu(0, 0, 260, 0);\n"
            "m->set_font(host.font);\n"
            "m->set_theme(host.theme);\n"
            "\n"
            "m->add_item(\"menu item 1\");\n"
            "m->add_item(\"menu item 2\");\n"
            "m->set_dot_badge();\n"
            "m->add_divider();\n"
            "\n"
            "m->add_group(\"menu group\", true);  // 可折叠分组\n"
            "  m->add_item(\"menu item 3-1\");\n"
            "  m->add_group(\"menu group 2\", false);\n"
            "    m->set_checked();                // 选中态（绿色）\n"
            "    m->add_item(\"menu item 3-2-1\");\n"
            "  m->end_group();\n"
            "m->end_group();\n"
            "\n"
            "m->add_parent(\"menu item 7\");       // 父级 → 右侧弹出子面板\n"
            "  m->add_item(\"menu item 7-2\");\n"
            "  m->add_item(\"menu item 7-3\");\n"
            "\n"
            "host.content->add_child(m);");
    }
    if (host.help) {
        const char* lines[] = {
            "Menu（基础垂直菜单）API",
            "  add_item(title, cb)：添加普通菜单项",
            "  add_divider()：添加分割线",
            "  add_group(title, allow_spread)：添加可折叠分组（子项缩进）",
            "  add_parent(title)：添加父级项（悬停显示右侧子面板）",
            "  end_group()：结束当前分组/父级嵌套层级",
            "  set_checked()：标记最后添加的项为选中态（绿色）",
            "  set_badge(text)：给最后添加的项添加文字徽章",
            "  set_dot_badge()：给最后添加的项添加圆点徽章",
            "  set_large(bool)：切换大尺寸菜单项（48px → 40px）",
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

}  // namespace

static void build(PageHost& host) {
    build_menubar(host);
}

static const PageDef kMenubarPage{34, "展示", "基础菜单", "基础菜单：垂直菜单 + 分组 + 父级子面板", build, nullptr, false, nullptr, 0, "Menu"};

static const bool kRegistered = []() {
    page::register_page(kMenubarPage);
    return true;
}();
