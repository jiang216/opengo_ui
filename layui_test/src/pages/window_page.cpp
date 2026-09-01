#include "../page_registry.h"

#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/panel.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/window_frame.h"

namespace {

void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

ui::Label* make_label(PageHost& host, const char* text, float x, float y, float w) {
    ui::Label* l = new ui::Label(x, y, w, 0, text);
    l->set_font(host.font);
    l->set_color(host.theme->text);
    return l;
}

// 1) 默认头部：WindowFrame 自带系统风格 TitleBar + 内容区相对坐标
void build_default_head(PageHost& host) {
    field_title(host, "默认头部（自绘标题栏 + 内容区相对坐标）");

    ui::WindowFrame* frame = new ui::WindowFrame(760, 240);
    frame->set_title("窗口容器 WindowFrame");
    frame->set_theme(host.theme);
    frame->set_close_visible(false);

    // 内容区浅底 body（相对坐标：左上角即标题栏下方原点）
    ui::Panel* body = new ui::Panel(16, 16, 560, 120);
    body->set_fill(ui::Vec4{0.96f, 0.97f, 0.98f, 1.f});
    frame->content()->add_child(body);
    frame->set_content_child(body, 16, 16, 560, 120);

    ui::Label* title = make_label(host, "这就是一个带自绘标题栏的窗口容器",
                                  32, 36, 400);
    title->set_color(ui::Vec4{0.16f, 0.45f, 0.85f, 1.f});
    frame->content()->add_child(title);
    frame->set_content_child(title, 32, 36, 400, 20);

    ui::Label* sub = make_label(host,
        "内容子控件以「容器相对坐标」定位：set_content_child(y) 的 y 是相对标题栏下方，\n"
        "on_layout 会自动把它重基准到绝对坐标，不会随拖动/缩放二次叠加。",
        32, 62, 520);
    frame->content()->add_child(sub);
    frame->set_content_child(sub, 32, 62, 520, 36);

    ui::Button* btn = new ui::Button(0, 0, 110, 36, "我是按钮");
    btn->set_font(host.font);
    btn->add_class("layui-btn layui-btn-primary");
    frame->content()->add_child(btn);
    frame->set_content_child(btn, 600, 52, 110, 36);

    host.content->add_child(frame);
}

// 2) 通用头部：set_head 换成任意 Widget 作 head（TitleBar 之上更自由的头部）
void build_custom_head(PageHost& host) {
    field_title(host, "通用头部（set_head 任意外部 Widget）");

    ui::WindowFrame* frame = new ui::WindowFrame(760, 220);

    // 自定义 head：任意 Widget 作头部（这里用一个空 Widget 承载头部相对坐标子项）
    ui::Widget* head = new ui::Widget();
    frame->set_head(head, 42.f);

    // 头部子项用「头部相对坐标」定位（相对头部左上角，即框架原点）
    ui::Panel* bar = new ui::Panel(0, 0, 760, 42);
    bar->set_fill(ui::Vec4{0.35f, 0.40f, 0.50f, 1.f});
    head->add_child(bar);
    frame->set_head_child(bar, 0, 0, 760, 42);

    ui::Label* headTxt = make_label(host, "自定义头部（通用 head，高度 42）", 16, 11, 400);
    headTxt->set_color(ui::Vec4{1.f, 1.f, 1.f, 1.f});
    head->add_child(headTxt);
    frame->set_head_child(headTxt, 16, 11, 400, 20);

    ui::Label* line = make_label(host, "只要是一个 Widget，就能通过 set_head 当头部；内容区一并下移。",
                                 20, 20, 640);
    frame->content()->add_child(line);
    frame->set_content_child(line, 20, 20, 640, 20);

    ui::Panel* content_pad = new ui::Panel(20, 56, 700, 100);
    content_pad->set_fill(ui::Vec4{0.96f, 0.97f, 0.98f, 1.f});
    frame->content()->add_child(content_pad);
    frame->set_content_child(content_pad, 20, 56, 700, 100);

    host.content->add_child(frame);
}

void build_page(PageHost& host) {
    build_default_head(host);
    build_custom_head(host);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/window_page.cpp：WindowFrame 窗口容器演示（UI-154）\n"
            "\n"
            "// 1) 默认头部：自带系统风格 TitleBar，内容区用相对坐标\n"
            "ui::WindowFrame* f = new ui::WindowFrame(760, 240);\n"
            "f->set_title(\"窗口容器 WindowFrame\");\n"
            "f->set_theme(theme);\n"
            "\n"
            "ui::Widget* body = f->content();           // 内容区（标题栏下方）\n"
            "body->add_child(btn);\n"
            "f->set_content_child(btn, 600, 52, 110, 36); // (x,y) 相对内容区顶部\n"
            "\n"
            "// 2) 通用头部：任意 Widget 作 head（默认 TitleBar 之外更自由）\n"
            "f->set_head(custom_head_widget, 42.f);");
    }
    if (host.help) {
        const char* lines[] = {
            "窗口容器（WindowFrame，UI-154）",
            "  WindowFrame(w,h)：自带顶部自绘标题栏的整窗容器",
            "  默认头部 = 系统风格 TitleBar（高度可 set_titlebar_height，默认 30）",
            "  内容区 f->content()：子控件 add_child 后由 f->set_content_child(child,x,y,w,h) 定位",
            "  set_content_child 的 (x,y) 是相对内容区顶部（标题栏下方）的容器相对坐标",
            "  通用头部：f->set_head(widget, height) 可换任意外部 Widget 作 head",
        };
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            label->set_font(host.font);
            label->set_color(host.theme->text);
            host.help->add_child(label);
        }
    }
}

}  // namespace

static void build(PageHost& host) {
    build_page(host);
}

static const PageDef kWindowPage{2, "布局", "窗口容器", "窗口容器：自绘标题栏 + 内容区相对坐标", build, nullptr, false, nullptr, 0, "WindowFrame / TitleBar / content_origin_y"};

static const bool kRegistered = []() {
    page::register_page(kWindowPage);
    return true;
}();
