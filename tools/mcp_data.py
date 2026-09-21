# -*- coding: utf-8 -*-
"""
opengl_ui MCP 知识服务数据（人工维护）——模板 / 脚手架 / 回归约束。

本文件是 `docs/mcp/templates.json` 与 `docs/mcp/constraints.json` 的唯一事实源：

    python tools/api_gen.py            # 连同 catalog.json 一并生成三份 JSON
    python tools/api_gen.py --check    # 校验三份 JSON 与 docs/UI_API.md 均最新

规则（AGENTS.md 改动工作流）：
    - 新增/修改模板或约束后重跑 `python tools/api_gen.py`，并把条目标进 docs/CHANGELOG.md。
    - constraints 的每条必须带 `ui_ids`（对应 CHANGELOG 条目 UI-###），防止约束失语义来源。
    - 本文件只放「内容」，解析 / 生成逻辑在 api_gen.py。
"""
import json
import os

# --------------------------------------------------------------------------
# TEMPLATES：可改装的 C++ 用法模板（category 与组件分类对齐；uses=用到的组件/入口）
# --------------------------------------------------------------------------
TEMPLATES = {
    "window-skeleton": {
        "title": "最小应用骨架（推荐入口 WindowContainer + Scene::run）",
        "category": "framework",
        "uses": ["WindowContainer", "Scene", "Font", "Theme", "Button", "Label"],
        "code": r"""// 推荐入口：WindowContainer 收敛「窗口 + 渲染器 + 自绘标题栏 + Scene」。
// Scene::run() 内部完成 poll → frame（layout/输入分发/advance/绘制）→ swap，
// 并自动冲刷 destroy() 队列、处理弹窗自动关闭与 CJK 字体续帧烘焙（勿手写五段主循环）。
#include "ui/all.h"

int main() {
    ui::Font font;
    font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);  // ASCII
    font.load_cjk_font_auto();                                // 中文懒烘培
    font.load_icon_font("iconfont.ttf", 18.f);                // layui 图标字体

    ui::Theme theme = ui::Theme::make_layui();                // make_layui()/dark()/light()

    ui::WindowContainer wc(1280, 800, "My App");
    if (!wc.ok()) return 1;
    wc.titlebar().set_font(&font);
    wc.titlebar().set_theme(&theme);
    wc.titlebar().set_icon(&font, ui::layui_icon::app);
    wc.scene().set_root_background(theme.window_bg);

    // 内容控件以 content_origin_y()（=标题栏高）为单一原点派生，禁止硬编码偏移。
    const float y = wc.content_origin_y();
    auto* btn = new ui::Button(0.f, y, 120.f, 40.f, "Click Me");
    btn->set_font(&font);
    btn->set_theme(&theme);
    btn->add_class("layui-btn layui-btn-primary");
    btn->set_on_click([] { ui::log_info("Button clicked!"); });
    wc.root().add_child(btn);

    wc.run();
    return 0;
}
""",
    },
    "hand-roll-loop": {
        "title": "裸 Window + Scene 自定义主循环（谨慎，需自行履行生命周期契约）",
        "category": "framework",
        "uses": ["Window", "Renderer", "Scene", "Font", "Widget"],
        "code": r"""// 仅当需要完全自绘/自管时才手写循环。必须履行的契约（违反会在运行期静默复现历史 Bug）：
//   ① set_chrome() 需在首帧 frame() 之前（UI-153/168）；② 每帧 renderer.begin_frame();
//   ③ 帧末 root.advance(dt) 或 Widget::flush_pending_destroys()（否则 destroy() 永不释放，UI-161/167）；
//   ④ 尺寸变化后 resized() -> renderer.set_viewport + root 重排；⑤ scissor 必须配对（UI-057/169）。
#include "ui/all.h"

int main() {
    ui::Window window(1280, 900, "demo");
    if (!window.handle()) return 1;
    ui::Renderer renderer;
    if (!renderer.init()) return 1;
    renderer.set_viewport(window.width(), window.height());

    ui::Font font;
    font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
    font.load_cjk_font_auto();

    ui::Theme theme = ui::Theme::make_layui();
    ui::Widget root;                                     // 根节点可栈对象
    root.set_bounds(0, 0, (float)window.width(), (float)window.height());

    auto* btn = new ui::Button(0, 0, 120, 40, "OK");
    btn->set_font(&font);
    btn->set_theme(&theme);
    root.add_child(btn);

    while (!window.should_close()) {
        window.poll_events();
        if (window.resized())
            renderer.set_viewport(window.width(), window.height());
        root.layout();
        root.dispatch_mouse_move(window.mouse_x(), window.mouse_y());
        if (window.mouse_pressed(0))
            root.dispatch_mouse_down(window.mouse_x(), window.mouse_y(), 0);
        if (window.mouse_released(0))
            root.dispatch_mouse_up(window.mouse_x(), window.mouse_y(), 0);
        renderer.begin_frame(theme.window_bg);
        root.draw(renderer);
        root.advance(1.f / 60.f);                        // 冲刷销毁队列 + 动画/计时器
        window.swap_buffers();
    }
    return 0;
}
""",
    },
    "table-page": {
        "title": "数据表格：列 / 行 / 排序 / 皮肤尺寸 / 双击编辑",
        "category": "data",
        "uses": ["Table", "ScrollBox"],
        "code": r"""// 数据表格（layui-table 外观 + 排序 + 皮肤/尺寸 + 双击行内编辑 + 分页可另开）。
auto* tb = new ui::Table(0, 0, 800, 400);
tb->set_font(&font);
tb->set_theme(&theme);
tb->add_class("layui-table");
tb->add_column("ID", 80.f, /*sortable=*/true);
tb->add_column("姓名", 160.f, /*sortable=*/true);
tb->add_column("状态", 120.f);
tb->add_row({"1", "张三", "在职"});
tb->add_row({"2", "李四", "离职"});
tb->set_skin(ui::TableSkin::Line);     // Default/Line/Row/Nob
tb->set_size(ui::TableSize::Normal);   // Normal=38/Lg=50/Sm=30
tb->set_even(true);                    // 偶数行浅底
tb->set_init_sort(0, true);            // 初始按第 0 列升序
tb->set_on_sort([](int col, bool asc) { ui::log_info("sort col=%d asc=%d", col, asc); });
tb->set_on_row_click([](int row) { ui::log_info("click row %d", row); });
tb->set_on_row_double_click([](int row) { ui::log_info("dblclick row %d", row); });
parent->add_child(tb);
// 分页：tb->set_paginated(true, 10); tb->auto_page_size();（按可视高自动页行数）
// 行内开关/复选：tb->set_cell_switch(col); tb->set_cell_checkbox(col); set_on_cell_form(cb)
// 操作按钮列：tb->add_button_column("操作", 180, {"查看","编辑","删除"}, {"view","edit","del"});
//              tb->set_on_action([](int row, const std::string& ev){});
// 流加载触底：tb->set_on_load_more(cb); tb->set_end_note("— 加载完成 —");
""",
    },
    "form-page": {
        "title": "表单：TextBox / ComboBox / CheckBox / Switch / RadioGroup / 提交按钮",
        "category": "form",
        "uses": ["TextBox", "ComboBox", "CheckBox", "Switch", "RadioGroup", "Button", "VBox"],
        "code": r"""// 表单页布局常用 VBox 纵向堆叠 + HBox 横向排布（先建控件、后 add_child）。
auto* form = new ui::VBox(0, 0, 600, 0);           // h=0 时 preferred_height 自适应
form->set_font(&font);
form->set_theme(&theme);
form->set_spacing(10.f);
form->set_padding(8.f);

auto* name = new ui::TextBox(0, 0, 300, 36, "");
name->set_font(&font);
name->set_theme(&theme);
name->set_placeholder("请输入姓名");
name->set_on_change([](const char* s) { ui::log_info("name=%s", s); });
form->add_child(name);

auto* city = new ui::ComboBox(0, 0, 300, 36, "选择城市");
city->set_font(&font);
city->set_theme(&theme);
city->add_item("北京"); city->add_item("上海"); city->add_item("广州");
city->set_on_change([](int idx) { ui::log_info("city=%d", idx); });
form->add_child(city);

auto* ag = new ui::RadioGroup(0, 0, 300, 28);
ag->set_font(&font);
ag->set_theme(&theme);
ag->add_item("男"); ag->add_item("女");
ag->set_on_change([](const char* v, int idx) { ui::log_info("gender=%s", v); });
form->add_child(ag);

auto* cb = new ui::CheckBox(0, 0, 160, 28, "同意协议");
cb->set_font(&font); cb->set_theme(&theme);
cb->set_on_change([](bool on) { ui::log_info("agree=%d", on); });
form->add_child(cb);

auto* sw = new ui::Switch(0, 0, 56, 28);
sw->set_font(&font); sw->set_theme(&theme);
sw->set_checked(true);
sw->set_on_change([](bool on) { ui::log_info("switch=%d", on); });
form->add_child(sw);

auto* submit = new ui::Button(0, 0, 120, 40, "提交");
submit->set_font(&font); submit->set_theme(&theme);
submit->add_class("layui-btn layui-btn-primary");
submit->set_on_click([] { ui::log_info("submit"); });
form->add_child(submit);

parent->add_child(form);
""",
    },
    "video-player": {
        "title": "视频播放（媒体模块，OPENGUI_MEDIA_FFMPEG=ON 构建）",
        "category": "media",
        "uses": ["VideoView", "Scene", "WindowContainer"],
        "code": r"""// VideoView 为 Widget（ui/media/video_view.h，需 OPENGUI_MEDIA_FFMPEG=ON）；
// 播放进度的推进由每帧 Scene::Frame 订阅里调用 advance(dt) 驱动（不要用主循环自旋）。
#include "ui/media/video_view.h"

// 在 WindowContainer/Scene 就绪后（root = scene.root()）：
auto* vv = new ui::media::VideoView;
vv->set_bounds(0, top, 960, 540);
root.add_child(vv);

if (vv->open_source("C:/path/video.mp4")) {   // 宽字符/中文路径安全
    vv->play();
    vv->set_loop(false);
    vv->set_speed(1.f);                        // 0.5x..2x
    vv->set_volume(0.8f);                      // 0..1
}

scene.subscribe(ui::ScenePhase::Frame, [&] {
    if (vv->advance(1.f / 60.f)) { /* 有帧，纹理已更新 */ }
});
// 控制：vv->pause(); vv->seek(seconds); vv->position(); vv->duration();
// 暂停中 step_frame() 精确推一帧；EOF 且 loop 时自动回卷。
// 其它媒体：ImageView（OPENGUI_MEDIA_OPENCV，ui/media/image_view.h）、AudioOut（winmm）。
""",
    },
    "layer-popup": {
        "title": "layer 弹层：PanelWindow 模态 + 遮罩 + 底部按钮 + 自动关闭",
        "category": "container",
        "uses": ["PanelWindow", "Button"],
        "code": r"""// PanelWindow 即 layer 弹层：拖拽移动 / 右下缩放 / 置顶 / 关闭回调。
auto* pw = new ui::PanelWindow(0, 0, 400, 260, "提示");
pw->set_font(&font);
pw->set_theme(&theme);
pw->set_shade(0.3f);                             // 遮罩覆盖视口
pw->set_title_visible(true);
pw->set_close_visible(true);
pw->set_maxmin(true);                            // 最大化/还原按钮
pw->set_footer_buttons({"取消", "确定"}, 1,       // 1=居中 / 2=右对齐
    [](int idx) { ui::log_info("footer btn %d", idx); });
pw->set_on_close([] { ui::log_info("window closed"); });
pw->add_child(new ui::Label(0, 0, 0, 0, "弹窗内容"));
root.add_child(pw);                              // 挂根（模态由点击触发 set_modal 语义）
// pw->set_auto_close(2.f);   // 秒后自动关闭（需 root.advance(dt) 驱动）
// 注：弹窗控件生命周期随根树回收；页面级弹窗在切页 destroy 钩子统一清理，防悬空引用。
""",
    },
    "scroll-list": {
        "title": "滚动列表：ScrollBox + VBox 流式堆叠 + 触底加载",
        "category": "layout",
        "uses": ["ScrollBox", "VBox", "Label"],
        "code": r"""// 内容超高自动滚动 + scissor 裁剪；配合 VBox 流式排布（追加项 mark_dirty 触发重排）。
auto* sb = new ui::ScrollBox(0, 0, 300, 400);
sb->set_font(&font);
sb->set_theme(&theme);
sb->set_padding(12.f);
sb->set_spacing(8.f);
sb->add_class("layui-side-fixed");               // 或 layui-round 圆角化

auto* list = new ui::VBox(0, 0, 0, 0);
list->set_font(&font); list->set_theme(&theme);
list->set_spacing(4.f);
sb->add_child(list);

for (int i = 0; i < 100; ++i) {                  // 运行期继续 add_child 即时重排
    auto* item = new ui::Label(0, 0, 0, 28, ("第 " + std::to_string(i) + " 条").c_str());
    item->set_font(&font); item->set_theme(&theme);
    list->add_child(item);
}

// 触底加载：距底部不足 mb (=0 严格到底) 触发一次，再滚回底部重新武装
sb->set_on_scroll_to_bottom([] { ui::log_info("reached bottom"); });
sb->set_bottom_margin(20.f);
// 视口锚定（不参与流式排布/滚动的固定条，如“返回顶部”）：
// sb->add_pinned(new ui::Button(...), 1, 16.f, 16.f);
parent->add_child(sb);
""",
    },
    "grid-page": {
        "title": "layui 栅格：Grid 12 列 span/offset（页面布局骨架）",
        "category": "layout",
        "uses": ["Grid", "Panel"],
        "code": r"""// 12 列基准：子控件带 layui-col-md{n}（span）与 layui-col-md-offset{n}（偏移）。
// 行内 span 满 12 自动换行、offset 计入已用列数；无类仍保持等宽模式。
ui::Grid* row = new ui::Grid(0, 0, 800, 60, 12);   // 12 列等宽基础
row->set_theme(&theme);
row->add_class("layui-row");                        // 行容器不画底色
auto* cell = new ui::Panel(0, 0, 0, 0);
cell->add_class("layui-col-md4 layui-col-md-offset2");  // span 4 + 偏移 2
row->add_child(cell);
parent->add_child(row);

// 栅格列演示色块：未着色格子用 grid-demo 基础色 #79C48C；工具色 layui-bg-* 优先
auto* demo = new ui::Panel(0, 0, 0, 0);
demo->add_class("layui-col-md3 layui-bg-green");    // span 3 + 工具色
row->add_child(demo);
""",
    },
    "dropdown-menu": {
        "title": "下拉菜单：Dropdown 触发器 + 菜单项 / 分隔线 + 选中回调",
        "category": "navigation",
        "uses": ["Dropdown", "Menu"],
        "code": r"""auto* dd = new ui::Dropdown(0, 0, 88, 38, "周边");
dd->set_font(&font);
dd->set_theme(&theme);
dd->add_item("官方网站", [] { ui::log_info("官网"); });
dd->add_separator();
dd->add_item("开源地址", [] { ui::log_info("开源"); });
// 触发器形态/事件/对齐：set_trigger(ui::Dropdown::Trigger)... 
// 自定义内容模板：dd->set_custom_content(scrollbox_containing_menu);
// 回调：dd->set_on_select([](const std::string& title){});
parent->add_child(dd);
""",
    },
    "multi-window": {
        "title": "多窗口：各自独立 GL 上下文 + Scene（逐层创建子窗口）",
        "category": "framework",
        "uses": ["Window", "Renderer", "Scene", "Font", "TitleBar"],
        "code": r"""// 每窗口 = 独立 Window + Renderer + Font + root 或 Scene。
// 绘制某窗口前必须先 window.make_current()；禁止同上下文运行两个 Renderer（UI-146）。
ui::Window win1(800, 600, "w1");
ui::Renderer r1; r1.init();
ui::Widget root1; root1.set_bounds(0, 0, 800, 600);
ui::Font f1; f1.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);

while (!win1.should_close()) {
    win1.poll_events();
    win1.make_current();                       // 关键：切换上下文
    // ... layout/dispatch/begin_frame/draw ...
    win1.swap_buffers();
}
// 更推荐每窗口一个 ui::Scene（生命周期统一驱动）；多窗口循环须两窗口都关才退出。
""",
    },
}

