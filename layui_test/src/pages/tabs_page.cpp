#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/hbox.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/tabs.h"
#include "ui/ui/theme.h"

namespace {

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

// 镜像用 fieldset 标题块（layui-elem-field layui-field-title 的 legend）
void add_demo_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

ui::Label* make_label(PageHost& host, const char* text, const ui::Vec4& color, float w = 780.f) {
    ui::Label* label = new ui::Label(0, 0, w, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(color);
    label->set_wrap(true);
    return label;
}

// ---- demo 1: 默认风格的 Tab（5 页 + 内容区） ----
ui::Tabs* make_default_tab(PageHost& host, float h) {
    ui::Tabs* tabs = new ui::Tabs(0, 0, 800, h);
    tabs->set_font(host.font);
    tabs->set_theme(host.theme);
    tabs->add_class("layui-tab");
    tabs->add_tab("网站设置",
                  "1. 高度默认自适应，也可以随意固宽。\n2. Tab进行了响应式处理，所以无需担心数量多少。");
    tabs->add_tab("用户管理", "内容2");
    tabs->add_tab("权限分配", "内容3");
    tabs->add_tab("商品管理", "内容4");
    tabs->add_tab("订单管理", "内容5");
    tabs->set_active(0);
    host.content->add_child(tabs);
    return tabs;
}

// ---- demo 2: 动态操作 Tab（lay-filter=demo + lay-allowclose，lay-id 11..55 + 三个操作按钮） ----
void build_dynamic_tab(PageHost& host) {
    ui::Tabs* tabs = new ui::Tabs(0, 0, 800, 90.f);
    tabs->set_font(host.font);
    tabs->set_theme(host.theme);
    tabs->add_class("layui-tab");
    tabs->add_tab("网站设置", "内容1");
    tabs->add_tab("用户管理", "内容2");
    tabs->add_tab("权限分配", "内容3");
    tabs->add_tab("商品管理", "内容4");
    tabs->add_tab("订单管理", "内容5");
    tabs->set_active(0);
    host.content->add_child(tabs);

    // 操作按钮行：新增Tab项 / 删除：商品管理 / 切换到：用户管理（lay-id → 下标）
    ui::HBox* row = new ui::HBox(0, 0, 800, 34.f);
    row->set_spacing(10.f);
    row->set_theme(host.theme);

    ui::Button* add_btn = new ui::Button(0, 0, 130, 34, "新增Tab项");
    add_btn->set_font(host.font);
    add_btn->set_theme(host.theme);
    add_btn->add_class("layui-btn layui-btn-primary");
    add_btn->set_on_click([tabs]() {
        static int k = 100;
        char title[32], content[32];
        snprintf(title, sizeof(title), "新选项 %d", k);
        snprintf(content, sizeof(content), "内容 %d", k);
        ++k;
        tabs->add_tab(title, content);
        tabs->set_active(tabs->count() - 1);
    });
    row->add_child(add_btn);

    ui::Button* del_btn = new ui::Button(0, 0, 190, 34, "删除：商品管理");
    del_btn->set_font(host.font);
    del_btn->set_theme(host.theme);
    del_btn->add_class("layui-btn layui-btn-primary");
    del_btn->set_on_click([tabs]() { tabs->remove_tab(3); });  // lay-id=44 → 商品管理（下标 3）
    row->add_child(del_btn);

    ui::Button* change_btn = new ui::Button(0, 0, 190, 34, "切换到：用户管理");
    change_btn->set_font(host.font);
    change_btn->set_theme(host.theme);
    change_btn->add_class("layui-btn layui-btn-primary");
    change_btn->set_on_click([tabs]() { tabs->set_active(1); });  // lay-id=22 → 用户管理（下标 1）
    row->add_child(change_btn);

    host.content->add_child(row);
}

// ---- demo 3: Hash 地址定位（说明 + 展示 lay-id 定位能力） ----
void build_hash_tab(PageHost& host) {
    ui::Tabs* tabs = new ui::Tabs(0, 0, 800, 90.f);
    tabs->set_font(host.font);
    tabs->set_theme(host.theme);
    tabs->add_class("layui-tab");
    tabs->add_tab("网站设置",
                  "点击该Tab的任一标题（对应 lay-id），再点下方按钮演示外部定位切换。");
    tabs->add_tab("用户管理", "内容2");
    tabs->add_tab("权限分配", "内容3");
    tabs->add_tab("商品管理", "内容4");
    tabs->add_tab("订单管理", "内容5");
    tabs->set_active(0);
    host.content->add_child(tabs);
}

// ---- demo 4: 简洁风格的 Tab（layui-tab-brief，内容 100px） ----
void build_brief_tab(PageHost& host) {
    ui::Tabs* tabs = new ui::Tabs(0, 0, 800, 100.f);
    tabs->set_font(host.font);
    tabs->set_theme(host.theme);
    tabs->add_class("layui-tab layui-tab-brief");
    tabs->add_tab("网站设置", "内容不一样是要有，因为你可以监听tab事件（阅读下文档就是了）");
    tabs->add_tab("用户管理", "内容2");
    tabs->add_tab("权限分配", "内容3");
    tabs->add_tab("商品管理", "内容4");
    tabs->add_tab("订单管理", "内容5");
    tabs->set_active(0);
    host.content->add_child(tabs);
}

// ---- demo 5: 卡片风格的 Tab（layui-tab-card，内容 100px，6 页） ----
void build_card_tab(PageHost& host) {
    ui::Tabs* tabs = new ui::Tabs(0, 0, 800, 100.f);
    tabs->set_font(host.font);
    tabs->set_theme(host.theme);
    tabs->add_class("layui-tab layui-tab-card");
    tabs->add_tab("网站设置", "默认宽度是相对于父元素100%适应的，你也可以固定宽度。");
    tabs->add_tab("用户管理", "2");
    tabs->add_tab("权限分配", "3");
    tabs->add_tab("商品管理", "4");
    tabs->add_tab("订单管理", "5");
    tabs->add_tab("操作日志", "6");
    tabs->set_active(0);
    host.content->add_child(tabs);
}

// ---- demo 6: 当 Tab 数超过一定宽度（固定 290px + 响应式展开图标） ----
void build_overflow_tab(PageHost& host) {
    ui::Tabs* tabs = new ui::Tabs(0, 0, 290, 100.f);
    tabs->set_font(host.font);
    tabs->set_theme(host.theme);
    tabs->add_class("layui-tab layui-tab-card");
    tabs->add_tab("网站设置",
                  "1. 宽度足够，就不会出现右上图标；宽度不够，就会开启展开功能。\n2. 如果你的宽度是自适应的，Tab会自动判断是否需要展开，并适用于所有风格。");
    tabs->add_tab("用户管理", "2");
    tabs->add_tab("权限分配", "3");
    tabs->add_tab("商品管理", "4");
    tabs->add_tab("订单管理", "5");
    tabs->add_tab("操作日志", "6");
    tabs->set_active(0);
    host.content->add_child(tabs);
}

// ---- demo 7: 带删除功能的 Tab（card + lay-allowclose，内容 150px，6 页） ----
void build_delete_tab(PageHost& host) {
    ui::Tabs* tabs = new ui::Tabs(0, 0, 800, 150.f);
    tabs->set_font(host.font);
    tabs->set_theme(host.theme);
    tabs->add_class("layui-tab layui-tab-card");
    tabs->set_allow_close(true);
    tabs->add_tab("网站设置",
                  "1. 我个人比较喜欢卡片风格的，所以你发现又是以卡片的风格举例。\n2. 删除功能适用于所有风格，点击标题右缘的 × 删除该页。");
    tabs->add_tab("用户基本管理", "2");
    tabs->add_tab("权限分配", "3");
    tabs->add_tab("商品管理", "4");
    tabs->add_tab("订单管理", "5");
    tabs->add_tab("操作日志", "6");
    tabs->set_active(0);
    host.content->add_child(tabs);
}

void build_tabs(PageHost& host) {
    // demo 1: 默认风格
    add_demo_title(host, "默认风格的 Tab");
    make_default_tab(host, 110.f);

    // demo 2: 动态操作 Tab
    add_demo_title(host, "动态操作 Tab");
    build_dynamic_tab(host);

    // demo 3: Hash 地址定位
    add_demo_title(host, "Hash 地址定位");
    build_hash_tab(host);

    // demo 4: 简洁风格
    add_demo_title(host, "简洁风格的 Tab");
    build_brief_tab(host);

    // demo 5: 卡片风格
    add_demo_title(host, "卡片风格的 Tab");
    build_card_tab(host);

    // demo 6: 超宽展开
    add_demo_title(host, "当 Tab 数超过一定宽度");
    build_overflow_tab(host);

    // demo 7: 带删除功能
    add_demo_title(host, "带删除功能的 Tab");
    build_delete_tab(host);

    // 结尾引用
    ui::BlockQuote* bq = new ui::BlockQuote(800, 0,
        "更多我们未能呈现的示例，还有待您在阅读文档、以及不断使用的过程，去深入挖掘。");
    bq->set_font(host.font);
    bq->set_theme(host.theme);
    host.content->add_child(bq);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/tabs_page.cpp（节选）：克隆 layui demo/tab.html 预览区全部 demo\n"
            "\n"
            "ui::Tabs* tabs = new ui::Tabs(0, 0, 800, 110.f);\n"
            "tabs->set_font(host.font);\n"
            "tabs->set_theme(host.theme);\n"
            "tabs->add_class(\"layui-tab\");           // card/brief 变体\n"
            "tabs->add_tab(\"网站设置\", \"内容文本\"); // 每页可带内容区，切换联动\n"
            "tabs->add_tab(\"用户管理\");\n"
            "tabs->set_active(0);\n"
            "\n"
            "// 动态操作：新增/删除/切换（lay-id → 下标）\n"
            "tabs->add_tab(\"新选项\", \"内容\");\n"
            "tabs->remove_tab(3);      // 删除：商品管理\n"
            "tabs->set_active(1);      // 切换到：用户管理\n"
            "\n"
            "// 允许关闭：标题右缘 × 删除该页\n"
            "tabs->set_allow_close(true);\n"
            "\n"
            "// 超宽响应式：容器 290px + 折叠展开三角\n"
            "ui::Tabs* narrow = new ui::Tabs(0, 0, 290, 100.f);\n"
            "narrow->set_font(host.font);\n"
            "narrow->set_theme(host.theme);\n"
            "narrow->add_class(\"layui-tab layui-tab-card\");\n"
            "narrow->add_tab(\"网站设置\");\n"
            "narrow->add_tab(\"用户管理\");");
    }
    if (host.help) {
        const char* lines[] = {
            "选项卡（Tabs）API",
            "  add_tab(title, content)：追加页，content 为内容区文本（可选）",
            "  set_content(i, text)：设置指定页内容",
            "  count()：当前页数 | remove_tab(i)：删除指定页（lay-id 定位）",
            "  set_active(i)：激活并触发 on_change(int)",
            "  set_allow_close(bool)：允许删除，标题右缘 ×",
            "类名：layui-tab（默认）/ layui-tab-brief（简约下划线）/ layui-tab-card（卡片式）",
            "内容区：标题行下方显示当前激活页 content，切换自动联动",
            "超宽响应式：标题总宽超出容器时右侧展开三角，点击弹出下拉列全部页",
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
    build_tabs(host);
}

static const PageDef kTabsPage{36, "展示", "选项卡", "选项卡：默认/卡片/简约", build, nullptr, false, nullptr, 0, "Tabs"};

static const bool kRegistered = []() {
    page::register_page(kTabsPage);
    return true;
}();