#include "../page_registry.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/code.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"

#include <chrono>
#include <cmath>

namespace {

// 自绘动画圆形（克隆 layui anim.html）：点击重放；loop 型持续旋转。
// 渲染器无变换支持，动画用插值手工实现（UI-033）。
class AnimCircle : public ui::Widget {
public:
    enum Kind {
        SlideDown, SlideDownBit, SlideUp, SlideUpBit,
        Scale, ScaleSpring, ScaleSmall, ScaleSmallSpring,
        FadeIn, FadeOut, Rotate, RotateLoop,
    };

    AnimCircle(float size, Kind kind, const char* text)
        : ui::Widget(0, 0, size, size), kind_(kind), text_(text ? text : "") {}

    void set_font(ui::Font* f) { font_ = f; }

    bool on_mouse_down(float x, float y, int button) override {
        if (button != 0) return false;
        if (contains(x, y)) {
            start_ = std::chrono::steady_clock::now();
            started_ = true;
            return true;
        }
        return false;
    }

    void on_draw(ui::Renderer& renderer) override {
        if (!font_) return;
        if (!started_) {
            start_ = std::chrono::steady_clock::now();
            started_ = true;
        }
        const float elapsed = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - start_).count();
        const float dur = 0.65f;
        const float t = std::min(1.0f, elapsed / dur);

        const float cx = bounds_.x + bounds_.w * 0.5f;
        const float cy = bounds_.y + bounds_.h * 0.5f;
        const float R = std::min(bounds_.w, bounds_.h) * 0.5f;

        float ox = 0.f, oy = 0.f;      // 位移
        float k = 1.f;                 // 半径缩放
        float alpha = 1.f;             // 透明度
        float angle = 0.f;             // 旋转角
        const float ease = t * t * (3.f - 2.f * t);

        switch (kind_) {
            case SlideDown:
            case SlideDownBit: {
                const float dist = (kind_ == SlideDown ? 1.5f : 0.45f) * R;
                oy = dist * (1.f - ease);
                break;
            }
            case SlideUp:
            case SlideUpBit: {
                const float dist = (kind_ == SlideUp ? 1.5f : 0.45f) * R;
                oy = -dist * (1.f - ease);
                break;
            }
            case Scale: k = ease; break;
            case ScaleSpring: k = std::max(0.f, 1.f - std::exp(-4.5f * t) * std::cos(6.f * t)); break;
            case ScaleSmall: k = 1.f - ease; break;
            case ScaleSmallSpring: k = std::exp(-4.5f * t) * std::cos(6.f * t); break;
            case FadeIn: alpha = ease; break;
            case FadeOut: alpha = 1.f - ease; break;
            case Rotate: angle = t * 2.f * 3.14159265f; break;
            case RotateLoop: {
                const float period = 2.f;
                const float lt = std::fmod(elapsed, period) / period;
                angle = lt * 2.f * 3.14159265f;
                break;
            }
        }

        const float rr = R * k;
        const ui::Vec4 bg{0.0f, 0.59f, 0.53f, alpha};
        if (rr > 0.5f) {
            ui::Rect rc{bounds_.x + ox, bounds_.y + oy, bounds_.w, bounds_.h};
            // 以中心缩放：调整矩形使半径 rr
            rc.x = cx + ox - rr;
            rc.y = cy + oy - rr;
            rc.w = 2.f * rr;
            rc.h = 2.f * rr;
            renderer.draw_rounded_rect(rc, rr, bg);
            if (alpha > 0.05f) {
                const float tw = font_->measure_text(text_.c_str());
                const float lh = font_->line_height();
                font_->draw_text(renderer, text_.c_str(),
                                 rc.x + (rc.w - tw) * 0.5f, rc.y + (rc.h - lh) * 0.5f,
                                 ui::Vec4{1, 1, 1, alpha});
            }
        }
        if (kind_ == Rotate || kind_ == RotateLoop) {
            // 旋转可见性：圆周上的小标记点（无变换支持的旋转演示）
            const float mx = cx + (R - 14.f) * std::cos(angle);
            const float my = cy + (R - 14.f) * std::sin(angle);
            renderer.draw_rounded_rect({mx - 6.f, my - 6.f, 12.f, 12.f}, 6.f, {1, 1, 1, 1});
        }
    }

private:
    Kind kind_;
    std::string text_;
    ui::Font* font_ = nullptr;
    std::chrono::steady_clock::time_point start_{};
    bool started_ = false;
};

