#include "ui/gl/renderer.h"

#include "ui/core/log.h"
#include <glad/gl.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>

namespace ui {
namespace {

// 共享顶点着色器：aPos(0..1) -> 局部像素坐标 vLocal、按 uUv 子矩形变换的纹理坐标 vUv
const char* kVertexShader = R"GLSL(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 uProj;
uniform vec4 uRect;   // x, y, w, h（像素）
uniform vec4 uUv;     // u0, v0, u1, v1（纹理管线使用）
uniform float uCosRot;   // cos(旋转角)，CPU 预算（不逐顶点算 trig）
uniform float uSinRot;   // sin(旋转角)
out vec2 vLocal;      // 相对矩形左上角，像素
out vec2 vUv;         // 子矩形内 0..1
void main() {
    vec2 pos = aPos * uRect.zw + uRect.xy;
    vec2 center = uRect.xy + uRect.zw * 0.5;
    vec2 p = pos - center;
    pos = vec2(p.x * uCosRot - p.y * uSinRot, p.x * uSinRot + p.y * uCosRot) + center;
    vLocal = aPos * uRect.zw;
    vUv = uUv.xy + aPos * (uUv.zw - uUv.xy);
    gl_Position = uProj * vec4(pos, 0.0, 1.0);
}
)GLSL";

// 颜色管线片元着色器：圆角矩形 SDF，fwidth 做 1px 抗锯齿；uOutline>0 时输出圆角描边环
const char* kColorFragmentShader = R"GLSL(
#version 330 core
in vec2 vLocal;
uniform vec4 uColor;
uniform vec2 uSize;   // w, h（像素）
uniform float uRadius;
uniform float uOutline;  // 0=填充；>0=描边厚度（像素，圆角环）
out vec4 fragColor;
void main() {
    vec2 half_size = uSize * 0.5;
    vec2 q = abs(vLocal - half_size) - half_size + uRadius;
    // 标准圆角矩形 SDF = 外部距离 + 内部（负）距离。
    // 禁止删除 min(max(q.x, q.y), 0.0) 内部项：radius=0（draw_quad）时内部 d 会变 0，
    // a=0.5 半透明，全界面直角矩形灰蒙蒙（UI-037）。init() 有 self_test_quad_opacity 回归自检。
    float d = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - uRadius;
    float aa = max(fwidth(d), 1e-4);
    float a;
    if (uOutline <= 0.0) {
        a = 1.0 - smoothstep(-aa, aa, d);
    } else {
        // 圆角环 = 外缘覆盖度 ∧ ¬内缘覆盖度；内缘是"挖掉"不是"再叠一层"，
        // 否则按钮内部（d 深负）a_out≈1 且 a_in≈1 导致整个内部被描边色盖满（UI-054）
        float a_out = 1.0 - smoothstep(-aa, aa, d);
        float a_in = 1.0 - smoothstep(-aa, aa, d + uOutline);
        a = a_out * (1.0 - a_in);
    }
    fragColor = vec4(uColor.rgb, uColor.a * a);
}
)GLSL";

// 纹理管线片元着色器：采样 uTex，与 uColor 调制；uRadius>0 时 SDF 圆角裁剪
const char* kTextureFragmentShader = R"GLSL(
#version 330 core
in vec2 vLocal;
in vec2 vUv;
uniform sampler2D uTex;
uniform vec4 uColor;
uniform vec2 uSize;
uniform float uRadius;
out vec4 fragColor;
void main() {
    vec4 tex = texture(uTex, vUv);
    if (uRadius <= 0.0) {
        fragColor = tex * uColor;
        return;
    }
    vec2 half_size = uSize * 0.5;
    vec2 q = abs(vLocal - half_size) - half_size + uRadius;
    float d = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - uRadius;
    float aa = max(fwidth(d), 1e-4);
    float a = 1.0 - smoothstep(-aa, aa, d);
    fragColor = tex * vec4(uColor.rgb, uColor.a * a);
}
)GLSL";

