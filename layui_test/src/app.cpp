#include "page_registry.h"

#include "ui/core/log.h"
#include "ui/core/version.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/window.h"
#include "ui/ui/code.h"
#include "ui/ui/dropdown.h"
#include "ui/ui/image.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/menu.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/switch.h"
#include "ui/ui/tabs.h"
#include "ui/ui/theme.h"
#include "ui/ui/titlebar.h"
#include "ui/ui/widget.h"
#include "ui/ui/scene.h"
#include "ui/ui/window_container.h"
#include "ui/media/video_container.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <glad/gl.h>

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp")
#endif

namespace {

constexpr float kHeaderH = 60.f;
constexpr float kSidebarW = 240.f;  // 新镜像 docs/2 侧栏宽（ws-menu 240）
constexpr float kChildSidebarW = 160.f;  // 第二列子菜单宽（镜像 side-child 160）
constexpr float kTabTitleH = 40.f;

// --probe-video：遍历控件树，打印每个 VideoContainer 的布局/播放/控制条显隐状态。
// UI-197 验证用具：不承载 ui 库逻辑，纯「读取+打印」；控制条显隐 = 首个子控件 visible
// （update_controls_visibility 对所有控制条子控件整体 set_visible，任意一个可代表）。
void probe_video_containers(ui::Widget& w, int depth) {
    for (ui::Widget* child : w.children()) {
        if (!child) continue;
        if (auto* vc = dynamic_cast<ui::VideoContainer*>(child)) {
            const ui::Rect& b = vc->bounds();
            const bool controls_on =
                !child->children().empty() && child->children().front()->visible;
            ui::log_info("probe[%d]: VideoContainer bounds(%.0f,%.0f %.0fx%.0f) playing=%d "
                         "controls=%d has_video=%d pos=%.2f",
                         depth, b.x, b.y, b.w, b.h, vc->playing() ? 1 : 0, controls_on ? 1 : 0,
                         vc->has_video() ? 1 : 0, vc->position());
        }
        probe_video_containers(*child, depth + 1);
    }
}

// 顶部导航可点击文本（克隆新镜像 docs/2 深色头部导航项）：白字、hover/active 变绿
class ClickText : public ui::Widget {
public:
    explicit ClickText(const char* text) : ui::Widget(0, 0, 0, 0), text_(text ? text : "") {}
    void set_font(ui::Font* f) { font_ = f; }
    void set_theme(const ui::Theme* t) { theme_ = t; }
    void set_on_click(std::function<void()> cb) { on_click_ = std::move(cb); }
    void set_active(bool on) { active_ = on; }

    float preferred_width() const { return font_ ? font_->measure_text(text_.c_str()) : 0.f; }

    bool on_mouse_up(float x, float y, int button) override {
        if (button != 0) return false;
        if (contains(x, y)) {
            if (on_click_) on_click_();
            return true;
        }
        return false;
    }
    void on_draw(ui::Renderer& renderer) override {
        if (!font_) return;
        const ui::Vec4 green = theme_ ? theme_->success : ui::Vec4{0.09f, 0.72f, 0.47f, 1.0f};
        ui::Vec4 color = ui::Vec4{1, 1, 1, 1};  // 深色头部：默认白字
        if (active_ || hovered_) color = green;
        font_->draw_text(renderer, text_.c_str(), bounds_.x,
                         bounds_.y + (bounds_.h - font_->line_height()) * 0.5f, color);
    }

private:
    std::string text_;
    ui::Font* font_ = nullptr;
    const ui::Theme* theme_ = nullptr;
    bool active_ = false;
    std::function<void()> on_click_;
};

// 顶部栏（新镜像 docs/2 .ws-header 克隆）：深灰底 #2f363c + 底部 1px 分隔线；
// 左 logo 图片（240px 区域）+ 右侧导航文本 + 下拉
class Header : public ui::Widget {
public:
    Header(float w, float h) : ui::Widget(0, 0, w, h) {
        logo_img_ = new ui::Image(0, 0, 0, 0);
        add_child(logo_img_);
    }

    void set_font(ui::Font* f) { font_ = f; }
    void set_theme(const ui::Theme* t) { theme_ = t; }

    void load_logo(const char* path) { logo_img_->load_from_file(path); }

    ClickText* add_nav(const char* text, std::function<void()> cb) {
        ClickText* ct = new ClickText(text);
        ct->set_font(font_);
        ct->set_theme(theme_);
        ct->set_on_click(std::move(cb));
        navs_.push_back(ct);
        add_child(ct);
        return ct;
    }

    void add_dropdown(ui::Dropdown* dd) {
        dd->set_font(font_);
        dd->set_theme(theme_);
        dropdowns_.push_back(dd);
        add_child(dd);
    }

    void add_theme_switch(ui::Switch* sw) {
        sw->set_font(font_);
        sw->set_theme(theme_);
        theme_switch_ = sw;
        add_child(sw);
    }

