// ui::Scene——统一 UI 生命周期（生产者→分配者→消费者→清理重置）实现，见 scene.h。
#include "ui/ui/scene.h"

#include "ui/gl/renderer.h"
#include "ui/gl/window.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <vector>

namespace ui {

Scene::Scene(ui::Window& window, ui::Renderer& renderer)
    : window_(window), renderer_(renderer) {
    root_.set_bounds(0.f, 0.f, (float)window_.width(), (float)window_.height());
    notify_phase(ScenePhase::Ready, 0.f);
}

void Scene::set_chrome(Widget* chrome, float top) {
    // Debug 前置断言：chrome 必须在首帧 layout() 之前注册（构造后/Ready 阶段），
    // 否则 content_origin_y() 在首帧后改值，已按旧原点排布的内容不会重排，静默错位（UI-153/168）。
    assert(!frame_began_ && "Scene::set_chrome() must be called before the first frame()");
    chrome_ = chrome;
    content_y_ = top < 0.f ? 0.f : top;
}

Scene::~Scene() {
    shutdown();
}

size_t Scene::subscribe(ScenePhase phase, PhaseFn fn) {
    const size_t i = (size_t)phase;
    if (i >= phases_.size() || !fn) return 0;
    size_t token = next_token_++;
    if (token == 0) token = next_token_++;  // 0 保留作「无有效 token」，跳过
    phases_[i].push_back({token, std::move(fn)});
    return token;
}

size_t Scene::on(ScenePhase phase, std::function<void(Scene&)> fn) {
    if (!fn) return 0;
    return subscribe(phase, [fn = std::move(fn)](Scene& s, float) { fn(s); });
}

void Scene::unsubscribe(size_t token) {
    if (token == 0) return;
    for (auto& v : phases_) {
        for (auto it = v.begin(); it != v.end(); ++it) {
            if (it->token == token) {
                v.erase(it);
                return;
            }
        }
    }
}

void Scene::set_event_hook(SceneEvent event, EventHook hook) {
    const size_t i = (size_t)event;
    if (i < hooks_.size()) hooks_[i] = std::move(hook);
}

void Scene::clear_observers() {
    for (auto& v : phases_) v.clear();
    for (auto& h : hooks_) h = nullptr;
}

void Scene::notify_phase(ScenePhase p, float dt) {
    const auto& v = phases_[(size_t)p];
    // 快照语义：观察者可能在回调中再订阅/订阅者销毁自身控件（UI-072/073 悬空治理）
    std::vector<PhaseFn> snap;
    snap.reserve(v.size());
    for (const auto& ob : v) snap.push_back(ob.fn);
    for (auto& fn : snap) {
        if (fn) fn(*this, dt);
    }
}

// 单帧：分配（layout + 各输入 dispatch + advance）→ 消费（begin_frame + draw）。
// 标准路径收敛各应用重复的主循环模板；按键/滚轮/鼠标明细开关见各 public bool。
void Scene::frame(float dt) {
    frame_began_ = true;  // 首帧起 set_chrome 前置断言生效（见 set_chrome）
    // 鼠标按下/抬起边沿在 Frame 订阅者之前快照（peek 非消耗读）。控件树分发依赖这些边沿，
    // 而 Frame 订阅者（如应用拖拽逻辑）也会消费 window.mouse_pressed/released——若 Scene 在
    // 订阅者之后才读，边沿会被最先读者吞掉，导致控件树永远收不到按下/抬起（标题栏等按钮失效，
    // UI-173）。此处用非消耗 peek 各取所需：分发者与订阅者共享同一边沿。
    const bool mouse_down_edge = dispatch_mouse && window_.mouse_pressed_peek(0);
    const bool mouse_up_edge = dispatch_mouse && window_.mouse_released_peek(0);
    // 帧前派发：媒体推进/动画/工具逻辑（如 video.advance、carousel.advance）
    notify_phase(ScenePhase::Frame, dt);
    root_.advance(dt);

    // 窗口尺寸变化 -> 视口与根树
    if (dispatch_resize && window_.resized()) {
        renderer_.set_viewport(window_.width(), window_.height());
        root_.set_bounds(0.f, 0.f, (float)window_.width(), (float)window_.height());
        if (hooks_[(size_t)SceneEvent::Resized] &&
            hooks_[(size_t)SceneEvent::Resized](*this)) {
            // 已消费（应用自处理布局）；仍继续本轮事件分发
        }
    }

    root_.layout();

    // ---- 分配：鼠标 / 滚轮 ----
    const float mx = window_.mouse_x(), my = window_.mouse_y();
    if (dispatch_mouse) {
        if (hooks_[(size_t)SceneEvent::MouseMove] &&
            hooks_[(size_t)SceneEvent::MouseMove](*this))
            ;  // 已消费
        else
            root_.dispatch_mouse_move(mx, my);
        if (mouse_down_edge) {
            if (hooks_[(size_t)SceneEvent::MouseDown] &&
                hooks_[(size_t)SceneEvent::MouseDown](*this))
                ;
            else
                root_.dispatch_mouse_down(mx, my, 0);
        }
        if (mouse_up_edge) {
            if (hooks_[(size_t)SceneEvent::MouseUp] &&
                hooks_[(size_t)SceneEvent::MouseUp](*this))
                ;
            else
                root_.dispatch_mouse_up(mx, my, 0);
        }
        // UI-184：鼠标分发后收尾——
        // ① 释放丢失（失焦/光标离开时有键按下）的复位标记已由 mouse_released_peek 本轮
        //    读到 up 并分发，此处确认清除并举证把对应键按下状态复位为 false；
        // ② 依 root 捕获者状态同步 OS 级鼠标捕获：有捕获者 → SetCapture（移出窗口释放仍
        //    回调本窗口 → dispatch_mouse_up 正常复位），否则 ReleaseCapture。
        window_.settle_mouse_reset();
        if (root_.capture_widget()) window_.acquire_mouse_capture();
        else window_.release_mouse_capture();
    }
    if (dispatch_scroll) {
        event_scroll_dx_ = window_.scroll_dx();
        event_scroll_dy_ = window_.scroll_dy();
        if (event_scroll_dx_ != 0.f || event_scroll_dy_ != 0.f) {
            const bool consumed =
                hooks_[(size_t)SceneEvent::Scroll] && hooks_[(size_t)SceneEvent::Scroll](*this);
            if (!consumed)
                root_.dispatch_mouse_scroll(mx, my, event_scroll_dx_, event_scroll_dy_);
        }
    }

    // ---- 分配：键盘（导航键 + 文本编辑键）----
    if (dispatch_nav_keys) {
        // 逐键检查；任一键被 Key 钩子消费（返回 true）则不再向下分发（应用自定义键如 video 空格）
        auto dispatch = [&](int key, int mods) {
            if (!window_.key_pressed(key)) return;
            event_key_ = key;
            if (hooks_[(size_t)SceneEvent::Key] && hooks_[(size_t)SceneEvent::Key](*this))
                return;
            root_.dispatch_key_down(key, mods);
        };
        for (int nav : {ui::KeyTab, ui::KeyUp, ui::KeyDown, ui::KeyLeft, ui::KeyRight}) {
            dispatch(nav, window_.key_down(ui::KeyLeftShift) ? ui::ModShift : 0);
        }
        for (int k : {ui::KeyEnter, ui::KeySpace, ui::KeyBackspace, ui::KeyDelete, ui::KeyHome,
                      ui::KeyEnd}) {
            dispatch(k, 0);
        }
        // Escape 属应用语义（如 layui_test 关弹窗），由各应用经 Key 钩子自处理，标准模板不外发。
    }

    // ---- 分配：字符 ----
    if (dispatch_text) {
        const int nchar = window_.char_pressed_count();
        for (int i = 0; i < nchar; ++i) {
            event_key_ = 0;
            if (hooks_[(size_t)SceneEvent::Char] && hooks_[(size_t)SceneEvent::Char](*this))
                continue;
            root_.dispatch_char(window_.char_pressed_at(i));
        }
        window_.clear_chars();
    }

    // ---- 消费：绘制 ----
    // 事件分发（鼠标/滚轮/键盘）后、绘制前再布局一次（UI-157）：事件回调（如点击）常在
    // 同帧里 new 控件 + add_child，而帧首 layout() 已跑过 → 新控件保持 (0,0,0,0) 会被
    // 本帧 draw() 画在左上角闪一帧。layout() 带 `dirty_ || dirty_children_` 门控，无变更
    // 帧即时 return（零开销），有变更（事件阶段新建）时把新控件排位后才绘制，消除 (0,0) 闪现。
    renderer_.begin_frame(clear_);
    root_.layout();
    root_.draw(renderer_);

    // 帧后派发：HUD / Tooltip / 状态文本直绘（GL 上下文与 renderer 仍处于可用态）
    notify_phase(ScenePhase::PostDraw, dt);
}

void Scene::run() {
    auto prev = std::chrono::steady_clock::now();
    while (running_ && !window_.should_close()) {
        window_.poll_events();
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - prev).count();
        prev = now;
        frame(dt);
        window_.swap_buffers();
    }
}

