#include "../page_registry.h"

#include "ui/core/math.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_style.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

constexpr float kCW = 1000.f;    // 预览内容宽（镜像 ws-content 文档列宽）
constexpr float kGap = 15.f;     // layui-col-space15 列间距
constexpr float kPadV = 20.f;    // 色块内边距（镜像 .ws-docs-color>div div padding:20px 10px）
constexpr float kPadH = 10.f;
constexpr float kLinePitch = 22.4f;  // line-height 1.6 × font-size 14

// 色块：纯色矩形 + 居中（或左对齐）文本行（镜像 docs 颜色页色板样式）
struct ColorBlock : ui::Widget {
    ColorBlock(float w, const ui::Vec4& fill, std::vector<std::string> lines,
               const ui::Vec4& text_color, float radius, bool center)
        : Widget(0, 0, w, 0),
          fill_(fill),
          lines_(std::move(lines)),
          text_color_(text_color),
          radius_(radius),
          center_(center) {
        set_bounds(0, 0, w, static_cast<float>(lines_.size()) * kLinePitch + 2.f * kPadV);
    }
    float preferred_height() const override { return bounds_.h; }
    void set_font(ui::Font* f) { font_ = f; }
    void on_draw(ui::Renderer& renderer) override {
        renderer.draw_rounded_rect(bounds_, radius_, fill_);
        if (!font_) return;
        const float lh = font_->line_height();
        float ty = bounds_.y + kPadV + (kLinePitch - lh) * 0.5f;
        for (const auto& line : lines_) {
            float tx = bounds_.x + kPadH;
            if (center_) tx = bounds_.x + (bounds_.w - font_->measure_text(line.c_str())) * 0.5f;
            font_->draw_text(renderer, line.c_str(), tx, ty, text_color_);
            ty += kLinePitch;
        }
    }
    ui::Vec4 fill_;
    std::vector<std::string> lines_;
    ui::Vec4 text_color_;
    float radius_;
    bool center_;
    ui::Font* font_ = nullptr;
};

// h2 小节标题（镜像文档 h2：大号加粗深字）
ui::Label* h2(PageHost& host, const char* text) {
    auto* l = new ui::Label(0, 0, kCW, 0, text);
    l->set_font(host.font);
    l->set_theme(host.theme);
    l->set_color(host.theme->text);
    l->set_text_scale(1.6f);
    l->set_bold(true);
    host.content->add_child(l);
    return l;
}

// 正文段落
void para(PageHost& host, const char* t) {
    auto* l = new ui::Label(0, 0, kCW, 0, t);
    l->set_font(host.font);
    l->set_theme(host.theme);
    l->set_color(host.theme->text_secondary);
    l->set_wrap(true);
    host.content->add_child(l);
}

// 一行色块容器（带列间距；gap=false 时镜像无 col-space15 的行；h 为色卡统一高度）
ui::HBox* color_row(PageHost& host, float gap = kGap, float h = 0.f) {
    auto* r = new ui::HBox(0, 0, kCW, h);
    r->set_padding(0.f);
    r->set_spacing(gap);
    r->set_theme(host.theme);
    host.content->add_child(r);
    return r;
}

// 白字双行色卡（hex + 名称），宽度按 n 等分 kCW
void add_card(ui::HBox* r, PageHost& host, int n, const char* hexstr, const char* name) {
    const float w = (kCW - kGap * (n - 1)) / n;
    r->add_child(new ColorBlock(w, ui::layui::hex(hexstr), {hexstr, name},
                                ui::Vec4{1, 1, 1, 1}, 3.f, true));
}

// 中色调单行块（radius 0、黑字，镜像 ws-docs-necolor；可指定白字/自定义标签）
void add_neutral(ui::HBox* r, float w, const char* fill_hex, const char* label,
                 bool white_text = false) {
    r->add_child(new ColorBlock(w, ui::layui::hex(fill_hex), {label},
                                white_text ? ui::Vec4{1, 1, 1, 1} : ui::Vec4{0.f, 0.f, 0.f, 1.f},
                                0.f, true));
}

// 七色调整宽横条（左对齐，镜像 ws-docs-bgcolor）
void add_bar(PageHost& host, const char* fill_hex, const char* text, bool dark_text = false) {
    host.content->add_child(new ColorBlock(
        kCW, ui::layui::hex(fill_hex), {text},
        dark_text ? ui::Vec4{0.2f, 0.2f, 0.2f, 1.f} : ui::Vec4{1, 1, 1, 1}, 3.f, false));
}

