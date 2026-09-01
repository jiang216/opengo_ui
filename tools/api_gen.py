#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate docs/UI_API.md — UI 库全 API 手册（方法清单 + 调用示例 + 功能注释 + 调用依赖）。

用法：
    $env:PYTHONIOENCODING='utf-8'
    python tools/api_gen.py            # 重新生成 docs/UI_API.md（覆盖）
    python tools/api_gen.py --check    # 仅校验当前文件与生成结果一致（不一致 exit 1）

文档内容：
    - 方法 / 数据成员 / 常量 / 枚举清单：从 ui/include、core/include、gl/include、
      media/*/include 各公开头文件自动抽取（签名 + 头内注释）。
    - 调用示例 / 依赖说明：本文件 EXAMPLES 与 NOTES 字典（人工维护），生成时嵌入各节。
    - 文档为 AUTO-GENERATED，禁手改（内容统一在本文件维护）。
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "docs", "UI_API.md")

# --------------------------------------------------------------------------
# 模块清单：(显示名, include 前缀, include 根目录相对仓库的路径)
# --------------------------------------------------------------------------
MODULES = [
    ("ui_core（纯 C++ 基础）", "ui/core", "core/include"),
    ("ui_gl（渲染 / 窗口 / 字体）", "ui/gl", "gl/include"),
    ("ui/ui 组件", "ui/ui", "ui/include"),
    ("ui_media（可选模块）", "ui/media", "media/ffmpeg/include"),
    ("ui_media（可选模块）", "ui/media", "media/opencv/include"),
]

# 各头文件相对 include 根的顺序（决定目录小节内顺序）
CORE_ORDER = ["keycodes.h", "log.h", "math.h", "version.h"]
GL_ORDER = ["window.h", "renderer.h", "texture.h", "font.h"]
UI_ORDER = [
    "widget.h", "scene.h", "theme.h", "layui_style.h", "layui_icons.h",
    "badge.h", "blockquote.h", "breadcrumb.h", "button.h", "card.h",
    "carousel.h", "checkbox.h", "circle.h", "code.h", "collapse.h",
    "colorpicker.h", "combobox.h", "countdown_text.h", "dropdown.h",
    "fieldset.h", "grid.h", "hbox.h", "hline.h", "icon.h", "image.h", "label.h",
    "laydate.h", "laypage.h", "laytpl.h", "menubar.h", "menu.h", "navmenu.h", "panel.h",
    "panelwindow.h", "progress.h", "radiobutton.h", "radiogroup.h",
    "rate.h", "scrollbox.h", "slider.h", "switch.h", "table.h",
    "tabs.h", "textbox.h", "timeline.h", "titlebar.h", "transfer.h", "tree.h",
    "upload.h", "vbox.h", "window_frame.h",
]

# --------------------------------------------------------------------------
# 人工维护：调用依赖说明（key = include 前缀路径，如 "ui/ui/button.h"）
# --------------------------------------------------------------------------
NOTES = {
    "ui/gl/window.h": "前置：无。`Window` 需先于 `Renderer::init()` 创建（提供 GL 3.3 Core 上下文）。",
    "ui/gl/renderer.h": "前置：GL 上下文就绪后调用 `init()` 一次；每次绘制前 `begin_frame()`；窗口缩放时 `set_viewport()`。",
    "ui/gl/texture.h": "前置：GL 上下文就绪（Renderer init 之后）。",
    "ui/gl/font.h": "前置：GL 上下文就绪；`draw_text/draw_icon/measure_*` 依赖 Renderer。中文字体需 `load_cjk_font*` 后才可绘制非 ASCII。",
    "ui/ui/widget.h": "前置：无。根节点由应用持有，`add_child` 的控件为裸指针、由根节点或布局容器接管释放（见 `clear_children`/`destroy` 注释）。",
    "ui/ui/scene.h": "前置：需要 `ui::Window`（已创建）+ `Renderer::init()`（GL 3.3 Core）。Scene 随窗口同步创建、构造即广播 Ready；每窗口一个 Scene 绑定同一 Window/Renderer/root。清理重置只清库内自管对象（flush_pending_destroys + close_popups），不裁子树——子节点可为栈对象。",
    "ui/ui/theme.h": "前置：无。把 `Theme` 对象地址经 `set_theme(&theme)` 分发给控件，切换主题时重建同一地址即可即时生效。",
    "ui/ui/layui_style.h": "前置：无。供绘制层查询 layui 工具色；`hex()` 十六进制转 Vec4。",
    "ui/ui/layui_icons.h": "前置：配合 `Font::load_icon_font()`（iconfont.ttf）与 `Font::draw_icon()` 使用。",
    "ui/ui/grid.h": "前置：`set_theme()`；子控件带 `layui-col-*` 类时自动进入 12 列栅格模式；跨断点演示见 layui_test grid_page。",
}

# --------------------------------------------------------------------------
# 人工维护：调用示例（key = include 前缀路径）
# --------------------------------------------------------------------------
EXAMPLES = {
    "ui/gl/window.h":
"""```cpp
ui::Window window(1280, 900, "demo");
if (!window.handle()) return 1;
while (!window.should_close()) {
    window.poll_events();          // 更新鼠标/键盘/滚轮/字符队列（边沿触发）
    // ... root.layout(); root.dispatch_*(); renderer.begin_frame(...);
    window.swap_buffers();
}
```""",
    "ui/gl/renderer.h":
"""```cpp
ui::Renderer renderer;
if (!renderer.init()) return 1;            // GL 上下文就绪后一次
renderer.begin_frame(theme.window_bg);     // 每帧清屏
renderer.draw_quad({10, 10, 200, 60}, ui::Vec4{1, 0, 0, 1});   // radius=0 别名
renderer.draw_rounded_rect({10, 80, 200, 60}, 8.f, theme.accent);
renderer.draw_rect_outline({10, 150, 200, 60}, 2.f, theme.border);
renderer.draw_textured_quad(rect, tex.id(), {0,0,1,1}, ui::Vec4{1,1,1,1});
renderer.draw_polygon(pts, n, color);
renderer.push_scissor({0, 0, 400, 300});   // 可嵌套
renderer.pop_scissor();
```""",
    "ui/gl/texture.h":
"""```cpp
ui::Texture tex;
tex.load_from_file("sprite.png");        // stb_image 解码
tex.create(320, 240, rgba);              // 或手工建纹理
tex.update(rgba);                        // 整图更新（尺寸须一致，视频逐帧用）
tex.update_sub(x, y, w, h, rgba);        // 局部更新（动态字体图集用）
renderer.draw_textured_quad({0,0,320,240}, tex.id(), {0,0,1,1}, ui::Vec4{1,1,1,1});
```""",
    "ui/gl/font.h":
"""```cpp
ui::Font font;
font.load_from_file("C:/Windows/Fonts/arial.ttf", 18.f);
font.load_cjk_font_auto();               // 懒烘培中文字体（环境变量 OPENGUI_CJK_FONT 可自定义）
font.load_icon_font("iconfont.ttf", 18.f);
font.draw_text(renderer, "你好 UI", 10, 10, theme.text);
float w = font.measure_text("宽度");
font.draw_icon(renderer, ui::layui_icon::ic_about, cx, cy, 18.f, theme.success);
```""",
    "ui/ui/widget.h":
"""```cpp
ui::Widget root;                        // 根节点（应用持有栈对象）
root.set_bounds(0, 0, w, h);

ui::Button* b = new ui::Button(0, 0, 100, 36, "OK");
b->add_class("layui-btn layui-btn-primary");   // CSS 类名机制
root.add_child(b);                      // 裸指针，由树接管

// 每帧：先布局，再分发事件，最后绘制
root.layout();
root.dispatch_mouse_move(x, y);
root.dispatch_key_down(key, mods);
root.draw(renderer);

// 生命周期：销毁子树用 clear_children()（递归释放）；销毁单控件及其子树用 destroy()
root.clear_children();
b->destroy();
```""",
    "ui/ui/scene.h":
"""```cpp
ui::Window window(1280, 900, "demo");
ui::Renderer renderer; renderer.init();
ui::Widget root; root.set_bounds(0, 0, 1280, 900);

ui::Scene scene(&window, &renderer, &root);   // 随窗口同步创建，构造广播 Ready
scene.run();                                  // poll + 分配(layout/dispatch/advance) + 消费(begin_frame+draw) + swap

// 或自定义主循环：run_for(秒) 承载 --smoke 通用语义
scene.run_for(2.0f);

// 全量订阅：阶段向量广播 + 逐事件钩子
scene.subscribe(ui::ScenePhase::Frame, [&]{ /* 每帧推进媒体/动画 */ });
scene.subscribe(ui::ScenePhase::PostDraw, [&]{ renderer.draw_text(...); }); // 应用直绘 HUD
scene.on(ui::SceneEvent::Key, [&](int key, int mods){ if (key==UI_KEY_ESCAPE) { scene.request_exit(); } return false; });

