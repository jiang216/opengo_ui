#include "../page_registry.h"

#include "ui/ui/card.h"
#include "ui/ui/code.h"
#include "ui/ui/collapse.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/panel.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

constexpr float kGray = 0.98f;  // layui-bg-gray #fafafa

void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

void style_label(ui::Label* l, PageHost& host, const ui::Vec4& c) {
    l->set_font(host.font);
    l->set_theme(host.theme);
    l->set_color(c);
}

// 灰底两列容器（layui-bg-gray padding 30 + layui-row layui-col-space15）
ui::HBox* gray_two_col(PageHost& host, float h) {
    ui::HBox* row = new ui::HBox(0, 0, 800, h);
    row->set_fill(ui::Vec4{kGray, kGray, kGray, 1.0f});
    row->set_padding(15.f);
    row->set_spacing(15.f);
    row->set_theme(host.theme);
    host.content->add_child(row);
    return row;
}

// 1) 普通面板：layui-panel 白底 + #eee 边框 + 阴影（无 header，大留白）
void build_plain_panels(PageHost& host) {
    field_title(host, "普通面板");
    ui::HBox* row = gray_two_col(host, 220.f);
    for (int i = 0; i < 2; ++i) {
        ui::Card* card = new ui::Card(0, 0, 375.f, 220.f);
        card->set_font(host.font);
        card->set_theme(host.theme);
        ui::Label* txt = new ui::Label(0, 0, 300, 0, "一个面板");
        style_label(txt, host, host.theme->text_secondary);
        txt->set_wrap(true);
        card->add_child(txt);
        row->add_child(card);
    }
}

// 2) 卡片面板：layui-card header + body（两列）
void build_card_panels(PageHost& host) {
    field_title(host, "卡片面板");
    ui::HBox* row = gray_two_col(host, 180.f);
    {
        ui::Card* card = new ui::Card(0, 0, 375.f, 180.f);
        card->set_font(host.font);
        card->set_theme(host.theme);
        card->set_header("卡片面板");
        ui::Label* txt = new ui::Label(0, 0, 340, 0,
            "卡片式面板面板通常用于非白色背景色的主体内\n从而映衬出边框投影");
        style_label(txt, host, host.theme->text_secondary);
        txt->set_wrap(true);
        card->add_child(txt);
        row->add_child(card);
    }
    {
        ui::Card* card = new ui::Card(0, 0, 375.f, 180.f);
        card->set_font(host.font);
        card->set_theme(host.theme);
        card->set_header("卡片面板");
        ui::Label* txt = new ui::Label(0, 0, 340, 0,
            "结合 layui 的栅格系统\n轻松实现响应式布局");
        style_label(txt, host, host.theme->text_secondary);
        txt->set_wrap(true);
        card->add_child(txt);
        row->add_child(card);
    }
}

// 折叠项标题文案
const char* kQ1 = "为什么JS社区大量采用未发布或者未广泛支持的语言特性？";
const char* kQ2 = "为什么前端工程师多不愿意用 Bootstrap 框架？";
const char* kQ3 = "layui 更适合哪些开发者？";
const char* kQ4 = "一个折叠面板标题？";
const char* kA1 = "有不少其他答案说是因为JS太差……JS 虽然有很多问题，但设计总体还是优秀的。——来自知乎@贺师俊";
const char* kA2 = "因为不适合。如果希望开发长期的项目或者制作产品类网站，那么就需要实现特定的设计……——来自知乎@Kayo";
const char* kA3 = "准确地说，layui 更多是面向那些追求开发简单的前端工程师们，以及所有层次的服务端程序员。";
const char* kA4 = "一个折叠面板的内容。。。";

ui::Label* make_colla_label(PageHost& host, const char* text) {
    ui::Label* l = new ui::Label(0, 0, 720, 0, text);
    style_label(l, host, host.theme->text_secondary);
    l->set_wrap(true);
    return l;
}

void fill_4_items(ui::Collapse* col, PageHost& host) {
    col->set_font(host.font);
    col->set_theme(host.theme);
    col->add_item(kQ1);
    col->add_child(make_colla_label(host, kA1));
    col->add_item(kQ2);
    col->add_child(make_colla_label(host, kA2));
    col->add_item(kQ3);
    col->add_child(make_colla_label(host, kA3));
    col->add_item(kQ4);
    col->add_child(make_colla_label(host, kA4));
}

// 3) 常规折叠面板：layui-collapse 非手风琴（可多开）
void build_regular_collapse(PageHost& host) {
    field_title(host, "常规折叠面板");
    ui::Collapse* col = new ui::Collapse(0, 0, 800, 0);
    col->set_accordion(false);
    fill_4_items(col, host);
    host.content->add_child(col);
}

