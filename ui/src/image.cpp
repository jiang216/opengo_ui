#include "ui/ui/image.h"

#include <cstdio>
#include <string>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "ui/core/log.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"

namespace ui {

namespace {
bool file_readable(const char* path) {
    FILE* f = std::fopen(path, "rb");
    if (f) {
        std::fclose(f);
        return true;
    }
    return false;
}

// 可执行文件所在目录（Windows）；非 Win32 或无路径时返回空串。
std::string exe_directory() {
#if defined(_WIN32)
    wchar_t buf[MAX_PATH];
    const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, buf, (int)n, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string p((size_t)len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, buf, (int)n, &p[0], len, nullptr, nullptr);
    const size_t slash = p.find_last_of("/\\");
    if (slash == std::string::npos) return {};
    return p.substr(0, slash);
#else
    return {};
#endif
}

bool is_bare_filename(const char* path) {
    for (const char* p = path; *p; ++p)
        if (*p == '/' || *p == '\\') return false;
    return true;
}
}  // namespace

Image::Image(float x, float y, float w, float h) : Widget(x, y, w, h) {
    display_w_ = w;
    display_h_ = h;
    has_display_size_ = (w > 0.f && h > 0.f);
}

Image::~Image() {
    // texture_ 由 std::unique_ptr 自动释放（阶段2 生命周期优化，消除手动 delete）
}

bool Image::load_from_file(const char* path) {
    mark_dirty();
        if (!path || !*path) return false;

    // 资产定位（ui 库能力）：裸文件名先试 CWD，再回退 exe 目录同名文件，
    // 使资源不依赖进程工作目录（配合 POST_BUILD 拷贝三件套即开箱可用）。
    // 先做存在性预检，避免对不存在路径逐条触发 gl 层 [error] 噪音（UI-048）。
    std::string exe_cand;
    if (is_bare_filename(path)) {
        const std::string d = exe_directory();
        if (!d.empty()) exe_cand = d + "/" + path;
    }
    const char* targets[2] = {path, exe_cand.c_str()};
    for (int i = 0; i < 2; ++i) {
        const char* t = targets[i];
        if (!t || !*t) continue;
        if (!file_readable(t)) continue;
        auto tex = std::make_unique<Texture>();
        if (!tex->load_from_file(t)) {  // 解码失败由 gl 层 log
            return false;
        }
        texture_ = std::move(tex);
        loaded_ = true;
        // 未显式指定显示尺寸时，跟随图片原始尺寸
        if (display_w_ <= 0.f || display_h_ <= 0.f) {
            display_w_ = (float)texture_->width();
            display_h_ = (float)texture_->height();
        }
        has_display_size_ = (display_w_ > 0.f && display_h_ > 0.f);
        return true;
    }
    log_error("image: cannot open %s (cwd / exe dir)", path);
    return false;
}

void Image::set_lay_src(const char* path) {
    mark_dirty();
        if (!path || !*path) return;
    lay_src_ = path;
    loaded_ = false;  // 尚未进入可视屏，不加载
    texture_.reset();  // 独占指针归还，替换旧纹理（阶段2）
}

float Image::preferred_height() const {
    // 懒加载/显式定尺寸：加载前也预留显示高度，保证信息流列表布局稳定不塌陷
    if (loaded_) return display_h_;
    if (has_display_size_) return display_h_;
    return 0.f;
}

void Image::on_draw(Renderer& renderer) {
    // 懒加载（lay-src）：仅在自身进入当前可视屏时才真正解码，且只加载一次。
    // current_scissor() 即「当前屏」矩形（ScrollBox 等裁剪容器已 push_scissor）；
    // 无论滚动条上滑还是下滑，都只加载当前屏的图片（layui flow.lazyimg 语义）。
    if (!loaded_ && !lay_src_.empty()) {
        const Rect vis = renderer.current_scissor();
        const Rect b = bounds_;
        if (b.x + b.w >= vis.x && b.x <= vis.x + vis.w && b.y + b.h >= vis.y && b.y <= vis.y + vis.h) {
            load_from_file(lay_src_.c_str());
        }
    }
    if (!loaded_ || !texture_) return;
    renderer.draw_textured_quad(bounds_, texture_->id(), Rect{0.f, 0.f, 1.f, 1.f},
                                Vec4{1, 1, 1, 1});
}

}  // namespace ui
