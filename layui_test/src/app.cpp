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
#include "ui/ui/menu.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/switch.h"
#include "ui/ui/tabs.h"
#include "ui/ui/theme.h"
#include "ui/ui/widget.h"

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

namespace {

constexpr float kHeaderH = 60.f;
constexpr float kSidebarW = 240.f;  // 新镜像 docs/2 侧栏宽（ws-menu 240）
constexpr float kChildSidebarW = 160.f;  // 第二列子菜单宽（镜像 side-child 160）
constexpr float kTabTitleH = 40.f;

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
        // 左 240px logo 区域（与侧栏对齐，镜像 ws-logo 宽 240）
        logo_img_->set_bounds(0.f, 0.f, kSidebarW, bounds_.h);
        // 右侧导航从右往左排布（镜像 ws-header-menu 右 16px）：
        // 顺序（左→右）＝ 组件 / 笔记 / 生态(dropdown) / 2.13.9(dropdown) / 主题开关
        // 因此入队顺序 [组件, 笔记, 生态, 2.13.9, 开关] → 从右往左依次摆放。
        float x = bounds_.w - 16.f;
        if (theme_switch_) {
            const float sw_w = 44.f;
            theme_switch_->set_bounds(x - sw_w, (bounds_.h - 24.f) * 0.5f, sw_w, 24.f);
            x -= sw_w + 24.f;
        }
        // 下拉（生态、2.13.9）：从右往左，最后加入的靠右
        for (int i = (int)dropdowns_.size() - 1; i >= 0; --i) {
            const float dd_w = 88.f, dd_h = 38.f;
            dropdowns_[i]->set_bounds(x - dd_w, (bounds_.h - dd_h) * 0.5f, dd_w, dd_h);
            x -= dd_w + 24.f;
        }
        // 导航文本（组件、笔记）从右往左布局：navs_ 顺序 = [组件, 笔记] → 组件靠左
        for (int i = (int)navs_.size() - 1; i >= 0; --i) {
            const float w = navs_[i]->preferred_width();
            navs_[i]->set_bounds(x - w, 0.f, w, bounds_.h);
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

int main(int argc, char** argv) {
    double smoke_seconds = 0.0;
    const char* shot_path = nullptr;
    int shot_page = -1;
    float shot_scroll = 0.f;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--smoke") == 0 && i + 1 < argc)
            smoke_seconds = std::atof(argv[++i]);
        else if (std::strcmp(argv[i], "--shot") == 0 && i + 1 < argc)
            shot_path = argv[++i];
        else if (std::strcmp(argv[i], "--shot-page") == 0 && i + 1 < argc)
            shot_page = std::atoi(argv[++i]);
        else if (std::strcmp(argv[i], "--shot-scroll") == 0 && i + 1 < argc)
            shot_scroll = (float)std::atof(argv[++i]);
    }
    const auto smoke_start = std::chrono::steady_clock::now();
    bool shot_done_ = false;

    ui::log_info("layui_test %s starting (%d pages)", ui::version_string(),
                 (int)page::all_pages().size());

    ui::Window window(1280, 900, "layui_test: clone of layui demo site");
    if (!window.handle()) {
        ui::log_error("no GL context");
        return 1;
    }
    ui::Renderer renderer;
    if (!renderer.init()) {
        ui::log_error("renderer init failed");
        return 1;
    }
    renderer.set_viewport(window.width(), window.height());

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

    ui::Widget root;
    root.set_bounds(0, 0, (float)window.width(), (float)window.height());

    // 第二列子菜单可见态：选中数据表格板块时为 true，内容区右移 kChildSidebarW
    bool child_visible = false;
    const float content_y = kHeaderH + kTabTitleH;

    // ---- 顶部栏：深色头部（新镜像 docs/2）+ logo + 组件/笔记/生态 + 主题开关 ----
    Header header((float)window.width(), kHeaderH);
    header.set_font(&font);
    header.set_theme(&theme);
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
    ui::ScrollBox sidebar(0, kHeaderH, kSidebarW, (float)window.height() - kHeaderH);
    sidebar.set_theme(&theme);
    sidebar.set_padding(0.f);
    sidebar.set_spacing(0.f);
    sidebar.add_class("ws-menu");
    root.add_child(&sidebar);

    ui::Menu nav(0, 0, kSidebarW, (float)window.height() - kHeaderH);
    nav.set_font(&font);
    nav.set_theme(&theme);
    sidebar.add_child(&nav);

    // ---- 第二列子菜单（数据表格板块，新镜像无此列，但测试程序需导航 30+ 子页）----
    ui::ScrollBox child_sidebar(kSidebarW, kHeaderH, kChildSidebarW,
                                (float)window.height() - kHeaderH);
    child_sidebar.set_theme(&theme);
    child_sidebar.set_padding(0.f);
    child_sidebar.set_spacing(0.f);
    child_sidebar.add_class("ws-menu");
    child_sidebar.visible = false;  // 默认隐藏，选中数据表格板块时显示
    root.add_child(&child_sidebar);
    ui::Menu child_nav(0, 0, kChildSidebarW, (float)window.height() - kHeaderH);
    child_nav.set_font(&font);
    child_nav.set_theme(&theme);
    child_sidebar.add_child(&child_nav);

    // ---- 内容区：Tabs（预览/查看代码/帮助）+ 三个视图 ----
    // 位置由 place_content() 依据第二列可见态动态计算（构造先占位，随后 place_content 定界）
    ui::Tabs tabs(kSidebarW, kHeaderH, (float)window.width() - kSidebarW, kTabTitleH);
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
    code.visible = false;
    root.add_child(&code);

    ui::ScrollBox help(kSidebarW, content_y, (float)window.width() - kSidebarW,
                       (float)window.height() - content_y);
    help.set_theme(&theme);
    help.add_class("layui-body");
    help.set_padding(24.f);
    help.set_spacing(16.f);
    help.visible = false;
    root.add_child(&help);

    auto set_view = [&](int idx) {
        preview.visible = (idx == 0);
        code.visible = (idx == 1);
        help.visible = (idx == 2);
    };
    tabs.set_on_change(set_view);

    // 内容区/第二列定界：依据 child_visible 动态计算起点与宽度
    auto place_content = [&]() {
        const float content_x = kSidebarW + (child_visible ? kChildSidebarW : 0.f);
        const float cw = (float)window.width() - content_x;
        const float ch = (float)window.height() - content_y;
        child_sidebar.visible = child_visible;  // 子菜单可见态跟随 child_visible
        child_sidebar.set_bounds(kSidebarW, kHeaderH, kChildSidebarW, (float)window.height() - kHeaderH);
        child_nav.set_bounds(0, 0, kChildSidebarW, (float)window.height() - kHeaderH);
        tabs.set_bounds(content_x, kHeaderH, cw, kTabTitleH);
        preview.set_bounds(content_x, content_y, cw, ch);
        code.set_bounds(content_x, content_y, cw, ch);
        help.set_bounds(content_x, content_y, cw, ch);
    };

    // 页面按 order 排序（跨 TU 静态注册顺序不确定），保证侧栏分组/顺序确定性
    std::vector<PageDef> pages = page::all_pages();
    std::stable_sort(pages.begin(), pages.end(),
                     [](const PageDef& a, const PageDef& b) { return a.order < b.order; });

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
    render_page = [&](int idx) {
        if (idx < 0 || idx >= (int)pages.size()) return;
        PageHost host;
        host.content = &preview;
        host.code = &code;
        host.help = &help;
        host.font = &font;
        host.theme = &theme;
        // 先清理上一页的根级弹出（destroy 钩子），再关闭全部弹出面板（分离根子指针），
        // 最后释放控件树 —— 防止切页后 root 残留悬空引用（UI-033）
        if (current_page >= 0 && current_page < (int)pages.size() &&
            pages[current_page].destroy)
            pages[current_page].destroy(host);
        root.close_popups();
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
    // --shot-page <n>：截图指定页（首帧即该页），供脚本化像素验证
    if (shot_page >= 0 && shot_page < (int)pages.size()) {
        navigate(shot_page);
        // --shot-scroll <px>：截图前滚动内容区（验证折叠区控件，如按钮页圆角按钮）
        if (shot_scroll > 0.f) preview.set_scroll_offset(shot_scroll);
    }

    auto last = std::chrono::steady_clock::now();
    auto prev = last;
    int frames = 0;
    char hud[64] = "";  // 右上角 HUD：每秒 tick 更新内容，每帧绘制（不闪烁，UI-049）
    // 冒烟自动切页游标（每 500ms 切下一页，验证 release_tree 无崩溃）
    int smoke_page = 0;
    auto last_switch = last;
    bool smoke_switched_any = false;
    while (!window.should_close()) {
        window.poll_events();
        const float dt = std::chrono::duration<float>(std::chrono::steady_clock::now() - prev).count();
        prev = std::chrono::steady_clock::now();
        if (smoke_seconds > 0.0 &&
            std::chrono::duration<double>(std::chrono::steady_clock::now() - smoke_start).count() >=
                smoke_seconds) {
            ui::log_info("smoke: exiting after %.1fs%s", smoke_seconds,
                         smoke_switched_any ? " (page switches ok)" : "");
            break;
        }
        if (smoke_seconds > 0.0 && (int)pages.size() > 1 &&
            std::chrono::steady_clock::now() - last_switch >= std::chrono::milliseconds(500)) {
            smoke_page = (smoke_page + 1) % (int)pages.size();
            navigate(smoke_page);
            last_switch = std::chrono::steady_clock::now();
            smoke_switched_any = true;
        }
        (void)dt;
        root.advance(dt);

        if (window.resized()) {
            renderer.set_viewport(window.width(), window.height());
            root.set_bounds(0, 0, (float)window.width(), (float)window.height());
            header.set_bounds(0, 0, (float)window.width(), kHeaderH);
            sidebar.set_bounds(0, kHeaderH, kSidebarW, (float)window.height() - kHeaderH);
            nav.set_bounds(0, 0, kSidebarW, (float)window.height() - kHeaderH);
            place_content();
        }

        root.layout();
        root.dispatch_mouse_move(window.mouse_x(), window.mouse_y());
        if (window.mouse_pressed(0)) root.dispatch_mouse_down(window.mouse_x(), window.mouse_y(), 0);
        if (window.mouse_released(0)) root.dispatch_mouse_up(window.mouse_x(), window.mouse_y(), 0);
        const float sdx = window.scroll_dx();
        const float sdy = window.scroll_dy();
        if (sdx != 0.f || sdy != 0.f)
            root.dispatch_mouse_scroll(window.mouse_x(), window.mouse_y(), sdx, sdy);

        const int nav_keys[] = {ui::KeyTab, ui::KeyUp, ui::KeyDown, ui::KeyLeft, ui::KeyRight};
        for (int k : nav_keys) {
            if (window.key_pressed(k))
                root.dispatch_key_down(k, window.key_down(ui::KeyLeftShift) ? ui::ModShift : 0);
        }
        if (window.key_pressed(ui::KeyEnter)) root.dispatch_key_down(ui::KeyEnter, 0);
        if (window.key_pressed(ui::KeySpace)) root.dispatch_key_down(ui::KeySpace, 0);
        if (window.key_pressed(ui::KeyBackspace)) root.dispatch_key_down(ui::KeyBackspace, 0);
        if (window.key_pressed(ui::KeyDelete)) root.dispatch_key_down(ui::KeyDelete, 0);
        if (window.key_pressed(ui::KeyHome)) root.dispatch_key_down(ui::KeyHome, 0);
        if (window.key_pressed(ui::KeyEnd)) root.dispatch_key_down(ui::KeyEnd, 0);
        if (window.key_pressed(ui::KeyEscape)) root.close_topmost_popup();
        const int nchar = window.char_pressed_count();
        for (int i = 0; i < nchar; ++i) root.dispatch_char(window.char_pressed_at(i));
        window.clear_chars();

        renderer.begin_frame(theme.window_bg);
        root.draw(renderer);

        if (shot_path && !shot_done_) {
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
                std::fprintf(f, "WIDTH %d\nHEIGHT %d\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n", w, h);
                fwrite(flipped.data(), 1, flipped.size(), f);
                fclose(f);
                ui::log_info("shot saved: %s (%dx%d)", shot_path, w, h);
            } else {
                ui::log_error("shot save failed: %s", shot_path);
            }
            break;
        }

        ++frames;
        auto now = std::chrono::steady_clock::now();
        if (now - last >= std::chrono::seconds(1)) {
            std::snprintf(hud, sizeof(hud), "layui_test | page %d/%d | %.0f fps", current_page + 1,
                          (int)pages.size(), frames / std::chrono::duration<double>(now - last).count());
            frames = 0;
            last = now;
        }
        if (hud[0]) {
            // 头部中段空区（logo 区之后、右侧控件群之前）：右对齐会压在
            // 主题开关/生态/2.13.9 下拉上（UI-105）
            const float hud_x = kSidebarW + 16.f;
            font.draw_text(renderer, hud, hud_x, (kHeaderH - font.line_height()) * 0.5f,
                           theme.text_secondary);
        }

        window.swap_buffers();
    }
    return 0;
}
