#include "../page_registry.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/grid.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_style.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/widget.h"

namespace {

// 依据类名解析演示色块填充色：layui-bg-* 工具色优先，其次 grid-demo-bg1..3，
// 兜底 grid-demo 基础色 #79C48C（克隆自 global.css / layui.css）
ui::Vec4 demo_fill(const ui::Widget* w) {
    if (w->has_class("grid-demo-bg3")) return ui::layui::hex("#38814A");
    if (w->has_class("grid-demo-bg2")) return ui::layui::hex("#49A761");
    if (w->has_class("grid-demo-bg1")) return ui::layui::hex("#63BA79");
    for (const std::string& cls : w->classes())
        if (cls.rfind("layui-bg-", 0) == 0) return ui::layui::bg_fill(cls.c_str());
    if (w->has_class("grid-demo")) return ui::layui::hex("#79C48C");
    return ui::Vec4{1, 1, 1, 1};
}

// 栅格演示单元格：填充色块 + 居中白字（可换行），子 Label 垂直居中铺满
class Cell : public ui::Widget {
public:
    Cell(float h, const char* cls) : ui::Widget(0, 0, 0, h) {
        if (cls) add_class(cls);
    }
    float preferred_height() const override { return bounds_.h; }
    void on_layout() override {
        for (ui::Widget* c : children()) {
            const float ch = c->preferred_height();
            c->set_bounds(bounds_.x, bounds_.y + (bounds_.h - ch) * 0.5f, bounds_.w, ch);
        }
    }
    void on_draw(ui::Renderer& renderer) override {
        renderer.draw_quad(bounds_, demo_fill(this));
    }
};

// layui-row 行容器：无背景由 ui 库 Grid 的 layui-row 类实现（UI-039）

// 栅格嵌套用容器：作为外层列（layui-col-md*），填充 grid-demo 底色，内部再放子栅格
class NestedGrid : public ui::Grid {
public:
    using ui::Grid::Grid;
    void on_draw(ui::Renderer& renderer) override {
        renderer.draw_quad(bounds_, demo_fill(this));
    }
};

Cell* grid_cell(PageHost& host, const char* text, const char* cls, float h = 70.f) {
    Cell* p = new Cell(h, cls);
    p->add_class("grid-demo");  // 基础色 #79C48C：未显式着色的格子也填充（对齐 layui 文档）
    ui::Label* label = new ui::Label(0, 0, 0, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(ui::Vec4{1, 1, 1, 1});
    label->set_align(1);
    label->set_wrap(true);
    p->add_child(label);
    return p;
}

// 新建一行（可选 layui-col-space{n} 列间隔类），返回供填充单元格
ui::Grid* make_row(PageHost& host, const char* space_cls = nullptr) {
    ui::Grid* g = new ui::Grid(0, 0, 800, 40.f, 12);
    g->add_class("layui-row");  // layui-row：ui 库 Grid 据此不画背景（UI-039）
    g->set_theme(host.theme);
    g->set_padding(0.f);
    g->set_spacing(10.f);
    g->set_row_height(70.f);
    if (space_cls) g->add_class(space_cls);
    host.content->add_child(g);
    return g;
}

// 小节标题（layui-field-title 克隆由 ui 库 FieldsetTitle 提供，UI-039）
ui::FieldsetTitle* field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* ft = new ui::FieldsetTitle(800, 36, text);
    ft->set_font(host.font);
    ft->set_theme(host.theme);
    host.content->add_child(ft);
    return ft;
}

// 提示块（layui-elem-quote 克隆由 ui 库 BlockQuote 提供，UI-039）
ui::BlockQuote* quote(PageHost& host, const char* text) {
    ui::BlockQuote* q = new ui::BlockQuote(800, 0, text);
    q->set_font(host.font);
    q->set_theme(host.theme);
    host.content->add_child(q);
    return q;
}

// 栅格嵌套演示：外层列内嵌子栅格（显式 md 断点：内部 md3/md9/md12 横向排布，对齐 layui 视口语义）
ui::Grid* nested_cell(PageHost& host, const char* cls) {
    NestedGrid* inner = new NestedGrid(0, 0, 0, 0, 12);
    inner->set_theme(host.theme);
    inner->set_padding(10.f);  // layui .grid-demo 内衬
    inner->set_spacing(10.f);
    inner->set_row_height(70.f);
    inner->set_breakpoint(2);  // md
    if (cls) inner->add_class(cls);
    return inner;
}

// 查看代码 tab：本页 C++ 实现节选
const char* kGridCode =
    "// pages/grid_page.cpp（节选）：克隆 layui grid.html 的 8 组栅格演示\n"
    "// 断点由内容区宽度自动推断：>=1200 lg / >=992 md / >=768 sm / 其余 xs\n"
    "\n"
    "// 始终等比例水平排列（xs 恒等分，不随断点变化）\n"
    "ui::Grid* row = make_row(host);\n"
    "row->add_child(grid_cell(host, \"6/12\", \"layui-col-xs6 grid-demo-bg1\"));\n"
    "row->add_child(grid_cell(host, \"6/12\", \"layui-col-xs6\"));\n"
    "\n"
    "// 组合响应式：移动 xs 满行 / 桌面 md 按比例\n"
    "row = make_row(host);\n"
    "row->add_child(grid_cell(host, \"移动：12/12、桌面：8/12\",\n"
    "                         \"layui-col-xs12 layui-col-md8 grid-demo-bg1\"));\n"
    "row->add_child(grid_cell(host, \"移动：6/12、桌面：4/12\",\n"
    "                         \"layui-col-xs6 layui-col-md4\"));\n"
    "\n"
    "// 列间隔：layui-col-space10（行类名，列宽扣 n、列间留 n px 间隙，首末列贴边）\n"
    "row = make_row(host, \"layui-col-space10\");\n"
    "for (int i = 0; i < 2; ++i)\n"
    "    row->add_child(grid_cell(host, i % 2 ? \"3/12\" : \"9/12\",\n"
    "                             i % 2 ? \"layui-col-md3\" : \"layui-col-md9 grid-demo-bg1\"));\n"
    "\n"
    "// 列偏移：layui-col-md-offset4（12 列基准左移）\n"
    "row = make_row(host);\n"
    "row->add_child(grid_cell(host, \"4/12\", \"layui-col-md4 grid-demo-bg1\"));\n"
    "row->add_child(grid_cell(host, \"偏移4列\", \"layui-col-md4 layui-col-md-offset4\"));\n"
    "\n"
    "// 栅格嵌套：外层列（layui-col-md5）内再放一个 12 列子栅格（显式 set_breakpoint(md)）\n"
    "row = make_row(host);\n"
    "ui::Grid* inner = nested_cell(host, \"layui-col-md5 grid-demo grid-demo-bg1\");\n"
    "inner->add_child(grid_cell(host, \"内部列\", \"layui-col-md3 grid-demo-bg1\"));\n"
    "inner->add_child(grid_cell(host, \"内部列\", \"layui-col-md9 grid-demo-bg2\"));\n"
    "row->add_child(inner);\n";

// 帮助 tab：Grid API 说明
void build_help(PageHost& host) {
    const char* lines[] = {
        "栅格（Grid）API",
        "  Grid(x, y, w, h, cols)：12 列基准容器，等价于 .layui-row",
        "  add_child(...)：加入列控件，行内 span 满 12 自动换行",
        "列类（加在子控件上）：",
        "  layui-col-xs{n}  /  sm / md / lg：占 n/12 宽",
        "  layui-col-md-offset{n}：左移 n/12（offset 计入已用列数）",
        "  layui-col-space{n}（加在行上）：列间隙 n px",
        "断点：xs 无条件基础档，sm(>=768)/md(>=992)/lg(>=1200) 依次覆盖；",
        "  当前档取「断点及以下最高已定义档」。",
        "  Grid 未手动 set_breakpoint 时按自身宽度自动推断。",
    };
    for (const char* line : lines) {
        ui::Label* label = new ui::Label(0, 0, 800, 0, line);
        label->set_font(host.font);
        label->set_theme(host.theme);
        label->set_color(host.theme->text);
        host.help->add_child(label);
    }
}

void build_grid(PageHost& host) {
    host.content->set_spacing(10.f);  // 对齐 layui 文档 .layui-row margin-bottom:10px（切页时框架复位）

    quote(host, "注意：下述演示中的颜色只是做一个区分作用，并非栅格内置。");

    // 1. 始终等比例水平排列
    field_title(host, "始终等比例水平排列");
    {
        ui::Grid* g = make_row(host);
        g->add_child(grid_cell(host, "6/12", "layui-col-xs6 grid-demo-bg1"));
        g->add_child(grid_cell(host, "6/12", "layui-col-xs6"));
    }
    {
        ui::Grid* g = make_row(host);
        const char* cls[] = {"layui-col-xs3 grid-demo-bg1", "layui-col-xs3",
                             "layui-col-xs3 grid-demo-bg1", "layui-col-xs3"};
        for (const char* c : cls) g->add_child(grid_cell(host, "3/12", c));
    }

    // 2. 移动设备、桌面端的组合响应式展现
    field_title(host, "移动设备、桌面端的组合响应式展现");
    {
        ui::Grid* g = make_row(host);
        g->add_child(grid_cell(host, "移动：12/12、桌面：8/12",
                               "layui-col-xs12 layui-col-md8 grid-demo-bg1"));
        g->add_child(grid_cell(host, "移动：6/12、桌面：4/12",
                               "layui-col-xs6 layui-col-md4"));
        g->add_child(grid_cell(host, "移动：6/12、桌面：12/12",
                               "layui-col-xs6 layui-col-md12 grid-demo-bg2"));
    }

    // 3. 移动设备、平板、桌面端的复杂组合响应式展现
    field_title(host, "移动设备、平板、桌面端的复杂组合响应式展现");
    {
        ui::Grid* g = make_row(host);
        g->add_child(grid_cell(host, "移动：6/12 | 平板：6/12 | 桌面：4/12",
                               "layui-col-xs6 layui-col-sm6 layui-col-md4 grid-demo-bg1"));
        g->add_child(grid_cell(host, "移动：6/12 | 平板：6/12 | 桌面：4/12",
                               "layui-col-xs6 layui-col-sm6 layui-col-md4 layui-bg-red"));
        g->add_child(grid_cell(host, "移动：4/12 | 平板：12/12 | 桌面：4/12",
                               "layui-col-xs4 layui-col-sm12 layui-col-md4 layui-bg-blue"));
        g->add_child(grid_cell(host, "移动：4/12 | 平板：7/12 | 桌面：8/12",
                               "layui-col-xs4 layui-col-sm7 layui-col-md8 layui-bg-green"));
        g->add_child(grid_cell(host, "移动：4/12 | 平板：5/12 | 桌面：4/12",
                               "layui-col-xs4 layui-col-sm5 layui-col-md4 layui-bg-black"));
    }

    // 4. 常规布局：从小屏幕堆叠到桌面水平排列
    field_title(host, "常规布局：从小屏幕堆叠到桌面水平排列");
    {
        ui::Grid* g = make_row(host);
        for (int i = 0; i < 12; ++i)
            g->add_child(grid_cell(host, "1/12", i % 2 ? "layui-col-md1" : "layui-col-md1 grid-demo-bg1"));
    }
    {
        ui::Grid* g = make_row(host);
        g->add_child(grid_cell(host, "75%", "layui-col-md9 grid-demo-bg1"));
        g->add_child(grid_cell(host, "25%", "layui-col-md3"));
    }
    {
        ui::Grid* g = make_row(host);
        const char* cls[] = {"layui-col-md4 grid-demo-bg1", "layui-col-md4", "layui-col-md4 grid-demo-bg1"};
        for (const char* c : cls) g->add_child(grid_cell(host, "33.33%", c));
    }
    {
        ui::Grid* g = make_row(host);
        g->add_child(grid_cell(host, "50%", "layui-col-md6 grid-demo-bg1"));
        g->add_child(grid_cell(host, "50%", "layui-col-md6"));
    }

    // 5. 列间隔
    field_title(host, "列间隔");
    {
        ui::Grid* g = make_row(host, "layui-col-space1");
        const char* cls[] = {"layui-col-md3 grid-demo-bg1", "layui-col-md3",
                             "layui-col-md3 grid-demo-bg1", "layui-col-md3"};
        for (const char* c : cls) g->add_child(grid_cell(host, "1/4", c));
    }
    {
        ui::Grid* g = make_row(host, "layui-col-space5");
        const char* cls[] = {"layui-col-md4 grid-demo-bg1", "layui-col-md4", "layui-col-md4 grid-demo-bg1"};
        for (const char* c : cls) g->add_child(grid_cell(host, "1/3", c));
    }
    {
        ui::Grid* g = make_row(host, "layui-col-space10");
        g->add_child(grid_cell(host, "9/12", "layui-col-md9 grid-demo-bg1"));
        g->add_child(grid_cell(host, "3/12", "layui-col-md3"));
    }
    {
        ui::Grid* g = make_row(host, "layui-col-space15");
        g->add_child(grid_cell(host, "7/12", "layui-col-md7 grid-demo-bg1"));
        g->add_child(grid_cell(host, "5/12", "layui-col-md5"));
    }
    {
        ui::Grid* g = make_row(host, "layui-col-space30");
        g->add_child(grid_cell(host, "7/12", "layui-col-md7 grid-demo-bg1"));
        g->add_child(grid_cell(host, "5/12", "layui-col-md5"));
    }

    // 6. 列偏移
    field_title(host, "列偏移");
    {
        ui::Grid* g = make_row(host);
        g->add_child(grid_cell(host, "4/12", "layui-col-md4 grid-demo-bg1"));
        g->add_child(grid_cell(host, "偏移4列", "layui-col-md4 layui-col-md-offset4"));
        g->add_child(grid_cell(host, "偏移5列", "layui-col-md1 layui-col-md-offset5 grid-demo-bg1"));
        g->add_child(grid_cell(host, "不偏移", "layui-col-md1"));
    }
    {
        ui::Grid* g = make_row(host);
        g->add_child(grid_cell(host, "偏移3列", "layui-col-md3 layui-col-md-offset3 grid-demo-bg1"));
        g->add_child(grid_cell(host, "偏移1列", "layui-col-md3 layui-col-md-offset1"));
    }

    // 7. 栅格嵌套
    field_title(host, "栅格嵌套");
    {
        ui::Grid* g = make_row(host);
        ui::Grid* inner = nested_cell(host, "layui-col-md5 grid-demo grid-demo-bg1");
        inner->add_child(grid_cell(host, "内部列", "layui-col-md3 grid-demo-bg1"));
        inner->add_child(grid_cell(host, "内部列", "layui-col-md9 grid-demo-bg2"));
        inner->add_child(grid_cell(host, "内部列", "layui-col-md12 grid-demo-bg3"));
        g->add_child(inner);

        ui::Grid* inner2 = nested_cell(host, "layui-col-md7 grid-demo grid-demo-bg1");
        inner2->add_child(grid_cell(host, "内部列", "layui-col-md12"));
        inner2->add_child(grid_cell(host, "内部列", "layui-col-md9 grid-demo-bg2"));
        inner2->add_child(grid_cell(host, "内部列", "layui-col-md3 grid-demo-bg3"));
        g->add_child(inner2);
    }

    // 8. 流体容器（宽度自适应，不固定）
    field_title(host, "流体容器（宽度自适应，不固定）");
    {
        ui::Grid* g = make_row(host);
        const char* cls[] = {"layui-col-sm3 grid-demo-bg1", "layui-col-sm3",
                             "layui-col-sm3 grid-demo-bg1", "layui-col-sm3"};
        for (const char* c : cls) g->add_child(grid_cell(host, "25%", c));
    }

    quote(host, "更多我们未能呈现的示例，还有待您在阅读文档、以及不断使用的过程，去深入挖掘。");

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(kGridCode);
    }
    if (host.help) build_help(host);
}

}  // namespace

static void build(PageHost& host) {
    build_grid(host);
}

static const PageDef kGridPage{1, "布局", "栅格", "layui 栅格布局（12 列 span/offset，断点响应式）", build, nullptr, false, nullptr, 0, "Grid"};

static const bool kRegistered = []() {
    page::register_page(kGridPage);
    return true;
}();
