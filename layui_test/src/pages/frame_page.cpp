#include "../page_registry.h"

#include "ui/core/log.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/image.h"
#include "ui/ui/label.h"
#include "ui/ui/navmenu.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"
#include "ui/ui/widget.h"

#include <string>
#include <vector>

namespace {

// 内容主体内容（挂在 layui-body ScrollBox 内）：blockquote + 充数卡片，
// 高度 > 视口触发滚动条（对齐 layout-admin.html 的 body 纵向滚动）。
class BodyContent : public ui::Widget {
public:
    explicit BodyContent(float w) : ui::Widget(0, 0, w, kHeight) {}

    void set_font(ui::Font* f) { font_ = f; }
    float preferred_height() const override { return kHeight; }

    void on_draw(ui::Renderer& renderer) override {
        if (!font_) return;
        const float x0 = bounds_.x, y0 = bounds_.y, w = bounds_.w;
        const ui::Vec4 green{0.0f, 0.59f, 0.53f, 1.0f};
        const ui::Vec4 gray{0.95f, 0.95f, 0.95f, 1.0f};
        const ui::Vec4 line{0.93f, 0.93f, 0.93f, 1.0f};
        const float bx = x0 + 20.f;
        const float bw = w - 40.f;

        // blockquote（layui-elem-quote）
        const float qy = y0 + 20.f;
        renderer.draw_quad({bx, qy, bw, 74.f}, gray);
        renderer.draw_quad({bx, qy, 5.f, 74.f}, green);
        font_->draw_text(renderer, "layui 之所以赢得如此多人的青睐，更多是在于它“前后界面兼备”的能力。",
                         bx + 14.f, qy + 12.f, ui::Vec4{0.4f, 0.4f, 0.4f, 1.0f});
        font_->draw_text(renderer, "layui 管理基本布局，致力于让每一位开发者都能轻松搭建自己的管理系统模板。",
                         bx + 14.f, qy + 12.f + font_->line_height() + 6.f,
                         ui::Vec4{0.4f, 0.4f, 0.4f, 1.0f});

        // 充数卡片（layui-card layui-panel）
        const float cy0 = qy + 100.f;
        renderer.draw_quad({bx, cy0, bw, 40.f}, ui::Vec4{0.98f, 0.98f, 0.98f, 1.0f});
        renderer.draw_quad({bx, cy0 + 40.f, bw, 1.f}, line);
        font_->draw_text(renderer, "下面是充数内容，为的是出现滚动条", bx + 14.f,
                         cy0 + (40.f - font_->line_height()) * 0.5f, ui::Vec4{0.33f, 0.33f, 0.33f, 1.0f});
        renderer.draw_quad({bx, cy0 + 41.f, bw, kHeight - (cy0 - y0) - 41.f}, ui::Vec4{1, 1, 1, 1});
        for (int i = 0; i < 13; ++i)
            font_->draw_text(renderer, "充数内容", bx + 14.f, cy0 + 56.f + (float)i * (font_->line_height() + 8.f),
                             ui::Vec4{0.5f, 0.5f, 0.5f, 1.0f});
        font_->draw_text(renderer, "你还真滑到了底部呀", bx + 14.f,
                         cy0 + 56.f + 13.f * (font_->line_height() + 8.f) + 8.f,
                         ui::Vec4{0.6f, 0.6f, 0.6f, 1.0f});
    }

private:
    static constexpr float kHeight = 640.f;  // 高于内容视口（560-40-36=484）触发滚动
    ui::Font* font_ = nullptr;
};

// 自绘 mini 后台布局（克隆 layui layout-admin.html）：黑头 + 黑侧栏 + 白内容 + 灰底。
// 整个框架在一个控件内绘制，避免外层 ScrollBox 流式布局打散结构。
class AdminFrame : public ui::Widget {
public:
    AdminFrame(float w, float h) : ui::Widget(0, 0, w, h) {
        avatar_ = new ui::Image(0, 0, 36.f, 36.f);
        add_child(avatar_);
        // 侧栏折叠菜单：真实 NavMenu 子控件（点击一级组展开/折叠 + 子项选中态，UI-049/UI-050）
        menu_ = new ui::NavMenu(0, 0, 0, 0);
        add_child(menu_);
        // 用户区触发（命中测试/弹层 owner，不绘制）
        user_trigger_ = new ui::Widget(0, 0, 0, 0);
        add_child(user_trigger_);
        // 用户下拉菜单（Profile/Settings/Sign out，对齐 layout-admin.html 头部右侧）
        user_menu_ = new ui::VBox(0, 0, 0, 0);
        user_menu_->set_padding(0.f);
        user_menu_->set_spacing(0.f);
        user_menu_->add_class("layui-menu");
        user_menu_->visible = false;
        // 内容主体：layui-body 白底 + 内部可滚动（UI-050）
        body_ = new ui::ScrollBox(0, 0, 0, 0);
        body_->add_class("layui-body");
        body_->set_padding(0.f);
        body_->set_spacing(0.f);
        add_child(body_);
        body_content_ = new BodyContent(0);
        body_->add_child(body_content_);
        // 头部 "nav groups" 折叠菜单触发（命中测试/弹层 owner，不绘制）
        nav_groups_trigger_ = new ui::Widget(0, 0, 0, 0);
        add_child(nav_groups_trigger_);
        // 折叠菜单（menu 11/22/33，对齐 layout-admin.html 的 layui-nav-child）
        nav_groups_menu_ = new ui::VBox(0, 0, 0, 0);
        nav_groups_menu_->set_padding(0.f);
        nav_groups_menu_->set_spacing(0.f);
        nav_groups_menu_->add_class("layui-menu");
        nav_groups_menu_->visible = false;
    }

