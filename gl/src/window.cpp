#include "ui/gl/window.h"

#include "ui/core/log.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#if defined(_WIN32)
#include <windows.h>
#endif

namespace ui {

namespace {
GLFWwindow* g_primary_window = nullptr;
}

Window::Window(int width, int height, const char* title)
    : width_(width), height_(height) {
    if (!glfwInit()) {
        log_error("glfwInit failed");
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window_) {
        log_error("glfwCreateWindow failed");
        return;
    }
    g_primary_window = window_;
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))) {
        log_error("gladLoadGL failed");
    }
    log_info("GL %s | renderer %s | GLSL %s",
             reinterpret_cast<const char*>(glGetString(GL_VERSION)),
             reinterpret_cast<const char*>(glGetString(GL_RENDERER)),
             reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

    // 输入回调
    glfwSetWindowUserPointer(window_, this);
    glfwSetCursorPosCallback(window_, &Window::cursor_pos_cb);
    glfwSetMouseButtonCallback(window_, &Window::mouse_button_cb);
    glfwSetScrollCallback(window_, &Window::scroll_cb);
    glfwSetWindowSizeCallback(window_, &Window::resize_cb);
    glfwSetKeyCallback(window_, &Window::key_cb);
    glfwSetCharCallback(window_, &Window::char_cb);
}

Window::~Window() {
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
    if (g_primary_window == window_) g_primary_window = nullptr;
}

GLFWwindow* Window::primary_handle() { return g_primary_window; }

bool Window::should_close() const { return window_ ? glfwWindowShouldClose(window_) != 0 : true; }
void Window::poll_events() { glfwPollEvents(); }
void Window::swap_buffers() { glfwSwapBuffers(window_); }
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
    const bool v = mouse_released_[button];
    mouse_released_[button] = false;
    return v;
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

void Window::start_drag(float /*mouse_x*/, float /*mouse_y*/) {
    if (!window_) return;
    glfwGetWindowPos(window_, &start_wx_, &start_wy_);
#if defined(_WIN32)
    POINT pt;
    GetCursorPos(&pt);
    drag_screen_x_ = pt.x;
    drag_screen_y_ = pt.y;
#endif
    dragging_ = true;
}

void Window::update_drag(float /*mouse_x*/, float /*mouse_y*/) {
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

void Window::end_drag() { dragging_ = false; }

// ---- 光标形状 ----

void Window::set_cursor_arrow() {
    if (window_) {
        GLFWcursor* c = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        glfwSetCursor(window_, c);
        glfwDestroyCursor(c);
    }
}

void Window::set_cursor_hand() {
    if (window_) {
        GLFWcursor* c = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
        glfwSetCursor(window_, c);
        glfwDestroyCursor(c);
    }
}

void Window::set_cursor_resize_ns() {
    if (window_) {
        GLFWcursor* c = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        glfwSetCursor(window_, c);
        glfwDestroyCursor(c);
    }
}

void Window::set_cursor_resize_ew() {
    if (window_) {
        GLFWcursor* c = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        glfwSetCursor(window_, c);
        glfwDestroyCursor(c);
    }
}

void Window::set_cursor_ibeam() {
    if (window_) {
        GLFWcursor* c = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        glfwSetCursor(window_, c);
        glfwDestroyCursor(c);
    }
}

}  // namespace ui
