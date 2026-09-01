// api_demo：opengl_ui 全部公开 API 总调用示例（docs/UI_API.md 的可执行伴随程序 / 冒烟测试）。
//
// 覆盖范围：
//   - core  ：keycodes / log / math / version
//   - gl    ：window / renderer / texture / font（含逐帧原始绘制样例）
//   - ui/ui ：Widget 基类 + Theme + layui_style/icons + 全部控件
//   - media ：(FFmpeg) AudioOut / VideoView，(OpenCV) ImageView（条件编译）
//
// 每条公开 API 至少调用一次。通过聚合头 #include "ui/all.h" 拉入全部 ui 公开头。
// 用法：api_demo [--smoke <秒>] [--file <视频路径>]（--smoke 运行指定秒后自动退出，供 CI）。
#include "ui/all.h"

#if defined(OPENGUI_MEDIA_FFMPEG)
#include "ui/media/audio_out.h"
#include "ui/media/video_view.h"
#endif
#if defined(OPENGUI_MEDIA_OPENCV)
#include "ui/media/image_view.h"
#endif

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

// 素材定位：优先工作目录（POST_BUILD 已把素材拷到可执行目录），否则回退源码 assets/
static bool load_media_file(const char* name, std::function<bool(const char*)> loader) {
    char in_cwd[256], in_assets[256];
    std::snprintf(in_cwd, sizeof(in_cwd), "%s", name);
    std::snprintf(in_assets, sizeof(in_assets), "assets/%s", name);
    return loader(in_cwd) || loader(in_assets);
}

static ui::Label* caption(ui::VBox& box, ui::Font* font, const char* text) {
    ui::Label* l = new ui::Label(0, 0, 1220, 0, text);
    l->set_font(font);
    l->set_align(0);
    l->set_color(ui::Vec4{0.45f, 0.85f, 1.0f, 1.0f});
    box.add_child(l);
    return l;
}

static void exercise_core() {
    ui::log_info("opengl_ui %s", ui::version_string());
    ui::Vec2 p{10.f, 20.f};
    ui::Vec4 c{1.f, 0.f, 0.f, 0.5f};
    ui::Rect r{0.f, 0.f, 100.f, 80.f};
    ui::log_info("api_demo: vec2(%.0f,%.0f) vec4(%.1f,%.1f) rect(%.0fx%.0f @%.0f,%.0f) mod=%d",
                 p.x, p.y, c.r, c.g, r.w, r.h, r.x, r.y, (int)ui::KeyMod::ModShift);
    ui::log_info("api_demo: keys up/down/enter = %d/%d/%d", (int)ui::KeyUp, (int)ui::KeyDown,
                 (int)ui::KeyEnter);
}

static void exercise_gl_texture(ui::Texture& tex) {
    unsigned char rgba[16 * 16 * 4];
    for (int i = 0; i < 16 * 16; ++i) {
        rgba[i * 4 + 0] = (unsigned char)((i / 16) * 16);
        rgba[i * 4 + 1] = (unsigned char)((i % 16) * 16);
        rgba[i * 4 + 2] = 64;
        rgba[i * 4 + 3] = 255;
    }
    tex.create(16, 16, rgba);
    tex.update(rgba);
    tex.update_sub(0, 0, 16, 16, rgba);
    tex.bind(0);
    ui::log_info("api_demo: texture %u %dx%d", tex.id(), tex.width(), tex.height());
    load_media_file("test.png", [&tex](const char* p) { return tex.load_from_file(p); });
}

// 逐帧 gl 原始绘制样例：quad / rounded / outline / polygon / scissor / textured
static void draw_gl_samples(ui::Renderer& renderer, const ui::Texture& tex, const ui::Theme& theme,
                            float t) {
    renderer.draw_quad(ui::Rect{1040.f, 60.f, 200.f, 90.f}, ui::Vec4{0.25f, 0.30f, 0.36f, 1.f});
    renderer.draw_rounded_rect(ui::Rect{1040.f, 170.f, 200.f, 60.f}, 12.f, theme.accent);
    renderer.draw_rect_outline(ui::Rect{1040.f, 250.f, 200.f, 60.f}, 2.f, theme.border);
    const ui::Vec2 poly[3] = {{1060.f, 360.f}, {1180.f, 360.f}, {1120.f, 410.f}};
    renderer.draw_polygon(poly, 3, theme.warning);
    renderer.push_scissor(ui::Rect{1040.f, 440.f, 200.f, 120.f});
    renderer.draw_quad(ui::Rect{1030.f, 430.f, 220.f, 140.f}, theme.success);
    renderer.pop_scissor();
    const float pulse = 0.55f + 0.45f * (t - (int)t);
    renderer.draw_textured_quad(ui::Rect{1040.f, 590.f, 200.f, 200.f}, tex.id(),
                                ui::Rect{0.f, 0.f, 1.f, 1.f},
                                ui::Vec4{pulse, pulse, pulse, 1.f});
}

