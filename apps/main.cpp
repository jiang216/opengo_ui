#include "ui/core/keycodes.h"
#include "ui/core/log.h"
#include "ui/core/version.h"
#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/window.h"
#include "ui/ui/button.h"
#include "ui/ui/badge.h"
#include "ui/ui/breadcrumb.h"
#include "ui/ui/card.h"
#include "ui/ui/carousel.h"
#include "ui/ui/code.h"
#include "ui/ui/colorpicker.h"
#include "ui/ui/icon.h"
#include "ui/ui/checkbox.h"
#include "ui/ui/collapse.h"
#include "ui/ui/combobox.h"
#include "ui/ui/dropdown.h"
#include "ui/ui/grid.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/laypage.h"
#include "ui/ui/menubar.h"
#include "ui/ui/navmenu.h"
#include "ui/ui/panelwindow.h"
#include "ui/ui/progress.h"
#include "ui/ui/radiogroup.h"
#include "ui/ui/rate.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/slider.h"
#include "ui/ui/switch.h"
#include "ui/ui/table.h"
#include "ui/ui/tabs.h"
#include "ui/ui/textbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/timeline.h"
#include "ui/ui/transfer.h"
#include "ui/ui/tree.h"
#include "ui/ui/upload.h"
#include "ui/ui/laydate.h"
#include "ui/ui/vbox.h"
#include "ui/ui/widget.h"

#if defined(OPENGUI_MEDIA_FFMPEG)
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

// 演示素材定位：优先工作目录（POST_BUILD 已把素材拷到可执行目录），否则回退源码 assets/
static bool load_media_file(const char* name, std::function<bool(const char*)> loader) {
    char in_cwd[256], in_assets[256];
    std::snprintf(in_cwd, sizeof(in_cwd), "%s", name);
    std::snprintf(in_assets, sizeof(in_assets), "assets/%s", name);
    return loader(in_cwd) || loader(in_assets);
}

// 找到当前悬停且带 tooltip 的最上层控件（后添加的子节点在上层）
static ui::Widget* find_tooltip_widget(ui::Widget* w) {
    for (auto it = w->children().rbegin(); it != w->children().rend(); ++it) {
        if (ui::Widget* f = find_tooltip_widget(*it)) return f;
    }
    if (w->hovered() && !w->tooltip().empty()) return w;
    return nullptr;
}

