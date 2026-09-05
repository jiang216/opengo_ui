#include "../page_registry.h"

#include "ui/core/log.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/dropdown.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"
#include "ui/ui/titlebar.h"
#include "ui/ui/scene.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/window.h"

#include <algorithm>
#include <cstdio>
#include <vector>

// ---------------------------------------------------------------------------
// 多级窗口创建 demo（UI-189）——「多窗口」创建方式（UI-146 scene_page 子窗范式）
//
// 每层 = 一个独立 GLFW 窗口 + 独立 GL 上下文 / Renderer / Scene / Font：
//   - 第 1 层由页面按钮打开，主显示器居中（center_on_primary_monitor）；
//   - 更深的层由父窗内「创建窗口」按钮创建，从父窗位置向右下错位叠放（cascade）；
//   - 所有打开中的窗口由主壳 Scene 的 Frame 观察者统一驱动：
//     make_current -> frame(dt) -> swap_buffers，完成后切回主壳上下文。
// 每窗内容：自绘 TitleBar（拖动/最小化/最大化/X，UI-148/UI-172）+ 层级标识 +
// 列表（3 行，每行 = 下拉菜单 Dropdown + 按钮 Button）+ 状态 Label +
// 「创建窗口」（最深一层置灰；每窗至多一个子窗，点击后禁用自身）+「关闭本窗口」（Esc 亦可）。
//
// 生命周期：每窗独立 Scene，X / Esc / 关闭按钮 -> request_exit -> 驱动观察者下一帧关闭；
// 切页 destroy 钩子统一 unsubscribe + 关闭全部窗口（防悬空与泄漏，UI-033/071/146）。
// ---------------------------------------------------------------------------

