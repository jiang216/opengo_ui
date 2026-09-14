#pragma once

#include "ui/ui/widget.h"

namespace ui {
class Window;

// 输入法候选窗跟随（UI-100/UI-202）：
// opengl_ui 为 GLFW 自绘窗口，光标是绘制的竖线而非 Win32 CARET，
// 输入法组合窗/候选窗默认落在窗口左上角或屏幕角落，无法跟随输入框。
// 由应用在每帧渲染循环中调用一次：root 子树内聚焦控件为 TextBox 或 TextArea 时，
// 将 IME 组合窗/候选窗锚定到其光标处（CFS_POINT/CFS_CANDIDATEPOS，
// ptCurrentPos 为窗口客户区坐标）。非 Windows 平台为空操作。
void update_ime_caret(const Window& window, Widget* root);

}  // namespace ui