// 清理重置只清库内自管对象；子节点可为栈对象，严禁对子树 delete
```""",
    "ui/ui/theme.h":
"""```cpp
ui::Theme theme = ui::Theme::make_layui();   // 或 make_dark()/make_light()
// 分发同一地址：切换主题时重建该对象内容即可全局即时生效
button->set_theme(&theme);
label->set_theme(&theme);
label->set_color(theme.text_secondary);
```""",
    "ui/ui/layui_style.h":
"""```cpp
ui::Vec4 c = ui::layui::hex("#16baaa");          // 十六进制转色
ui::Vec4 bg = ui::layui::bg_fill("layui-bg-green");  // 工具色背景
ui::Vec4 fg = ui::layui::bg_color("layui-bg-red");    // 工具色前景（白字）
bool has = ui::layui::has_class(widget, "layui-btn");
```""",
    "ui/ui/layui_icons.h":
"""```cpp
// 图标码点常量（190 项，layui-iconfont.ttf 私用区 U+E600..E7FF）
font.load_icon_font("iconfont.ttf", 18.f);
font.draw_icon(renderer, ui::layui_icon::ic_about, cx, cy, 18.f, ui::Vec4{1,1,1,1});
```""",
    "ui/ui/badge.h":
"""```cpp
auto* b = new ui::Badge(0, 0, "新");      // 默认徽章
b->set_theme(&theme);
b->add_class("layui-badge layui-bg-orange");   // rim 描边用 layui-badge-rim，dot 用 layui-badge-dot
parent->add_child(b);
```""",
    "ui/ui/blockquote.h":
"""```cpp
auto* q = new ui::BlockQuote(0, 0, 600, 60, "注意：引用提示文字");
q->set_font(&font);
q->set_theme(&theme);
q->set_wrap(true);
parent->add_child(q);
```""",
    "ui/ui/breadcrumb.h":
"""```cpp
auto* bc = new ui::Breadcrumb(0, 0, 600, 30);
bc->set_font(&font);
bc->set_theme(&theme);
bc->add_item("首页");
bc->add_item("组件");
bc->add_item("当前", /*active=*/true);
parent->add_child(bc);
```""",
    "ui/ui/button.h":
"""```cpp
auto* b = new ui::Button(0, 0, 120, 40, "提交");
b->set_font(&font);
b->set_theme(&theme);
b->add_class("layui-btn layui-btn-danger layui-btn-lg");  // 变体/尺寸类名
b->set_on_click([] { ui::log_info("clicked"); });
parent->add_child(b);
```""",
    "ui/ui/card.h":
"""```cpp
auto* c = new ui::Card(0, 0, 400, 200);
c->set_font(&font);
c->set_theme(&theme);
c->set_title("卡片标题");
ui::Label* body = c->body();            // body 为内容容器（布局/添加子控件）
body->add_child(new ui::Label(0, 0, 0, 0, "卡片内容"));
parent->add_child(c);
```""",
    "ui/ui/carousel.h":
"""```cpp
auto* car = new ui::Carousel(0, 0, 800, 300);
car->set_font(&font);
car->set_theme(&theme);
car->add_item("图一");
car->add_item("图二");
car->add_item("图三");
parent->add_child(car);
```""",
    "ui/ui/checkbox.h":
"""```cpp
auto* cb = new ui::CheckBox(0, 0, 120, 28, "阅读协议");
cb->set_font(&font);
cb->set_theme(&theme);
cb->set_checked(true);
cb->set_on_change([](bool on) { ui::log_info(on ? "勾选" : "取消"); });
parent->add_child(cb);
```""",
    "ui/ui/circle.h":
"""```cpp
auto* c = new ui::Circle(0, 0, 120, 120);
c->set_theme(&theme);
c->set_value(0.72f);                    // 0..1 填充比例
parent->add_child(c);
```""",
    "ui/ui/code.h":
"""```cpp
auto* code = new ui::Code(0, 0, 600, 300);
code->set_font(&font);
code->set_theme(&theme);
code->set_lang("C++");
code->add_line("int main() { return 0; }");
parent->add_child(code);
code->clear();                          // 清空重填
```""",
    "ui/ui/collapse.h":
"""```cpp
auto* col = new ui::Collapse(0, 0, 600, 40);
col->set_font(&font);
col->set_theme(&theme);
col->add_item("设计哲学", ui::Collapse::kUnfolded);
col->add_item("从心开始");
col->set_on_change([](int idx, bool open) {});
parent->add_child(col);
```""",
    "ui/ui/colorpicker.h":
"""```cpp
auto* cp = new ui::ColorPicker(0, 0, 160, 36);
cp->set_theme(&theme);
cp->set_color(ui::Vec4{0.09f, 0.72f, 0.47f, 1.0f});
cp->set_on_change([](const ui::Vec4& c) { ui::log_info("选色"); });
parent->add_child(cp);
```""",
    "ui/ui/combobox.h":
"""```cpp
auto* cb = new ui::ComboBox(0, 0, 200, 36, "请选择");
cb->set_font(&font);
cb->set_theme(&theme);
cb->add_item("北京", [] { ui::log_info("选中北京"); });
cb->add_item("上海", nullptr);
parent->add_child(cb);
```""",
    "ui/ui/countdown_text.h":
"""```cpp
auto* cd = new ui::CountdownText(0, 0, 0, 0, "距离结束还有 10 秒");
cd->set_font(&font);
cd->set_theme(&theme);
cd->start();                            // 每帧自动走秒
parent->add_child(cd);
```""",
    "ui/ui/dropdown.h":
"""```cpp
auto* dd = new ui::Dropdown(0, 0, 88, 38, "周边");
dd->set_font(&font);
dd->set_theme(&theme);
dd->add_item("官方网站", [] {});
dd->add_separator();
dd->add_item("开源地址", [] {});
parent->add_child(dd);
```""",
    "ui/ui/fieldset.h":
"""```cpp
auto* fs = new ui::FieldsetTitle(0, 0, 600, 40, "区块标题");
fs->set_font(&font);
fs->set_theme(&theme);
parent->add_child(fs);
```""",
    "ui/ui/grid.h":
"""```cpp
ui::Grid* row = new ui::Grid(0, 0, 800, 40, 12);   // 12 列等宽基础
row->set_theme(&theme);
auto* cell = new ui::Panel(0, 0, 0, 0);
cell->add_class("layui-col-md4 layui-col-md-offset2");  // span 4 + 偏移 2
row->add_child(cell);
parent->add_child(row);
```""",
    "ui/ui/hbox.h":
"""```cpp
auto* box = new ui::HBox(0, 0, 800, 60);
box->set_spacing(10.f);
box->set_padding(8.f);
box->add_child(new ui::Button(0, 0, 100, 36, "A"));
box->add_child(new ui::Button(0, 0, 100, 36, "B"));
parent->add_child(box);
```""",
    "ui/ui/hline.h":
"""```cpp
auto* line = new ui::HLine(0, 0, 800, 1);      // 细分隔线
line->set_color(theme.border);
parent->add_child(line);
```""",
    "ui/ui/icon.h":
"""```cpp
auto* ic = new ui::Icon(0, 0, 18.f, ui::layui_icon::ic_about);
ic->set_font(&font);
ic->set_color(theme.success);
parent->add_child(ic);
```""",
    "ui/ui/label.h":
"""```cpp
auto* label = new ui::Label(0, 0, 600, 0, "多行\\n文本");
label->set_font(&font);
label->set_theme(&theme);
label->set_color(theme.text);
label->set_wrap(true);                   // 超宽自动换行
parent->add_child(label);
```""",
    "ui/ui/laydate.h":
"""```cpp
auto* date = new ui::Laydate(0, 0, 220, 36, "日期");
date->set_font(&font);
date->set_theme(&theme);
date->set_on_change([](const char* v) { ui::log_info("选中 %s", v); });
parent->add_child(date);
```""",
    "ui/ui/laypage.h":
"""```cpp
auto* page = new ui::Laypage(0, 0, 400, 36);
page->set_font(&font);
page->set_theme(&theme);
page->set_pages(20);
page->set_on_change([](int p) { ui::log_info("第 %d 页", p); });
parent->add_child(page);
```""",
    "ui/ui/laytpl.h":
"""```cpp
ui::LayTplValue data;
ui::laytpl_parse_json("{\\"title\\":\\"Layui\\",\\"list\\":[{\\"m\\":1}]}", data);
ui::LayTpl tpl("{{ d.title }} / {{# layui.each(d.list, function(i,it){ }}{{ it.m }}{{# }); }}");
std::string html = tpl.render(data);
```""",
    "ui/ui/menubar.h":
"""```cpp
auto* mb = new ui::MenuBar(0, 0, 600, 36);
mb->set_font(&font);
mb->set_theme(&theme);
mb->add_menu("菜单", { {"选项A", [] {}}, {"选项B", nullptr} });
parent->add_child(mb);
```""",
    "ui/ui/navmenu.h":
"""```cpp
auto* nav = new ui::NavMenu(0, 0, 200, 400);
nav->set_font(&font);
nav->set_theme(&theme);
nav->add_group("基础");
nav->add_item("按钮", [] {});
nav->set_active(0);
nav->expand_all();
parent->add_child(nav);
```""",
    "ui/ui/panel.h":
"""```cpp
auto* p = new ui::Panel(0, 0, 300, 200);
p->set_theme(&theme);
p->add_child(new ui::Label(0, 0, 0, 0, "面板内容"));
parent->add_child(p);
```""",
    "ui/ui/panelwindow.h":
"""```cpp
auto* w = new ui::PanelWindow(0, 0, 400, 260, "标题");
w->set_font(&font);
w->set_theme(&theme);
w->set_on_close([] { ui::log_info("窗口关闭"); });
w->add_child(new ui::Label(0, 0, 0, 0, "窗口内容"));
parent->add_child(w);
```""",
    "ui/ui/progress.h":
"""```cpp
auto* pg = new ui::Progress(0, 0, 500, 30);
pg->set_theme(&theme);
pg->add_class("layui-progress layui-progress-big");  // 大号进度条
pg->set_value(0.6f);
parent->add_child(pg);
```""",
    "ui/ui/radiobutton.h":
"""```cpp
auto* rb = new ui::RadioButton(0, 0, 80, 28, "男");
rb->set_font(&font);
rb->set_theme(&theme);
parent->add_child(rb);
```""",
    "ui/ui/radiogroup.h":
"""```cpp
auto* rg = new ui::RadioGroup(0, 0, 400, 28);
rg->set_font(&font);
rg->set_theme(&theme);
rg->add_item("男");
rg->add_item("女");
rg->set_on_change([](const char* v, int idx) { ui::log_info("选中 %s", v); });
parent->add_child(rg);
```""",
    "ui/ui/rate.h":
"""```cpp
auto* r = new ui::Rate(0, 0, 300, 30);
r->set_font(&font);
r->set_theme(&theme);
r->set_value(3);
r->set_readonly(false);
r->set_on_change([](int v) {});
parent->add_child(r);
```""",
    "ui/ui/scrollbox.h":
"""```cpp
auto* sb = new ui::ScrollBox(0, 0, 300, 400);
sb->set_theme(&theme);
sb->set_padding(12.f);
sb->set_spacing(8.f);
sb->add_child(child_widget);                 // 内容超高自动滚动 + scissor 裁剪
sb->set_on_scroll_to_bottom([] { ui::log_info("滚到底"); });  // 传 nullptr 清除
parent->add_child(sb);
```""",
    "ui/ui/slider.h":
"""```cpp
auto* sl = new ui::Slider(0, 0, 400, 30);
sl->set_theme(&theme);
sl->set_range(0.f, 100.f);
sl->set_value(60.f);
sl->set_on_change([](float v) {});
sl->set_value_silent(50.f);                  // 程序同步不触发回调
parent->add_child(sl);
```""",
    "ui/ui/switch.h":
"""```cpp
auto* sw = new ui::Switch(0, 0, 56, 28);
sw->set_theme(&theme);
sw->set_checked(true);
sw->set_on_change([](bool on) { ui::log_info(on ? "开" : "关"); });
parent->add_child(sw);
```""",
    "ui/ui/table.h":
"""```cpp
auto* tb = new ui::Table(0, 0, 800, 400);
tb->set_font(&font);
tb->set_theme(&theme);
tb->add_column("ID");
tb->add_column("姓名");
tb->add_row({"1", "张三"});                  // 行 = 与列等长的字符串列表
tb->add_class("layui-table");                // layui 外观类
parent->add_child(tb);
```""",
    "ui/ui/tabs.h":
"""```cpp
auto* tabs = new ui::Tabs(0, 0, 600, 40);
tabs->set_font(&font);
tabs->set_theme(&theme);
tabs->add_tab("预览");
tabs->add_tab("代码");
tabs->set_on_change([](int idx) { ui::log_info("切到 %d", idx); });
parent->add_child(tabs);
```""",
    "ui/ui/textbox.h":
"""```cpp
auto* tb = new ui::TextBox(0, 0, 300, 36);
tb->set_font(&font);
tb->set_theme(&theme);
tb->set_placeholder("请输入");
tb->set_on_change([](const char* s) {});
parent->add_child(tb);
tb->focus();                                 // focusable，可聚焦输入
```""",
    "ui/ui/timeline.h":
"""```cpp
auto* tl = new ui::Timeline(0, 0, 600, 200);
tl->set_theme(&theme);
tl->add_item("第一步", "说明文字");
tl->add_item("第二步");
parent->add_child(tl);
```""",
    "ui/ui/transfer.h":
"""```cpp
auto* tr = new ui::Transfer(0, 0, 700, 260);
tr->set_font(&font);
tr->set_theme(&theme);
tr->add_item("左项1");
tr->add_item("左项2");
parent->add_child(tr);
```""",
    "ui/ui/tree.h":
"""```cpp
auto* tree = new ui::Tree(0, 0, 300, 400);
tree->set_font(&font);
tree->set_theme(&theme);
tree->add_item("父节点", { "子1", "子2" });
tree->set_on_select([](const char* p, const char* name) {});
parent->add_child(tree);
```""",
    "ui/ui/upload.h":
"""```cpp
auto* up = new ui::Upload(0, 0, 300, 60);
up->set_font(&font);
up->set_theme(&theme);
up->add_class("layui-upload layui-upload-drag");  // 拖拽上传样式
up->set_on_select([](const char* path) { ui::log_info("选择 %s", path); });
parent->add_child(up);
```""",
    "ui/ui/vbox.h":
"""```cpp
auto* vb = new ui::VBox(0, 0, 400, 200);
vb->set_padding(10.f);
vb->set_spacing(8.f);
vb->add_child(new ui::Button(0, 0, 0, 36, "上"));   // 子控件等宽填满
vb->add_child(new ui::Button(0, 0, 0, 36, "下"));
parent->add_child(vb);
```""",
    "ui/ui/window_frame.h":
"""```cpp
// 窗口容器：内置自绘标题栏 + 下方内容区，内容子控件用容器相对坐标
auto* wf = new ui::WindowFrame(0, 0, 500, 320);
wf->set_font(&font);
wf->set_theme(&theme);
wf->set_title("窗口标题");
wf->set_titlebar_height(30.f);                    // 标题栏高度（默认 30）
wf->set_content_child(new ui::Panel(0, 0, 0, 0), 10.f, 10.f, 0.f, 0.f);  // 相对 titlebar 底缘
wf->set_on_close([] { ui::log_info("关闭"); });
parent->add_child(wf);
```""",
    "ui/media/video_view.h":
"""```cpp
// 仅当 media 启用（OPENGUI_MEDIA_FFMPEG=ON）时可用
auto* vv = new ui::media::VideoView;
vv->set_bounds(0, 0, 800, 450);
if (!vv->open_source("movie.mp4")) return;
vv->play();
vv->set_loop(true);
vv->set_speed(2.f);                       // 0.1..4 倍速
vv->seek(30.0);
vv->set_volume(0.8f);                     // 0..1 音量
// 主线程每帧：vv->advance(dt)（swap 出帧后内部上传纹理）
// 退出：vv->close_source();
parent->add_child(vv);
```""",
    "ui/media/audio_out.h":
"""```cpp
// winmm 系统播放，无死锁（回调线程不做 waveOutUnprepareHeader）
auto* ao = new ui::media::AudioOut;
if (!ao->open(44100, 2)) return;          // 打开默认输出设备，失败视为静音
ao->write(s16_stereo_samples, count);
ao->flush();                              // seek/循环回卷时清缓冲
// 结束：ao->close();
```""",
    "ui/media/image_view.h":
"""```cpp
// 仅当 media 启用（OPENGUI_MEDIA_OPENCV=ON）时可用
auto* iv = new ui::media::ImageView;
iv->set_bounds(0, 0, 400, 300);
if (!iv->load_from_file("photo.jpg")) return;   // 中文路径 OK
parent->add_child(iv);
```""",
}

# --------------------------------------------------------------------------
# 词法工具
# --------------------------------------------------------------------------

def mask_text(text):
    """注释/字符串置空（保留换行）。返回 (clean, line_of, comments)。
    comments: {a,b,sl,el,pure,frag}  a/b 为区间，sl/el 起止行，pure 是否行首注释，
    frag 注释原文。"""
    n = len(text)
    clean = list(text)
    line_of = [0] * n
    comments = []
    i, ln, ls = 0, 0, 0
    while i < n:
        line_of[i] = ln
        c = text[i]
        if c == '\n':
            ln += 1
            ls = i + 1
            i += 1
            continue
        if c == '/' and i + 1 < n and text[i + 1] == '/':
            j = i
            while j < n and text[j] != '\n':
                j += 1
            comments.append({"a": i, "b": j, "sl": ln, "el": ln,
                             "pure": text[ls:i].strip() == "", "frag": text[i:j]})
            for k in range(i, j):
                clean[k] = ' '
            i = j
            continue
        if c == '/' and i + 1 < n and text[i + 1] == '*':
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            seg = text[i:j]
            el = ln + seg.count('\n')
            comments.append({"a": i, "b": j, "sl": ln, "el": el,
                             "pure": text[ls:i].strip() == "", "frag": seg})
            for k in range(i, j):
                clean[k] = text[k] if text[k] == '\n' else ' '
            i = j
            continue
        if c in ('"', "'"):
            q = c
            j = i + 1
            while j < n:
                if text[j] == '\\':
                    j += 2
                    continue
                if text[j] == q:
                    j += 1
                    break
                j += 1
            for k in range(i, min(j, n)):
                clean[k] = text[k] if text[k] == '\n' else ' '
            i = j
            continue
        i += 1
    return ''.join(clean), line_of, comments


def brace_pairs(clean):
    stack = []
    pairs = {}
    for i, ch in enumerate(clean):
        if ch == '{':
            stack.append(i)
        elif ch == '}' and stack:
            o = stack.pop()
            pairs[o] = i
            pairs[i] = o
    return pairs


def iter_declarations(clean, start, end):
    """在 [start,end) 内按顶层语句切分（括号平衡）。{ 若被判定为函数体则吞到匹配 }；
    花括号初始化（其前无顶层函数括号）不吞。"""
    i = start
    stmt = start
    paren = 0
    seen_fn_paren = False  # 已出现顶层 () 且闭合
    fn_paren_open = -1
    while i < end:
        c = clean[i]
        if c == '(':
            if paren == 0:
                fn_paren_open = i
            paren += 1
        elif c == ')':
            paren -= 1
            if paren == 0:
                seen_fn_paren = True
        elif c == ';' and paren == 0:
            yield (stmt, i + 1)
            stmt = i + 1
            seen_fn_paren = False
            fn_paren_open = -1
        elif c == '{' and paren == 0:
            if seen_fn_paren:
                j = i
                depth = 0
                while j < end:
                    if clean[j] == '{':
                        depth += 1
                    elif clean[j] == '}':
                        depth -= 1
                        if depth == 0:
                            break
                    j += 1
                yield (stmt, j + 1)
                stmt = j + 1
                seen_fn_paren = False
                fn_paren_open = -1
                i = j
            # 花括号初始化：不吞，继续找 ';'
        i += 1
    if stmt < end:
        s = clean[stmt:end].strip()
        if s:
            yield (stmt, end)


def top_level_parens(d):
    out = []
    tdepth = 0
    for i, c in enumerate(d):
        if c == '<':
            tdepth += 1
        elif c == '>':
            tdepth = max(0, tdepth - 1)
        elif c == '(' and tdepth == 0:
            out.append(i)
    return out


def split_decl(decl):
    """返回 (kind, name, sig, value)。kind: method/field/func/const/enumitem/other"""
    d = decl.strip()
    if not d:
        return ("other", "", "", "")
    if d.startswith(("using ", "typedef ", "friend ", "#", "enum ")):
        if d.startswith("enum "):
            return ("other", "", "", "")
        return ("other", "", "", "")
    # 常量（命名空间级 constexpr）
    m = re.match(r'^(constexpr|const)\s+[\w:<>,\s*&]+\s+([A-Za-z_]\w*)\s*(=\s*[^;]+)?;?$', d)
    if m and '=' in d:
        name = m.group(2)
        val = re.search(r'=\s*([^;]+)', d)
        return ("const", name, d, val.group(1).strip() if val else "")
    # 找出所有顶层 '('
    parens = top_level_parens(d)
    if parens:
        pos = parens[0]
        before = d[:pos]
        m = re.search(r'([~A-Za-z_]\w*)\s*$', before)
        name = m.group(1) if m else None
        if name and not re.search(r'(\.|->|::)\s*$', before):
            sig = decl_sig(d)
            return ("method", name, sig, "")
    # 数据成员：无顶层函数括号（含数组/花括号初始化成员）
    if not re.search(r'\b(if|for|while|return|switch|else|case)\b', d):
        mm = re.search(r'([A-Za-z_]\w*)\s*(?=[={\[,;])', d)
        if mm and not re.search(r'\b(class|struct|namespace|enum)\b', d):
            return ("field", mm.group(1), d, "")
    return ("other", "", "", "")


def decl_sig(decl):
    """构造展示签名：取到首个顶层函数括号的匹配 ')' 为止，再附 const/override/= 0 等后缀。"""
    d = decl
    # 找顶层 ( ... ) 的匹配 ')'
    tdepth = 0
    p0 = -1
    depth = 0
    close = -1
    for i, c in enumerate(d):
        if c == '<':
            tdepth += 1
        elif c == '>':
            tdepth = max(0, tdepth - 1)
        elif c == '(' and tdepth == 0:
            if p0 < 0:
                p0 = i
            depth += 1
        elif c == ')' and tdepth == 0 and p0 >= 0:
            depth -= 1
            if depth == 0:
                close = i
                break
    if close < 0:
        return ' '.join(d.split())
    sig = d[:close + 1]
    rest = d[close + 1:]
    m = re.match(r'^\s*(const|override|final|noexcept|=\s*0|=\s*default|=\s*delete|\[[^\]]*\])?', rest)
    tail = (m.group(1) or '') if m else ''
    s = ' '.join((sig + ' ' + tail).split())
    s = re.sub(r'\s*=\s*(default|delete|0)\s*;?$', r' = \1', s)
    return s.strip().rstrip(';').strip()


# --------------------------------------------------------------------------
# 注释关联
# --------------------------------------------------------------------------

def build_comment_index(comments, lines):
    pure = {}
    trailing = {}
    for cm in comments:
        frag_lines = cm["frag"].split('\n')
        for ln in range(cm["sl"], cm["el"] + 1):
            k = ln - cm["sl"]
            frag = frag_lines[k] if k < len(frag_lines) else ''
            if cm["pure"] or k > 0:
                pure.setdefault(ln, []).append(frag)
        if not cm["pure"] and cm["sl"] not in pure:
            trailing.setdefault(cm["sl"], []).append(cm["frag"])
    return pure, trailing


def preceding_comment(pure, lines, start_line):
    parts = []
    ln = start_line - 1
    blank = 0
    while ln >= 0:
        if ln in pure:
            parts = pure[ln] + parts
            blank = 0
            ln -= 1
            continue
        if lines[ln].strip() == '':
            blank += 1
            if blank > 1:
                break
            ln -= 1
            continue
        break
    return ' '.join(clean_comment(p) for p in parts).strip()


def stmt_start_line(clean, line_of, sa, ea):
    while sa < ea and clean[sa] in ' \t\r\n':
        sa += 1
    return line_of[sa]


def strip_comment_markers(t):
    t = re.sub(r'^\s*//\s*', '', t)
    t = re.sub(r'^\s*/\*+\s*', '', t)
    t = re.sub(r'\s*\*+/\s*$', '', t)
    return t.strip()


def clean_comment(t):
    t = t.replace('\n', ' ')
    t = strip_comment_markers(t)
    return t


# --------------------------------------------------------------------------
# 结构扫描
# --------------------------------------------------------------------------

STRUCT_RE = re.compile(r'\b(class|struct)\s+([A-Za-z_]\w*)')
ENUM_RE = re.compile(r'\benum\b(?!\s+(class|struct)\b)\s+([A-Za-z_]\w*)?\s*$')
NS_RE = re.compile(r'\bnamespace\s+([A-Za-z_]\w*)?\s*$')
ACCESS_RE = re.compile(r'^\s*(public|private|protected)\s*:')

def scan_file(path):
    with open(path, 'r', encoding='utf-8') as f:
        text = f.read()
    clean, line_of, comments = mask_text(text)
    pairs = brace_pairs(clean)
    lines = text.split('\n')
    pure, trailing = build_comment_index(comments, lines)
    n = len(clean)

    # 识别结构体（class/struct/enum/namespace）及其花括号区间
    structs = []  # {kind,name,base,open,close,start_line}
    stack = []    # (kind, close_idx)
    i = 0
    while i < n:
        if clean[i] == '{':
            # 往回找语句起点，判定类型
            st = i
            while st > 0 and clean[st - 1] not in ';{}':
                st -= 1
            head = clean[st:i]
            m = STRUCT_RE.search(head)
            if m and not re.search(r'\b(enum|namespace)\b', head[:m.start()]):
                base = head[m.end():]
                base = re.sub(r'\bpublic\b|\bprotected\b|\bprivate\b', '', base)
                structs.append({"kind": m.group(1), "name": m.group(2),
                                "base": ' '.join(base.split()).strip(),
                                "open": i, "close": pairs.get(i, -1),
                                "start_line": line_of[m.start()]})
            else:
                m = ENUM_RE.search(head)
                if m:
                    structs.append({"kind": "enum", "name": m.group(2) or '',
                                    "base": "", "open": i,
                                    "close": pairs.get(i, -1),
                                    "start_line": line_of[m.start()]})
                else:
                    m = NS_RE.search(head)
                    if m:
                        structs.append({"kind": "namespace",
                                        "name": m.group(1) or '',
                                        "base": "", "open": i,
                                        "close": pairs.get(i, -1),
                                        "start_line": line_of[m.start()]})
        i += 1

    # 每个文件解析结果
    result = {"file_comment": "", "includes": [], "classes": [],
              "funcs": [], "enums": [], "consts": []}

    # 文件头注释（首个注释块）
    if comments:
        first = comments[0]
        if first["sl"] == 0 or all(l.strip() == '' for l in lines[:first["sl"]]):
            result["file_comment"] = clean_comment(first["frag"])

    # include
    for m in re.finditer(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', text, re.M):
        result["includes"].append(m.group(1))

    def parse_class(c):
        if c["kind"] == "enum":
            body = clean[c["open"] + 1:c["close"]]
            entries = []
            for m in re.finditer(r'([A-Za-z_]\w*)\s*(=\s*([^,}]+))?', body):
                name = m.group(1)
                if name in ('public', 'private', 'protected'):
                    continue
                if name == 'Key' or name == 'KeyMod':
                    pass
                entries.append({"name": name, "value": m.group(3).strip() if m.group(3) else ""})
            doc = preceding_comment(pure, lines, c["start_line"])
            return {"kind": "enum", "name": c["name"], "doc": doc, "entries": entries}
        if c["kind"] == "namespace":
            return None
        # class / struct：提取 public 区方法/字段
        body_start = c["open"] + 1
        body_end = c["close"]
        items = []  # (kind,name,sig,doc)
        # 定位访问区
        bd = clean[body_start:body_end]
        off = body_start
        ranges = []
        for mm in re.finditer(r'(public|private|protected)\s*:', bd):
            ranges.append((mm.group(1), off + mm.end()))
        if not ranges and c["kind"] == "struct":
            ranges = [("public", body_start)]  # struct 默认全 public
        for k in range(len(ranges)):
            a = ranges[k][1]
            b = ranges[k + 1][1] if k + 1 < len(ranges) else body_end
            if ranges[k][0] != 'public':
                continue
            for (sa, ea) in iter_declarations(clean, a, b):
                decl = clean[sa:ea]
                kind, name, sig, val = split_decl(decl)
                if kind == "other":
                    continue
                sl = stmt_start_line(clean, line_of, sa, ea)
                doc = preceding_comment(pure, lines, sl)
                t = trailing.get(line_of[ea - 1])
                if t:
                    doc = (doc + ' ' + ' '.join(clean_comment(x) for x in t)).strip()
                if kind == "method" and (name.startswith('~') or sig.rstrip().endswith('= delete')):
                    continue
                items.append({"kind": kind, "name": name, "sig": sig,
                              "doc": doc, "value": val, "start_line": sl})
        doc = preceding_comment(pure, lines, c["start_line"])
        return {"kind": c["kind"], "name": c["name"], "base": c["base"],
                "doc": doc, "items": items}

    # 处理命名空间与顶层自由函数 / 常量
    def parse_scope(start, end, depth):
        for (sa, ea) in iter_declarations(clean, start, end):
            decl = clean[sa:ea]
            kind, name, sig, val = split_decl(decl)
            if kind == "other":
                continue
            sl = stmt_start_line(clean, line_of, sa, ea)
            doc = preceding_comment(pure, lines, sl)
            t = trailing.get(line_of[ea - 1])
            if t:
                doc = (doc + ' ' + ' '.join(clean_comment(x) for x in t)).strip()
            if kind in ("func", "method"):
                result["funcs"].append({"name": name, "sig": sig, "doc": doc})
            elif kind == "const":
                result["consts"].append({"name": name, "sig": sig,
                                         "value": val, "doc": doc})

    # 遍历所有结构区间（含命名空间），避开类体，抽取自由函数/常量
    for c in structs:
        parsed = parse_class(c)
        if parsed is None:
            continue
        if parsed["kind"] == "enum":
            result["enums"].append(parsed)
        else:
            result["classes"].append(parsed)

    # 命名空间内 & 文件顶层的自由函数/常量：用 namespace 区间（不含嵌套类体）
    ns_scopes = [c for c in structs if c["kind"] == "namespace"]
    blocks = [c for c in structs if c["kind"] in ("class", "struct", "enum", "namespace")]

    def cut_ranges(start, end, inner):
        """把 [start,end) 切分为未被 inner 结构体占用的自由区间。"""
        cut = [start]
        for c in inner:
            if c["open"] > start and c["close"] < end:
                cut.append(c["open"])
                cut.append(c["close"] + 1)
        cut.append(end)
        return [(cut[k], cut[k + 1]) for k in range(0, len(cut) - 1, 2)]

    # 顶层（不含任何命名空间/类体）
    top_ranges = []
    pos = 0
    for ns in ns_scopes:
        if ns["open"] > pos:
            top_ranges.append((pos, ns["open"]))
        pos = ns["close"] + 1
    top_ranges.append((pos, n))
    for (a, b) in top_ranges:
        for (x, y) in cut_ranges(a, b, blocks):
            parse_scope(x, y, 0)
    # 命名空间内：取每个 ns 体，排除其中的类体区间
    for ns in ns_scopes:
        ns_start = ns["open"] + 1
        ns_end = ns["close"]
        inner = [c for c in structs if c["open"] > ns_start and c["close"] < ns_end]
        for (x, y) in cut_ranges(ns_start, ns_end, inner):
            parse_scope(x, y, 1)

    # 类内常量（namespace 级 constexpr 已在 parse_scope）——layui_icons 常量在命名空间内
    # 但本实现 constexpr 检测对 `constexpr uint32_t ic_404 = 0xE61C;` 有效：
    # 需确认 split_decl 对它的判定。const 匹配要求带 '='；ok。
    return result


# --------------------------------------------------------------------------
# 渲染
# --------------------------------------------------------------------------

def esc(s):
    if s is None:
        return ''
    s = str(s).replace('|', '\\|')
    return s


def row(cells):
    return '| ' + ' | '.join(cells) + ' |'


def fmt_sig(s):
    return '`' + s.replace('`', "'") + '`'