    ~AdminFrame() override {
        if (user_menu_->parent()) user_menu_->parent()->remove_child(user_menu_);
        user_menu_->clear_children();
        delete user_menu_;
        if (nav_groups_menu_->parent()) nav_groups_menu_->parent()->remove_child(nav_groups_menu_);
        nav_groups_menu_->clear_children();
        delete nav_groups_menu_;
    }

    void set_font(ui::Font* f) {
        font_ = f;
        menu_->set_font(f);
        body_content_->set_font(f);
    }
    void set_theme(const ui::Theme* t) {
        theme_ = t;
        menu_->set_theme(t);
        user_menu_->set_theme(t);
        nav_groups_menu_->set_theme(t);
    }
    ui::Image* avatar() const { return avatar_; }
    ui::NavMenu* menu() const { return menu_; }

    // 侧栏菜单结构（对齐 layout-admin.html 左侧 layui-nav-tree）
    void setup_menu() {
        menu_->add_group("menu group 1");
        menu_->add_item("menu 1");
        menu_->add_item("menu 2");
        menu_->add_item("menu 3");
        menu_->add_item("the links");
        menu_->add_group("menu group 2");
        menu_->add_item("list 1");
        menu_->add_item("list 2");
        menu_->add_item("超链接");
        menu_->add_group("click menu item");
        menu_->add_group("the links");
        menu_->set_active(0);
        menu_->expand_all();
        menu_->set_selected(0, 2);  // 默认选中 "menu 3"（layui .layui-this 绿底白字）
    }

    // 用户下拉菜单项
    void setup_user_menu() {
        const float kItemW = 120.f;
        const char* items[] = {"Your Profile", "Settings", "Sign out"};
        for (const char* s : items) {
            ui::Button* b = new ui::Button(0, 0, kItemW, 26.f, s);
            b->add_class("layui-menu li");
            b->set_font(font_);
            b->set_theme(theme_);
            b->set_on_click([this]() { close_popups(); });
            user_menu_->add_child(b);
        }
    }

    // 头部 "nav groups" 折叠菜单项（menu 11/22/33）
    void setup_nav_groups_menu() {
        const float kItemW = 110.f;
        const char* items[] = {"menu 11", "menu 22", "menu 33"};
        for (const char* s : items) {
            ui::Button* b = new ui::Button(0, 0, kItemW, 26.f, s);
            b->add_class("layui-menu li");
            b->set_font(font_);
            b->set_theme(theme_);
            b->set_on_click([this]() { close_popups(); });
            nav_groups_menu_->add_child(b);
        }
    }