int main(int argc, char** argv) {
    double smoke_seconds = 0.0;
    const char* media_file = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--smoke") == 0 && i + 1 < argc)
            smoke_seconds = std::atof(argv[++i]);
        else if (std::strcmp(argv[i], "--file") == 0 && i + 1 < argc)
            media_file = argv[++i];
    }
    exercise_core();

    ui::Window window(1280, 1280, "opengl_ui api_demo");
    if (!window.handle()) {
        ui::log_error("api_demo: no GL context");
        return 1;
    }
    window.set_vsync(1);

    ui::Renderer renderer;
    if (!renderer.init()) {
        ui::log_error("api_demo: renderer init failed");
        return 1;
    }
    renderer.set_viewport(window.width(), window.height());

    ui::Font font;
    if (!font.load_from_file("C:/Windows/Fonts/arial.ttf", 20.f))
        ui::log_error("api_demo: font load failed");
    if (!font.load_cjk_font_auto())
        ui::log_error("api_demo: cjk font load failed");
    if (!font.load_icon_font("assets/iconfont.ttf", 20.f) &&
        !font.load_icon_font("iconfont.ttf", 20.f))
        ui::log_error("api_demo: icon font load failed");
    ui::log_info("api_demo: font metrics asc=%.1f desc=%.1f line=%.1f px=%.1f cjk=%d icon=%d",
                 font.ascent(), font.descent(), font.line_height(), font.pixel_height(),
                 font.cjk_ready() ? 1 : 0, font.icon_loaded() ? 1 : 0);

    ui::Texture tex;
    exercise_gl_texture(tex);

    ui::Theme theme;

    // ui::Scene：统一驱动生命周期（生产者→分配者→消费者→清理重置）
    ui::Scene scene(window, renderer);
    scene.set_root_background(theme.window_bg);

    // 严格不回归：api_demo 迁移前仅 Tab/方向+Enter+Backspace+chars，无 Space/Delete/Home/End
    scene.dispatch_nav_keys = false;
    scene.set_event_hook(ui::SceneEvent::Key, [&](ui::Scene& s) {
        ui::Widget& r = s.root();
        const int k = s.event_key();
        if (k == ui::KeyTab || k == ui::KeyUp || k == ui::KeyDown || k == ui::KeyLeft ||
            k == ui::KeyRight)
            r.dispatch_key_down(k, window.key_down(ui::KeyLeftShift) ? ui::ModShift : 0);
        else if (k == ui::KeyEnter || k == ui::KeyBackspace)
            r.dispatch_key_down(k, 0);
        return true;  // 已消费（模板关闭，仅此钩子驱动原始键集）
    });

    ui::Widget& root = scene.root();

    ui::ScrollBox scroller(0, 0, 1024, (float)window.height());
    scroller.set_theme(&theme);
    scroller.set_padding(8.f);
    scroller.set_spacing(4.f);
    scroller.set_scroll_step(28.f);
    root.add_child(&scroller);

    ui::VBox* box = new ui::VBox(0, 0, 1008, 0);
    box->set_theme(&theme);
    box->set_spacing(6.f);
    box->set_padding(8.f);
    scroller.add_child(box);

    // 主循环要驱动 advance(dt) 的控件（数据杂项组 + media），提升到外层作用域
    ui::Carousel* carousel = nullptr;
    ui::Upload* upload = nullptr;
#if defined(OPENGUI_MEDIA_FFMPEG)
    ui::VideoView* video = nullptr;
