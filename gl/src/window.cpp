#include "ui/gl/window.h"

#include "ui/core/log.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#endif

namespace ui {

namespace {
GLFWwindow* g_primary_window = nullptr;
bool g_glfw_initialized = false;
bool g_glad_loaded = false;
}

// 全局初始化：glfwInit + OpenGL 函数加载（整个进程只需调一次）
void Window::init_glfw() {
    if (g_glfw_initialized) return;
    if (!glfwInit()) {
        log_error("glfwInit failed");
        return;
    }
    g_glfw_initialized = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
}

// 全局终止：glfwTerminate（应用退出时调一次，销毁所有窗口和上下文）
void Window::terminate_glfw() {
    if (!g_glfw_initialized) return;
    glfwTerminate();
    g_glfw_initialized = false;
    g_glad_loaded = false;
    g_primary_window = nullptr;
}

Window::Window(int width, int height, const char* title)
    : width_(width), height_(height) {
    if (!g_glfw_initialized) init_glfw();
    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window_) {
        log_error("glfwCreateWindow failed");
        return;
    }
    g_primary_window = window_;
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    if (!g_glad_loaded) {
        if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))) {
            log_error("gladLoadGL failed");
        } else {
            g_glad_loaded = true;
            log_info("GL %s | renderer %s | GLSL %s",
                     reinterpret_cast<const char*>(glGetString(GL_VERSION)),
                     reinterpret_cast<const char*>(glGetString(GL_RENDERER)),
                     reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
        }
    }

    // 输入回调
    glfwSetWindowUserPointer(window_, this);
    glfwSetCursorPosCallback(window_, &Window::cursor_pos_cb);
    glfwSetMouseButtonCallback(window_, &Window::mouse_button_cb);
    glfwSetScrollCallback(window_, &Window::scroll_cb);
    glfwSetWindowSizeCallback(window_, &Window::resize_cb);
    glfwSetKeyCallback(window_, &Window::key_cb);
    glfwSetCharCallback(window_, &Window::char_cb);
    glfwSetDropCallback(window_, &Window::drop_cb);
    glfwSetWindowFocusCallback(window_, &Window::focus_cb);
    glfwSetCursorEnterCallback(window_, &Window::cursor_enter_cb);
}

Window::~Window() {
    if (window_) glfwDestroyWindow(window_);
    if (g_primary_window == window_) g_primary_window = nullptr;
}

GLFWwindow* Window::primary_handle() { return g_primary_window; }

bool Window::should_close() const { return window_ ? glfwWindowShouldClose(window_) != 0 : true; }
void Window::poll_events() { glfwPollEvents(); }
void Window::wait_events_timeout(double timeout) { glfwWaitEventsTimeout(timeout); }
void Window::swap_buffers() { glfwSwapBuffers(window_); }
void Window::make_current() {
    if (window_) glfwMakeContextCurrent(window_);
}
void Window::set_vsync(int interval) { glfwSwapInterval(interval); }

bool Window::resized() {
    const bool v = resized_;
    resized_ = false;
    return v;
}

bool Window::mouse_down(int button) const {
    if (button < 0 || button > 2) return false;
    return mouse_down_[button];
}

bool Window::mouse_pressed(int button) {
    if (button < 0 || button > 2) return false;
    const bool v = mouse_pressed_[button];
    mouse_pressed_[button] = false;
    return v;
}

bool Window::mouse_released(int button) {
    if (button < 0 || button > 2) return false;
    // UI-184：除真实 release 边沿外，失焦/光标离开触发的「释放丢失待复位」也视作抬起，
    // 供按边沿消费的调用方及时复位 pressed_/capture_（自愈卡死态）。
    const bool v = mouse_released_[button] || mouse_capture_lost_[button];
    mouse_released_[button] = false;
    if (mouse_capture_lost_[button]) {
        mouse_capture_lost_[button] = false;
        mouse_down_[button] = false;
    }
    return v;
}

bool Window::mouse_pressed_peek(int button) const {
    if (button < 0 || button > 2) return false;
    return mouse_pressed_[button];
}

bool Window::mouse_released_peek(int button) const {
    if (button < 0 || button > 2) return false;
    // UI-184：同 mouse_released，失焦/光标离开的带复位标记也向 peek 读者暴露为抬起边沿。
    return mouse_released_[button] || mouse_capture_lost_[button];
}