# --------------------------------------------------------------------------
# SCAFFOLDS：多文件工程脚手架（id -> {title, files: {相对路径: 内容}}）
# --------------------------------------------------------------------------
_SCAFFOLD_MAIN = r"""// <APP_NAME> —— 基于 opengl_ui 的应用（WindowContainer 安全入口）
#include "ui/all.h"

int main() {
    ui::Font font;
    font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
    font.load_cjk_font_auto();
    font.load_icon_font("iconfont.ttf", 18.f);

    ui::Theme theme = ui::Theme::make_layui();

    ui::WindowContainer wc(1280, 800, "My App");
    if (!wc.ok()) return 1;
    wc.titlebar().set_font(&font);
    wc.titlebar().set_theme(&theme);
    wc.titlebar().set_icon(&font, ui::layui_icon::app);
    wc.scene().set_root_background(theme.window_bg);

    const float y = wc.content_origin_y();
    auto* btn = new ui::Button(0.f, y, 120.f, 40.f, "Click Me");
    btn->set_font(&font);
    btn->set_theme(&theme);
    btn->add_class("layui-btn layui-btn-primary");
    btn->set_on_click([] { ui::log_info("Button clicked!"); });
    wc.root().add_child(btn);

    wc.run();
    return 0;
}
"""

_SCAFFOLD_CMAKE = r"""cmake_minimum_required(VERSION 3.24)
project(<APP_NAME> LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指向 opengl_ui 分发 install/ 目录
list(APPEND CMAKE_PREFIX_PATH "<OPENGUI_INSTALL_DIR>")
find_package(opengl_ui REQUIRED)

add_executable(<APP_NAME> main.cpp)
target_link_libraries(<APP_NAME> PRIVATE opengl_ui::ui_ui)

# 若启用媒体构建（FFmpeg=ON/OpenCV=ON 的库），追加：
# target_link_libraries(<APP_NAME> PRIVATE opengl_ui::ui_media_ffmpeg opengl_ui::ui_media_opencv)
"""

