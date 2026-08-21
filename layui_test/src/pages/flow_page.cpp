#include "../page_registry.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/flowbox.h"
#include "ui/ui/image.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace {

// ---- 演示一：滚动加载用的灰色数字块（镜像 .flow-default li：48% 宽、100 高、居中编号） ----
// 该块是 flow 演示特有内容（编号瓦片），属测试页编排展示，非承载 ui 库逻辑。
class FlowItem : public ui::Widget {
public:
    FlowItem(float w, float h, const char* num, const ui::Theme* theme, ui::Font* font)
        : ui::Widget(0, 0, w, h), theme_(theme), font_(font), num_(num) {}

    void on_draw(ui::Renderer& r) override {
        const ui::Vec4 bg{0.93f, 0.93f, 0.93f, 1.0f};  // #eee，镜像 li background
        r.draw_rounded_rect(bounds_, 0.f, bg);
        const float text_w = font_->measure_text(num_.c_str());
        const float cx = bounds_.x + (bounds_.w - text_w) * 0.5f;
        const float cy = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
        font_->draw_text(r, num_.c_str(), cx, cy, theme_->text_secondary);
    }

private:
    const ui::Theme* theme_;
    ui::Font* font_;
    std::string num_;
};

// 通用：追加一个分节标题
void add_section(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

// ---- 演示一：信息流 - 滚动加载（镜像 LAY_demo1：flow.load 自动模式，自身滚动条） ----
void build_demo_scroll_load(PageHost& host) {
    add_section(host, "信息流 - 滚动加载");

    // 独立 600×400 可滚容器（scrollElem 语义：滚它的滚动条触发），嵌套于页面内容区
    ui::ScrollBox* box = new ui::ScrollBox(0, 0, 600, 400);
    box->set_theme(host.theme);
    box->set_spacing(4.f);
    box->set_padding(4.f);
    host.content->add_child(box);

    ui::FlowBox* wrap = new ui::FlowBox(0, 0, 592, 0);
    wrap->set_padding(0.f);
    wrap->set_spacing(4.f);
    box->add_child(wrap);

    auto state = std::make_shared<int>(0);        // 当前页（1..10）
    const int kPages = 10;                        // 总页数
    const int kPerPage = 8;                       // 每页 8 条

    // 只捕获 app 长期对象（host.font/host.theme 指向应用级 &font/&theme），
    // 不得捕获栈局部 PageHost host（build 返回即失效，回调期使用将悬空崩溃——UI-095）。
    ui::Font* app_font = host.font;
    const ui::Theme* app_theme = host.theme;

    // 追加第 page 页的编号瓦片（1..80）
    auto append_page = [wrap, app_font, app_theme, kPerPage](int page) {
        char buf[16];
        for (int i = 0; i < kPerPage; ++i) {
            int n = (page - 1) * kPerPage + i + 1;
            snprintf(buf, sizeof(buf), "%d", n);
            FlowItem* item = new FlowItem(292.f, 100.f, buf, app_theme, app_font);
            wrap->add_child(item);
        }
    };
    // 初始填满首屏并留明显滚动余量（镜像 layui flow.load 首次 done 后会自动续载直到内容超高）
    // —— 否则首屏内容仅比可视区高 ~20px，scroll_max 过小、滚动条乍看全满（UI-094），滚动加载失效。
    append_page(1);
    *state = 1;
    append_page(2);
    *state = 2;

    // 距底部触发距离（镜像 layui flow mb, 默认 50）：快到内容底部即预加载下一页，
    // 不必严格滚到底（UI-094；ScrollBox::set_bottom_margin）。彻底到末页则停止并显示「没有更多了」。
    box->set_bottom_margin(50.f);

    // 滚到底部（或距底部 50px 内）自动加载下一页（还在分页内则再次回到底部继续）
    box->set_on_scroll_to_bottom([box, wrap, state, app_font, app_theme, kPages, append_page]() {
        int& page = *state;
        if (page >= kPages) {
            // 末页：不再加载；贴一个“没有更多了”结束条
            ui::Label* end = new ui::Label(0, 0, 600, 0, "没有更多了");
            end->set_font(app_font);
            end->set_theme(app_theme);
            end->set_color(app_theme->text_muted);
            end->set_align(1);
            wrap->add_child(end);
            box->set_on_scroll_to_bottom(nullptr);  // 停止自动加载（UI-035 离开底部重新武装，这里彻底停）
            return;
        }
        ++page;
        append_page(page);
    });

    ui::Label* tip = new ui::Label(0, 0, 600, 0, "向下滚动本容器，滚动到底自动加载下一页（10 页 × 8 条 = 80 条）");
    tip->set_font(host.font);
    tip->set_theme(host.theme);
    tip->set_color(host.theme->text_secondary);
    host.content->add_child(tip);
}

// ---- 演示二：信息流 - 手工加载（镜像 LAY_demo2：flow.load isAuto:false → 自动生成「加载更多」按钮） ----
void build_demo_manual_load(PageHost& host) {
    add_section(host, "信息流 - 手工加载");

    ui::ScrollBox* box = new ui::ScrollBox(0, 0, 600, 300);
    box->set_theme(host.theme);
    box->set_spacing(4.f);
    box->set_padding(4.f);
    host.content->add_child(box);

    ui::FlowBox* wrap = new ui::FlowBox(0, 0, 592, 0);
    wrap->set_padding(0.f);
    wrap->set_spacing(4.f);
    box->add_child(wrap);

    auto state = std::make_shared<int>(1);   // 当前页（1..6）
    const int kPages = 6;                    // 总页数
    const int kPerPage = 6;                  // 每页 6 张图

    // 可选图片（镜像 lay-src 演示，复用 exe 目录已拷贝资产，纯测试展示）
    const char* kImgs[] = {"a01.jpg", "a02.png", "a03.png", "a04.jpg", "a05.png", "a06.png"};
    const int kImgCount = 6;

    // 追加第 page 页的懒加载图片
    auto append_page = [wrap, kImgs, kImgCount, kPerPage](int page) {
        for (int i = 0; i < kPerPage; ++i) {
            int idx = ((page - 1) * kPerPage + i) % kImgCount;
            ui::Image* img = new ui::Image(0, 0, 292.f, 140.f);
            img->set_lay_src(kImgs[idx]);   // 懒加载：仅进入当前可视屏才解码
            wrap->add_child(img);
        }
    };
    append_page(1);

    // isAuto:false → 在列表底部自动生成「加载更多」按钮（镜像 flow 自动插入按钮）
    ui::Button* more = new ui::Button(0, 0, 160.f, 34.f, "加载更多");
    more->set_font(host.font);
    more->set_theme(host.theme);
    more->add_class("layui-btn layui-btn-sm");
    wrap->add_child(more);

    more->set_on_click([box, wrap, more, state, kPages, append_page]() {
        int& page = *state;
        if (page >= kPages) {
            more->set_text("没有更多了");
            more->set_enabled(false);
            return;
        }
        ++page;
        append_page(page);
        more->bring_to_front();   // 按钮始终保持在列表最底部（镜像：按钮位于列表末端）
    });

    ui::Label* tip = new ui::Label(0, 0, 600, 0, "isAuto:false → layui 在列表底部自动生成「加载更多」按钮；点击加载下一页（6 页 × 6 图），图片 lay-src 懒加载");
    tip->set_font(host.font);
    tip->set_theme(host.theme);
    tip->set_color(host.theme->text_secondary);
    host.content->add_child(tip);
}

// ---- 演示三：图片懒加载（镜像 LAY_demo3：flow.lazyimg，仅加载当前屏图片） ----
void build_demo_lazyimg(PageHost& host) {
    add_section(host, "图片懒加载");

    ui::ScrollBox* box = new ui::ScrollBox(0, 0, 600, 300);
    box->set_theme(host.theme);
    box->set_spacing(4.f);
    box->set_padding(4.f);
    host.content->add_child(box);

    ui::FlowBox* wrap = new ui::FlowBox(0, 0, 592, 0);
    wrap->set_padding(0.f);
    wrap->set_spacing(4.f);
    box->add_child(wrap);

    // 15 张懒加载图片（镜像 site-demo-flow imgs），仅加载当前屏；滚动上/下都按需取图
    const char* kImgs[] = {"a01.jpg", "a02.png", "a03.png", "a04.jpg", "a05.png", "a06.png", "a07.jpg"};
    const int kImgCount = 7;
    for (int i = 0; i < 15; ++i) {
        ui::Image* img = new ui::Image(0, 0, 292.f, 200.f);
        img->set_lay_src(kImgs[i % kImgCount]);
        wrap->add_child(img);
    }

    ui::Label* tip = new ui::Label(0, 0, 600, 0,
                                   "lazyimg：图片带 lay-src，仅对当前屏图片解码（无论滚动条上滑还是下滑）。"
                                   "快速拉到列表中间只会加载中间区域的图。");
    tip->set_font(host.font);
    tip->set_theme(host.theme);
    tip->set_color(host.theme->text_secondary);
    host.content->add_child(tip);
}

void build_flow(PageHost& host) {
    // 镜像 flow.html 预览区的三个演示
    ui::Label* head = new ui::Label(0, 0, 800, 0, "layui.flow：信息流加载 + 图片懒加载");
    head->set_font(host.font);
    head->set_theme(host.theme);
    head->set_color(host.theme->text);
    host.content->add_child(head);

    build_demo_scroll_load(host);
    build_demo_manual_load(host);
    build_demo_lazyimg(host);

    add_section(host, "说明");
    ui::Label* note = new ui::Label(0, 0, 800, 0,
                                    "镜像 layui flow.html：①滚动加载（flow.load 自动，独立容器滚动到底 8 条/页 ×10 页，"
                                    "末页「没有更多了」）；②手工加载（isAuto:false → 列表底部自动生成「加载更多」按钮，"
                                    "6 图/页 ×6 页，lay-src 懒加载）；③图片懒加载（lazyimg，15 图仅加载当前屏）。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);

    if (host.code) {
        host.code->set_lang("JavaScript");
        host.code->set_code(
            "// 1:1 镜像 layui flow.html：流加载（3 个演示）\n"
            "layui.flow.load({\n"
            "  elem: '#LAY_demo1', scrollElem: '#LAY_demo1',  // 滚动加载：独立容器自身滚动条\n"
            "  done: function(page, next){\n"
            "    // 每批向容器追加数据；page<10 表示仍有分页继续自动加载\n"
            "    next(lis.join(''), page < 10);\n"
            "  }\n"
            "});\n"
            "\n"
            "flow.load({ elem:'#LAY_demo2', isAuto:false, isLazyimg:true,  // 手工加载：自动生成加载更多按钮\n"
            "  done:function(page,next){ /* 6 图/页，lay-src 懒加载 */ next(lis.join(''), page < 6); }\n"
            "});\n"
            "\n"
            "flow.lazyimg({ elem:'#LAY_demo3 img', scrollElem:'#LAY_demo3' });  // 图片懒加载：仅加载当前屏\n"
            "\n"
            "// C++ 侧等价实现（pages/flow_page.cpp）：\n"
            "// 滚动加载 —— 独立 ScrollBox，set_on_scroll_to_bottom() 自动加载下一页（UI-035）\n"
            "// 手工加载 —— 列表 FlowBox 末尾自动生成 Button「加载更多」\n"
            "// 图片懒加载 —— ui::Image::set_lay_src()（UI-093）：\n"
            "//   on_draw 用 renderer.current_scissor()（当前屏）检测进入可视区才 load_from_file");
    }
    if (host.help) {
        const char* lines[] = {
            "流加载（layui.flow）API",
            "  flow.load(options)：信息流加载，滚动到底/点击按钮逐页渲染",
            "    elem: 列表容器；scrollElem: 滚动条所在元素（默认 document）",
            "    isAuto: 默认 true 滚动自动加载；false 则列表底部自动生成「加载更多」按钮",
            "    isLazyimg: true 对列表内 lay-src 图片按当前屏懒加载",
            "    end: 末页显示文本（默认「没有更多了」）；mb: 距底部临界距离(默认50)",
            "    done(page, next)：next(html, 是否还有更多页)",
            "  flow.lazyimg(options)：对指定 elem 内带 lay-src 的 img 开启懒加载",
            "  C++ ui 库：ScrollBox::set_on_scroll_to_bottom()（UI-035）滚到底自动加载",
            "             Image::set_lay_src() 图片懒加载（进入当前屏才解码）",
            "  Renderer::current_scissor() 提供「当前屏」矩形（剪刀栈顶），供懒加载判可视",
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

void destroy_flow(PageHost& host) {
    // 切页清理：页面根 ScrollBox 的滚动到底回调会随整页子树释放，
    // 嵌套子 ScrollBox 同样随子树析构，这里无需额外置空（UI-039 整树释放）。
    (void)host;
}

}  // namespace

static void build(PageHost& host) {
    build_flow(host);
}

static const PageDef kFlowPage{43, "展示", "流加载", "流加载：分批/自动加载数据", build, destroy_flow, false, nullptr, 0, "Flow"};

static const bool kRegistered = []() {
    page::register_page(kFlowPage);
    return true;
}();