    void on_layout() override {
        if (!font_) return;
        const float header_h = kHeaderH;
        const float footer_h = 36.f;
        const float x0 = bounds_.x, y0 = bounds_.y, w = bounds_.w, h = bounds_.h;

        // 头部右侧用户区：头像 + "tester" + 箭头（对齐 layout-admin.html .layui-layout-right）
        const float name_w = font_->measure_text(kUserName);
        const float caret_w = font_->measure_text(kCaret);
        const float gap = 8.f;
        const float user_w = 12.f + 36.f + gap + name_w + 6.f + caret_w + 12.f;
        const float avatar_x = x0 + w - user_w;
        avatar_->set_bounds(avatar_x, y0 + (header_h - 36.f) * 0.5f, 36.f, 36.f);
        user_name_x_ = avatar_x + 36.f + gap;
        user_caret_x_ = user_name_x_ + name_w + 6.f;
        user_rect_ = ui::Rect{avatar_x, y0, user_w, header_h};
        user_trigger_->set_bounds(user_rect_.x, user_rect_.y, user_rect_.w, user_rect_.h);

        // 头部导航项：logo 之后从左到右（点击高亮，hover 变绿；nav groups 弹折叠菜单）
        navs_.clear();
        const float logo_w = font_->measure_text("layout demo");
        float nx = x0 + 16.f + logo_w + 28.f;
        const char* nav_texts[] = {"nav 1", "nav 2", "nav 3", "nav groups"};
        for (const char* t : nav_texts) {
            const float tw = font_->measure_text(t);
            navs_.push_back({t, nx, y0, tw, header_h});
            nx += tw + 24.f;
        }
        if (navs_.size() >= 4)  // "nav groups" 折叠菜单触发区
            nav_groups_trigger_->set_bounds(navs_[3].x, navs_[3].y, navs_[3].w, navs_[3].h);

        // 侧栏折叠菜单区域
        menu_->set_bounds(x0, y0 + header_h + 10.f, kSidebarW,
                          bounds_.h - header_h - footer_h - 10.f);
        // 内容主体（layui-body）：可滚动区域
        body_->set_bounds(x0 + kSidebarW, y0 + header_h, w - kSidebarW,
                          h - header_h - footer_h);
    }

    void on_draw(ui::Renderer& renderer) override {
        if (!font_) return;
        const float x0 = bounds_.x, y0 = bounds_.y, w = bounds_.w, h = bounds_.h;
        const ui::Vec4 black{0.20f, 0.22f, 0.24f, 1.0f};
        const ui::Vec4 dark{0.13f, 0.15f, 0.17f, 1.0f};
        const ui::Vec4 green{0.0f, 0.59f, 0.53f, 1.0f};
        const ui::Vec4 white{1, 1, 1, 1};

        const float header_h = kHeaderH;
        const float sidebar_w = kSidebarW;
        const float footer_h = 36.f;

        // ---- 头部黑条 ----
        renderer.draw_quad({x0, y0, w, header_h}, black);
        font_->draw_text(renderer, "layout demo", x0 + 16.f,
                         y0 + (header_h - font_->line_height()) * 0.5f, white);
        for (int i = 0; i < (int)navs_.size(); ++i) {
            const HeaderItem& ni = navs_[i];
            const ui::Vec4 color = (i == active_nav_ || i == hover_nav_)
                                       ? green
                                       : ui::Vec4{0.85f, 0.85f, 0.85f, 1.0f};
            font_->draw_text(renderer, ni.text.c_str(), ni.x,
                             y0 + (header_h - font_->line_height()) * 0.5f, color);
        }
        // 用户区：名称 + 箭头（头像为子控件 Image 自动绘制）
        font_->draw_text(renderer, kUserName, user_name_x_,
                         y0 + (header_h - font_->line_height()) * 0.5f,
                         ui::Vec4{0.95f, 0.95f, 0.95f, 1.0f});
        font_->draw_text(renderer, kCaret, user_caret_x_,
                         y0 + (header_h - font_->line_height()) * 0.5f,
                         ui::Vec4{0.7f, 0.7f, 0.7f, 1.0f});

        // ---- 左侧黑侧栏（菜单由 NavMenu 子控件绘制，覆盖其上） ----
        renderer.draw_quad({x0, y0 + header_h, sidebar_w, h - header_h - footer_h}, dark);

        // ---- 底部固定区域 ----
        renderer.draw_quad({x0, y0 + h - footer_h, w, footer_h}, ui::Vec4{0.93f, 0.93f, 0.93f, 1.0f});
        font_->draw_text(renderer, "底部固定区域", x0 + 16.f,
                         y0 + h - footer_h + (footer_h - font_->line_height()) * 0.5f,
                         ui::Vec4{0.4f, 0.4f, 0.4f, 1.0f});

        // ---- 内容主体：由 body_（ScrollBox layui-body 白底 + 滚动）子控件绘制 ----
        (void)white;
    }