def render_file_section(rel, parsed):
    out = []
    out.append(f"### `{rel}`")
    out.append("")
    ex = EXAMPLES.get(rel)
    if ex:
        out.append("**调用示例**：")
        out.append(ex)
        out.append("")
    note = NOTES.get(rel)
    if note:
        out.append(f"**依赖说明**：{note}")
        out.append("")
    dep = [d for d in parsed["includes"] if not d.startswith(('<', '"'))]
    if dep:
        out.append("**依赖 include**：" + '、'.join('`%s`' % d for d in dep))
        out.append("")
    if parsed["file_comment"]:
        out.append(f"> {parsed['file_comment']}")
        out.append("")
    for cls in parsed["classes"]:
        out.append(f"**类 `{cls['name']}`**")
        if cls["base"]:
            out.append(f"（继承 `{cls['base']}`）")
        if cls["doc"]:
            out.append("")
            out.append(f"> {cls['doc']}")
        methods = [it for it in cls["items"] if it["kind"] == "method"]
        fields = [it for it in cls["items"] if it["kind"] == "field"]
        if methods:
            out.append("")
            out.append(row(["方法", "功能说明"]))
            out.append(row(["---", "---"]))
            for it in methods:
                out.append(row([fmt_sig(it["sig"]), esc(it["doc"] or '')]))
        if fields:
            out.append("")
            out.append(row(["数据成员", "说明"]))
            out.append(row(["---", "---"]))
            for it in fields:
                out.append(row([fmt_sig(it["sig"]), esc(it["doc"] or '')]))
        out.append("")
    if parsed["funcs"]:
        out.append("**自由函数**（命名空间 `ui` / `ui::layui`）：")
        out.append("")
        out.append(row(["函数", "功能说明"]))
        out.append(row(["---", "---"]))
        for f in parsed["funcs"]:
            out.append(row([fmt_sig(f["sig"]), esc(f["doc"] or '')]))
        out.append("")
    if parsed["enums"]:
        for e in parsed["enums"]:
            out.append(f"**枚举 `{e['name']}`**" + (f"：{e['doc']}" if e['doc'] else ''))
            out.append("")
            out.append(row(["常量", "值", "说明"]))
            out.append(row(["---", "---", "---"]))
            for en in e["entries"]:
                out.append(row([f"`{en['name']}`", f"`{en['value']}`", '']))
            out.append("")
    if parsed["consts"]:
        out.append("**常量**：")
        out.append("")
        out.append(row(["常量", "值", "说明"]))
        out.append(row(["---", "---", "---"]))
        for c in parsed["consts"]:
            if c['value'].startswith('0x'):
                val = f"0x{int(c['value'], 16):04X}"
            else:
                val = c['value']
            out.append(row([f"`{c['name']}`", f"`{val}`", esc(c["doc"] or '')]))
        out.append("")
    return out


