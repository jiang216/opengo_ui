#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/circle.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hline.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

void build_auxiliar(PageHost& host) {
    auto field_title = [&](const char* text) {
        ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
        t->set_font(host.font);
        t->set_theme(host.theme);
        host.content->add_child(t);
    };

    field_title("引用区块 - 默认风格");
    {
        ui::BlockQuote* q = new ui::BlockQuote(800, 0, "这个貌似不用多介绍，因为你已经在太多的地方都看到");
        q->set_font(host.font);
        q->set_theme(host.theme);
        host.content->add_child(q);
    }

    field_title("引用区块 - 一般风格");
    {
        ui::BlockQuote* q = new ui::BlockQuote(800, 0, "猿强，则国强。国强，则猿更强！\n——孟子的崇拜者", true);
        q->set_font(host.font);
        q->set_theme(host.theme);
        host.content->add_child(q);
    }

    field_title("字段集区块");
    {
        ui::Fieldset* fs = new ui::Fieldset(800, 0, "爱好");
        fs->set_font(host.font);
        fs->set_theme(host.theme);
        ui::Label* body = new ui::Label(0, 0, 700, 0, "你可以在这里放任何内容，比如表单神马的");
        body->set_font(host.font);
        body->set_theme(host.theme);
        body->set_color(host.theme->text_secondary);
        fs->add_child(body);
        host.content->add_child(fs);
    }

    field_title("带标题的横线");
    {
        ui::Label* text = new ui::Label(0, 0, 800, 0, "内容区域");
        text->set_font(host.font);
        text->set_theme(host.theme);
        text->set_color(host.theme->text);
        host.content->add_child(text);
    }

    field_title("分割线");
    {
        ui::Label* d1 = new ui::Label(0, 0, 800, 0, "默认分割线");
        d1->set_font(host.font);
        d1->set_theme(host.theme);
        d1->set_color(host.theme->text);
        host.content->add_child(d1);
        host.content->add_child(new ui::HLine(800, 2, ui::Vec4{0.93f, 0.93f, 0.93f, 1.0f}));

        struct ColoredLine {
            const char* name;
            ui::Vec4 color;
        };
        const ColoredLine lines[] = {
            {"赤色分割线", {1.0f, 0.34f, 0.13f, 1.0f}},
            {"橙色分割线", {1.0f, 0.72f, 0.0f, 1.0f}},
            {"墨绿分割线", {0.0f, 0.59f, 0.53f, 1.0f}},
            {"青色分割线", {0.37f, 0.72f, 0.47f, 1.0f}},
            {"蓝色分割线", {0.12f, 0.62f, 1.0f, 1.0f}},
            {"黑色分割线", {0.2f, 0.2f, 0.2f, 1.0f}},
        };
        for (const ColoredLine& l : lines) {
            ui::Label* name = new ui::Label(0, 0, 800, 0, l.name);
            name->set_font(host.font);
            name->set_theme(host.theme);
            name->set_color(host.theme->text);
            host.content->add_child(name);
            host.content->add_child(new ui::HLine(800, 2, l.color));
        }
    }

    field_title("纯圆角");
    {
        ui::Circle* c = new ui::Circle(110, "circle");
        c->set_font(host.font);
        c->set_color(ui::Vec4{0.09f, 0.72f, 0.47f, 1.0f});
        host.content->add_child(c);
    }

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/auxiliar_page.cpp（节选）：克隆 layui auxiliar.html\n"
            "\n"
            "// 引用区块：默认灰底 / nm 无底，左侧 5px 绿色竖条（ui 库组件）\n"
            "ui::BlockQuote* q = new ui::BlockQuote(800, 0, \"猿强，则国强\", true);\n"
            "q->set_font(host.font);\n"
            "q->set_theme(host.theme);\n"
            "host.content->add_child(q);\n"
            "\n"
            "// 字段集：上下边框 + legend\n"
            "ui::Fieldset* fs = new ui::Fieldset(800, 0, \"爱好\");\n"
            "fs->add_child(body_label);\n"
            "host.content->add_child(fs);\n"
            "\n"
            "// 分割线 / 带标题横线 / 纯圆角\n"
            "host.content->add_child(new ui::FieldsetTitle(800, 26, \"分割线\"));\n"
            "host.content->add_child(new ui::HLine(800, 2, {1.0f, 0.34f, 0.13f, 1.0f}));\n"
            "host.content->add_child(new ui::Circle(110, \"circle\"));");
    }
    if (host.help) {
        const char* lines[] = {
            "辅助性元素（ui 库组件）",
            "  BlockQuote：引用区块（默认灰底 / nm 无底）",
            "  Fieldset / FieldsetTitle：字段集与带标题横线",
            "  HLine：彩色分割线（红/橙/墨绿/青/蓝/黑）",
            "  Circle：纯圆角圆形底 + 居中文本",
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
    build_auxiliar(host);
}

static const PageDef kAuxiliarPage{37, "展示", "辅助元素", "引用区块/字段集/分割线/圆角", build, nullptr, false, nullptr, 0, "Blockquote / Fieldset / hr"};

static const bool kRegistered = []() {
    page::register_page(kAuxiliarPage);
    return true;
}();