    bool on_mouse_move(float x, float y) override {
        hover_nav_ = -1;
        if (!contains(x, y) || y < bounds_.y || y >= bounds_.y + kHeaderH) return false;
        for (int i = 0; i < (int)navs_.size(); ++i) {
            const HeaderItem& ni = navs_[i];
            if (x >= ni.x && x <= ni.x + ni.w) {
                hover_nav_ = i;
                break;
            }
        }
        return false;
    }

    bool on_mouse_down(float x, float y, int button) override {
        if (button != 0 || !contains(x, y)) return false;
        if (y >= bounds_.y && y < bounds_.y + kHeaderH) {
            for (int i = 0; i < (int)navs_.size(); ++i) {
                const HeaderItem& ni = navs_[i];
                if (x >= ni.x && x <= ni.x + ni.w) {
                    if (i == 3) {
                        toggle_nav_groups();  // "nav groups"：折叠菜单（menu 11/22/33）
                    } else {
                        close_popups();  // 关闭 nav groups / 用户区弹出
                        active_nav_ = i;  // nav 1/2/3：点击高亮
                        body_->reset_scroll();  // 内容主体回到顶部（对齐站内导航语义）
                    }
                    return true;
                }
            }
            if (user_trigger_ && user_trigger_->contains(x, y)) {
                toggle_user_menu();  // 用户区：头像 + tester + 箭头（UI-049）
                return true;
            }
        }
        return false;
    }

private:
    static constexpr float kHeaderH = 40.f;
    static constexpr float kSidebarW = 170.f;
    static constexpr const char* kUserName = "tester";
    static constexpr const char* kCaret = "v";

    struct HeaderItem {
        std::string text;
        float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
    };

    void toggle_user_menu() {
        if (user_menu_->visible) {
            close_popups();
            return;
        }
        close_popups();  // 关闭其它弹出（nav groups 折叠菜单）
        const float popup_w = 120.f;
        user_menu_->set_bounds(user_rect_.x + user_rect_.w - popup_w, user_rect_.y + user_rect_.h,
                               popup_w, user_menu_->preferred_height());
        if (!user_menu_->parent()) root()->add_child(user_menu_);
        open_popup(user_trigger_, user_menu_);
    }

    // 头部 "nav groups" 折叠菜单：展开/关闭（对齐 layout-admin.html layui-nav-child）
    void toggle_nav_groups() {
        if (nav_groups_menu_->visible) {
            close_popups();
            return;
        }
        close_popups();  // 关闭其它弹出（用户菜单）
        const float popup_w = 110.f;
        nav_groups_menu_->set_bounds(navs_[3].x, bounds_.y + kHeaderH, popup_w,
                                     nav_groups_menu_->preferred_height());
        if (!nav_groups_menu_->parent()) root()->add_child(nav_groups_menu_);
        open_popup(nav_groups_trigger_, nav_groups_menu_);
    }