SCAFFOLDS = {
    "app": {
        "title": "最小应用工程（无媒体）",
        "files": {
            "CMakeLists.txt": _SCAFFOLD_CMAKE,
            "main.cpp": _SCAFFOLD_MAIN,
            "README.md": "# <APP_NAME>\n\n基于 opengl_ui 的最小应用。构建后请把 glfw3.dll 拷至 exe 目录（或走 install 的 bin/）。\n",
        },
    },
    "app-media": {
        "title": "带媒体能力（FFmpeg/OpenCV）的应用工程",
        "files": {
            "CMakeLists.txt": "cmake_minimum_required(VERSION 3.24)\n"
            "project(<APP_NAME> LANGUAGES CXX)\n\n"
            "set(CMAKE_CXX_STANDARD 17)\n"
            "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n"
            'list(APPEND CMAKE_PREFIX_PATH "<OPENGUI_INSTALL_DIR>")\n'
            "find_package(opengl_ui REQUIRED)\n\n"
            "add_executable(<APP_NAME> main.cpp)\n"
            "target_link_libraries(<APP_NAME> PRIVATE opengl_ui::ui_ui\n"
            "    opengl_ui::ui_media_ffmpeg opengl_ui::ui_media_opencv)\n",
            "main.cpp": ('// 媒体应用：video_player / image_viewer 模式见模板 opengui_get_template("video-player")。\n'
                         '#include "ui/all.h"\nint main() { return 0; }\n'),
        },
    },
}

