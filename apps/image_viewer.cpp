// 图像查看器示例应用（M16 收官）：打开图片 / 滚轮缩放 / 拖拽平移 / 自适应窗口。
// 用法：opengl_ui_image_viewer [图片路径]
//  - 打开：按钮或 Ctrl+O（Win32 文件对话框，宽字符路径，支持中文）
//  - 缩放：滚轮（以鼠标为锚点）；双击复位到适配窗口
//  - 平移：左键拖拽
//  - --smoke <秒>：运行指定秒后自动退出（CI/冒烟）

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include "ui/core/log.h"
#include "ui/core/version.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"
#include "ui/gl/window.h"
#include "ui/ui/button.h"
#include "ui/ui/label.h"
#include "ui/ui/scene.h"
#include "ui/ui/window_container.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

std::string utf8_from_wchar(const std::wstring& w) {
    if (w.empty()) return {};
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s((size_t)n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}

// 宽字符路径读字节 -> OpenCV imdecode（绕开 imread 窄路径的 ANSI 编码问题）
bool load_image_wide(const std::wstring& wpath, ui::Texture& tex, std::string* out_name) {
    HANDLE h = CreateFileW(wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    DWORD sz = GetFileSize(h, nullptr);
    std::vector<unsigned char> data(sz);
    DWORD got = 0;
    BOOL ok = ReadFile(h, data.data(), sz, &got, nullptr);
    CloseHandle(h);
    if (!ok || got != sz) return false;

    cv::Mat img = cv::imdecode(data, cv::IMREAD_UNCHANGED);
    if (img.empty()) return false;

    cv::Mat rgba;
    switch (img.channels()) {
        case 4: cv::cvtColor(img, rgba, cv::COLOR_BGRA2RGBA); break;
        case 3: cv::cvtColor(img, rgba, cv::COLOR_BGR2RGBA); break;
        case 1: cv::cvtColor(img, rgba, cv::COLOR_GRAY2RGBA); break;
        default: rgba = img; break;
    }
    if (!rgba.isContinuous()) rgba = rgba.clone();
    tex.create(rgba.cols, rgba.rows, rgba.data);
    if (out_name) *out_name = utf8_from_wchar(wpath);
    ui::log_info("image_viewer: loaded %s (%dx%d)", out_name ? out_name->c_str() : "?",
                 rgba.cols, rgba.rows);
    return true;
}

// Win32 打开文件对话框（宽字符）
bool open_file_dialog(std::wstring& out) {
    wchar_t buf[MAX_PATH] = {0};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"Images\0*.png;*.jpg;*.jpeg;*.bmp;*.webp;*.gif;*.tif;*.tiff;*.exr\0All\0*.*\0";
    ofn.lpstrFile = buf;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (!GetOpenFileNameW(&ofn)) return false;
    out = buf;
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    double smoke_seconds = 0.0;
    std::wstring open_path;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--smoke") == 0 && i + 1 < argc) smoke_seconds = std::atof(argv[++i]);
        else if (argv[i][0] != '-' && open_path.empty())
            open_path = std::wstring(argv[i], argv[i] + std::strlen(argv[i]));
    }
    ui::log_info("image_viewer %s", ui::version_string());

    // 带自绘标题栏的窗口容器（UI-168）：居中 + 可拖动/最小化/最大化/关闭
    ui::WindowContainer container(960, 720, "image viewer (M16)");
    if (!container.ok()) return 1;
    ui::Window& window = container.window();
    ui::Renderer& renderer = container.renderer();
    ui::Scene& scene = container.scene();

    ui::Font font;
    font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
    font.load_cjk_font_auto();
    container.titlebar().set_font(&font);

    // ui::Scene：统一驱动生命周期；image_viewer 迁移前无标准键/字符分发，仅鼠标+滚轮(Ctrl+O 走 Key 钩子)
    scene.set_root_background(ui::Vec4{0.16f, 0.17f, 0.19f, 1.f});
    scene.dispatch_nav_keys = false;
    scene.dispatch_text = false;
    scene.dispatch_scroll = false;  // 滚轮仅用于缩放，不外发到树

    ui::Widget& root = scene.root();

    // 内容区起始 y（= 标题条高）：顶部工具条下移让出标题条（UI-168）
    const float top = container.content_origin_y();
    // 顶部工具条：打开按钮 + 状态文本
    ui::Button open_btn(8, top + 8, 90, 30, "Open...");
    open_btn.set_font(&font);
    root.add_child(&open_btn);
    ui::Label status(110, top + 10, 840, 24, "Ctrl+O: open | wheel: zoom | drag: pan | double-click: fit");
    status.set_font(&font);
    root.add_child(&status);

    ui::Texture tex;
    bool has_image = false;
    std::string file_name;
    float zoom = 1.f;
    float pan_x = 0.f, pan_y = 0.f;
    bool dragging = false;
    float drag_x0 = 0.f, drag_y0 = 0.f;
    float pan_x0 = 0.f, pan_y0 = 0.f;
    double last_click = 0.0;

    auto open_by_wpath = [&](const std::wstring& w) {
        if (load_image_wide(w, tex, &file_name)) {
            has_image = true;
            zoom = 1.f;
            pan_x = pan_y = 0.f;
        } else {
            ui::log_error("image_viewer: cannot load %s", utf8_from_wchar(w).c_str());
        }
    };
    open_btn.set_on_click([&]() {
        std::wstring w;
        if (open_file_dialog(w)) open_by_wpath(w);
    });

    if (!open_path.empty()) open_by_wpath(open_path);

    // ---- Scene 生命周期订阅 ----
    // 帧前：滚轮缩放 / 拖拽平移 / Ctrl+O 与状态更新（读 window 原始状态，与旧主循环一致）
    scene.subscribe(ui::ScenePhase::Frame, [&](ui::Scene&, float) {
        // 滚轮缩放（以鼠标为锚点）
        const float sdy = window.scroll_dy();
        if (has_image && sdy != 0.f) {
            const float mx = window.mouse_x(), my = window.mouse_y();
            const float cx = (mx - pan_x) / zoom, cy = (my - pan_y) / zoom;
            zoom = std::clamp(zoom * (sdy > 0 ? 1.15f : 1.f / 1.15f), 0.05f, 32.f);
            pan_x = mx - cx * zoom;
            pan_y = my - cy * zoom;
        }
        // 拖拽平移
        if (window.mouse_pressed(0) && has_image) {
            dragging = true;
            drag_x0 = window.mouse_x();
            drag_y0 = window.mouse_y();
            pan_x0 = pan_x;
            pan_y0 = pan_y;
        }
        if (window.mouse_released(0)) {
            const double now = std::chrono::duration<double>(
                                   std::chrono::steady_clock::now().time_since_epoch())
                                   .count();
            if (dragging && now - last_click < 0.3 && std::abs(window.mouse_x() - drag_x0) < 6 &&
                std::abs(window.mouse_y() - drag_y0) < 6) {
                // 双击复位
                zoom = 1.f;
                pan_x = pan_y = 0.f;
            }
            dragging = false;
            last_click = now;
        }
        if (dragging) {
            pan_x = pan_x0 + (window.mouse_x() - drag_x0);
            pan_y = pan_y0 + (window.mouse_y() - drag_y0);
        }
    });

    // Ctrl+O 打开（非标准键集 → Key 事件钩子，未消费返回 false 不阻塞其它）
    scene.set_event_hook(ui::SceneEvent::Key, [&](ui::Scene&) {
        if (window.key_pressed(ui::KeyO) && window.key_down(ui::KeyLeftControl)) {
            std::wstring w;
            if (open_file_dialog(w)) open_by_wpath(w);
        }
        return false;
    });

    // 帧后：绘制图像 + 状态文本
    scene.on(ui::ScenePhase::PostDraw, [&](ui::Scene&) {
        if (has_image && tex.width() > 0 && tex.height() > 0) {
            const float iw = (float)tex.width(), ih = (float)tex.height();
            const float view_x = 0.f, view_y = top + 40.f, view_w = (float)window.width(),
                        view_h = (float)window.height() - (top + 40.f);
            const float base_scale = std::min(view_w / iw, view_h / ih);
            const float disp_w = iw * base_scale * zoom;
            const float disp_h = ih * base_scale * zoom;
            const float cx = view_x + view_w / 2.f + pan_x;
            const float cy = view_y + view_h / 2.f + pan_y;
            const ui::Rect r{cx - disp_w / 2.f, cy - disp_h / 2.f, disp_w, disp_h};
            renderer.push_scissor(ui::Rect{view_x, view_y, view_w, view_h});
            renderer.draw_textured_quad(r, tex.id(), ui::Rect{0, 0, 1, 1}, ui::Vec4{1, 1, 1, 1});
            renderer.pop_scissor();

            char sbuf[256];
            std::snprintf(sbuf, sizeof(sbuf), "%s  %dx%d  zoom %.0f%%", file_name.c_str(),
                          tex.width(), tex.height(), zoom * 100.f);
            status.set_text(sbuf);
        } else {
            status.set_text("Ctrl+O: open | wheel: zoom | drag: pan | double-click: fit");
        }
    });

    // 运行：--smoke 指定秒自动退出，否则直至窗口关闭
    if (smoke_seconds > 0.0) {
        scene.run_for(smoke_seconds);
    } else {
        scene.run();
    }
    return 0;
}
