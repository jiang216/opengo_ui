#pragma once

#include "ui/core/math.h"

#include <functional>
#include <vector>

namespace ui {

// 2D 渲染器（GL 3.3 Core）：像素坐标 y 向下，(0,0) 为左上角
class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    // 必须在 GL 上下文就绪后调用一次
    bool init();

    void begin_frame(const Vec4& clear_color);
    void set_viewport(int width, int height);

    // 纯色矩形（radius > 0 时为圆角，fwidth SDF 抗锯齿）。
    // 契约（防 UI-037 回归）：radius=0（draw_quad）矩形内部必须全不透明；
    // 颜色管线 SDF 若缺内部距离项，内部会以 50% alpha 混合（全界面灰蒙蒙）。
    void draw_quad(const Rect& rect, const Vec4& color) { draw_rounded_rect(rect, 0.f, color); }
    void draw_rounded_rect(const Rect& rect, float radius, const Vec4& color);

    // 矩形描边（厚度为 t 的边框，4 条薄 quad 拼合，无圆角）
    void draw_rect_outline(const Rect& rect, float thickness, const Vec4& color);

    // 圆角矩形描边：厚度为 thickness 的圆角环，半径/圆角随 radius 自动（UI-053）
    void draw_rounded_rect_outline(const Rect& rect, float radius, float thickness,
                                   const Vec4& color);

    // 纹理四边形：uv = {u0, v0, u1, v1}（0..1，v 为纹理上边→下边）；radius>0 时 SDF 圆角裁剪；
    // rotation 为绕矩形中心旋转角（弧度，0 不旋转），用于图标/旋转动画（如时间线 loading）
    void draw_textured_quad(const Rect& rect, unsigned int texture_id, const Rect& uv,
                            const Vec4& color, float radius = 0.f, float rotation = 0.f);

    // 填充多边形（凸多边形三角扇，首顶点为扇心；像素坐标 y 向下）。顶点按顺时针或逆时针均可
    void draw_polygon(const Vec2* points, int count, const Vec4& color);

    // 裁剪：此后绘制被限制在 rect 内（可嵌套，pop 恢复上一个）。像素坐标 y 向下
    void push_scissor(const Rect& rect);
    void pop_scissor();

    // 当前视口尺寸（由 set_viewport 记录，供 UI 层使用）
    int viewport_width() const { return viewport_width_; }
    int viewport_height() const { return viewport_height_; }

    // 当前裁剪区（scissor 栈顶）：即当前「可视屏」矩形（像素坐标 y 向下）。
    // 未 push_scissor 时返回整个视口。供 LazyImage 判断「是否在当前屏内」以触发懒加载（lay-src）。
    Rect current_scissor() const;

private:
    bool setup_unit_quad();
    bool create_color_pipeline();
    bool create_texture_pipeline();
    bool create_polygon_pipeline();

    // 颜色管线圆角矩形/描边共用的绘制体（outline=0 即填充）
    void draw_rounded_rect_impl(const Rect& rect, float radius, float outline,
                                const Vec4& color);

    // init() 后执行一次的回归自检（UI-037）：向离屏 FBO 绘制一个 radius=0 的 4x4 白色矩形，
    // 读回中心像素 alpha，非全不透明则 log_error（发布/CI 前拦截 SDF 回归）。
    void self_test_quad_opacity();

    // init() 后执行一次的回归自检（UI-054）：向离屏 FBO 绘制一个圆角环，
    // 读回中心像素 alpha，非近透明则 log_error（拦截圆角环内部被描边色盖满回归）。
    void self_test_ring_opacity();

    // 上述自检共用：离屏 32x32 FBO 执行 draw，读回中心像素（返回 false 表示 FBO 未就绪）
    bool run_offscreen_check(const std::function<void()>& draw, unsigned char px[4]);

    // 应用一个像素坐标（y 向下）裁剪矩形到 GL scissor（原点左下角，y 需翻转）。
    // push/pop 共用，避免重复 y-flip 逻辑（性能/维护优化）。
    void apply_scissor_gl(const Rect& r);

    unsigned int color_program_ = 0;
    unsigned int tex_program_ = 0;
    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;
    // 纯色管线 uniform 位置
    unsigned int u_proj_ = 0, u_rect_ = 0, u_color_ = 0, u_radius_ = 0, u_size_ = 0, u_outline_ = 0;
    unsigned int u_cosrot_ = 0, u_sinrot_ = 0;
    // 纹理管线 uniform 位置
    unsigned int t_proj_ = 0, t_rect_ = 0, t_color_ = 0, t_uv_ = 0, t_tex_ = 0;
    unsigned int t_size_ = 0, t_radius_ = 0;
    unsigned int t_cosrot_ = 0, t_sinrot_ = 0;
    // 多边形管线（动态 VBO，每帧上传顶点）
    unsigned int poly_program_ = 0;
    unsigned int poly_vao_ = 0;
    unsigned int poly_vbo_ = 0;
    unsigned int p_proj_ = 0, p_color_ = 0;
    int viewport_width_ = 0;
    int viewport_height_ = 0;
    // 正交投影矩阵：只依赖视口尺寸，由 set_viewport 缓存，避免每个 draw call 重算/上传（性能优化）
    float proj_[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
                       0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
    // 最近一次 glUseProgram 的 program（去重，避免每 draw call 频繁切换 program）
    unsigned int active_program_ = 0;
    // scissor 栈：push 与栈顶求交后压栈，pop 恢复栈顶矩形（嵌套裁剪，UI-057）
    std::vector<Rect> scissor_stack_;
};

}  // namespace ui