void build_color(PageHost& host) {
    // ---- 顶部引用块（镜像原文） ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "Layui 试图在清新与深沉之间寻求某种柔和，以不过分刺激大脑皮层的神经介质，反馈出经久耐看的微妙视图。"
        "您可以在我们精心调配的如下色系中合理搭配，减少不必要的违和感，从而使您的 Web 页面看上去尽可能融洽。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- 基色调（3 等分） ----
    h2(host, "基色调");
    {
        auto* r = color_row(host, kGap, 84.8f);
        add_card(r, host, 3, "#16baaa", "蓝绿色");
        add_card(r, host, 3, "#16b777", "清新绿");
        add_card(r, host, 3, "#1e9fff", "经典蓝");
    }
    para(host,
        "Layui 选取以象征清新与包容的「蓝绿色」作为主色调，它介于蓝与绿之间，亦可称之为「青色」，"
        "所谓青者，取之于蓝而深于绿，此间包罗万象，跨越时空。这也是我们对技术创作与人生哲学之间的一点思考。");

    // ---- 辅色调（4 等分） ----
    h2(host, "辅色调");
    {
        auto* r = color_row(host, kGap, 84.8f);
        add_card(r, host, 4, "#ff5722", "错误 - Danger");
        add_card(r, host, 4, "#ffb800", "警示 - Warning");
        add_card(r, host, 4, "#16b777", "成功 - Success");
        add_card(r, host, 4, "#31bdec", "引导 - Info");
    }
    para(host,
        "页面中同样也少不了辅助色彩的点缀，以对不同含义的内容加以区分。Layui 从暖色系（红/橙）和冷色系（绿/蓝）中，"
        "各自调取了我们认为最具代表性的颜色值，它们通常在不同的场景中发挥着不同的作用。");

    // ---- 中色调（radius 0 黑字；md6/md6 + md2×6 + md6/md6 三行铺满，无列间距） ----
    h2(host, "中色调");
    {
        const float half = kCW / 2.f;
        const float sixth = kCW / 6.f;
        auto* r1 = color_row(host, 0.f, 62.4f);
        add_neutral(r1, half, "#fafafa", "#fafafa");
        add_neutral(r1, half, "#f6f6f6", "#f7f7f7");  // 镜像原文即如此标注
        auto* r2 = color_row(host, 0.f, 62.4f);
        add_neutral(r2, sixth, "#eeeeee", "#eeeeee");
        add_neutral(r2, sixth, "#e2e2e2", "#e2e2e2");
        add_neutral(r2, sixth, "#dddddd", "#dddddd");
        add_neutral(r2, sixth, "#d2d2d2", "#d2d2d2");
        add_neutral(r2, sixth, "#cccccc", "#cccccc");
        add_neutral(r2, sixth, "#c2c2c2", "#c2c2c2");
        auto* r3 = color_row(host, 0.f, 62.4f);
        add_neutral(r3, half, "#2f363c", "#2f363c");  // 镜像 necolor 规则下黑字
        add_neutral(r3, half, "#23292e", "#23292e", true);
    }
    para(host,
        "中性的颜色是页面中的重要组成部分，也被称之为无彩色系，一般用于文本、背景和边框等。"
        "它能对由彩色进行视觉冲淡，以达到和谐自然的效果。Layui 认为恰当的灰度调配代表着极简，"
        "这是一种神奇的色系，是视觉疲劳的栖息，低调而优雅，且永不过时。");

    // ---- 七色调（整宽横条依次堆叠） ----
    h2(host, "七色调");
    add_bar(host, "#ff5722", "#ff5722 - 红");
    add_bar(host, "#ffb800", "#ffb800 - 橙");
    add_bar(host, "#16baaa", "#16baaa - 绿");
    add_bar(host, "#1e9fff", "#1e9fff - 蓝");
    add_bar(host, "#a233c6", "#a233c6 - 紫");
    add_bar(host, "#2f363c", "#2f363c - 深");
    add_bar(host, "#fafafa", "#fafafa - 浅", true);
    para(host,
        "Layui 的色调基础，除了前面提到的主辅中色之外，还建立在以上罗列的七种颜色之上。"
        "你同样也可以遵循我们提供的色调方案去扩展你的页面元素，使得能与 Layui 的主题风格无缝融合。");

    // ---- 标语 ----
    h2(host, "标语");
    auto* bq2 = new ui::BlockQuote(kCW, 0, "不热衷于视觉设计的开发者不是一个好作家。 —— 贤心");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/color_page.cpp：一比一克隆 layui 文档「通用 > 颜色」\n"
            "\n"
            "#include \"ui/ui/layui_style.h\"\n"
            "\n"
            "// 基色调\n"
            "ui::Vec4 primary   = ui::layui::hex(\"#16baaa\");  // 蓝绿色\n"
            "ui::Vec4 fresh     = ui::layui::hex(\"#16b777\");  // 清新绿\n"
            "ui::Vec4 classic   = ui::layui::hex(\"#1e9fff\");  // 经典蓝\n"
            "\n"
            "// 辅色调\n"
            "ui::Vec4 danger  = ui::layui::hex(\"#ff5722\");\n"
            "ui::Vec4 warning = ui::layui::hex(\"#ffb800\");\n"
            "ui::Vec4 success = ui::layui::hex(\"#16b777\");\n"
            "ui::Vec4 info    = ui::layui::hex(\"#31bdec\");\n"
            "\n"
            "// 中性灰阶 #fafafa -> #c2c2c2 与深色 #2f363c / #23292e\n"
            "// 七色调 = 红/橙/绿/蓝/紫/深/浅（layui-bg-* 同源）");
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
            "基色调：#16baaa 蓝绿 / #16b777 清新绿 / #1e9fff 经典蓝",
            "辅色调：#ff5722 Danger / #ffb800 Warning /",
            "        #16b777 Success / #31bdec Info",
            "中色调：#fafafa~#c2c2c2 灰阶 + #2f363c / #23292e 深色",
            "七色调：红橙绿蓝紫深浅（layui-bg-red/orange/green/blue/",
            "        purple/black/gray）。",
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

static const PageDef kColorPage{9, "通用", "颜色", "layui 官方色彩页克隆：基色调/辅色调/中色调/七色调/标语", build, nullptr, false, nullptr, 0, "Color"};

static const bool kRegistered = []() {
    page::register_page(kColorPage);
    return true;
}();