# --------------------------------------------------------------------------
# CONSTRAINTS：回归约束（从 docs/ARCHITECTURE.md 回归清单 + CHANGELOG 提炼）
# 每条必须带 ui_ids（CHANGELOG 条目），防约束失语义来源。
# --------------------------------------------------------------------------
CONSTRAINTS = [
    {
        "id": "C001",
        "rule": "文本垂直居中必须用 Font::line_height()（= ascent−descent，descent 为负），禁止 (ascent+descent) 写法（少算 2×|descent|）；draw_text 字形左上角需 std::round 对齐整数像素。",
        "ui_ids": ["UI-053", "UI-055"],
        "danger": "文本下移约 4px / 文字发虚",
    },
    {
        "id": "C002",
        "rule": "Renderer::init() 在 GL 上下文就绪后调用一次；每帧绘制前必须 begin_frame()（内部清空/关 scissor）；push_scissor/pop_scissor 必须配对，退化矩形 push 静默不入栈。",
        "ui_ids": ["UI-008", "UI-057", "UI-169"],
        "danger": "滚动内容穿透标题栏/Header、跨帧脏裁剪",
    },
    {
        "id": "C003",
        "rule": "推荐 ui::WindowContainer + Scene::run()/run_for()。禁/慎手写五段主循环；若手写：帧末必须 root.advance(dt) 或 Widget::flush_pending_destroys()，否则 destroy() 控件永不释放（泄漏 + 僵尸模态）。set_chrome() 须在首帧 frame() 之前。",
        "ui_ids": ["UI-146", "UI-157", "UI-161", "UI-167", "UI-168", "UI-153"],
        "danger": "内存泄漏、控件不重排、(0,0) 闪帧、标题条与内容错位",
    },
    {
        "id": "C004",
        "rule": "Widget 容器不拥有子树，root 子节点可为栈对象：释放子树用 Widget::clear_children()（递归 delete），单个控件用 destroy()（调用后指针失效）；禁止对栈对象 delete（堆损坏 0xC0000374）。Scene 清理只清库内自管对象，禁止 clear_children()。",
        "ui_ids": ["UI-146", "UI-032", "UI-040"],
        "danger": "double-free / 堆损坏崩溃",
    },
    {
        "id": "C005",
        "rule": "draw_quad 为 radius=0 的 draw_rounded_rect 别名，SDF 必须含内部距离项 min(max(q.x,q.y),0)（缺则内部 50% 半透明、全界面灰蒙蒙）；圆角描边必须 draw_rounded_rect_outline 中空环（a_out*(1-a_in)），禁止回退矩形描边。",
        "ui_ids": ["UI-004", "UI-037", "UI-038", "UI-053", "UI-054"],
        "danger": "半透明灰蒙 / 描边内部被整块盖满",
    },
    {
        "id": "C006",
        "rule": "帧级门控渲染：Widget::draw() 全树绘制（脏标记仅作门控）；事件回调内直接改状态必须显式 mark_dirty() 自报（状态写自报金芯）；三处 draw 根节点入口带（dirty_‖dirty_children_）门控 layout，防事件阶段新建控件 (0,0) 闪帧。",
        "ui_ids": ["UI-135", "UI-137", "UI-138", "UI-154", "UI-156", "UI-157", "UI-175"],
        "danger": "开关/下拉/输入需要多次交互才刷新",
    },
    {
        "id": "C007",
        "rule": "CJK 字形惰性烘培按 16ms 配额限量（默认 12 字形）；UI_125 起懒烘培超标帧回退方块宽、后续帧补齐；首显防闪用 Font::prebake_text()/Table::prebake_visible_text() 预热。中文字体须 load_cjk_font* 后才可绘制非 ASCII。",
        "ui_ids": ["UI-036", "UI-125", "UI-127", "UI-136"],
        "danger": "首显方块字 / 单帧卡顿 / 中文不显示",
    },
    {
        "id": "C008",
        "rule": "多窗口：每窗口独立 GL 上下文 + Scene；绘制前 window.make_current()；禁止同上下文运行两个 Renderer（实例级缓存冲突）。",
        "ui_ids": ["UI-146"],
        "danger": "GL 状态串扰/错误绘制",
    },
    {
        "id": "C009",
        "rule": "media 模块默认 OFF；开启须 media ON 且 -DCMAKE_BUILD_TYPE=Release（OpenCV 仅发布库，Debug 下 debug_build_guard 命名空间错位 → LNK2019）；VideoView 头不暴露 FFmpeg C 头。",
        "ui_ids": ["UI-010", "UI-011", "UI-021"],
        "danger": "LNK2019 / 头文件污染",
    },
    {
        "id": "C010",
        "rule": "浅色底上文本不得用近白 token：默认 Theme 结构 text 近白、panel 恒白——白底面板菜单项/折叠标题等须用 text_secondary（深色）绘制（UI-201）；容器 layui-bg-* 类由 table 用主题化着色（UI-203）。",
        "ui_ids": ["UI-201", "UI-203"],
        "danger": "白字白底不可读",
    },
    {
        "id": "C011",
        "rule": "字体/图标：ASCII 与图标（U+E600..ECFF + U+1000..100F）全量烘培，stbtt_BakeFontBitmap 返回值为 bottom_y 非字数；advance 不得为 0；图集 2x 过采样（pixel_height×2）。emoji 回退链防 .notdef tofu 入图集。",
        "ui_ids": ["UI-046", "UI-056", "UI-128", "UI-026"],
        "danger": "字形缺失（如 '6/12' 只渲染 '/1'）",
    },
    {
        "id": "C012",
        "rule": "状态写自报标脏（金芯）：Switch/ComboBox/Slider/TextBox/TextArea/Menu/Transfer/ScrollBox 等事件内直接改成员状态必须显式 mark_dirty()；ScrollBox/Code 覆写 draw 末尾必须清自身脏（dirty_=dirty_children_=false）。",
        "ui_ids": ["UI-137", "UI-138", "UI-139", "UI-140", "UI-141", "UI-156", "UI-158"],
        "danger": "状态变更不即时重绘",
    },
    {
        "id": "C013",
        "rule": "子控件 bounds 为绝对坐标：Header/TitleBar 子项必须叠加父 bounds_.y 偏移；内容区原点以 Scene::content_origin_y() 为单一来源派生，禁止硬编码标题条高偏移。",
        "ui_ids": ["UI-149", "UI-151", "UI-153", "UI-155"],
        "danger": "与标题条重叠 / 顶部导航错位",
    },
    {
        "id": "C014",
        "rule": "模态/捕获：set_modal() 后事件仅路由给模态子树；dispatch 模态分支须判 modal_->visible（隐藏即解除拦截，防僵尸模态死锁）；on_mouse_down 返回 true 即捕获；移出窗口/失焦按下不释放由 Window SetCapture + settle_mouse_reset 自愈（应用侧勿手工清 pressed_）。",
        "ui_ids": ["UI-009", "UI-019", "UI-133", "UI-184", "UI-185"],
        "danger": "输入锁死 / pressed 悬死",
    },
    {
        "id": "C015",
        "rule": "图片等运行时资产：裸文件名加载先 CWD、miss 回退 exe 目录，勿在应用内堆叠 CWD 相对路径回退链（Image::load_from_file 自带 exe 回退）；POST_BUILD 拷 logo.png/iconfont.ttf/glfw3.dll 到 exe 目录。",
        "ui_ids": ["UI-047", "UI-048"],
        "danger": "资产找不到 / 加载噪音错误",
    },
]


def dump_templates() -> str:
    return json.dumps(
        {"kind": "opengui_templates", "templates": TEMPLATES,
         "scaffolds": SCAFFOLDS},
        ensure_ascii=False, indent=2)


def dump_constraints() -> str:
    return json.dumps(
        {"kind": "opengui_constraints", "constraints": CONSTRAINTS},
        ensure_ascii=False, indent=2)


def mcp_data_dir() -> str:
    return os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "docs", "mcp")