#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/icon.h"
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

// 说明行（次级文字色，自动换行）
void para_line(PageHost& host, const char* text) {
    auto* l = new ui::Label(0, 0, kCW, 0, text);
    l->set_font(host.font);
    l->set_theme(host.theme);
    l->set_color(host.theme->text_secondary);
    l->set_wrap(true);
    host.content->add_child(l);
}

void build_fixbar(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "固定条 fixbar：固定在页面右下角的快捷工具组（返回顶部、客服、二维码等）。"
        "本页演示其构成与交互——点击「返回顶部」按钮会真实滚动内容区到顶部。");
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

    // ---- Fieldset 2: 交互演示 ----
    field_title(host, "返回顶部交互");
    para_line(host,
        "下方按钮模拟右下角「返回顶部」：点击后将内容区滚动位置复位到顶部。"
        "真实场景中该按钮在内容下滚一屏后出现。");
    {
        auto* r = new ui::HBox(0, 0, kCW, 46.f);
        r->set_padding(0.f);
        r->set_spacing(12.f);
        r->set_theme(host.theme);
        auto* btn = new ui::Button(0, 0, 130.f, 38.f, "返回顶部");
        btn->set_font(host.font);
        btn->set_theme(host.theme);
        btn->add_class("layui-btn");
        btn->set_on_click([&host]() { host.content->set_scroll_offset(0.f); });
        r->add_child(btn);
        host.content->add_child(r);
    }

    // ---- 撑高内容，便于验证滚动复位 ----
    for (int i = 0; i < 12; ++i) {
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
        "视口级固定定位（不随内容滚动）依赖外壳层布局钩子，当前以页内呈现替代；"
        "接入后本页将挂载到预览区右下角常驻。");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/fixbar_page.cpp：克隆 layui fixbar\n"
            "\n"
            "// 工具组：VBox{ Button(layui-btn-primary) * N }，\n"
            "// 按钮内嵌 Icon（top/service/qr）。\n"
            "\n"
            "// 返回顶部：直接复位内容区滚动\n"
            "btn->set_on_click([&host] { host.content->set_scroll_offset(0.f); });\n"
            "\n"
            "// 规划：外壳层提供 viewport 级锚点（右下角常驻）+ 出现阈值");
    }
    if (host.help) {
        const char* lines[] = {
            "固定条说明",
            "",
            "构成：竖排 VBox 工具组，44px 方块按钮 + 私用区图标。",
            "",
            "已支持：",
            "  返回顶部 —— 点击复位 content ScrollBox 滚动位置。",
            "",
            "规划（ui 库迭代项）：",
            "  视口级固定锚点（不随滚动）；出现阈值（下滚一屏渐显）；",
            "  自定义项（客服/二维码弹层）。",
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
