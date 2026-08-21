#include "../page_registry.h"

#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/table.h"
#include "ui/ui/theme.h"

#include <string>
#include <vector>

namespace {

// 小节标题：带 legend 横线（layui-elem-field layui-field-title）
void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

// 顶部引用块：本页面只是演示静态表格……（layui-elem-quote）
void top_quote(PageHost& host) {
    ui::Label* q = new ui::Label(0, 0, 800, 0,
                                 "本页面只是演示静态表格，如果你需要的是数据表格，可前往：表格模块示例");
    q->set_font(host.font);
    q->set_theme(host.theme);
    q->set_color(host.theme->text_secondary);
    host.content->add_child(q);
}

// 人物表四列：人物/民族/出场时间/格言（镜像五条数据）
static const char* kRows[5][4] = {
    {"贤心", "汉族", "1989-10-14", "人生似修行"},
    {"张爱玲", "汉族", "1920-09-30", "于千万人之中遇见你所遇见的人，于千万年之中，时间的无涯的荒野里…"},
    {"Helen Keller", "拉丁美裔", "1880-06-27", " Life is either a daring adventure or nothing."},
    {"岳飞", "汉族", "1103-北宋崇宁二年", "教科书再滥改，也抹不去“民族英雄”的事实"},
    {"孟子", "华夏族（汉族）", "公元前-372年", "猿强，则国强。国强，则猿更强！ "},
};

void fill_person_table(ui::Table* t) {
    t->add_column("人物", 150.f);
    t->add_column("民族", 150.f);
    t->add_column("出场时间", 200.f);
    t->add_column("格言", 280.f);
    for (int i = 0; i < 5; ++i)
        t->add_row({kRows[i][0], kRows[i][1], kRows[i][2], kRows[i][3]});
}

// 尺寸表三列：昵称/加入时间/签名（镜像三条数据）
static const char* kSizeRows[3][3] = {
    {"贤心", "2016-11-29", "人生就像是一场修行"},
    {"许闲心", "2016-11-28", "于千万人之中遇见你所遇见的人，于千万年之中，时间的无涯的荒野里…"},
    {"sentsin", "2016-11-27", " Life is either a daring adventure or nothing."},
};

void fill_size_table(ui::Table* t) {
    t->add_column("昵称", 150.f);
    t->add_column("加入时间", 200.f);
    t->add_column("签名", 300.f);
    for (int i = 0; i < 3; ++i)
        t->add_row({kSizeRows[i][0], kSizeRows[i][1], kSizeRows[i][2]});
}

void build_table(PageHost& host) {
    top_quote(host);

    // 默认表格（layui-table 默认：完整网格）
    field_title(host, "默认表格");
    {
        ui::Table* t = new ui::Table(0, 0, 800, 0.f);
        t->set_font(host.font);
        t->set_theme(host.theme);
        fill_person_table(t);
        host.content->add_child(t);
    }

    // 行边框表格（lay-skin="line"）
    field_title(host, "行边框表格");
    {
        ui::Table* t = new ui::Table(0, 0, 800, 0.f);
        t->set_font(host.font);
        t->set_theme(host.theme);
        t->set_skin(ui::TableSkin::Line);
        fill_person_table(t);
        host.content->add_child(t);
    }

    // 列边框表格（lay-even lay-skin="row"）
    field_title(host, "列边框表格");
    {
        ui::Table* t = new ui::Table(0, 0, 800, 0.f);
        t->set_font(host.font);
        t->set_theme(host.theme);
        t->set_skin(ui::TableSkin::Row);
        t->set_even(true);
        fill_person_table(t);
        host.content->add_child(t);
    }

    // 无边框表格（lay-even lay-skin="nob"）
    field_title(host, "无边框表格");
    {
        ui::Table* t = new ui::Table(0, 0, 800, 0.f);
        t->set_font(host.font);
        t->set_theme(host.theme);
        t->set_skin(ui::TableSkin::Nob);
        t->set_even(true);
        fill_person_table(t);
        host.content->add_child(t);
    }

    // 其它尺寸表格（lay-size="lg" 大 / lay-size="sm" 小）
    field_title(host, "其它尺寸表格");
    {
        ui::Table* t = new ui::Table(0, 0, 800, 0.f);
        t->set_font(host.font);
        t->set_theme(host.theme);
        t->set_size(ui::TableSize::Lg);
        fill_size_table(t);
        host.content->add_child(t);

        ui::Table* t2 = new ui::Table(0, 0, 800, 0.f);
        t2->set_font(host.font);
        t2->set_theme(host.theme);
        t2->set_size(ui::TableSize::Sm);
        fill_size_table(t2);
        host.content->add_child(t2);
    }

    // 结尾引用块
    ui::Label* note = new ui::Label(0, 0, 800, 0,
                                    "更多我们未能呈现的示例，还有待您在阅读文档、以及不断使用的过程，去深入挖掘。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_secondary);
    host.content->add_child(note);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/table_page.cpp：克隆 layui table-element.html（静态表格）五组 demo\n"
            "\n"
            "ui::Table* t = new ui::Table(0, 0, 800, 0.f);\n"
            "t->set_font(host.font); t->set_theme(host.theme);\n"
            "t->set_skin(ui::TableSkin::Line);   // Default / Line / Row / Nob\n"
            "t->set_even(true);                   // 偶数行 #FAFAFA（lay-even）\n"
            "t->set_size(ui::TableSize::Lg);      // Normal / Lg / Sm\n"
            "t->add_column(\"人物\", 150.f);\n"
            "t->add_row({\"贤心\", \"汉族\", \"1989-10-14\"});\n"
            "host.content->add_child(t);");
    }
    if (host.help) {
        const char* lines[] = {
            "静态表格（Table）API",
            "  add_column(title, width[, sortable])：添加列",
            "  add_row(cells) / clear_rows()：数据行增删",
            "  set_skin(TableSkin)：Default 网格 / Line 仅行线 / Row 仅列线 / Nob 无内线",
            "  set_even(bool)：偶数行 #FAFAFA 着色（lay-even）",
            "  set_size(TableSize)：Normal 38 / Lg 50 / Sm 30（行高与左右内边距）",
            "交互：行 hover 高亮、表头排序、双击单元格行内编辑、纵向滚动",
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
    build_table(host);
}

static const PageDef kTablePage{30, "展示", "静态表格", "静态表格：默认/行边框/列边框/无边框/尺寸（镜像 demo/table-element.html）", build, nullptr, false, nullptr, 0, "Table"};

static const bool kRegistered = []() {
    page::register_page(kTablePage);
    return true;
}();