namespace {

constexpr int kMaxLevel = 4;
constexpr float kW = 440.f, kH = 320.f;
constexpr float kCascadeX = 52.f, kCascadeY = 36.f;  // 逐层错位（向右下角叠放）

// 一级窗口的运行环境（全部为独立堆对象；GL 资源绑定本窗上下文）
struct LevelWin {
    ui::Window* win = nullptr;
    ui::Renderer* renderer = nullptr;
    ui::Scene* scene = nullptr;
    ui::Font* font = nullptr;
    int level = 0;
};

std::vector<LevelWin> g_wins;
size_t g_token = 0;             // 驱动观察者 token（挂在主壳 Scene Frame，首次开窗时订阅）
ui::Scene* g_main_scene = nullptr;  // 主壳 Scene（切回上下文用；destroy 钩子置 null）
ui::Label* g_status = nullptr;      // 本页状态 Label（切页 destroy 钩子置 null，防悬空）

// 关闭一个窗口：先置其上下文当前，再 shutdown 并释放其渲染/字体/Scene/窗口
void close_level(LevelWin lv) {
    if (!lv.win) return;
    ui::log_info("multilevel: window closed (level %d)", lv.level);
    lv.win->make_current();
    if (lv.scene) {
        lv.scene->shutdown();
        delete lv.scene;
    }
    delete lv.renderer;
    delete lv.font;
    delete lv.win;
}

// 全部关闭：从跟踪列表摘除 + 切回主壳上下文
void close_all_windows() {
    for (LevelWin& lv : g_wins) close_level(lv);
    g_wins.clear();
    if (g_main_scene) g_main_scene->window().make_current();
}

// 关闭单个窗口并摘除跟踪项（lv 值拷贝：先取原始句柄再删，摘除按句柄比对）
void close_window(LevelWin lv) {
    ui::Window* raw = lv.win;
    close_level(lv);
    auto& v = g_wins;
    v.erase(std::remove_if(v.begin(), v.end(),
                           [raw](const LevelWin& o) { return o.win == raw; }),
            v.end());
}

// 第 level 层窗口工厂（递归）：source 为触发方 Scene（第 1 层 = 主壳；更深 = 父窗 Scene）。
// theme 为 app 长期对象；回调只捕获本窗长命对象与值参数（UI-071/072）。
void open_level(ui::Scene& source, const ui::Theme* theme, int level) {
    if (level > kMaxLevel) return;
    if (!source.window().handle()) return;

    char title[64];
    std::snprintf(title, sizeof(title), "多级窗口 - 第 %d 层", level);

    LevelWin lv;
    lv.level = level;
    lv.win = new ui::Window((int)kW, (int)kH, title);
    if (!lv.win->handle()) {
        delete lv.win;
        return;
    }
    // 位置：第 1 层主显示器居中；更深层从父窗（触发方）位置错位叠放
    if (level == 1) {
        lv.win->center_on_primary_monitor();
    } else {
        int px = 0, py = 0;
        source.window().get_position(px, py);
        lv.win->set_position(px + (int)kCascadeX, py + (int)kCascadeY);
    }

    lv.win->make_current();  // 第二独立 GL 上下文置当前
    lv.renderer = new ui::Renderer();
    lv.renderer->init();
    lv.renderer->set_viewport(lv.win->width(), lv.win->height());

    // 每窗独立字体（GL 纹理绑定本窗上下文）；标题栏图标字体由 TitleBar 自动从 exe 目录懒装载（UI-172）
    lv.font = new ui::Font();
    lv.font->load_from_file("C:/Windows/Fonts/arial.ttf", 16.f);
    lv.font->load_cjk_font_auto();

    lv.scene = new ui::Scene(*lv.win, *lv.renderer);
    lv.scene->set_root_background(ui::Vec4{0.10f, 0.12f, 0.15f, 1.f});
    ui::Widget& r = lv.scene->root();
    ui::Scene* scn = lv.scene;  // 本窗长命对象：回调捕获，随窗销毁

    // 自绘标题条：拖动 / 最小化 / 最大化 / X 关闭（图标字体自动懒装载）；X -> request_exit
    ui::TitleBar* titlebar = new ui::TitleBar(kW, 30.f, title, lv.win);
    titlebar->set_font(lv.font);
    titlebar->set_theme(theme);
    titlebar->set_on_close([scn]() {
        if (scn) scn->request_exit();
    });
    r.add_child(titlebar);

    // 窗内 Escape 关闭本窗
    lv.scene->set_event_hook(ui::SceneEvent::Key, [scn](ui::Scene& s) {
        if (s.event_key() == ui::KeyEscape) {
            s.request_exit();
            return true;
        }
        return false;
    });

    // 顶部：层级标识
    char level_txt[64];
    std::snprintf(level_txt, sizeof(level_txt), "第 %d 层 / 共 %d 层", level, kMaxLevel);
    ui::Label* level_lbl = new ui::Label(16, 44, kW - 32.f, 20, level_txt);
    level_lbl->set_font(lv.font);
    level_lbl->set_theme(theme);
    level_lbl->set_color(theme->text);
    r.add_child(level_lbl);

    // 窗内状态 Label：列表操作 / 下拉选择回写
    ui::Label* st = new ui::Label(16, 202, kW - 32.f, 18, "就绪：操作下列列表或点「创建窗口」深入到下一层");
    st->set_font(lv.font);
    st->set_theme(theme);
    st->set_color(theme->text_secondary);
    r.add_child(st);

    // 列表：VBox 3 行，每行 = HBox{ 下拉菜单 Dropdown + 按钮 Button }
    ui::VBox* list = new ui::VBox(16, 74, kW - 32.f, 118.f);
    list->set_theme(theme);
    list->set_padding(0.f);
    list->set_spacing(8.f);
    for (int i = 0; i < 3; ++i) {
        ui::HBox* row = new ui::HBox(0, 0, kW - 32.f, 34);
        row->set_theme(theme);
        row->set_padding(0.f);
        row->set_spacing(8.f);

        char dd_title[32], btn_text[32];
        std::snprintf(dd_title, sizeof(dd_title), "行%d 操作", i + 1);
        std::snprintf(btn_text, sizeof(btn_text), "查看行%d", i + 1);

        ui::Dropdown* dd = new ui::Dropdown(0, 0, 200, 34, dd_title);
        dd->set_font(lv.font);
        dd->set_theme(theme);
        dd->add_item("详情");
        dd->add_item("编辑");
        dd->add_separator();
        dd->add_item("复制");
        dd->add_item("删除");
        dd->set_on_select([level, i, st](const char* sel) {
            if (!st) return;
            char buf[80];
            std::snprintf(buf, sizeof(buf), "第%d层·行%d 下拉：%s", level, i + 1, sel);
            st->set_text(buf);
        });
        row->add_child(dd);

        ui::Button* btn = new ui::Button(0, 0, 130, 34, btn_text);
        btn->set_font(lv.font);
        btn->set_theme(theme);
        btn->add_class("layui-btn layui-btn-sm layui-btn-normal");
        btn->set_on_click([level, i, st]() {
            if (!st) return;
            char buf[80];
            std::snprintf(buf, sizeof(buf), "第%d层·行%d 按钮被点击", level, i + 1);
            st->set_text(buf);
        });
        row->add_child(btn);

        list->add_child(row);
    }
    r.add_child(list);

    // 「创建窗口」按钮：最深一层置灰；点击后自身置灰（每窗至多一个子窗）+ 打开下一层
    const char* create_text = (level < kMaxLevel) ? "创建窗口（下一层）" : "已是最后一层";
    ui::Button* create_btn = new ui::Button(16, 250, 190, 36, create_text);
    create_btn->set_font(lv.font);
    create_btn->set_theme(theme);
    create_btn->add_class("layui-btn layui-btn-primary");
    if (level >= kMaxLevel) create_btn->set_enabled(false);
    create_btn->set_on_click([scn, theme, level, create_btn]() {
        create_btn->set_enabled(false);
        if (scn) open_level(*scn, theme, level + 1);
    });
    r.add_child(create_btn);

    // 「关闭本窗口」按钮：request_exit -> 下一帧驱动观察者关闭本窗
    ui::Button* close_btn = new ui::Button(220, 250, 130, 36, "关闭本窗口");
    close_btn->set_font(lv.font);
    close_btn->set_theme(theme);
    close_btn->add_class("layui-btn layui-btn-danger");
    close_btn->set_on_click([scn]() {
        if (scn) scn->request_exit();
    });
    r.add_child(close_btn);

    // 首次开窗：订阅主壳 Frame 驱动观察者（每帧推进全部打开中的窗口，完成后切回主壳上下文）
    if (level == 1 && !g_token) {
        g_main_scene = &source;
        g_token = source.subscribe(ui::ScenePhase::Frame,
                                   [](ui::Scene& main, float dt) {
                                       if (g_wins.empty()) return;
                                       // GLFW 事件队列全局单份：每帧统一 poll 一次
                                       g_wins.front().win->poll_events();
                                       // 快照：遍历中点击「创建窗口」新开窗，下一帧才被驱动
                                       const std::vector<LevelWin> snap = g_wins;
                                       for (LevelWin lv : snap) {
                                           if (lv.win->should_close() || !lv.scene->running()) {
                                               close_window(lv);
                                               continue;
                                           }
                                           lv.win->make_current();
                                           lv.scene->frame(dt);
                                           lv.win->swap_buffers();
                                       }
                                       main.window().make_current();  // 关键：切回主壳上下文
                                   });
    }

    g_wins.push_back(lv);
    ui::log_info("multilevel: window opened (level %d)", level);
    source.window().make_current();  // 收尾切回触发方上下文（父窗/主壳）

    if (g_status) {
        char s[96];
        if (level == 1)
            std::snprintf(s, sizeof(s),
                          "已打开第 1 层窗口（主屏居中）：逐层点「创建窗口」可深入到第 %d 层", kMaxLevel);
        else
            std::snprintf(s, sizeof(s), "已创建到第 %d 层窗口", level);
        g_status->set_text(s);
    }
}

void destroy_multilevel(PageHost& host) {
    if (host.scene && g_token) host.scene->unsubscribe(g_token);
    g_token = 0;
    close_all_windows();
    g_main_scene = nullptr;
    if (host.content) host.content->root()->close_popups();
    g_status = nullptr;
}

void build_multilevel(PageHost& host) {
    ui::ScrollBox* content = host.content;
    ui::Font* font = host.font;
    const ui::Theme* theme = host.theme;

    char bq[384];
    std::snprintf(bq, sizeof(bq),
                  "多级窗口创建（多窗口方式）：每层 = 独立 GLFW 窗口 + 独立 GL 上下文 / Renderer / Scene / Font，"
                  "共 %d 层。点「打开第 1 层窗口」主屏居中弹出；每层窗口含「创建窗口」按钮 + 列表（3 行，每行 = 下拉菜单 + 按钮）。"
                  "点「创建窗口」从父窗向右下错位创建下一层；到第 %d 层按钮自动置灰。"
                  "每窗可拖标题移动 / 最小化 / 最大化 / X 或 Esc 关闭；多层可同时存在、各自独立渲染互不污染。",
                  kMaxLevel, kMaxLevel);
    ui::BlockQuote* bq_widget = new ui::BlockQuote(800, 0, bq);
    bq_widget->set_font(font);
    bq_widget->set_theme(theme);
    host.content->add_child(bq_widget);

    ui::HBox* row = new ui::HBox(0, 0, 800, 34.f);
    row->set_padding(0.f);
    row->set_spacing(8.f);
    row->set_theme(theme);

    ui::Scene* main_scn = host.scene;
    ui::Button* open1 = new ui::Button(0, 0, 190, 34, "打开第 1 层窗口");
    open1->set_font(font);
    open1->set_theme(theme);
    open1->add_class("layui-btn layui-btn-primary");
    open1->set_on_click([main_scn, theme]() {
        if (main_scn) open_level(*main_scn, theme, 1);
    });
    row->add_child(open1);

    ui::Button* close_all = new ui::Button(0, 0, 120, 34, "全部关闭");
    close_all->set_font(font);
    close_all->set_theme(theme);
    close_all->add_class("layui-btn layui-btn-normal");
    close_all->set_on_click([]() {
        close_all_windows();
        if (g_status) g_status->set_text("已全部关闭");
    });
    row->add_child(close_all);

    host.content->add_child(row);

    g_status = new ui::Label(0, 0, 800, 0, "未打开窗口");
    g_status->set_font(font);
    g_status->set_theme(theme);
    g_status->set_color(theme->text_secondary);
    host.content->add_child(g_status);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/multilevel_window_page.cpp：多级窗口创建 demo（多窗口方式，UI-189）\n"
            "\n"
            "// 每层 = 独立 GLFW 窗口 + 独立 Renderer/Scene/Font（GL 资源绑定本窗上下文）\n"
            "void open_level(ui::Scene& source, const ui::Theme* theme, int level) {\n"
            "    ui::Window* win = new ui::Window(440, 320, \"多级窗口\");\n"
            "    if (level == 1) win->center_on_primary_monitor();   // 第 1 层主屏居中\n"
            "    else { int px,py; source.window().get_position(px,py);\n"
            "           win->set_position(px + 52, py + 36); }      // 更深层从父窗错位\n"
            "\n"
            "    win->make_current();                                // 第二独立 GL 上下文\n"
            "    ui::Renderer* renderer = new ui::Renderer(); renderer->init();\n"
            "    ui::Font* font = new ui::Font();                    // 每窗独立字体\n"
            "    font->load_from_file(\"C:/Windows/Fonts/arial.ttf\", 16.f);\n"
            "    font->load_cjk_font_auto();\n"
            "    ui::Scene* scene = new ui::Scene(*win, *renderer);\n"
            "    ui::Scene* scn = scene;        // 本窗长命对象；回调捕获，随窗销毁\n"
            "\n"
            "    ui::TitleBar* tb = new ui::TitleBar(440, 30, \"多级窗口\", win); // 拖动/X\n"
            "    tb->set_on_close([scn]() { scn->request_exit(); });\n"
            "    scene->set_event_hook(ui::SceneEvent::Key, [](ui::Scene& s) { // Esc 关本窗\n"
            "        if (s.event_key() == ui::KeyEscape) { s.request_exit(); return true; }\n"
            "        return false;\n"
            "    });\n"
            "\n"
            "    // 列表 3 行（Dropdown + Button 回写窗内状态 Label）+「创建窗口」递归下一层\n"
            "    ui::Button* create = new ui::Button(16, 250, 190, 36, \"创建窗口\");\n"
            "    create->set_on_click([=]() { create->set_enabled(false);\n"
            "                                open_level(*scn, theme, level + 1); });\n"
            "    scene->root().add_child(tb); scene->root().add_child(create);\n"
            "    // ...\n"
            "\n"
            "    // 主壳 Frame 观察者统一驱动（首次开窗时订阅一次）：\n"
            "    // 每帧对所有打开的窗口：poll -> make_current -> frame(dt) -> swap；\n"
            "    // 完成切回主壳上下文（Window::make_current），否则主窗渲染错乱。\n"
            "    token = main.subscribe(ui::ScenePhase::Frame, [](ui::Scene& m, float dt) {\n"
            "        for (auto& lv : wins) {\n"
            "            if (lv.win->should_close() || !lv.scene->running()) close(lv);\n"
            "            else { lv.win->make_current(); lv.scene->frame(dt); lv.win->swap_buffers(); }\n"
            "        }\n"
            "        m.window().make_current();\n"
            "    });\n"
            "    source.window().make_current();  // 收尾切回触发方上下文\n"
            "}\n"
            "\n"
            "// 最深层（kMaxLevel）创建按钮置灰；每窗至多创建一个子窗（点击后 set_enabled(false)）");
    }
    if (host.help) {
        const char* lines[] = {
            "多级窗口创建（多窗口方式，UI-189）",
            "",
            "思路：每层 = 独立 GLFW 窗口 + 独立 GL 上下文 / Renderer / Scene / Font，",
            "复用既有多窗口架构（UI-146 scene_page 子窗范式）。",
            "",
            "位置：",
            "  第 1 层由页面按钮打开，主显示器居中（center_on_primary_monitor）；",
            "  更深层由父窗「创建窗口」按钮创建，从父窗位置向右下错位叠放（Window::set_position）。",
            "",
            "驱动：主壳 Scene 的 Frame 观察者每帧对所有打开中的窗口：",
            "  poll_events -> make_current -> scene->frame(dt) -> swap_buffers",
            "  -> 主壳 make_current 切回（interlacing 防主窗渲染错乱）。",
            "",
            "每层内容：",
            "  自绘 TitleBar（拖动 / 最小化 / 最大化 / X，图标字体自动懒装载 UI-172）；",
            "  层级标识；VBox 列表 3 行（行 = Dropdown 下拉菜单 + Button 按钮）；",
            "  「创建窗口」（最深一层置灰）；「关闭本窗口」（X / Esc 亦可）。",
            "",
            "生命周期：",
            "  X / Esc / 关闭按钮 -> request_exit -> 下一帧驱动观察者关闭该窗并摘除跟踪项；",
            "  切页 destroy 钩子统一 unsubscribe + 全部关闭（防悬空与泄漏，UI-033/071）。",
        };
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            label->set_font(font);
            label->set_theme(theme);
            label->set_color(theme->text);
            host.help->add_child(label);
        }
    }
}

}  // namespace

static void build(PageHost& host) {
    build_multilevel(host);
}

static const PageDef kMultilevelWindowPage{51, "交互", "多级窗口",
                                           "多窗口方式创建：每层独立 GLFW 窗口逐层深入 4 层",
                                           build, destroy_multilevel, false, nullptr, 0, "Window×N / Scene / Dropdown"};

static const bool kRegistered = []() {
    page::register_page(kMultilevelWindowPage);
    return true;
}();