// scene_multiwin：多窗口（每窗口独立 GL 上下文）的 ui::Scene 演示（UI-146）。
//
// 展示 ui::Scene 的多窗口能力：每窗口 = 一个 Scene + 各自 Window/Renderer/Font/root。
// GLFW 每窗口默认独立 GL 上下文；绘制某窗口前须先 make_current(该窗口)。
// 主循环逐帧：poll 各窗口 → 逐窗口 make_current + scene.frame(dt) + swap_buffers。
//
// 单窗口渲染循环由 scene.run() 收敛；多窗口因需交替上下文，用场景自行编排
// scene.frame(dt)（frame 内即分配+消费，不含 poll/swap，便于嵌入多窗口循环）。

#include "ui/core/log.h"
#include "ui/core/version.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/window.h"
#include "ui/ui/button.h"
#include "ui/ui/label.h"
#include "ui/ui/scene.h"
#include "ui/ui/theme.h"
#include "ui/ui/window_container.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// --smoke <秒>：运行指定秒后自动退出（CI/冒烟）
int main(int argc, char** argv) {
    double smoke_seconds = 0.0;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--smoke") == 0 && i + 1 < argc)
            smoke_seconds = std::atof(argv[++i]);
    }

    ui::log_info("opengl_ui %s scene_multiwin", ui::version_string());

    // ---- 窗口 A：主窗口（独立上下文；带自绘标题栏容器 UI-168）----
    ui::WindowContainer containerA(480, 360, "scene_multiwin: A");
    if (!containerA.ok()) return 1;
    ui::Font fontA;
    fontA.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
    ui::Theme themeA = ui::Theme::make_dark();
    containerA.titlebar().set_font(&fontA);
    containerA.titlebar().set_theme(&themeA);
    ui::Scene& sceneA = containerA.scene();
    sceneA.set_root_background(themeA.window_bg);
    sceneA.dispatch_nav_keys = false;  // 仅鼠标；保持简单
    ui::Widget& rootA = containerA.root();
    {
        ui::Button btn(120, 150, 240, 48, "Window A button");
        btn.set_font(&fontA);
        btn.set_theme(&themeA);
        btn.set_on_click([]() { ui::log_info("scene_multiwin: A clicked"); });
        rootA.add_child(&btn);
    }

    // ---- 窗口 B：第二个窗口（独立上下文；创建即切到 B 上下文）----
    ui::WindowContainer containerB(420, 320, "scene_multiwin: B");
    if (!containerB.ok()) return 1;
    ui::Font fontB;
    fontB.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
    ui::Theme themeB = ui::Theme::make_layui();
    containerB.titlebar().set_font(&fontB);
    containerB.titlebar().set_theme(&themeB);
    ui::Scene& sceneB = containerB.scene();
    sceneB.set_root_background(themeB.window_bg);
    sceneB.dispatch_nav_keys = false;
    ui::Widget& rootB = containerB.root();
    {
        ui::Label lb(40, 40, 340, 80, "Window B (independent context)");
        lb.set_font(&fontB);
        lb.set_theme(&themeB);
        rootB.add_child(&lb);
    }

    ui::Window& winA = containerA.window();
    ui::Window& winB = containerB.window();

    // ---- 多窗口主循环：逐窗口 make_current + frame + swap ----
    // UI-173：任一窗口点关闭即整体退出（&&）。此前用 || 需两窗口全部关闭才退出，
    // 表现为「点一个窗口的关闭没反应、必须两个都点」。
    const auto start = std::chrono::steady_clock::now();
    auto prev = start;
    while (winA.should_close() == false && winB.should_close() == false &&
           (smoke_seconds <= 0.0 ||
            std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() <
                smoke_seconds)) {
        winA.poll_events();
        winB.poll_events();
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - prev).count();
        prev = now;

        // 窗口 A
        winA.make_current();
        sceneA.frame(dt);
        winA.swap_buffers();

        // 窗口 B
        winB.make_current();
        sceneB.frame(dt);
        winB.swap_buffers();
    }

    ui::log_info("scene_multiwin: done");
    ui::Window::terminate_glfw();
    return 0;
}