#endif

    // ---- core / version ----
    caption(*box, &font, "== core: math / keycodes / log / version ==");

    // ---- Theme + layui_style + layui_icons ----
    caption(*box, &font, "== theme / layui_style / layui_icons ==");
    {
        ui::Theme dark = ui::Theme::make_dark();
        ui::Theme light = ui::Theme::make_light();
        ui::Theme layui = ui::Theme::make_layui();
        (void)dark;
        (void)light;
        (void)layui;
        const ui::Vec4 bg = ui::layui::bg_color("layui-btn");
        const ui::Vec4 fill = ui::layui::bg_fill("layui-bg-blue");
        const ui::Vec4 col = ui::layui::hex("#ff6600");
        ui::log_info("api_demo: style bg=(%.2f) fill=(%.2f) hex=(%.2f,%.2f,%.2f) class=%d", bg.r,
                     fill.r, col.r, col.g, col.b, ui::layui::has_class(&root, "layui-row") ? 1 : 0);
        ui::Button btn_cls(0, 0, 300, 34, "style probe");
        btn_cls.add_class("layui-btn layui-bg-blue");
        ui::log_info("api_demo: has_class=%d classes=%zu from_class=%08x",
                     btn_cls.has_class("layui-bg-blue") ? 1 : 0, btn_cls.classes().size(),
                     (unsigned)ui::layui_icon::from_class("star"));
        btn_cls.remove_class("layui-bg-blue");
        ui::log_info("api_demo: icons star=%04X set=%04X ok=%04X", (unsigned)ui::layui_icon::star,
                     (unsigned)ui::layui_icon::set, (unsigned)ui::layui_icon::ok);
    }

    // ---- Widget 基类 API ----
    caption(*box, &font, "== Widget 基类（bounds/class/子树/命中/捕获/焦点/tooltip/弹窗/模态）==");
    {
        ui::Widget* w = new ui::Widget(0, 0, 300, 60);
        w->set_bounds(10.f, 10.f, 300.f, 60.f);
        const ui::Rect& b = w->bounds();
        w->add_class("probe");
        ui::log_info("api_demo: widget b=(%.0f,%.0f %.0fx%.0f) class=%d visible=%d contains=%d",
                     b.x, b.y, b.w, b.h, w->has_class("probe") ? 1 : 0, w->visible ? 1 : 0,
                     w->contains(20.f, 20.f) ? 1 : 0);
        ui::Widget* child = new ui::Widget(0, 0, 100, 20);
        w->add_child(child);
        w->add_child(new ui::Widget(0, 0, 100, 20));
        w->bring_to_front();
        ui::log_info("api_demo: parent=%d children=%zu", w->parent() == nullptr ? 1 : 0,
                     w->children().size());
        w->remove_child(child);
        delete child;
        w->clear_children();
        w->capture_mouse();
        w->release_mouse_capture();
        w->set_tooltip("widget base api probe");
        ui::log_info("api_demo: tooltip=%s hovered=%d pressed=%d", w->tooltip().c_str(),
                     w->hovered() ? 1 : 0, w->pressed() ? 1 : 0);
        w->focusable = true;
        w->focus();
        w->blur();
        w->focus_next();
        w->focus_prev();
        w->set_modal(nullptr);
        ui::log_info("api_demo: focused=%d modal=%p root=%p", w->focused() ? 1 : 0, (void*)w->modal(),
                     (void*)w->root());
        w->open_popup(nullptr, nullptr);
        ui::log_info("api_demo: open_popups=%d", w->has_open_popups() ? 1 : 0);
        w->close_popups();
        w->destroy();  // 堆对象，整树销毁（此后不得再访问）
    }

    // ---- 基础控件 ----
    caption(*box, &font, "== Button / CheckBox / RadioButton / RadioGroup / Switch / Slider / Progress / TextBox / Label / Icon ==");
    {
        ui::Button* btn = new ui::Button(0, 0, 300, 34, "click me");
        btn->set_font(&font);
        btn->set_theme(&theme);
        btn->set_on_click([]() { ui::log_info("api_demo: button clicked"); });
        btn->set_tooltip("a button");
        box->add_child(btn);

        ui::CheckBox* cb = new ui::CheckBox(0, 0, 300, 24, "checkbox");
        cb->set_font(&font);
        cb->set_theme(&theme);
        cb->set_checked(true);
        cb->toggle();
        cb->set_on_change([](bool v) { ui::log_info("api_demo: checkbox %d", v); });
        ui::log_info("api_demo: checkbox checked=%d", cb->checked() ? 1 : 0);
        box->add_child(cb);

        ui::RadioButton* rb = new ui::RadioButton(0, 0, 300, 24, "radio button");
        rb->set_font(&font);
        rb->set_theme(&theme);
        rb->set_checked(true);
        rb->set_on_activate([]() { ui::log_info("api_demo: radio activated"); });
        box->add_child(rb);

        ui::RadioGroup* rg = new ui::RadioGroup(0, 0, 300, 90);
        rg->set_font(&font);
        rg->set_theme(&theme);
        rg->add_option("one");
        rg->add_option("two");
        rg->add_option("three");
        rg->select(1);
        rg->set_option_height(26.f);
        rg->set_spacing(4.f);
        rg->set_on_change([](int i) { ui::log_info("api_demo: radiogroup %d", i); });
        ui::log_info("api_demo: radiogroup selected=%d", rg->selected());
        box->add_child(rg);

        ui::Switch* sw = new ui::Switch(0, 0, 80, 28);
        sw->set_font(&font);
        sw->set_theme(&theme);
        sw->set_text("ON");
        sw->set_checked(true);
        sw->set_on_change([](bool v) { ui::log_info("api_demo: switch %d", v); });
        ui::log_info("api_demo: switch text=%s checked=%d", sw->text().c_str(),
                     sw->checked() ? 1 : 0);
        box->add_child(sw);

        ui::Slider* slider = new ui::Slider(0, 0, 300, 22);
        slider->set_theme(&theme);
        slider->set_range(0.f, 100.f);
        slider->set_value(42.f);
        slider->set_on_change([](float v) { ui::log_info("api_demo: slider %.1f", v); });
        ui::log_info("api_demo: slider value=%.1f dragging=%d", slider->value(),
                     slider->dragging() ? 1 : 0);
        slider->set_value_silent(50.f);
        box->add_child(slider);

        ui::Progress* prog = new ui::Progress(0, 0, 300, 14);
        prog->set_font(&font);
        prog->set_theme(&theme);
        prog->set_show_text(true);
        prog->set_value(0.6f);
        ui::log_info("api_demo: progress value=%.2f", prog->value());
        box->add_child(prog);

        ui::TextBox* input = new ui::TextBox(0, 0, 300, 36);
        input->set_font(&font);
        input->set_theme(&theme);
        input->set_text("hello");
        input->set_on_change([](const char* s) { ui::log_info("api_demo: textbox '%s'", s); });
        input->set_on_enter([]() { ui::log_info("api_demo: textbox enter"); });
        ui::log_info("api_demo: textbox text=%s", input->text().c_str());
        box->add_child(input);

        ui::Label* label = new ui::Label(0, 0, 300, 0,
                                         "Label with wrap; 支持自动换行的多行文本 Label demo.");
        label->set_font(&font);
        label->set_theme(&theme);
        label->set_wrap(true);
        label->set_color(ui::Vec4{1.f, 1.f, 1.f, 1.f});
        label->set_align(1);
        ui::log_info("api_demo: label text=%s wrap=%d", label->text().c_str(),
                     label->wrap() ? 1 : 0);
        label->set_text("Label updated");
        box->add_child(label);

        ui::Icon* icon = new ui::Icon(0, 0, 28, 28, "star");
        icon->set_font(&font);
        icon->set_theme(&theme);
        icon->set_color(ui::Vec4{1.f, 0.8f, 0.2f, 1.f});
        icon->set_icon("set");
        box->add_child(icon);
    }

    // ---- 布局容器 ----
    caption(*box, &font, "== HBox / VBox / Grid / Panel / Card / PanelWindow / Fieldset / HLine / BlockQuote / ScrollBox ==");
    {
        ui::HBox* hrow = new ui::HBox(0, 0, 300, 34);
        hrow->set_theme(&theme);
        hrow->set_spacing(8.f);
        hrow->set_padding(0.f);
        ui::Button* hb1 = new ui::Button(0, 0, 140, 34, "h-left");
        hb1->set_font(&font);
        hb1->set_theme(&theme);
        hrow->add_child(hb1);
        ui::Button* hb2 = new ui::Button(0, 0, 140, 34, "h-right");
        hb2->set_font(&font);
        hb2->set_theme(&theme);
        hrow->add_child(hb2);
        box->add_child(hrow);

        ui::VBox* nested = new ui::VBox(0, 0, 300, 0);
        nested->set_theme(&theme);
        nested->set_spacing(4.f);
        nested->set_padding(4.f);
        for (int i = 0; i < 3; ++i) {
            ui::Button* nb = new ui::Button(0, 0, 0, 26, "nested");
            nb->set_font(&font);
            nb->set_theme(&theme);
            nested->add_child(nb);
        }
        box->add_child(nested);

        ui::Grid* grid = new ui::Grid(0, 0, 300, 0, 3);
        grid->set_theme(&theme);
        grid->set_cols(3);
        grid->set_row_height(26.f);
        grid->set_spacing(4.f);
        grid->set_padding(2.f);
        grid->set_span_cols(1);
        grid->set_breakpoint(0);
        for (int i = 0; i < 6; ++i) {
            ui::Button* cell = new ui::Button(0, 0, 0, 0, "cell");
            cell->set_font(&font);
            cell->set_theme(&theme);
            grid->add_child(cell);
        }
        box->add_child(grid);

        ui::Panel* panel = new ui::Panel(0, 0, 300, 40);
        panel->set_theme(&theme);
        box->add_child(panel);

        ui::Card* card = new ui::Card(0, 0, 300, 60);
        card->set_font(&font);
        card->set_theme(&theme);
        card->set_header("Card");
        card->set_spacing(6.f);
        ui::log_info("api_demo: card header=%s", card->header().c_str());
        ui::Button* card_btn = new ui::Button(0, 0, 0, 26, "in card");
        card_btn->set_font(&font);
        card_btn->set_theme(&theme);
        card->add_child(card_btn);
        box->add_child(card);

        ui::PanelWindow* win = new ui::PanelWindow(0, 0, 300, 120, "PanelWindow");
        win->set_font(&font);
        win->set_theme(&theme);
        win->set_title("Window title");
        win->set_on_close([]() { ui::log_info("api_demo: window close"); });
        const ui::Rect cr = win->client_rect();
        ui::log_info("api_demo: client_rect %.0fx%.0f", cr.w, cr.h);
        ui::Button* win_btn = new ui::Button(0, 0, 0, 26, "in window");
        win_btn->set_font(&font);
        win_btn->set_theme(&theme);
        win->add_child(win_btn);
        box->add_child(win);

        ui::Fieldset* fs = new ui::Fieldset(300, 40, "Fieldset");
        fs->set_font(&font);
        fs->set_theme(&theme);
        box->add_child(fs);
        ui::FieldsetTitle* fst = new ui::FieldsetTitle(300, 24, "FieldsetTitle");
        fst->set_font(&font);
        fst->set_theme(&theme);
        box->add_child(fst);

        ui::HLine* hline = new ui::HLine(300, 2, ui::Vec4{0.4f, 0.4f, 0.45f, 1.f});
        box->add_child(hline);

        ui::BlockQuote* bq = new ui::BlockQuote(300, 40, "BlockQuote text", false);
        bq->set_font(&font);
        bq->set_theme(&theme);
        bq->set_text("BlockQuote updated");
        box->add_child(bq);

        ui::ScrollBox* inner = new ui::ScrollBox(0, 0, 300, 60);
        inner->set_theme(&theme);
        inner->set_spacing(2.f);
        inner->set_padding(2.f);
        ui::VBox* inner_list = new ui::VBox(0, 0, 0, 0);
        inner_list->set_theme(&theme);
        for (int i = 0; i < 8; ++i) {
            ui::Label* item = new ui::Label(0, 0, 0, 0, "scroll item");
            item->set_font(&font);
            item->set_theme(&theme);
            inner_list->add_child(item);
        }
        inner->add_child(inner_list);
        inner->set_on_scroll_to_bottom([]() { ui::log_info("api_demo: scroll bottom"); });
        ui::log_info("api_demo: scroll offset=%.1f", inner->scroll_offset());
        box->add_child(inner);
    }

    // ---- 导航 / 折叠 ----
    caption(*box, &font, "== MenuBar / NavMenu / Breadcrumb / Tabs / Dropdown / Collapse / Tree ==");
    {
        ui::MenuBar* menubar = new ui::MenuBar(0, 0, 300, 28);
        menubar->set_font(&font);
        menubar->set_theme(&theme);
        menubar->add_menu("File");
        menubar->add_item("Exit", []() { ui::log_info("api_demo: menu exit"); });
        menubar->add_menu("Help");
        menubar->add_item("About", []() { ui::log_info("api_demo: menu about"); });
        box->add_child(menubar);

        ui::NavMenu* nav = new ui::NavMenu(0, 0, 300, 110);
        nav->set_font(&font);
        nav->set_theme(&theme);
        nav->add_group("group a");
        nav->add_item("item a1", []() { ui::log_info("api_demo: nav a1"); });
        nav->add_item("item a2");
        nav->add_group("group b");
        nav->add_item("item b1");
        nav->set_active(0);
        nav->expand_all();
        box->add_child(nav);

        ui::Breadcrumb* crumb = new ui::Breadcrumb(0, 0, 300, 0);
        crumb->set_font(&font);
        crumb->set_theme(&theme);
        crumb->add_item("Home");
        crumb->add_item("UI");
        crumb->add_current("API");
        box->add_child(crumb);

        ui::Tabs* tabs = new ui::Tabs(0, 0, 300, 70);
        tabs->set_font(&font);
        tabs->set_theme(&theme);
        tabs->add_tab("one");
        tabs->add_tab("two");
        tabs->set_active(1);
        tabs->set_on_change([](int i) { ui::log_info("api_demo: tabs %d", i); });
        ui::log_info("api_demo: tabs active=%d", tabs->active());
        box->add_child(tabs);

        ui::Dropdown* dropdown = new ui::Dropdown(0, 0, 160, 30, "menu");
        dropdown->set_font(&font);
        dropdown->set_theme(&theme);
        dropdown->add_item("copy", []() { ui::log_info("api_demo: dropdown copy"); });
        dropdown->add_separator();
        dropdown->add_item("paste");
        box->add_child(dropdown);

        ui::Collapse* collapse = new ui::Collapse(0, 0, 300, 60);
        collapse->set_font(&font);
        collapse->set_theme(&theme);
        collapse->add_item("question?");
        ui::Button* clp_btn = new ui::Button(0, 0, 0, 26, "answer");
        clp_btn->set_font(&font);
        clp_btn->set_theme(&theme);
        collapse->add_child(clp_btn);
        collapse->set_active(0);
        collapse->set_on_change([](int i) { ui::log_info("api_demo: collapse %d", i); });
        ui::log_info("api_demo: collapse active=%d", collapse->active());
        box->add_child(collapse);

        ui::Tree* tree = new ui::Tree(0, 0, 300, 100);
        tree->set_font(&font);
        tree->set_theme(&theme);
        const int n_root = tree->add_node("root");
        const int n_a = tree->add_node("a", n_root);
        tree->add_node("a1", n_a);
        tree->set_node_text(n_a, "a*");
        tree->set_expanded(n_root, true);
        tree->set_on_click([](int i) { ui::log_info("api_demo: tree %d", i); });
        ui::log_info("api_demo: tree expanded=%d", tree->is_expanded(n_root) ? 1 : 0);
        tree->expand_all(false);
        tree->expand_all(true);
        box->add_child(tree);
    }

    // ---- 数据 / 杂项 ----
    caption(*box, &font, "== Table / Transfer / Timeline / Laypage / Laydate / Upload / Carousel / Code / ColorPicker / CountdownText / Badge / Circle / Rate ==");
    {
        ui::Table* table = new ui::Table(0, 0, 300, 120);
        table->set_font(&font);
        table->set_theme(&theme);
        table->add_column("name", 120, true);
        table->add_column("type", 90, false);
        table->add_row({"glfw", "core"});
        table->add_row({"ffmpeg", "media"});
        table->set_on_sort([](int col, bool asc) { ui::log_info("api_demo: sort %d %d", col, asc); });
        table->set_on_row_click([](int row) { ui::log_info("api_demo: row %d", row); });
        ui::log_info("api_demo: table cols=%d w0=%.0f sel=%d", table->col_count(),
                     table->col_width(0), table->selected_row());
        box->add_child(table);
        table->clear_rows();

        ui::Transfer* transfer = new ui::Transfer(0, 0, 300, 110);
        transfer->set_font(&font);
        transfer->set_theme(&theme);
        transfer->set_left_label("left");
        transfer->set_right_label("right");
        transfer->add_item("one");
        transfer->add_item("two");
        transfer->set_on_change([](ui::TransferDirection dir, const std::vector<std::string>& v) {
            ui::log_info("api_demo: transfer %s %zu items",
                         dir == ui::TransferDirection::ToRight ? "->right" : "<-left", v.size());
        });
        ui::log_info("api_demo: transfer right=%zu", transfer->right_items().size());
        box->add_child(transfer);

        ui::Timeline* tl = new ui::Timeline(0, 0, 300, 0);
        tl->set_font(&font);
        tl->set_theme(&theme);
        tl->add_item("2026-08-16", "api_demo written");
        box->add_child(tl);

        ui::Laypage* laypage = new ui::Laypage(0, 0, 300, 26);
        laypage->set_font(&font);
        laypage->set_theme(&theme);
        laypage->set_total(50);
        laypage->set_page(2);
        laypage->set_on_change([](int p) { ui::log_info("api_demo: laypage %d", p); });
        ui::log_info("api_demo: laypage page=%d", laypage->page());
        box->add_child(laypage);

        ui::Laydate* laydate = new ui::Laydate(0, 0, 300, 40);
        laydate->set_font(&font);
        laydate->set_theme(&theme);
        laydate->set_date(2026, 8, 16);
        laydate->next_month();
        laydate->prev_month();
        laydate->set_on_change([](int y, int m, int d) {
            ui::log_info("api_demo: laydate %d-%02d-%02d", y, m, d);
        });
        box->add_child(laydate);

        ui::Upload* upload_panel = new ui::Upload(0, 0, 300, 90);
        upload_panel->set_font(&font);
        upload_panel->set_theme(&theme);
        upload_panel->on_pick = []() { ui::log_info("api_demo: upload pick"); };
        upload_panel->add_file("file_a.bin", 1024 * 64);
        upload_panel->add_file("file_b.bin", 1024 * 256);
        upload = upload_panel;
        box->add_child(upload_panel);

        ui::Carousel* carousel_slide = new ui::Carousel(0, 0, 300, 90);
        carousel_slide->set_font(&font);
        carousel_slide->set_theme(&theme);
        carousel_slide->add_slide(ui::Vec4{0.12f, 0.62f, 1.f, 1.f}, "slide 1");
        carousel_slide->add_slide(ui::Vec4{0.09f, 0.72f, 0.47f, 1.f}, "slide 2");
        carousel_slide->set_auto_play(true, 2.f);
        carousel_slide->set_on_change([](int i) { ui::log_info("api_demo: carousel %d", i); });
        carousel_slide->next();
        carousel_slide->prev();
        ui::log_info("api_demo: carousel count=%d current=%d", carousel_slide->count(),
                     carousel_slide->current());
        carousel = carousel_slide;
        box->add_child(carousel);

        ui::Code* code = new ui::Code(0, 0, 300, 70);
        code->set_font(&font);
        code->set_theme(&theme);
        code->set_lang("cpp");
        code->set_code("int main() {}");
        code->add_line("// more");
        ui::log_info("api_demo: code empty=%d", code->empty() ? 1 : 0);
        box->add_child(code);
        code->clear();

        ui::ColorPicker* picker = new ui::ColorPicker(0, 0, 300, 60);
        picker->set_font(&font);
        picker->set_theme(&theme);
        picker->set_color(ui::Vec4{1.f, 0.4f, 0.1f, 1.f});
        picker->set_on_change([](const ui::Vec4& c) {
            ui::log_info("api_demo: color %.2f,%.2f,%.2f", c.r, c.g, c.b);
        });
        ui::log_info("api_demo: color r=%.2f", picker->color().r);
        box->add_child(picker);

        ui::CountdownText* countdown = new ui::CountdownText(0, 0, 300, 0);
        countdown->set_font(&font);
        countdown->set_theme(&theme);
        countdown->set_total(10.f);
        countdown->restart();
        box->add_child(countdown);

        ui::Badge* badge = new ui::Badge(0, 0, "new");
        badge->set_font(&font);
        badge->set_theme(&theme);
        badge->set_text("updated");
        ui::log_info("api_demo: badge text=%s", badge->text().c_str());
        box->add_child(badge);

        ui::Circle* circle = new ui::Circle(40, "50%");
        circle->set_font(&font);
        circle->set_color(ui::Vec4{0.2f, 0.8f, 0.5f, 1.f});
        box->add_child(circle);

        ui::Rate* rate = new ui::Rate(0, 0, 300, 22);
        rate->set_theme(&theme);
        rate->set_value(4);
        rate->set_readonly(false);
        rate->set_on_change([](int v) { ui::log_info("api_demo: rate %d", v); });
        ui::log_info("api_demo: rate value=%d readonly=%d", rate->value(), rate->readonly() ? 1 : 0);
        box->add_child(rate);
    }

