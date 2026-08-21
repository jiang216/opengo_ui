// edit_test：TextBox/TextArea 输入链路冒烟（UI-101/102/103 回归）
// 验证：字符输入、控制字符过滤、on_enter、Ctrl+A/C/X/V、默认剪贴板兜底
#include <ui/all.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <string>

using namespace ui;

int main() {
    Window win(400, 300, "edit_test");
    if (!win.handle()) {
        std::printf("FAIL: window create\n");
        return 1;
    }
    Widget root(0, 0, 400, 300);
    int fails = 0;
    auto check = [&](bool ok, const char* name) {
        std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", name);
        if (!ok) ++fails;
    };

    // ---- TextBox ----
    TextBox tb(10, 10, 200, 30);
    tb.set_font(nullptr);  // 输入/回车/剪贴板链路不依赖字体
    root.add_child(&tb);
    int enters = 0;
    tb.set_on_enter([&] { ++enters; });

    tb.focus();
    root.dispatch_char('a');
    root.dispatch_char(0x16);  // Ctrl+V 组合产生的 WM_CHAR 控制字符应被过滤
    check(tb.text() == "a", "textbox char input + ctrl-char filtered");

    root.dispatch_key_down(KeyEnter, 0);
    check(enters == 1, "textbox on_enter fired");

    root.dispatch_key_down(KeyA, ModControl);
    check(tb.has_selection(), "textbox ctrl+a select all");
    root.dispatch_key_down(KeyC, ModControl);
    const char* clip = glfwGetClipboardString(win.handle());
    check(clip && std::string(clip) == "a", "textbox ctrl+c -> clipboard");
    root.dispatch_key_down(KeyX, ModControl);
    check(tb.text().empty(), "textbox ctrl+x cut");
    glfwSetClipboardString(win.handle(), "xyz");
    root.dispatch_key_down(KeyV, ModControl);
    check(tb.text() == "xyz", "textbox ctrl+v paste");

    // ---- TextArea（未注入回调：内置剪贴板兜底 UI-103）----
    Font font;
    // on_key_down 依赖字体（行高/换行计算），加载系统字体
    if (!font.load_from_file("C:/Windows/Fonts/arial.ttf", 16.f) ||
        !font.load_cjk_font("C:/Windows/Fonts/msyh.ttc")) {
        std::printf("FAIL: font load\n");
        return 1;
    }
    TextArea ta(10, 60, 200, 80);
    ta.set_font(&font);
    root.add_child(&ta);
    ta.focus();  // 自动清 TextBox 焦点
    root.dispatch_char(0x4F60);  // 你
    root.dispatch_char(0x597D);  // 好
    check(ta.text() == "\xE4\xBD\xA0\xE5\xA5\xBD", "textarea cjk input");
    root.dispatch_key_down(KeyA, ModControl);
    check(ta.has_selection(), "textarea ctrl+a");
    root.dispatch_key_down(KeyC, ModControl);
    clip = glfwGetClipboardString(win.handle());
    check(clip && std::string(clip) == "\xE4\xBD\xA0\xE5\xA5\xBD",
          "textarea ctrl+c default clipboard");
    glfwSetClipboardString(win.handle(), "xyz");
    root.dispatch_key_down(KeyV, ModControl);
    check(ta.text() == "xyz", "textarea ctrl+v replace selection");

    if (fails)
        std::printf("== %d FAILURES ==\n", fails);
    else
        std::printf("== ALL PASS ==\n");
    return fails ? 1 : 0;
}
