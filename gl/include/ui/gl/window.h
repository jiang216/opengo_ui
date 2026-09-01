#pragma once

#include "ui/gl/cursor_icon_manager.h"

#include <string>
#include <vector>

struct GLFWwindow;
struct GLFWmonitor;
struct GLFWimage;

namespace ui {

// GLFW 窗口 + GL 3.3 Core 上下文封装（M0 骨架 + M1 鼠标追踪）
class Window {
public:
    // GLFW 全局初始化/终止（整个进程只需调一次）；
    // 多窗口架构下 ~Window() 不再调 glfwTerminate()，由应用层在退出时调 terminate_glfw()
    static void init_glfw();
    static void terminate_glfw();

    Window(int width, int height, const char* title);
    ~Window();

    bool should_close() const;
    void poll_events();
    void wait_events_timeout(double timeout);
    void swap_buffers();
    void set_vsync(int interval);
    // 把本窗口的 GL 上下文设为当前（多窗口各自独立上下文：绘制前先 make_current）
    void make_current();

    int width() const { return width_; }
    int height() const { return height_; }
    GLFWwindow* handle() const { return window_; }

    // 最近创建窗口的句柄（剪贴板等系统级 API 用；无窗口时返回 nullptr）
    static GLFWwindow* primary_handle();

    // 边沿触发：本轮 poll 内发生过窗口尺寸变化（读一次后清除）
    bool resized();

    // ---- 窗口管理 ----
    void set_title(const char* title);
    void set_icon(int count, const GLFWimage* images);
    void set_position(int xpos, int ypos);
    void get_position(int& xpos, int& ypos) const;
    // 在主显示器工作区内居中（避免无边框窗口默认落在屏幕角落 0,0）
    void center_on_primary_monitor();
    void set_size(int width, int height);
    void get_size(int& width, int& height) const;
    void set_size_limits(int minw, int minh, int maxw, int maxh);
    void set_aspect_ratio(int numer, int denom);
    void get_framebuffer_size(int& width, int& height) const;
    void get_frame_size(int& left, int& top, int& right, int& bottom) const;
    float get_content_scale() const;
    float get_opacity() const;
    void set_opacity(float opacity);
    void iconify();
    void restore();
    void maximize();
    void show();
    void hide();
    void focus();
    void request_attention();
    bool is_visible() const;
    bool is_iconified() const;
    bool is_maximized() const;
    bool is_focused() const;
    bool is_resizable() const;
    bool is_decorated() const;
    void set_should_close(bool value);
    void set_resizable(bool value);
    void set_decorated(bool value);
    void set_auto_iconify(bool value);
    void set_floating(bool value);

    // ---- 拖拽（无边框窗口）----
    void start_drag(float mouse_x, float mouse_y);
    void update_drag(float mouse_x, float mouse_y);
    void end_drag();
    bool is_dragging() const { return dragging_; }

    // ---- 光标形状 ----
    void set_cursor(CursorType type);  // 统一光标接口
    void set_cursor_arrow();
    void set_cursor_hand();
    void set_cursor_resize_ns();
    void set_cursor_resize_ew();
    void set_cursor_ibeam();

    // ---- 剪贴板 ----
    void set_clipboard_string(const char* string);
    const char* get_clipboard_string() const;

    // ---- 时间 ----
    static double get_time();
    static void set_time(double time);

    // ---- 显示器 ----
    static GLFWmonitor* get_primary_monitor();
    static GLFWmonitor** get_monitors(int& count);
    static void get_monitor_position(GLFWmonitor* monitor, int& xpos, int& ypos);
    static void get_monitor_workarea(GLFWmonitor* monitor, int& xpos, int& ypos, int& width, int& height);
    static void get_monitor_content_scale(GLFWmonitor* monitor, float& xscale, float& yscale);
    static const char* get_monitor_name(GLFWmonitor* monitor);
    static void get_video_modes(GLFWmonitor* monitor, int& count, void*& modes);
    static void get_video_mode(GLFWmonitor* monitor, void*& mode);

    // ---- 输入（像素坐标，y 向下，(0,0)=左上角）----
    float mouse_x() const { return mouse_x_; }
    float mouse_y() const { return mouse_y_; }
    // button: 0=左键 1=右键 2=中键
    bool mouse_down(int button) const;
    // 边沿触发：本轮 poll 内新按下/新抬起（读一次后清除）
    bool mouse_pressed(int button);
    bool mouse_released(int button);
    // 边沿触发(peek)：本轮 poll 内新按下/新抬起，但不清除标志。
    // 供 Scene 在 Frame 订阅者之前快照同一边沿（避免最先读者的消费吞掉后续分发者的边沿，
    // 见 scene.cpp 鼠标分发）——「生产者-消费方」与「分配者-控件树」可用非消耗读各取所需。
    bool mouse_pressed_peek(int button) const;
    bool mouse_released_peek(int button) const;
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

    // ---- 文件拖放 ----
    int drop_count() const { return static_cast<int>(drop_paths_.size()); }
    const std::string& drop_path(int i) const { return drop_paths_[static_cast<size_t>(i)]; }
    void clear_drops() { drop_paths_.clear(); }

private:
    static void cursor_pos_cb(GLFWwindow* w, double x, double y);
    static void mouse_button_cb(GLFWwindow* w, int button, int action, int mods);
    static void scroll_cb(GLFWwindow* w, double x, double y);
    static void resize_cb(GLFWwindow* w, int width, int height);
    static void key_cb(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void char_cb(GLFWwindow* w, unsigned int codepoint);
    static void drop_cb(GLFWwindow* w, int count, const char** paths);

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
    std::vector<std::string> drop_paths_;
    bool dragging_ = false;
    float drag_off_x_ = 0.f;   // 非 Windows 回退用
    float drag_off_y_ = 0.f;
    int drag_screen_x_ = 0;    // Win32 绝对屏幕坐标（防振荡）
    int drag_screen_y_ = 0;
    int start_wx_ = 0;
    int start_wy_ = 0;
};

}  // namespace ui