    void on_layout() override {
        // 本 ui 库子控件 bounds 为绝对坐标（draw 无父级坐标变换），故子项均需叠加父 bounds_.y 偏移，
        // 否则导航栏内容（logo/文本/开关）会绘制在父顶部 (y=0) 而与自绘标题条重叠。
        const float oy = bounds_.y;
        // 左 240px logo 区域（与侧栏对齐，镜像 ws-logo 宽 240）
        logo_img_->set_bounds(0.f, oy, kSidebarW, bounds_.h);
        // 右侧导航从右往左排布（镜像 ws-header-menu 右 16px）：
        // 顺序（左→右）＝ 组件 / 笔记 / 生态(dropdown) / 2.13.9(dropdown) / 主题开关
        // 因此入队顺序 [组件, 笔记, 生态, 2.13.9, 开关] → 从右往左依次摆放。
        float x = bounds_.w - 16.f;
        if (theme_switch_) {
            const float sw_w = 44.f;
            theme_switch_->set_bounds(x - sw_w, oy + (bounds_.h - 24.f) * 0.5f, sw_w, 24.f);
            x -= sw_w + 24.f;
        }
        // 下拉（生态、2.13.9）：从右往左，最后加入的靠右
        for (int i = (int)dropdowns_.size() - 1; i >= 0; --i) {
            const float dd_w = 88.f, dd_h = 38.f;
            dropdowns_[i]->set_bounds(x - dd_w, oy + (bounds_.h - dd_h) * 0.5f, dd_w, dd_h);
            x -= dd_w + 24.f;
        }
        // 导航文本（组件、笔记）从右往左布局：navs_ 顺序 = [组件, 笔记] → 组件靠左
        for (int i = (int)navs_.size() - 1; i >= 0; --i) {
            const float w = navs_[i]->preferred_width();
            navs_[i]->set_bounds(x - w, oy, w, bounds_.h);
            x -= w + 32.f;
        }
    }

    void on_draw(ui::Renderer& renderer) override {
        // 深灰底 #2f363c（镜像 ws-header）
        renderer.draw_quad(bounds_, ui::Vec4{0.18f, 0.21f, 0.24f, 1.0f});
        renderer.draw_quad({bounds_.x, bounds_.y + bounds_.h - 1.f, bounds_.w, 1.f},
                           ui::Vec4{0.22f, 0.24f, 0.27f, 1.0f});
    }

private:
    ui::Image* logo_img_ = nullptr;
    std::vector<ClickText*> navs_;
    std::vector<ui::Dropdown*> dropdowns_;
    ui::Switch* theme_switch_ = nullptr;
    ui::Font* font_ = nullptr;
    const ui::Theme* theme_ = nullptr;
};

}  // namespace

