#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <windows.h>
#include <imm.h>
#endif

#include "ui/ui/ime.h"

#include "ui/gl/window.h"
#include "ui/ui/textbox.h"

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
    auto* box = dynamic_cast<TextBox*>(find_focused(root));
    if (!box) return;

    // CFS_POINT / CFS_CANDIDATEPOS 的 ptCurrentPos 均为【客户区坐标】（MSDN），
    // 禁止 ClientToScreen 换算（换算后候选窗落窗口原点+屏幕坐标处，等效失效）。
    // y 取控件底边：候选窗弹在输入框下方而非顶部。
    const auto& b = box->bounds();
    POINT pt = {static_cast<LONG>(b.x + box->caret_offset()),
                static_cast<LONG>(b.y + b.h)};

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
