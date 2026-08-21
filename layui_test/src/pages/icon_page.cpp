#include "../page_registry.h"

#include "icon_catalog.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/card.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/icon.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"

namespace {

constexpr float kCW = 1000.f;


// 单元格：镜像 docs-icon 卡片（白底 1px 描边，四行＝字形/中文名/码点/类名）
ui::Card* cell(PageHost& h, const IconItem& it) {
    auto* card = new ui::Card(0, 0, 156.f, 0);
    card->set_font(h.font);
    card->set_theme(h.theme);
    card->set_spacing(5.f);
    card->set_outline(ui::Vec4{0.91f, 0.91f, 0.91f, 1.f}, 1.f);  // 镜像 #e8e8e8 描边
    auto* ic = new ui::Icon(0, 0, 26.f, 26.f, it.name);
    ic->set_font(h.font);
    ic->set_theme(h.theme);
    card->add_child(ic);
    auto* cn = new ui::Label(0, 0, 126.f, 0, it.cn);
    cn->set_font(h.font);
    cn->set_theme(h.theme);
    cn->set_color(h.theme->text);
    cn->set_align(1);  // 镜像 docs-icon-name 水平居中
    card->add_child(cn);
    std::string code = std::string("&#x") + it.cp + ";";
    auto* cp = new ui::Label(0, 0, 126.f, 0, code.c_str());
    cp->set_font(h.font);
    cp->set_theme(h.theme);
    cp->set_color(h.theme->text_muted);
    cp->set_align(1);
    cp->set_text_scale(0.78f);  // 镜像小字灰阶（docs-icon-code）
    card->add_child(cp);
    std::string cls = std::string("layui-icon-") + it.name;
    auto* l = new ui::Label(0, 0, 126.f, 0, cls.c_str());
    l->set_font(h.font);
    l->set_theme(h.theme);
    l->set_color(h.theme->text_muted);
    l->set_align(1);
    l->set_text_scale(0.78f);  // 镜像小字灰阶（docs-icon-fontclass）
    card->add_child(l);
    return card;
}

void build_icon(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "layui-icon 图标库（195 项＝镜像 192 + layim 3）：字形来自 iconfont.ttf 码点，"
        "由 ui::Icon + ui::layui_icon 码点表渲染。用法 class=\"layui-icon layui-icon-xxx\"。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- 彩色图标 ----
    {
        auto* title = new ui::FieldsetTitle(kCW, 26, "彩色图标（set_color）");
        title->set_font(host.font);
        title->set_theme(host.theme);
        host.content->add_child(title);

        auto* r = new ui::HBox(0, 0, kCW, 30.f);
        r->set_padding(0.f);
        r->set_spacing(28.f);
        r->set_theme(host.theme);
        struct C { const char* name; ui::Vec4 color; };
        static const C kColored[] = {
            {"heart-fill", {1.f, 0.34f, 0.13f, 1.f}},
            {"star-fill", {1.f, 0.72f, 0.f, 1.f}},
            {"like", {0.09f, 0.72f, 0.47f, 1.f}},
            {"notice", {0.12f, 0.62f, 1.f, 1.f}},
            {"diamond", {0.64f, 0.20f, 0.78f, 1.f}},
            {"fire", {0.18f, 0.25f, 0.34f, 1.f}},
        };
        for (const C& c : kColored) {
            if (!ui::layui_icon::from_class(c.name)) continue;  // 表中无此名则跳过
            auto* ic = new ui::Icon(0, 0, 24.f, 24.f, c.name);
            ic->set_font(host.font);
            ic->set_color(c.color);
            r->add_child(ic);
        }
        host.content->add_child(r);
    }

    // ---- 按钮图标（镜像 h2#icon 两个 layui-btn-container） ----
    {
        auto* title = new ui::FieldsetTitle(kCW, 26, "按钮图标");
        title->set_font(host.font);
        title->set_theme(host.theme);
        host.content->add_child(title);

        // 容器一：默认尺寸——「按钮 ▾」（文字在前图标在后）+ left/right/edit/share 纯图标
        {
            auto* r = new ui::HBox(0, 0, kCW, 38.f);
            r->set_padding(0.f);
            r->set_spacing(8.f);
            r->set_theme(host.theme);

            auto* b1 = new ui::Button(0, 0, 76.f, 38.f, "按钮");
            b1->set_font(host.font);
            b1->set_theme(host.theme);
            b1->set_icon("down");
            b1->set_icon_right(true);  // 镜像：文字在前、down 图标在后
            r->add_child(b1);

            static const char* const kDef[] = {"left", "right", "edit", "share"};
            for (const char* name : kDef) {
                auto* b = new ui::Button(0, 0, 44.f, 38.f, "");
                b->set_font(host.font);
                b->set_theme(host.theme);
                b->set_icon(name);
                r->add_child(b);
            }
            host.content->add_child(r);
        }

        // 容器二（单行 9 个，同镜像一个 container）：
        // sm primary×5（left/right/edit/delete/share）+ sm disabled delete
        // + sm normal left + sm warm right + sm danger edit
        {
            auto* r = new ui::HBox(0, 0, kCW, 30.f);
            r->set_padding(0.f);
            r->set_spacing(8.f);
            r->set_theme(host.theme);
            struct I { const char* icon; const char* cls; };
            static const I kSm[] = {
                {"left",   "layui-btn layui-btn-sm layui-btn-primary"},
                {"right",  "layui-btn layui-btn-sm layui-btn-primary"},
                {"edit",   "layui-btn layui-btn-sm layui-btn-primary"},
                {"delete", "layui-btn layui-btn-sm layui-btn-primary"},
                {"share",  "layui-btn layui-btn-sm layui-btn-primary"},
                {"delete", "layui-btn layui-btn-sm layui-btn-disabled"},
                {"left",   "layui-btn layui-btn-sm layui-btn-normal"},
                {"right",  "layui-btn layui-btn-sm layui-btn-warm"},
                {"edit",   "layui-btn layui-btn-sm layui-btn-danger"},
            };
            for (const I& i : kSm) {
                auto* b = new ui::Button(0, 0, 34.f, 30.f, "");
                b->set_font(host.font);
                b->set_theme(host.theme);
                b->set_icon(i.icon);
                b->add_class(i.cls);
                r->add_child(b);
            }
            host.content->add_child(r);
        }
    }

    // ---- 全部图标（镜像「图标列表」一比一：6 列描边卡片 × 4 行内容）----
    {
        auto* title = new ui::FieldsetTitle(kCW, 26, "全部图标");
        title->set_font(host.font);
        title->set_theme(host.theme);
        host.content->add_child(title);

        constexpr int kPerRow = 6;
        for (int base = 0; base < kIconCount; base += kPerRow) {
            auto* r = new ui::HBox(0, 0, kCW, 0);  // h=0 自适应（HBox::preferred_height）
            r->set_padding(0.f);
            r->set_spacing(8.f);
            r->set_theme(host.theme);
            for (int i = base; i < base + kPerRow && i < kIconCount; ++i)
                r->add_child(cell(host, kIcons[i]));
            host.content->add_child(r);
        }
    }

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/icon_page.cpp：克隆 layui 图标文档\n"
            "\n"
            "#include \"ui/ui/icon.h\"\n"
            "\n"
            "ui::Icon* ic = new ui::Icon(0, 0, 30, 30, \"star\");\n"
            "ic->set_font(host.font);   // iconfont.ttf 已并入 Font 私用区\n"
            "ic->set_color(theme.accent);\n"
            "\n"
            "// 码点查询：ui::layui_icon::from_class(\"star\") -> 0xE67A\n"
            "// 字体加载：Font::load_icon_font(\"iconfont.ttf\", 18.f)");
    }
    if (host.help) {
        const char* lines[] = {
            "图标 API 一览",
            "",
            "ui::Icon：单个私用区图标码点控件",
            "  构造 Icon(x, y, w, h, \"star\")：名字不含 layui-icon- 前缀",
            "  set_icon(name)：换图标；找不到则清空",
            "  set_color(Vec4)：着色（默认主题文字色）",
            "",
            "ui::layui_icon（layui_icons.h，自动生成勿手改）",
            "  from_class(name)：类名 -> 码点，找不到返回 0",
            "  常量表：ic_404 / about / star ... 共 195 项",
            "",
            "字体：exe 目录 iconfont.ttf 经 Font::load_icon_font 并入同一 Font，",
            "与中文文本共用（POST_BUILD 拷贝，UI-048 exe 目录回退）。",
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

static void build(PageHost& host) { build_icon(host); }

static const PageDef kIconPage{11, "通用", "图标", "layui-icon 图标库：按钮图标 + 彩色示例 + 195 项全量网格", build, nullptr, false, nullptr, 0, "Icon"};

static const bool kRegistered = []() {
    page::register_page(kIconPage);
    return true;
}();
