#include "../page_registry.h"

#include "ui/ui/badge.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/navbar.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/tabs.h"
#include "ui/ui/theme.h"

namespace {

// 小节标题：带 legend 横线（layui-elem-field layui-field-title）
void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

void style_badge(ui::Badge* b, PageHost& host) {
    b->set_font(host.font);
    b->set_theme(host.theme);
    b->set_bounds(0, 0, b->preferred_width(), 18.f);
}

// 灰底容器（layui-bg-gray #fafafa，承载徽章行）
ui::HBox* gray_box(PageHost& host, float h) {
    ui::HBox* box = new ui::HBox(0, 0, 800, h);
    box->set_fill(ui::Vec4{0.98f, 0.98f, 0.98f, 1.0f});
    box->set_padding(15.f);
    box->set_spacing(12.f);
    box->set_theme(host.theme);
    host.content->add_child(box);
    return box;
}

// 带行标签的徽章行：左侧灰字行名 + 徽章（行高足够容纳徽章 + 上下留白）
ui::HBox* labeled_row(PageHost& host, const char* title, float h = 48.f) {
    ui::HBox* box = gray_box(host, h);
    ui::Label* t = new ui::Label(0, 0, 200, h, title);
    t->set_font(host.font);
    t->set_theme(host.theme);
    t->set_color(host.theme->text_secondary);
    box->add_child(t);
    return box;
}

ui::Badge* add_badge(ui::HBox* row, PageHost& host, const char* text, const char* cls = nullptr) {
    ui::Badge* b = new ui::Badge(0, 0, text);
    style_badge(b, host);
    if (cls) b->add_class(cls);
    row->add_child(b);
    return b;
}

// 区块1：小徽章、大家族（灰底，三行：小圆点 / 常规弧形 / 边框）
void build_family(PageHost& host) {
    field_title(host, "小徽章、大家族");
    {
        const char* cols[] = {nullptr, "layui-bg-orange", "layui-bg-green", "layui-bg-cyan",
                              "layui-bg-blue", "layui-bg-black", "layui-bg-gray"};
        ui::HBox* row = labeled_row(host, "小圆点：");
        for (const char* c : cols) {
            std::string cls = c ? std::string("layui-badge-dot ") + c : std::string("layui-badge-dot");
            add_badge(row, host, "", cls.c_str());
        }
    }
    {
        struct Arc {
            const char* text;
            const char* cls;
        };
        const Arc arcos[] = {
            {"6", nullptr}, {"99", nullptr}, {"61728", nullptr}, {"赤", nullptr},
            {"橙", "layui-bg-orange"}, {"绿", "layui-bg-green"}, {"青", "layui-bg-cyan"},
            {"蓝", "layui-bg-blue"}, {"黑", "layui-bg-black"}, {"灰", "layui-bg-gray"},
        };
        ui::HBox* row = labeled_row(host, "常规弧形徽章：");
        for (const Arc& a : arcos) add_badge(row, host, a.text, a.cls);
    }
    {
        ui::HBox* row = labeled_row(host, "边框徽章：");
        add_badge(row, host, "6", "layui-badge-rim");
        add_badge(row, host, "Hot", "layui-badge-rim");
    }

    host.content->add_child(new ui::Label(0, 0, 800, 4, ""));
}

// 区块2 行内 label（左对齐文本 helper）
void note(PageHost& host, const char* text) {
    ui::Label* l = new ui::Label(0, 0, 800, 0, text);
    l->set_font(host.font);
    l->set_theme(host.theme);
    l->set_color(host.theme->text_secondary);
    host.content->add_child(l);
}

// 区块2：与其它元素的搭配
void build_combo(PageHost& host) {
    field_title(host, "与其它元素的搭配");

    // 按钮 + 内嵌徽章（layui-badge 内嵌在按钮文本右缘）
    {
        ui::HBox* row = new ui::HBox(0, 0, 800, 44.f);
        row->set_padding(0.f);
        row->set_spacing(15.f);
        row->set_theme(host.theme);
        host.content->add_child(row);

        ui::Button* b1 = new ui::Button(0, 0, 130.f, 30.f, "查看消息");
        b1->set_font(host.font);
        b1->set_theme(host.theme);
        b1->add_class("layui-btn");
        b1->set_badge("1");           // layui-badge layui-bg-gray 灰底
        b1->add_class("layui-bg-gray");
        row->add_child(b1);

        ui::Button* b2 = new ui::Button(0, 0, 100.f, 30.f, "动态");
        b2->set_font(host.font);
        b2->set_theme(host.theme);
        b2->add_class("layui-btn");
        b2->set_dot_badge();          // layui-badge-dot layui-bg-orange 橙点
        b2->add_class("layui-bg-orange");
        row->add_child(b2);
    }
    note(host, "小Tips:设置导航靠右对齐（或居中对齐）非常简单。");

    // 导航栏 + 徽章（NavBar set_badge / set_dot_badge，红底）
    {
        ui::Label* ntitle = new ui::Label(0, 0, 800, 0, "导航装饰：");
        ntitle->set_font(host.font);
        ntitle->set_theme(host.theme);
        ntitle->set_color(host.theme->text_secondary);
        host.content->add_child(ntitle);

        ui::NavBar* nav = new ui::NavBar(0, 0, 800, 40.f);
        nav->set_font(host.font);
        nav->set_theme(host.theme);
        nav->add_item("控制台");
        nav->set_badge("9");
        nav->add_item("个人中心");
        nav->set_dot_badge();
        host.content->add_child(nav);
    }
    note(host, "");

    // 选项卡 + 徽章（Tabs set_badge / set_dot_badge，标题内嵌）
    {
        ui::Tabs* tabs = new ui::Tabs(0, 0, 800, 100.f);
        tabs->set_font(host.font);
        tabs->set_theme(host.theme);
        tabs->add_tab("网站设置");
        tabs->add_tab("用户管理", "用户管理内容");
        tabs->set_dot_badge();
        tabs->add_tab("最新邮件", "最新邮件内容");
        tabs->set_badge("99+");
        host.content->add_child(tabs);
    }

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/badge_page.cpp：克隆 layui badge.html 全部两组 demo\n"
            "\n"
            "// Badge 类名：layui-badge 默认红 / layui-badge-dot 圆点 / layui-badge-rim 描边\n"
            "//           叠加 layui-bg-red/orange/green/cyan/blue/purple/black/gray 换底\n"
            "ui::Badge* b = new ui::Badge(0, 0, \"6\");\n"
            "b->set_bounds(0, 0, b->preferred_width(), 18.f);\n"
            "b->add_class(\"layui-bg-green\");\n"
            "row->add_child(b);\n"
            "\n"
            "// 按钮内嵌徽章：Button set_badge / set_dot_badge（叠加 layui-bg-* 换底）\n"
            "btn->add_class(\"layui-btn\");\n"
            "btn->set_badge(\"1\"); btn->add_class(\"layui-bg-gray\");   // 查看消息\n"
            "btn->set_dot_badge(); btn->add_class(\"layui-bg-orange\");  // 动态\n"
            "\n"
            "// 导航栏内嵌徽章：NavBar set_badge / set_dot_badge（红底）\n"
            "nav->add_item(\"控制台\"); nav->set_badge(\"9\");\n"
            "nav->add_item(\"个人中心\"); nav->set_dot_badge();\n"
            "\n"
            "// 选项卡标题内嵌徽章：Tabs set_badge / set_dot_badge\n"
            "tabs->add_tab(\"用户管理\"); tabs->set_dot_badge();\n"
            "tabs->add_tab(\"最新邮件\", \"...\"); tabs->set_badge(\"99+\");");
    }
    if (host.help) {
        const char* lines[] = {
            "徽章（Badge）API",
            "  Badge(x, y, text)：小徽标，默认红底白字",
            "  preferred_width()：内容自适应宽",
            "类名：",
            "  layui-badge / layui-badge-dot（8px 圆点）/ layui-badge-rim（描边）",
            "  layui-bg-red/orange/green/cyan/blue/black/gray：叠加换底色",
            "结合：",
            "  Button set_badge / set_dot_badge：按钮内嵌徽章",
            "  NavBar set_badge / set_dot_badge：导航项内嵌徽章",
            "  Tabs set_badge / set_dot_badge：选项卡标题内嵌徽章",
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

void build_badges(PageHost& host) {
    build_family(host);
    build_combo(host);
}

}  // namespace

static void build(PageHost& host) {
    build_badges(host);
}

static const PageDef kBadgePage{36, "展示", "徽章", "徽章：弧形/圆点/描边 + 结合按钮导航选项卡", build, nullptr, false, nullptr, 0, "Badge"};

static const bool kRegistered = []() {
    page::register_page(kBadgePage);
    return true;
}();