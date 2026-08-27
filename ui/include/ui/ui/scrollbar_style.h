#pragma once

// ui 库垂直滚动条共享样式/状态（TextArea / ScrollBox 统一实现）。
// 内部协作头：不进 all.h、不对外承诺 ABI；应用代码请勿直接依赖。
//
// 视觉规格（UI-115 基础 + UI-116 overlay 风格化）：
//   - 常态宽 6px，悬停 thumb / 拖拽中加宽至 8px；右缘留白 4px；胶囊圆角（半宽）
//   - 自动隐藏：控件未悬停、未拖拽、且超过 2s 无滚动活动时不绘制（overlay 风格）
//   - thumb 三态色：默认 text_dim@55% → 悬停 text_dim 全色 → 拖拽 accent
//   - track 懒显示：仅活动期绘制 button_pressed@40%（默认不画轨道底色）

#include "ui/core/math.h"
#include "ui/gl/renderer.h"
#include "ui/ui/theme.h"

#include <algorithm>
#include <chrono>

namespace ui {
namespace scrollbar {

constexpr float kBarW = 6.f;          // 常态宽
constexpr float kBarWHover = 8.f;     // 悬停 thumb / 拖拽中宽
constexpr float kMargin = 6.f;        // 距控件右缘留白
constexpr float kHitPad = 4.f;        // 命中容差（横向外扩）
constexpr float kThumbMinH = 28.f;    // thumb 最小高度
constexpr double kAutoHideDelay = 2.0;// 无活动自动隐藏延时（秒）

// 每控件一份的交互状态
struct State {
    bool dragging = false;       // 正在拖拽 thumb
    double last_active = -1e30;  // 上次滚动/命中时刻（steady_clock 秒）
};

namespace detail {
inline double now_s() {
    return std::chrono::duration<double>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}
}  // namespace detail

inline void touch(State& s) { s.last_active = detail::now_s(); }

// 是否处于活动期（绘制与命中的总开关）：悬停 / 拖拽 / 近期滚过
inline bool active(const State& s, bool widget_hovered) {
    return widget_hovered || s.dragging ||
           (detail::now_s() - s.last_active) < kAutoHideDelay;
}

// 当前条宽：拖拽或悬停 thumb 列时加宽
inline float bar_w(const State& s, bool over_bar) {
    return (s.dragging || over_bar) ? kBarWHover : kBarW;
}

// 轨道几何（vis_h 为控件可视高度）
inline Rect track_rect(const Rect& b, float w) {
    return {b.x + b.w - kMargin - w, b.y + 4.f, w, std::max(1.f, b.h - 8.f)};
}

// thumb 几何（content_h 内容总高；scroll 当前偏移）
inline Rect thumb_rect(const Rect& track, float content_h, float vis_h, float scroll) {
    if (content_h <= 0.f) return track;
    const float frac = vis_h / content_h;
    const float th = std::max(kThumbMinH, track.h * frac);
    const float max_off = std::max(0.f, content_h - vis_h);
    const float t = max_off > 0.f ? scroll / max_off : 0.f;
    return {track.x, track.y + (track.h - th) * t, track.w, th};
}

// 命中测试（横向外扩 kHitPad 容差）
inline bool hit(const Rect& track, float x, float y) {
    return x >= track.x - kHitPad && x <= track.x + track.w + kHitPad &&
           y >= track.y && y <= track.y + track.h;
}

// 点击轨道直接跳转：y → scroll 偏移
inline float scroll_from_y(const Rect& track, float y, float max_off) {
    if (track.h <= 0.f) return 0.f;
    const float t = std::min(std::max((y - track.y) / track.h, 0.f), 1.f);
    return t * max_off;
}

// 绘制轨道 + thumb（仅活动期调用；颜色三态见文件头注释）
inline void draw(Renderer& r, const Theme* t, const Rect& track, const Rect& thumb,
                 const State& s, bool over_bar) {
    const Vec4 track_c{t->button_pressed.r, t->button_pressed.g, t->button_pressed.b, 0.40f};
    Vec4 thumb_c = s.dragging ? t->accent
                              : Vec4{t->text_dim.r, t->text_dim.g, t->text_dim.b,
                                     over_bar ? 1.0f : 0.55f};
    r.draw_rounded_rect(track, track.w * 0.5f, track_c);
    r.draw_rounded_rect(thumb, thumb.w * 0.5f, thumb_c);
}

}  // namespace scrollbar
}  // namespace ui