struct AnimDef {
    const char* text;
    const char* name;
    AnimCircle::Kind kind;
};

const AnimDef kAnims[] = {
    {"顶部往下滑入", "layui-anim-down", AnimCircle::SlideDown},
    {"微微往下滑入", "layui-anim-downbit", AnimCircle::SlideDownBit},
    {"底部往上滑入", "layui-anim-up", AnimCircle::SlideUp},
    {"微微往上滑入", "layui-anim-upbit", AnimCircle::SlideUpBit},
    {"平滑放大", "layui-anim-scale", AnimCircle::Scale},
    {"弹簧式放大", "layui-anim-scaleSpring", AnimCircle::ScaleSpring},
    {"平滑放小", "layui-anim-scalesmall", AnimCircle::ScaleSmall},
    {"弹簧式放小", "layui-anim-scalesmall-spring", AnimCircle::ScaleSmallSpring},
    {"渐现", "layui-anim-fadein", AnimCircle::FadeIn},
    {"渐隐", "layui-anim-fadeout", AnimCircle::FadeOut},
    {"360度旋转", "layui-anim-rotate", AnimCircle::Rotate},
    {"循环动画", "追加：layui-anim-loop", AnimCircle::RotateLoop},
};

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

void build_anim(PageHost& host) {
    add_section(host, "动画过程演示（点击圆形重放动画）");
    constexpr float kCircle = 96.f;
    constexpr int kPerRow = 3;
    for (int i = 0; i < (int)(sizeof(kAnims) / sizeof(kAnims[0])); i += kPerRow) {
        ui::HBox* row = new ui::HBox(0, 0, 640, kCircle + 30.f);
        row->set_spacing(16.f);
        row->set_padding(4.f);
        for (int j = 0; j < kPerRow && i + j < (int)(sizeof(kAnims) / sizeof(kAnims[0])); ++j) {
            const AnimDef& d = kAnims[i + j];
            ui::VBox* cell = new ui::VBox(0, 0, 190, kCircle + 30.f);
            cell->set_padding(2.f);
            cell->set_spacing(6.f);
            AnimCircle* circle = new AnimCircle(kCircle, d.kind, d.text);
            circle->set_font(host.font);
            cell->add_child(circle);
            ui::Label* name = new ui::Label(0, 0, 190, 0, d.name);
            name->set_font(host.font);
            name->set_theme(host.theme);
            name->set_color(host.theme->text_muted);
            name->set_align(1);
            cell->add_child(name);
            row->add_child(cell);
        }
        host.content->add_child(row);
    }

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/anim_page.cpp（节选）：克隆 layui anim.html\n"
            "\n"
            "// 自绘 AnimCircle：点击重放动画，on_draw 用 std::chrono 计算进度并插值\n"
            "AnimCircle* circle = new AnimCircle(96, AnimCircle::SlideDown, \"顶部往下滑入\");\n"
            "// 动画种类：down/up 滑入、scale 缩放(可弹簧)、fadein/out 渐显隐、rotate 旋转(可循环)\n"
            "circle->set_font(host.font);\n"
            "host.content->add_child(circle);");
    }
    if (host.help) {
        const char* lines[] = {
            "动画（AnimCircle 自绘部件）",
            "  12 种 layui 动画：上/下滑入、平滑/弹簧缩放、渐现渐隐、360° 旋转、循环",
            "  点击圆形重放动画；渲染器无变换支持，动画由 on_draw 插值实现",
            "  loop 型（layui-anim-loop）持续旋转，无需点击",
        };
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            label->set_font(host.font);
            label->set_theme(host.theme);
            label->set_color(host.theme->text);
            host.help->add_child(label);
        }
    }
}

}  // namespace

static void build(PageHost& host) {
    build_anim(host);
}

static const PageDef kAnimPage{12, "通用", "动画", "12 种内置动画：点击重放", build, nullptr, false, nullptr, 0, "Anim"};

static const bool kRegistered = []() {
    page::register_page(kAnimPage);
    return true;
}();
