#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/switch.h"
#include "ui/ui/theme.h"
#include "ui/ui/titlebar.h"
#include "ui/ui/vbox.h"
#include "ui/ui/scene.h"
#include "ui/ui/widget.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/window.h"

#include <cstdio>

// ---------------------------------------------------------------------------
// 生命周期 / Scene 新功能 demo（UI-146）
//
// 本页同时承载「静态说明」与「实时演示」：
//   - 静态：周期四阶段 / 发布-订阅 / 分配开关 / 多窗口 make_current 说明（code/help tab）。
//   - 实时（经 host.scene = 框架级 ui::Scene）：
//       * PostDraw 直绘 HUD：预选区右上角叠加 Scene 阶段小标（subscribe/on + unsubscribe）。
//       * Frame 阶段订阅：每帧递增页面内计数 Label（体现 Frame 分配前阶段）。
//       * request_exit：请求整体退出（演示 Scene 退出语义）。
//       * 子窗口：第二 GLFW 窗口 + 独立 Renderer/Scene，经 Frame 观察者每帧
//         make_current->frame->swap 驱动，完成后切回主窗口上下文（Window::make_current）。
//
// 生命周期：所有订阅在切页时经 destroy_scene 统一 unsubscribe，且不持有已 clear_children
// 释放的页面控件指针——杜绝切页悬空（App 级订阅不受影响，unsubscribe 只摘本页 token）。
// ---------------------------------------------------------------------------

namespace {

constexpr float kCW = 1000.f;

// 本页实时状态：文件级单例（页面 build 返回后闭包仍可访问；切页 destroy 时复位/摘订阅）
struct SceneDemoState {
    size_t hud_token = 0;      // PostDraw 直绘 HUD 订阅 token
    size_t frame_token = 0;    // Frame 计数订阅 token
    long frames = 0;
    ui::Label* frame_label = nullptr;   // 每帧更新；切页 clear 后置 null
    ui::Font* font = nullptr;           // App 级字体（长命）

    // HUD 绘制位置（预选区屏幕坐标，订阅时快照）
    float hud_x = 0.f, hud_y = 0.f, hud_w = 0.f, hud_h = 0.f;

