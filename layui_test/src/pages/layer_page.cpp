#include "../page_registry.h"

#include "ui/core/math.h"
#include "ui/gl/font.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/panelwindow.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

#include <functional>
#include <vector>

namespace {

// 当前打开的 layer 窗口（根级弹出，非预览子控件）：关闭/切页时统一清理，避免悬空/泄漏（UI-033/UI-040）
std::vector<ui::PanelWindow*> g_windows;
ui::Widget* g_root = nullptr;

void cleanup_windows() {
    // Widget::destroy()：摘除父引用 + 递归释放子树 + delete 自身一条龙（UI-040）
    for (ui::PanelWindow* w : g_windows) w->destroy();
    g_windows.clear();
}

void destroy_layer(PageHost& host) {
    cleanup_windows();
    if (host.content) host.content->root()->close_popups();
}

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

using BtnCb = std::function<void(ui::Button*)>;
void add_buttons(PageHost& host, const std::vector<std::pair<const char*, BtnCb>>& items) {
    ui::HBox* row = new ui::HBox(0, 0, 800, 34.f);
    row->set_padding(0.f);
    row->set_spacing(8.f);
    row->set_theme(host.theme);
    for (const auto& item : items) {
        // 按钮宽 = 文本宽 + 左右内边距（layui-btn 左右 26px），否则 HBox 取 0 宽致文本重叠
        const float text_w = host.font ? host.font->measure_text(item.first) : 60.f;
        ui::Button* btn = new ui::Button(0, 0, text_w + 52.f, 30.f, item.first);
        btn->add_class("layui-btn layui-btn-sm");
        btn->set_font(host.font);
        btn->set_theme(host.theme);
        const BtnCb cb = item.second;
        btn->set_on_click([btn, cb]() { cb(btn); });
        row->add_child(btn);
    }
    host.content->add_child(row);
}

// 统一弹窗工厂：构造 + 外观 + 定位 + 打开（多窗口演示不清理已有窗口）
// opts 语义（复刻 layer.html 各 demo 参数）：
//   title     标题（空字符串且 title_visible=false 表示无标题栏，如公告层）
//   show_title / show_close / maxmin / shade / auto_close
//   footer = 底部按钮文本数组，footer_align=0左 1中 2右，on_footer(下标) 回调
//   offset   定位（t/r/b/l/lt/lb/rt/rb/auto）
// 注意：content/font/theme 必须是 app 长期对象（preview 等），不得传栈局部 host——
//       弹窗按钮回调存活到用户点击，若捕获栈局部 host（build 后销毁）将悬空崩溃（UI-071）
ui::PanelWindow* open_layer(ui::ScrollBox* content, ui::Font* font, const ui::Theme* theme,
                            ui::Widget* trigger, const char* title, float w, float h,
                            const char* offset, bool show_title, bool show_close, bool maxmin,
                            float shade, float auto_close,
                            const std::vector<std::string>& footer, int footer_align,
                            std::function<void(int)> on_footer, const char* content_text) {
    ui::Widget* root = content->root();
    g_root = root;

    ui::PanelWindow* win = new ui::PanelWindow(0, 0, w, h, title ? title : "");
    win->set_font(font);
    win->set_theme(theme);
    win->set_title_visible(show_title);
    win->set_close_visible(show_close);
    win->set_maxmin(maxmin);
    win->set_shade(shade);
    win->set_auto_close(auto_close);
    if (!footer.empty()) win->set_footer_buttons(footer, footer_align, on_footer);

    const ui::Rect rb = root->bounds();
    const float cx = rb.x + rb.w * 0.5f;
    const float cy = rb.y + rb.h * 0.5f;
    float x = cx - w * 0.5f, y = cy - h * 0.5f;
    const std::string off = offset ? offset : "auto";
    if (off == "t") { x = cx - w * 0.5f; y = rb.y + 20.f; }
    else if (off == "r") { x = rb.x + rb.w - w - 20.f; y = cy - h * 0.5f; }
    else if (off == "b") { x = cx - w * 0.5f; y = rb.y + rb.h - h - 20.f; }
    else if (off == "l") { x = rb.x + 20.f; y = cy - h * 0.5f; }
    else if (off == "lt") { x = rb.x + 20.f; y = rb.y + 20.f; }
    else if (off == "lb") { x = rb.x + 20.f; y = rb.y + rb.h - h - 20.f; }
    else if (off == "rt") { x = rb.x + rb.w - w - 20.f; y = rb.y + 20.f; }
    else if (off == "rb") { x = rb.x + rb.w - w - 20.f; y = rb.y + rb.h - h - 20.f; }
    win->set_bounds(x, y, w, h);

    if (content_text && content_text[0] != '\0') {
        ui::Label* body = new ui::Label(12, 16, w - 40,
                                        h - (show_title ? 50.f : 0.f) - 16.f -
                                            (footer.empty() ? 0.f : 50.f),
                                        content_text);
        body->set_wrap(true);
        body->set_font(font);
        body->set_theme(theme);
        body->set_color(theme->text);
        win->add_child(body);
    }

    win->set_on_close([root]() { root->close_popups(); });
    if (shade > 0.f) root->set_modal(win);
    root->add_child(win);
    root->open_popup(trigger, win);
    g_windows.push_back(win);
    win->layout();
    return win;
}

void build_layer(PageHost& host) {
    // 捕获 app 长期对象（preview/status 等 new 控件），不捕获栈局部 host（build 后销毁）：
    // 按钮回调存活到用户点击，捕获悬空 host 会 dereference 崩溃（UI-071）
    ui::ScrollBox* content = host.content;
    ui::Font* font = host.font;
    const ui::Theme* theme = host.theme;

    add_section(host, "弹出层（layer）——点击按钮弹出；可拖拽标题移动 / 右下角缩放 / x 关闭");
    ui::Label* status = new ui::Label(0, 0, 800, 0, "未弹出");
    status->set_font(font);
    status->set_theme(theme);
    status->set_color(theme->text_secondary);
    host.content->add_child(status);

    // 1. 多窗口模式 + 层叠置顶 + Esc 关闭（复刻 setTop）
    add_buttons(host, {
        {"多窗口模式 + 层叠置顶 + Esc 关闭", [content, font, theme, status](ui::Button* b) {
            cleanup_windows();
            open_layer(content, font, theme, b, "窗口 A（左上）", 390.f, 260.f, "lt", true, true,
                       true, 0.f, 0.f, {"继续弹出", "全部关闭"}, 1,
                       [content, font, theme, b](int i) {
                           if (i == 0) {
                               // 继续弹出：再开一个随机位置窗口（层叠置顶）
                               open_layer(content, font, theme, b, "窗口 C（随机）", 320.f, 200.f,
                                          "auto", true, true, true, 0.f, 0.f,
                                          {"继续弹出", "全部关闭"}, 1,
                                          [content, font, theme, b](int j) {
                                              if (j == 1) cleanup_windows();
                                          },
                                          "点击窗口即置于最顶端（layer.setTop），按 Esc 关闭最上层。");
                           } else if (i == 1) {
                               cleanup_windows();
                           }
                       },
                       "多窗口模式：点击某个窗口即置于最顶端（layer.setTop），按 Esc 键关闭最上层。");
            status->set_text("多窗口：弹出窗口，可独立拖拽/置顶/最大化，Esc 关闭最上层");
        }},
    });

    // 2. 配置一个透明的询问框（复刻 confirmTrans：layer.msg 风格，20s 自动关闭 + 3 按钮）
    add_buttons(host, {
        {"透明的询问框（20s 自动关闭）", [content, font, theme, status](ui::Button* b) {
            // layer.msg：无标题栏/无关闭钮，居中紧凑框，底部按钮条 + 自动关闭
            open_layer(content, font, theme, b, "", 360.f, 160.f, "auto", false, false, false, 0.f,
                       20.f, {"明白了", "知道了", "哦"}, 1, nullptr,
                       "大部分参数都是可以公用的\n合理搭配，展示不一样的风格\n\n（20s 后自动关闭）");
            status->set_text("透明询问框（msg 风格：无标题栏/关闭钮，20s 自动关闭，3 按钮）");
        }},
    });

    // 3. 示范一个公告层（复刻 notice：无标题栏/关闭钮 + 深色内容 + 遮罩 0.8 + 居中两按钮）
    add_buttons(host, {
        {"示范一个公告层", [content, font, theme, status](ui::Button* b) {
            ui::PanelWindow* win = open_layer(
                content, font, theme, b, "", 340.f, 220.f, "auto", false, false, false, 0.8f, 0.f,
                {"火速围观", "残忍拒绝"}, 1, [](int i) { (void)i; cleanup_windows(); }, "");
            win->add_class("layui-layer-notice");
            // 深色内容（#393D49）配白字，加内边距（镜像 50px）
            ui::Label* body = new ui::Label(20, 20, 340 - 40, 220 - 40 - 50.f,
                                            "你知道吗？亲！layer ≠ layui\n\n"
                                            "layer 只是作为 layui 的一个弹层模块，由于其用户基数较大，"
                                            "所以常常会有人以为 layui 是 layerui。\n\n"
                                            "layer 虽然已被 layui 收编为内置弹层模块，但仍会作为一个独立组件全力维护。");
            body->set_wrap(true);
            body->set_font(font);
            body->set_theme(theme);
            body->set_color(ui::Vec4{1.f, 1.f, 1.f, 1.f});
            win->add_child(body);
            win->layout();
            status->set_text("公告层：无标题栏/关闭钮，深色内容 + 遮罩 0.8，按钮居中");
        }},
    });

    // 4. 边缘/居中弹出（复刻 offset 九方向）
    add_section(host, "边缘弹出（offset）");
    const struct { const char* name; const char* off; } offsets[] = {
        {"上弹出", "t"}, {"右弹出", "r"}, {"下弹出", "b"}, {"左弹出", "l"},
        {"左上弹出", "lt"}, {"左下弹出", "lb"}, {"右上弹出", "rt"}, {"右下弹出", "rb"},
        {"居中弹出", "auto"},
    };
    std::vector<std::pair<const char*, BtnCb>> items;
    for (const auto& o : offsets) {
        items.emplace_back(o.name, [content, font, theme, status, o](ui::Button* b) {
            char buf[48];
            snprintf(buf, sizeof(buf), "弹出：%s", o.name);
            status->set_text(buf);
            open_layer(content, font, theme, b, o.name, 300.f, 150.f, o.off, true, true, false,
                       0.f, 0.f, {"关闭全部"}, 1, [](int i) { (void)i; cleanup_windows(); },
                       "以边缘为基准的 layer 弹出示例。");
        });
    }
    add_buttons(host, items);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/layer_page.cpp（节选）：克隆 layui layer.html\n"
            "\n"
            "// 弹出窗口 = root 级 popup + PanelWindow(layui 外观)\n"
            "ui::PanelWindow* win = new ui::PanelWindow(0, 0, 380.f, 240.f, \"窗口\");\n"
            "win->add_class(\"layui-layer\");\n"
            "win->set_font(host.font);\n"
            "win->set_theme(host.theme);\n"
            "win->set_maxmin(true);                 // 最大化/还原按钮\n"
            "win->set_shade(0.8f);                  // 遮罩\n"
            "win->set_auto_close(20.f);             // 20s 自动关闭\n"
            "win->set_footer_buttons({\"明白了\"}, 1, cb);  // 底部按钮条\n"
            "root->add_child(win);\n"
            "root->open_popup(trigger_button, win);   // 点击外部自动关闭\n"
            "// 支持：拖拽标题栏移动、右下角缩放、x 关闭、offset 定位、置顶、Esc 关闭最上层");
    }
    if (host.help) {
        const char* lines[] = {
            "弹出层（layer）API",
            "  root->open_popup(owner, popup)：注册根级弹出（点击外部自动关闭）",
            "  root->close_popups()：关闭全部弹出并分离根子指针",
            "  root->close_topmost_popup()：关闭最上层弹出（Esc，UI-069）",
            "  root->set_modal(win)：模态（遮罩时事件只路由给窗口）",
            "  PanelWindow：拖拽/缩放/关闭；set_title_visible/set_close_visible/set_maxmin",
            "  set_shade(alpha)：底部遮罩；set_auto_close(秒)：自动关闭（需 advance）",
            "  set_footer_buttons(labels, align, cb)：底部按钮条",
            "  offset：上/下/左/右/四角/居中定位；窗口生命周期由 destroy 钩子托管（切页安全）",
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
    build_layer(host);
}

static const PageDef kLayerPage{50, "交互", "弹出层", "layer 弹出：多窗口/公告/边缘定位", build, destroy_layer, false, nullptr, 0, "Layer"};

static const bool kRegistered = []() {
    page::register_page(kLayerPage);
    return true;
}();