#if defined(OPENGUI_MEDIA_FFMPEG)
    caption(*box, &font, "== media/ffmpeg: AudioOut + VideoView ==");
    {
        // AudioOut（全局命名空间）：16bit PCM 输出
        AudioOut ao;
        if (ao.open(44100, 2)) {
            const std::int16_t silence[512] = {0};
            ao.write(silence, 512);
            ao.pause();
            ao.resume();
            ao.flush();
            ui::log_info("api_demo: audio open");
        } else {
            ui::log_info("api_demo: audio device unavailable (silent)");
        }
        ao.close();
        ui::log_info("api_demo: audio is_open=%d", ao.is_open() ? 1 : 0);

        ui::VideoView* video_view = new ui::VideoView();
        video_view->set_bounds(0, 0, 300, 170);
        bool ok = false;
        if (media_file) {
            ok = video_view->open_source(media_file);
        } else {
            ok = load_media_file("test_h264.mp4",
                                 [video_view](const char* p) { return video_view->open_source(p); });
            if (!video_view->has_video())
                ok = load_media_file("test.mp4",
                                     [video_view](const char* p) { return video_view->open_source(p); });
        }
        ui::log_info("api_demo: video open=%d has_video=%d has_audio=%d dur=%.1fs", ok ? 1 : 0,
                     video_view->has_video() ? 1 : 0, video_view->has_audio() ? 1 : 0,
                     video_view->duration());
        video_view->set_loop(true);
        video_view->set_speed(1.f);
        video_view->set_volume(0.8f);
        video_view->play();
        video_view->set_playing(true);
        video_view->step_frame();
        video_view->seek(0.f);
        ui::log_info("api_demo: video playing=%d loop=%d speed=%.1f vol=%.1f pos=%.1f",
                     video_view->playing() ? 1 : 0, video_view->loop() ? 1 : 0, video_view->speed(),
                     video_view->volume(), video_view->position());
        video_view->pause();
        video = video_view;
        box->add_child(video_view);
    }
