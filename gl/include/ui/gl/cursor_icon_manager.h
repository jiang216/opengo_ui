#pragma once

#include <cstdint>

struct GLFWcursor;

namespace ui {

// 光标类型枚举
enum class CursorType {
    Arrow, Hand, IBeam,
    ResizeEW, ResizeNS, ResizeNWSE, ResizeNESW
};

// 光标图标管理器：统一管理光标的创建、缓存和切换
// 只绘制一个基础 EW (↔) 图标，旋转生成全部 4 个 resize 方向
class CursorIconManager {
public:
    CursorIconManager();
    ~CursorIconManager();

    // 获取指定方向的光标（0=EW, 1=NS, 2=NWSE, 3=NESW）
    GLFWcursor* get_cursor(int direction) const;

private:
    // 绘制 48×48 基础 EW (↔) 光标
    void build_ew_base();

    // 旋转像素缓冲区（最近邻插值，保持像素锐利）
    // src: 48×48 RGBA, dst: 32×32 RGBA
    void rotate_and_cache(float angle_degrees, int index);

    // 设置单个像素（带边界检查）
    void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    // 绘制填充三角形（用于箭头）
    void fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    // 绘制线段（Bresenham）
    void draw_line(int x0, int y0, int x1, int y1,
                   uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    // 绘制填充矩形
    void fill_rect(int x0, int y0, int x1, int y1,
                   uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    static constexpr int kBaseSize = 48;   // 基础光标尺寸
    static constexpr int kCursorSize = 16;  // 最终光标尺寸

    uint8_t base_[kBaseSize * kBaseSize * 4] = {};  // 基础像素数据（RGBA）
    GLFWcursor* cursors_[4] = {nullptr};            // 4 个方向的缓存光标
};

}  // namespace ui
