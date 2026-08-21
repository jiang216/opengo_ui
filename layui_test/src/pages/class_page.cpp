#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_style.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

constexpr float kCW = 1000.f;

void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

// 说明行（次级文字色，自动换行）
void para_line(PageHost& host, const char* text) {
    auto* l = new ui::Label(0, 0, kCW, 0, text);
    l->set_font(host.font);
    l->set_theme(host.theme);
    l->set_color(host.theme->text_secondary);
    l->set_wrap(true);
    host.content->add_child(l);
}

void build_class(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "公共类：一套与组件无关的工具类，覆盖字号、字色、字重、背景、边框等基础样式，"
        "任意元素叠加即可生效。本页逐类演示（样式值取自框架 layui 样式层）。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- 字体大小 ----
    field_title(host, "字体大小 layui-font-12/14/16/18/20/22");
    para_line(host,
        "六档字号类；框架 Font 按 18px 烘培（2x 过采样图集），"
        "Label::set_text_scale 按 px/18 倍率缩放渲染，≤2x（36px）内清晰。");
    {
        auto* r = new ui::HBox(0, 0, kCW, 44.f);
        r->set_padding(0.f);
        r->set_spacing(24.f);
        r->set_theme(host.theme);
        static const int kSizes[] = {12, 14, 16, 18, 20, 22};
        for (int px : kSizes) {
            std::string txt = std::string("layui-font-") + std::to_string(px) + " 示例";
            auto* l = new ui::Label(0, 0, 150.f, 0, txt.c_str());
            l->set_font(host.font);
            l->set_theme(host.theme);
            l->set_color(host.theme->text);
            l->set_text_scale((float)px / 18.f);
            r->add_child(l);
        }
        host.content->add_child(r);
    }

    // ---- 字体颜色 ----
    field_title(host, "字体颜色 layui-font-red/orange/green/cyan/blue/purple/black/gray");
    {
        auto* r = new ui::HBox(0, 0, kCW, 30.f);
        r->set_padding(0.f);
        r->set_spacing(18.f);
        r->set_theme(host.theme);
        const char* names[] = {"red", "orange", "green", "cyan", "blue", "purple", "black", "gray"};
        for (const char* n : names) {
            std::string cls = std::string("layui-font-") + n;
            auto* l = new ui::Label(0, 0, 112.f, 0, cls.c_str());
            l->set_font(host.font);
            l->set_theme(host.theme);
            l->set_color(ui::layui::font_color(cls.c_str()));
            r->add_child(l);
        }
        host.content->add_child(r);
    }

    // ---- 字重 ----
    field_title(host, "字体加粗 layui-font-bold");
    {
        auto* r = new ui::HBox(0, 0, kCW, 34.f);
        r->set_padding(0.f);
        r->set_spacing(28.f);
        r->set_theme(host.theme);
        auto* normal = new ui::Label(0, 0, 260.f, 0, "常规字重（默认）");
        normal->set_font(host.font);
        normal->set_theme(host.theme);
        normal->set_color(host.theme->text);
        r->add_child(normal);
        auto* bold = new ui::Label(0, 0, 300.f, 0, "layui-font-bold —— 加粗文本示例");
        bold->set_font(host.font);
        bold->set_theme(host.theme);
        bold->set_color(host.theme->text);
        bold->set_bold(true);
        r->add_child(bold);
        host.content->add_child(r);
    }

    // ---- 背景色 ----
    field_title(host, "背景色 layui-bg-red/orange/green/cyan/blue/purple/black/gray");
    {
        auto* r = new ui::HBox(0, 0, kCW, 38.f);
        r->set_padding(0.f);
        r->set_spacing(10.f);
        r->set_theme(host.theme);
        const char* names[] = {"red", "orange", "green", "cyan", "blue", "purple", "black", "gray"};
        for (const char* n : names) {
            std::string cls = std::string("layui-bg-") + n;
            auto* b = new ui::Button(0, 0, 110.f, 36.f, cls.c_str());
            b->set_font(host.font);
            b->set_theme(host.theme);
            b->add_class(cls.c_str());
            b->set_enabled(false);  // 纯展示，不响应点击
            r->add_child(b);
        }
        host.content->add_child(r);
    }

    // ---- 边框 ----
    field_title(host, "边框 layui-border / layui-border-red/orange/green/cyan/blue/purple/black");
    {
        auto* r = new ui::HBox(0, 0, kCW, 44.f);
        r->set_padding(0.f);
        r->set_spacing(10.f);
        r->set_theme(host.theme);
        const char* names[] = {"", "-red", "-orange", "-green", "-cyan", "-blue", "-purple", "-black"};
        for (const char* n : names) {
            std::string cls = std::string("layui-btn layui-btn-primary layui-border") + n;
            std::string show = n[0] ? n + 1 : std::string("默认");
            auto* b = new ui::Button(0, 0, 110.f, 38.f, ("边框" + show).c_str());
            b->set_font(host.font);
            b->set_theme(host.theme);
            b->add_class(cls.c_str());
            r->add_child(b);
        }
        host.content->add_child(r);
    }

    // ---- 底部引用块 ----
    auto* bq2 = new ui::BlockQuote(kCW, 0,
        "圆角（layui-circle）、内联块（layui-inline）等其余工具类随对应组件呈现；"
        "色值来源见「通用 → 颜色」。");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/class_page.cpp：克隆 layui 公共类文档\n"
            "\n"
            "// 字号缩放（Font 基准 18px 烘培，图集 2x 过采样）\n"
            "label->set_text_scale(22.f / 18.f);   // layui-font-22\n"
            "\n"
            "// 仿粗体（字形 x 错位叠加一次）\n"
            "label->set_bold(true);\n"
            "\n"
            "// 工具色着色（前景/背景）\n"
            "label->set_color(ui::layui::bg_color(\"layui-bg-blue\"));\n"
            "button->add_class(\"layui-bg-green\");\n"
            "\n"
            "// 边框按钮\n"
            "button->add_class(\"layui-btn layui-btn-primary layui-border-red\");");
    }
    if (host.help) {
        const char* lines[] = {
            "公共类清单",
            "",
            "字号：layui-font-12 / 14 / 16 / 18 / 20 / 22",
            "      映射 Label::set_text_scale(px/18)；≤2x 清晰。",
            "字色：layui-font-{red orange green cyan blue purple black gray}",
            "字重：layui-font-bold → Label::set_bold(true) 仿粗体（x 错位叠加）。",
            "背景：layui-bg-{red orange green cyan blue purple black gray}",
            "边框：layui-border 与 layui-border-{red orange green cyan blue purple black}",
            "其他：layui-circle 圆形、layui-inline 内联块",
            "",
            "框架映射：类名 -> 样式令牌统一在 ui::layui 样式层解析。",
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

static void build(PageHost& host) { build_class(host); }

static const PageDef kClassPage{39, "展示", "公共类", "公共类：字号/字色/字重/背景/边框工具类", build, nullptr, false, nullptr, 0, "Class"};

static const bool kRegistered = []() {
    page::register_page(kClassPage);
    return true;
}();