void Scene::run_for(double seconds) {
    const auto start = std::chrono::steady_clock::now();
    auto prev = start;
    while (running_ && !window_.should_close()) {
        window_.poll_events();
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - prev).count();
        prev = now;
        frame(dt);
        window_.swap_buffers();
        if (std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() >=
            seconds) {
            break;
        }
    }
}

void Scene::reset() {
    // 清空观察者防闭包悬空引用旧树（UI-072/073/111 悬空治理）；随后广播 Reset→Ready
    clear_observers();
    // 清理库内自管对象：排空延迟销毁队列（flush 仅删 destroy() 标记的堆控件）+ 关弹窗。
    // 注意：不调用 clear_children()——Widget 容器默认不拥有子树，其子节点可能是栈上对象
    // （如 root.add_child(&栈Label)），clear_children 会对栈对象 delete 造成堆损坏（UI-146）。
    ui::Widget::flush_pending_destroys();
    notify_phase(ScenePhase::Reset, 0.f);
    root_.close_popups();
    root_.set_bounds(0.f, 0.f, (float)window_.width(), (float)window_.height());
    notify_phase(ScenePhase::Ready, 0.f);
}

void Scene::shutdown() {
    // 幂等：已 shutdown。注意不能以 !running_ 判断——request_exit() 已把 running_ 置 false，
    // 此时 shutdown()（尤其 ~Scene() 于析构路径调用）仍必须完整广播 Cleanup/Shutdown、
    // 冲刷延迟销毁队列并关弹窗，否则 cleanup 观察者收不到通知、销毁队列残留（泄漏）。
    if (shut_down_) return;
    shut_down_ = true;
    notify_phase(ScenePhase::Cleanup, 0.f);
    // 同 reset：只清理库内自管对象（延迟销毁队列 + 弹窗），不删子树（子节点可能为栈对象）
    ui::Widget::flush_pending_destroys();
    root_.close_popups();
    notify_phase(ScenePhase::Shutdown, 0.f);
    clear_observers();
    running_ = false;
}

}  // namespace ui
