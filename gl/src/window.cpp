#include "ui/gl/window.h"

#include "ui/core/log.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>

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

}  // namespace ui
