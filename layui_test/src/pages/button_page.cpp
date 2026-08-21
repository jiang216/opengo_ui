#include "../page_registry.h"

#include "ui/ui/button.h"
#include "ui/ui/button_group.h"
#include "ui/ui/flowbox.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

// 小节标题：灰色大字
ui::Label* section_title(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    return label;
}

// 按钮行：横向排列，子按钮定宽；垂直居中，尺寸类按钮按各自 preferred_height 显示高度差
// （layui 行内按钮 vertical-align:middle 语义）
ui::HBox* button_row(PageHost& host, float h) {
    ui::HBox* box = new ui::HBox(0, 0, 800, h);
    box->set_padding(0.f);
    box->set_spacing(8.f);
    box->set_theme(host.theme);
    box->set_valign(ui::Valign::Center);
    host.content->add_child(box);
    return box;
}

ui::Button* add_btn(PageHost& host, ui::Widget* row, const char* text, const char* cls,
                    float w = 96.f) {
    ui::Button* btn = new ui::Button(0, 0, w, 38.f, text);
    btn->set_bounds(0, 0, w, 38.f);
    btn->set_font(host.font);
    btn->set_theme(host.theme);
    if (cls) btn->add_class(cls);
    row->add_child(btn);
    return btn;
}

// 图标按钮构造（set_icon 走 iconfont.ttf，layui_icons.h 类名）
ui::Button* make_icon_btn(PageHost& host, const char* icon, const char* cls, float w = 44.f,
                          float h = 38.f) {
    ui::Button* btn = new ui::Button(0, 0, w, h, "");
    btn->set_bounds(0, 0, w, h);
    btn->set_font(host.font);
    btn->set_theme(host.theme);
    btn->set_icon(icon);
    if (cls) btn->add_class(cls);
    return btn;
}

// 图标按钮：加入 HBox 行
ui::Button* add_icon_btn(PageHost& host, ui::HBox* row, const char* icon, const char* cls,
                         float w = 44.f, float h = 38.f) {
    ui::Button* btn = make_icon_btn(host, icon, cls, w, h);
    row->add_child(btn);
    return btn;
}

// 图标按钮：加入按钮组
ui::Button* add_group_icon_btn(PageHost& host, ui::ButtonGroup* group, const char* icon,
                               const char* cls, float w = 44.f, float h = 38.f) {
    ui::Button* btn = make_icon_btn(host, icon, cls, w, h);
    group->add_button(btn);
    return btn;
}