// 4) 手风琴折叠：lay-accordion（同一时刻仅一个展开）
void build_accordion(PageHost& host) {
    field_title(host, "手风琴折叠");
    ui::Collapse* col = new ui::Collapse(0, 0, 800, 0);
    col->set_accordion(true);
    fill_4_items(col, host);
    col->set_active(0);  // 首项默认展开
    host.content->add_child(col);
}

// 嵌套手风琴：外层文豪/科学家/艺术家，文豪内嵌唐代/宋代/当代，唐代再嵌诗人
ui::Collapse* build_nested(PageHost& host, int depth) {
    ui::Collapse* col = new ui::Collapse(0, 0, 800, 0);
    col->set_font(host.font);
    col->set_theme(host.theme);
    col->set_accordion(true);
    if (depth == 0) {
        // 诗人层
        col->add_item("杜甫");
        col->add_child(make_colla_label(host, "伟大的诗人"));
        col->add_item("李白");
        col->add_child(make_colla_label(host, "伟大的诗仙"));
        col->add_item("王勃");
        col->add_child(make_colla_label(host, "千古绝唱《滕王阁序》"));
        col->set_active(0);
        return col;
    }
    // 朝代层
    col->add_item("唐代");
    col->add_child(build_nested(host, depth - 1));
    col->add_item("宋代");
    col->add_child(make_colla_label(host, "比如苏轼、李清照"));
    col->add_item("当代");
    col->add_child(make_colla_label(host, "比如七堇年"));
    col->set_active(0);
    return col;
}

// 5) 面板嵌套：手风琴嵌套
void build_nested_panels(PageHost& host) {
    field_title(host, "面板嵌套");
    ui::Collapse* col = new ui::Collapse(0, 0, 800, 0);
    col->set_font(host.font);
    col->set_theme(host.theme);
    col->set_accordion(true);
    col->add_item("文豪");
    col->add_child(build_nested(host, 1));  // 文豪 -> 朝代
    col->add_item("科学家");
    col->add_child(make_colla_label(host, "伟大的科学家"));
    col->add_item("艺术家");
    col->add_child(make_colla_label(host, "浑身散发着艺术细胞"));
    col->set_active(0);
    host.content->add_child(col);
}

void build_panels(PageHost& host) {
    build_plain_panels(host);
    build_card_panels(host);
    build_regular_collapse(host);
    build_accordion(host);
    build_nested_panels(host);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/card_page.cpp：克隆 layui panel.html 全部 5 组 demo\n"
            "\n"
            "// 1) 普通面板：Card 无 header + 灰底两列容器（HBox set_fill #fafafa）\n"
            "ui::Card* card = new ui::Card(0, 0, 375.f, 220.f);\n"
            "card->set_theme(host.theme);\n"
            "card->add_child(label);\n"
            "\n"
            "// 2) 卡片面板：Card set_header + body 子控件\n"
            "card->set_header(\"卡片面板\");\n"
            "\n"
            "// 3) 常规折叠（非手风琴）：Collapse set_accordion(false)\n"
            "ui::Collapse* col = new ui::Collapse(0, 0, 800, 0);\n"
            "col->set_accordion(false);\n"
            "col->add_item(title); col->add_child(content);\n"
            "\n"
            "// 4) 手风琴折叠：Collapse set_accordion(true) + set_active(0)\n"
            "col->set_accordion(true); col->set_active(0);\n"
            "\n"
            "// 5) 面板嵌套：Collapse 作为另一 Collapse 的子控件\n"
            "col->add_child(build_nested(host, 1));");
    }
    if (host.help) {
        const char* lines[] = {
            "面板页（Card + Collapse）",
            "  Card：白底圆角 2 + 阴影；set_header 有/无标题条",
            "  HBox set_fill(#fafafa)：layui-bg-gray 灰底两列容器",
            "  Collapse：set_accordion(true) 手风琴 / (false) 非手风琴多开",
            "  Collapse：add_item(title) 后 add_child(内容) 填充面板项",
            "  面板嵌套：Collapse 可作为另一 Collapse 的子控件无限嵌套",
        };
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            style_label(label, host, host.theme->text);
            host.help->add_child(label);
        }
    }
}

}  // namespace

static void build(PageHost& host) {
    build_panels(host);
}

static const PageDef kCardPage{38, "展示", "面板", "面板：普通/卡片/折叠/手风琴/嵌套", build, nullptr, false, nullptr, 0, "Panel / Card / Collapse"};

static const bool kRegistered = []() {
    page::register_page(kCardPage);
    return true;
}();
