#pragma once

// ui::Scene——随窗口同步创建、统一驱动 UI 生命周期的观察者/场景对象（UI-146）。
//
// 把「生产者 → 分配者 → 消费者 → 清理重置」四阶段收敛为一个对象：
//   - 生产者   ：绑定 ui::Window（GLFW 边沿触发队列——鼠标/滚轮/键盘/字符/拖放）；
//   - 分配者   ：Scene::frame() 内统一调度 layout + dispatch_mouse_* + dispatch_key_* +
//                dispatch_char + advance(dt)（标准路径，收敛各应用重复的主循环模板）；
//   - 消费者   ：begin_frame(clear) + root.draw(renderer)；
//   - 清理重置 ：reset()/shutdown() 统一 flush_pending_destroys + close_popups，
//                随 ~Scene() 自动收尾，杜绝悬空与泄漏。
//                注意：不清除子树——Widget 容器默认不拥有子树，其子节点可能是栈上对象
//                （root.add_child(&栈Label)），clear_children 会对栈对象 delete 造成堆损坏。
//
// 观察者/发布订阅（全量）：
//   - subscribe(ScenePhase, fn) 阶段向量广播（一阶段可挂多个观察者）；
//   - set_event_hook(SceneEvent, hook) 逐事件钩子（返回 true = 消费，阻止进一步处理）。
//
// 多窗口：每窗口 = 一个 Scene（各自 Window/Renderer/root）。GLFW 每窗口默认独立 GL
// 上下文，各自 Renderer/Font/Texture 互不串扰。注意：禁止在同一 GL 上下文内运行两个
// Renderer（Render 实例级缓存 active_program_/scissor_stack_ 等 GL 全局状态会冲突）。

#include "ui/core/math.h"
#include "ui/ui/widget.h"

#include <array>
#include <functional>

namespace ui {
class Window;
class Renderer;

// 生命周期阶段（subscribe 广播用）。dt 为当前帧步长（秒），非时序阶段（Ready/Reset/
// Cleanup/Shutdown）dt 恒为 0。
enum class ScenePhase {
    Ready,     // Scene 构造后、首帧前：应用在此搭建 UI 树 / 挂订阅
    Reset,     // reset() 清空子树与弹窗后：应用在此重建新一轮 UI（可先 subscribe 再 reset）
    Frame,     // 每帧调度阶段（分配输入到 root）之前，dt 已算好
    PostDraw,  // 每帧 begin_frame+root.draw 之后（应用在此追加 HUD/Tooltip 等直绘）
    Cleanup,   // shutdown/reset 释放控件树与弹窗前：应用在此释放树外资源
    Shutdown,  // shutdown 最后阶段（GL 上下文仍存续）
    COUNT,
};

// 逐事件钩子（set_event_hook）。返回 true = 该事件已被消费，Scene 不再向下处理。
enum class SceneEvent {
    MouseMove,   // 每帧一次（hook 内可访问 window.mouse_*）
    MouseDown,   // 左键按下边沿
    MouseUp,     // 左键抬起边沿
    Scroll,      // 本帧滚轮量非 0
    Key,         // 有任何按键按下边沿（data=该键码；见 event_key/data）
    Char,        // 本帧收到字符（event_key 无效）
    Drop,        // 本帧收到文件拖放
    Resized,     // 窗口尺寸变化
    COUNT,
};

class Scene {
public:
    // 生产：绑定窗口/渲染器。构造即广播 ScenePhase::Ready。
    // 窗口、渲染器由调用方持有（非拥有关系），存活期必须 ≥ Scene。
    Scene(ui::Window& window, ui::Renderer& renderer);
    ~Scene();  // 清理重置：广播 Cleanup/Shutdown + flush_pending_destroys + close_popups

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // ---- 资源访问 ----
    ui::Window& window() { return window_; }
    ui::Renderer& renderer() { return renderer_; }
    // 根控件树（Scene 拥有，构造时按窗口尺寸初始化 bounds；reset 清空后重建）
    ui::Widget& root() { return root_; }
    void set_root_background(const ui::Vec4& clear_color) { clear_ = clear_color; }

    // ---- 顶部装帧（chrome）内容区原点 ----
    // 注册一个自绘顶部条（如无边框窗口的自绘 TitleBar）与它占用的顶部高度 top。
    // 库由此掌握「内容区起始 y」，Rest/Draw/HUD 等直接绘制可经 content_origin_y()
    // 统一对齐，避免各应用在 header/sidebar/content/HUD 里各自硬编码同一偏移量。
    // chrome 仍由应用作为 root 普通子控件加入（其自身以绝对坐标绘制于 y=0）；
    // 本 API 仅登记其高度与指针（不拥有、不移动 chrome），供内容区消费。
    void set_chrome(Widget* chrome, float top);
    Widget* chrome() const { return chrome_; }
    // 内容区起始 y（= 顶部 chrome 占用高度，未注册时为 0）
    float content_origin_y() const { return content_y_; }

