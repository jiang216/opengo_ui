// ui::WindowContainer——带自绘标题栏的窗口容器实现，见 window_container.h。
#include "ui/ui/window_container.h"

#include "ui/core/log.h"

namespace ui {

WindowContainer::WindowContainer(int width, int height, const char* title)
    : window_(width, height, title),
      titlebar_((float)width, kTitleBarHeight, title, &window_),
      scene_(window_, renderer_) {
    if (!window_.handle() || !renderer_.init()) {
        ui::log_error("WindowContainer: window/renderer init failed (%s)", title ? title : "");
        return;  // ok() == false，错误路径与应用侧一致
    }
    renderer_.set_viewport(window_.width(), window_.height());

    // 无边框窗口默认落屏幕 0,0（UI-147）→ 创建即居中，标题条为唯一拖动/关闭入口
    window_.center_on_primary_monitor();

    // 标题条：注册为 chrome（内容区原点）+ 加入 root；关闭 → should_close（run 退出）。
    // 最小化 / 最大化(还原) 由 TitleBar 内建（构造已注入 &window_）。
    scene_.root().add_child(&titlebar_);
    scene_.set_chrome(&titlebar_, kTitleBarHeight);
    titlebar_.set_on_close([this]() { window_.set_should_close(true); });

    // 窗口尺寸变化：Frame 阶段 window_.width() 已由 resize 回调更新（见 gl/window.cpp），
    // 随帧把标题条同步为全宽；应用无需为改条宽度单独挂 Resized 钩子（UI-168 回归）。
    scene_.subscribe(ScenePhase::Frame, [this](Scene&, float) {
        titlebar_.set_bounds(0.f, 0.f, (float)window_.width(), kTitleBarHeight);
    });

    ok_ = true;
}

WindowContainer::~WindowContainer() = default;

}  // namespace ui