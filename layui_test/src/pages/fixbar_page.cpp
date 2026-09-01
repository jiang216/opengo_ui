#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"

namespace {

constexpr float kCW = 1000.f;

void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

void build_fixbar(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "固定条 fixbar：固定在页面右下角的快捷工具组（返回顶部、客服、二维码等）。"
        "本页已把工具组锚定到预览区右下角（ScrollBox::add_pinned，不随内容滚动），"
        "内容下滚超过一屏后出现，点击「返回顶部」真实复位滚动。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- Fieldset 1: 构成 ----
    field_title(host, "固定条构成（layui-fixbar）");
    {
        // 竖排工具组：与 layui fixbar 右下角样式一致（48px 方块 + 分隔）
        auto* bar = new ui::VBox(0, 0, 52.f, 0);
        bar->set_padding(4.f);
        bar->set_spacing(6.f);
        bar->set_theme(host.theme);

        struct Item { const char* icon; };
        static const Item kItems[] = {
            {"top"},
            {"service"},
            {"email"},
        };
        for (const Item& it : kItems) {
            auto* cell = new ui::Button(0, 0, 44.f, 44.f, "");
            cell->set_font(host.font);
            cell->set_theme(host.theme);
            cell->add_class("layui-btn layui-btn-primary");
            cell->set_icon(it.icon);
            bar->add_child(cell);
        }
        host.content->add_child(bar);
    }

    // ---- 视口级锚定：预览区右下角常驻工具组（UI-110 add_pinned）----
    {
        auto* bar = new ui::VBox(0, 0, 52.f, 0);
        bar->set_padding(4.f);
        bar->set_spacing(6.f);
        bar->set_theme(host.theme);
        auto* top = new ui::Button(0, 0, 44.f, 44.f, "");
        top->set_font(host.font);
        top->set_theme(host.theme);
        top->add_class("layui-btn layui-btn-primary");
        top->set_icon("top");
        top->set_on_click([&host]() { host.content->set_scroll_offset(0.f); });
        bar->add_child(top);
        // layui 行为：下滚一屏后出现；滚回顶部隐藏（用既有 set_on_scroll 编排）
        bar->set_visible(false);  // UI-156
        host.content->add_pinned(bar, ui::PinCorner::BottomRight, 20.f, 20.f);
        host.content->set_on_scroll([&host, bar]() {
            bar->set_visible(host.content->scroll_offset() > host.content->bounds().h);  // UI-156
        });
    }

    // ---- 撑高内容（>2 屏），保证「下滚一屏出现」阈值可达 ----
    for (int i = 0; i < 40; ++i) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "占位段落 %02d —— 内容区滚动演示用文本行。", i + 1);
        auto* l = new ui::Label(0, 0, kCW, 0, buf);
        l->set_font(host.font);
        l->set_theme(host.theme);
        l->set_color(host.theme->text_muted);
        host.content->add_child(l);
    }

    // ---- 底部引用块 ----
    auto* bq2 = new ui::BlockQuote(kCW, 0,
        "视口级锚定能力（ui 库 ScrollBox::add_pinned）：控件照常加入子树（生命周期/"
        "命中不变），但不参与流式排布与内容高度，每次布局钉在可视区角落、不随滚动移动；"
        "出现阈值用既有 set_on_scroll 编排（下滚一屏渐显）。");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/fixbar_page.cpp：克隆 layui fixbar（UI-110 视口锚定）\n"
            "\n"
            "auto* bar = new ui::VBox(0, 0, 52.f, 0);\n"
            "bar->add_child(top_btn);          // 返回顶部：复位 content 滚动\n"
            "\n"
            "// 锚定到预览区右下角，不随内容滚动：\n"
            "host.content->add_pinned(bar, ui::PinCorner::BottomRight, 20.f, 20.f);\n"
            "\n"
            "// 出现阈值：下滚一屏后出现（既有 set_on_scroll 编排）\n"
            "host.content->set_on_scroll([&] {\n"
            "    bar->visible = host.content->scroll_offset()\n"
            "                   > host.content->bounds().h;\n"
            "});");
    }
    if (host.help) {
        const char* lines[] = {
            "固定条说明",
            "",
            "构成：竖排 VBox 工具组，44px 方块按钮 + 私用区图标。",
            "",
            "已支持（UI-110 ScrollBox::add_pinned）：",
            "  视口级锚定 —— 控件钉在可视区右下角，不随内容滚动、不计入内容高度；",
            "  出现阈值 —— 下滚一屏后出现，滚回顶部隐藏（set_on_scroll 编排）；",
            "  返回顶部 —— 点击复位 content ScrollBox 滚动位置。",
            "",
            "规划能力：自定义项弹层（客服/二维码）。",
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

static void build(PageHost& host) { build_fixbar(host); }

static const PageDef kFixbarPage{43, "展示", "固定条", "固定条 fixbar：右下角工具组 + 返回顶部", build, nullptr, false, nullptr, 0, "Fixbar"};

static const bool kRegistered = []() {
    page::register_page(kFixbarPage);
    return true;
}();