// --smoke <秒>：运行指定秒后自动退出（CI/冒烟：验证启动、解码、音频初始化不崩溃）
// --file <路径>：用指定视频文件替换默认演示素材（VideoView 加载；UTF-8 路径）
int main(int argc, char** argv) {
    double smoke_seconds = 0.0;
    const char* media_file = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--smoke") == 0 && i + 1 < argc)
            smoke_seconds = std::atof(argv[++i]);
        else if (std::strcmp(argv[i], "--file") == 0 && i + 1 < argc)
            media_file = argv[++i];
    }
    const auto smoke_start = std::chrono::steady_clock::now();

    ui::log_info("opengl_ui %s starting", ui::version_string());

    ui::Window window(1280, 1280, "opengl_ui demo (M17d)");
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

    // 字体（系统 Arial，像素高度 20）
    ui::Font font;
    if (!font.load_from_file("C:/Windows/Fonts/arial.ttf", 20.f)) {
        ui::log_error("font load failed");
    }
    // 中文字体（懒烘培）：env OPENGUI_CJK_FONT 自定义 → 系统候选（UI-036）
    if (!font.load_cjk_font_auto()) {
        ui::log_error("cjk font load failed");
    }
    // 图标字体（M17d）：layui iconfont.ttf，私用区 U+E600..E7FF
    if (!font.load_icon_font("assets/iconfont.ttf", 20.f) &&
        !font.load_icon_font("iconfont.ttf", 20.f)) {
        ui::log_error("icon font load failed");
    }

    ui::Theme theme;

    // 控件树：root -> VBox(居中) -> [Label 标题, TextBox, Label 回显, Button x3, Label 提示]
    ui::Widget root;
    root.set_bounds(0, 0, (float)window.width(), (float)window.height());

    ui::VBox box(440, 20, 400, 920);
    box.set_theme(&theme);
    box.set_spacing(10.f);
    box.set_padding(20.f);
    root.add_child(&box);

    ui::Label title(0, 0, 360, 0, "opengl_ui M14: window/menu/tooltip");
    title.set_font(&font);
    title.set_theme(&theme);
    title.set_align(1);
    box.add_child(&title);

    ui::TextBox input(0, 0, 360, 40);
    input.set_font(&font);
    input.set_theme(&theme);
    box.add_child(&input);

    ui::Label echo(0, 0, 360, 0, "type above");
    echo.set_font(&font);
    echo.set_theme(&theme);
    echo.set_align(1);
    box.add_child(&echo);
    input.set_on_change([&](const char* s) { echo.set_text(s); });

    ui::Button btn_a(0, 0, 360, 48, "Button A");
    btn_a.set_font(&font);
    btn_a.set_theme(&theme);
    box.add_child(&btn_a);

    ui::Button btn_b(0, 0, 360, 48, "Button B");
    btn_b.set_font(&font);
    btn_b.set_theme(&theme);
    box.add_child(&btn_b);

    ui::Button btn_c(0, 0, 360, 48, "Button C");
    btn_c.set_font(&font);
    btn_c.set_theme(&theme);
    box.add_child(&btn_c);

    int count = 0;
    btn_a.set_on_click([&]() { ++count; });
    btn_b.set_on_click([&]() { count += 10; });
    btn_c.set_on_click([&]() { count = 0; });

    ui::Label slider_cap(0, 0, 360, 0, "slider value");
    slider_cap.set_font(&font);
    slider_cap.set_theme(&theme);
    slider_cap.set_align(1);
    box.add_child(&slider_cap);

    ui::Slider slider(0, 0, 360, 24);
    slider.set_theme(&theme);
    slider.set_range(0.f, 100.f);
    box.add_child(&slider);

    ui::Label slider_val(0, 0, 360, 0, "0.0");
    slider_val.set_font(&font);
    slider_val.set_theme(&theme);
    slider_val.set_align(1);
    box.add_child(&slider_val);
    slider.set_on_change([&](float v) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f", v);
        slider_val.set_text(buf);
    });

    ui::ScrollBox scrollbox(0, 0, 360, 80);
    scrollbox.set_theme(&theme);
    scrollbox.set_padding(4.f);
    scrollbox.set_spacing(2.f);
    box.add_child(&scrollbox);
    // M12：嵌套容器——ScrollBox 内放 VBox，VBox 内放条目
    ui::VBox* list = new ui::VBox(0, 0, 0, 0);
    list->set_theme(&theme);
    list->set_padding(4.f);
    list->set_spacing(2.f);
    scrollbox.add_child(list);
    for (int i = 0; i < 20; ++i) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "item %02d (%.0f%%)", i, i * 5.2f);
        ui::Label* item = new ui::Label(0, 0, 0, 0, buf);
        item->set_font(&font);
        item->set_theme(&theme);
        list->add_child(item);
    }

    ui::CheckBox cb_opt(0, 0, 360, 24, "extra features enabled");
    cb_opt.set_font(&font);
    cb_opt.set_theme(&theme);
    box.add_child(&cb_opt);

    ui::Label radio_cap(0, 0, 360, 0, "render mode");
    radio_cap.set_font(&font);
    radio_cap.set_theme(&theme);
    box.add_child(&radio_cap);

    ui::RadioGroup radio(0, 0, 360, 84);
    radio.set_font(&font);
    radio.set_theme(&theme);
    radio.add_option("standard");
    radio.add_option("dark preview");
    radio.add_option("wireframe");
    box.add_child(&radio);

    ui::Label combo_cap(0, 0, 360, 0, "theme palette");
    combo_cap.set_font(&font);
    combo_cap.set_theme(&theme);
    box.add_child(&combo_cap);

    ui::ComboBox combo(0, 0, 360, 28);
    combo.set_font(&font);
    combo.set_theme(&theme);
    combo.add_option("auto");
    combo.add_option("light");
    combo.add_option("dark");
    combo.add_option("layui");
    box.add_child(&combo);

    ui::Label status(0, 0, 360, 0, "state: check on | mode - | palette -");
    status.set_font(&font);
    status.set_theme(&theme);
    status.set_align(1);
    box.add_child(&status);
    auto update_status = [&]() {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "state: check %s | mode %d | palette %d",
                      cb_opt.checked() ? "on" : "off", radio.selected(), combo.selected());
        status.set_text(buf);
    };
    cb_opt.set_on_change([&](bool) { update_status(); });
    radio.set_on_change([&](int) { update_status(); });
    // M10：下拉选主题调色板 -> 运行期切换主题（控件持有 &theme 每次绘制读取）
    combo.set_on_change([&](int idx) {
        if (idx == 1) theme = ui::Theme::make_light();
        else if (idx == 2) theme = ui::Theme::make_dark();
        else if (idx == 3) theme = ui::Theme::make_layui();
        update_status();
    });
    update_status();

    // M11：自动换行多行文本（按控件宽度逐码点断行，CJK 按方块字宽近似）
    ui::Label desc(0, 0, 360, 0,
                   "This label wraps automatically to multiple lines when the text is longer "
                   "than the widget width. Word boundaries are ignored: breaking happens at "
                   "character level, which also handles CJK width approximation.");
    desc.set_font(&font);
    desc.set_theme(&theme);
    desc.set_wrap(true);
    box.add_child(&desc);

    ui::Label hint(0, 0, 360, 0, "drag: slider | type: input | wheel: scroll | Tab: focus");
    hint.set_font(&font);
    hint.set_theme(&theme);
    hint.set_align(1);
    box.add_child(&hint);

    // M12：左侧独立卡片演示 HBox 横向 + Grid 网格布局
    ui::VBox left(40, 20, 360, 300);
    left.set_theme(&theme);
    left.set_spacing(10.f);
    left.set_padding(20.f);
    root.add_child(&left);

    ui::Label ltitle(0, 0, 320, 0, "M12: HBox / Grid");
    ltitle.set_font(&font);
    ltitle.set_theme(&theme);
    ltitle.set_align(1);
    left.add_child(&ltitle);

    ui::Label hcap(0, 0, 320, 0, "HBox (row)");
    hcap.set_font(&font);
    hcap.set_theme(&theme);
    left.add_child(&hcap);

    ui::HBox hrow(0, 0, 320, 40);
    hrow.set_theme(&theme);
    hrow.set_padding(0.f);
    hrow.set_spacing(8.f);
    ui::Button hb_a(0, 0, 150, 40, "left");
    ui::Button hb_b(0, 0, 150, 40, "right");
    hb_a.set_font(&font);
    hb_a.set_theme(&theme);
    hb_b.set_font(&font);
    hb_b.set_theme(&theme);
    hb_a.set_on_click([&]() { count += 1; });
    hb_b.set_on_click([&]() { count += 10; });
    hrow.add_child(&hb_a);
    hrow.add_child(&hb_b);
    left.add_child(&hrow);

    ui::Label gcap(0, 0, 320, 0, "Grid 2x2");
    gcap.set_font(&font);
    gcap.set_theme(&theme);
    left.add_child(&gcap);

    ui::Grid grid(0, 0, 320, 0, 2);
    grid.set_theme(&theme);
    grid.add_class("layui-row");  // UI-039：layui-row 行容器由 ui 库 Grid 识别为无背景
    grid.set_row_height(36.f);
    grid.set_padding(0.f);
    grid.set_spacing(8.f);
    for (int i = 0; i < 4; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "cell %d", i + 1);
        ui::Button* cell = new ui::Button(0, 0, 0, 0, buf);
        cell->set_font(&font);
        cell->set_theme(&theme);
        cell->set_on_click([&, i]() { count += (i + 1) * 100; });
        grid.add_child(cell);
    }
    left.add_child(&grid);

    // ---- M17a：layui 轻量组件克隆（类名语法 + Theme::make_layui）----
    ui::Card layui_card(40, 20, 360, 320);
    layui_card.set_font(&font);
    layui_card.set_theme(&theme);
    layui_card.set_header("M17a: layui components");
    root.add_child(&layui_card);

    ui::Breadcrumb crumb(0, 0, 330, 0);
    crumb.set_font(&font);
    crumb.set_theme(&theme);
    crumb.add_class("layui-breadcrumb");
    crumb.add_item("Home");
    crumb.add_item("UI");
    crumb.add_current("layui clone");
    layui_card.add_child(&crumb);

    ui::Progress prog(0, 0, 330, 6);
    prog.set_font(&font);
    prog.set_theme(&theme);
    prog.set_value(0.55f);
    layui_card.add_child(&prog);

    ui::Progress prog_big(0, 0, 330, 18);
    prog_big.add_class("layui-progress-big");
    prog_big.set_font(&font);
    prog_big.set_theme(&theme);
    prog_big.set_value(0.8f);
    layui_card.add_child(&prog_big);

    // 徽标行：默认 / rim / dot / 工具色
    ui::HBox badge_row(0, 0, 330, 28);
    badge_row.set_theme(&theme);
    badge_row.set_spacing(8.f);
    badge_row.set_padding(0.f);
    layui_card.add_child(&badge_row);

    ui::Badge badge(0, 0, "new");
    badge.set_font(&font);
    badge.set_theme(&theme);
    badge.set_bounds(0, 0, 46, 18);
    badge.add_class("layui-badge");
    badge_row.add_child(&badge);

    ui::Badge badge_rim(0, 0, "rim");
    badge_rim.set_font(&font);
    badge_rim.set_theme(&theme);
    badge_rim.set_bounds(0, 0, 46, 18);
    badge_rim.add_class("layui-badge-rim");
    badge_row.add_child(&badge_rim);

    ui::Badge badge_dot(0, 0, "");
    badge_dot.set_font(&font);
    badge_dot.set_theme(&theme);
    badge_dot.set_bounds(0, 0, 8, 8);
    badge_dot.add_class("layui-badge-dot");
    badge_row.add_child(&badge_dot);

    ui::Badge badge_bg(0, 0, "bg-blue");
    badge_bg.set_font(&font);
    badge_bg.set_theme(&theme);
    badge_bg.set_bounds(0, 0, 64, 18);
    badge_bg.add_class("layui-badge layui-bg-blue");
    badge_row.add_child(&badge_bg);

    // 评分：点击设置值，hover 预览
    ui::Rate rate(0, 0, 330, 24);
    rate.set_theme(&theme);
    rate.set_value(3);
    layui_card.add_child(&rate);

    // 时间轴
    ui::Timeline tl(0, 0, 330, 0);
    tl.set_font(&font);
    tl.set_theme(&theme);
    tl.add_item("M17a", "layui lightweight widgets");
    tl.add_item("M17b", "forms & buttons");
    layui_card.add_child(&tl);

    // 工具色横条（layui-bg-*）
    ui::HBox bg_row(0, 0, 330, 28);
    bg_row.set_theme(&theme);
    bg_row.set_spacing(4.f);
    bg_row.set_padding(0.f);
    layui_card.add_child(&bg_row);
    const char* bg_names[] = {"red", "orange", "green", "cyan", "blue", "purple", "black", "gray"};
    for (int i = 0; i < 8; ++i) {
        ui::Badge* chip = new ui::Badge(0, 0, bg_names[i]);
        chip->set_font(&font);
        chip->set_theme(&theme);
        chip->set_bounds(0, 0, 40, 18);
        char cls[64];
        std::snprintf(cls, sizeof(cls), "layui-badge layui-bg-%s", bg_names[i]);
        chip->add_class(cls);
        bg_row.add_child(chip);
    }

    // ---- M17b：layui 按钮变体 + Switch 开关（克隆 layui.css）----
    ui::Card m17b_card(40, 356, 360, 560);
    m17b_card.set_font(&font);
    m17b_card.set_theme(&theme);
    m17b_card.set_header("M17b: layui buttons / switch");
    root.add_child(&m17b_card);

    // 按钮变体行 1：默认 / normal / warm / danger
    ui::HBox btn_row1(0, 0, 330, 38);
    btn_row1.set_theme(&theme);
    btn_row1.set_spacing(8.f);
    btn_row1.set_padding(0.f);
    m17b_card.add_child(&btn_row1);
    const char* btn_variants[] = {"Default", "Normal", "Warm", "Danger"};
    const char* btn_classes[] = {"layui-btn", "layui-btn layui-btn-normal",
                                 "layui-btn layui-btn-warm", "layui-btn layui-btn-danger"};
    for (int i = 0; i < 4; ++i) {
        ui::Button* b = new ui::Button(0, 0, 76, 38, btn_variants[i]);
        b->set_font(&font);
        b->set_theme(&theme);
        b->add_class(btn_classes[i]);
        btn_row1.add_child(b);
    }

    // 按钮变体行 2：primary / radius / disabled / xs
    ui::HBox btn_row2(0, 0, 330, 38);
    btn_row2.set_theme(&theme);
    btn_row2.set_spacing(8.f);
    btn_row2.set_padding(0.f);
    m17b_card.add_child(&btn_row2);
    ui::Button* btn_primary = new ui::Button(0, 0, 76, 38, "Primary");
    btn_primary->set_font(&font);
    btn_primary->set_theme(&theme);
    btn_primary->add_class("layui-btn layui-btn-primary");
    btn_row2.add_child(btn_primary);
    ui::Button* btn_radius = new ui::Button(0, 0, 76, 38, "Radius");
    btn_radius->set_font(&font);
    btn_radius->set_theme(&theme);
    btn_radius->add_class("layui-btn layui-btn-radius");
    btn_row2.add_child(btn_radius);
    ui::Button* btn_disabled = new ui::Button(0, 0, 76, 38, "Disabled");
    btn_disabled->set_font(&font);
    btn_disabled->set_theme(&theme);
    btn_disabled->add_class("layui-btn layui-btn-disabled");
    btn_row2.add_child(btn_disabled);
    ui::Button* btn_xs = new ui::Button(0, 0, 76, 38, "xs");
    btn_xs->set_font(&font);
    btn_xs->set_theme(&theme);
    btn_xs->add_class("layui-btn layui-btn-xs");
    btn_row2.add_child(btn_xs);

    // Switch 开关：开（绿底白滑块）+ 关（白底灰滑块）
    ui::HBox sw_row(0, 0, 330, 24);
    sw_row.set_theme(&theme);
    sw_row.set_spacing(12.f);
    sw_row.set_padding(0.f);
    m17b_card.add_child(&sw_row);
    ui::Switch sw_on(0, 0, 60, 24);
    sw_on.set_font(&font);
    sw_on.set_theme(&theme);
    sw_on.set_checked(true);
    sw_on.set_text("ON");
    sw_row.add_child(&sw_on);
    ui::Switch sw_off(0, 0, 60, 24);
    sw_off.set_font(&font);
    sw_off.set_theme(&theme);
    sw_off.set_checked(false);
    sw_off.set_text("OFF");
    sw_row.add_child(&sw_off);

    // ---- M17b：Tabs 标签页（layui-tab-brief）----
    ui::Tabs tabs(0, 0, 330, 120);
    tabs.set_font(&font);
    tabs.set_theme(&theme);
    tabs.add_class("layui-tab-brief");
    tabs.add_tab("design");
    tabs.add_tab("code");
    tabs.add_tab("docs");
    m17b_card.add_child(&tabs);
    ui::Label tab_info(0, 0, 330, 0, "tab: design");
    tab_info.set_font(&font);
    tab_info.set_theme(&theme);
    m17b_card.add_child(&tab_info);
    tabs.set_on_change([&](int idx) {
        const char* names[] = {"design", "code", "docs"};
        char buf[64];
        std::snprintf(buf, sizeof(buf), "tab: %s", names[idx]);
        tab_info.set_text(buf);
    });

    // ---- M17b：Collapse 折叠面板（layui-collapse，手风琴）----
    ui::Collapse collapse(0, 0, 330, 100);
    collapse.set_font(&font);
    collapse.set_theme(&theme);
    collapse.add_item("How to clone layui?");
    {
        ui::Label* l1 = new ui::Label(0, 0, 300, 24, "Add layui class names; style tokens from layui.css");
        l1->set_font(&font);
        l1->set_theme(&theme);
        collapse.add_child(l1);
    }
    collapse.add_item("Any limitations?");
    {
        ui::Label* l2 = new ui::Label(0, 0, 300, 24, "ASCII font only; icon glyphs pending (M17d)");
        l2->set_font(&font);
        l2->set_theme(&theme);
        collapse.add_child(l2);
    }
    collapse.set_active(0);
    m17b_card.add_child(&collapse);

    // ---- M17b：Dropdown 下拉菜单（layui-dropdown / layui-menu）----
    ui::Dropdown dropdown(0, 0, 120, 38, "menu");
    dropdown.set_font(&font);
    dropdown.set_theme(&theme);
    dropdown.add_item("copy", []() { ui::log_info("dropdown: copy"); });
    dropdown.add_separator();
    dropdown.add_item("paste", []() { ui::log_info("dropdown: paste"); });
    dropdown.add_item("settings", []() { ui::log_info("dropdown: settings"); });
    m17b_card.add_child(&dropdown);

    // ---- M17c：layui 数据与弹层（Table / Tree / NavMenu / Laypage / layer）----
    // 右列仅 media OFF 构建空闲；media ON 时右列让给图像/视频