def build():
    lines = []
    A = lines.append
    A("# UI 库 API 手册")
    A("")
    A("> 本文件为图谱节点 `DOC:UI_API`，AUTO-GENERATED（由 `python tools/api_gen.py` 生成，禁手改）。")
    A("> 功能注释 / 方法签名抽取自各公开头文件；调用示例 / 依赖说明在 `tools/api_gen.py` 的")
    A("> `EXAMPLES` / `NOTES` 字典中维护。新增 API 或改注释后：重新生成本文件并同步 CHANGELOG。")
    A("> 生成器结构化解析缺陷修复记录见 [UI-042](CHANGELOG.md#ui-042-api-文档生成器toolsapigenpy结构化解析修复2026-08-16)。")
    A("")
    A("## 一、用法总览")
    A("")
    A("一条 include 拉入 core / gl / ui/ui 全部公开头：")
    A("")
    A("```cpp")
    A('#include "ui/all.h"')
    A("```")
    A("")
    A("媒体模块（ui_media，可选、默认不编译）不在聚合头内，启用后按需 include：")
    A("")
    A("```cpp")
    A('#include "ui/media/video_view.h"')
    A('#include "ui/media/audio_out.h"')
    A('#include "ui/media/image_view.h"')
    A("```")
    A("")
    A("### 全局入口与主循环")
    A("")
    A("最小应用骨架（完整参考 `layui_test/src/app.cpp`）：")
    A("")
    A("```cpp")
    A('#include "ui/all.h"')
    A("")
    A("ui::Window window(1280, 900, \"demo\");       // ① 窗口（提供 GL 3.3 Core 上下文）")
    A("if (!window.handle()) return 1;")
    A("ui::Renderer renderer;                       // ② 渲染器（须在 GL 上下文就绪后 init）")
    A("if (!renderer.init()) return 1;")
    A("renderer.set_viewport(window.width(), window.height());")
    A("ui::Font font;                               // ③ 字体（ASCII + 中文懒烘培 + 图标）")
    A("font.load_from_file(\"C:/Windows/Fonts/arial.ttf\", 18.f);")
    A("font.load_cjk_font_auto();")
    A("font.load_icon_font(\"iconfont.ttf\", 18.f);")
    A("ui::Theme theme = ui::Theme::make_layui();   // ④ 主题（make_dark/light/layui）")
    A("ui::Widget root;                             // ⑤ 控件树根节点")
    A("root.set_bounds(0, 0, (float)window.width(), (float)window.height());")
    A("ui::Button* btn = new ui::Button(0, 0, 120, 40, \"OK\");")
    A("btn->set_font(&font);")
    A("btn->set_theme(&theme);")
    A("root.add_child(btn);")
    A("")
    A("while (!window.should_close()) {             // ⑥ 主循环：事件 → 布局 → 绘制")
    A("    window.poll_events();")
    A("    if (window.resized()) { renderer.set_viewport(window.width(), window.height());")
    A("        root.set_bounds(0, 0, (float)window.width(), (float)window.height()); }")
    A("    root.layout();")
    A("    root.dispatch_mouse_move(window.mouse_x(), window.mouse_y());")
    A("    if (window.mouse_pressed(0)) root.dispatch_mouse_down(window.mouse_x(), window.mouse_y(), 0);")
    A("    if (window.mouse_released(0)) root.dispatch_mouse_up(window.mouse_x(), window.mouse_y(), 0);")
    A("    if (window.key_pressed(ui::KeyTab)) root.dispatch_key_down(ui::KeyTab, 0);")
    A("    renderer.begin_frame(theme.window_bg);")
    A("    root.draw(renderer);")
    A("    window.swap_buffers();")
    A("}")
    A("```")
    A("")
    A("## 二、分层依赖")
    A("")
    A("依赖方向：`media → ui → gl → core`（单向，禁止反向 include）。")
    A("")
    A("| 层 | 头文件根 | 依赖 |")
    A("| --- | --- | --- |")
    A("| core | `ui/core/*` | 无（纯 C++） |")
    A("| gl | `ui/gl/*` | core + 3rdparty（GLFW/GLAD/stb） |")
    A("| ui | `ui/ui/*` | gl + core |")
    A("| media（可选） | `ui/media/*` | ui + gl + core + FFmpeg/OpenCV |")
    A("")

    for idx, (mod_title, prefix, incroot) in enumerate(MODULES):
        A(f"## {3 + idx}、{mod_title}（`{prefix}`）")
        A("")
        order = None
        if prefix == "ui/core":
            order = CORE_ORDER
        elif prefix == "ui/gl":
            order = GL_ORDER
        elif prefix == "ui/ui":
            order = UI_ORDER
        hdr_dir = os.path.join(ROOT, incroot)
        files = {}
        for root_dir, _, fnames in os.walk(hdr_dir):
            for fn in fnames:
                if fn.endswith('.h'):
                    rel = os.path.relpath(os.path.join(root_dir, fn), hdr_dir)
                    rel = rel.replace('\\', '/')
                    files[fn] = rel
        keys = [f for f in files if files[f].startswith(prefix)]
        if order:
            keys = [k for k in order if k in files]
        for fn in keys:
            rel = files[fn]
            info = scan_file(os.path.join(hdr_dir, rel))
            for line in render_file_section(rel, info):
                A(line)
            A("")
    return '\n'.join(lines)


def main():
    content = build()
    if '--check' in sys.argv:
        try:
            with open(OUT, 'r', encoding='utf-8') as f:
                cur = f.read()
        except OSError:
            print('UI_API.md missing')
            sys.exit(1)
        if cur != content:
            print('UI_API.md out of date: run `python tools/api_gen.py`')
            sys.exit(1)
        print('UI_API.md up to date')
        sys.exit(0)
    with open(OUT, 'w', encoding='utf-8') as f:
        f.write(content)
    print('generated %s (%d bytes)' % (OUT, len(content.encode('utf-8'))))


if __name__ == '__main__':
    main()