    // ---- 全量发布-订阅（观察者）----
    using PhaseFn = std::function<void(Scene&, float dt)>;
    // 返回订阅 token（非 0），供 unsubscribe 单独摘除（页面级局部订阅切页时摘除，防悬空）。
    size_t subscribe(ScenePhase phase, PhaseFn fn);
    // 便捷重载：不关心 dt 的阶段/帧回调；同样返回 token
    size_t on(ScenePhase phase, std::function<void(Scene&)> fn);
    // 按 token 摘除单个阶段观察者；token 为 0 或不存在则无操作。
    // 与 clear_observers() 不同：仅移除这一个订阅，不清其他（App 级）订阅。
    void unsubscribe(size_t token);
    // 逐事件钩子：仅一个槽位（重复 set 覆盖）。返回 true 表示消费。
    using EventHook = std::function<bool(Scene&)>;
    void set_event_hook(SceneEvent event, EventHook hook);
    // 清空全部订阅与钩子（reset/shutdown 内部调用，防闭包悬空引用旧树/旧窗口）
    void clear_observers();
    // 最近一次 Key/Scroll/MouseDown 等事件的键码（event_key()）或滚轮量（event_scroll_dx/dy）
    int event_key() const { return event_key_; }
    float event_scroll_dx() const { return event_scroll_dx_; }
    float event_scroll_dy() const { return event_scroll_dy_; }

    // ---- 分配者/消费者开关（保留各应用迁移前按键语义，默认全开）----
    bool dispatch_mouse = true;    // mouse_move + mouse_down/up
    bool dispatch_scroll = true;   // scroll_dx/dy -> dispatch_mouse_scroll
    bool dispatch_nav_keys = true; // Tab/方向/Enter/Space/Backspace/Delete/Home/End/Escape
    bool dispatch_text = true;     // char 队列 -> dispatch_char
    bool dispatch_resize = true;   // window.resized() -> set_viewport + root.set_bounds

    // ---- 消费者：运行 ----
    // 单帧：算 dt（由调用方传入）→ 分配 → 绘制。不进入 poll 循环。
    void frame(float dt);
    // 直至窗口关闭：poll_events（生产者）→ frame
    void run();
    // 运行至超时自动退出（--smoke 通用语义）：内部按帧 poll 并累计运行时长
    void run_for(double seconds);
    bool running() const { return running_; }
    // 请求退出（Run 循环下一帧即止；如 demo_exit 按钮）
    void request_exit() { running_ = false; }

    // ---- 清理重置 ----
    // 关弹窗 + 排空延迟销毁队列 + 广播 Reset→Ready（重跑新一轮生命周期；先订阅 Reset/Ready 再调用）
    // 不删子树（子节点可能为栈对象，见类注释）
    void reset();
    // 广播 Cleanup/Shutdown + flush_pending_destroys（析构自动调用；可手动提前）。
    // 与 running_ 独立：request_exit() 只停 run 循环，shutdown() 仍会完整广播
    // Cleanup/Shutdown 阶段（此前 以 !running_ 提前 return 会吞掉收尾通知）。
    // 幂等：多次调用只广播一次。
    void shutdown();

private:
    void notify_phase(ScenePhase p, float dt);

    ui::Window& window_;
    ui::Renderer& renderer_;
    ui::Widget root_;
    ui::Widget* chrome_ = nullptr;   // 顶部装帧（自绘 TitleBar），不拥有，应用自持
    float content_y_ = 0.f;          // 内容区起始 y（顶部 chrome 占用高度）
    ui::Vec4 clear_{0.12f, 0.12f, 0.12f, 1.f};
    // 每阶段观察者表：{token, fn}；token 稳定不随删除位移（unsubscribe 按 token 精确摘除）
    struct Observer {
        size_t token;
        PhaseFn fn;
    };
    std::array<std::vector<Observer>, (size_t)ScenePhase::COUNT> phases_;
    size_t next_token_ = 1;  // 0 保留作「无有效 token」
    std::array<EventHook, (size_t)SceneEvent::COUNT> hooks_;
    bool running_ = true;
    bool shut_down_ = false;  // shutdown() 幂等标志（与 running_ 独立，见 shutdown 注释）
    bool frame_began_ = false;  // 首帧 frame() 已执行（Debug 前置断言用：set_chrome 须在首帧前）

    // 事件随路数据（供 Key/Scroll 钩子读取）
    int event_key_ = 0;
    float event_scroll_dx_ = 0.f;
    float event_scroll_dy_ = 0.f;
};

}  // namespace ui
