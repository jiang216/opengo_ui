#include "ui/gl/cursor_icon_manager.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace ui {

CursorIconManager::CursorIconManager() {
    build_ew_base();
    rotate_and_cache(0.f, 0);    // EW (↔)
    rotate_and_cache(90.f, 1);   // NS (↕)
    rotate_and_cache(45.f, 2);   // NWSE (↘)
    rotate_and_cache(135.f, 3);  // NESW (↙)
}

CursorIconManager::~CursorIconManager() {
    for (int i = 0; i < 4; ++i) {
        if (cursors_[i]) glfwDestroyCursor(cursors_[i]);
    }
}

GLFWcursor* CursorIconManager::get_cursor(int direction) const {
    if (direction >= 0 && direction < 4) return cursors_[direction];
    return nullptr;
}

// ---- 基础 EW (↔) 光标绘制（48×48） ----

void CursorIconManager::set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (x < 0 || x >= kBaseSize || y < 0 || y >= kBaseSize) return;
    uint8_t* p = base_ + (y * kBaseSize + x) * 4;
    p[0] = r; p[1] = g; p[2] = b; p[3] = a;
}

void CursorIconManager::fill_rect(int x0, int y0, int x1, int y1,
                                   uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);
    x0 = std::max(0, std::min(x0, kBaseSize - 1));
    x1 = std::max(0, std::min(x1, kBaseSize - 1));
    y0 = std::max(0, std::min(y0, kBaseSize - 1));
    y1 = std::max(0, std::min(y1, kBaseSize - 1));
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            uint8_t* p = base_ + (y * kBaseSize + x) * 4;
            p[0] = r; p[1] = g; p[2] = b; p[3] = a;
        }
    }
}

void CursorIconManager::draw_line(int x0, int y0, int x1, int y1,
                                   uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        set_pixel(x0, y0, r, g, b, a);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void CursorIconManager::fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2,
                                       uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // 找 bounding box
    int minX = std::min({x0, x1, x2});
    int maxX = std::max({x0, x1, x2});
    int minY = std::min({y0, y1, y2});
    int maxY = std::max({y0, y1, y2});

    minX = std::max(0, minX);
    maxX = std::min(kBaseSize - 1, maxX);
    minY = std::max(0, minY);
    maxY = std::min(kBaseSize - 1, maxY);

    // 扫描线填充
    for (int y = minY; y <= maxY; ++y) {
        int crossCount = 0;
        int xIntersections[2] = {0, 0};
        int idx = 0;

        // 检查三条边
        int ex0 = x0, ey0 = y0, ex1 = x1, ey1 = y1;
        for (int edge = 0; edge < 3; ++edge) {
            if (edge == 0) { ex0 = x0; ey0 = y0; ex1 = x1; ey1 = y1; }
            else if (edge == 1) { ex0 = x1; ey0 = y1; ex1 = x2; ey1 = y2; }
            else { ex0 = x2; ey0 = y2; ex1 = x0; ey1 = y0; }

            if ((ey0 <= y && ey1 > y) || (ey1 <= y && ey0 > y)) {
                float t = (float)(y - ey0) / (float)(ey1 - ey0);
                int ix = (int)(ex0 + t * (ex1 - ex0));
                if (idx < 2) xIntersections[idx++] = ix;
            }
        }

        if (idx == 2) {
            if (xIntersections[0] > xIntersections[1])
                std::swap(xIntersections[0], xIntersections[1]);
            for (int x = xIntersections[0]; x <= xIntersections[1]; ++x) {
                set_pixel(x, y, r, g, b, a);
            }
        }
    }
}