#if !defined(OPENGUI_MEDIA_OPENCV) && !defined(OPENGUI_MEDIA_FFMPEG)
    ui::Card m17c_card(880, 20, 360, 660);
    m17c_card.set_font(&font);
    m17c_card.set_theme(&theme);
    m17c_card.set_header("M17c: data & layer");
    root.add_child(&m17c_card);

    // Table：layui-table 克隆（表头灰底 + 排序箭头 + 行 hover + 行内编辑）
    ui::Table table(0, 0, 330, 200);
    table.set_font(&font);
    table.set_theme(&theme);
    table.add_column("name", 120, true);
    table.add_column("type", 90, true);
    table.add_column("status", 120, true);
    table.add_row({"glfw", "core", "ok"});
    table.add_row({"stb", "core", "ok"});
    table.add_row({"ffmpeg", "media", "skip"});
    table.add_row({"opencv", "media", "skip"});
    table.add_row({"glad", "core", "ok"});
    m17c_card.add_child(&table);

    // HBox：NavMenu（垂直导航）+ Tree（树形）
    ui::HBox nav_tree_row(0, 0, 330, 260);
    nav_tree_row.set_theme(&theme);
    nav_tree_row.set_padding(0.f);
    nav_tree_row.set_spacing(10.f);
    m17c_card.add_child(&nav_tree_row);

    ui::NavMenu navmenu(0, 0, 130, 260);
    navmenu.set_font(&font);
    navmenu.set_theme(&theme);
    navmenu.add_group("components");
    navmenu.add_item("table", []() { ui::log_info("nav: table"); });
    navmenu.add_item("tree", []() { ui::log_info("nav: tree"); });
    navmenu.add_item("page", []() { ui::log_info("nav: laypage"); });
    navmenu.add_group("window");
    navmenu.add_item("layer", []() { ui::log_info("nav: layer"); });
    navmenu.add_item("modal", []() { ui::log_info("nav: modal"); });
    navmenu.set_active(0);
    nav_tree_row.add_child(&navmenu);

    ui::Tree tree(0, 0, 190, 260);
    tree.set_font(&font);
    tree.set_theme(&theme);
    const int n_root = tree.add_node("layui");
    const int n_ui = tree.add_node("ui", n_root);
    const int n_gl = tree.add_node("gl", n_root);
    tree.add_node("table", n_ui);
    tree.add_node("tree", n_ui);
    tree.add_node("window", n_gl);
    tree.set_expanded(n_root, true);
    tree.set_expanded(n_ui, true);
    tree.set_expanded(n_gl, false);
    tree.set_on_click([](int) { ui::log_info("tree: click"); });
    nav_tree_row.add_child(&tree);

    // Laypage：分页条（可见页码折叠 + 省略号）
    ui::Laypage laypage(0, 0, 330, 28);
    laypage.set_font(&font);
    laypage.set_theme(&theme);
    laypage.set_total(25);
    laypage.set_page(3);
    laypage.set_on_change([](int p) { ui::log_info("laypage: page %d", p + 1); });
    m17c_card.add_child(&laypage);

    // PanelWindow layer 外观：白底圆角 2 + 标题栏高 50 + 右上角 ×
    ui::PanelWindow layer_win(880, 700, 360, 220, "layer window");
    layer_win.set_font(&font);
    layer_win.set_theme(&theme);
    layer_win.add_class("layui-layer");
    root.add_child(&layer_win);
    ui::Label layer_text(0, 0, 320, 24, "layui-layer style: white bg, h50 title");
    layer_text.set_font(&font);
    layer_text.set_theme(&theme);
    layer_win.add_child(&layer_text);
