#include "../page_registry.h"

#include "ui/core/math.h"
#include "ui/gl/renderer.h"
#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_style.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"

namespace {

constexpr float kCW = 1000.f;

// 色块卡片：纯色圆角矩形（layui 官方色彩页的色板样式）
struct Swatch : ui::Widget {
    Swatch(float w, float h, const ui::Vec4& c) : fill(c) { set_bounds(0, 0, w, h); }
    void on_draw(ui::Renderer& renderer) override {
        renderer.draw_rounded_rect(bounds_, 4.f, fill);
        renderer.draw_rect_outline(bounds_, 1.f, ui::Vec4{0.85f, 0.85f, 0.85f, 1.f});
    }
    ui::Vec4 fill;
};

// 一张色卡：色块 + 名称 + HEX（纵向 VBox）
ui::VBox* card(PageHost& h, const char* name, const char* hexstr, float w = 150.f) {
    auto* v = new ui::VBox(0, 0, w, 0);
    v->set_padding(0.f);
    v->set_spacing(6.f);
    v->set_theme(h.theme);
    auto* sw = new Swatch(w - 20.f, 64.f, ui::layui::hex(hexstr));
    v->add_child(sw);
    auto* n = new ui::Label(0, 0, w - 20.f, 0, name);
    n->set_font(h.font); n->set_theme(h.theme); n->set_color(h.theme->text);
    v->add_child(n);
    auto* x = new ui::Label(0, 0, w - 20.f, 0, hexstr);
    x->set_font(h.font); x->set_theme(h.theme); x->set_color(h.theme->text_muted);
    v->add_child(x);
    return v;
}

ui::HBox* row(PageHost& h) {
    auto* r = new ui::HBox(0, 0, kCW, 0);
    r->set_padding(0.f);
    r->set_spacing(16.f);
    r->set_theme(h.theme);
    return r;
}

void para(PageHost& h, const char* t) {
    auto* l = new ui::Label(0, 0, kCW, 0, t);
    l->set_font(h.font);
    l->set_theme(h.theme);
    l->set_color(h.theme->text_secondary);
    l->set_wrap(true);
    h.content->add_child(l);
}

void build_color(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "layui 规范化的色彩体系：默认主题色、经典主题色与八种工具色。"
        "页面中的色值即框架实际取色来源（ui::layui::hex / layui-bg-*）。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- 主色 ----
    para(host, "主色（默认主题）与经典色：2.8+ 默认主题色为 #16baaa，经典主题色 #009688。");
    {
        auto* r = row(host);
        r->add_child(card(host, "默认主题色", "#16baaa"));
        r->add_child(card(host, "经典主题色", "#009688"));
        host.content->add_child(r);
    }

    // ---- 工具色 ----
    para(host, "工具色（layui-bg-* / layui-btn-* 同源）：用于按钮、徽章、边框等场景。");
    {
        struct C { const char* name; const char* hex; };
        static const C kTools[] = {
            {"赤", "#ff5722"}, {"橙", "#ffb800"}, {"绿", "#009688"},
            {"藏青", "#2f4056"}, {"蓝", "#1e9fff"}, {"紫", "#a233c6"},
            {"黑", "#393d49"}, {"灰", "#fafafa"},
        };
        auto* r = row(host);
        for (const C& c : kTools) r->add_child(card(host, c.name, c.hex));
        host.content->add_child(r);
    }

    // ---- 文字颜色 ----
    para(host, "文字颜色示例（layui-font-red/orange/green/cyan/blue/purple/black/gray）：");
    {
        auto* r = row(host);
        struct C { const char* cls; const char* name; };
        static const C kFonts[] = {
            {"red", "红色文字"}, {"orange", "橙色文字"}, {"green", "绿色文字"},
            {"cyan", "藏青文字"}, {"blue", "蓝色文字"}, {"purple", "紫色文字"},
            {"black", "黑色文字"}, {"gray", "灰色文字"},
        };
        for (const C& c : kFonts) {
            std::string cls = std::string("layui-bg-") + c.cls;
            auto* l = new ui::Label(0, 0, 110.f, 0, c.name);
            l->set_font(host.font);
            l->set_theme(host.theme);
            l->set_color(ui::layui::bg_color(cls.c_str()));
            r->add_child(l);
        }
        host.content->add_child(r);
    }

    // ---- 背景色 ----
    para(host, "背景色示例（layui-bg-*，白字前景）：");
    {
        auto* r = row(host);
        struct C { const char* cls; const char* name; };
        static const C kBgs[] = {
            {"red", "红底白字"}, {"orange", "橙底白字"}, {"green", "绿底白字"},
            {"cyan", "藏青底白字"}, {"blue", "蓝底白字"}, {"black", "黑底白字"},
        };
        for (const C& c : kBgs) {
            std::string cls = std::string("layui-bg-") + c.cls;
            auto* sw = new Swatch(120.f, 38.f, ui::layui::bg_fill(cls.c_str()));
            r->add_child(sw);
        }
        host.content->add_child(r);
    }

    // ---- 底部引用块 ----
    auto* bq2 = new ui::BlockQuote(kCW, 0,
        "更多浅色系（layui-bg-gray 等）与组件级配色（按钮/徽章/进度条主题），"
        "见对应组件页；颜色选择器交互见「交互 → 颜色选择器」。");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/color_page.cpp：克隆 layui 颜色文档\n"
            "\n"
            "#include \"ui/ui/layui_style.h\"\n"
            "\n"
            "ui::Vec4 accent   = ui::layui::hex(\"#16baaa\");  // 默认主题色\n"
            "ui::Vec4 classic  = ui::layui::hex(\"#009688\");  // 经典主题色\n"
            "ui::Vec4 red_fill = ui::layui::bg_fill(\"layui-bg-red\");\n"
            "ui::Vec4 red_text = ui::layui::bg_color(\"layui-bg-red\");\n"
            "\n"
            "// 控件着色统一走 layui 样式层，保证与 layui.css 取值一致");
    }
    if (host.help) {
        const char* lines[] = {
            "颜色 API 一览",
            "",
            "ui::layui 命名空间（layui_style.h）",
            "  hex(\"#16baaa\")：十六进制串 -> Vec4",
            "  bg_color(\"layui-bg-*\")：工具色前景（白字/灰字）",
            "  bg_fill(\"layui-bg-*\")：工具色背景填充",
            "",
            "内置工具色：red #ff5722 / orange #ffb800 / green #009688 /",
            "cyan #2f4056 / blue #1e9fff / purple #a233c6 / black #393d49 /",
            "gray #fafafa；默认主题色 #16baaa。",
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

static void build(PageHost& host) { build_color(host); }

static const PageDef kColorPage{9, "通用", "颜色", "layui 色彩体系：主题色/工具色/文字与背景色", build, nullptr, false, nullptr, 0, "Color"};

static const bool kRegistered = []() {
    page::register_page(kColorPage);
    return true;
}();
