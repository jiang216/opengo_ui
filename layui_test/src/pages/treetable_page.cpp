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
        "树形表格 = 表格 + 树形层级：以缩进与树形符号呈现父子行。"
        "当前用 ui::Table 静态还原 layui treetable 官方示例，"
        "展开/折叠交互待 ui 库 Table 增加行层级能力后接入。");
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
        t->add_column("操作", 240.f);
        const char* rows[][4] = {
            {"数据管理", "", "", ""},
            {"　├─ 新增数据接口", "url", "增删改", "查看 编辑 删除"},
            {"　│　└─ 数据筛选", "url/filter", "查", "查看"},
            {"　├─ 修改数据接口", "url/:id", "改", "查看 编辑"},
            {"　└─ 删除数据接口", "url/:id", "删", "查看 删除"},
        };
        for (auto& r : rows) t->add_row({r[0], r[1], r[2], r[3]});
        host.content->add_child(t);
    }

    // ---- Fieldset 2: 组织架构 ----
    field_title(host, "组织架构");
    {
        ui::Table* t = new ui::Table(0, 0, kCW, 0.f);
        t->set_font(host.font);
        t->set_theme(host.theme);
        t->add_column("机构", 320.f);
        t->add_column("成员", 160.f);
        t->add_column("角色", 200.f);
        const char* rows[][3] = {
            {"总公司", "", ""},
            {"　├─ 研发部", "贤心", "负责人"},
            {"　│　├─ 前端组", "张爱玲", "组长"},
            {"　│　└─ 后端组", "许闲心", "组长"},
            {"　├─ 市场部", "Helen Keller", "负责人"},
            {"　└─ 人事部", "岳飞", "负责人"},
        };
        for (auto& r : rows) t->add_row({r[0], r[1], r[2]});
        host.content->add_child(t);
    }

    // ---- 底部引用块 ----
    auto* bq2 = new ui::BlockQuote(kCW, 0,
        "layui treetable 的异步数据、复选框联动、折叠记忆等能力，"
        "依赖数据表格模块的树形扩展，规划于 ui 库 Table 的后续迭代。");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/treetable_page.cpp：静态还原 layui treetable 示例\n"
            "\n"
            "ui::Table* t = new ui::Table(0, 0, w, 0.f);\n"
            "t->add_column(\"数据接口\", 320.f);\n"
            "t->add_column(\"参数\", 200.f);\n"
            "t->add_row({\"　├─ 新增数据接口\", \"url\", \"增删改\", \"查看 编辑 删除\"});\n"
            "\n"
            "// 层级用全角空格 + ├─/│/└─ 符号表达；\n"
            "// 后续：Table 行层级 + 折叠箭头列（规划中）");
    }
    if (host.help) {
        const char* lines[] = {
            "树形表格说明",
            "",
            "当前实现：ui::Table 静态表格 + 缩进树形符号还原官方示例。",
            "",
            "规划能力（ui 库 Table 迭代项）：",
            "  add_row 带层级 parent_index，自动渲染折叠箭头列；",
            "  点击箭头折叠/展开子树；复选框父子联动；异步懒加载子行。",
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

static const PageDef kTreetablePage{33, "展示", "树形表格", "树形表格：表格式树形层级（静态还原官方示例）", build, nullptr, false, nullptr, 0, "TreeTable"};

static const bool kRegistered = []() {
    page::register_page(kTreetablePage);
    return true;
}();