    // 子窗口
    ui::Scene* main_scene = nullptr;
    ui::Window* child = nullptr;
    ui::Renderer* child_renderer = nullptr;
    ui::Scene* child_scene = nullptr;
    ui::Font* child_font = nullptr;
    size_t child_token = 0;    // 驱动观察者（挂在主 Scene Frame）
};
static SceneDemoState g;

void field_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

void add_label(PageHost& host, const char* text, const ui::Vec4& color) {
    ui::Label* l = new ui::Label(0, 0, kCW, 0, text);
    l->set_font(host.font);
    l->set_theme(host.theme);
    l->set_color(color);
    host.content->add_child(l);
}

// ---- 实时：PostDraw 直绘 HUD ----
void draw_hud(ui::Scene& s) {
    if (g.hud_w <= 0.f || !g.font) return;
    ui::Renderer& r = s.renderer();
    r.draw_quad({g.hud_x, g.hud_y, g.hud_w, g.hud_h}, ui::Vec4{0.05f, 0.05f, 0.06f, 0.85f});
    r.draw_rounded_rect({g.hud_x, g.hud_y, g.hud_w, g.hud_h}, 4.f, ui::Vec4{0.30f, 0.88f, 0.65f, 1.f});
    g.font->draw_text(r, "Scene PostDraw 直绘 HUD", g.hud_x + 10.f,
                      g.hud_y + (g.hud_h - g.font->line_height()) * 0.5f,
                      ui::Vec4{0.30f, 0.88f, 0.65f, 1.f});
}

// ---- 实时：子窗口驱动（主 Scene Frame 观察者）----
static void close_child() {
    if (!g.child) return;
    if (g.main_scene && g.child_token) g.main_scene->unsubscribe(g.child_token);
    g.child_token = 0;
    if (g.child_scene) {
        g.child_scene->shutdown();
        delete g.child_scene;
        g.child_scene = nullptr;
    }
    if (g.child_renderer) {
        delete g.child_renderer;
        g.child_renderer = nullptr;
    }
    if (g.child_font) {
        delete g.child_font;
        g.child_font = nullptr;
    }
    if (g.child) {
        delete g.child;
        g.child = nullptr;
    }
    if (g.main_scene) g.main_scene->window().make_current();  // 切回主窗口上下文
}

static void open_child(ui::Scene& main) {
    if (g.child) return;  // 已存在
    g.main_scene = &main;

    g.child = new ui::Window(440, 300, "Scene 子窗口 demo（make_current）");
    if (!g.child->handle()) {
        delete g.child;
        g.child = nullptr;
        g.main_scene = nullptr;
        return;
    }
    g.child->center_on_primary_monitor();  // 同主窗口：居中且带自绘标题条（UI-147/168）
    g.child->make_current();  // 子窗口上下文置为当前（第二独立 GL 上下文）
    g.child_renderer = new ui::Renderer();
    g.child_renderer->init();
    g.child_renderer->set_viewport(g.child->width(), g.child->height());

    g.child_font = new ui::Font();
    g.child_font->load_from_file("C:/Windows/Fonts/arial.ttf", 16.f);

    g.child_scene = new ui::Scene(*g.child, *g.child_renderer);
    g.child_scene->set_root_background(ui::Vec4{0.10f, 0.12f, 0.15f, 1.f});
    ui::Widget& r = g.child_scene->root();

    auto* titlebar = new ui::TitleBar(440.f, 30.f, "Scene 子窗口 demo", g.child);
    titlebar->set_font(g.child_font);
    titlebar->set_on_close([]() {
        if (g.child_scene) g.child_scene->request_exit();  // 关闭钮 → 下一帧 close_child
    });
    r.add_child(titlebar);

    auto* title = new ui::Label(20, 50, 400, 0, "子窗口：独立 GL 上下文 + 独立 Scene");
    title->set_font(g.child_font);
    title->set_color(ui::Vec4{0.30f, 0.88f, 0.65f, 1.f});
    r.add_child(title);

    auto* sub = new ui::Label(20, 86, 400, 0,
        "由主 Scene 的 Frame 观察者每帧驱动：\nchild.make_current() -> frame(dt) -> swap_buffers()\n-> 主窗 make_current() 切回。");
    sub->set_font(g.child_font);
    sub->set_color(ui::Vec4{0.85f, 0.85f, 0.85f, 1.f});
    r.add_child(sub);

    auto* btn = new ui::Button(20, 220, 180, 40, "关闭子窗口");
    btn->set_font(g.child_font);
    btn->add_class("layui-btn layui-btn-primary");
    btn->set_on_click([]() {
        if (g.child_scene) g.child_scene->request_exit();  // 触发下一帧 close_child
    });
    r.add_child(btn);

    // 子窗内 Escape 关闭
    g.child_scene->set_event_hook(ui::SceneEvent::Key, [](ui::Scene& s) {
        if (s.event_key() == ui::KeyEscape) { s.request_exit(); return true; }
        return false;
    });

    // 驱动观察者：每帧推进子窗口，然后切回主窗口上下文
    g.child_token = main.subscribe(ui::ScenePhase::Frame, [](ui::Scene& m, float dt) {
        if (!g.child || !g.child_scene) return;
        g.child->poll_events();
        if (g.child->should_close() || !g.child_scene->running()) {
            close_child();
            return;
        }
        g.child->make_current();
        g.child_scene->frame(dt);
        g.child->swap_buffers();
        m.window().make_current();  // 关键：切回主窗口上下文，否则主窗渲染错乱
    });

    main.window().make_current();  // 收尾切回主窗口
}

void build_scene(PageHost& host) {
    g.font = host.font;

    // ---- 顶部引用块 ----
    auto* bq = new ui::BlockQuote(kCW, 0,
        "ui::Scene（UI-146）：把「生产者 → 分配者 → 消费者 → 清理重置」四阶段收敛为一个对象，"
        "随窗口同步创建。本测试程序整个外壳即由 Scene 驱动（Frame 冒烟切页 / Resized 钩子 / "
        "Key 钩子 Esc / PostDraw 截图 + HUD）。本页演示其生命周期与发布-订阅能力。");
    bq->set_font(host.font);
    bq->set_theme(host.theme);
    host.content->add_child(bq);

    // ---- Fieldset 1：四阶段 ----
    field_title(host, "生命周期四阶段（ScenePhase）");
    {
        static const char* phases[] = {
            "Ready —— Scene 构造后、首帧前：应用在此搭建 UI 树 / 挂订阅",
            "Reset —— reset() 清空自管对象后：重建新一轮 UI（可先 subscribe 再 reset）",
            "Frame —— 每帧分配输入（layout + dispatch + advance）之前，dt 已算好",
            "PostDraw —— begin_frame + root.draw 之后：应用直绘 HUD / Tooltip / 状态文本",
            "Cleanup —— shutdown/reset 释放自管对象前：应用释放树外资源",
            "Shutdown —— shutdown 最后阶段（GL 上下文仍存续）",
        };
        for (const char* p : phases) add_label(host, p, host.theme->text);
    }

    // ---- Fieldset 2：发布-订阅（实时）----
    field_title(host, "发布-订阅（subscribe / on / set_event_hook / unsubscribe）");
    {
        add_label(host,
            "阶段向量广播：subscribe(phase, fn) / on(phase, fn)，一阶段可挂多个观察者，返回 token；"
            "unsubscribe(token) 单独摘除（本页切页即调用，不影响 App 级订阅）。",
            host.theme->text);
        add_label(host,
            "逐事件钩子：set_event_hook(SceneEvent, hook) 单槽、返回 true 即消费；"
            "event_key() / event_scroll_dx/y() 取随路数据。",
            host.theme->text);

        auto* hud_sw = new ui::Switch(0, 0, 46, 24);
        hud_sw->set_font(host.font);
        hud_sw->set_theme(host.theme);
        hud_sw->set_text("ON|OFF");
        auto* hud_lbl = new ui::Label(0, 0, 0, 0, "启用「PostDraw 直绘 HUD」（预选区右上角小标）");
        hud_lbl->set_font(host.font);
        hud_lbl->set_theme(host.theme);
        hud_lbl->set_color(host.theme->text);
        auto* hud_row = new ui::VBox(0, 0, kCW, 0);
        hud_row->set_theme(host.theme);
        hud_row->set_spacing(6.f);
        hud_row->add_child(hud_sw);
        hud_row->add_child(hud_lbl);
        host.content->add_child(hud_row);
        hud_sw->set_on_change([&host](bool on) {
            ui::Scene* scn = host.scene;
            if (!scn) return;
            if (on) {
                if (!g.hud_token) {
                    // 订阅 PostDraw：快照 HUD 位置（预选区屏幕坐标）
                    const ui::Rect b = host.content->bounds();
                    g.hud_x = b.x + b.w - 230.f;
                    g.hud_y = b.y;
                    g.hud_w = 220.f;
                    g.hud_h = 28.f;
                    g.hud_token = scn->on(ui::ScenePhase::PostDraw, [](ui::Scene& s) { draw_hud(s); });
                }
            } else {
                scn->unsubscribe(g.hud_token);
                g.hud_token = 0;
                g.hud_w = 0.f;
            }
        });
    }

    // ---- Fieldset 3：Frame 阶段订阅（实时计数）----
    field_title(host, "Frame 阶段订阅（分配输入之前）");
    {
        add_label(host, "订阅 Frame 每帧递增一个计数 Label，体现「Frame 阶段早于 layout/draw」。",
                  host.theme->text);
        auto* frame_lbl = new ui::Label(0, 0, kCW, 0, "Frame 计数值：0（订阅后每帧递增）");
        frame_lbl->set_font(host.font);
        frame_lbl->set_theme(host.theme);
        frame_lbl->set_color(host.theme->success);
        host.content->add_child(frame_lbl);
        g.frame_label = frame_lbl;

        auto* frame_sw = new ui::Switch(0, 0, 46, 24);
        frame_sw->set_font(host.font);
        frame_sw->set_theme(host.theme);
        frame_sw->set_text("ON|OFF");
        auto* frame_row = new ui::VBox(0, 0, kCW, 0);
        frame_row->set_theme(host.theme);
        frame_row->set_spacing(6.f);
        frame_row->add_child(frame_sw);
        auto* frame_tl = new ui::Label(0, 0, 0, 0, "启用 Frame 每帧计数");
        frame_tl->set_font(host.font);
        frame_tl->set_theme(host.theme);
        frame_tl->set_color(host.theme->text);
        frame_row->add_child(frame_tl);
        host.content->add_child(frame_row);
        frame_sw->set_on_change([&host](bool on) {
            ui::Scene* scn = host.scene;
            if (!scn) return;
            if (on) {
                if (!g.frame_token)
                    g.frame_token = scn->on(ui::ScenePhase::Frame, [](ui::Scene&) {
                        ++g.frames;
                        if (g.frame_label) {
                            char buf[96];
                            std::snprintf(buf, sizeof(buf),
                                          "Frame 计数值：%ld（订阅后每帧递增）", g.frames);
                            g.frame_label->set_text(buf);
                        }
                    });
            } else {
                scn->unsubscribe(g.frame_token);
                g.frame_token = 0;
            }
        });
    }

    // ---- Fieldset 4：request_exit / 多窗口（实时）----
    field_title(host, "request_exit 与多窗口（Window::make_current）");
    {
        auto* row1 = new ui::VBox(0, 0, kCW, 0);
        row1->set_theme(host.theme);
        row1->set_spacing(6.f);
        auto* exit_btn = new ui::Button(0, 0, 220, 38, "请求退出 request_exit");
        exit_btn->set_font(host.font);
        exit_btn->set_theme(host.theme);
        exit_btn->add_class("layui-btn layui-btn-primary");
        exit_btn->set_on_click([&host]() { if (host.scene) host.scene->request_exit(); });
        row1->add_child(exit_btn);
        auto* exit_tl = new ui::Label(0, 0, 0, 0,
            "点击调用 scene->request_exit()：主循环下一帧即止（会关掉整个程序）。");
        exit_tl->set_font(host.font);
        exit_tl->set_theme(host.theme);
        exit_tl->set_color(host.theme->text_muted);
        row1->add_child(exit_tl);
        host.content->add_child(row1);

        auto* row2 = new ui::VBox(0, 0, kCW, 0);
        row2->set_theme(host.theme);
        row2->set_spacing(6.f);
        auto* child_btn = new ui::Button(0, 0, 220, 38, "打开演示子窗口");
        child_btn->set_font(host.font);
        child_btn->set_theme(host.theme);
        child_btn->add_class("layui-btn layui-btn-normal");
        child_btn->set_on_click([&host]() { if (host.scene) open_child(*host.scene); });
        row2->add_child(child_btn);
        auto* child_tl = new ui::Label(0, 0, 0, 0,
            "打开第二 GLFW 窗口（独立 GL 上下文 + 独立 Scene）。由主 Scene 的 Frame 观察者驱动："
            "child.make_current() -> child.frame(dt) -> swap_buffers() -> 主窗 make_current()。");
        child_tl->set_font(host.font);
        child_tl->set_theme(host.theme);
        child_tl->set_color(host.theme->text_muted);
        row2->add_child(child_tl);
        host.content->add_child(row2);
    }

    // ---- Fieldset 5：分配开关（说明，不做全局开关以免切坏外壳）----
    field_title(host, "分配开关（默认全开；App 全局语义）");
    {
        static const char* sw[] = {
            "dispatch_mouse —— mouse_move + down/up 是否分发给根树",
            "dispatch_scroll —— scroll_dx/dy 是否分发（滚轮量经 event_scroll_dx/y 取）",
            "dispatch_nav_keys —— Tab/方向/Enter/Space/Backspace/Delete/Home/End",
            "dispatch_text —— 字符队列是否分发到焦点控件",
            "dispatch_resize —— window.resized() 更新视口 + 根树尺寸",
            "Escape 不在标准模板：属应用语义（本测试程序经 Key 钩子关最上层弹窗）",
        };
        for (const char* s : sw) add_label(host, s, host.theme->text_muted);
    }

    // ---- 底部引用块 ----
    auto* bq2 = new ui::BlockQuote(kCW, 0,
        "清理重置（reset/shutdown）：只清理库内自管对象（flush_pending_destroys + close_popups），"
        "不裁子树——Widget 容器默认不拥有子树，子节点可为栈上对象（root.add_child(&栈Label)），"
        "对栈对象 delete 会堆损坏（0xC0000374）。");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// ui::Scene 生命周期（UI-146）——统一「生产者→分配者→消费者→清理重置」\n"
            "\n"
            "ui::Window window(1280, 900, \"demo\");\n"
            "ui::Renderer renderer; renderer.init();\n"
            "ui::Scene scene(window, renderer);   // 构造即广播 Ready\n"
            "\n"
            "scene.run();    // poll + 分配(layout/dispatch/advance) + 消费(begin_frame+draw) + swap\n"
            "scene.run_for(2.0f);   // run_for 承载 --smoke 通用语义\n"
            "\n"
            "// 发布-订阅：阶段向量广播 + 逐事件钩子\n"
            "auto tok = scene.on(ui::ScenePhase::PostDraw, [](ui::Scene& s) {\n"
            "    s.renderer().draw_quad({10,10,220,28}, {0,0,0,0.85f});  // HUD 直绘\n"
            "});\n"
            "scene.unsubscribe(tok);   // 按 token 单独摘除（页面级订阅切页用）\n"
            "scene.set_event_hook(ui::SceneEvent::Key, [](ui::Scene& s) {\n"
            "    if (s.event_key() == ui::KeyEscape) { s.request_exit(); return true; }\n"
            "    return false;\n"
            "});\n"
            "\n"
            "// 多窗口：每窗口独立 Scene/GL 上下文，绘制前 make_current，完成后切回\n"
            "child.make_current(); child_scene.frame(dt); child.swap_buffers();\n"
            "main.window().make_current();");
    }
    if (host.help) {
        const char* lines[] = {
            "生命周期 / Scene 说明",
            "",
            "ui::Scene 把主循环模板收敛为四阶段对象：",
            "  生产者 Window（GLFW 边沿触发输入队列）",
            "  分配者 frame()（layout + dispatch + advance）",
            "  消费者 begin_frame + root.draw",
            "  清理重置 shutdown()/reset()（自管对象，不裁子树）",
            "",
            "发布-订阅：",
            "  subscribe(phase, fn) / on(phase, fn) 阶段向量广播，返回 token；",
            "  unsubscribe(token) 单独摘除；set_event_hook(event, hook) 逐事件（单槽）。",
            "",
            "本页实时演示：",
            "  PostDraw 直绘 HUD（预选区右上角小标）；",
            "  Frame 每帧计数；request_exit 整体退出；",
            "  子窗口多窗口（make_current -> frame -> swap -> 切回）。",
            "",
            "分配开关与 Escape 语义见页内说明表。",
            "",
            "注意：切页时本页在 destroy 钩子统一 unsubscribe 全部订阅并关子窗口。",
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

void destroy_scene(PageHost& host) {
    // 切页：先关子窗口 + 摘除本页全部订阅，再让框架 clear_children 释放页面控件。
    // 顺序保证：destroy 在 preview.clear_children() 之前调用，故此处访问页面控件仍安全。
    close_child();
    if (host.scene) {
        host.scene->unsubscribe(g.hud_token);
        host.scene->unsubscribe(g.frame_token);
        host.scene->unsubscribe(g.child_token);
    }
    g.hud_token = g.frame_token = g.child_token = 0;
    g.frame_label = nullptr;
    g.hud_w = 0.f;
    g.font = nullptr;
}

}  // namespace

static void build(PageHost& host) { build_scene(host); }

static const PageDef kScenePage{48, "展示", "生命周期", "ui::Scene 统一生命周期：四阶段 / 发布-订阅 / 多窗口 make_current", build, destroy_scene, false, nullptr, 0, "Scene"};

static const bool kRegistered = []() {
    page::register_page(kScenePage);
    return true;
}();
