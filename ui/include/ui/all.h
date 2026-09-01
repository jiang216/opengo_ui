#pragma once

// 聚合头（umbrella header）：一条 #include 拉入 core / gl / ui 三层全部公开头文件。
//
// 用法：
//   #include "ui/all.h"
//
// 说明：
//   - 覆盖 core（math/log/version/keycodes）、gl（window/renderer/texture/font）、
//     ui/ui（Widget 基类 + 全部 layui 控件）三层公开 API；
//   - media（ui/media/ffmpeg VideoView、ui/media/opencv ImageView）为可选模块（默认不编译），
//     不包含在本聚合头内；启用后按需 #include "ui/media/video_view.h" 等；
//   - 全部 API 清单 / 调用示例 / 依赖说明见 docs/UI_API.md（由 tools/api_gen.py 生成）。

// ---- ui_core（纯 C++ 基础，无第三方依赖）----
#include "ui/core/keycodes.h"
#include "ui/core/log.h"
#include "ui/core/math.h"
#include "ui/core/version.h"

// ---- ui_gl（GLFW 窗口 + 2D 渲染器 + 纹理 + 字体）----
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"
#include "ui/gl/window.h"

// ---- ui/ui 组件（Widget 基类 + 主题 + 样式解析 + 图标常量 + 全部控件）----
#include "ui/ui/widget.h"
#include "ui/ui/theme.h"
#include "ui/ui/ime.h"
#include "ui/ui/scene.h"
#include "ui/ui/layui_style.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/badge.h"
#include "ui/ui/blockquote.h"
#include "ui/ui/breadcrumb.h"
#include "ui/ui/button.h"
#include "ui/ui/button_group.h"
#include "ui/ui/card.h"
#include "ui/ui/carousel.h"
#include "ui/ui/checkbox.h"
#include "ui/ui/circle.h"
#include "ui/ui/code.h"
#include "ui/ui/collapse.h"
#include "ui/ui/colorpicker.h"
#include "ui/ui/combobox.h"
#include "ui/ui/countdown_text.h"
#include "ui/ui/dropdown.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/grid.h"
#include "ui/ui/hbox.h"
#include "ui/ui/hline.h"
#include "ui/ui/icon.h"
#include "ui/ui/iconlabel.h"
#include "ui/ui/image.h"
#include "ui/ui/label.h"
#include "ui/ui/laydate.h"
#include "ui/ui/laypage.h"
#include "ui/ui/menubar.h"
#include "ui/ui/navmenu.h"
#include "ui/ui/panel.h"
#include "ui/ui/panelwindow.h"
#include "ui/ui/progress.h"
#include "ui/ui/radiobutton.h"
#include "ui/ui/radiogroup.h"
#include "ui/ui/rate.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/slider.h"
#include "ui/ui/switch.h"
#include "ui/ui/table.h"
#include "ui/ui/tabs.h"
#include "ui/ui/textarea.h"
#include "ui/ui/textbox.h"
#include "ui/ui/timeline.h"
#include "ui/ui/titlebar.h"
#include "ui/ui/transfer.h"
#include "ui/ui/tree.h"
#include "ui/ui/upload.h"
#include "ui/ui/vbox.h"