#endif

#if defined(OPENGUI_MEDIA_OPENCV)
    // 右侧：图像控件（OpenCV 载入 → 纹理）
    ui::ImageView image_view;
    image_view.set_bounds(880, 80, 360, 240);
    load_media_file("test.png", [&image_view](const char* p) { return image_view.load_from_file(p); });
    root.add_child(&image_view);
#endif

#if defined(OPENGUI_MEDIA_FFMPEG)
    // 右侧下方：视频控件（后台线程解码 → 主线程逐帧上传纹理）
    ui::VideoView video_view;
    video_view.set_bounds(880, 340, 360, 240);
    if (media_file) {
        if (!video_view.open_source(media_file))
            ui::log_error("VideoView: cannot open %s", media_file);
    } else {
        load_media_file("test_h264.mp4", [&video_view](const char* p) { return video_view.open_source(p); });
        if (!video_view.has_video())  // 回退带音频 h265 / 旧无音频素材
            load_media_file("test_av.mp4", [&video_view](const char* p) { return video_view.open_source(p); });
        if (!video_view.has_video())
            load_media_file("test.mp4", [&video_view](const char* p) { return video_view.open_source(p); });
    }
    root.add_child(&video_view);

    // 播放控制：Play/Pause 切换 + 进度条 + 时间显示
    ui::Button vid_toggle(880, 588, 90, 30, "Pause");
    vid_toggle.set_font(&font);
    vid_toggle.set_theme(&theme);
    root.add_child(&vid_toggle);
    vid_toggle.set_on_click([&]() {
        if (video_view.playing()) {
            video_view.pause();
            vid_toggle.set_text("Play");
        } else {
            video_view.play();
            vid_toggle.set_text("Pause");
        }
    });

    ui::Slider vid_seek(880, 628, 360, 24);
    vid_seek.set_theme(&theme);
    vid_seek.set_range(0.f, 1.f);
    vid_seek.set_value_silent(0.f);
    root.add_child(&vid_seek);
    vid_seek.set_on_change([&](float t) {
        if (video_view.has_video()) video_view.seek(t * (float)video_view.duration());
    });

    ui::Label vid_time(880, 662, 360, 20, "0.0 / 0.0 s");
    vid_time.set_font(&font);
    vid_time.set_theme(&theme);
    root.add_child(&vid_time);

    // M13：倍速 + 逐帧步进 + 循环开关
    ui::Label vid_speed_lb(880, 692, 360, 20, "speed 1.00x");
    vid_speed_lb.set_font(&font);
    vid_speed_lb.set_theme(&theme);
    root.add_child(&vid_speed_lb);

    ui::Slider vid_speed(880, 716, 360, 24);
    vid_speed.set_theme(&theme);
    vid_speed.set_range(0.5f, 2.0f);
    vid_speed.set_value_silent(1.0f);
    root.add_child(&vid_speed);
    vid_speed.set_on_change([&](float s) { video_view.set_speed(s); });

    ui::Button vid_step(880, 750, 90, 30, "step frame");
    vid_step.set_font(&font);
    vid_step.set_theme(&theme);
    root.add_child(&vid_step);
    vid_step.set_on_click([&]() {
        vid_toggle.set_text("Play");  // 步进会进入暂停态
        video_view.step_frame();
    });

    ui::CheckBox vid_loop(880, 790, 200, 24, "loop playback");
    vid_loop.set_font(&font);
    vid_loop.set_theme(&theme);
    vid_loop.set_checked(true);
    root.add_child(&vid_loop);
    vid_loop.set_on_change([&](bool v) { video_view.set_loop(v); });

    // M14 遗留修复：音频音量滑块（实时作用于解码 PCM）
    ui::Label vid_vol_lb(880, 798, 360, 20, "volume 1.00");
    vid_vol_lb.set_font(&font);
    vid_vol_lb.set_theme(&theme);
    root.add_child(&vid_vol_lb);

    ui::Slider vid_vol(880, 824, 360, 24);
    vid_vol.set_theme(&theme);
    vid_vol.set_range(0.f, 1.f);
    vid_vol.set_value_silent(1.0f);
    root.add_child(&vid_vol);
    vid_vol.set_on_change([&](float v) { video_view.set_volume(v); });
    vid_vol.set_tooltip("audio volume 0..1 (muted if source has no audio)");
