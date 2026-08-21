#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/table.h"
#include "ui/ui/theme.h"

namespace {

constexpr float kCW = 1000.f;

void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

void build_treetable(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "树形表格 = 表格 + 树形层级：add_tree_row(depth, cells) 追加带层级行，"
        "有子级的行首列渲染展开三角（▼ 展开 / ▶ 折叠），点击切换、整棵子树随之显隐。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- Fieldset 1: 数据接口 ----
    field_title(host, "数据接口");
    {
        ui::Table* t = new ui::Table(0, 0, kCW, 0.f);
        t->set_font(host.font);
        t->set_theme(host.theme);
        t->add_column("数据接口", 320.f);
        t->add_column("参数", 200.f);
        t->add_column("权限", 160.f);
        t->add_tree_row(0, {"数据管理", "", ""});
        t->add_tree_row(1, {"新增数据接口", "url", "增删改"});
        t->add_tree_row(2, {"数据筛选", "url/filter", "查"});
        t->add_tree_row(1, {"修改数据接口", "url/:id", "改"});
        t->add_tree_row(1, {"删除数据接口", "url/:id", "删"});
        host.content->add_child(t);
    }

    // ---- Fieldset 2: 组织架构（含展开/折叠状态提示）----
    field_title(host, "组织架构");
    {
        ui::Label* status = new ui::Label(0, 0, kCW, 0, "点击行首三角可展开/折叠子树。");
        status->set_font(host.font);
        status->set_theme(host.theme);
        status->set_color(host.theme->text);

        ui::Table* t = new ui::Table(0, 0, kCW, 0.f);
        t->set_font(host.font);
        t->set_theme(host.theme);
        t->add_column("机构", 320.f);
        t->add_column("成员", 160.f);
        t->add_column("角色", 200.f);
        const char* names[] = {"总公司", "研发部", "前端组", "后端组", "市场部", "人事部"};
        int d = 0;
        t->add_tree_row(d = 0, {names[0], "", ""});
        t->add_tree_row(d = 1, {names[1], "贤心", "负责人"});
        t->add_tree_row(d = 2, {names[2], "张爱玲", "组长"});
        t->add_tree_row(d = 2, {names[3], "许闲心", "组长"});
        t->add_tree_row(d = 1, {names[4], "Helen Keller", "负责人"});
        t->add_tree_row(d = 1, {names[5], "岳飞", "负责人"});
        t->set_on_expand([&host, status, names](int data_row, bool expanded) {
            const char* n = (data_row >= 0 && data_row < 6) ? names[data_row] : "?";
            std::string msg = std::string(expanded ? "已展开：" : "已折叠：") + n +
                              (expanded ? "（子树显示）" : "（子树隐藏）") +
                              "   [data_row=" + std::to_string(data_row) + "]";
            status->set_text(msg.c_str());
            (void)host;
        });
        host.content->add_child(status);
        host.content->add_child(t);
    }

    // ---- 底部引用块 ----
    auto* bq2 = new ui::BlockQuote(kCW, 0,
        "layui treetable 的异步懒加载子行、复选框父子联动等能力，"
        "规划于 ui 库 Table 的后续迭代。");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/treetable_page.cpp：真实树形层级（UI-109）\n"
            "\n"
            "ui::Table* t = new ui::Table(0, 0, w, 0.f);\n"
            "t->add_column(\"机构\", 320.f);\n"
            "t->add_tree_row(0, {\"总公司\", \"\", \"\"});       // 根\n"
            "t->add_tree_row(1, {\"研发部\", \"贤心\", \"负责人\"}); // 子\n"
            "t->add_tree_row(2, {\"前端组\", \"张爱玲\", \"组长\"}); // 孙\n"
            "\n"
            "// 有子级的行首列自动渲染展开三角，点击切换：\n"
            "t->set_on_expand([](int data_row, bool expanded) {\n"
            "    // 展开/折叠回调\n"
            "});");
    }
    if (host.help) {
        const char* lines[] = {
            "树形表格说明",
            "",
            "ui::Table 树形能力（UI-109）：",
            "  add_tree_row(depth, cells)：按先序追加带层级行，depth 0=根；",
            "  有子级的行首列渲染展开三角（▼ 展开 / ▶ 折叠），点击整棵子树显隐；",
            "  set_on_expand(cb)：展开/折叠回调（数据行下标, 展开态）；",
            "  set_row_expanded(data_row, on)：程序化设置展开态；",
            "  分页在树形过滤结果上切片（折叠的子树不计入页数）；",
            "  树形模式禁用排序（扁平重排会破坏层级先序）。",
            "",
            "规划能力：异步懒加载子行、复选框父子联动、折叠状态记忆。",
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

static void build(PageHost& host) { build_treetable(host); }

static const PageDef kTreetablePage{33, "展示", "树形表格", "树形表格：表格式树形层级（展开/折叠交互）", build, nullptr, false, nullptr, 0, "TreeTable"};

static const bool kRegistered = []() {
    page::register_page(kTreetablePage);
    return true;
}();
