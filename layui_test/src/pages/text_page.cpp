#include "../page_registry.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

const char* kDemoText =
    "//在里面存放任意的文本内容\n"
    "test\n"
    "test\n"
    "test\n";

void add_field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

ui::Code* add_code(PageHost& host, float h = 0.f) {
    ui::Code* code = new ui::Code(0, 0, 800, h);
    code->set_font(host.font);
    code->set_theme(host.theme);
    host.content->add_child(code);
    return code;
}

void build_text(PageHost& host) {
    add_field_title(host, "默认修饰");
    {
        ui::Code* code = add_code(host);
        code->set_code(kDemoText);
    }

    add_field_title(host, "notepad 风格");
    {
        ui::Code* code = add_code(host);
        code->set_title("JavaScript");
        code->set_skin(ui::Code::Skin::kNotepad);
        code->set_code(kDemoText);
    }

    add_field_title(host, "叠加使用");
    {
        ui::Code* outer = add_code(host);
        outer->set_code("//在外面存放任意的文本内容\n"
                        "test\n"
                        "test\n"
                        "test\n");
        ui::Code* inner = new ui::Code(0, 0, 0, 0);
        inner->set_font(host.font);
        inner->set_theme(host.theme);
        inner->set_code(kDemoText);
        outer->add_child(inner);

        ui::Code* notepad_outer = add_code(host);
        notepad_outer->set_title("JavaScript");
        notepad_outer->set_skin(ui::Code::Skin::kNotepad);
        notepad_outer->set_code("//在里面存放任意的文本内容\n"
                                "test\n"
                                "test\n"
                                "test\n");
        ui::Code* n1 = new ui::Code(0, 0, 0, 0);
        n1->set_font(host.font);
        n1->set_theme(host.theme);
        n1->set_skin(ui::Code::Skin::kNotepad);
        n1->set_code("//在里面存放任意的文本内容\n"
                     "test\n"
                     "test\n"
                     "test\n");
        notepad_outer->add_child(n1);
        ui::Code* n2 = new ui::Code(0, 0, 0, 0);
        n2->set_font(host.font);
        n2->set_theme(host.theme);
        n2->set_skin(ui::Code::Skin::kNotepad);
        n2->set_code("//在里面存放任意的文本内容\n"
                     "test\n"
                     "test\n"
                     "test\n");
        n1->add_child(n2);
        ui::Code* n3 = new ui::Code(0, 0, 0, 0);
        n3->set_font(host.font);
        n3->set_theme(host.theme);
        n3->set_skin(ui::Code::Skin::kNotepad);
        n3->set_code("//在里面存放任意的文本内容\n"
                     "test\n"
                     "test\n"
                     "test\n");
        n2->add_child(n3);
        ui::Code* n4 = new ui::Code(0, 0, 0, 0);
        n4->set_font(host.font);
        n4->set_theme(host.theme);
        n4->set_skin(ui::Code::Skin::kNotepad);
        n4->set_code("//在里面存放任意的文本内容\n"
                     "test\n"
                     "test\n"
                     "test\n");
        n3->add_child(n4);
    }

    add_field_title(host, "固定高度");
    {
        ui::Code* code = add_code(host);
        code->set_max_height(150.f);
        code->set_code("//在里面存放任意的文本内容\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n"
                       "test\n");
    }

    ui::BlockQuote* q = new ui::BlockQuote(800, 0,
                                           "更多我们未能呈现的示例，还有待您在阅读文档、以及不断使用的过程，去深入挖掘。");
    q->set_font(host.font);
    q->set_theme(host.theme);
    host.content->add_child(q);

    if (host.code) {
        host.code->set_lang("HTML");
        host.code->set_code(
            "<!-- 克隆 layui demo/code.html：layui-code 代码块组件（文本行修饰） -->\n"
            "\n"
            "<!-- 默认修饰：无标题无皮肤无高度 -->\n"
            "<pre class=\"layui-code\">//在里面存放任意的文本内容\n"
            "test\ntest\ntest</pre>\n"
            "\n"
            "<!-- notepad 风格：lay-title + lay-skin -->\n"
            "<pre class=\"layui-code\" lay-title=\"JavaScript\" lay-skin=\"notepad\">\n"
            "test\ntest\ntest</pre>\n"
            "\n"
            "<!-- 叠加使用：code 块可嵌套 code 块（ui::Code 支持子节点） -->\n"
            "<!-- 固定高度：lay-height=\"150px\" 超高滚动 -->\n"
            "<pre class=\"layui-code\" lay-height=\"150px\">...</pre>\n"
            "\n"
            "// C++ 侧等价实现（pages/text_page.cpp）：\n"
            "// ui::Code set_title/set_lang/set_skin/set_max_height + add_child 嵌套\n"
            "// 小节标题用 FieldsetTitle（layui-elem-field layui-field-title 克隆）");
    }
    if (host.help) {
        const char* lines[] = {
            "layui-code 代码块组件（ui::Code）API",
            "  set_title()：标题条左侧文本（lay-title）",
            "  set_lang()：标题条右侧语言标签（lay-lang）",
            "  set_skin(Skin)：默认 / notepad 两种风格",
            "  set_max_height()：固定最大高度，内容超高内部滚动",
            "  内容自适应高度：未设 max_height 时按行数 + 子节点撑高",
            "  嵌套：Code 可作为子节点叠加使用",
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
    build_text(host);
}

static const PageDef kTextPage{44, "展示", "文本行修饰", "layui-code 代码块组件：默认/notepad/叠加/固定高度", build, nullptr, false, nullptr, 0, "Code"};

static const bool kRegistered = []() {
    page::register_page(kTextPage);
    return true;
}();
