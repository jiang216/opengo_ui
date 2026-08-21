#include "../page_registry.h"

#include "ui/ui/button.h"
#include "ui/ui/carousel.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/switch.h"
#include "ui/ui/theme.h"

#include <cstdio>
#include <string>

namespace {

void add_fieldset_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* ft = new ui::FieldsetTitle(800, 40.f, text);
    ft->set_font(host.font);
    ft->set_theme(host.theme);
    host.content->add_child(ft);
}

ui::Carousel* make_carousel(PageHost& host, float w, float h) {
    ui::Carousel* c = new ui::Carousel(0, 0, w, h);
    c->set_font(host.font);
    c->set_theme(host.theme);
    host.content->add_child(c);
    return c;
}

// ── 1. 常规轮播（test1：5 页 + arrow always）─────────────────────────────────
void build_demo1(PageHost& host) {
    add_fieldset_title(host, "常规轮播");
    ui::Carousel* c = make_carousel(host, 600, 260.f);
    c->add_slide({0.09f, 0.73f, 0.67f, 1.0f}, "条目1");
    c->add_slide({0.09f, 0.72f, 0.47f, 1.0f}, "条目2");
    c->add_slide({0.12f, 0.62f, 0.95f, 1.0f}, "条目3");
    c->add_slide({0.62f, 0.33f, 0.89f, 1.0f}, "条目4");
    c->add_slide({0.35f, 0.35f, 0.35f, 1.0f}, "条目5");
    c->set_arrow(ui::CarouselArrow::Always);

    ui::Carousel* c2 = make_carousel(host, 600, 120.f);
    c2->add_slide({0.09f, 0.73f, 0.67f, 1.0f}, "条目1");
    c2->add_slide({0.09f, 0.72f, 0.47f, 1.0f}, "条目2");
    c2->set_interval(1800.f);
    c2->set_anim(ui::CarouselAnim::Fade);
    c2->set_autoplay(true);
    c2->set_arrow(ui::CarouselArrow::Always);
}

// ScrollBox::on_layout 会重排每个直接子节点，因此每行控件需包进一个 HBox 横向排布。
ui::HBox* make_row(PageHost& host) {
    ui::HBox* row = new ui::HBox(0, 0, 800, 40.f);
    row->set_padding(0.f);
    row->set_spacing(10.f);
    row->set_theme(host.theme);
    host.content->add_child(row);
    return row;
}

void add_row_label(ui::HBox* row, PageHost& host, const char* text, float w) {
    ui::Label* l = new ui::Label(0, 0, w, 0, text);
    l->set_font(host.font);
    l->set_theme(host.theme);
    l->set_color(host.theme->text);
    row->add_child(l);
}

ui::Button* add_row_btn(ui::HBox* row, PageHost& host, const char* text, std::function<void()> cb) {
    ui::Button* b = new ui::Button(0, 0, 100.f, 32.f, text);
    b->set_font(host.font);
    b->set_theme(host.theme);
    b->set_on_click(std::move(cb));
    row->add_child(b);
    return b;
}

// ── 2. 设定各种参数（test3：交互调节 anim/arrow/indicator/autoplay/interval）──
void build_demo2(PageHost& host) {
    add_fieldset_title(host, "设定各种参数");
    ui::Carousel* c = make_carousel(host, 600, 260.f);
    c->add_slide({0.09f, 0.73f, 0.67f, 1.0f}, "条目1");
    c->add_slide({0.09f, 0.72f, 0.47f, 1.0f}, "条目2");
    c->add_slide({0.12f, 0.62f, 0.95f, 1.0f}, "条目3");
    c->add_slide({0.62f, 0.33f, 0.89f, 1.0f}, "条目4");
    c->add_slide({0.35f, 0.35f, 0.35f, 1.0f}, "条目5");
    c->set_arrow(ui::CarouselArrow::Always);
    c->set_autoplay(true);

    // 动画类型
    ui::HBox* rowA = make_row(host);
    add_row_label(rowA, host, "动画类型", 90.f);
    add_row_btn(rowA, host, "左右切换", [c] { c->set_anim(ui::CarouselAnim::Default); });
    add_row_btn(rowA, host, "上下切换", [c] { c->set_anim(ui::CarouselAnim::Updown); });
    add_row_btn(rowA, host, "渐隐渐显", [c] { c->set_anim(ui::CarouselAnim::Fade); });

    // 箭头状态
    ui::HBox* rowB = make_row(host);
    add_row_label(rowB, host, "箭头状态", 90.f);
    add_row_btn(rowB, host, "悬停显示", [c] { c->set_arrow(ui::CarouselArrow::Hover); });
    add_row_btn(rowB, host, "始终显示", [c] { c->set_arrow(ui::CarouselArrow::Always); });
    add_row_btn(rowB, host, "不显示", [c] { c->set_arrow(ui::CarouselArrow::None); });

    // 指示器位置
    ui::HBox* rowC = make_row(host);
    add_row_label(rowC, host, "指示器位置", 90.f);
    add_row_btn(rowC, host, "容器内部", [c] { c->set_indicator(ui::CarouselIndicator::Inside); });
    add_row_btn(rowC, host, "容器外部", [c] { c->set_indicator(ui::CarouselIndicator::Outside); });
    add_row_btn(rowC, host, "不显示", [c] { c->set_indicator(ui::CarouselIndicator::None); });

    // 自动切换 + 时间间隔
    ui::HBox* rowD = make_row(host);
    add_row_label(rowD, host, "自动切换", 90.f);
    ui::Switch* sw = new ui::Switch(0, 8.f, 50.f, 24.f);
    sw->set_font(host.font);
    sw->set_theme(host.theme);
    sw->set_checked(true);
    sw->set_on_change([c](bool on) { c->set_autoplay(on); });
    rowD->add_child(sw);
    add_row_label(rowD, host, "时间间隔(±1000ms)", 150.f);
    add_row_btn(rowD, host, "-", [c] { c->set_interval(std::max(500.f, c->interval() - 1000.f)); });
    add_row_btn(rowD, host, "+", [c] { c->set_interval(c->interval() + 1000.f); });
}