void build_buttons(PageHost& host) {
    host.content->add_child(section_title(host, "基础按钮"));

    ui::HBox* r1 = button_row(host, 38.f);
    add_btn(host, r1, "原始按钮", "layui-btn layui-btn-primary");
    add_btn(host, r1, "默认按钮", "layui-btn");
    add_btn(host, r1, "主要按钮", "layui-btn layui-btn-normal");
    add_btn(host, r1, "暖色按钮", "layui-btn layui-btn-warm");
    add_btn(host, r1, "危险按钮", "layui-btn layui-btn-danger");
    add_btn(host, r1, "禁用按钮", "layui-btn layui-btn-disabled");

    host.content->add_child(section_title(host, "边框按钮"));

    ui::HBox* r2 = button_row(host, 38.f);
    add_btn(host, r2, "原始按钮", "layui-btn layui-btn-primary layui-border");
    add_btn(host, r2, "绿色按钮", "layui-btn layui-btn-primary layui-border-green");
    add_btn(host, r2, "蓝色按钮", "layui-btn layui-btn-primary layui-border-blue");
    add_btn(host, r2, "橙色按钮", "layui-btn layui-btn-primary layui-border-orange");
    add_btn(host, r2, "红色按钮", "layui-btn layui-btn-primary layui-border-red");
    add_btn(host, r2, "黑色按钮", "layui-btn layui-btn-primary layui-border-black");

    host.content->add_child(section_title(host, "不同尺寸"));

    ui::HBox* r3 = button_row(host, 44.f);
    add_btn(host, r3, "大型按钮", "layui-btn layui-btn-lg", 104.f);
    add_btn(host, r3, "默认按钮", "layui-btn");
    add_btn(host, r3, "小型按钮", "layui-btn layui-btn-sm");
    add_btn(host, r3, "迷你按钮", "layui-btn layui-btn-xs");

    ui::HBox* r4 = button_row(host, 44.f);
    add_btn(host, r4, "大型按钮", "layui-btn layui-btn-lg layui-btn-normal", 104.f);
    add_btn(host, r4, "默认按钮", "layui-btn layui-btn-normal");
    add_btn(host, r4, "小型按钮", "layui-btn layui-btn-sm layui-btn-normal");
    add_btn(host, r4, "迷你按钮", "layui-btn layui-btn-xs layui-btn-normal");

    ui::HBox* r5 = button_row(host, 44.f);
    add_btn(host, r5, "大型按钮", "layui-btn layui-btn-lg layui-btn-primary", 104.f);
    add_btn(host, r5, "默认按钮", "layui-btn layui-btn-primary");
    add_btn(host, r5, "小型按钮", "layui-btn layui-btn-sm layui-btn-primary");
    add_btn(host, r5, "迷你按钮", "layui-btn layui-btn-xs layui-btn-primary");

    host.content->add_child(section_title(host, "流体按钮"));

    ui::HBox* r6 = button_row(host, 38.f);
    ui::Button* fluid = add_btn(host, r6, "流体按钮（宽度自适应父容器）", "layui-btn layui-btn-fluid", 800.f);
    fluid->set_bounds(0, 0, 800.f, 38.f);

    host.content->add_child(section_title(host, "图标按钮"));

    ui::HBox* r7 = button_row(host, 38.f);
    add_icon_btn(host, r7, "left", "layui-btn");
    add_icon_btn(host, r7, "right", "layui-btn");
    add_icon_btn(host, r7, "edit", "layui-btn");
    add_icon_btn(host, r7, "share", "layui-btn");
    add_icon_btn(host, r7, "delete", "layui-btn");
    add_icon_btn(host, r7, "down", "layui-btn");

    ui::HBox* r8 = button_row(host, 38.f);
    add_icon_btn(host, r8, "left", "layui-btn layui-btn-danger");
    add_icon_btn(host, r8, "right", "layui-btn layui-btn-danger");
    add_icon_btn(host, r8, "edit", "layui-btn layui-btn-danger");
    add_icon_btn(host, r8, "share", "layui-btn layui-btn-danger");
    add_icon_btn(host, r8, "delete", "layui-btn layui-btn-danger");
    add_icon_btn(host, r8, "down", "layui-btn layui-btn-danger");

    ui::HBox* r9 = button_row(host, 30.f);
    add_icon_btn(host, r9, "left", "layui-btn layui-btn-primary layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "right", "layui-btn layui-btn-primary layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "edit", "layui-btn layui-btn-primary layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "share", "layui-btn layui-btn-primary layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "delete", "layui-btn layui-btn-primary layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "down", "layui-btn layui-btn-primary layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "left", "layui-btn layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "right", "layui-btn layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "edit", "layui-btn layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "share", "layui-btn layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "delete", "layui-btn layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "down", "layui-btn layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "left", "layui-btn layui-btn-normal layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "right", "layui-btn layui-btn-normal layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "edit", "layui-btn layui-btn-normal layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "share", "layui-btn layui-btn-normal layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "delete", "layui-btn layui-btn-normal layui-btn-sm", 34.f, 30.f);
    add_icon_btn(host, r9, "down", "layui-btn layui-btn-normal layui-btn-sm", 34.f, 30.f);

    ui::HBox* r10 = button_row(host, 30.f);
    ui::Button* bdel = add_btn(host, r10, "删除", "layui-btn layui-btn-sm layui-btn-danger", 88.f);
    bdel->set_icon("delete");
    ui::Button* bfor = add_btn(host, r10, "禁用", "layui-btn layui-btn-xs layui-btn-disabled", 80.f);
    bfor->set_icon("delete");

    host.content->add_child(section_title(host, "圆角按钮"));

    ui::HBox* r11 = button_row(host, 38.f);
    add_btn(host, r11, "原始按钮", "layui-btn layui-btn-primary layui-btn-radius");
    add_btn(host, r11, "默认按钮", "layui-btn layui-btn-radius");
    add_btn(host, r11, "主要按钮", "layui-btn layui-btn-normal layui-btn-radius");
    add_btn(host, r11, "暖色按钮", "layui-btn layui-btn-warm layui-btn-radius");
    add_btn(host, r11, "危险按钮", "layui-btn layui-btn-danger layui-btn-radius");
    add_btn(host, r11, "禁用按钮", "layui-btn layui-btn-disabled layui-btn-radius");

    ui::HBox* r12 = button_row(host, 44.f);
    add_btn(host, r12, "大型圆角按钮", "layui-btn layui-btn-lg layui-btn-primary layui-btn-radius",
            120.f);
    ui::Button* del = add_btn(host, r12, "删除", "layui-btn layui-btn-sm layui-btn-normal", 88.f);
    del->set_icon("delete");
    ui::Button* forbid = add_btn(host, r12, "禁用", "layui-btn layui-btn-xs layui-btn-disabled",
                                 80.f);
    forbid->set_icon("delete");

    host.content->add_child(section_title(host, "按钮组"));

    // layui .layui-btn-group+.layui-btn-group { margin-left: 10px }：三个组同行排列
    ui::HBox* grp_row = button_row(host, 38.f);
    grp_row->set_spacing(10.f);

    ui::ButtonGroup* g1 = new ui::ButtonGroup(0, 0, 288.f, 38.f);
    g1->set_theme(host.theme);
    ui::Button* ga = new ui::Button(0, 0, 96.f, 38.f, "增加");
    ga->set_font(host.font);
    ga->set_theme(host.theme);
    ga->add_class("layui-btn");
    g1->add_button(ga);
    ui::Button* gb = new ui::Button(0, 0, 96.f, 38.f, "编辑");
    gb->set_font(host.font);
    gb->set_theme(host.theme);
    gb->add_class("layui-btn");
    g1->add_button(gb);
    ui::Button* gc = new ui::Button(0, 0, 96.f, 38.f, "删除");
    gc->set_font(host.font);
    gc->set_theme(host.theme);
    gc->add_class("layui-btn");
    g1->add_button(gc);
    grp_row->add_child(g1);

    ui::ButtonGroup* g2 = new ui::ButtonGroup(0, 0, 160.f, 30.f);
    g2->set_theme(host.theme);
    add_group_icon_btn(host, g2, "left", "layui-btn layui-btn-sm", 40.f, 30.f);
    add_group_icon_btn(host, g2, "right", "layui-btn layui-btn-sm", 40.f, 30.f);
    add_group_icon_btn(host, g2, "edit", "layui-btn layui-btn-sm", 40.f, 30.f);
    add_group_icon_btn(host, g2, "down", "layui-btn layui-btn-sm", 40.f, 30.f);
    grp_row->add_child(g2);

    ui::ButtonGroup* g3 = new ui::ButtonGroup(0, 0, 180.f, 30.f);
    g3->set_theme(host.theme);
    ui::Button* g3a = new ui::Button(0, 0, 60.f, 30.f, "增加");
    g3a->set_font(host.font);
    g3a->set_theme(host.theme);
    g3a->add_class("layui-btn layui-btn-primary layui-btn-sm");
    g3->add_button(g3a);
    add_group_icon_btn(host, g3, "edit", "layui-btn layui-btn-primary layui-btn-sm", 40.f, 30.f);
    add_group_icon_btn(host, g3, "share", "layui-btn layui-btn-primary layui-btn-sm", 40.f, 30.f);
    add_group_icon_btn(host, g3, "delete", "layui-btn layui-btn-primary layui-btn-sm", 40.f, 30.f);
    grp_row->add_child(g3);

    host.content->add_child(section_title(host, "按钮容器"));

    ui::Label* intro = new ui::Label(
        0, 0, 800, 0,
        "尽管按钮在同节点并排时会自动拉开间距，但在按钮太多的情况，效果并不是很美好。因为你需要用到按钮容器");
    intro->set_font(host.font);
    intro->set_theme(host.theme);
    intro->set_color(host.theme->text_muted);
    intro->set_wrap(true);
    host.content->add_child(intro);

    ui::FlowBox* container = new ui::FlowBox(0, 0, 800, 0);
    container->set_padding(0.f);
    container->set_spacing(10.f);  // layui .layui-btn-container 按钮右/下 10px margin
    container->set_theme(host.theme);
    host.content->add_child(container);
    const char* cn_names[] = {"按钮一", "按钮二", "按钮三", "按钮四", "按钮五", "按钮六"};
    for (int i = 0; i < 18; ++i) add_btn(host, container, cn_names[i % 6], "layui-btn");
    add_btn(host, container, "按钮一", "layui-btn");
    add_btn(host, container, "按钮二", "layui-btn");
}

}  // namespace

static void build(PageHost& host) {
    build_buttons(host);
}

static const PageDef kButtonPage{10, "通用", "按钮", "layui 按钮风格与尺寸", build, nullptr, false, nullptr, 0, "Button"};

static const bool kRegistered = []() {
    page::register_page(kButtonPage);
    return true;
}();