void Window::settle_mouse_reset() {
    for (int i = 0; i < 3; ++i) {
        if (mouse_capture_lost_[i]) {
            mouse_capture_lost_[i] = false;
            mouse_down_[i] = false;
        }
    }
}

void Window::cursor_pos_cb(GLFWwindow* w, double x, double y) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    self->mouse_x_ = (float)x;
    self->mouse_y_ = (float)y;
}

void Window::mouse_button_cb(GLFWwindow* w, int button, int action, int mods) {
    (void)mods;
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self || button < 0 || button > 2) return;
    if (action == GLFW_PRESS) {
        self->mouse_down_[button] = true;
        self->mouse_pressed_[button] = true;
    } else if (action == GLFW_RELEASE) {
        self->mouse_down_[button] = false;
        self->mouse_released_[button] = true;
    }
}

// UI-184：Windows 下用 SetCapture 把后续 WM_LBUTTONUP 锁定投递给本窗口，即使指针移出
// 客户区/失焦释放也能收到 GLFW_RELEASE → pressed_/capture_ 复位，根治「移出窗口释放丢失」。
void Window::acquire_mouse_capture() {
    if (!window_ || os_mouse_captured_) return;
    os_mouse_captured_ = true;
#if defined(_WIN32)
    HWND hwnd = glfwGetWin32Window(window_);
    if (hwnd) SetCapture(hwnd);
#else
    (void)0;
#endif
}

void Window::release_mouse_capture() {
    if (!window_ || !os_mouse_captured_) return;
    os_mouse_captured_ = false;
#if defined(_WIN32)
    HWND hwnd = glfwGetWin32Window(window_);
    if (hwnd) ReleaseCapture();
#else
    (void)0;
#endif
}

// 焦点丢失：若仍有键处于按下，则该按下/释放序列可能已脱离本窗口投递（失焦后 OS 捕获被转移），
// 把对应键标记为「释放丢失待复位」。下次 Scene::frame 由 mouse_released_ 读到 up → 分发
// dispatch_mouse_up → 清 pressed_/capture_，自愈卡死态（UI-184）。
void Window::focus_cb(GLFWwindow* w, int focused) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self || focused) return;  // 获得焦点不处理
    for (int i = 0; i < 3; ++i) {
        if (self->mouse_down_[i]) self->mouse_capture_lost_[i] = true;
    }
}

// 光标离开：同理，若仍有键按下且指针已移出，释放极可能落在窗口外不回调本窗口 → 置复位标记。
void Window::cursor_enter_cb(GLFWwindow* w, int entered) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self || entered) return;  // 进入不处理
    for (int i = 0; i < 3; ++i) {
        if (self->mouse_down_[i]) self->mouse_capture_lost_[i] = true;
    }
}

void Window::scroll_cb(GLFWwindow* w, double x, double y) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    self->scroll_dx_ += (float)x;
    self->scroll_dy_ += (float)y;
}

void Window::resize_cb(GLFWwindow* w, int width, int height) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self || width <= 0 || height <= 0) return;
    self->width_ = width;
    self->height_ = height;
    self->resized_ = true;
}

float Window::scroll_dx() {
    const float v = scroll_dx_;
    scroll_dx_ = 0.f;
    return v;
}

float Window::scroll_dy() {
    const float v = scroll_dy_;
    scroll_dy_ = 0.f;
    return v;
}

bool Window::key_down(int key) const {
    if (key < 0 || key >= 512) return false;
    return key_state_[key];
}

bool Window::key_pressed(int key) {
    if (key < 0 || key >= 512) return false;
    const bool v = key_pressed_[key];
    key_pressed_[key] = false;
    return v;
}

int Window::key_pressed_mods(int key) const {
    if (key < 0 || key >= 512) return 0;
    return key_mods_[key];  // 惰性清除：下一轮 PRESS 覆盖，仅边沿消费时读取
}

bool Window::key_released(int key) {
    if (key < 0 || key >= 512) return false;
    const bool v = key_released_[key];
    key_released_[key] = false;
    return v;
}

void Window::key_cb(GLFWwindow* w, int key, int scancode, int action, int mods) {
    (void)scancode;
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self || key < 0 || key >= 512) return;
    if (action == GLFW_PRESS) {
        self->key_state_[key] = true;
        self->key_pressed_[key] = true;
        self->key_mods_[key] = mods;  // UI-102：记录按下瞬间修饰键
    } else if (action == GLFW_RELEASE) {
        self->key_state_[key] = false;
        self->key_released_[key] = true;
    }
}