// ── 3. 填充轮播元素 - 以图片为例（test10：7 张图片 778x440）──────────────────
// 图片取自仓库 file/img 目录（a01.jpg / a02.png / ... 在 build 时拷贝到 exe 目录，
// 由 Texture::load_from_file 直接加载；路径即裸文件名，运行时 CWD = exe 目录）。
void build_demo3(PageHost& host) {
    add_fieldset_title(host, "填充轮播元素 - 以图片为例");
    ui::Carousel* c = make_carousel(host, 778.f, 440.f);
    c->add_image_slide("a01.jpg");
    c->add_image_slide("a02.png");
    c->add_image_slide("a03.png");
    c->add_image_slide("a04.jpg");
    c->add_image_slide("a05.png");
    c->add_image_slide("a06.png");
    c->add_image_slide("a07.jpg");
    c->set_interval(5000.f);
    c->set_autoplay(true);
    c->set_arrow(ui::CarouselArrow::Always);
}

void build_carousel(PageHost& host) {
    build_demo1(host);
    build_demo2(host);
    build_demo3(host);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/carousel_page.cpp — 一比一克隆 layui carousel.html 三个 demo 组\n"
            "\n"
            "ui::Carousel* c = new ui::Carousel(0, 0, 600, 260);\n"
            "c->set_font(host.font); c->set_theme(host.theme);\n"
            "\n"
            "// 添加页：纯色底 + 标题，或图片\n"
            "c->add_slide({0.09f,0.73f,0.67f,1.f}, \"条目1\");\n"
            "c->add_image_slide(\"a01.jpg\");   // file/img 图片 build 时拷贝到 exe 目录\n"
            "\n"
            "// 配置项（对应 layui render 选项）\n"
            "c->set_anim(ui::CarouselAnim::Default);   // default/updown/fade\n"
            "c->set_arrow(ui::CarouselArrow::Always);  // hover/always/none\n"
            "c->set_indicator(ui::CarouselIndicator::Inside); // inside/outside/none\n"
            "c->set_autoplay(true);\n"
            "c->set_interval(3000.f);   // 毫秒\n"
            "c->set_size(600, 260);\n"
            "\n"
            "c->next(); c->prev();        // 手动切页\n"
            "c->set_index(0);             // 跳到指定页\n"
            "// 自动轮播/动画由主循环 root->advance(dt) 驱动\n"
            "host.content->add_child(c);");
    }
    if (host.help) {
        const char* lines[] = {
            "轮播（Carousel）API — 一比一克隆 carousel.html 三个 demo 组",
            "",
            "1. 常规轮播：arrow always + 默认动画（5 页）；fade 动画 + interval 1800（2 页）",
            "2. 设定各种参数：set_anim(Default/Updown/Fade)、set_arrow(Hover/Always/None)、",
            "   set_indicator(Inside/Outside/None)、set_autoplay + set_interval(毫秒) 交互调节",
            "3. 填充图片：add_image_slide(path)，778x440 7 张轮播（图片取自 file/img），",
            "通用：add_slide / add_image_slide / next() / prev() / set_index() / current()",
            "自动轮播/动画计数：主循环每帧 root->advance(dt) 驱动（on_advance）",
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
    build_carousel(host);
}

static const PageDef kCarouselPage{45, "展示", "轮播", "轮播：常规/参数/图片", build, nullptr, false, nullptr, 0, "Carousel"};

static const bool kRegistered = []() {
    page::register_page(kCarouselPage);
    return true;
}();