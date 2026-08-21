#include "ui/ui/theme.h"

namespace ui {

Theme Theme::make_dark() {
    Theme t;
    t.window_bg = {0.12f, 0.13f, 0.15f, 1.0f};
    t.button_normal = {0.22f, 0.23f, 0.26f, 1.0f};
    t.button_hover = {0.28f, 0.29f, 0.33f, 1.0f};
    t.button_pressed = {0.16f, 0.17f, 0.20f, 1.0f};
    t.accent = {0.13f, 0.58f, 0.95f, 1.0f};
    t.accent_hover = {0.10f, 0.49f, 0.84f, 1.0f};
    t.accent_pressed = {0.08f, 0.40f, 0.72f, 1.0f};
    t.text = {0.92f, 0.93f, 0.95f, 1.0f};
    t.text_dim = {0.62f, 0.64f, 0.68f, 1.0f};
    t.radius = 6.f;
    return t;
}

Theme Theme::make_light() {
    Theme t;
    t.window_bg = {0.93f, 0.94f, 0.96f, 1.0f};
    t.button_normal = {0.88f, 0.90f, 0.93f, 1.0f};
    t.button_hover = {0.81f, 0.84f, 0.89f, 1.0f};
    t.button_pressed = {0.72f, 0.76f, 0.82f, 1.0f};
    t.input_bg = {1.0f, 1.0f, 1.0f, 1.0f};
    t.input_border = {0.91f, 0.91f, 0.91f, 1.0f};
    t.accent = {0.12f, 0.42f, 0.72f, 1.0f};
    t.accent_hover = {0.09f, 0.36f, 0.63f, 1.0f};
    t.accent_pressed = {0.06f, 0.30f, 0.53f, 1.0f};
    t.text = {0.15f, 0.16f, 0.18f, 1.0f};
    t.text_dim = {0.44f, 0.46f, 0.50f, 1.0f};
    t.radius = 6.f;
    return t;
}

Theme Theme::make_layui() {
    Theme t;
    t.window_bg = {0.95f, 0.95f, 0.95f, 1.0f};  // 浅灰底
    t.button_normal = {1.0f, 1.0f, 1.0f, 1.0f};  // 白：控件/面板底（HBox/VBox/TextBox 等）
    t.button_hover = {0.06f, 0.63f, 0.58f, 1.0f};
    t.button_pressed = {0.04f, 0.53f, 0.48f, 1.0f};
    t.input_bg = {1.0f, 1.0f, 1.0f, 1.0f};
    t.input_border = {0.90f, 0.90f, 0.90f, 1.0f};
    t.accent = {0.09f, 0.73f, 0.67f, 1.0f};   // 主色 #16baaa
    t.accent_hover = {0.06f, 0.63f, 0.58f, 1.0f};
    t.accent_pressed = {0.04f, 0.53f, 0.48f, 1.0f};
    t.text = {0.15f, 0.16f, 0.18f, 1.0f};      // 正文近黑
    t.text_dim = {0.6f, 0.6f, 0.6f, 1.0f};     // #999 弱化
    t.radius = 2.f;                            // layui 圆角 2px
    t.success = {0.09f, 0.72f, 0.47f, 1.0f};   // #16b777
    t.info = {0.12f, 0.62f, 1.0f, 1.0f};       // #1e9fff
    t.warning = {1.0f, 0.72f, 0.0f, 1.0f};     // #ffb800
    t.danger = {1.0f, 0.34f, 0.13f, 1.0f};     // #ff5722
    t.border = {0.93f, 0.93f, 0.93f, 1.0f};    // #eee
    t.panel = {1.0f, 1.0f, 1.0f, 1.0f};
    t.text_secondary = {0.37f, 0.37f, 0.37f, 1.0f};  // #5f5f5f
    t.text_muted = {0.6f, 0.6f, 0.6f, 1.0f};         // #999
    return t;
}

}  // namespace ui
