#include "../page_registry.h"

#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/panel.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/tree.h"

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

ui::Tree* make_tree(PageHost& host, float h) {
    ui::Tree* t = new ui::Tree(0, 0, 380.f, h);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
    return t;
}

// 模拟 data（五级树：id/field/href/checked/spread/disabled/fixed）
void build_data(ui::Tree* t) {
    const int a1 = t->add_node({"一级1", 1, "name1", "", false, false, true, true});
    const int b11 = t->add_node({"二级1-1 可允许跳转", 3, "name11", "https://www.layui.com/", false, false, false, false}, a1);
    const int c113 = t->add_node({"三级1-1-3", 23, "", "", false, false, false, false}, b11);
    const int d1131 = t->add_node({"四级1-1-3-1", 24, "", "", false, false, false, false}, c113);
    t->add_node({"五级1-1-3-1-1", 30, "", "", false, false, false, false}, d1131);
    t->add_node({"五级1-1-3-1-2", 31, "", "", false, false, false, false}, d1131);
    const int c111 = t->add_node({"三级1-1-1", 7, "", "", false, false, false, false}, b11);
    t->add_node({"四级1-1-1-1 可允许跳转", 15, "", "https://www.layui.com/doc/", false, false, false, false}, c111);
    const int c112 = t->add_node({"三级1-1-2", 8, "", "", false, false, false, false}, b11);
    t->add_node({"四级1-1-2-1", 32, "", "", false, false, false, false}, c112);
    const int b12 = t->add_node({"二级1-2", 4, "", "", false, false, false, true}, a1);
    t->add_node({"三级1-2-1", 9, "", "", true, false, false, false}, b12);
    t->add_node({"三级1-2-2", 10, "", "", false, false, false, false}, b12);
    const int b13 = t->add_node({"二级1-3", 20, "", "", false, false, false, false}, a1);
    t->add_node({"三级1-3-1", 21, "", "", false, false, false, false}, b13);
    t->add_node({"三级1-3-2", 22, "", "", false, false, false, false}, b13);
    const int a2 = t->add_node({"一级2", 2, "", "", false, false, false, true});
    const int b21 = t->add_node({"二级2-1", 5, "", "", false, false, false, true}, a2);
    t->add_node({"三级2-1-1", 11, "", "", false, false, false, false}, b21);
    t->add_node({"三级2-1-2", 12, "", "", false, false, false, false}, b21);
    const int b22 = t->add_node({"二级2-2", 6, "", "", false, false, false, false}, a2);
    t->add_node({"三级2-2-1", 13, "", "", false, false, false, false}, b22);
    t->add_node({"三级2-2-2", 14, "", "", true, false, false, false}, b22);
    const int a3 = t->add_node({"一级3", 16, "", "", false, false, false, false});
    const int b31 = t->add_node({"二级3-1", 17, "", "", false, true, false, false}, a3);
    t->add_node({"三级3-1-1", 18, "", "", false, false, false, false}, b31);
    t->add_node({"三级3-1-2", 19, "", "", false, false, false, false}, b31);
    const int b32 = t->add_node({"二级3-2", 27, "", "", false, false, false, false}, a3);
    t->add_node({"三级3-2-1", 28, "", "", false, false, false, false}, b32);
    t->add_node({"三级3-2-2", 29, "", "", false, false, false, false}, b32);
}

// 模拟 data1（省份-城市）
void build_data1(ui::Tree* t) {
    const int jx = t->add_node({"江西", 1, "", "", false, false, false, true});
    const int nc = t->add_node({"南昌", 1000, "", "", false, false, false, false}, jx);
    t->add_node({"青山湖区", 10001, "", "", false, false, false, false}, nc);
    t->add_node({"高新区", 10002, "", "", false, false, false, false}, nc);
    t->add_node({"九江", 1001, "", "", false, false, false, false}, jx);
    t->add_node({"赣州", 1002, "", "", false, false, false, false}, jx);
    const int gx = t->add_node({"广西", 2, "", "", false, false, false, false});
    t->add_node({"南宁", 2000, "", "", false, false, false, false}, gx);
    t->add_node({"桂林", 2001, "", "", false, false, false, false}, gx);
    const int sx = t->add_node({"陕西", 3, "", "", false, false, false, false});
    t->add_node({"西安", 3000, "", "", false, false, false, false}, sx);
    t->add_node({"延安", 3001, "", "", false, false, false, false}, sx);
}

