#include "../page_registry.h"

#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/panel.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/transfer.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {

void add_fieldset_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* ft = new ui::FieldsetTitle(800, 40.f, text);
    ft->set_font(host.font);
    ft->set_theme(host.theme);
    host.content->add_child(ft);
}

void add_note(PageHost& host, const char* text) {
    ui::Label* note = new ui::Label(0, 4, 800, 0, text);
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

ui::Transfer* make_transfer(PageHost& host, float h) {
    ui::Transfer* t = new ui::Transfer(0, 0, 560.f, h);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
    return t;
}

// 模拟 data1（文人）
void fill_writers(ui::Transfer* t) {
    t->add_item("李白");
    t->add_item("杜甫");
    t->add_item("苏轼");
    t->add_item("李清照");
    t->add_item("鲁迅", true);  // disabled
    t->add_item("巴金");
    t->add_item("冰心");
    t->add_item("矛盾");
    t->add_item("贤心");
}

// 模拟 data2（美食）
void fill_food(ui::Transfer* t) {
    t->add_item("瓦罐汤");
    t->add_item("油酥饼");
    t->add_item("炸酱面");
    t->add_item("串串香", true);  // disabled
    t->add_item("豆腐脑");
    t->add_item("驴打滚");
    t->add_item("北京烤鸭");
    t->add_item("烤冷面");
    t->add_item("毛血旺", true);  // disabled
    t->add_item("肉夹馍");
    t->add_item("臊子面");
    t->add_item("凉皮");
    t->add_item("羊肉泡馍");
    t->add_item("冰糖葫芦", true);  // disabled
    t->add_item("狼牙土豆");
}

// ── 1. 基础效果（test1）──────────────────────────────────────────────────────
void build_demo1(PageHost& host) {
    add_fieldset_title(host, "基础效果");
    ui::Transfer* t = make_transfer(host, 240.f);
    fill_writers(t);
}

// ── 2. 定义标题及数据源（test2）──────────────────────────────────────────────
void build_demo2(PageHost& host) {
    add_fieldset_title(host, "定义标题及数据源");
    ui::Transfer* t = make_transfer(host, 210.f);
    t->set_left_label("候选文人");
    t->set_right_label("获奖文人");
    fill_writers(t);
}

// ── 3. 初始右侧数据集合（test3）──────────────────────────────────────────────
void build_demo3(PageHost& host) {
    add_fieldset_title(host, "初始右侧数据集合");
    ui::Transfer* t = make_transfer(host, 300.f);
    fill_food(t);
    // value: ["1","3","5","7","9","11"] → 瓦罐汤/炸酱面/豆腐脑/北京烤鸭/毛血旺(disabled)/臊子面
    t->add_right_item("瓦罐汤");
    t->add_right_item("炸酱面");
    t->add_right_item("豆腐脑");
    t->add_right_item("北京烤鸭");
    t->add_right_item("毛血旺", true);
    t->add_right_item("臊子面");
}

// ── 4. 显示搜索框（test4）────────────────────────────────────────────────────
void build_demo4(PageHost& host) {
    add_fieldset_title(host, "显示搜索框");
    ui::Transfer* t = make_transfer(host, 240.f);
    t->set_left_label("文本墨客");
    t->set_right_label("获奖文人");
    t->set_show_search(true);
    fill_writers(t);
}

// ── 5. 数据格式解析（test5）──────────────────────────────────────────────────
void build_demo5(PageHost& host) {
    add_fieldset_title(host, "数据格式解析");
    ui::Transfer* t = make_transfer(host, 150.f);
    t->add_item("李白");
    t->add_item("杜甫");
    t->add_item("贤心");
}

// ── 6. 穿梭时的回调（test6）──────────────────────────────────────────────────
void build_demo6(PageHost& host) {
    add_fieldset_title(host, "穿梭时的回调");
    ui::Transfer* t = make_transfer(host, 240.f);
    fill_writers(t);
    ui::Label* status = new ui::Label(0, 4, 800, 0, "穿梭回调：待触发");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    t->set_on_change([status](ui::TransferDirection dir, const std::vector<std::string>& moved) {
        std::string joined;
        for (size_t i = 0; i < moved.size(); ++i)
            joined += (i ? ", " : "") + moved[i];
        const char* side = (dir == ui::TransferDirection::ToRight) ? "左边 → 右边" : "右边 → 左边";
        char buf[256];
        std::snprintf(buf, sizeof(buf), "穿梭回调：%s 数据：%s", side, joined.c_str());
        status->set_text(buf);
    });
}

// ── 7. 实例调用（test7：getData + reload）────────────────────────────────────
void build_demo7(PageHost& host) {
    add_fieldset_title(host, "实例调用");

    ui::HBox* row = new ui::HBox(0, 0, 800, 40.f);
    row->set_padding(0.f);
    row->set_spacing(12.f);
    row->set_valign(ui::Valign::Center);
    row->set_theme(host.theme);

    ui::Transfer* t = make_transfer(host, 240.f);
    fill_writers(t);

    ui::Label* status = new ui::Label(0, 4, 800, 0, "右侧数据：空");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);

    ui::Button* get = new ui::Button(0, 0, 0.f, 38.f, "获取右侧数据");
    get->set_font(host.font);
    get->set_theme(host.theme);
    get->add_class("layui-btn");
    get->set_on_click([t, status]() {
        const auto& right = t->right_items();
        std::string joined;
        for (size_t i = 0; i < right.size(); ++i)
            joined += (i ? ", " : "") + right[i];
        char buf[256];
        std::snprintf(buf, sizeof(buf), "右侧数据：%s", joined.empty() ? "空" : joined.c_str());
        status->set_text(buf);
    });
    row->add_child(get);

    ui::Button* reload = new ui::Button(0, 0, 0.f, 38.f, "重载实例");
    reload->set_font(host.font);
    reload->set_theme(host.theme);
    reload->add_class("layui-btn");
    reload->set_on_click([t]() {
        // transfer.reload('key123', {title:['文人','喜欢的文人'], value:['2','5','9'], showSearch:true})
        t->clear();
        t->set_left_label("文人");
        t->set_right_label("喜欢的文人");
        t->set_show_search(true);
        fill_writers(t);
        t->add_right_item("杜甫");
        t->add_right_item("鲁迅", true);
        t->add_right_item("巴金");
    });
    row->add_child(reload);
    host.content->add_child(row);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/transfer_page.cpp — 一比一克隆 layui transfer.html 全部 7 个 demo\n"
            "\n"
            "// 1. 基础效果\n"
            "ui::Transfer* t = new ui::Transfer(0, 0, 560, 240.f);\n"
            "t->add_item(\"李白\"); ...\n"
            "\n"
            "// 2. 定义标题及数据源\n"
            "t->set_left_label(\"候选文人\"); t->set_right_label(\"获奖文人\");\n"
            "\n"
            "// 3. 初始右侧数据集合\n"
            "t->add_right_item(\"瓦罐汤\"); t->add_right_item(\"毛血旺\", true);\n"
            "\n"
            "// 4. 显示搜索框\n"
            "t->set_show_search(true);   // 各面板头部输入实时过滤\n"
            "\n"
            "// 5. 数据格式解析\n"
            "t->add_item(\"李白\"); ...\n"
            "\n"
            "// 6. 穿梭时的回调\n"
            "t->set_on_change([](ui::TransferDirection dir, const auto& moved) { ... });\n"
            "\n"
            "// 7. 实例调用：getData / reload\n"
            "t->right_items();   // 获取右侧数据\n"
            "t->clear(); ... t->add_right_item(...);   // 重载实例");
    }
    if (host.help) {
        const char* lines[] = {
            "穿梭框（Transfer）API — 一比一克隆 transfer.html 全部 7 个 demo",
            "",
            "1. 基础效果",
            "   左右列表 + 中间搬移按钮",
            "2. 定义标题及数据源",
            "   set_left_label / set_right_label 自定义标题",
            "3. 初始右侧数据集合",
            "   add_right_item(name, disabled) 预置右侧项",
            "4. 显示搜索框",
            "   set_show_search(true) 各面板头部输入过滤",
            "5. 数据格式解析",
            "   add_item(name, disabled)（disabled 项不可选中/搬移）",
            "6. 穿梭时的回调",
            "   set_on_change(方向, 搬移数据) 穿梭回调",
            "7. 实例调用",
            "   right_items() 取右侧数据（getData）；clear() + 重建（reload）",
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

void build_transfer(PageHost& host) {
    build_demo1(host);
    build_demo2(host);
    build_demo3(host);
    build_demo4(host);
    build_demo5(host);
    build_demo6(host);
    build_demo7(host);
}

}  // namespace

static void build(PageHost& host) {
    build_transfer(host);
}

static const PageDef kTransferPage{56, "交互", "穿梭框", "穿梭框：待选/已选搬移", build, nullptr, false, nullptr, 0, "Transfer"};

static const bool kRegistered = []() {
    page::register_page(kTransferPage);
    return true;
}();