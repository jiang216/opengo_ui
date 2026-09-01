// ui::Scene——统一 UI 生命周期（生产者→分配者→消费者→清理重置）实现，见 scene.h。
#include "ui/ui/scene.h"

#include "ui/gl/renderer.h"
#include "ui/gl/window.h"

#include <algorithm>
#include <chrono>
#include <vector>

namespace ui {

Scene::Scene(ui::Window& window, ui::Renderer& renderer)
    : window_(window), renderer_(renderer) {
    root_.set_bounds(0.f, 0.f, (float)window_.width(), (float)window_.height());
    notify_phase(ScenePhase::Ready, 0.f);
}

void Scene::set_chrome(Widget* chrome, float top) {
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
        if (window_.mouse_pressed(0)) {
            if (hooks_[(size_t)SceneEvent::MouseDown] &&
                hooks_[(size_t)SceneEvent::MouseDown](*this))
                ;
            else
                root_.dispatch_mouse_down(mx, my, 0);
        }
        if (window_.mouse_released(0)) {
            if (hooks_[(size_t)SceneEvent::MouseUp] &&
                hooks_[(size_t)SceneEvent::MouseUp](*this))
                ;
            else
                root_.dispatch_mouse_up(mx, my, 0);
        }
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
    renderer_.begin_frame(clear_);
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
    if (!running_) return;  // 幂等：已 shutdown
    notify_phase(ScenePhase::Cleanup, 0.f);
    // 同 reset：只清理库内自管对象（延迟销毁队列 + 弹窗），不删子树（子节点可能为栈对象）
    ui::Widget::flush_pending_destroys();
    root_.close_popups();
    notify_phase(ScenePhase::Shutdown, 0.f);
    clear_observers();
    running_ = false;
}

}  // namespace ui