unsigned int Window::char_pressed_at(int i) const {
    if (i < 0 || i >= char_count_) return 0;
    return char_queue_[i];
}

void Window::clear_chars() { char_count_ = 0; }

void Window::char_cb(GLFWwindow* w, unsigned int codepoint) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    if (self->char_count_ < 64) self->char_queue_[self->char_count_++] = codepoint;
}

void Window::drop_cb(GLFWwindow* w, int count, const char** paths) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(w));
    if (!self) return;
    for (int i = 0; i < count; ++i)
        self->drop_paths_.emplace_back(paths[i]);
}

// ---- 窗口管理 ----

void Window::set_title(const char* title) {
    if (window_) glfwSetWindowTitle(window_, title);
}

void Window::set_icon(int count, const GLFWimage* images) {
    if (window_) glfwSetWindowIcon(window_, count, images);
}

void Window::set_position(int xpos, int ypos) {
    if (window_) glfwSetWindowPos(window_, xpos, ypos);
}

void Window::get_position(int& xpos, int& ypos) const {
    if (window_) glfwGetWindowPos(window_, &xpos, &ypos);
    else { xpos = 0; ypos = 0; }
}

void Window::center_on_primary_monitor() {
    if (!window_) return;
    int wsx, wsy, wsw, wsh;
    get_monitor_workarea(get_primary_monitor(), wsx, wsy, wsw, wsh);
    int ww, wh;
    get_size(ww, wh);
    // 窗口大于工作区时至少贴左上（不出现窗口超出屏幕不可达）
    int x = wsx + (wsw > ww ? (wsw - ww) / 2 : 0);
    int y = wsy + (wsh > wh ? (wsh - wh) / 2 : 0);
    set_position(x, y);
}

void Window::set_size(int width, int height) {
    if (window_) glfwSetWindowSize(window_, width, height);
}

void Window::get_size(int& width, int& height) const {
    if (window_) glfwGetWindowSize(window_, &width, &height);
    else { width = 0; height = 0; }
}

void Window::set_size_limits(int minw, int minh, int maxw, int maxh) {
    if (window_) glfwSetWindowSizeLimits(window_, minw, minh, maxw, maxh);
}

void Window::set_aspect_ratio(int numer, int denom) {
    if (window_) glfwSetWindowAspectRatio(window_, numer, denom);
}

void Window::get_framebuffer_size(int& width, int& height) const {
    if (window_) glfwGetFramebufferSize(window_, &width, &height);
    else { width = 0; height = 0; }
}

void Window::get_frame_size(int& left, int& top, int& right, int& bottom) const {
    if (window_) glfwGetWindowFrameSize(window_, &left, &top, &right, &bottom);
    else { left = 0; top = 0; right = 0; bottom = 0; }
}

float Window::get_content_scale() const {
    if (!window_) return 1.f;
    float xscale = 1.f, yscale = 1.f;
    glfwGetWindowContentScale(window_, &xscale, &yscale);
    return xscale;
}

float Window::get_opacity() const {
    return window_ ? glfwGetWindowOpacity(window_) : 1.f;
}

void Window::set_opacity(float opacity) {
    if (window_) glfwSetWindowOpacity(window_, opacity);
}

void Window::iconify() { if (window_) glfwIconifyWindow(window_); }
void Window::restore() { if (window_) glfwRestoreWindow(window_); }
void Window::maximize() { if (window_) glfwMaximizeWindow(window_); }
void Window::show() { if (window_) glfwShowWindow(window_); }
void Window::hide() { if (window_) glfwHideWindow(window_); }
void Window::focus() { if (window_) glfwFocusWindow(window_); }
void Window::request_attention() { if (window_) glfwRequestWindowAttention(window_); }

bool Window::is_visible() const {
    return window_ && glfwGetWindowAttrib(window_, GLFW_VISIBLE);
}

bool Window::is_iconified() const {
    return window_ && glfwGetWindowAttrib(window_, GLFW_ICONIFIED);
}

bool Window::is_maximized() const {
    return window_ && glfwGetWindowAttrib(window_, GLFW_MAXIMIZED);
}

bool Window::is_focused() const {
    return window_ && glfwGetWindowAttrib(window_, GLFW_FOCUSED);
}

bool Window::is_resizable() const {
    return window_ && glfwGetWindowAttrib(window_, GLFW_RESIZABLE);
}

