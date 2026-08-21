#pragma once

#include "ui/core/math.h"

namespace ui {

// 全局主题（M1：控件样式集中管理，M10：预设工厂 + 运行期切换）
struct Theme {
    Vec4 window_bg{0.12f, 0.13f, 0.15f, 1.0f};

    Vec4 button_normal{0.22f, 0.23f, 0.26f, 1.0f};
    Vec4 button_hover{0.28f, 0.29f, 0.33f, 1.0f};
    Vec4 button_pressed{0.16f, 0.17f, 0.20f, 1.0f};

    // 输入类控件（TextBox/TextArea）：内容底 + 边框
    Vec4 input_bg{0.16f, 0.17f, 0.19f, 1.0f};
    Vec4 input_border{0.35f, 0.36f, 0.40f, 1.0f};

    Vec4 accent{0.13f, 0.58f, 0.95f, 1.0f};   // 主色（Bootstrap primary）
    Vec4 accent_hover{0.10f, 0.49f, 0.84f, 1.0f};
    Vec4 accent_pressed{0.08f, 0.40f, 0.72f, 1.0f};

    Vec4 text{0.92f, 0.93f, 0.95f, 1.0f};
    Vec4 text_dim{0.62f, 0.64f, 0.68f, 1.0f};

    float radius = 6.f;  // 圆角半径（像素）

    // ---- layui 克隆（M17a）：语义色 / 边框 / 面板 / 次级文字 ----
    Vec4 success{0.09f, 0.72f, 0.47f, 1.0f};    // #16b777 表单焦点/成功
    Vec4 info{0.12f, 0.62f, 1.0f, 1.0f};        // #1e9fff normal 蓝
    Vec4 warning{1.0f, 0.72f, 0.0f, 1.0f};      // #ffb800 warm 黄
    Vec4 danger{1.0f, 0.34f, 0.13f, 1.0f};      // #ff5722 danger 红
    Vec4 border{0.93f, 0.93f, 0.93f, 1.0f};     // #eee 边框
    Vec4 panel{1.0f, 1.0f, 1.0f, 1.0f};         // 面板/卡片白底
    Vec4 text_secondary{0.37f, 0.37f, 0.37f, 1.0f};  // #5f5f5f 次级文字
    Vec4 text_muted{0.6f, 0.6f, 0.6f, 1.0f};    // #999 弱化文字

    // 预设（M10）：新增主题只需在此加一个工厂函数，控件无需改动
    static Theme make_dark();
    static Theme make_light();
    static Theme make_layui();
};

}  // namespace ui