    ui::Font* font_ = nullptr;
    const ui::Theme* theme_ = nullptr;
    ui::Image* avatar_ = nullptr;
    ui::NavMenu* menu_ = nullptr;
    ui::Widget* user_trigger_ = nullptr;
    ui::VBox* user_menu_ = nullptr;
    ui::ScrollBox* body_ = nullptr;
    BodyContent* body_content_ = nullptr;
    ui::Widget* nav_groups_trigger_ = nullptr;
    ui::VBox* nav_groups_menu_ = nullptr;
    std::vector<HeaderItem> navs_;
    ui::Rect user_rect_{0.f, 0.f, 0.f, 0.f};
    float user_name_x_ = 0.f;
    float user_caret_x_ = 0.f;
    int active_nav_ = 0;
    int hover_nav_ = -1;
};

// 头像来源：layui demo 的 avatar/*.jpg 实为 HTML 404 页（demo 站点死链），
// 取官方镜像资源库 3rdparty/src/layui/static/images/layui/logo.png
//（POST_BUILD 已拷贝至 exe 目录；Image::load_from_file 裸文件名回退 exe 目录，UI-048）
bool load_avatar_file(ui::Image* img) {
    return img->load_from_file("logo.png");
}

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

void build_frame(PageHost& host) {
    add_section(host, "管理界面大布局（layui-layout-admin 克隆）");
    AdminFrame* frame = new AdminFrame(800, 560);
    frame->set_font(host.font);
    frame->set_theme(host.theme);
    frame->setup_menu();          // 侧栏折叠菜单（NavMenu：点一级组展开/折叠 + 子项选中态）
    frame->setup_user_menu();     // 用户区下拉（Profile/Settings/Sign out）
    frame->setup_nav_groups_menu();  // 头部 "nav groups" 折叠菜单（menu 11/22/33）
    if (!load_avatar_file(frame->avatar())) {
        ui::log_error("frame page: avatar load failed");
    }
    host.content->add_child(frame);

    add_section(host, "说明");
    ui::Label* note = new ui::Label(0, 0, 800, 0,
                                    "本应用外壳即为此布局（顶部导航 + 侧栏 + 内容区），此处为页面内的迷你复刻演示。"
                                    "迷你框架可交互：侧栏菜单点一级组折叠/展开、点子项选中（绿底白字），"
                                    "头部导航 hover 变绿、点击高亮，\"nav groups\" 弹出折叠菜单（menu 11/22/33），"
                                    "右上角用户区弹出菜单，内容主体可滚动。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/frame_page.cpp（节选）：克隆 layui layout-admin.html\n"
            "\n"
            "// 自绘 AdminFrame：黑头 logo + 左导航 + 用户区（头像+tester+箭头）、黑侧栏折叠菜单、白内容可滚动、灰底\n"
            "AdminFrame* frame = new AdminFrame(800, 560);\n"
            "frame->set_font(host.font);\n"
            "frame->set_theme(host.theme);\n"
            "frame->setup_menu();              // ui::NavMenu：点击一级组展开/折叠 + set_selected 选中态\n"
            "frame->setup_user_menu();         // 用户区下拉菜单（open_popup + VBox）\n"
            "frame->setup_nav_groups_menu();   // 头部 nav groups 折叠菜单（menu 11/22/33）\n"
            "load_avatar_file(frame->avatar());   // ui::Image 加载头像（stb_image 解码）\n"
            "host.content->add_child(frame);\n"
            "// 内容主体 = ui::ScrollBox(layui-body) + 高内容控件：滚轮/拖动滚动条可滚动\n"
            "// 对应布局类：layui-layout-admin / layui-header / layui-side / layui-body / layui-footer");
    }
    if (host.help) {
        const char* lines[] = {
            "后台布局（layout-admin 克隆）",
            "  结构：layui-header（logo + 水平导航）+ layui-side（垂直导航树）+ layui-body + layui-footer",
            "  本页为预览区内的迷你复刻；应用自身外壳即完整版该布局",
            "  交互：侧栏菜单点一级组展开/折叠、点子项选中（NavMenu 选中态绿底白字）、头部导航 hover 变绿/点击高亮、",
            "        \"nav groups\" 点击弹出折叠菜单（menu 11/22/33）、右上角用户区（头像 + tester + 箭头）弹出菜单、",
            "        内容主体为 layui-body 可滚动区（滚轮 / 拖动滚动条）",
            "  移动端（<768px）：侧栏隐藏、header 收缩（本复刻固定桌面尺寸）",
        };
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            label->set_font(host.font);
            label->set_theme(host.theme);
            label->set_color(host.theme->text);
            host.help->add_child(label);
        }
    }
}

}  // namespace

static void build(PageHost& host) {
    build_frame(host);
}

static const PageDef kFramePage{0, "布局", "框架", "管理界面大布局（mini 复刻）", build, nullptr, false, nullptr, 0, "Layout"};

static const bool kRegistered = []() {
    page::register_page(kFramePage);
    return true;
}();
