#include "../page_registry.h"

#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/timeline.h"

namespace {

// 小节标题：带 legend 横线（layui-elem-field layui-field-title）
void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

// 常规时间线（fieldset「常规时间线」）：4 条，标题 h3，内容含多行/列表/强调/图标
void build_regular(PageHost& host) {
    field_title(host, "常规时间线");

    ui::Timeline* tl = new ui::Timeline(0, 0, 800, 0.f);
    tl->set_font(host.font);
    tl->set_theme(host.theme);

    tl->add_item_rich("8月18日", true, {
        {ui::LineKind::Text, "layui 2.0 的一切准备工作似乎都已到位。发布之弦，一触即发。"},
        {ui::LineKind::Text, "不枉近百个日日夜夜与之为伴。因小而大，因弱而强。"},
        {ui::LineKind::Text, "无论它能走多远，抑或如何支撑？至少我曾倾注全心，无怨无悔"},
        {ui::LineKind::Icon, "", ui::layui_icon::face_smile_b},
    });

    tl->add_item_rich("8月16日", true, {
        {ui::LineKind::Text, "杜甫的思想核心是儒家的仁政思想，他有\u201C致君尧舜上，再使风俗淳\u201D的宏伟抱负。个人最爱的名篇有："},
        {ui::LineKind::Bullet, "《登高》"},
        {ui::LineKind::Bullet, "《茅屋为秋风所破歌》"},
    });

    tl->add_item_rich("8月15日", true, {
        {ui::LineKind::Text, "中国人民抗日战争胜利日"},
        {ui::LineKind::Text, "铭记、感恩"},
        {ui::LineKind::Text, "所有为中华民族浴血奋战的英雄将士"},
        {ui::LineKind::Text, "永垂不朽"},
    });

    tl->add_item("过去", "");
    host.content->add_child(tl);
}

// 简约时间线：大事记（fieldset「简约时间线：大事记」）：5 条仅标题，末条图标旋转（loading-1）
void build_simple(PageHost& host) {
    field_title(host, "简约时间线：大事记");

    ui::Timeline* tl = new ui::Timeline(0, 0, 800, 0.f);
    tl->set_font(host.font);
    tl->set_theme(host.theme);

    tl->add_item("2018年，layui 5.0 发布。并发展成为中国最受欢迎的前端UI框架（期望）", "");
    tl->set_axis_icon(ui::layui_icon::fire);  // 首条轴点 icon 为火苗（layui-icon-fire 0xE756）
    tl->add_item("2017年，layui 里程碑版本 2.0 发布", "");
    tl->add_item("2017年，layui 里程碑版本 2.0 发布", "");
    tl->add_item("2016年，layui 首个版本发布", "");
    tl->add_item("2015年，layui 孵化", "");
    tl->add_item("更久前，轮子时代。维护几个独立组件：layer等", "");
    tl->set_axis_icon(ui::layui_icon::loading_1);  // layui-icon-loading-1 旋转动画
    tl->set_axis_rotate(360.f);                    // 每帧绕中心旋转 360°/s
    host.content->add_child(tl);

    // 引用块：更多我们未能呈现的示例……
    ui::Label* note = new ui::Label(0, 0, 800, 0, "更多我们未能呈现的示例，还有待您在阅读文档、以及不断使用的过程，去深入挖掘。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_secondary);
    host.content->add_child(note);
}

void build_timeline(PageHost& host) {
    build_regular(host);
    build_simple(host);
}

}  // namespace

static void build(PageHost& host) {
    build_timeline(host);
}

static const PageDef kTimelinePage{40, "展示", "时间线", "时间线：常规/简约（镜像 demo/timeline.html）", build, nullptr, false, nullptr, 0, "Timeline"};

static const bool kRegistered = []() {
    page::register_page(kTimelinePage);
    return true;
}();