#endif

    // ---- M17d：图标 + 特殊组件（Icon / Carousel / Code / ColorPicker / Transfer / Laydate / Upload）----
    ui::Card m17d_card(40, 940, 360, 320);
    m17d_card.set_font(&font);
    m17d_card.set_theme(&theme);
    m17d_card.set_header("M17d: icon & special");
    root.add_child(&m17d_card);

    ui::ScrollBox m17d_box(0, 0, 330, 260);
    m17d_box.set_theme(&theme);
    m17d_box.set_spacing(10.f);
    m17d_card.add_child(&m17d_box);

    // Icon 行：layui-icon 私用区字符（类名 -> 码点 -> 渲染）
    ui::HBox icon_row(0, 0, 330, 28);
    icon_row.set_theme(&theme);
    icon_row.set_spacing(8.f);
    icon_row.set_padding(0.f);
    m17d_box.add_child(&icon_row);
    const char* icon_names[] = {"star", "set", "face-smile", "camera", "chat", "heart", "link", "bug"};
    for (const char* iname : icon_names) {
        ui::Icon* ic = new ui::Icon(0, 0, 28, 28, iname);
        ic->set_font(&font);
        ic->set_theme(&theme);
        ic->add_class("layui-icon");
        icon_row.add_child(ic);
    }

    // Carousel：轮播（3 页 + 箭头 + 指示点）
    ui::Carousel carousel(0, 0, 330, 120);
    carousel.set_font(&font);
    carousel.set_theme(&theme);
    carousel.add_slide(ui::Vec4{0.12f, 0.62f, 1.0f, 1.0f}, "opengl_ui 0.1.0");
    carousel.add_slide(ui::Vec4{0.09f, 0.73f, 0.67f, 1.0f}, "layui clone");
    carousel.add_slide(ui::Vec4{0.64f, 0.20f, 0.78f, 1.0f}, "M17d icons");
    carousel.set_auto_play(true, 2.0f);
    m17d_box.add_child(&carousel);

    // Code：代码块
    ui::Code code(0, 0, 330, 110);
    code.set_font(&font);
    code.set_theme(&theme);
    code.set_lang("cpp");
    code.set_code("// layui clone\ni = 0;\nfont.load_icon_font(ttf);\nIcon icon(0,0,28,28,\"star\");");
    m17d_box.add_child(&code);

    // ColorPicker：色板 + 预设 + hex
    ui::ColorPicker picker(0, 0, 330, 150);
    picker.set_font(&font);
    picker.set_theme(&theme);
    m17d_box.add_child(&picker);
    picker.set_on_change([](const ui::Vec4& c) {
        ui::log_info("colorpicker: #%02X%02X%02X", (int)(c.r * 255.f), (int)(c.g * 255.f),
                     (int)(c.b * 255.f));
    });

    // Transfer：穿梭框
    ui::Transfer transfer(0, 0, 330, 130);
    transfer.set_font(&font);
    transfer.set_theme(&theme);
    transfer.set_left_label("modules");
    transfer.set_right_label("enabled");
    transfer.add_item("glfw");
    transfer.add_item("stb");
    transfer.add_item("glad");
    transfer.add_item("ffmpeg");
    m17d_box.add_child(&transfer);

    // Laydate：日期选择
    ui::Laydate laydate(0, 0, 330, 170);
    laydate.set_font(&font);
    laydate.set_theme(&theme);
    laydate.set_date(2025, 8, 10);
    m17d_box.add_child(&laydate);
    laydate.set_on_change([](int y, int m, int d) { ui::log_info("laydate: %d-%02d-%02d", y, m, d); });

    // Upload：模拟上传
    ui::Upload upload(0, 0, 330, 130);
    upload.set_font(&font);
    upload.set_theme(&theme);
    m17d_box.add_child(&upload);
    upload.add_file("assets/test.png", 1024 * 48);
    upload.add_file("assets/test.mp4", 1024 * 2048);

    // ---- M14：菜单栏（横向菜单 + 下拉）+ Tooltip + 可移动/缩放窗口（模态）----
    ui::MenuBar menubar(0, 0, 1280, 30);
    menubar.set_font(&font);
    menubar.set_theme(&theme);
    root.add_child(&menubar);

    // 模态窗口（初始隐藏）：About 对话框，演示拖动/缩放/关闭 + 模态屏蔽
    ui::PanelWindow about_win(400, 320, 360, 200, "About");
    about_win.set_font(&font);
    about_win.set_theme(&theme);
    about_win.visible = false;
    root.add_child(&about_win);

    auto close_about = [&]() {
        about_win.visible = false;
        root.set_modal(nullptr);
    };
    about_win.set_on_close(close_about);

    ui::VBox about_box(400, 346, 320, 160);
    about_box.set_theme(&theme);
    about_box.set_spacing(10.f);
    about_box.set_padding(16.f);
    ui::Label about_text(0, 0, 288, 0, "opengl_ui 0.1.0\nM14: windows, menus,\nmodals and tooltips");
    about_text.set_font(&font);
    about_text.set_theme(&theme);
    about_box.add_child(&about_text);
    ui::Button about_ok(0, 0, 120, 30, "OK");
    about_ok.set_font(&font);
    about_ok.set_theme(&theme);
    about_box.add_child(&about_ok);
    about_win.add_child(&about_box);
    about_ok.set_on_click(close_about);

    bool demo_exit = false;
    menubar.add_menu("File");
    menubar.add_item("Open...", []() { ui::log_info("menu: Open (not implemented)"); });
    menubar.add_item("Exit", [&]() { demo_exit = true; });
    menubar.add_menu("View");
    menubar.add_item("Theme: Light", [&]() { theme = ui::Theme::make_light(); });
    menubar.add_item("Theme: Dark", [&]() { theme = ui::Theme::make_dark(); });
    menubar.add_item("Theme: layui", [&]() { theme = ui::Theme::make_layui(); });
    menubar.add_menu("Help");
    menubar.add_item("About...", [&]() {
        about_win.visible = true;
        root.set_modal(&about_win);
    });

    // Tooltip 演示
