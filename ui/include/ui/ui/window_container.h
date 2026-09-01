#pragma once

// ui::WindowContainer——带自绘标题栏的窗口容器（UI-168）。
//
// 把分散在测试程序里的「窗口/渲染器/场景/标题条」四件套收敛为一个对象，一行创建
// 「带标题栏、居中、可拖动、可关闭」的窗口：
//   - 无边框窗口（GLFW_DECORATED=FALSE，见 init_glfw）创建即居中到主显示器工作区，
//     不再落屏幕 0,0（UI-147 回归）；
//   - 顶部自绘系统风格标题条（ui::TitleBar），注册为 Scene chrome：
//     空白区拖动整窗、右侧最小化 / 最大化(还原) / 关闭三键；
//     关闭 → window.set_should_close(true)（run 循环下一帧自然退出）；
//   - 窗口尺寸变化：容器随 Frame 自动把标题条同步为全宽（应用无需自己处理改条宽度）；
//   - 内容区起始 y 由库统一掌握（content_origin_y() = 标题条高），其余控件以此为
//     单一原点派生（UI-153 回归）。
//
// 使用惯例：
//   ui::WindowContainer container(1280, 800, "title");
//   if (!container.ok()) return 1;
//   container.titlebar().set_font(&font);   // 应用自持 Font/Theme，注入标题条样式
//   container.titlebar().set_theme(&theme);
//   container.titlebar().set_icon(&font, ui::layui_icon::app);
//   container.scene().set_root_background(...);
//   随后经 container.root() 搭建控件树，container.run() / run_for(秒) 驱动。
//
// 生命周期：Window / Renderer / Scene / TitleBar 全为本容器成员（声明序构造），
// 容器析构即按序收尾；多窗口即每窗口一个 WindowContainer。

#include "ui/gl/renderer.h"
#include "ui/gl/window.h"
#include "ui/ui/scene.h"
#include "ui/ui/titlebar.h"

#include <cstddef>

namespace ui {

class WindowContainer {
public:
    // 标题条高度（= chrome 占用高，content_origin_y() 的返回值）
    static constexpr float kTitleBarHeight = 30.f;

    WindowContainer(int width, int height, const char* title);
    ~WindowContainer();

    WindowContainer(const WindowContainer&) = delete;
    WindowContainer& operator=(const WindowContainer&) = delete;

    // ---- 资源访问 ----
    ui::Window& window() { return window_; }
    ui::Renderer& renderer() { return renderer_; }
    ui::Scene& scene() { return scene_; }
    ui::TitleBar& titlebar() { return titlebar_; }
    ui::Widget& root() { return scene_.root(); }
    // 内容区起始 y（= 标题条高；内容控件以该值为单一原点派生）
    float content_origin_y() const { return scene_.content_origin_y(); }

    // 创建是否可用（窗口句柄 + GL 上下文 + Renderer 初始化均成功）
    bool ok() const { return ok_; }

    // ---- 便捷运行（透传 Scene::run / run_for）----
    void run() { scene_.run(); }
    void run_for(double seconds) { scene_.run_for(seconds); }

private:
    bool ok_ = false;
    ui::Window window_;
    ui::Renderer renderer_;
    ui::TitleBar titlebar_;
    ui::Scene scene_;
};

}  // namespace ui