void CursorIconManager::build_ew_base() {
    std::memset(base_, 0, sizeof(base_));

    const int cx = kBaseSize / 2;  // 24
    const int cy = kBaseSize / 2;  // 24

    // 水平粗线（3px 高）：从 x=8 到 x=40
    fill_rect(8, cy - 1, 40, cy + 1, 255, 255, 255, 255);

    // 左箭头（指向左的三角形）
    // 顶点：(7, cy), (16, cy-6), (16, cy+6)
    fill_triangle(7, cy, 16, cy - 6, 16, cy + 6, 255, 255, 255, 255);

    // 右箭头（指向右的三角形）
    // 顶点：(40, cy), (31, cy-6), (31, cy+6)
    fill_triangle(40, cy, 31, cy - 6, 31, cy + 6, 255, 255, 255, 255);

    // 添加黑色描边：遍历所有白色像素，检查 4 邻域是否有透明像素
    // 如果有，将透明像素设为黑色
    uint8_t outlined[kBaseSize * kBaseSize * 4];
    std::memcpy(outlined, base_, sizeof(outlined));

    for (int y = 0; y < kBaseSize; ++y) {
        for (int x = 0; x < kBaseSize; ++x) {
            uint8_t* p = base_ + (y * kBaseSize + x) * 4;
            if (p[3] == 255) {  // 白色像素
                // 检查 4 邻域
                const int dx4[] = {-1, 1, 0, 0};
                const int dy4[] = {0, 0, -1, 1};
                for (int d = 0; d < 4; ++d) {
                    int nx = x + dx4[d];
                    int ny = y + dy4[d];
                    if (nx >= 0 && nx < kBaseSize && ny >= 0 && ny < kBaseSize) {
                        uint8_t* np = base_ + (ny * kBaseSize + nx) * 4;
                        if (np[3] == 0) {  // 透明邻居
                            outlined[(ny * kBaseSize + nx) * 4 + 0] = 0;
                            outlined[(ny * kBaseSize + nx) * 4 + 1] = 0;
                            outlined[(ny * kBaseSize + nx) * 4 + 2] = 0;
                            outlined[(ny * kBaseSize + nx) * 4 + 3] = 255;
                        }
                    }
                }
            }
        }
    }
    std::memcpy(base_, outlined, sizeof(outlined));
}

// ---- 旋转并缓存 ----

void CursorIconManager::rotate_and_cache(float angle_degrees, int index) {
    float angle = angle_degrees * (float)M_PI / 180.0f;
    float cos_a = std::cos(angle);
    float sin_a = std::sin(angle);

    float src_cx = kBaseSize / 2.0f;
    float src_cy = kBaseSize / 2.0f;
    float dst_cx = kCursorSize / 2.0f;
    float dst_cy = kCursorSize / 2.0f;

    // 对于 0°/90°/180°/270°，从 48x48 中心裁剪 32x32
    // 对于 45°/135°，旋转后裁剪中心 32x32
    uint8_t pixels[kCursorSize * kCursorSize * 4];
    std::memset(pixels, 0, sizeof(pixels));

    if (angle_degrees == 0.f) {
        // 0°：直接从中心裁剪
        int offset = (kBaseSize - kCursorSize) / 2;
        for (int y = 0; y < kCursorSize; ++y) {
            for (int x = 0; x < kCursorSize; ++x) {
                const uint8_t* src = base_ + ((y + offset) * kBaseSize + (x + offset)) * 4;
                uint8_t* dst = pixels + (y * kCursorSize + x) * 4;
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
            }
        }
    } else if (angle_degrees == 90.f) {
        // 90°：从 48x48 中心裁剪 32x32，然后旋转
        int offset = (kBaseSize - kCursorSize) / 2;
        for (int y = 0; y < kCursorSize; ++y) {
            for (int x = 0; x < kCursorSize; ++x) {
                // 90° 旋转：(x,y) -> (kCursorSize-1-y, x)
                int rx = kCursorSize - 1 - y;
                int ry = x;
                const uint8_t* src = base_ + ((ry + offset) * kBaseSize + (rx + offset)) * 4;
                uint8_t* dst = pixels + (y * kCursorSize + x) * 4;
                dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
            }
        }
    } else {
        // 45°/135°：旋转 48x48，然后裁剪中心 32x32
        for (int y = 0; y < kCursorSize; ++y) {
            for (int x = 0; x < kCursorSize; ++x) {
                float dx = x - dst_cx;
                float dy = y - dst_cy;
                // 逆旋转找到源坐标
                float src_x = cos_a * dx + sin_a * dy + src_cx;
                float src_y = -sin_a * dx + cos_a * dy + src_cy;

                int sx = (int)std::round(src_x);
                int sy = (int)std::round(src_y);

                if (sx >= 0 && sx < kBaseSize && sy >= 0 && sy < kBaseSize) {
                    const uint8_t* src = base_ + (sy * kBaseSize + sx) * 4;
                    uint8_t* dst = pixels + (y * kCursorSize + x) * 4;
                    dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
                }
            }
        }
    }

    GLFWimage img = {kCursorSize, kCursorSize, pixels};
    cursors_[index] = glfwCreateCursor(&img, kCursorSize / 2, kCursorSize / 2);
}

}  // namespace ui