bool Window::is_decorated() const {
    return window_ && glfwGetWindowAttrib(window_, GLFW_DECORATED);
}

void Window::set_should_close(bool value) {
    if (window_) glfwSetWindowShouldClose(window_, value ? GLFW_TRUE : GLFW_FALSE);
}

void Window::set_resizable(bool value) {
    if (window_) glfwSetWindowAttrib(window_, GLFW_RESIZABLE, value ? GLFW_TRUE : GLFW_FALSE);
}

void Window::set_decorated(bool value) {
    if (window_) glfwSetWindowAttrib(window_, GLFW_DECORATED, value ? GLFW_TRUE : GLFW_FALSE);
}

void Window::set_auto_iconify(bool value) {
    if (window_) glfwSetWindowAttrib(window_, GLFW_AUTO_ICONIFY, value ? GLFW_TRUE : GLFW_FALSE);
}

void Window::set_floating(bool value) {
    if (window_) glfwSetWindowAttrib(window_, GLFW_FLOATING, value ? GLFW_TRUE : GLFW_FALSE);
}

// ---- 剪贴板 ----

void Window::set_clipboard_string(const char* string) {
    if (window_) glfwSetClipboardString(window_, string);
}

const char* Window::get_clipboard_string() const {
    return window_ ? glfwGetClipboardString(window_) : nullptr;
}

// ---- 时间 ----

double Window::get_time() { return glfwGetTime(); }
void Window::set_time(double time) { glfwSetTime(time); }

// ---- 显示器 ----

GLFWmonitor* Window::get_primary_monitor() { return glfwGetPrimaryMonitor(); }

GLFWmonitor** Window::get_monitors(int& count) { return glfwGetMonitors(&count); }

void Window::get_monitor_position(GLFWmonitor* monitor, int& xpos, int& ypos) {
    if (monitor) glfwGetMonitorPos(monitor, &xpos, &ypos);
    else { xpos = 0; ypos = 0; }
}

void Window::get_monitor_workarea(GLFWmonitor* monitor, int& xpos, int& ypos, int& width, int& height) {
    if (monitor) glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);
    else { xpos = 0; ypos = 0; width = 0; height = 0; }
}

void Window::get_monitor_content_scale(GLFWmonitor* monitor, float& xscale, float& yscale) {
    if (monitor) glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    else { xscale = 1.f; yscale = 1.f; }
}

const char* Window::get_monitor_name(GLFWmonitor* monitor) {
    return monitor ? glfwGetMonitorName(monitor) : nullptr;
}

void Window::get_video_modes(GLFWmonitor* monitor, int& count, void*& modes) {
    if (monitor) {
        modes = (void*)glfwGetVideoModes(monitor, &count);
    } else {
        count = 0;
        modes = nullptr;
    }
}

void Window::get_video_mode(GLFWmonitor* monitor, void*& mode) {
    mode = monitor ? (void*)glfwGetVideoMode(monitor) : nullptr;
}

// ---- 拖拽（无边框窗口）----
// 使用 Win32 GetCursorPos（绝对屏幕坐标）计算增量，
// 避免 GLFW cursor_pos_cb（窗口相对坐标）在 glfwSetWindowPos 后因原点偏移
// 导致回调值与 glfwGetCursorPos 不一致，产生 6px 振荡（见 [DRAG] 调试日志）。

void Window::start_drag(float mouse_x, float mouse_y) {
    if (!window_) return;
    glfwGetWindowPos(window_, &start_wx_, &start_wy_);
#if defined(_WIN32)
    POINT pt;
    GetCursorPos(&pt);
    drag_screen_x_ = pt.x;
    drag_screen_y_ = pt.y;
#else
    // 非 Windows 回退：GLFW cursor_pos_cb 给出窗口相对坐标。记录按下时鼠标相对
    // 窗口原点的偏移，update_drag 用「当前鼠标 − 按下时鼠标」的增量平移窗口：
    // 新位置 = 起始窗口位置 + (mouse − drag_off)。此前 drag_off_* 从未被赋值
    // 恒为 0，dx=mouse_x 直接当绝对坐标用，窗口跳到 (start_wx_+mouse_x,...) 错误。
    drag_off_x_ = mouse_x;
    drag_off_y_ = mouse_y;
#endif
    dragging_ = true;
    acquire_mouse_capture();  // UI-184：整窗拖动也持 OS 捕获，移出窗口释放不丢
}

