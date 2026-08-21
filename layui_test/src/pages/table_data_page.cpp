#include "../page_registry.h"

#include "ui/ui/code.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/table.h"
#include "ui/ui/theme.h"

#include <vector>

namespace {

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

void build_table_data(PageHost& host) {
    add_section(host, "数据表格（table 模块：本地数据 + 排序 / 行事件 / 双击编辑）");
    ui::Label* status = new ui::Label(0, 0, 800, 0, "未选中行");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);

    {
        ui::Table* table = new ui::Table(0, 0, 800, 340.f);
        table->set_font(host.font);
        table->set_theme(host.theme);
        table->add_column("ID", 60.f, true);
        table->add_column("用户名", 90.f);
        table->add_column("性别", 60.f, true);
        table->add_column("城市", 90.f);
        table->add_column("签名", 170.f);
        table->add_column("积分", 70.f, true);
        table->add_column("评分", 70.f, true);
        table->add_column("职业", 90.f);
        table->add_column("财富", 90.f, true);

        const std::vector<std::vector<std::string>> rows = {
            {"1", "贤心", "男", "海南", "好 好 学 习", "70", "4.5", "前端", "30000"},
            {"2", "张三", "男", "浙江", "一层一层一层地嵌套", "150", "3.8", "后端", "50000"},
            {"3", "李四", "男", "北京", "吃兔兔", "80", "4.0", "产品", "42000"},
            {"4", "王五", "女", "上海", "设计源于生活", "90", "4.9", "设计", "36000"},
            {"5", "赵六", "男", "广州", "加油鸭", "55", "3.5", "测试", "28000"},
            {"6", "孙七", "女", "深圳", "奋斗每一天", "120", "4.7", "运营", "33000"},
            {"7", "周八", "男", "成都", "不急不躁", "65", "3.9", "前端", "31000"},
            {"8", "吴九", "女", "杭州", "代码是诗", "110", "4.6", "后端", "48000"},
        };
        for (const auto& row : rows) table->add_row(row);

        table->set_on_sort([](int col, bool asc) {
            (void)col; (void)asc;  // 表头排序已由控件内部完成（UI-031）
        });
        table->set_on_row_click([status](int row) {
            char buf[48];
            snprintf(buf, sizeof(buf), "点击第 %d 行（row click 事件）", row + 1);
            status->set_text(buf);
        });
        host.content->add_child(table);
    }

    add_section(host, "说明");
    ui::Label* note = new ui::Label(0, 0, 800, 0,
                                    "克隆 layui table.html：ID/积分/评分/财富/性别 等列可排序；"
                                    "行点击触发 row click 事件；双击单元格进入行内编辑。"
                                    "官方示例为分页接口数据，此处用本地数据演示表格模块能力。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/table_data_page.cpp（节选）：克隆 layui table.html（数据表格）\n"
            "\n"
            "ui::Table* table = new ui::Table(0, 0, 800, 340.f);\n"
            "table->add_column(\"ID\", 60.f, true);      // sortable 可排序\n"
            "table->add_column(\"用户名\", 90.f);\n"
            "table->add_row({\"1\", \"贤心\", \"男\", \"海南\", \"签名\", \"70\", \"4.5\", \"前端\", \"30000\"});\n"
            "table->set_on_row_click([](int row) { /* row click 事件 */ });\n"
            "host.content->add_child(table);\n"
            "// 官方 table.render 走分页接口；本页为本地数据静态展示");
    }
    if (host.help) {
        const char* lines[] = {
            "数据表格（Table）API",
            "  add_column(title, width, sortable)：添加列",
            "  add_row(cells) / clear_rows()：数据行增删",
            "  set_on_sort(cb) / set_on_row_click(cb)：排序与行点击回调",
            "交互：表头排序（升/降箭头变色）、行 hover 高亮、双击单元格行内编辑、纵向滚动",
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

static void build_simple_data(PageHost& host) {
    build_table_data(host);  // 镜像 "简单数据表格"（table.html 首页 demo：排序/行事件/双击编辑）
}

// 板块锚点页：「数据表格」在主菜单只占一项，选中时 app 展开第二列子菜单
static void build_anchor(PageHost&) {}

static const PageDef kTableDataAnchor{31, "展示", "数据表格",
    "数据表格模板入口，点击展开第二列子菜单（镜像 site-demo-table-nav，22 个 demo）",
    build_anchor, nullptr, true, "数据表格", 0, "Table"};

// 镜像 "简单数据表格" 子页（table.html 首页）：排序 / 行事件 / 双击编辑
static const PageDef kSimpleData{100, "组件示例", "简单数据表格",
    "镜像 demo/table.html：排序/行事件/双击编辑", build_simple_data, nullptr, false, "数据表格", 1};

static const bool kRegistered = []() {
    page::register_page(kTableDataAnchor);
    page::register_page(kSimpleData);
    return true;
}();
