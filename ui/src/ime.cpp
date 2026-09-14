#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX  // 防 Windows min/max 宏污染 std::max（textarea.h→scrollbar_style.h）
#endif
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <windows.h>
#include <imm.h>
#endif

#include "ui/ui/ime.h"

#include "ui/gl/window.h"
#include "ui/ui/textarea.h"
#include "ui/ui/textbox.h"

#include <algorithm>

namespace ui {
namespace {

// 递归找子树内聚焦控件（Widget::focused_descendant 为 private，自行遍历）
Widget* find_focused(Widget* w) {
    if (!w || !w->visible) return nullptr;
    if (w->focused()) return w;
    for (auto* c : w->children()) {
        if (auto* r = find_focused(c)) return r;
    }
    return nullptr;
}

}  // namespace

void update_ime_caret(const Window& window, Widget* root) {
#ifdef _WIN32
    // 记录上次设置的屏幕坐标：未变化不重复设置（避免打断用户拖动候选窗）；
    // 窗口移动后屏幕坐标变化会自动跟随
    static POINT last = {-1, -1};

    if (!root) return;
    Widget* focused = find_focused(root);
    if (!focused) return;

    POINT pt = {};
    if (auto* box = dynamic_cast<TextBox*>(focused)) {
        // 单行 TextBox：候选窗锚到输入框底边光标处（UI-100）
        const auto& b = box->bounds();
        pt = {static_cast<LONG>(b.x + box->caret_offset()),
              static_cast<LONG>(b.y + b.h)};
    } else if (auto* ta = dynamic_cast<TextArea*>(focused)) {
        // 多行 TextArea：候选窗锚到光标所在行底部（UI-202）
        const auto& b = ta->bounds();
        float ax = 0.f, ay = 0.f;
        ta->caret_anchor(ax, ay);
        pt = {static_cast<LONG>(b.x + ax), static_cast<LONG>(b.y + ay)};
    } else {
        return;
    }

    HWND hwnd = glfwGetWin32Window(window.handle());
    if (!hwnd) return;
    if (pt.x == last.x && pt.y == last.y) return;
    last = pt;

    HIMC himc = ImmGetContext(hwnd);
    if (!himc) return;
    COMPOSITIONFORM cf{};
    cf.dwStyle = CFS_POINT;
    cf.ptCurrentPos = pt;
    ImmSetCompositionWindow(himc, &cf);
    CANDIDATEFORM cand{};
    cand.dwIndex = 0;
    cand.dwStyle = CFS_CANDIDATEPOS;
    cand.ptCurrentPos = pt;
    ImmSetCandidateWindow(himc, &cand);
    ImmReleaseContext(hwnd, himc);
#else
    (void)window;
    (void)root;
#endif
}

}  // namespace ui