// 模拟 data2（餐饮 checked/disabled/fixed）
void build_data2(ui::Tree* t) {
    const int zc = t->add_node({"早餐", 1, "", "", false, false, false, true});
    t->add_node({"油条", 5, "", "", false, false, false, false}, zc);
    t->add_node({"包子", 6, "", "", false, false, false, false}, zc);
    t->add_node({"豆浆", 7, "", "", false, false, false, false}, zc);
    const int wc = t->add_node({"午餐", 2, "", "", false, false, true, true});
    t->add_node({"藜蒿炒腊肉", 8, "", "", false, false, false, false}, wc);
    t->add_node({"西湖醋鱼", 9, "", "", false, false, false, false}, wc);
    t->add_node({"小白菜", 10, "", "", false, false, false, false}, wc);
    t->add_node({"海带排骨汤", 11, "", "", false, false, false, false}, wc);
    const int wan = t->add_node({"晚餐", 3, "", "", false, false, false, false});
    t->add_node({"红烧肉", 12, "", "", false, true, false, false}, wan);
    t->add_node({"番茄炒蛋", 13, "", "", false, false, false, false}, wan);
    const int ye = t->add_node({"夜宵", 4, "", "", false, false, false, false});
    t->add_node({"小龙虾", 14, "", "", false, false, true, false}, ye);
    t->add_node({"香辣蟹", 15, "", "", true, false, false, false}, ye);
    t->add_node({"烤鱿鱼", 16, "", "", false, false, false, false}, ye);
}

void add_btn_group(PageHost& host, const std::vector<std::pair<const char*, std::function<void()>>>& btns) {
    ui::HBox* row = new ui::HBox(0, 0, 800, 40.f);
    row->set_padding(0.f);
    row->set_spacing(12.f);
    row->set_valign(ui::Valign::Center);
    row->set_theme(host.theme);
    for (auto& [txt, fn] : btns) {
        ui::Button* b = new ui::Button(0, 0, 0.f, 38.f, txt);
        b->set_font(host.font);
        b->set_theme(host.theme);
        b->add_class("layui-btn");
        b->set_on_click(std::move(fn));
        row->add_child(b);
    }
    host.content->add_child(row);
}

// ── 1. 基本演示（test12：showCheckbox + isJump + 按钮）─────────────────────────
void build_demo1(PageHost& host) {
    add_fieldset_title(host, "基本演示");
    ui::Tree* t = make_tree(host, 320.f);
    t->set_show_checkbox(true);
    t->set_is_jump(true);
    build_data(t);
    ui::Label* status = new ui::Label(0, 4, 800, 0, "选中节点：无");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    t->set_on_click([status](int idx) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "选中节点：id 已回调（节点 %d）", idx);
        status->set_text(buf);
    });
    add_btn_group(host, {
        {"获取选中节点数据", [t, status]() {
            const auto chk = t->get_checked();
            char buf[128];
            std::snprintf(buf, sizeof(buf), "选中节点数据：%d 项已勾选", (int)chk.size());
            status->set_text(buf);
        }},
        {"勾选指定节点", [t]() {
            t->set_checked(0, true);  // 勾选一级1（含子孙传播）
        }},
        {"重载实例", [t]() {
            t->expand_all(true);
        }},
    });
}

// ── 2. 常规用法（test1：点击节点收缩）─────────────────────────────────────────
void build_demo2(PageHost& host) {
    add_fieldset_title(host, "常规用法");
    ui::Tree* t = make_tree(host, 260.f);
    build_data1(t);
}

// ── 3. 无连接线风格（test13：showLine=false）──────────────────────────────────
void build_demo3(PageHost& host) {
    add_fieldset_title(host, "无连接线风格");
    ui::Tree* t = make_tree(host, 260.f);
    t->set_show_line(false);
    build_data1(t);
}

// ── 4. 仅节点左侧图标控制收缩（test2：onlyIconControl）────────────────────────
void build_demo4(PageHost& host) {
    add_fieldset_title(host, "仅节点左侧图标控制收缩");
    ui::Tree* t = make_tree(host, 260.f);
    t->set_only_icon_control(true);
    build_data1(t);
}

// ── 5. 手风琴模式（test4：accordion）──────────────────────────────────────────
void build_demo5(PageHost& host) {
    add_fieldset_title(host, "手风琴模式");
    ui::Tree* t = make_tree(host, 220.f);
    t->set_accordion(true);
    const int you = t->add_node({"优秀", 0});
    t->add_node({"80 ~ 90", 0}, you);
    t->add_node({"90 ~ 100", 0}, you);
    const int liang = t->add_node({"良好", 0});
    t->add_node({"70 ~ 80", 0}, liang);
    t->add_node({"60 ~ 70", 0}, liang);
    const int nu = t->add_node({"要努力奥", 0});
    t->add_node({"0 ~ 60", 0}, nu);
}