void Window::update_drag(float mouse_x, float mouse_y) {
    if (!window_ || !dragging_) return;
#if defined(_WIN32)
    POINT pt;
    GetCursorPos(&pt);
    int dx = pt.x - drag_screen_x_;
    int dy = pt.y - drag_screen_y_;
    glfwSetWindowPos(window_, start_wx_ + dx, start_wy_ + dy);
#else
    // 非 Windows 平台回退到 GLFW 坐标（可能有抖动）
    float dx = mouse_x - drag_off_x_;
    float dy = mouse_y - drag_off_y_;
    glfwSetWindowPos(window_, start_wx_ + (int)dx, start_wy_ + (int)dy);
#endif
}

void Window::end_drag() {
    dragging_ = false;
    release_mouse_capture();  // UI-184：拖动结束释放 OS 捕获
}

// ---- 光标形状 ----

namespace {
// 函数局部静态：程序退出时自动调用 CursorIconManager 析构释放全部 GLFW 光标，
// 消除原 `new CursorIconManager()` 永不 delete 的全局泄漏。
CursorIconManager& get_cursor_manager() {
    static CursorIconManager manager;
    return manager;
}

// 标准系统光标缓存：arrow/hand/ns/ew/ibeam 建一次复用，
// 避免每次 set_cursor_* 都 glfwCreateStandardCursor + Destroy（频繁切换时反复创建销毁）。
struct StandardCursorCache {
    GLFWcursor* arrow = nullptr;
    GLFWcursor* hand = nullptr;
    GLFWcursor* vresize = nullptr;
    GLFWcursor* hresize = nullptr;
    GLFWcursor* ibeam = nullptr;
    bool built = false;

    void ensure() {
        if (built) return;
        arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
        vresize = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        hresize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        ibeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        built = true;
    }
    void destroy() {
        if (arrow) glfwDestroyCursor(arrow);
        if (hand) glfwDestroyCursor(hand);
        if (vresize) glfwDestroyCursor(vresize);
        if (hresize) glfwDestroyCursor(hresize);
        if (ibeam) glfwDestroyCursor(ibeam);
        arrow = hand = vresize = hresize = ibeam = nullptr;
        built = false;
    }
};
StandardCursorCache& standard_cursors() {
    static StandardCursorCache cache;
    return cache;
}

// 五个 set_cursor_* 共用：确保标准光标缓存已建立，然后把光标应用到窗口。
void apply_standard_cursor(GLFWwindow* w, GLFWcursor* cursor) {
    if (!w) return;
    standard_cursors().ensure();
    if (cursor) glfwSetCursor(w, cursor);
}
}

void Window::set_cursor(CursorType type) {
    if (!window_) return;
    switch (type) {
        case CursorType::Arrow:
            set_cursor_arrow(); break;
        case CursorType::Hand:
            set_cursor_hand(); break;
        case CursorType::IBeam:
            set_cursor_ibeam(); break;
        case CursorType::ResizeEW: {
            GLFWcursor* c = get_cursor_manager().get_cursor(0);  // 0° = EW
            if (c) glfwSetCursor(window_, c);
            break;
        }
        case CursorType::ResizeNS: {
            GLFWcursor* c = get_cursor_manager().get_cursor(1);  // 90° = NS
            if (c) glfwSetCursor(window_, c);
            break;
        }
        case CursorType::ResizeNWSE: {
            GLFWcursor* c = get_cursor_manager().get_cursor(2);  // 45° = NWSE
            if (c) glfwSetCursor(window_, c);
            break;
        }
        case CursorType::ResizeNESW: {
            GLFWcursor* c = get_cursor_manager().get_cursor(3);  // 135° = NESW
            if (c) glfwSetCursor(window_, c);
            break;
        }
    }
}

void Window::set_cursor_arrow() {
    apply_standard_cursor(window_, standard_cursors().arrow);
}

void Window::set_cursor_hand() {
    apply_standard_cursor(window_, standard_cursors().hand);
}

void Window::set_cursor_resize_ns() {
    apply_standard_cursor(window_, standard_cursors().vresize);
}

void Window::set_cursor_resize_ew() {
    apply_standard_cursor(window_, standard_cursors().hresize);
}

void Window::set_cursor_ibeam() {
    apply_standard_cursor(window_, standard_cursors().ibeam);
}

}  // namespace ui