// 多边形顶点着色器：aPos 直接为像素坐标（y 向下）
const char* kPolygonVertexShader = R"GLSL(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 uProj;
void main() {
    gl_Position = uProj * vec4(aPos, 0.0, 1.0);
}
)GLSL";

// 多边形片元着色器：纯色填充
const char* kPolygonFragmentShader = R"GLSL(
#version 330 core
uniform vec4 uColor;
out vec4 fragColor;
void main() {
    fragColor = uColor;
}
)GLSL";

bool compile_shader(unsigned int type, const char* src, unsigned int* out) {
    unsigned int sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    int ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512] = {0};
        glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
        log_error("shader compile error: %s", log);
        glDeleteShader(sh);
        return false;
    }
    *out = sh;
    return true;
}

bool link_program(unsigned int vs, unsigned int fs, unsigned int* out) {
    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    int ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512] = {0};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        log_error("program link error: %s", log);
        glDeleteProgram(program);
        return false;
    }
    *out = program;
    return true;
}

}  // namespace

Renderer::~Renderer() {
    if (color_program_) glDeleteProgram(color_program_);
    if (tex_program_) glDeleteProgram(tex_program_);
    if (poly_program_) glDeleteProgram(poly_program_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (poly_vao_) glDeleteVertexArrays(1, &poly_vao_);
    if (poly_vbo_) glDeleteBuffers(1, &poly_vbo_);
}

bool Renderer::init() {
    if (!setup_unit_quad()) return false;
    if (!create_color_pipeline() || !create_texture_pipeline() || !create_polygon_pipeline()) return false;
    self_test_quad_opacity();
    self_test_ring_opacity();
    return true;
}

bool Renderer::setup_unit_quad() {
    // 单位矩形（两个三角形），坐标在 VS 中由 uRect 映射到像素
    const float quad[] = {
        0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
        1.f, 0.f, 0.f, 1.f, 1.f, 1.f,
    };
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    return true;
}

bool Renderer::create_color_pipeline() {
    unsigned int vs = 0, fs = 0;
    if (!compile_shader(GL_VERTEX_SHADER, kVertexShader, &vs)) return false;
    if (!compile_shader(GL_FRAGMENT_SHADER, kColorFragmentShader, &fs)) return false;
    if (!link_program(vs, fs, &color_program_)) return false;

    u_proj_ = glGetUniformLocation(color_program_, "uProj");
    u_color_ = glGetUniformLocation(color_program_, "uColor");
    u_rect_ = glGetUniformLocation(color_program_, "uRect");
    u_radius_ = glGetUniformLocation(color_program_, "uRadius");
    u_size_ = glGetUniformLocation(color_program_, "uSize");
    u_outline_ = glGetUniformLocation(color_program_, "uOutline");
    u_cosrot_ = glGetUniformLocation(color_program_, "uCosRot");
    u_sinrot_ = glGetUniformLocation(color_program_, "uSinRot");
    return true;
}

bool Renderer::create_texture_pipeline() {
    unsigned int vs = 0, fs = 0;
    if (!compile_shader(GL_VERTEX_SHADER, kVertexShader, &vs)) return false;
    if (!compile_shader(GL_FRAGMENT_SHADER, kTextureFragmentShader, &fs)) return false;
    if (!link_program(vs, fs, &tex_program_)) return false;

    t_proj_ = glGetUniformLocation(tex_program_, "uProj");
    t_rect_ = glGetUniformLocation(tex_program_, "uRect");
    t_color_ = glGetUniformLocation(tex_program_, "uColor");
    t_uv_ = glGetUniformLocation(tex_program_, "uUv");
    t_tex_ = glGetUniformLocation(tex_program_, "uTex");
    t_size_ = glGetUniformLocation(tex_program_, "uSize");
    t_radius_ = glGetUniformLocation(tex_program_, "uRadius");
    t_cosrot_ = glGetUniformLocation(tex_program_, "uCosRot");
    t_sinrot_ = glGetUniformLocation(tex_program_, "uSinRot");
    return true;
}

bool Renderer::create_polygon_pipeline() {
    unsigned int vs = 0, fs = 0;
    if (!compile_shader(GL_VERTEX_SHADER, kPolygonVertexShader, &vs)) return false;
    if (!compile_shader(GL_FRAGMENT_SHADER, kPolygonFragmentShader, &fs)) return false;
    if (!link_program(vs, fs, &poly_program_)) return false;

    p_proj_ = glGetUniformLocation(poly_program_, "uProj");
    p_color_ = glGetUniformLocation(poly_program_, "uColor");

    glGenVertexArrays(1, &poly_vao_);
    glBindVertexArray(poly_vao_);
    glGenBuffers(1, &poly_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, poly_vbo_);
    // 每帧上传顶点，动态绘制
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    return true;
}

void Renderer::begin_frame(const Vec4& clear_color) {
    // UI-169 系统性加固：每帧重置 scissor 状态。任何未被配平 pop 收尾的裁剪区
    // （退化矩形 push 空转后父级被误弹 / 异常提前返回跳过 pop 等）若残留到下一帧，
    // 会带着过期 GL_SCISSOR_TEST 状态绘制后续内容。此处清栈 + 关闭，杜绝跨帧脏状态。
    // Debug 前置断言：进入本帧时 scissor 栈应为空。若非空，说明上一帧存在未配平的
    // pop_scissor（或本帧在 begin_frame 之前就 push 过）——正是 UI-169 类缺陷的症状，Debug 下立即暴露。
    assert(scissor_stack_.empty() && "scissor stack not empty at begin_frame: unmatched push_scissor in previous frame/prologue");
    scissor_stack_.clear();
    glDisable(GL_SCISSOR_TEST);
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::set_viewport(int width, int height) {
    viewport_width_ = width;
    viewport_height_ = height;
    glViewport(0, 0, width, height);
    // 正交投影矩阵只依赖视口尺寸，这里一次性算好缓存，避免每个 draw call 重算/上传
    const float w = (float)width;
    const float h = (float)height;
    if (w <= 0.f || h <= 0.f) return;
    proj_[0] = 2.f / w;  proj_[1] = 0.f;      proj_[2] = 0.f;  proj_[3] = 0.f;
    proj_[4] = 0.f;      proj_[5] = -2.f / h; proj_[6] = 0.f;  proj_[7] = 0.f;
    proj_[8] = 0.f;      proj_[9] = 0.f;      proj_[10] = -1.f; proj_[11] = 0.f;
    proj_[12] = -1.f;    proj_[13] = 1.f;     proj_[14] = 0.f;  proj_[15] = 1.f;
}

namespace {
// 只在 program 变化时切换（去重 per-draw 状态切换），同时使能混合/关闭深度测试一次
void bind_color_blend(unsigned int program, unsigned int& active_program) {
    if (active_program != program) {
        glUseProgram(program);
        active_program = program;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
}
}  // namespace

Rect Renderer::current_scissor() const {
    if (!scissor_stack_.empty()) return scissor_stack_.back();
    return {0.f, 0.f, (float)viewport_width_, (float)viewport_height_};
}

void Renderer::draw_rounded_rect(const Rect& rect, float radius, const Vec4& color) {
    draw_rounded_rect_impl(rect, radius, 0.f, color);
}

void Renderer::draw_rounded_rect_outline(const Rect& rect, float radius, float thickness,
                                         const Vec4& color) {
    if (thickness <= 0.f) return;
    draw_rounded_rect_impl(rect, radius, thickness, color);
}

void Renderer::draw_rounded_rect_impl(const Rect& rect, float radius, float outline,
                                      const Vec4& color) {
    if (rect.w <= 0.f || rect.h <= 0.f) return;
    bind_color_blend(color_program_, active_program_);

    const float max_radius = std::min(rect.w, rect.h) * 0.5f;
    const float r = std::min(std::max(radius, 0.f), max_radius);

    glUniformMatrix4fv(u_proj_, 1, GL_FALSE, proj_);
    glUniform4f(u_rect_, rect.x, rect.y, rect.w, rect.h);
    glUniform4f(u_color_, color.r, color.g, color.b, color.a);
    glUniform1f(u_radius_, r);
    glUniform1f(u_outline_, outline);
    glUniform2f(u_size_, rect.w, rect.h);
    glUniform1f(u_cosrot_, 1.f);
    glUniform1f(u_sinrot_, 0.f);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// UI-037 / UI-054 回归自检共用：在离屏 32x32 FBO 上执行 draw，读回中心像素 (16,16)。
// 返回 false 表示 FBO 未就绪（不判失败）；view/FBO 状态自会恢复。
bool Renderer::run_offscreen_check(const std::function<void()>& draw, unsigned char px[4]) {
    const int saved_w = viewport_width_;
    const int saved_h = viewport_height_;

    GLuint fbo = 0, tex = 0;
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    bool ok = false;
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        set_viewport(32, 32);
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
        draw();
        glFinish();
        glReadPixels(16, 16, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
        set_viewport(saved_w, saved_h);
        ok = true;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteTextures(1, &tex);
    glDeleteFramebuffers(1, &fbo);
    return ok;
}

// UI-037 回归自检：draw_quad（radius=0）矩形内部必须全不透明。
// 颜色管线 SDF 若缺内部距离项 min(max(q.x,q.y),0)，radius=0 时内部 a=0.5，
// 全界面直角矩形 50% 半透明（"灰蒙蒙"）。init() 后执行一次拦截。
void Renderer::self_test_quad_opacity() {
    unsigned char px[4] = {0, 0, 0, 0};
    if (!run_offscreen_check([this] {
            draw_quad(Rect{0.f, 0.f, 32.f, 32.f}, Vec4{1.f, 1.f, 1.f, 1.f});
        }, px))
        return;
    if (px[3] < 250 || px[0] < 250) {
        log_error("renderer self-test FAILED: draw_quad(radius=0) interior not opaque "
                  "(rgba=%d,%d,%d,%d) — color-pipeline SDF missing interior distance term? (UI-037)",
                  px[0], px[1], px[2], px[3]);
    }
}

// UI-054 回归自检：圆角环内部必须镂空。若片元把内缘当"再叠一层"（a_out*a_in）而非
// "挖掉"（a_out*(1-a_in)），按钮内部会被描边色整块盖满。init() 后执行一次：画一个
// 32x32 圆角环，读回中心像素，非近透明即 log_error。
void Renderer::self_test_ring_opacity() {
    unsigned char px[4] = {0, 0, 0, 0};
    if (!run_offscreen_check([this] {
            draw_rounded_rect_outline(Rect{0.f, 0.f, 32.f, 32.f}, 8.f, 2.f,
                                      Vec4{1.f, 1.f, 1.f, 1.f});
        }, px))
        return;
    if (px[3] > 30) {
        log_error("renderer self-test FAILED: rounded-rect outline interior not hollow "
                  "(center alpha=%d) — ring must be outer∧not-inner (UI-054)", px[3]);
    }
}

void Renderer::draw_rect_outline(const Rect& rect, float thickness, const Vec4& color) {
    if (rect.w <= 0.f || rect.h <= 0.f) return;
    const float t = std::max(thickness, 0.f);
    // 上
    draw_quad(Rect{rect.x, rect.y, rect.w, t}, color);
    // 下
    draw_quad(Rect{rect.x, rect.y + rect.h - t, rect.w, t}, color);
    // 左
    draw_quad(Rect{rect.x, rect.y, t, rect.h}, color);
    // 右
    draw_quad(Rect{rect.x + rect.w - t, rect.y, t, rect.h}, color);
}

void Renderer::draw_textured_quad(const Rect& rect, unsigned int texture_id, const Rect& uv,
                                  const Vec4& color, float radius, float rotation) {
    if (rect.w <= 0.f || rect.h <= 0.f) return;
    bind_color_blend(tex_program_, active_program_);

    const float max_radius = std::min(rect.w, rect.h) * 0.5f;
    const float r = std::min(std::max(radius, 0.f), max_radius);
    // 旋转角 trig 在 CPU 预算一次（避免 per-vertex 逐字形算 cos/sin）
    const float cosr = std::cos(rotation);
    const float sinr = std::sin(rotation);
    // uv = {u0, v0, u1, v1}（用 Rect 的 x,y,w,h 承载）
    glUniformMatrix4fv(t_proj_, 1, GL_FALSE, proj_);
    glUniform4f(t_rect_, rect.x, rect.y, rect.w, rect.h);
    glUniform4f(t_color_, color.r, color.g, color.b, color.a);
    glUniform4f(t_uv_, uv.x, uv.y, uv.w, uv.h);
    glUniform2f(t_size_, rect.w, rect.h);
    glUniform1f(t_radius_, r);
    glUniform1f(t_cosrot_, cosr);
    glUniform1f(t_sinrot_, sinr);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glUniform1i(t_tex_, 0);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Renderer::push_scissor(const Rect& rect) {
    if (rect.w <= 0.f || rect.h <= 0.f) return;
    // 与当前裁剪区求交：嵌套时子裁剪区必须落在父裁剪区内，
    // 否则 pop 恢复父级后仍沿用子裁剪区矩形（UI-057）
    Rect effective = rect;
    if (!scissor_stack_.empty()) {
        const Rect& top = scissor_stack_.back();
        const float x0 = std::max(rect.x, top.x);
        const float y0 = std::max(rect.y, top.y);
        const float x1 = std::min(rect.x + rect.w, top.x + top.w);
        const float y1 = std::min(rect.y + rect.h, top.y + top.h);
        if (x1 <= x0 || y1 <= y0) {
            effective = Rect{0.f, 0.f, 0.f, 0.f};
        } else {
            effective = Rect{x0, y0, x1 - x0, y1 - y0};
        }
    }
    scissor_stack_.push_back(effective);
    if (scissor_stack_.size() == 1) glEnable(GL_SCISSOR_TEST);
    apply_scissor_gl(effective);
}

void Renderer::apply_scissor_gl(const Rect& r) {
    // GL scissor 原点是左下角，因此 y 需要先翻转
    const int y_gl = (int)((float)viewport_height_ - (r.y + r.h));
    glScissor((int)r.x, y_gl, (int)r.w, (int)r.h);
}

void Renderer::draw_polygon(const Vec2* points, int count, const Vec4& color) {
    if (!points || count < 3) return;
    bind_color_blend(poly_program_, active_program_);

    glUniformMatrix4fv(p_proj_, 1, GL_FALSE, proj_);
    glUniform4f(p_color_, color.r, color.g, color.b, color.a);

    glBindVertexArray(poly_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, poly_vbo_);
    glBufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(float), points, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, count);
    glBindVertexArray(0);
}

void Renderer::pop_scissor() {
    // UI-169：栈空时直接 return——任何 push 遇退化矩形(w/h<=0)不入栈的调用点，
    // 若配套 pop 仍执行会误弹**父级**裁剪区并过早关闭 GL_SCISSOR_TEST（滚动内容穿透
    // 标题栏/Header 的根因）。幂等空吞使 push/pop 配对在退化输入下安全。
    if (scissor_stack_.empty()) return;
    scissor_stack_.pop_back();
    if (scissor_stack_.empty()) {
        glDisable(GL_SCISSOR_TEST);
        return;
    }
    // 恢复上一级裁剪区矩形（嵌套裁剪的关键，UI-057）
    apply_scissor_gl(scissor_stack_.back());
}

}  // namespace ui