#ifdef _WIN32
// 崩溃诊断：未处理异常时打印异常码/地址 + 符号化调用栈（配合 RelWithDebInfo 的 PDB）
namespace {
void print_crash_stack(const CONTEXT* ctx) {
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    if (!SymInitialize(GetCurrentProcess(), nullptr, TRUE)) return;
    STACKFRAME64 frame{};
    frame.AddrPC.Offset = ctx->Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx->Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx->Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
    for (int i = 0; i < 48; ++i) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(),
                         &frame, const_cast<CONTEXT*>(ctx), nullptr, SymFunctionTableAccess64,
                         SymGetModuleBase64, nullptr))
            break;
        const DWORD64 pc = frame.AddrPC.Offset;
        if (!pc) break;
        char buf[sizeof(SYMBOL_INFO) + 256] = {};
        auto* sym = reinterpret_cast<SYMBOL_INFO*>(buf);
        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = 255;
        DWORD64 disp = 0;
        if (SymFromAddr(GetCurrentProcess(), pc, &disp, sym))
            ui::log_error("  #%d %s+0x%llx (%s)", i, sym->Name,
                          static_cast<unsigned long long>(disp), sym->Name);
        else
            ui::log_error("  #%d pc=0x%llx", i, static_cast<unsigned long long>(pc));
    }
}
LONG WINAPI on_unhandled_crash(EXCEPTION_POINTERS* ep) {
    ui::log_error("CRASH code=0x%08lx addr=%p", ep->ExceptionRecord->ExceptionCode,
                  ep->ExceptionRecord->ExceptionAddress);
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && ep->ExceptionRecord->NumberParameters >= 2)
        ui::log_error("AV %s address 0x%p",
                      ep->ExceptionRecord->ExceptionInformation[0] ? "WRITE" : "READ",
                      reinterpret_cast<void*>(ep->ExceptionRecord->ExceptionInformation[1]));
    print_crash_stack(ep->ContextRecord);
    fflush(stdout);
    fflush(stderr);
    return EXCEPTION_CONTINUE_SEARCH;
}
}  // namespace
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    SetUnhandledExceptionFilter(on_unhandled_crash);
#endif
    double smoke_seconds = 0.0;
    const char* shot_path = nullptr;
    int shot_page = -1;
    int start_page = -1;
    float shot_scroll = 0.f;
    bool shot_after_click = false;  // --shot-after-click：点击注入后再截图（验证点击后状态）
    std::vector<std::pair<float, float>> click_points;  // --click x,y（可重复）
    struct DragSpec { float x1, y1, x2, y2; };          // --drag x1,y1 x2,y2：按下→多步移动→抬起
    std::vector<DragSpec> drag_points;
    std::vector<std::pair<float, float>> rclick_points;  // --rclick x,y（右键按钮1注入，UI-198 上下文菜单验证）
    std::vector<std::pair<float, float>> click_after_rclick_points;  // --click-after-rclick x,y：菜单弹出后下一帧点菜单项
    int click_after_wait = 0;  // 菜单弹板行几何需 layout 后才算出，右键帧之后再多等一帧再点
    bool click_after_done = false;
    bool cursor_warp = false;             // --cursor x,y：程序化定位真实光标（悬停/移出确定性验证，UI-197）
    float cursor_x = 0.f, cursor_y = 0.f;
    bool probe_video = false;             // --probe-video：打印控件树中各 VideoContainer 状态
    bool list_pages = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--smoke") == 0 && i + 1 < argc)
            smoke_seconds = std::atof(argv[++i]);
        else if (std::strcmp(argv[i], "--list-pages") == 0)
            list_pages = true;
        else if (std::strcmp(argv[i], "--shot") == 0 && i + 1 < argc)
            shot_path = argv[++i];
        else if (std::strcmp(argv[i], "--shot-page") == 0 && i + 1 < argc)
            shot_page = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--page") == 0 && i + 1 < argc)
            start_page = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--shot-scroll") == 0 && i + 1 < argc)
            shot_scroll = (float)std::atof(argv[++i]);
        else if (std::strcmp(argv[i], "--click") == 0 && i + 1 < argc) {
            float cx = 0.f, cy = 0.f;
            if (std::sscanf(argv[++i], "%f,%f", &cx, &cy) == 2)
                click_points.push_back({cx, cy});
        } else if (std::strcmp(argv[i], "--drag") == 0 && i + 1 < argc) {
            DragSpec d;
            if (std::sscanf(argv[++i], "%f,%f,%f,%f", &d.x1, &d.y1, &d.x2, &d.y2) == 4)
                drag_points.push_back(d);
        } else if (std::strcmp(argv[i], "--rclick") == 0 && i + 1 < argc) {
            float rx = 0.f, ry = 0.f;
            if (std::sscanf(argv[++i], "%f,%f", &rx, &ry) == 2)
                rclick_points.push_back({rx, ry});
        } else if (std::strcmp(argv[i], "--click-after-rclick") == 0 && i + 1 < argc) {
            float cx = 0.f, cy = 0.f;
            if (std::sscanf(argv[++i], "%f,%f", &cx, &cy) == 2)
                click_after_rclick_points.push_back({cx, cy});
        } else if (std::strcmp(argv[i], "--shot-after-click") == 0) {
            shot_after_click = true;
        } else if (std::strcmp(argv[i], "--cursor") == 0 && i + 1 < argc) {
            if (std::sscanf(argv[++i], "%f,%f", &cursor_x, &cursor_y) == 2)
                cursor_warp = true;
        } else if (std::strcmp(argv[i], "--probe-video") == 0) {
            probe_video = true;
        }
    }
    bool shot_done_ = false;
    int probe_frames = 0;  // --probe-video 触发帧（第 2 帧起状态稳定：布局完成 + 光标/点击已注入）

    ui::log_info("layui_test %s starting (%d pages)", ui::version_string(),
                 (int)page::all_pages().size());

    // 带自绘标题栏的窗口容器：居中 + 可拖动/最小化/最大化/关闭（UI-168）
    ui::WindowContainer container(1280, 900, "layui_test: clone of layui demo site");
    if (!container.ok()) {
        ui::log_error("window container init failed");
        return 1;
    }
    ui::Window& window = container.window();
    ui::Renderer& renderer = container.renderer();
    ui::Scene& scene = container.scene();

    ui::Font font;
    if (!font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f))
        ui::log_error("font load failed");
    // 中文字体（懒烘培）：env OPENGUI_CJK_FONT 自定义 → 系统候选（UI-036）
    if (!font.load_cjk_font_auto())
        ui::log_error("cjk font load failed");
    std::string exe_icon;
    if (argc > 0 && argv[0]) {
        const std::string p = argv[0];
        const size_t slash = p.find_last_of("/\\");
        if (slash != std::string::npos) exe_icon = p.substr(0, slash) + "/iconfont.ttf";
    }
    auto load_icons = [&]() {
        if (!exe_icon.empty() && font.load_icon_font(exe_icon.c_str(), 18.f)) return true;
        if (font.load_icon_font("iconfont.ttf", 18.f)) return true;
        if (font.load_icon_font("../assets/iconfont.ttf", 18.f)) return true;
        if (font.load_icon_font("assets/iconfont.ttf", 18.f)) return true;
        return false;
    };
    if (!load_icons()) ui::log_error("icon font load failed");

    ui::Theme theme = ui::Theme::make_layui();

    // 标题条样式注入：容器已注册 titlebar 为 chrome（content_origin_y 已生效），
    // 字体/主题/应用图标由应用注入（UI-168，keep UI-147/148 回归项）
    container.titlebar().set_font(&font);
    container.titlebar().set_theme(&theme);
    container.titlebar().set_icon(&font, ui::layui_icon::app);

    // ui::Scene：统一驱动生命周期（生产者→分配者→消费者→清理重置）
    scene.set_root_background(theme.window_bg);

    ui::Widget& root = scene.root();

    // 第二列子菜单可见态：选中数据表格板块时为 true，内容区右移 kChildSidebarW
    bool child_visible = false;

    // 库级顶部装帧：容器已注册标题条为 chrome，内容区起始 y 由库统一掌握（UI-153/168）。
    // 以下所有内容坐标（header/sidebar/tabs/content/HUD）均以该原点为单一来源派生。
    const float kChromeTop = container.content_origin_y();
    const float kTopH = kChromeTop + kHeaderH;            // 内容区起始 y（标题条 + 头部）
    const float content_y = kChromeTop + kHeaderH + kTabTitleH;

    // ---- 顶部栏：深色头部（新镜像 docs/2）+ logo + 组件/笔记/生态 + 主题开关 ----
    Header header((float)window.width(), kHeaderH);
    header.set_font(&font);
    header.set_theme(&theme);
    // 构造即定位于标题条下方（UI-148 回归）：y 以库内容区原点 kChromeTop 下移，
    // 否则首帧会与自绘 TitleBar(y=0) 重叠，标题条被深色 Header 盖住不可见
    header.set_bounds(0, kChromeTop, (float)window.width(), kHeaderH);
    root.add_child(&header);
    header.load_logo("logo.png");  // POST_BUILD 已拷贝到 exe 目录（Image 带 exe 回退）
    // 顺序（左→右）＝ 组件(active) / 笔记 / 生态(dropdown) / 2.13.9(dropdown) / 主题开关
    ClickText* docs_nav = header.add_nav("组件", []() {});
    docs_nav->set_active(true);
    header.add_nav("笔记", []() {});
    ui::Dropdown* eco = new ui::Dropdown(0, 0, 88.f, 38.f, "生态");
    eco->add_item("Layui 第三方扩展组件", []() {});
    eco->add_separator();
    eco->add_item("Layui 纯静态主题库", []() {});
    eco->add_item("Layui 深色主题", []() {});
    header.add_dropdown(eco);
    ui::Dropdown* version = new ui::Dropdown(0, 0, 88.f, 38.f, "2.13.9");
    version->add_item("v2.13.9 最新", []() {});
    version->add_separator();
    version->add_item("v2.13.8", []() {});
    version->add_item("v2.13.7", []() {});
    header.add_dropdown(version);
    ui::Switch* theme_sw = new ui::Switch(0, 0, 44.f, 24.f);
    theme_sw->set_text("ON|OFF");
    header.add_theme_switch(theme_sw);

    // ---- 侧栏：可滚动白色容器（新镜像 ws-menu 白底）+ 分组 Menu（layui-menu 克隆）----
    ui::ScrollBox sidebar(0, kTopH, kSidebarW, (float)window.height() - kTopH);
    sidebar.set_theme(&theme);
    sidebar.set_padding(0.f);
    sidebar.set_spacing(0.f);
    sidebar.add_class("ws-menu");
    root.add_child(&sidebar);

    ui::Menu nav(0, 0, kSidebarW, (float)window.height() - kTopH);
    nav.set_font(&font);
    nav.set_theme(&theme);
    sidebar.add_child(&nav);

    // ---- 第二列子菜单（数据表格板块，新镜像无此列，但测试程序需导航 30+ 子页）----
    ui::ScrollBox child_sidebar(kSidebarW, kTopH, kChildSidebarW,
                                (float)window.height() - kTopH);
    child_sidebar.set_theme(&theme);
    child_sidebar.set_padding(0.f);
    child_sidebar.set_spacing(0.f);
    child_sidebar.add_class("ws-menu");
    child_sidebar.set_visible(false);  // 默认隐藏，选中数据表格板块时显示  UI-156
    root.add_child(&child_sidebar);
    ui::Menu child_nav(0, 0, kChildSidebarW, (float)window.height() - kTopH);
    child_nav.set_font(&font);
    child_nav.set_theme(&theme);
    child_sidebar.add_child(&child_nav);

    // ---- 内容区：Tabs（预览/查看代码/帮助）+ 三个视图 ----
    // 位置由 place_content() 依据第二列可见态动态计算（构造先占位，随后 place_content 定界）
    ui::Tabs tabs(kSidebarW, kTopH, (float)window.width() - kSidebarW, kTabTitleH);
    tabs.set_font(&font);
    tabs.set_theme(&theme);
    tabs.add_class("layui-tab-brief");
    tabs.add_tab("预览");
    tabs.add_tab("查看代码");
    tabs.add_tab("帮助");
    root.add_child(&tabs);

    ui::ScrollBox preview(kSidebarW, content_y, (float)window.width() - kSidebarW,
                          (float)window.height() - content_y);
    preview.set_theme(&theme);
    preview.add_class("layui-body");
    preview.set_padding(24.f);
    preview.set_spacing(16.f);
    root.add_child(&preview);

    ui::Code code(kSidebarW, content_y, (float)window.width() - kSidebarW,
                  (float)window.height() - content_y);
    code.set_font(&font);
    code.set_theme(&theme);
    code.set_visible(false);  // UI-156：显隐经 set_visible（含 mark_dirty）
    root.add_child(&code);

    ui::ScrollBox help(kSidebarW, content_y, (float)window.width() - kSidebarW,
                       (float)window.height() - content_y);
    help.set_theme(&theme);
    help.add_class("layui-body");
    help.set_padding(24.f);
    help.set_spacing(16.f);
    help.set_visible(false);  // UI-156
    root.add_child(&help);

    auto set_view = [&](int idx) {
        preview.set_visible(idx == 0);
        code.set_visible(idx == 1);
        help.set_visible(idx == 2);
    };
    tabs.set_on_change(set_view);

    // 内容区/第二列定界：依据 child_visible 动态计算起点与宽度
    auto place_content = [&]() {
        const float content_x = kSidebarW + (child_visible ? kChildSidebarW : 0.f);
        const float cw = (float)window.width() - content_x;
        const float ch = (float)window.height() - content_y;
        child_sidebar.set_visible(child_visible);  // 子菜单可见态跟随 child_visible  UI-156
        child_sidebar.set_bounds(kSidebarW, kTopH, kChildSidebarW, (float)window.height() - kTopH);
        child_nav.set_bounds(0, 0, kChildSidebarW, (float)window.height() - kTopH);
        tabs.set_bounds(content_x, kTopH, cw, kTabTitleH);
        preview.set_bounds(content_x, content_y, cw, ch);
        code.set_bounds(content_x, content_y, cw, ch);
        help.set_bounds(content_x, content_y, cw, ch);
    };

    // 页面按 order 排序（跨 TU 静态注册顺序不确定），保证侧栏分组/顺序确定性
    std::vector<PageDef> pages = page::all_pages();
    std::stable_sort(pages.begin(), pages.end(),
                     [](const PageDef& a, const PageDef& b) { return a.order < b.order; });
    if (list_pages) {
        for (int i = 0; i < (int)pages.size(); ++i)
            std::printf("%d\t%s\t%s\n", i, pages[i].group, pages[i].title);
        std::fflush(stdout);
        return 0;
    }

    int current_page = -1;
    // 每个页面在侧栏中的 (组, 子项) 坐标，切页时同步 NavMenu 选中态（UI-050）
    std::vector<std::pair<int, int>> page_sel;
    // 第二列子菜单：(板块, 子页 index 列表)。navigate 切到板块锚点页时据此重建 child_nav。
    std::string active_board;         // 当前子菜单所属板块名（如 "数据表格"）
    std::function<void(int)> navigate;  // 前置声明，供 populate_child 递归引用（切到锚点后渲染首个子页）
    std::function<void(int)> render_page;  // 纯渲染指定页（不含导航/子列逻辑）
    auto populate_child = [&](const char* board) {
        child_nav.clear();
        child_nav.add_group(board ? board : "子菜单");
        // 点击分组头（如"数据表格"）：同时收起/展开整列子菜单（缺 UI-148 归因：分组头仅折叠
        // 组内子项、不联动外部列显隐，需宿主经 on_group_click 回调联动 child_sidebar 显隐）
        child_nav.set_on_group_click([&](const char* /*g*/) {
            child_visible = !child_visible;
            place_content();
        });
        active_board = board ? board : "";
        // 按 (seg, order) 排序板内子页，seg 变化间插分隔线（镜像 <hr> 分段）
        std::vector<int> sub;
        for (int i = 0; i < (int)pages.size(); ++i)
            if (pages[i].board && std::strcmp(pages[i].board, board) == 0 && !pages[i].board_anchor)
                sub.push_back(i);
        std::stable_sort(sub.begin(), sub.end(), [&](int a, int b) {
            if (pages[a].seg != pages[b].seg) return pages[a].seg < pages[b].seg;
            return pages[a].order < pages[b].order;
        });
        int last_seg = -1;
        for (int i : sub) {
            if (pages[i].seg != last_seg && last_seg != -1) child_nav.add_divider();
            if (last_seg == -1 || pages[i].seg != last_seg) last_seg = pages[i].seg;
            const int cap = i;
            child_nav.add_item(pages[i].title, [&, cap]() { navigate(cap); });
        }
        child_nav.end_group();  // add_group/end_group 配对闭合，保持层级卫生
    };
    // 页面宿主对象：生命周期必须覆盖全部页面控件回调——页面 build 里以 [&host]
    // 捕获的回调（按钮点击/滚动回调等）在 render_page 返回后仍会触发，
    // 此前 host 为 lambda 栈局部变量，回调触发时读到悬空引用（垃圾值）。
    PageHost host;
    render_page = [&](int idx) {
        if (idx < 0 || idx >= (int)pages.size()) return;
        host.content = &preview;
        host.code = &code;
        host.help = &help;
        host.font = &font;
        host.theme = &theme;
        host.scene = &scene;
        // 先清理上一页的根级弹出（destroy 钩子），再关闭全部弹出面板（分离根子指针），
        // 最后释放控件树 —— 防止切页后 root 残留悬空引用（UI-033）
        if (current_page >= 0 && current_page < (int)pages.size() &&
            pages[current_page].destroy)
            pages[current_page].destroy(host);
        root.close_popups();
        preview.set_on_scroll({});   // 切页前先清除上一页的滚动回调，
                                     // 否则回调中捕获的旧页控件（如 fixbar bar）
                                     // 已被 clear_children 释放，导致悬空调用崩溃。
        preview.clear_children();
        preview.reset_scroll();  // 切页滚动位置复位（UI-047：不残留旧页偏移）
        preview.set_spacing(16.f);  // 复位默认预览间距；页面 build 内可覆盖（如栅格页 10）
        help.clear_children();
        help.reset_scroll();
        code.clear();
        ui::Label* title = new ui::Label(0, 0, 800, 0, pages[idx].title);
        title->set_font(&font);
        title->set_theme(&theme);
        title->set_color(theme.text_secondary);
        preview.add_child(title);
        host.content = &preview;
        host.code = &code;
        host.help = &help;
        host.font = &font;
        host.theme = &theme;
        pages[idx].build(host);
        if (code.empty()) {
            code.set_lang("C++");
            code.add_line("// 该页面暂无示例代码（尚未克隆）。");
        }
        current_page = idx;
        set_view(0);
    };
    navigate = [&](int idx) {
        if (idx < 0 || idx >= (int)pages.size()) return;
        // 板块锚点页：显示第二列子菜单，切到该板块第一个子页，并同步主导航选中态
        if (pages[idx].board_anchor && pages[idx].board) {
            if (!child_visible) { child_visible = true; place_content(); }
            populate_child(pages[idx].board);
            if (idx < (int)page_sel.size())
                nav.set_selected(page_sel[idx].first, page_sel[idx].second);
            // 锚点不渲染；渲染该板块首个 board 子页
            for (int i = 0; i < (int)pages.size(); ++i) {
                if (pages[i].board && !pages[i].board_anchor &&
                    std::strcmp(pages[i].board, pages[idx].board) == 0) {
                    render_page(i);
                    return;
                }
            }
            return;
        }
        // 非锚点：若当前是板块子页，保持第二列；切到其他板块子页也保持并更新选中
        if (pages[idx].board && !pages[idx].board_anchor) {
            if (!child_visible) { child_visible = true; place_content(); }
            if (active_board != pages[idx].board) populate_child(pages[idx].board);
            render_page(idx);
            // 主导航选中同步到该板块锚点
            for (int i = 0; i < (int)pages.size(); ++i) {
                if (pages[i].board && pages[i].board_anchor &&
                    std::strcmp(pages[i].board, pages[idx].board) == 0 && i < (int)page_sel.size()) {
                    nav.set_selected(page_sel[i].first, page_sel[i].second);
                    return;
                }
            }
            return;
        }
        // 普通页：隐藏第二列
        if (child_visible) { child_visible = false; place_content(); }
        render_page(idx);
        if (idx < (int)page_sel.size())
            nav.set_selected(page_sel[idx].first, page_sel[idx].second);
    };

    // 导航：按页 order 分组（同组连续），add_group/add_item 交替追加。
    // 板块子页（board 非空且非锚点）不进主导航，仅在选中板块锚点时于第二列列出。
    // add_group 会加深 Menu 内部层级，换组前必须 end_group 闭合上一组，
    // 否则后续各组逐级嵌套成「子级折叠菜单」（UI-104：分组必须同级）。
    {
        const char* cur_group = nullptr;
        int gi = -1, ii = 0;
        for (int i = 0; i < (int)pages.size(); ++i) {
            // 板块子页：跳过（不占主导航项）
            if (pages[i].board && !pages[i].board_anchor) continue;
            if (!cur_group || std::strcmp(cur_group, pages[i].group) != 0) {
                if (cur_group) nav.end_group();
                nav.add_group(pages[i].group);
                cur_group = pages[i].group;
                ++gi;
                ii = 0;
            }
            const int cap = i;
            nav.add_item_sub(pages[i].title, pages[i].subtitle ? pages[i].subtitle : "",
                             [&navigate, cap]() { navigate(cap); });
            page_sel.push_back({gi, ii});
            ++ii;
        }
        if (cur_group) nav.end_group();
    }
    place_content();    // 定界内容区（第二列默认隐藏，content 起点 = kSidebarW）
    if (!pages.empty()) navigate(0);
    // --page <n>：启动即导航指定页并**持续运行**（真实鼠标/失焦复现用，区别于 --shot-page 截完即退）
    if (start_page >= 0 && start_page < (int)pages.size()) navigate(start_page);
    // --shot-page <n>：截图指定页（首帧即该页），供脚本化像素验证
    if (shot_page >= 0 && shot_page < (int)pages.size()) {
        navigate(shot_page);
        // --shot-scroll <px>：截图前滚动内容区（验证折叠区控件，如按钮页圆角按钮）
        if (shot_scroll > 0.f) preview.set_scroll_offset(shot_scroll);
    }

    // ---- Scene 生命周期订阅 ----
    auto last = std::chrono::steady_clock::now();  // HUD 每秒 tick 计时
    int frames = 0;
    char hud[64] = "";  // 右上角 HUD：每秒 tick 更新内容，每帧绘制（不闪烁，UI-049）
    // 冒烟自动切页游标（每 500ms 切下一页，验证 release_tree 无崩溃）
    int smoke_page = 0;
    auto last_switch = std::chrono::steady_clock::now();
    bool smoke_switched_any = false;
    bool clicks_injected = false;
    bool rclick_done = false;  // UI-198：右键只注入一次（勿逐帧重建打开中的上下文菜单）
    int inject_wait = 0;  // 等首个完整布局帧后再注入（UI-190）：Frame 阶段先于 layout/dispatch，首帧点击必偏

    // 帧前：冒烟自动切页 + --cursor 光标定位 + --click 合成点击注入（脚本化交互验证）
    scene.subscribe(ui::ScenePhase::Frame, [&](ui::Scene&, float) {
        // --cursor x,y：每帧重定位真实光标（同步写 mouse_x_/y_，UI-197 悬停确定性验证）。
        // 须逐帧调用：run() 的 poll_events 每秒会以 OS 实际光标位置刷回鼠标状态，仅在启动时
        // warp 一次会被真实光标覆盖；逐帧打点后 dispatch_mouse_move 每帧稳定读到目标位置，
        // 悬停显示/移出隐藏结果与 OS 光标位置解耦（合成点击仍走下方注入，两者互不干扰）。
        if (cursor_warp) window.set_cursor_pos(cursor_x, cursor_y);
        if (smoke_seconds > 0.0 && (int)pages.size() > 1 &&
            std::chrono::steady_clock::now() - last_switch >= std::chrono::milliseconds(500)) {
            smoke_page = (smoke_page + 1) % (int)pages.size();
            navigate(smoke_page);
            last_switch = std::chrono::steady_clock::now();
            smoke_switched_any = true;
        }
        // 首次布局后注入合成点击（与旧主循环一致），再布局一次保证首帧截图正确。
        // UI-190：Frame 观察者运行于 scene.frame() 的「分配输入（layout+dispatch）」之前，
        // 首帧注入时页面控件尚未布局（bounds 仍是初值）会全部脱靶；延迟到第 2 帧（页面已
        // 完成首帧布局）再注入，仍保持「单次点击」语义。
        if (!click_points.empty() || !drag_points.empty() || !rclick_points.empty()) {
            if (inject_wait >= 1) {
                clicks_injected = true;
                for (const auto& c : click_points) {
                    root.dispatch_mouse_move(c.first, c.second);
                    root.dispatch_mouse_down(c.first, c.second, 0);
                    root.dispatch_mouse_up(c.first, c.second, 0);
                }
                for (const auto& d : drag_points) {
                    // 拖拽注入：按下→8 步插值移动→抬起（滑块 carrying_ 期间 move 逐帧改值）
                    root.dispatch_mouse_move(d.x1, d.y1);
                    root.dispatch_mouse_down(d.x1, d.y1, 0);
                    for (int i = 1; i <= 8; ++i) {
                        const float t = i / 8.f;
                        root.dispatch_mouse_move(d.x1 + (d.x2 - d.x1) * t, d.y1 + (d.y2 - d.y1) * t);
                    }
                    root.dispatch_mouse_up(d.x2, d.y2, 0);
                }
                // 右键（button 1）注入（UI-198）：命中控件树 → 打开上下文菜单（VideoContainer/Dropdown）。
                // 只注入一次（rclick_done）：菜单弹出后需保持 open 状态供后续 --click-after-rclick /
                // --cursor 悬停交互，逐帧重注入会反复重建菜单、重置展开态导致后续点击脱靶。
                if (!rclick_done) {
                    rclick_done = true;
                    for (const auto& r : rclick_points) {
                        root.dispatch_mouse_move(r.first, r.second);
                        root.dispatch_mouse_down(r.first, r.second, 1);
                        root.dispatch_mouse_up(r.first, r.second, 1);
                    }
                }
                root.layout();
            } else {
                ++inject_wait;
            }
        }
        // --click-after-rclick（UI-198）：菜单弹出后下一帧点菜单项，验证动作回写状态。
        // 右键注入帧里菜单弹板虽已 add_child 到 root，但行几何（it.y0/y1）要等 layout 才算，
        // 同帧点必脱靶 → 等 clicks_injected 后再等一帧（click_after_wait>=1）才命中。
        if (!click_after_rclick_points.empty() && clicks_injected && !click_after_done) {
            if (click_after_wait < 1) {
                ++click_after_wait;
            } else {
                for (const auto& c : click_after_rclick_points) {
                    root.dispatch_mouse_move(c.first, c.second);
                    root.dispatch_mouse_down(c.first, c.second, 0);
                    root.dispatch_mouse_up(c.first, c.second, 0);
                }
                root.layout();
                click_after_done = true;
            }
        }
    });

    // 窗口尺寸变化：Scene 已更新视口+root；标题条宽度由容器随 Frame 自同步（UI-168）；
    // 此处仅重排顶部/侧栏 + 内容
    scene.set_event_hook(ui::SceneEvent::Resized, [&](ui::Scene&) {
        header.set_bounds(0, kChromeTop, (float)window.width(), kHeaderH);
        sidebar.set_bounds(0, kTopH, kSidebarW, (float)window.height() - kTopH);
        nav.set_bounds(0, 0, kSidebarW, (float)window.height() - kTopH);
        place_content();
        return true;
    });

    // Escape：关最上层弹窗（非标准编辑键 → Key 钩子自处理；其余键留给标准模板分发）
    scene.set_event_hook(ui::SceneEvent::Key, [&](ui::Scene& s) {
        if (s.event_key() == ui::KeyEscape) {
            root.close_topmost_popup();
            return true;
        }
        return false;
    });

    // 帧后：截图（glReadPixels，begin_frame+root.draw 之后）+ HUD 绘制
    scene.on(ui::ScenePhase::PostDraw, [&](ui::Scene& s) {
        // --probe-video：布局完成 + 光标/点击已注入后打印一次（端帧状态，含 dispatch 结果）
        if (probe_video && probe_frames < 2) {
            ++probe_frames;
            if (probe_frames >= 2) probe_video_containers(root, 0);
        }
        if (shot_path && !shot_done_ && (!shot_after_click ||
                                        (click_points.empty() && drag_points.empty() && rclick_points.empty() &&
                                         click_after_rclick_points.empty()) ||
                                        (clicks_injected &&
                                         (click_after_rclick_points.empty() || click_after_done)))) {
            shot_done_ = true;
            const int w = window.width();
            const int h = window.height();
            std::vector<unsigned char> px((size_t)w * h * 4);
            glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
            std::vector<unsigned char> flipped((size_t)w * h * 4);
            for (int y = 0; y < h; ++y)
                std::memcpy(flipped.data() + (size_t)(h - 1 - y) * w * 4,
                            px.data() + (size_t)y * w * 4, (size_t)w * 4);
            FILE* f = fopen(shot_path, "wb");
            if (f) {
                fwrite("P7\n", 1, 3, f);
                std::fprintf(f,
                             "WIDTH %d\nHEIGHT %d\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n",
                             w, h);
                fwrite(flipped.data(), 1, flipped.size(), f);
                fclose(f);
                ui::log_info("shot saved: %s (%dx%d)", shot_path, w, h);
            } else {
                ui::log_error("shot save failed: %s", shot_path);
            }
            s.request_exit();  // 截图完成即退出
        }

        ++frames;
        auto now = std::chrono::steady_clock::now();
        if (now - last >= std::chrono::seconds(1)) {
            std::snprintf(hud, sizeof(hud), "layui_test | page %d/%d | %.0f fps", current_page + 1,
                          (int)pages.size(),
                          frames / std::chrono::duration<double>(now - last).count());
            frames = 0;
            last = now;
        }
        if (hud[0]) {
            // 头部中段空区（logo 区之后、右侧控件群之前）：右对齐会压在
            // 主题开关/生态/2.13.9 下拉上（UI-105）。垂直以库内容区原点 kChromeTop 下移，
            // 在深色 Header（y=kChromeTop..+kHeaderH）内垂直居中，避免与自绘标题条重叠（UI-153）。
            const float hud_x = kSidebarW + 16.f;
            font.draw_text(renderer, hud, hud_x,
                           kChromeTop + (kHeaderH - font.line_height()) * 0.5f,
                           theme.text_secondary);
        }
    });

    // 运行：--smoke 指定秒自动退出（含切页统计），否则直至窗口关闭
    // --cursor x,y：先程序化定位真实光标（同步回调更新 window.mouse_x_/y_）。
    // Scene::frame 每帧用该位置 dispatch_mouse_move → 悬停/移出结果确定，不受合成
    // 点击注入（早于本行的 Frame 观察者）覆盖（UI-197 悬停显隐的确定性验证）。
    if (cursor_warp) window.set_cursor_pos(cursor_x, cursor_y);
    if (smoke_seconds > 0.0) {
        scene.run_for(smoke_seconds);
        ui::log_info("smoke: exiting after %.1fs%s", smoke_seconds,
                     smoke_switched_any ? " (page switches ok)" : "");
    } else {
        scene.run();
    }
    return 0;
}