#if defined(OPENGUI_MEDIA_FFMPEG)
    vid_seek.set_tooltip("drag to seek");
    vid_speed.set_tooltip("playback speed 0.5x..2.0x");
    vid_toggle.set_tooltip("pause / resume");
#endif
    hb_a.set_tooltip("count += 1");
    hb_b.set_tooltip("count += 10");
    grid.set_tooltip("2x2 grid buttons");

    auto last = std::chrono::steady_clock::now();
    auto prev = last;
    int frames = 0;
    while (!window.should_close()) {
        window.poll_events();
        if (demo_exit) break;
        const float dt = std::chrono::duration<float>(std::chrono::steady_clock::now() - prev).count();
        prev = std::chrono::steady_clock::now();
        if (smoke_seconds > 0.0 &&
            std::chrono::duration<double>(std::chrono::steady_clock::now() - smoke_start).count() >=
                smoke_seconds) {
            ui::log_info("smoke: exiting after %.1fs", smoke_seconds);
            break;
        }
#if defined(OPENGUI_MEDIA_FFMPEG)
        video_view.advance(dt);        if (video_view.has_video()) {
            // 进度同步：拖动中不覆盖手柄位置（避免与用户输入打架）
            const float dur = (float)video_view.duration();
            const float pos = video_view.position();
            if (!vid_seek.dragging())
                vid_seek.set_value_silent(dur > 0.f ? pos / dur : 0.f);
            char tbuf[48];
            std::snprintf(tbuf, sizeof(tbuf), "%.1f / %.1f s", pos, dur);
            vid_time.set_text(tbuf);
            char spbuf[32];
            std::snprintf(spbuf, sizeof(spbuf), "speed %.2fx", video_view.speed());
            vid_speed_lb.set_text(spbuf);
            vid_loop.set_checked(video_view.loop());
            char vbuf[32];
            std::snprintf(vbuf, sizeof(vbuf), "volume %.2f", video_view.volume());
            vid_vol_lb.set_text(vbuf);
        }
#endif

        // M17d：轮播自动播放 + 上传进度推进
        carousel.advance(dt);
        upload.advance(dt);

        // 窗口尺寸变化 -> 更新视口与 root
        if (window.resized()) {
            renderer.set_viewport(window.width(), window.height());
            root.set_bounds(0, 0, (float)window.width(), (float)window.height());
        }

        // 输入分发到控件树
        root.layout();
        root.dispatch_mouse_move(window.mouse_x(), window.mouse_y());
        if (window.mouse_pressed(0)) root.dispatch_mouse_down(window.mouse_x(), window.mouse_y(), 0);
        if (window.mouse_released(0)) root.dispatch_mouse_up(window.mouse_x(), window.mouse_y(), 0);
        const float sdx = window.scroll_dx();
        const float sdy = window.scroll_dy();
        if (sdx != 0.f || sdy != 0.f) {
            count += (int)sdy;
            root.dispatch_mouse_scroll(window.mouse_x(), window.mouse_y(), sdx, sdy);
        }
        // 键盘：Tab/方向键切换焦点；Enter/Space 激活聚焦控件
        const int nav_keys[] = {ui::KeyTab, ui::KeyUp, ui::KeyDown, ui::KeyLeft, ui::KeyRight};
        for (int nav : nav_keys) {
            if (window.key_pressed(nav))
                root.dispatch_key_down(nav, window.key_down(ui::KeyLeftShift) ? ui::ModShift : 0);
        }
        if (window.key_pressed(ui::KeyEnter)) root.dispatch_key_down(ui::KeyEnter, 0);
        if (window.key_pressed(ui::KeySpace)) root.dispatch_key_down(ui::KeySpace, 0);
        if (window.key_pressed(ui::KeyBackspace))
            root.dispatch_key_down(ui::KeyBackspace, 0);
        if (window.key_pressed(ui::KeyDelete)) root.dispatch_key_down(ui::KeyDelete, 0);
        if (window.key_pressed(ui::KeyHome)) root.dispatch_key_down(ui::KeyHome, 0);
        if (window.key_pressed(ui::KeyEnd)) root.dispatch_key_down(ui::KeyEnd, 0);
        // 文本输入：本轮收到的字符队列转发给焦点控件
        const int nchar = window.char_pressed_count();
        for (int i = 0; i < nchar; ++i) root.dispatch_char(window.char_pressed_at(i));
        window.clear_chars();

        renderer.begin_frame(theme.window_bg);
        root.draw(renderer);

        // Tooltip（M14）：悬停控件带提示文本时在鼠标旁绘制小面板
        if (ui::Widget* tt = find_tooltip_widget(&root)) {
            const std::string& s = tt->tooltip();
            const float tw = font.measure_text(s.c_str()) + 18.f;
            float tx = window.mouse_x() + 14.f;
            float ty = window.mouse_y() + 16.f;
            if (tx + tw > (float)window.width()) tx = window.mouse_x() - tw - 10.f;
            renderer.draw_rounded_rect(ui::Rect{tx, ty, tw, 26.f}, 4.f,
                                       ui::Vec4{0.05f, 0.05f, 0.06f, 0.95f});
            font.draw_text(renderer, s.c_str(), tx + 8.f, ty + 4.f, theme.text);
        }

        // HUD：右上角 FPS + 计数
        ++frames;
        auto now = std::chrono::steady_clock::now();
        char hud[128];
        if (now - last >= std::chrono::seconds(1)) {
            double fps = frames / std::chrono::duration<double>(now - last).count();
            frames = 0;
            last = now;
            std::snprintf(hud, sizeof(hud), "fps %.1f | count %d", fps, count);
        } else {
            std::snprintf(hud, sizeof(hud), "count %d", count);
        }
        font.draw_text(renderer, hud, 1000.f, 24.f, theme.text_dim);

        window.swap_buffers();
    }
    return 0;
}
