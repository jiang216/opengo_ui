#pragma once

struct GLFWwindow;

namespace ui {

// GLFW 窗口 + GL 3.3 Core 上下文封装（M0 骨架 + M1 鼠标追踪）
class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    bool should_close() const;
    void poll_events();
    void swap_buffers();
    void set_vsync(int interval);

    int width() const { return width_; }
    int height() const { return height_; }
    GLFWwindow* handle() const { return window_; }

    // 最近创建窗口的句柄（剪贴板等系统级 API 用；无窗口时返回 nullptr）
    static GLFWwindow* primary_handle();

    // 边沿触发：本轮 poll 内发生过窗口尺寸变化（读一次后清除）
    bool resized();

    // ---- 输入（像素坐标，y 向下，(0,0)=左上角）----
    float mouse_x() const { return mouse_x_; }
    float mouse_y() const { return mouse_y_; }
    // button: 0=左键 1=右键 2=中键
    bool mouse_down(int button) const;
    // 边沿触发：本轮 poll 内新按下/新抬起（读一次后清除）
    bool mouse_pressed(int button);
    bool mouse_released(int button);
    // 边沿触发：本轮 poll 内滚轮滚动量（dx,dy，读一次后清零）
    float scroll_dx();
    float scroll_dy();
    // 键盘（key 用 ui::Key 常量）：当前按住 / 本轮按下边沿 / 本轮抬起边沿
    bool key_down(int key) const;
    bool key_pressed(int key);
    bool key_released(int key);
    // 本轮按下边沿发生时的修饰键快照（UI-102）：SendInput 等快速序列在主循环
    // 轮询时可能已全部抬起，实时查询 key_down 拼 mods 会丢失；本方法返回
    // PRESS 回调瞬间的 mods，与 key_pressed 配合使用（先调 key_pressed 再取）。
    int key_pressed_mods(int key) const;
    // 文本输入：本轮 poll 内收到的 Unicode 码点队列（读一次后清空），返回数量
    int char_pressed_count() const { return char_count_; }
    unsigned int char_pressed_at(int i) const;
    void clear_chars();

private:
    static void cursor_pos_cb(GLFWwindow* w, double x, double y);
    static void mouse_button_cb(GLFWwindow* w, int button, int action, int mods);
    static void scroll_cb(GLFWwindow* w, double x, double y);
    static void resize_cb(GLFWwindow* w, int width, int height);
    static void key_cb(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void char_cb(GLFWwindow* w, unsigned int codepoint);

    GLFWwindow* window_ = nullptr;
    int width_ = 0;
    int height_ = 0;
    bool resized_ = false;
    float mouse_x_ = 0.f;
    float mouse_y_ = 0.f;
    bool mouse_down_[3] = {false, false, false};
    bool mouse_pressed_[3] = {false, false, false};
    bool mouse_released_[3] = {false, false, false};
    float scroll_dx_ = 0.f;
    float scroll_dy_ = 0.f;
    bool key_state_[512] = {false};
    bool key_pressed_[512] = {false};
    bool key_released_[512] = {false};
    int key_mods_[512] = {0};
    unsigned int char_queue_[64] = {0};
    int char_count_ = 0;
};

}  // namespace ui
