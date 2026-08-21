#pragma once

#include "ui/core/math.h"

namespace ui {

class Widget;

// layui 样式解析层（M17a）：类名 -> 样式令牌，克隆自 layui.css 提取值（硬编码，不解析 CSS 文件）。
// 用法：控件构造后 add_class("layui-progress layui-progress-big")，绘制时查询本层取样式。
namespace layui {

// 工具色：bg-red/orange/green/cyan/blue/purple/black/gray -> 前景色（#fff 文字）
Vec4 bg_color(const char* cls);
// 工具色背景底（layui-bg-* 的 background-color 值）
Vec4 bg_fill(const char* cls);

// 十六进制颜色串 "#16baaa" / "16baaa" -> Vec4
Vec4 hex(const char* hex);

// 组件便捷查询：widget 是否带某类
bool has_class(const Widget* w, const char* cls);

}  // namespace layui
}  // namespace ui