#endif

#if defined(OPENGUI_MEDIA_OPENCV)
    caption(*box, &font, "== media/opencv: ImageView ==");
    {
        ui::ImageView* image = new ui::ImageView();
        image->set_bounds(0, 0, 300, 170);
        const bool ok = load_media_file("test.png", [image](const char* p) { return image->load_from_file(p); });
        ui::log_info("api_demo: image open=%d size=%dx%d", ok ? 1 : 0, image->image_width(),
                     image->image_height());
        box->add_child(image);
    }
#endif

    // 直接文本绘制（gl Font API）
    ui::Label* font_info = new ui::Label(0, 0, 300, 0, "Font: draw/measure/icon");
    font_info->set_font(&font);
    font_info->set_theme(&theme);
    box->add_child(font_info);

    // ---- Scene 生命周期订阅 ----
    // 帧前：媒体/轮播/上传推进 + 主题清屏色同步
    scene.subscribe(ui::ScenePhase::Frame, [&](ui::Scene&, float dt) {
        scene.set_root_background(theme.window_bg);
#if defined(OPENGUI_MEDIA_FFMPEG)
        if (video && video->has_video()) video->advance(dt);
#endif
        if (carousel) carousel->advance(dt);
        if (upload) upload->advance(dt);
    });

    // 帧后：GL 直绘样例 + HUD + 图标（begin_frame+root.draw 之后）
    auto last = std::chrono::steady_clock::now();
    int frames = 0;
    scene.on(ui::ScenePhase::PostDraw, [&](ui::Scene&) {
        const float t = std::chrono::duration<float>(
                            std::chrono::steady_clock::now().time_since_epoch())
                            .count();
        draw_gl_samples(renderer, tex, theme, t);

        ++frames;
        auto now = std::chrono::steady_clock::now();
        char hud[160];
        if (now - last >= std::chrono::seconds(1)) {
            const double fps = frames / std::chrono::duration<double>(now - last).count();
            frames = 0;
            last = now;
            std::snprintf(hud, sizeof(hud),
                          "api_demo fps %.1f | measure 'hello'=%.0f | cw=%d=%u", fps,
                          (double)font.measure_text("hello"), (int)'a', (unsigned)font.char_width('a'));
        }
        font.draw_text(renderer, hud, 24.f, 24.f, theme.text_dim);
        if (font.icon_loaded()) font.draw_icon(renderer, ui::layui_icon::star, 1200.f, 40.f, 20.f,
                                               ui::Vec4{1.f, 0.85f, 0.3f, 1.f});
    });

    // 运行：--smoke 指定秒自动退出，否则直至窗口关闭
    if (smoke_seconds > 0.0) {
        scene.run_for(smoke_seconds);
    } else {
        scene.run();
    }

    // Font 释放前调用（GL 上下文存续期间）
    window.swap_buffers();
    return 0;
}