// ── 6. 点击节点新窗口跳转（test5：isJump + href）──────────────────────────────
void build_demo6(PageHost& host) {
    add_fieldset_title(host, "点击节点新窗口跳转");
    ui::Tree* t = make_tree(host, 320.f);
    t->set_is_jump(true);
    build_data(t);
    ui::Label* status = new ui::Label(0, 4, 800, 0, "isJump：点击带 href 节点显示跳转目标");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text_muted);
    host.content->add_child(status);
}

// ── 7. 开启复选框（test7：showCheckbox）───────────────────────────────────────
void build_demo7(PageHost& host) {
    add_fieldset_title(host, "开启复选框");
    ui::Tree* t = make_tree(host, 260.f);
    t->set_show_checkbox(true);
    build_data2(t);
}

// ── 8. 开启节点操作图标（test9：edit add/update/del）─────────────────────────
void build_demo8(PageHost& host) {
    add_fieldset_title(host, "开启节点操作图标");
    ui::Tree* t = make_tree(host, 320.f);
    t->set_editable({ui::TreeEdit::Add, ui::TreeEdit::Update, ui::TreeEdit::Del});
    build_data1(t);
    ui::Label* status = new ui::Label(0, 4, 800, 0, "操作图标：未触发");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    t->set_on_edit([status](int idx, ui::TreeEdit act) {
        const char* name = act == ui::TreeEdit::Add ? "add" :
                           act == ui::TreeEdit::Update ? "update" : "del";
        char buf[128];
        std::snprintf(buf, sizeof(buf), "操作图标：%s（节点 %d）", name, idx);
        status->set_text(buf);
    });
}

void build_tree(PageHost& host) {
    build_demo1(host);
    build_demo2(host);
    build_demo3(host);
    build_demo4(host);
    build_demo5(host);
    build_demo6(host);
    build_demo7(host);
    build_demo8(host);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/tree_page.cpp — 一比一克隆 layui tree.html 全部 8 个 demo\n"
            "\n"
            "// 1. 基本演示：复选框 + 跳转 + 按钮（getChecked/setChecked/reload）\n"
            "ui::Tree* t = new ui::Tree(0, 0, 380.f, 320.f);\n"
            "t->set_show_checkbox(true); t->set_is_jump(true);\n"
            "t->set_on_click([](int idx){});\n"
            "auto chk = t->get_checked(); t->set_checked(0, true); t->expand_all(true);\n"
            "\n"
            "// 2. 常规用法：点击节点收缩（默认）\n"
            "// 3. 无连接线风格：t->set_show_line(false);\n"
            "// 4. 仅图标控制收缩：t->set_only_icon_control(true);\n"
            "// 5. 手风琴模式：t->set_accordion(true);\n"
            "// 6. 点击跳转：t->set_is_jump(true);\n"
            "// 7. 开启复选框：t->set_show_checkbox(true);\n"
            "// 8. 节点操作图标：t->set_editable({TreeEdit::Add,Update,Del});\n"
            "t->set_on_edit([](int idx, ui::TreeEdit a){});\n"
            "// 节点数据：add_node(TreeNodeSpec{title,id,field,href,disabled,fixed,checked,spread}, parent)");
    }
    if (host.help) {
        const char* lines[] = {
            "树形组件（Tree）API — 一比一克隆 tree.html 全部 8 个 demo",
            "",
            "1. 基本演示：复选框 + 跳转 + getChecked/setChecked/reload",
            "   set_show_checkbox / set_is_jump / get_checked / set_checked / expand_all",
            "2. 常规用法：点击节点收缩（默认）",
            "3. 无连接线风格：set_show_line(false)",
            "4. 仅节点左侧图标控制收缩：set_only_icon_control(true)",
            "5. 手风琴模式：set_accordion(true)（展开收起同级兄弟）",
            "6. 点击节点新窗口跳转：set_is_jump(true) + add_node(spec 带 href)",
            "7. 开启复选框：set_show_checkbox(true)；半选父级呈 -",
            "8. 节点操作图标：set_editable({Add,Update,Del}) + set_on_edit",
            "节点数据：add_node(TreeNodeSpec, parent) / add_node(text, parent)",
            "set_node_id / set_node_href / set_node_disabled / set_node_checked",
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
    build_tree(host);
}

static const PageDef kTreePage{44, "展示", "树形组件", "树形组件：基本/复选框/手风琴/操作图标", build, nullptr, false, nullptr, 0, "Tree"};

static const bool kRegistered = []() {
    page::register_page(kTreePage);
    return true;
}();