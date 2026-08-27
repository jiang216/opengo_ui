#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
class Texture;
struct Theme;

// 轮播动画类型（layui anim）
enum class CarouselAnim { Default, Updown, Fade };
// 箭头显示状态（layui arrow）：hover=悬停显示，always=始终显示，none=不显示
enum class CarouselArrow { Hover, Always, None };
// 指示器位置（layui indicator）：inside=容器内部，outside=容器外部（下移一行），none=不显示
enum class CarouselIndicator { Inside, Outside, None };

// 轮播（M17d，layui-carousel 克隆）：多页内容循环切换，左右箭头 + 指示点，
// 支持 anim 切换动画（左右/上下/渐隐）、arrow 箭头状态、indicator 指示器位置、
// 自动轮播（interval 毫秒）、图片或纯色底+标题页、运行时 reload 重配尺寸/参数（UI-089）。
// 自动轮播/动画计数由 on_advance(dt) 驱动（应用层每帧 root->advance(dt)）。
class Carousel : public Widget {
public:
    Carousel(float x, float y, float w, float h);
    ~Carousel() override;

    // 添加一页：背景色 + 标题
    void add_slide(const Vec4& bg, const char* title);
    // 添加图片页（layui 填充图片）：加载路径内的图片作为该页内容
    bool add_image_slide(const char* path);
    void clear();
    int count() const { return (int)slides_.size(); }
    int current() const { return current_; }
    void set_index(int idx);

    void set_font(Font* font) {  font_ = font; mark_dirty(); }
    void set_theme(const Theme* theme) {  theme_ = theme; mark_dirty(); }
    void set_on_change(std::function<void(int)> cb) { on_change_ = std::move(cb); }

    // layui 配置项
    void set_anim(CarouselAnim anim) {  anim_ = anim; mark_dirty(); }
    CarouselAnim anim() const { return anim_; }
    void set_arrow(CarouselArrow arrow) {  arrow_ = arrow; mark_dirty(); }
    CarouselArrow arrow() const { return arrow_; }
    void set_indicator(CarouselIndicator ind) {  indicator_ = ind; mark_dirty(); }
    CarouselIndicator indicator() const { return indicator_; }
    void set_autoplay(bool on) {  autoplay_ = on; mark_dirty(); }
    bool autoplay() const { return autoplay_; }
    // interval：毫秒（layui interval，默认 3000）
    void set_interval(float ms) {  interval_ = ms > 0.f ? ms : 3000.f; mark_dirty(); }
    float interval() const { return interval_; }  // 毫秒
    void set_size(float w, float h) {  set_bounds(bounds_.x, bounds_.y, w, h); mark_dirty(); }

    // 旧 API 兼容（UI-089 前）：秒为单位开关自动轮播
    void set_auto_play(bool on, float seconds = 3.f) { 
        autoplay_ = on;
        if (seconds > 0.f) interval_ = seconds * 1000.f; mark_dirty(); }

    void next();
    void prev();

    // 旧 API 兼容：应用层每帧调用（自动轮播/动画计时）
    void advance(float dt);

    void on_advance(float dt) override;
    void on_layout() override;
    bool on_mouse_move(float x, float y) override;
    bool on_mouse_down(float x, float y, int button) override;
    void on_draw(Renderer& renderer) override;

private:
    struct Slide {
        Vec4 bg{0.9f, 0.9f, 0.9f, 1.0f};
        std::string title;
        Texture* image = nullptr;  // 图片页（可选）
    };
    void set_current_now(int idx);
    void start_transition(int from, int to);

    std::vector<Slide> slides_;
    int current_ = 0;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(int)> on_change_;
    CarouselAnim anim_ = CarouselAnim::Default;
    CarouselArrow arrow_ = CarouselArrow::Hover;
    CarouselIndicator indicator_ = CarouselIndicator::Inside;
    bool autoplay_ = false;
    float interval_ = 3000.f;  // 毫秒
    float timer_ = 0.f;        // autoplay 累计（秒）
    // 切换过渡（anim 动画）：from→to 进度 t（0..1），t<0 表示无过渡
    int trans_from_ = 0;
    int trans_to_ = 0;
    float trans_t_ = -1.f;
};

}  // namespace ui