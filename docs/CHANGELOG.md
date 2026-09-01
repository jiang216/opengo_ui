# CHANGELOG

本文件为 Bug / 功能条目事实源。每条 `## 前缀-编号 标题（YYYY-MM-DD）` 对应一个图谱节点，
修改涉及已有功能时核对「回归清单」，修 Bug / 加功能必须追加条目防错误复发。

## UI-155: 新增窗口容器 WindowFrame——自带自绘标题栏 + 内容区相对坐标 + 通用头部槽（2026-09-01）
- **类型**：功能新增（ui 库新增 `ui::WindowFrame` 可复用窗口容器控件）
- **严重度**：中
- **根因**：本库坐标模型为**绝对像素坐标**（`widget.h`），`TitleBar` 只是普通 Widget，库**没有「把内容自动置于标题栏下方并按容器相对坐标摆放」的整窗容器**——每个使用方都要自己为子控件叠加标题栏高度偏移，且多子控件用绝对坐标时易二次叠加。UI-153 已为 Scene 提供内容区原点（`content_origin_y`），但仍需使用方逐个子控件手动 `+head_h` 重算绝对坐标，缺少可复用的「容器即窗口」抽象。
- **修复**（ui 库新增 `ui::WindowFrame`，`ui/include/ui/ui/window_frame.h` + `ui/src/window_frame.cpp`）：
  - `WindowFrame(w,h)` 自带**顶部自绘标题栏**（默认系统风格 `ui::TitleBar`，标题栏高 `head_h_` 参数化默认 30）+ **内容区**在标题栏下方铺满剩余高度。
  - 内容子控件用**容器相对坐标**定位：`frame.content()` 拿内容容器，`frame->set_content_child(child,x,y,w,h)` 的 `(x,y)` 相对内容区顶部（标题栏下方）；控件内部以**相对矩形快照**为唯一事实源，`on_layout` 按快照重基准为绝对坐标（绝对 = 框架原点 + 内容区原点 + 相对），**不会随拖动/缩放二次叠加**（区别于使用方手写 `+offset` 的绝对写死）。
  - **通用头部槽**：`set_head(widget, height)` 可把**任意 Widget** 当头部（`set_head_child(child,x,y,w,h)` 以头部相对坐标摆放其子项，与 `set_content_child` 对称）；默认 TitleBar 由 `set_title/set_icon/set_on_close/on_minimize/on_maximize/set_*_visible` 透传（无子控件自绘，不受头部相对坐标影响）。
  - 默认 TitleBar 自绘（`titlebar.cpp` 以 `bounds_` 直接绘制三键/图标/标题，无子控件），故通用头部相对坐标仅对非默认 head 生效。
- **回归清单**（新增保证项，不得破坏既有）：
  1. `WindowFrame` 顶部标题栏渲染为一条干净深色带（默认 `#262A2F` 等宽满宽），内容子控件全部绘制于标题栏**下方**，绝不与标题栏重叠。
  2. 内容子控件以 `set_content_child` 容器相对坐标定位后，`on_layout` 重基准为绝对坐标，任意两次布局/拖动/缩放后相对位置不变（不二次叠加）。
  3. 通用头部 `set_head` + `set_head_child`：头部子项相对头部左上角摆放，内容区随之整体下移 head_h。
  4. 默认 TitleBar 透传（set_title/三键/关闭回调）及 UI-147/148/150/153 回归项全部保持。
- **验证**（Windows cmake MSVC Release）：`ui_ui` 重编译通过、`layui_test` 重链通过（新增演示页「窗口容器」）。`layui_test --smoke 2` exit 0。截屏（`--shot-page 8`）像素采样：演示页两个 WindowFrame——默认头部框标题栏带（winy≈231..259）为干净深色 `rgb(38,42,47)` 满宽、内容区绘制于其下 (winy≈260 起)；通用头部框头部带（winy≈529..569）渲染为独立条带、内容区随之下移. 注：演示页用 `Panel::set_fill` 的「自定义面板色」实为既有 `Panel` 的 `a=-1` 哨兵→走主题色（`panel.cpp`），非 WindowFrame 问题。
- **影响范围**：`ui/include/ui/ui/window_frame.h`、`ui/src/window_frame.cpp`、`ui/CMakeLists.txt`、`ui/include/ui/all.h`、`layui_test/src/pages/window_page.cpp`、`docs/ARCHITECTURE.md`、`docs/CHANGELOG.md`。

## UI-154: 子树变更不标脏使长命容器首帧不重排、新子控件在 (0,0) 闪一帧——基类 add_child/remove_child/clear_children 自报标脏（2026-09-01）
- **类型**：缺陷修复（ui 库 `Widget` 布局门控与树变更标脏缺口的缺陷）
- **严重度**：中（layui_test 每次切换子菜单内容面板其内容首帧在左上角 0,0 闪现一帧再归位）
- **根因**：`Widget::layout()`（`widget.cpp`）带**帧级门控**——`on_layout()` 仅在 `dirty_ || dirty_children_` 时执行，否则提前返回不重排。但 `Widget::add_child/remove_child/clear_children` 只调 `on_children_changed()` 而**从不 `mark_dirty()`**；基类 `on_children_changed()` 为空，`ScrollBox` 覆写它仅置 `needs_layout_`（只抑制滚动条绘制），**不参与 layout 门控**。于是长命容器（内容面板 `preview`、`code`、`help`、`sidebar` 这些 ScrollBox）在切换子菜单时被 `clear_children()` + 重新 `add_child` 重建，但容器自身不脏 → 当帧 `root_.layout()` 递归进它却跳过 `on_layout()` → 新子控件保持构造默认 `(0,0,0,0)` bounds → 首帧绘制在窗口左上角 (0,0)，直到下一次标脏（如 UI-136 CJK 烘焙 `mark_dirty_all`）才触发重排归位。用户所见即「从左上角 0,0 闪一下再画到对应坐标」。新建的子容器（构造即 `dirty_=true`）无此问题，惟长命容器重建时暴露。
- **修复**（ui 库 `Widget` 基类）：`add_child`/`remove_child`/`clear_children` 三处末尾各加 `mark_dirty()`，使任意子树变更即把容器及祖先置脏 → 下一 `root_.layout()` 必重排，新子控件**同帧**归位，消除 0,0 闪帧。`mark_dirty()` 幂等且沿父链短路，重复标脏无害。已排查全库：VBox/HBox/Grid/FlowBox/ScrollBox/ComboBox/Tabs/Table 子控件均由 `on_layout` 流式定位，标脏只让其「更该重排」正是目标；弹出面板（挂 root、位置由 `sync_panel`/`place_popup` 幂等重算）不在 add-child 路径，不受影响；`ScrollBox::add_pinned` 已自带 `mark_dirty` 冗余无害；`PanelWindow` 是唯一 on_layout 主动重定位所有子控件的容器，既有用法（子件构造 0,0 + on_layout 定位）无回归。
- **回归清单**（新增保证项，不得破坏既有）：
  1. 任意**长命容器**（内容面板/代码区/帮助/侧栏/可复用的 VBox 等）`clear_children()`+`add_child` 重建后，下一帧子控件即绘制在正确坐标，**不再首帧在 (0,0) 闪一帧**。
  2. 子控件在流式容器（VBox/HBox/Grid/FlowBox/ScrollBox/ComboBox/Table）内增删后布局随即更新，位置正确。
  3. 弹出面板（ComboBox/下拉/date/overflow 挂 root）相对位置不受影响（`sync_panel` 幂等重算）。
  4. `PanelWindow` 子控件随窗口拖拽/缩放仍正确重定位，相对偏移不变。
  5. UI-135/136/140 返回项保持：帧级门控渲染完备、CJK 烘焙续帧重排、ScrollBox/Code 覆写 draw 清脏。
- **验证**（Windows cmake MSVC Release）：`ui_ui` 重编译通过、`layui_test` 重链通过；`layui_test --smoke 2` exit 0。截屏/逐帧对比切换数据表格板块子菜单（anchor 页间切换，`place_content` 不改变 bounds 的场景），内容直接绘制在正确坐标、无 (0,0) 闪帧；`opengl_ui_demo` 弹出层页（PanelWindow）打开/拖拽/缩放子控件相对位置正确。
- **影响范围**：`ui/src/widget.cpp`、`docs/ARCHITECTURE.md`、`docs/CHANGELOG.md`。

## UI-153: 库级「顶部装帧内容区原点」——Scene::set_chrome/content_origin_y，其余控件统一绘制于标题栏下方 + 修复帧率 HUD 与标题条重叠（2026-09-01）
- **类型**：功能增强（ui 库 `ui::Scene`）+ 缺陷修复（测试程序 layui_test 帧率 HUD 直绘点位错误）
- **严重度**：中
- **根因**：库坐标模型为**绝对像素坐标**（`widget.h`），`TitleBar` 只是普通 `Widget`，库**没有任何机制**让其余内容自动绘制在标题栏下方——每个使用方都要自己为 header/sidebar/tabs/content/HUD 分别硬编码同一顶部偏移。layui_test 里 header/sidebar/nav/content/截图四处手动 `+kTitleBarH`，但**帧率 HUD 直绘**（`app.cpp` 的 `font.draw_text`）漏加偏移，`y=(kHeaderH-line_height)*0.5=20px`（绝对），落在标题条带(0..30)内，与自绘标题条重叠。
- **修复**（ui 库 `ui::Scene`，UI-153 新 API）：
  - `Scene::set_chrome(Widget* chrome, float top)`：注册顶部装帧并登记其占用高度，Scene 掌握内容区起始 y（`content_y_`，存成员）。
  - `Scene::content_origin_y()`：返回内容区起始 y（未注册为 0），供 PostDraw/HUD/截图/钩子统一对齐；`Scene::chrome()` 返回装帧指针。
  - 只登记高度/指针，**不移动 chrome**（应用仍将其作 root 普通子控件置于 y=0）；内容区坐标改由该原点为单一来源派生。
- **修复**（测试程序 layui_test）：注册 `scene.set_chrome(&titlebar, kTitleBarH)`，`header/sidebar/tabs/content` 的 y 均以 `scene.content_origin_y()`（本地 `kChromeTop`）为单一来源派生（`kTopH=kChromeTop+kHeaderH`、`content_y=…+kTabTitleH`），不再各自硬编码 `kTitleBarH`；帧率 HUD 的 `draw_text` y 改为 `kChromeTop+(kHeaderH-line_height)*0.5f`，在深色 Header 内垂直居中、不再压标题条。
- **回归清单**（新增保证项，不得破坏既有）：
  1. 标题条带（window y 0..kTitleBarH）除标题条自身标题/图标/三键外，无任何导航内容/HUD 与其重叠。
  2. 帧率 HUD 文本垂直居中于 Header（window y≈kChromeTop+(kHeaderH-line_height)/2），位于标题条下方，不与标题条重叠。
  3. Header 的 logo/导航文本/下拉/开关、sidear/nav/tabs/content 全部以库 `content_origin_y()` 为单一原点派生，位于标题条下方（承接 UI-149/151 回归项）。
  4. 窗口 Resize 后以上布局仍正确（Resize 钩子沿用同一 `kChromeTop` 派生）。
- **验证**（Windows cmake MSVC Release）：`ui_ui` 重编译通过、`layui_test` 重链通过；`layui_test --smoke 2` exit 0。截屏像素采样：标题条 winy=15 `rgb(38,42,47)` 干净、Header winy=40 `rgb(46,54,61)`、HUD 文本行 winy≈54..68（峰值 58..64）落于 Header 带内、winy=60 采样 `rgb(81,83,85)`＝HUD 文字色、标题条带 (0..30) 无 HUD/导航内容。
- **影响范围**：`ui/include/ui/ui/scene.h`、`ui/src/scene.cpp`、`layui_test/src/app.cpp`、`docs/CHANGELOG.md`。

## UI-151: 修复主窗口顶部导航题条重叠的深层根因——Header 子项未随父 y 偏移（本库子控件 bounds 为绝对坐标）（2026-09-01）
- **类型**：缺陷修复（测试程序 layui_test 自定义 `Header` 布局缺漏；非 ui 库缺陷）
- **严重度**：高（UI-149 仅修复 Header 外框定位，未解决其**内容**仍压在标题条的真实重叠）
- **根因**：本 ui 库布局模型为**子控件 bounds 即绝对坐标**（`Widget::draw` 递归调用子 `on_draw` 直接以 `bounds_` 绘制、无父级坐标叠加；
  库内容器如 `card.cpp`/`collapse.cpp`/`blockquote.cpp`/`checkbox.cpp` 在 `on_layout` 中都手动叠加 `bounds_.x/y`）。而 layui_test 的
  `Header`（`app.cpp`）`on_layout` 把所有子项（`logo_img_`/navs/dropdowns/`theme_switch_`）按**局部 (0,0) 起**摆放，未叠加父 `bounds_.y`。
  于是把 Header 外框 `set_bounds(0,kTitleBarH,..)` 下移后：外框背景（`on_draw` 用 `bounds_`）确实下移到 y=30，但各子项仍绘制在
  绝对 y=0.. → **logo 图片、导航文本、下拉、主题开关全部压在自绘 TitleBar 上**，与标题栏重叠。UI-149 只动了外框位置、未动内容，
  故用户看到「背景分离了但 logo/文本/按钮没下移」。
- **修复**（测试程序 layui_test `app.cpp`）：`Header::on_layout` 引入 `const float oy = bounds_.y;`，对所有子项坐标叠加 `oy`：
  `logo_img_`(0,oy)、`theme_switch_`/dropdowns 的垂直居中 `oy+(h-child_h)*0.5f`、navs(0,oy,..)。使导航栏**内容随 Header 外框整体下移**
  到标题条下方 y=kTitleBarH 起。遵循库内容器叠加父偏移的一致约定。
- **回归清单**（新增保证项，不得破坏既有）：
  1. 标题条带（y=0..kTitleBarH）内除标题条自身图标/三键外不再有任何导航内容（logo/文本/开关）与之重叠。
  2. 导航栏内容（logo 图、组件/笔记文本、生态/2.13.9 下拉、主题开关）垂直位于 Header 外框 y=kTitleBarH 起，与标题条不相交。
  3. UI-147/148/149/150 回归项全部保持：标题条三键/图标/拖动/关闭、Header 底色可区分、Resize 后布局不变。
- **验证**（Windows cmake MSVC Release）：`layui_test` 重编译重链通过；`layui_test --smoke 2` exit 0。截屏扫描主窗口顶部：
  x=400 纵扫 y=0..30 为纯标题条深色 `rgb(38,42,47)`（无内容），y≈32 起出现导航内容亮像素、y=36..88 为 Header 带；
  全宽内容扫描 y=3..21 为 0 内容（标题条清空），y=24..87 为导航内容带 → 标题条与导航内容不再重叠。
- **影响范围**：`layui_test/src/app.cpp`、`docs/CHANGELOG.md`。

## UI-150: 自绘 TitleBar 默认背景与下方 Header 同色导致"无缝融合、标题条不可区分"——改为独立更深灰（2026-09-01）
- **类型**：缺陷修复（ui 库 `ui::TitleBar` 默认配色缺陷）
- **严重度**：低
- **根因**：UI-147/148 的 `ui::TitleBar::on_draw` 深色底用 `Vec4{0.18,0.21,0.24}`（= `#2F363C`），与主窗口顶部自定义
  `Header`（layui_test `app.cpp`）的下方导航栏底色**完全相同**。二者上下紧贴（TitleBar y=0..30、Header y=30..90）且同色，
  中间仅 1px 分隔线，视觉上标题条"无缝融入"导航栏变成一整块，用户无法把标题条识别为导航栏上方独立的一条。
- **修复**（ui 库 `ui/src/titlebar.cpp`）：TitleBar 顶部深色底改为**更深一档** `Vec4{0.149,0.165,0.184}`（≈ `#262A2F`），
  与紧邻其下的 Header `#2F363C` 形成明确明度差，标题条作为独立深色条清晰可见。分隔线与三键/图标/标题绘制不变。
  只改背景常量，不引入新 API，不影响其它使用 TitleBar 的场景。
- **回归清单**（新增保证项，不得破坏既有）：
  1. TitleBar 背景色（`#262A2F`）与紧邻下方页头/导航栏底色（如 `#2F363C`）**明度可区分**，标题条作为独立条带可见。
  2. UI-147/148/149 回归项全部保持：三键 hover/按下红底、关闭 `on_close_`、拖动整窗、`set_icon` 左上图标、标题裁剪防压键。
- **验证**（Windows cmake MSVC Release）：`ui_ui` 重建通过，`layui_test` 重链通过。截屏扫描主窗口顶部条带：
  标题条 y=0..30 采样 `rgb(38,42,47)`、下方 Header y=30..90 采样 `rgb(46,54,61)`，两带明度差明显、分界在 y=30。
- **影响范围**：`ui/src/titlebar.cpp`、`docs/CHANGELOG.md`。

## UI-149: 修复主窗口自绘标题条被顶部 Header 遮住——Header 构造默认 y=0 与 TitleBar 重叠，首帧标题条不可见（2026-09-01）
- **类型**：缺陷修复（测试程序 layui_test 布局定位错误，非 ui 库缺陷）
- **严重度**：中
- **根因**：UI-148 主窗口布局中，自定义 `Header`（`layui_test/src/app.cpp` 的 `Header(float w,float h)`）构造默认为
  `Widget(0,0,w,h)`（y=0），与自绘 `TitleBar`（y=0,h=30）**重叠**。Header 在 root 中晚于 TitleBar 加入、绘制在其上，
  其深色底 + logo 首帧即盖住标题条（三键 + 标题不可见）。Header 仅在被加到 `kTitleBarH=30` 处，位置修正只发生在
  **Resize 事件**（`SceneEvent::Resized` 钩子里的 `header.set_bounds(0,kTitleBarH,...)`）→ 首帧/未 resize 时标题条被遮，
  直到窗口发生尺寸变化（用户所见「隐藏后再显示才显示」）才自我纠正。属测试程序把构建期坐标与布局期坐标割裂的定位缺漏。
- **修复**（测试程序 layui_test，非 ui 库）：`Header` 构造加入 root 前显式 `header.set_bounds(0, kTitleBarH, window.width(), kHeaderH)`
  与 Resize 钩子同值，保证**首帧即让出标题条**，无需等 Resize 事件纠正。标题条（y=0..30）与 Header（y=30..90）首帧即正确上下排布。
- **回归清单**（新增保证项，不得破坏既有）：
  1. 主窗口首帧（未发生任何 Resize）自绘标题条即完整可见：三键在右上、标题/应用图标在左上，不被 Header 遮盖。
  2. Header（logo / 导航文本 / 下拉 / 主题开关）位于标题条下方 y=kTitleBarH 起，二者不重叠、不错位。
  3. 窗口 Resize 后标题条/Header 位置仍正确（Resize 钩子逻辑不变）。
- **验证**（Windows cmake MSVC Release）：`layui_test` 编译链接通过；`layui_test --smoke 2` exit 0（"page switches ok"）。
  截屏采样顶部条带：y=15 行检测到标题文字亮像素（x≈17..149）与右上窗口键（x≈1195 最小化 / x≈1264 关闭），
  Header 深色带位于 y=30..90，标题条与 Header 首帧正确分离。
- **影响范围**：`layui_test/src/app.cpp`、`docs/CHANGELOG.md`。

## UI-148: TitleBar 补齐 最小化/最大化/关闭 三键 + 左上角应用图标；Menu 分组头箭头改用 iconfont + 可联动收起子菜单列（2026-09-01）
- **类型**：功能补强（ui 库，UI-147 基础上的标题条交互补全 + 菜单分组头图标与联动缺口修复）
- **严重度**：中
- **根因**：
  - `ui::TitleBar`（UI-147）仅有标题 + 右上关闭钮，缺系统标题栏常规的「最小化/最大化/关闭」三键与左上角应用图标；
    而 gl 库 `Window` 已具备 `iconify()/maximize()/restore()/is_maximized()`，属标题条未接线。
  - `ui::Menu` 分组头折叠箭头用 `renderer.draw_polygon` 画**裸三角形**（`ui/src/menu.cpp` 原 L239-247），未走 `Font::draw_icon`
    使用 iconfont 码点（图标库已有 `layui_icon::down/right`）→ 子菜单标题栏箭头为粗糙几何三角、风格不统一。
  - 点击分组头仅 `it.spread = !it.spread`（折叠组内子项），**不联动外部子菜单列显隐**——子菜单列显隐受 `app.cpp` 的
    `child_sidebar.visible = child_visible` 控制，分组头点击无法收起/展开整列，属菜单与宿主间缺联动回调。
- **修复**（ui 库，禁止测试程序打补丁）：
  - `ui::TitleBar`（`ui/src/titlebar.cpp` + `titlebar.h`）右侧补齐三键：最小化 → `window_->iconify()`；最大化/还原
    → 依 `window_->is_maximized()` 切 `maximize()/restore()`（图标随态切 `□`/`⧉`）；关闭 → `on_close_`。三键带 hover/按下红底，
    命中区与拖拽互斥（`pressed_`/`dragging_`）。新增 `set_minimize_visible/set_maximize_visible`；新增
    `set_icon(Font*, uint32_t codepoint)` 在左上画应用图标（`Font::draw_icon`，默认不画）。测试程序主标题条接
    `set_icon(&font, ui::layui_icon::app)`。
  - `ui::Menu`（`ui/src/menu.cpp`）分组头折叠箭头改 `font_->draw_icon(renderer, layui_icon::down/right, ...)`，弃 `draw_polygon`
    裸三角（`draw_icon` 缺字形/缺图集时安全提前返回，兼容未烘焙 icon 字体）。
  - `ui::Menu` 新增 `set_on_group_click(std::function<void(const char*)>)`：分组头点击、`spread` 切换后触发回调，供宿主联动
    收起/展开外部面板。测试程序 `populate_child` 接入：点击子菜单分组头（如"数据表格"）在 `child_visible` 与
    `place_content()` 间切换，实现整列子菜单收起/展开。
- **回归清单**（新增保证项，且不得破坏既有）：
  1. UI-147 回归项全部保持：空白区拖动整窗、关闭钮 `on_close_`、`window_` 为空仅绘制不崩溃、`center_on_primary_monitor` 居中、
     resize 时标题条全宽置顶、Scene 子窗口标题条关闭子窗，均不变。
  2. `TitleBar` 三键命中区与拖拽互斥：鼠标落在任一窗口键上按下→触发对应操作，不误触发拖动；空白区按下→拖动。
  3. 最小化/最大化/还原：`iconify()` 最小化、点击最大化钮在最大/还原间切换且图标随 `is_maximized()` 更新。
  4. `set_icon` 仅当提供了 icon 字体与码点才绘制左上图标；未设置不绘制、不崩溃。
  5. Menu 分组头箭头改 iconfont 后仍正常指示折叠态（展开 `down`/折叠 `right`）；`draw_icon` 对缺字形字体安全返回不崩。
  6. `set_on_group_click` 仅分组头点击触发（`add_group` 项）；点击可同步收起/展开 `child_sidebar` 整列，`place_content` 正确重排内容区。
- **验证**（Windows cmake MSVC Release）：`layui_test` 编译链接通过；`layui_test --smoke 2` exit 0（"page switches ok"）；
  `--shot`/`--shot-page` 截图正常落盘（1280x900 PAM）。
- **影响范围**：`ui/include/ui/ui/titlebar.h`、`ui/src/titlebar.cpp`、`ui/include/ui/ui/menu.h`、`ui/src/menu.cpp`、
  `layui_test/src/app.cpp`、`docs/CHANGELOG.md`、`docs/UI_API.md`。

## UI-147: 新增自绘系统风格标题条 TitleBar + 窗口居中——无边框窗口可拖动 / 关闭（2026-09-01）
- **类型**：功能新增（ui 库新增 `ui::TitleBar` 可复用控件 + gl 库补 `Window::center_on_primary_monitor`）
- **严重度**：中
- **根因**：`gl/src/window.cpp` 的 `init_glfw()` 全局 `glfwWindowHint(GLFW_DECORATED, GLFW_FALSE)` 令所有 `ui::Window` 无边框，
  而 ui/gl 库虽已具备 `Window::start_drag/update_drag/end_drag`，却**无任何自绘标题栏 / 窗口居中机制**，导致测试程序窗口
  绘制在 0,0、无法拖动。按 AGENTS「ui 库能力缺陷归因到 ui 库」强制修库侧，禁止测试程序打补丁。
- **修复**（ui 库 + gl 库）：
  - 新增 `ui::TitleBar`（`ui/include/ui/ui/titlebar.h` + `ui/src/titlebar.cpp`）：系统标题栏风格一条（深色底 + 底部 1px 分隔线 +
    左侧标题文字 + 右上关闭钮 hover/按下红底）。空白区 mouse_down → `window_->start_drag`、move（拖动中，经 `capture_mouse` 保证
    OS 窗口拖动期间持续收 move 事件）→ `update_drag`、up → `end_drag`；关闭钮命中 → `on_close_` 回调（宿主注入退出/关闭窗口）。
    `window_` 可为空，此时仅绘制不拖动。字体未设时绘制全部跳过；主题未设时回退内置深色主题。
  - gl 库补 `Window::center_on_primary_monitor()`：在主显示器工作区内居中（`get_monitor_workarea` + `get_size`，窗口大于工作区时贴左上）。
  - 测试程序接入：主窗口创建后居中 + 顶部加标题条（关闭钮→`request_exit`）；Scene 子窗口（scene_page）加标题条（关闭钮→子窗 `request_exit`），
    子窗内容下移标题条高。主窗口侧栏/导航/标签页整体下移 `kTitleBarH` 以让出标题条。
- **回归清单**（新增保证项，且不得破坏既有）：
  1. `TitleBar` 空白区拖动整窗、关闭钮点击触发的 `on_close_` 行为不变；`window_` 为空时仅绘制不崩溃。
  2. `Window::center_on_primary_monitor` 使窗口在主显示器工作区内完整可见（窗口大于工作区时贴左上，不越界）。
  3. 主窗口标题条在窗口 resize（Resized 钩子）时保持全宽并置于 y=0；侧栏/导航/标签随 `kTopH` 整体下移不错位。
  4. Scene 子窗口标题条关闭钮关闭子窗（`request_exit` → 下一帧 `close_child`），客户区内容下移标题条高不重叠。
  5. `GLFW_DECORATED=GLFW_FALSE` 前提下，标题条为主窗口与外接子窗口提供唯一拖动入口。
- **验证**（Windows cmake MSVC Release）：`layui_test` 编译链接通过（仅 MSB8029/MSB8012 临时目录警告）；`layui_test --smoke 2` exit 0（"page switches ok"）。
- **影响范围**：`ui/include/ui/ui/titlebar.h`（新）、`ui/src/titlebar.cpp`（新）、`ui/include/ui/all.h`、`ui/CMakeLists.txt`、
  `gl/include/ui/gl/window.h`、`gl/src/window.cpp`（新增 `Window::center_on_primary_monitor`）、`layui_test/src/app.cpp`、
  `layui_test/src/pages/scene_page.cpp`、`docs/CHANGELOG.md`、`docs/UI_API.md`。

## UI-146: 新增统一生命周期抽象 ui::Scene——生产者→分配者→消费者→清理重置（2026-09-01）
- **类型**：新功能 + 架构演进（ui 库新增 `ui::Scene`，收敛各应用主循环模板）
- **严重度**：中（新增能力，5 个渲染循环应用全部迁移为 Scene 驱动，行为不回归）
- **归因**：ui 库（`ui/include/ui/ui/scene.h` + `ui/src/scene.cpp`）+ gl 库（`Window::make_current` 补缺口）+ 测试程序迁移（apps、layui_test）。
- **修复**（ui 库）：
  - 新增 `ui::Scene(window, renderer)`：随窗口同步创建，构造广播 `ScenePhase::Ready`；`frame(dt)` 单帧统一完成
    **分配**（layout + mouse/scroll/键盘导航/文本 dispatch + `root.advance(dt)`）+ **消费**（`begin_frame(clear)` + `root.draw`），
    `run()/run_for(seconds)` 含 poll + swap 的完整循环、`run_for` 承载 `--smoke` 通用语义。
  - 生命周期阶段 `ScenePhase`（Ready/Reset/Frame/PostDraw/Cleanup/Shutdown）向量广播，`subscribe(phase, fn)`
    一阶段多观察者、`on(phase, fn)` 便捷重载；二者均返回**订阅 token**（非 0），`unsubscribe(token)` 可按 token
    单独摘除单个阶段观察者（不清其他 App 级订阅），页面级局部订阅切页时即用此摘除防悬空；
    逐事件钩子 `SceneEvent`（MouseMove/Down/Up/Scroll/Key/Char/Drop/Resized）单槽位、返回 true=消费。
  - 分配开关 `dispatch_mouse/scroll/nav_keys/text/resize`（默认全开）保留各应用迁移前按键语义；Escape 不在标准模板
    （属应用语义，如 layui_test 关弹窗经 Key 钩子自处理）。
  - **清理重置**（`~Scene`/`shutdown`/`reset`）：只清理库内自管对象——`flush_pending_destroys` + `close_popups`。
    **不调用 `clear_children()`**：Widget 容器默认不拥有子树，其子节点可能是**栈上对象**（`root.add_child(&栈Label)`），
    `clear_children` 会对栈对象 `delete` 造成堆损坏（0xC0000374）。此为迁移验证发现的硬约束，已写入回归清单。
  - 多窗口：每窗口 = 独立 Scene + Window/Renderer/Font/root；GLFW 每窗口默认独立 GL 上下文，绘制前须
    `window.make_current()`（新增 `Window::make_current()` 补齐切换上下文的 ui_gl 缺口）。禁止同上下文双 Renderer
    （Renderer 实例级缓存 active_program_/scissor_stack_/proj_[16] 等 GL 全局状态）。
- **测试程序迁移**（行为保真矩阵）：
  - `apps/main.cpp`（opengl_ui_demo）：全套 nav+Enter/Space/Backspace/Delete/Home/End+chars 默认保留；媒体/轮播/上传
    `advance` + 主题清屏色同步入 Frame 订阅，Tooltip/HUD 入 PostDraw，Exit 按钮改 `scene.request_exit()`。
  - `apps/api_demo.cpp`：原无 Space/Delete/Home/End → `dispatch_nav_keys=false` + Key 事件钩子仅分发原 Tab/方向/Enter/
    Backspace（严格不回归）。
  - `apps/image_viewer.cpp`：原无标准键/字符分发 → 关 nav/text/scroll，滚轮缩放/拖拽/Ctrl+O 走 Frame 订阅与 Key 钩子，
    图像直绘入 PostDraw。
  - `apps/video_player.cpp`：原无 nav 键 → 关 nav/text，Space=播放（Key 钩子）与 Ctrl+O，resize 重排入 Resized 钩子。
  - `layui_test/src/app.cpp`：全套键默认 + Escape 经 Key 钩子关弹窗；冒烟自动切页/`--click` 注入入 Frame，截图入 PostDraw
    （glReadPixels 后 `scene.request_exit()`）。
  - **新增** `apps/scene_multiwin.cpp`：双窗口各自独立 GL 上下文，逐窗口 make_current + `scene.frame(dt)` + swap 的主循环，
    验证多 Scene 并行。
   - `apps/edit_test.cpp` / `apps/media_load_test.cpp` 无渲染循环，保持不动。
   - **新增** `layui_test/src/pages/scene_page.cpp`（展示 > 生命周期 demo）：静态说明（四阶段 / 发布-订阅 /
     分配开关 / 多窗口 make_current）+ 实时交互（PostDraw 直绘 HUD、Frame 每帧计数、request_exit、
     子窗口驱动），全部订阅经页面 `destroy` 钩子统一 `unsubscribe` 摘除（页面级 token，不影响 App 级订阅）。
- **回归清单**（新增保证项，且不得破坏既有）：
  1. Scene 清理重置不得对栈上子控件 delete（仅 flush + close_popups），否则堆损坏崩溃（UI-146 根因）。
  2. 各应用迁移后按键语义与迁移前严格一致（api_demo/image_viewer/video_player 关闭非原分发即保证）。
  3. 多窗口每窗口独立 GL 上下文，绘制前 make_current；禁止同上下文双 Renderer（实例级 GL 全局状态冲突）。
  4. 5 个渲染应用 + scene_multiwin 均以 Scene 驱动且菜单/按钮/弹窗/导航/截图验证不回归。
  5. 页面级局部订阅必须 `unsubscribe(token)` 单独摘除（不经 `clear_observers` 清掉 App 级订阅），且摘除须在
     页面控件 `clear_children` 之前（本页经 `destroy` 钩子统一摘除），防止切页悬空闭包引用旧树。
- **验证**（Windows cmake MSVC Release + Debug）：
  - 根工程 `--preset x64`（media OFF）全目标编译通过；`--preset x64-all`（media ON）编译通过。
  - `opengl_ui_demo`/`api_demo`/`image_viewer`/`video_player`/`layui_test`/`scene_multiwin` 全部 `--smoke 2` **exit 0**
    （迁移前 Release 因 Scene 对栈控件 clear_children 而 exit 0xC0000374，修复后全绿）。
  - `layui_test --smoke 2` 输出 "page switches ok"（Scene Frame 订阅自动切页正常）。
  - `layui_test --shot-page 0 --shot x.pam` 输出 "shot saved (1280x900)" 且生成合法 PAM 头（PostDraw 截图 + request_exit 正常）。
  - `scene_multiwin --smoke 2` 双上下文渲染 "done" 且 exit 0。
- **影响范围**：`ui/include/ui/ui/scene.h`（新）、`ui/src/scene.cpp`（新）、`ui/CMakeLists.txt`、`ui/include/ui/all.h`、
  `gl/include/ui/gl/window.h`、`gl/src/window.cpp`（新增 `Window::make_current`）、`apps/CMakeLists.txt`、
  `apps/main.cpp`、`apps/api_demo.cpp`、`apps/image_viewer.cpp`、`apps/video_player.cpp`、`apps/scene_multiwin.cpp`（新）、
  `layui_test/src/app.cpp`、`layui_test/src/page_registry.h`、`layui_test/src/pages/scene_page.cpp`（新）、
  `docs/CHANGELOG.md`、`docs/ARCHITECTURE.md`、`docs/UI_API.md`。

## UI-145: 全方位优化批次——渲染性能/内存生命周期/代码去重/构建与清理（2026-08-31）
- **类型**：优化批次（性能 / 内存 / 可维护性 / 构建；不改对外 API 语义）
- **严重度**：低中（纯优化，各功能行为经编译验证等价）
- **阶段1 渲染性能**（`gl/`，归因 ui 库）：
  - `Renderer::set_viewport` 计算并缓存正交投影到 `proj_[16]`；删去 `draw_rounded_rect_impl`/`draw_textured_quad`/`draw_polygon` 每次调用重复算投影并上传。
  - 颜色/纹理管线按需切换 program 的混合/深度状态集中到 `bind_color_blend`；避免高频 draw 反复设置状态。
  - 逐顶点 `cos/sin(uRotation)` 移除，改为 CPU 预计算并以 `uCosRot`/`uSinRot` uniform 传入（每顶点省两次三角函数）。
- **阶段2 内存/生命周期**（`ui/`+`gl/`）：
  - `Widget` 5 个事件 dispatch 的子节点遍历：小缓冲（≤32）栈数组快照免堆分配，超限回退堆 vector（语义同 UI-073 快照）。
  - `CursorIconManager` 泄漏：`get_cursor_manager` 改函数局部静态，删除从未释放的 `new`。
  - 标准光标（arrow/hand/resize/ibeam）新增 `StandardCursorCache` 一次创建复用 + `ensure/destroy`，替代每次 set 都 create+destroy。
  - `Image::texture_` 由裸指针 `Texture*` 改 `std::unique_ptr<Texture>`，随控件析构自动释放（消除手动 delete 泄漏风险）。
- **阶段3 代码去重**（`gl/`+`core/`+`ui/`）：
  - `Font::draw_text` 两个重载合并到私有 `draw_text_impl`（可选 `end` 字节界），消除双份绘制循环防逻辑漂移。
  - UTF-8 解码统一到新 `core/include/ui/core/utf8.h`（`next_index/prev_index/decode_cp`），textbox/textarea/table 原各拷一份的 `next_cp/prev_cp/decode_cp` 收敛为转发实现。
  - `Renderer` push/pop scissor 的 y-flip 收敛到 `apply_scissor_gl`；5 个 `set_cursor_*` 收敛到 `apply_standard_cursor` helper。
- **阶段4 构建**（CMake）：
  - Release 启用 LTO（`INTERPROCEDURAL_OPTIMIZATION_RELEASE`）+ `-O3`（GCC/Clang）/ `/O2`（MSVC）。
  - 新增 `OPENGUI_USE_PCH`（默认 ON）为 core/gl/ui 库加 `target_precompile_headers`。
  - `apps/CMakeLists.txt` 将多段 `POST_BUILD` 依赖 DLL / 素材拷贝合并为按目标单条（genex 条件），减少重复命令。
  - **Windows 实机补充修复（CMake 4.2.2 / MSVC 19.51，2026-08-31）**：
    - 根 `CMakeLists.txt` 新增 `add_compile_definitions(GLFW_INCLUDE_NONE)`：GLFW 默认在未定义该宏时（仅 Windows 分支）include 系统 `<GL/gl.h>`，
      与 GLAD `<glad/gl.h>` 同 TU 冲突（`#error: OpenGL (gl.h) header already included`）；`GLFW_INCLUDE_NONE` 统一定义后由各 TU 显式 include `glad/gl.h` 提供 GL API。Linux 验证因无系统 `GL/gl.h` 且用 GL 头 stub 未暴露此问题。
    - `apps/CMakeLists.txt` POST_BUILD 的嵌套生成器表达式 `$<$<BOOL:...>:COMMAND ... $<TARGET_FILE_DIR:...>>` 在 CMake 4.x 下解析残留字面 `$<0:`，致 `copy_if_different ... Release $<0:` 报「系统找不到指定的路径 / 命令语法不正确」exit 255；
      改为 `if(...)` 分支显式拼接命令列表（`_DLL_CMDS`/`_ASSET_CMDS`），语义不变、与 CMake 版本无关。
  - **保留** `vcpkg.json`/`conanfile.py`/`portfile.cmake`（文档化打包分发通道，非死文件，继续支持 CMake export+vcpkg+Conan 三选一）。
- **阶段5 清理**：删除再生成/冗余产物（约 2GB）：`*.obj`、`shot_*.png`、`tmp/`、`tools/__pycache__/`、`3rdparty/src/layui-v2.13.8/`（zip 保留可重解压）、顶层 CMake 构建树 `build/ cmake-build-*`、过时顶层 `graphgen-dist/`（权威版本在 `tools/graphgen-dist/`）。
- **回归清单**：
  1. 投影矩阵缓存后，圆角/纹理/多边形绘制坐标、窗口缩放纵横比不回归（`set_viewport` 变化仍触发重传）。
  2. 文字/图标绘制、仿粗体偏移、像素对齐（UI-055）不回归；`max_len` 重载按字节界截断语义不变。
  3. 事件分发对子节点增删的回调安全性不回归（快照语义同 UI-073）；光标样式切换/回收/泄漏修复。
  4. 图片加载（含懒加载 lay-src UI-093）、重复设置 lay_src 释放旧纹理不回归。
  5. UTF-8 编辑（多字节光标移动/删除、CJK 宽度）-textbox/textarea/table 不回归（函数体与统一实现逐字节等价）。
  6. 嵌套裁剪（UI-057）/标准光标 set 行为不变。
  7. Release 构建 + PCH 正常编译产出，各 demo 可运行；依赖 DLL 与素材照常拷贝到可执行目录。
- **验证**：Linux 下 core/gl/ui 全部 `.cpp` 经 g++ `-fsyntax-only` **0 错误**编译通过（临时 GL 头 stub，未提交）。**Windows 主机实机验证（2026-08-31，CMake 4.2.2 + MSVC 19.51.36246，含上述补充修复）**：
  - `--preset x64` 与 `--preset install` 双预设 **Release 全量构建 + install 成功**（PCH/LTO 生效）：ui_core/ui_gl/ui_ui 三大静态库 + opengl_ui_demo/opengl_ui_api_demo/opengl_ui_edit_test 三 exe。
  - POST_BUILD 依赖拷贝正常：`glfw3.dll` / `iconfont.ttf` 已拷至 exe 目录；install 产物（头文件/库/config/GLFW DLL/资产）完整。
  - `opengl_ui_edit_test` 冒烟 **10/10 PASS**（文本输入、Ctrl 组合、剪贴板、CJK 输入）、exit 0。
  - `opengl_ui_demo` 实机启动：1280x1280 窗口渲染正常，菜单/按钮/滑块/表格/左侧导航/CJK 中文全部显示（无 UI-135「字体擦除/缺渲染」），交互（点击/滚轮）后仍稳定运行无崩溃。
- **影响范围**：`gl/`(renderer/window/font/cursor_icon_manager)、`ui/`(widget/image/textbox/textarea/table)、`core/`(新增 utf8.h)、`CMakeLists.txt`（根/gl/ui/apps）、docs/CHANGELOG.md。

## UI-143: Table 新增下拉单元格列——表格行内下拉选择（2026-08-30）
- **类型**：功能新增（audio_text 插件配置面板表格式：翻译路径/模型/URL/api_key/目标语言/启用/测试 单行表，「目标语言」列须行内下拉）
- **严重度**：中
- **根因**：Table 此前仅有文本行内编辑（UI-134）/ 开关复选（UI-131）/ 按钮列 / 树形等能力，**缺行内下拉选择列**。宿主在表格上浮一层伪下拉会破坏滚动/命中/编辑协同，按「ui 库能力缺陷归因到 ui 库」强制归因在库内新增列类型。
- **修复**（ui 库级）：
  - `ColKind` 新增 `ComboBox`；新增 `set_cell_combobox(col)` / `set_combobox_options(col, options)` / `set_on_cell_combobox(visible_row, col, index, text)`。
  - 该列每格绘制格文本 + 右侧下拉三角（#c2c2c2，与 ComboBox 皮肤一致）；表头标题居中（与开关/复选列 UI-131 一致）。
  - 单击单元格展开行内 ComboBox（`begin_combo`）：复用 ComboBox 皮肤与 `open_popup` 弹出机制，定位沿用 `begin_edit` 坐标约定（表头下 + 列偏移 − 内容滚动偏移）。
  - 选中即写回该格文本（`rows_` 直写，无需 rebuild 即生效）并触发回调。
  - 生命周期：`combo_box_` **对象池式复用**（on_change 回调栈内禁止 delete 触发者自身——`ComboBox::select` 返回后仍执行 `close()/focus()`，delete 即 use-after-free）；`close_combo` 仅隐藏；`rebuild_rows` / 点击格子以外区域统一关闭。
  - `ComboBox::open()` 由 private 提为 public（Table 需显式展开面板）。
- **回归清单**：
  1. 文本/开关/复选/按钮/树形/分页既有列类型行为不变（仅新增枚举分支）。
  2. 下拉列单击展开面板；选项点击选中写回单元格并触发 `on_cell_combobox`（参数为可见行号/列号/选项下标/文本）。
  3. 选中后 ComboBox 隐藏，不残留点击拦截（`visible=false` 不参与 dispatch 命中/绘制，widget.cpp:273）。
  4. 展开中点击表格其他区域自动关闭；滚动/重建（reload/sort/set_page/add_row）自动关闭。
  5. 未设置 `set_combobox_options` 的下拉列展开为空选项面板，不崩溃。
  6. 行内编辑（双击文本格）与下拉列互不干扰：点击下拉列前先 commit 编辑框。
- **验证**：opengl_UI Release 构建 + install 同步（ui_ui.lib / table.h / combobox.h）→ audio_text 插件配置面板表格式实施后实机验证。
- **影响范围**：`ui/include/ui/ui/table.h`、`ui/src/table.cpp`、`ui/include/ui/ui/combobox.h` + install 同步。

## UI-142: ScrollBox 固定头部 + 背景直角化——页面顶部留白固定且与左栏拼全宽横条（2026-08-27）
- **类型**：功能改进 / 视觉一致性（audio_text 多页面 UI：设置页顶部留白随滚动上移 + 右面板圆角与左侧导航直角交界出现月牙空隙）
- **严重度**：中
- **根因**：
  - 设置页 `ScrollBox` 顶部内边距 `padding_` 参与 `content_height`/`scroll_max`/`on_layout` 光标计算，卡片随滚动整体上移，首个卡片上方的空白留白随之「滚动到顶外」而被感知为「顶部留白移动」。
  - 面板默认画 `draw_rounded_rect(bounds_, radius=6, button_normal)`，左上角 (200,36) 圆角弧内露出更暗的 `window_bg`，与左侧 `SidebarNav` 直角 `{0.18,0.21,0.24}` 交界产生月牙空隙。
- **修复**（ui 库级）：
  - `ScrollBox` 新增 `set_header_height(float)`：置顶一条**固定高度、不随滚动移动**的头部带；内容区/滚动范围/裁剪区均以 `header_height_` 为顶边内边距，header 高度之上永远只显示滚动容器背景（固定留白）。
  - `ScrollBox` 默认背景由圆角矩形改为**直角** `draw_quad(bounds_, button_normal)`，消除面板圆角与左侧导航直角交界的月牙空隙；保留 `layui-round` class 可显式切回圆角（下拉面板等既有圆角分支不受影响）。
- **回归清单**：
  1. ScrollBox 内容超高时滚动，顶部固定头带（header_height）**始终停驻在容器顶部**，内容从其下方滚过。
  2. 未调用 `set_header_height` 的 ScrollBox（0）行为与旧版一致（顶部留白=padding，随内容滚动）。
  3. 背景直角化后，`layui-round` class 的 ScrollBox 仍画圆角（下拉面板等不回归）。
  4. 滚动条 thumb 范围 / 命中裁剪与固定头带一致（内容不滚穿头带区）。
- **验证**：opengl_UI Release 构建 + install 同步（ui_ui.lib）→ audio_text 重链 voice_assistant.exe；设置/关于/语音/规则/声纹页实机验证固定横条贯穿全宽、滚动时固定、无圆角空隙。
- **影响范围**：`ui/include/ui/ui/scrollbox.h`、`ui/src/scrollbox.cpp` + install lib 同步（ui_ui.lib）。

## UI-144: ScrollBox 滚动条按下滑块不再立即跳转，点击轨道空白仍跳转（2026-08-28）
- **类型**：Bug 修复 / 交互精准性（audio_text 设置页滚动条：按住滑块后「上下滑动的距离太大」）
- **严重度**：中
- **根因**：`ScrollBox::on_mouse_down` 对滚动条内**任意点击（含命中滑块）**都执行 `set_scroll(scroll_from_y(y))`，把滑块顶瞬间对齐到鼠标 y——按住滑块中部时按下即下跳约半个滑块高，配合 `kThumbMinH=28` 最小高 clamp，感知为「鼠标定位后滑块上下滑动距离太大」。
- **修复**（ui 库级，最简方案）：`on_mouse_down` 按命中区域区分——
  - **命中滑块**：仅 `sb_.dragging=true`，**不再**按下瞬间 `set_scroll`（消除按下即跳）；
  - **命中轨道空白**（滑块上/下方）：**保持** `set_scroll(scroll_from_y(y))` 跳转到点击位置（原有行为不变）。
  - 拖拽跟随仍用 `scroll_from_y(y)`（不做 grab 偏移），保持最简。
- **回归清单**：
  1. 按住滑块按下**不再**立即跳动；拖动时滑块跟随光标滚动。
  2. 点击轨道空白处（滑块上/下方）**仍**跳转到点击对应位置。
  3. 滚轮滚动 / 触底 / 固定头带（UI-142 header_height）不回归。
- **验证**：opengl_UI Release 构建 + install 同步（ui_ui.lib）→ audio_text 重链 voice_assistant.exe；设置页实机验证按滑块不跳、点轨道空白跳转、滚轮/触底正常。
- **影响范围**：`ui/src/scrollbox.cpp` + install lib 同步（ui_ui.lib）。

## UI-141: Transfer 双侧列表滚轮改 scroll 漏 mark_dirty——同类漏标脏补全（2026-08-27）
- **类型**：Bug 修复 / 交互正确性（UI-140 同类漏标脏的系统性复查补漏）
- **严重度**：中（`Transfer::on_mouse_scroll` 双侧列表滚轮直接改 `side->scroll` 并 `clamp_scroll`，却不 `mark_dirty()` → 帧级门控检不到 → 滚轮滚动不刷新，需额外脏事件才补画。与 UI-140 已修的 `Code::on_mouse_scroll` 同款漏标点）
- **根因**：`transfer.cpp` `on_mouse_scroll`（滚动条目）只改 `side->scroll` 状态、不标脏；其公开方法 `move`/`add_item`/`clear`/`set_show_search` 等均有 `mark_dirty()`，唯独滚动路径漏标。
- **修复**（ui 库级，宿主零改动）：`Transfer::on_mouse_scroll` 末尾补 `mark_dirty()`。
- **回归清单**：
  1. Transfer 双侧列表内容超高时滚轮滚动**即时刷新**，无需额外交互。
  2. Transfer 搜索 / 移动 / 勾选行为不回归（原有 mark_dirty 保留）。

## UI-140: ScrollBox/Code 覆写 draw 不清自身脏标记→dirty_children_ 常脏，短路 mark_dirty 上抛、容器内控件点击/悬停不刷新（2026-08-27）
- **类型**：Bug 修复 / 渲染门控正确性（audio_text：设置页 ScrollBox 内全部控件——开关 / 下拉 / 滑条 / 输入框点击后需额外滚动或移动鼠标才刷新；左侧导航半透明 hover 高亮不跟随。UI-137/138/139 依次补齐 mark_dirty 后**复测仍无改善**，证明根因非漏标脏，而是「标了脏也到不了 root」）
- **严重度**：高（容器常脏使子控件 `mark_dirty()` 沿父链上抛时在此 `break` 短路，root 永不脏 → 帧级门控 `if(!root.is_dirty()) continue` 跳过整帧渲染 → 容器内任意交互视觉停驻，直到有路径让 root 脏才补画，被感知为「点击无反应、滚动/移动鼠标/点别处才显示」）
- **根因**：`mark_dirty()`（widget.cpp:37 `if (p->dirty_children_) break;`）是沿父链的短路优化——祖先只要 `dirty_children_` 已为 true 就停止上抛。而 **`ScrollBox::draw()`（scrollbox.cpp:221-239）与 `Code::draw()`（code.cpp:69-77）覆写了基类 `Widget::draw` 却不像基类那样在末尾清掉自己的 `dirty_/dirty_children_`**（基类 widget.cpp:576-577 会清）。于是 ScrollBox/Code 一旦被任一子控件标过脏，其 `dirty_children_` 即**永久为 true** → 其内所有控件再标脏都被短路吞掉、永远到不了 root → 不重绘。仅容器自身标脏（ScrollBox `set_scroll` 触发滚动）或容器外事件（移鼠标到左导航等非 ScrollBox 控件、点别处）能让 root 脏 → 补画。精确解释「点一下无反应，滚动/移动鼠标/点别处才显示」；无 ScrollBox 的 speaker/voice 页正常；UI-137/138 给容器内控件补的 mark_dirty 每次都被短路吞掉故无效。
- **修复**（ui 库级，宿主零改动）：`ScrollBox::draw()` 与 `Code::draw()` 在绘制完子树后补 `dirty_ = false; dirty_children_ = false;`（与基类 `Widget::draw` 末尾一致），解除常脏短路，恢复子控件标脏一路传回 root → 帧级门控即时重绘。附带：`Code::on_mouse_scroll` 改 `scroll_` 时补 `mark_dirty()`（此前改滚动不经标脏，与 ScrollBox::set_scroll 语义相反，同属漏标脏）。
- **回归清单**：
  1. 设置页 ScrollBox 内开关 / 下拉 / 滑条 / 输入框：点击或悬停后**即时刷新**，无需滚动 / 移动鼠标 / 点别处。
  2. speaker / voice 页（无 ScrollBox）行为不受影响、不回归。
  3. ScrollBox 内下拉面板等 overlay 节点正常绘制、滚动条 overlay 行为不变。
  4. Code（notepad）滚动即时刷新。

## UI-139: 帧级门控下 Menu 左侧导航 hover 高亮不跟随——on_mouse_move 更新 hover_ 却漏标脏（2026-08-27）
- **类型**：Bug 修复 / 交互正确性（audio_text：左侧垂直导航菜单——半透明 hover 高亮框只有首个命中项定位准确，继续在菜单项间移动不跟随过去，需额外交互一次才刷新到新位置）
- **严重度**：中（UI-135「所有状态变更必须显式 mark_dirty」规则下，`Menu::on_mouse_move` 直接更新 `hover_` / `active_hover_`（决定哪一行画高亮）却全程不标脏，帧级门控检不到 → 高亮视觉停在首个命中行，直到下一次脏帧才补画，被感知为「跟随框移不动 / 只对了一次」）
- **根因**：`Menu::on_mouse_move`（menu.cpp）重排 `hover_` / `active_hover_` 指向并据此绘制高亮（`on_draw` 里 `if(hover_==&it)` 画 quad），但方法本身不调 `mark_dirty()`；基类 `Widget::dispatch_mouse_move` 只在**控件 hover 边界翻转**（`was_hovered` 变化）时标脏（widget.cpp `if(!was_hovered) mark_dirty()`），对**控件内部 hover 目标在菜单项间移动**这一场景完全不标脏 → 高亮静止。
- **修复**（ui 库级，宿主零改动）：`menu.cpp` `on_mouse_move` 开头记录 `hover_` / `active_hover_` 旧值，末尾及每条返回路径前比对；若 hover 目标变化则 `mark_dirty()`，驱动帧级门控在「跟随框移到新菜单项」的帧即时重绘。
- **回归清单**：
  1. 左侧导航：鼠标在多项之间来回移动，半透明高亮框**即时跟随**到当前项，不需额外点击 / 切页。
  2. 子面板（Parent 项 sub-panel）内 hover / 移出子面板：高亮与子面板跟随 / 关闭即时反映。
  3. 移出整个菜单：高亮清除即时显示。
  4. UI-135 语义不变：菜单上无鼠标移动 / hover 目标未变时依旧停帧省 CPU（标脏仅在 hover 目标实际变化时发生）。
- **验证**：opengl_UI Release 构建 + install 同步（ui_ui.lib）→ audio_text 重链 voice_assistant.exe 成功；opengl_UI + audio_text 双库 graphgen --check 0 错 0 警。
- **影响范围**：`ui/src/menu.cpp` + install lib 同步（ui_ui.lib）。

## UI-138: 帧级门控下 TextBox/TextArea 编辑与光标/选区/滚动路径漏标脏（2026-08-27）
- **类型**：Bug 修复 / 交互正确性（audio_text：设置页监听端口等输入框、关于页只读 TextArea——输入/光标移动/拖选/滚轮滚动需额外交互一次才刷新）
- **严重度**：中（UI-135「所有状态变更必须显式 mark_dirty」规则下，文本框/多行框大量事件路径直接改 `text_`/`caret_`/`sel_`/`scroll_` 却漏标脏，帧级门控检不到 → 输入、光标移动、选区、滚动视觉停旧态，直到下一次脏帧才补画，被感知为「输入延迟/失效」「需滚动一下才显示」）
- **根因**：UI-135 后脏检测只认 `mark_dirty()` 自报。TextBox / TextArea 的编辑与光标状态写大多在「公共方法与事件回调」内直接改状态且不自报：
  1. TextBox：`move_caret` / `erase_cp` / `erase_selection` / `paste_clipboard` / `on_key_down`（左右/Home/End 光标、Ctrl+A 全选、Backspace/Delete）/ `on_char` / `on_mouse_down` / `on_mouse_move`（拖选）均漏标脏。
  2. TextArea：`erase_selection` / `replace_selection` / `erase_cp` / `on_key_down`（方向键光标/选区、Ctrl+A/V/X、Enter）/ `on_char` / `on_mouse_down` / `on_mouse_move`（拖选 + 边缘自动滚屏 + 滚动条拖拽）/ `on_mouse_up` / `on_mouse_scroll` / `scroll_to_end` 均漏标脏——尤其滚动路径（`on_mouse_scroll` / `sb_` 拖拽 / `scroll_to_end`）漏标，直接对应「滚动需额外一拍才显示」。
- **修复**（ui 库级，宿主零改动）：`textbox.cpp` / `textarea.cpp` 在上述每个编辑 / 光标 / 选区 / 滚动事件与公共方法内补 `mark_dirty()`；滚轮与拖拽滚动仅在真正改变 `scroll_` 时标脏（不随 hover 全刷）。
- **回归清单**：
  1. 输入框逐字符输入即时显示、光标随方向键即时移动、拖选实时高亮。
  2. Ctrl+A 全选 / Ctrl+C/V/X 即时反映视觉。
  3. TextArea 方向键光标移动即时、滚轮滚动即时、拖动滚动条即时、新增内容 `scroll_to_end` 即时。
  4. UI-135 语义不变：静止场景（无任何编辑/光标/滚动写）依旧停帧省 CPU；标脏仅在真正改状态时发生。
- **验证**：opengl_UI Release 构建 + install 同步（ui_ui.lib）→ audio_text 重链 voice_assistant.exe 成功；audio_text + opengl_UI 双库 graphgen --check 0 错 0 警。
- **影响范围**：`ui/src/textbox.cpp`、`ui/src/textarea.cpp` + install lib 同步（ui_ui.lib）。

## UI-137: 帧级门控下控件状态写统一自报标脏——开关/下拉/滑块事件内直改漏标点（2026-08-27）
- **类型**：Bug 修复 / 交互正确性（audio_text 设置页：个别开关需多次点击才切换、下拉菜单需多次点击才展开、键盘改值无重绘）
- **严重度**：中（UI-135「所有状态变更必须显式 mark_dirty」规则下，事件回调内直接改成员状态又漏标脏的路径在帧级门控中表现为「点了没反应/要点两次」，并能与 UI-125 惰性烘焙配额叠加成可见间歇）
- **根因**：
  - UI-135 后脏检测只认 `mark_dirty()` 自报；任何事件回调内**直接改成员状态而未标脏**的路径，root `is_dirty()` 检不到 → 帧级门控跳过重绘 → 视觉停在旧态，直到下一次交互产生的脏帧才补画，被用户感知为「无响应 / 需多点一次」。
  - 具体漏标点：
    1. `Switch::on_mouse_down` 直接翻 `checked_`——仅靠 dispatch 的按下态副作用兜底；`Switch::on_key_down`（空格切换）**完全不标脏**。
    2. `ComboBox` 开合 toggle（`on_mouse_down`）与键盘方向键改选项路径不标脏（`select()`/`set_selected` 等 setter 已带）。
    3. `Slider` 鼠标/键盘改值统一走 `emit()`，但 `emit()` 本身不标脏（`set_value` 等 setter 已带，键盘方向键路径漏）。
- **修复**（ui 库级，宿主零改动）：
  - `switch.cpp`：`on_mouse_down`/`on_key_down` 统一改走 `set_checked(!checked_)`（内部 mark_dirty），再触发 `on_change_`。
  - `combobox.cpp`：`open()`、`on_mouse_down`（开合 toggle）、`on_key_down`（方向键改选项）显式 `mark_dirty()`。
  - `slider.cpp`：`emit()` 开头加 `mark_dirty()`，所有值变更路径（含键盘）统一自报。
- **回归清单**：
  1. 设置页全部开关**单击即切换**、状态标签即时刷新（不依赖第二次点击/切页）。
  2. 三个下拉（麦克风 / VAD 停顿档位 / 普通转录最长录音）**单击即开合**、键盘方向键切换选项即时高亮。
  3. 滑块拖动 / 键盘改值即时重绘（如录音指示器透明度）。
  4. UI-135 语义不变：未发生状态写时依旧停帧省 CPU（新增标脏仅发生在真正改状态时，静止场景 CPU 维持 ≤2%）。
  5. UI-136 惰性烘焙不变：开关/下拉/滑块交互产生的脏帧照常驱动烘焙，无冲突。
- **验证**：ui 库 Release 构建 + install 同步（ui_ui.lib）→ audio_text 重链 voice_assistant.exe 成功；regression.ps1 259 项 0 失败；opengl_UI graphgen --check 0 错 0 警。运行期精确复现已由宿主侧按用户指示终止（间歇性确切换机未实锤，本次按「金芯：状态写自报」方案一次性补齐全部漏标点根治）。
- **影响范围**：`ui/src/switch.cpp`、`ui/src/combobox.cpp`、`ui/src/slider.cpp` + install lib 同步（ui_ui.lib）。

## UI-136: 帧级门控下 CJK 惰性烘焙自动续帧——待烘焙队列 + bake-epoch 全树重排（2026-08-27）
- **类型**：Bug 修复 / 渲染正确性（audio_text：启动 / 切页后设置页与初始化日志文本残缺为空格占位、排列错位，需交互或切页一次才显示正确）
- **严重度**：高（UI-125 逐帧烘焙配额 × 帧级门控静默停摆的交互回归：首帧后文本永久停留在「方块字宽占位」，布局宽度近似错位）
- **根因**：
  - CJK 字形懒烘焙（UI-125）按 16ms 窗口定额限量（默认 12 字形/帧），只有「渲染帧」才会烘焙下一批；配额耗尽的码点本帧回退方块字宽近似（绘制为空——视觉上即「空格/缺失」）。
  - 帧级门控（IDLE-00x + UI-135）下根 `is_dirty()` 为 false 即跳过渲染：首个渲染帧只烘 ≤12 个新码点，根 clean 后停帧，其余数百个码点永远停在占位；仅当交互 / 切页产生脏帧才能再烘 ≤12 个——呈现为「交互一会儿 / 切页一次才正确」。
  - 且仅根置脏不足以让 Label 以真实宽度重排：子节点自身 `dirty_` 仍为 false，`layout()` 跳过其 `on_layout()`，故首次布局用近似宽度算出的行 / 列宽不会自愈 → 排列错位。
  - 只影响接可控门控的宿主（audio_text）；layui_test 每帧渲染、渐进烘焙自然完成，无此问题。
- **修复**（ui 库级，宿主零改动）：
  - `gl/include/ui/gl/font.h` + `gl/src/font.cpp`：
    1. 新增 `mutable pending_cps_`（因配额未烘但已被 draw/measure 请求的码点；成功烘焙摘除，缺失 / 无位图 / 图集满等结构性失败也摘除——防永久 pending 无限渲染）；
    2. 新增 `mutable bake_epoch_`（每次新字形成功上屏 +1，xadvance 可能变化）；
    3. 全局字体注册表 + 静态查询 `Font::any_pending_bake()` / `Font::bake_epoch_sum()`（构造注册 / 析构注销）；顺带补 `emoji_info_` 原析构遗漏的释放。
  - `ui/include/ui/ui/widget.h` + `ui/src/widget.cpp`：
    4. 新增递归 `Widget::mark_dirty_all()`（自身 + 子树逐节点置脏）；
    5. `Widget::advance()` 根分支（每帧无条件执行、位于宿主帧级门控之前）：
       `if (Font::any_pending_bake() || bake_epoch_sum() != last_epoch) mark_dirty_all();`
       ——待烘字形续帧渲染下一批；epoch 变化做最终一次全树重排；两者皆静恢复空闲停帧。
- **回归清单**：
  1. 帧级门控宿主（audio_text）首显 / 切页：设置页、初始化日志等 CJK 文本启动后约 0.5s 内自动全量补齐、无需交互；行 / 列宽最终按真实字形 advance 重排正确，无「空格占位」残留、无近似宽度错位。
  2. 补齐完成 / 无脏 → 门控停帧恢复，空闲 CPU 维持 ≈2%（语音页实测 2.2%，设置页 ≈4%），不允许续帧忙循环。
  3. UI-125 渐进语义不变：单帧仍 ≤12 字形 / 16ms 配额，不引入单帧批量光栅化 spike；`prebake_text()` 强制路径行为不变。
  4. 防无限渲染：缺失码点（missing_）、无位图、图集满等结构失败摘除 pending，图集 / 码点不会导致永久渲染循环。
  5. 非门控宿主（layui_test 每帧渲染）零影响：epoch 标脏对每帧渲染天然无害。
- **验证**：ui 库 Release 构建 + install 同步（ui_ui.lib / ui_gl.lib / font.h / widget.h 双 install 时间戳核对）；audio_text 重建后实测——全新启动首帧初始化日志完整、设置页首显 300ms 内补齐且空闲 3s 不回落、滚动 / 切页正常、5s 空闲 CPU 2.2%；regression.ps1 259 项 0 失败；opengl_UI graphgen --check 0 错 0 警。
- **影响范围**：`gl/include/ui/gl/font.h`、`gl/src/font.cpp`、`ui/include/ui/ui/widget.h`、`ui/src/widget.cpp` + install lib / 头文件同步（ui_ui.lib、ui_gl.lib）。

## UI-135: 帧级门控渲染完备性——draw 移除脏守卫全树绘制 + hover/pressed/focus/scroll 显式标脏（2026-08-27）
- **类型**：Bug 修复 / 渲染正确性（audio_text：首帧后控件字体被擦除只剩日志区；demo/layui_test 整窗空白；系统设置页无法滚动）
- **严重度**：高（IDLE-00x 空闲帧渲染优化引入的回归：画面持续残缺 + 滚动完全失效）
- **根因**：
  - `Widget::draw()` 带 `if (!dirty_ && !dirty_children_) return;` 子树跳过守卫，与 renderer `begin_frame()` 每帧全屏 `glClear` 不匹配：帧缓冲每帧被清空，第二帧起只有被标脏的控件重绘，干净控件像素被清掉只剩背景。三处症状同根：应用首帧完整、后续仅持续标脏的日志区可见；demo / layui_test 首帧后清空为纯背景；
  - 帧级门控（root.is_dirty() 决定整帧渲染或跳过）下，一切“状态变更 → 需重绘”的路径都必须显式 `mark_dirty()`。漏标点：
    1. `dispatch_mouse_move`（捕获命中 / hover 进入移出）、`dispatch_mouse_down`（按下）、`dispatch_mouse_up`（抬起 / 拖出释放）、`set_focused()`——基类直接改写 `hovered_`/`pressed_`/`focused_`，改门控后高亮 / 按压 / 焦点视觉不刷新；
    2. `ScrollBox::set_scroll()` 滚动偏移变更未标脏 → 滚轮 / 拖动滚动条改变 `scroll_offset_` 但根保持 clean，渲染被跳过 → 滚动完全失效（长内容页“卡住”假象）；
    3. overlay 滚动条 2s 自动隐藏按时间迁移状态，无标脏则滚动条残留上帧像素不消失。
- **修复**：
  - `Widget::draw()` 删除子树跳过守卫（保留 `if (!visible) return;` 与帧末清除脏标记）——渲染帧内必须全树绘制；脏标记只作帧级门控，加注释固化语义；
  - `widget.cpp` `dispatch_mouse_move/down/up` 与 `set_focused()` 补 `mark_dirty()`（捕获命中 / hover 进移 / 按下 / 抬起 / 拖出释放 / 焦点态）；
  - `ScrollBox::set_scroll()` 偏移变化时 `mark_dirty()`（滚轮 / 拖动 / `set_scroll_offset` 统一收敛）；新增 `ScrollBox::on_advance()` 检测滚动条活动态迁移并标脏，2s 自动隐藏正确消失。
- **回归清单**：
  1. 应用启动 / 切页全量渲染完整：静态控件不因每帧清屏被擦除；日志区持续重绘正常；demo、layui_test、audio_text 画面完整。
  2. 空闲期帧级门控仍生效：根保持 clean 时跳过渲染，CPU/GPU 低占用（audio_text 实测空闲 ≈2% 单核）。
  3. hover / 按下 / 抬起 / 拖出释放 / 焦点变更均触发整帧重绘，按钮高亮、按压态、输入框焦点边框 / 光标即时刷新。
  4. ScrollBox 滚轮 / 拖动滚动条 / 程序化 `set_scroll_offset` 均平滑重绘；滚动条悬停浮现、拖动加宽、2s 无活动自动隐藏。
  5. UI-134（Label scissor / 滚动条命中区加宽）、UI-131（表格开关皮肤）、ScrollBox 触底加载 / 锚定 / 下拉面板等既有语义不变。
- **验证**：ui 库 Release 构建 + install（强制时间戳校验同步双 install/lib）；layui_test 全页渲染（像素 99% 非背景 + OCR 读全正文）；audio_text 重链后：整窗渲染完整、设置页滚动实测有像素变化（16341px）、切页稳定 340ms、空闲 CPU 2.2%；下游 regression.ps1 全 PASS（259 项 0 失败）；双仓库 graphgen --check 0 错 0 警。
- **影响范围**：`ui/src/widget.cpp`、`ui/src/scrollbox.cpp`、`ui/include/ui/ui/scrollbox.h`（+ install/lib、头文件同步）。

## UI-134: ScrollBar 可命中区加宽 + Label scissor 裁剪防溢出（2026-08-27）
- **类型**：Bug 修复 / 可用性
- **严重度**：低
- **根因**：
  1. ScrollBar `kMargin=4px` / `kHitPad=2px`，滚动条轨道距控件右缘仅 4px，可命中区域总宽仅 14-16px，窗口缩放时难以选中
  2. `Label::on_draw()` 无 scissor 裁剪，换行文本溢出 bounds 时绘制到相邻控件上方（与滚动条重叠）
- **修复**：
  1. `scrollbar_style.h`：`kMargin` 4→6px，`kHitPad` 2→4px（命中区总宽 20px）
  2. `label.cpp`：`on_draw` 加 `push_scissor` / `pop_scissor` 裁剪到 bounds
- **回归清单**：
  1. ScrollBox / TextArea 滚动条外观不变（仅间距微调 2px）
  2. 滚动条可命中区域增大，窗口缩放时更容易选中
  3. Label 文本不再溢出 bounds，与相邻控件无重叠
  4. 下游 audio_text 回归全 PASS（259 项）
- **验证**：Release 构建 + install 通过；下游 audio_text regression.ps1 全 PASS（259 项 0 失败）
- **影响范围**：`ui/include/ui/ui/scrollbar_style.h`、`ui/src/label.cpp`

## UI-133: 模态拦截可见性防御——modal 控件隐藏后不再吞输入，修复僵尸模态全 UI 死锁（2026-08-23）
- **类型**：Bug 修复 / 健壮性（audio_text CONF-002：确认弹窗点掉后返回主界面，全部按钮/功能永久失灵）
- **严重度**：高（宿主未接 advance() 时，一次 destroy 即可永久锁死整个界面输入）
- **根因**：UI-072 延迟销毁下 `destroy()` 仅置 `visible=false` 入队；而 7 个 dispatch_* 入口的模态分支 `if (!parent_ && modal_)` **不检查 modal_->visible**——模态控件隐藏后仍持续把全部鼠标/键盘事件路由给不可见子树并整体吞掉。宿主主循环若未调用根节点 `advance()`（其帧末才执行 `flush_pending_destroys()`），队列永不冲刷、`modal_` 永久指向僵尸控件 → 输入全锁死。audio_text 主循环正是未接 advance 的集成形态。
- **修复**：`ui/src/widget.cpp` 全部 7 处模态分支统一改为 `if (!parent_ && modal_ && modal_->visible)`（mouse move/down/up/scroll、key down/up、char）——**模态语义收敛为「可见才模态」**：destroy 待释放、手动隐藏等任何不可见状态自动解除拦截；set_modal 处补注释固化该契约。
- **回归清单**：
  1. 可见模态弹窗照常屏蔽外部输入（PanelWindow confirm/layer 用法行为不变）。
  2. modal 控件 destroy()/visible=false 后当帧起输入恢复直达下层控件，无僵尸拦截。
  3. flush_pending_destroys 照常清理 modal_/capture_/popups_ 引用（UI-072 语义不变）。
  4. layui_test layer 页与 audio_text CONF-001 弹窗正常弹出/关闭/重复弹窗防护不变。
- **验证**：Release 构建+install 通过；下游 audio_text 补接 flush 后重链 regression.ps1 全 PASS（201 项 0 失败）；双仓库 graphgen --check 0 错 0 警。
- **影响范围**：`ui/src/widget.cpp`（7 处模态分支 + set_modal 注释）。

## UI-132: Table preferred_height 高度语义修正——默认内容跟随，新增 set_fixed_height/set_max_content_height（2026-08-23）
- **类型**：Bug 修复 / API 语义（audio_text SPK-005 用户确认 app 端绕过修复有效后，要求按 AGENTS「先归因再修复」规则回归 ui 库根治遗留）
- **严重度**：高（流式容器内表格高度被首次分配值永久冻结，add_row 后行区全部裁剪不可见）
- **根因**：`Table::preferred_height()` 首行 `if (bounds_.h > 0.f) return bounds_.h;`——流式容器（Card/ScrollBox/VBox…）按子件 preferred 分配高度；空表期（0 行）分到仅表头高后 `bounds_.h > 0` 恒成立，此后 add_row 增长永不反映：行区 body_h = bounds_.h − 表头 ≈ 0，所有数据行被裁剪。`Widget::layout()` 无条件递归不可见子树（TAB-001「bounds 恒最新」设计）使隐藏期布局也持续以冻结值刷新，缺陷必现。
- **修复**：
  - `preferred_height()` 改为默认**内容跟随**：每次调用按 `表头 + 可见行数×行高 [+ 分页条]` 重算，删除 bounds_.h 冻结分支；
  - 新增 `set_fixed_height(h)`（h>0 显式锁定 preferred_height，固定视口+内部滚动场景；<=0 解除回内容跟随）；
  - 新增 `set_max_content_height(h)`（内容跟随模式的上限钳制，流式容器防溢出；超出部分由表格内部滚动区承载）；
  - 迁移说明：旧「构造 h>0 + 放流式容器」依赖冻结语义的用法须显式改调 set_fixed_height——排查 apps/layui_test 全部 Table 用法：流式页均 h=0 构造（新旧语义一致）、api_demo 一处 h=120 进 VBox（测试程序展示效果随内容伸缩更正确）；手动 set_bounds 页面父级不读 preferred 零影响。
- **回归清单**：
  1. 流式容器内 Table 高度随 add_row/clear_rows 即时伸缩：空表=仅表头、N 行=表头+N×行高[+分页条]，不再冻结。
  2. set_fixed_height(h>0)：preferred_height 恒返回 h，内部纵向滚动行为与旧 h>0 构造一致。
  3. set_max_content_height(h>0)：内容高超过 h 时钳制为 h，超出行经表格内部滚动区可达。
  4. 手动 set_bounds 布局页面（父级不读 preferred）：摆位与绘制零变化。
  5. 分页模式（visible_rows_ 为当前页切片）：preferred 随每页行数正确变化；auto_page_size（UI-123）行为不变。
  6. 树形折叠 / 流加载（visible_rows_ 过滤或增长）：preferred 跟随过滤后可见行数，UI-126/UI-127 语义不变。
- **验证**：ui 库 Release 构建+install 通过；apps+layui_test 编译通过；下游 audio_text 改用新 API 后重链 regression.ps1 全 PASS；双仓库 graphgen --check 0 错 0 警。
- **影响范围**：`ui/include/ui/ui/table.h`、`ui/src/table.cpp`（preferred_height 语义修正 + 两个新 setter）。

## UI-131: draw_cell_form 参数错位修复——单元格开关恒走复选分支绘矩形；表单列表头居中（2026-08-23）
- **类型**：Bug 修复（audio_text SPK-005 用户复核：声纹状态列开关「没看到更改，还是绘制的矩形，并且没有对齐状态标签列」）
- **严重度**：高（UI-130 皮肤改造对用户完全不可见——目标绘制分支从未执行）
- **根因**：`TableRow::on_draw` 调用 `draw_cell_form(renderer, x, cy, w, c)`，而定义为 `(Renderer& r, float x, float cy, float col, int cell_idx)`——按位置绑定后形参 `col` 收到**列宽值**、真正的列号 `c` 进入 `cell_idx` 又被 `(void)` 丢弃；函数内 `is_cell_switch(90)`（90 为列宽数值）恒 false → 永远走 checkbox 分支绘空心矩形。点击交互走 `Table::on_mouse_up` 独立命中测试（真实列号）故功能正常、仅绘制错——SPK-003 期「只有一个矩形」与 UI-130 后「还是矩形」同根因。
- **修复**：
  - `draw_cell_form` 签名改为 `(Renderer& r, float x, float cy, float w, int col)` 与调用点语义一致（w=列宽、col=列号），删除无用 cell_idx，局部盒宽改名 bw 防形参遮蔽；
  - 表单列（CellSwitch / CellCheckbox）表头标题改为**水平居中**绘制，与单元格控件（居中）对齐。
- **回归清单**：
  1. 表格行内开关（set_cell_switch 列）实际渲染为圆角开关皮肤：开=绿底白滑块靠右、关=白底灰边灰滑块靠左，不再出现空心矩形。
  2. 开关列/复选列表头标题水平居中，与单元格控件水平中线对齐；普通文本列、序号列表头仍左对齐不变。
  3. 单元格复选框（set_cell_checkbox）：选中=accent 圆角方块、未选=灰描边、居中，不受影响。
  4. 点击翻转、on_cell_form 回调、排序箭头、复选/单选列、合计行、分页路径零影响。
- **验证**：Release 构建+install 通过；下游 audio_text 重链后 regression.ps1 全 PASS；双仓库 graphgen --check 0 错 0 警。
- **影响范围**：`ui/src/table.cpp`（draw_cell_form 签名修正 + 表单列表头居中）。

## UI-130: 开关皮肤统一——draw_switch_skin 共享函数，表格行内开关与 Switch 同源渲染（2026-08-23）
- **类型**：Bug 修复 / 控件统一（audio_text SPK-004 用户报告：声纹表状态列行内开关「只有一个矩形，无法明确看到开关状态」，要求项目开关统一为圆角开关）
- **根因**：`Table::draw_cell_form` 对 cell switch **自绘简化矩形块**（teal 色块+小方点、无滑块行程语义），与 `ui::Switch`（layui-form-switch 克隆：绿底白滑块靠右/白底灰边灰滑块靠左）两套皮肤并存，表格内开关状态难以辨识——属 ui 库缺陷（AGENTS 规则：能力缺陷回归 ui 库修复）。
- **修复**：
  - 新增共享皮肤函数 `ui::draw_switch_skin(renderer, rect, theme, checked, enabled, hovered, pressed)`（switch.h 声明/switch.cpp 实现）：三态绘制逻辑自 `Switch::on_draw` 原样抽取，取色走传入 Theme（空指针回退默认主题）；
  - `Switch::on_draw` 改为调用该函数（文本/自定义内容/聚焦环仍在本体绘制）；
  - `Table::draw_cell_form` switch 分支改调该函数——行内开关与页面开关**同源同款渲染**；复选框分支不变。
- **回归清单**：
  1. `ui::Switch` 本体渲染零变化（同一皮肤同参数路径；hover/pressed/focus 语义不变）。
  2. 表格行内开关（set_cell_switch）：开=绿底白滑块靠右、关=白底灰边灰滑块靠左、尺寸 42×24 居中，状态一目了然。
  3. 单元格复选框（set_cell_checkbox）、复选/单选列、操作列按钮绘制不受影响。
  4. 下游 audio_text regression.ps1 全 PASS。
- **验证**：Release 构建+install 通过；下游 audio_text SPK-004 接入后 regression.ps1 全 PASS（193 项 0 失败）；双仓库 graphgen --check 0 错 0 警。
- **影响范围**：`ui/include/ui/ui/switch.h`、`ui/src/switch.cpp`、`ui/src/table.cpp`。

## UI-129: Table 单元格表单切换回调——set_on_cell_form 点击翻转通知调用方（2026-08-23）
- **类型**：功能补全（audio_text SPK-003 声纹管理页「启用/禁用」由操作列按钮改为状态列行内开关，需感知 cell switch 切换事件；此前 `set_cell_switch`/`set_cell_checkbox` 点击仅翻转单元格文本、无任何回调，调用方无法响应）
- **实现**：
  - `Table::set_on_cell_form(cb)`：单元格表单切换回调（参数：可见行号, 列号, 切换后的选中态）；
  - `on_mouse_down` 单元格表单分支翻转 `rows_[data][col]` 文本后触发回调（"1"/"0" 语义不变）。
- **回归清单**：
  1. 未注册回调时行为与之前完全一致（仅翻转文本），既有调用方零影响。
  2. 复选/单选列（on_selection_）、操作列按钮（on_action_）、单元格事件列（on_cell_event_）、树形/分页/流加载路径均不受影响。
  3. 回调内触发行控件重建安全（dispatch 子节点快照迭代 + 表格自身不销毁，与操作列删除按钮同一既有模式）。
- **验证**：Release 构建+install 通过；下游 audio_text SPK-003 接入后 regression.ps1 全 PASS（193 项 0 失败）。
- **影响范围**：`ui/include/ui/ui/table.h`、`ui/src/table.cpp`。

## UI-128: emoji 渲染回退链——缺字 tofu 防护 + Segoe UI Emoji 单色轮廓烘焙（2026-08-22）
- **类型**：Bug 修复（audio_text 用户报告：ASR 模型输出 emoji（😔 U+1F614 / 🎼 U+1F3BC / 😊 U+1F60A），界面只绘制出矩形方块；正则规则层无法影响显示——rule_engine 规则作用于识别文本→命令匹配链路，与字形渲染互不相干）
- **根因**：
  1. 字形源三路均不覆盖增补平面：ASCII 静态图集、iconfont.ttf 仅 layui 私用区（E600..ECFF/1000..100F）、CJK 懒烘培用中文字体（msyh/simhei/simsun/Deng 均无 emoji 字形）；
  2. `bake_cjk()` 不检查 `stbtt_FindGlyphIndex()==0`——字体查无此字时 stbtt 回退 .notdef 字形，而中文字体 .notdef 即矩形框轮廓，被当正常字形烘入图集缓存 `dyn_` → 每帧稳定画出 tofu 方块；
  3. 缺失码点不入 `dyn_` → 每次绘制都重试光栅化，白白消耗 UI-125 配额。
- **修复**：
  - 新增 `load_emoji_font(path)` / `load_emoji_font_auto()`：`OPENGUI_EMOJI_FONT` 环境变量 → Windows `seguiemj.ttf`；stbtt 烘其 glyf 单色轮廓（COLR 彩层忽略，形状可辨）；macOS/Linux 位图彩色 emoji 不默认探测（静默失败不刷日志）；
  - `bake_cjk()` 改显式回退链：中文字体 `FindGlyphIndex>0` → emoji 字体 `FindGlyphIndex>0` → 两源皆无记入 `missing_` 确认缺失集（negative cache），杜绝 .notdef tofu 入图集且后续绘制直接走方块字宽近似不再重试；
  - 变体选择符 U+FE0F / 零宽连接符 U+200D 零宽处理（draw_text 跳过、char_width 返回 0）——emoji 带 FE0F 后缀不再多占一个方块宽。
- **回归清单**：
  1. emoji 文本首帧即为正确单色轮廓字形（非矩形方块）；带 FE0F 后缀不产生额外空位。
  2. 中日韩文本渲染零变化（cjk 字体命中路径不变）；ASCII/图标零变化。
  3. 无 emoji 字体环境（探测失败）：缺字码点走方块宽近似且每码点仅尝试光栅化一次（missing_ 短路）。
  4. 图集打包/配额机制不变；atlas full 日志行为不变。
  5. 下游 audio_text regression.ps1 全 PASS。
- **验证**：opengl_UI Release 构建+install 通过；Windows seguiemj.ttf 存在性已确认；audio_text 接入 load_emoji_font_auto 后 regression.ps1 全 PASS（174 项）；双仓库 graphgen --check 0 错 0 警。
- **影响范围**：`gl/include/ui/gl/font.h`、`gl/src/font.cpp`。

## UI-127: 流加载/表格首显闪烁——触底重检分帧 + prebake_visible_text 字形预热（2026-08-22）
- **类型**：Bug 修复（audio_text 用户报告：[FLOW-001] 流加载续灌瞬间与规则页四 tab 手动切换瞬间，预览区闪烁；排查确认非深灰背景/白表清屏覆盖问题，为两个机制叠加）
- **根因**：
  1. **CJK 懒烘焙配额渐进补齐**（UI-125 机制残留）：非活动表格从未 draw → 其文本字形从未烘焙；流加载续灌批次含大量首见码点。首帧超配额字形走方块字宽回退、后续数帧逐字补齐 = 文字浮现/宽度跳动闪烁。UI-125 已提供 `prebake_text` 但下游零调用；
  2. **触底同步递归级联**：UI-126 `rearm_bottom()` 在 `check_bottom()` 内同步重入 on_load_more→reload→rebuild_rows→rearm 递归，近底部时一帧内连续多批全量重建已加载行（O(k·N)），单帧工作量暴增 vsync 跳帧，加重闪烁观感。
- **修复**：
  - `ScrollBox::rearm_bottom()` 改挂起语义：只置 `pending_rearm_`，`on_layout()` 开头消费 `check_bottom()`——主循环 layout 阶段改树安全，回调重建的行控件在本次布局内即归位无乱帧；每帧至多消费一次＝每帧至多灌一批，打断同帧递归级联；
  - 新增 `Table::prebake_visible_text()`：预热表头 + visible_rows_ 全部单元格 + 尾注的 CJK 字形（转发 `Font::prebake_text`，不受配额限制且自动跳过已烘焙码点）。下游在数据灌入后调用即可消除首显/续灌闪烁。
- **回归清单**：
  1. 流加载触底续灌：新批次文字首帧即为正确字形（无方块宽回退、无渐进补齐跳动）；数据量大时每帧至多灌一批。
  2. 表格首次显示（切页签/页面）：文字完整无闪烁；第二次显示不退化。
  3. 触底级联收敛语义不变：内容底推出触发区或耗尽断链；首屏不足一屏自动补满（分帧下跨数帧完成）。
  4. UI-035/UI-094 既有触底语义零变化（mb=0 严格滚到底、滚离重武装路径未动）。
  5. 分页模式表格零行为变化（prebake 仅读数据、rearm 分帧仅非分页路径受益）。
  6. 下游 audio_text regression.ps1 全 PASS。
- **验证**：opengl_UI Release 构建+install 通过；audio_text 四表接入 prebake_visible_text，regression.ps1 全 PASS（174 项）；双仓库 graphgen --check 0 错 0 警。
- **影响范围**：`ui/include/ui/ui/scrollbox.h`、`ui/src/scrollbox.cpp`、`ui/include/ui/ui/table.h`、`ui/src/table.cpp`。

## UI-126: Table 流加载支持——触底重检 rearm + 尾注 + on_load_more（2026-08-22）
- **类型**：功能补全（audio_text 用户要求规则页四表由分页改为 layui flow 信息流滚动加载；ScrollBox 已有 `set_on_scroll_to_bottom`（UI-035/UI-094），但直接用于 Table 存在停摆缺口）
- **根因**：①`check_bottom()` 仅在 `set_scroll()`（滚动事件时机）调用，且触发一次后需滚离底部才重新武装——续灌批次若未把内容底推出触发区（用户已在底部无法再滚），加载永久停摆，缺 layui flow.load「DOM 插入后再查一次 reachBottom」语义；②Table 未暴露 body_ 触底钩子与续灌配套 API。
- **实现**：
  - `ScrollBox::rearm_bottom()`：解除 `bottom_fired_` 武装并立即 `check_bottom()`；
  - `Table::set_on_load_more(cb)`：转发 body_ 触底回调，仅非分页模式生效；`set_paginated` 内同步挂/摘钩子，防「先挂回调后开分页」误触发；
  - `Table::rebuild_rows()` 尾部自动 `rearm_bottom()`：触底→cb 续灌→重建→重检级联，直到内容底推出触发区或数据耗尽（回调方停止 append 即断链）；副作用＝首屏不足一屏自动补满（layui flow 正确行为）；同步再入递归深度以填满一屏批次数为界，有限收敛；
  - `Table::set_end_note(text)`：非分页且非空时在行尾渲染只读提示行 Label（灰字居中跨列、随内容滚动），文本变化才重建；不参与 data_row_of/选中/编辑/命中；
  - `Table::reset_scroll()`：过滤重灌后回顶。
- **回归清单**：
  1. UI-035 语义不变：mb=0 严格滚到底触发、滚离重武装；原路径零改动。
  2. UI-093/094 流加载页 demo 行为不回退（批次足够大时无感知差异；不足一屏场景反而正确补满）。
  3. 分页模式表格零行为变化：rebuild 尾部两段均被 `!paginated_` 守卫；切入分页自动摘除触底回调。
  4. 触底→续灌→重建→重检级联收敛：每批推进内容高度或耗尽停止 append 后不再触发。
  5. 尾注区域点击不穿透下层控件（on_mouse_down 越界行号兜底 return true）；尾注不参与行号映射/选中/编辑/排序。
  6. 合计行/树形/单元格表单/操作列路径不受影响。
- **验证**：opengl_UI Release 构建+install 通过；下游 audio_text FLOW-001 四表转流加载，regression.ps1 全 PASS。
- **影响范围**：`ui/include/ui/ui/scrollbox.h`、`ui/include/ui/ui/table.h`、`ui/src/table.cpp`。

## UI-125: CJK 字形逐帧烘焙配额 + prebake_text 预热（2026-08-22）
- **类型**：性能修复（audio_text 用户报告：页面切换瞬间预览区闪烁；排查确认非渲染逻辑缺陷，为首次打开新页时一帧内数十个新 CJK 码点逐字 stbtt 光栅化 + glTexSubImage2D 上传拉长单帧所致）
- **根因**：`ensure_dynamic` 无烘焙速率限制——首次绘制含新汉字的页面时，全部字形在同一帧内光栅化并上传图集，vsync 下该帧耗时激增，呈可感知卡顿/闪烁。
- **实现**：
  - 逐帧配额：按 16ms 时间窗口限量烘焙（默认 12 字形/窗口），超配额本帧返回 nullptr——draw/measure 已有方块字宽回退，文本宽度短暂近似、后续帧渐进补齐，单帧不再长卡；
  - 公开 `Font::prebake_text(const char* utf8)`：不受配额强制烘焙一段文本全部 CJK 字形，供启动预热一次性消除首绘开销（重复码点/ASCII/图标自动跳过）。
- **回归清单**：
  1. 首次打开含大量新汉字的页面：无整帧冻结观感，文字在数帧内渐进完整显示。
  2. 已烘焙字形绘制与度量不变；超配额帧内缺失字形按字号方块宽近似，不崩溃不错位累积。
  3. `prebake_text` 后同批文本首绘即为完整字形。
  4. 图集满日志行为不变（atlas full 后停止烘焙）。
  5. 下游 audio_text regression.ps1 全 PASS。
- **验证**：opengl_UI Release 构建+install 通过；下游 regression.ps1 全 PASS（163 项）。
- **影响范围**：`gl/include/ui/gl/font.h`、`gl/src/font.cpp`。

## UI-124: 表格行单击选中高亮（2026-08-22）
- **类型**：功能补全（audio_text 用户报告：表格选中后无状态反馈，无法看到当前定位到哪一行）
- **根因**：`TableRow::on_draw` 三个分支（hover / 选中 / 偶数行）全部画同一颜色 #FAFAFA——注释写"hover/选中行高亮 #f8f8f8"但从未使用区分色；且 `Table::selected_row()` 仅 radio 模式返回值、普通模式恒 -1，Table 不跟踪单击选中行，无 `set_selected_row` API。
- **修复**：
  - Table 新增 `sel_row_`（可见行号）与公开 `set_selected_row(int)`；`on_mouse_down` 行点击分支记录选中行；`reload()/clear_rows()/set_page()/sort_rows()` 时自动清除（可见行号失效场景）；
  - `selected_row()` 普通模式返回 `sel_row_`（radio 模式语义不变）；
  - 渲染区分三态：选中 = #16baaa α0.18 叠加（kSelBg）、hover = #EFEFEF（kHoverBg）、其余白/灰底；删除冗余 even_row 分支。
- **回归清单**：
  1. 单击数据行 → 该行青色半透明高亮，移开鼠标后保持；再点其他行切换。
  2. hover 行浅灰加深，与选中色可区分。
  3. reload/翻页/排序后选中态清除，无残留错位高亮。
  4. 复选/单选列表现不变；radio 模式 selected_row() 仍返回单选行。
  5. 下游 audio_text 规则页四表（数据表/正则/模式映射/快捷键）选中定位可见。
- **验证**：opengl_UI Release 构建通过并 install；下游 regression.ps1 全 PASS（163 项）+ GUI 冒烟。
- **影响范围**：`ui/include/ui/ui/table.h`、`ui/src/table.cpp`。

## UI-001: 初始化项目文档图谱（2026-08-09）

接入 graphgen 文档知识图谱：`--init` 安装引擎、opencode 技能/插件，配置 `prefixes`，
创建本条目文件作为图谱种子节点。

## UI-002: 3rdparty 依赖构建管线（2026-08-09）

新增 `scripts/build/*.ps1` 一键构建全部第三方依赖（双架构 x64/x86），
产物统一安装到 `3rdparty/install/{x64,x86}`：GLFW 3.4（CMake 安装，目标名 `glfw`）、
GLAD 2（gl.h/gl.c 生成产物）、stb（stb_image.h/stb_truetype.h 直拷）、
FFmpeg 5.1.9（MSVC shared，DLL+import lib 在 `bin/`）、OpenCV 4.5.2（world 库，嵌套布局）。

验证：x64/x86 双架构 configure+build+install 均成功，产物落位正确（见 [BUILD.md](BUILD.md)）。

## 相关历史条目

- [UI-001](CHANGELOG.md#ui-001-初始化项目文档图谱2026-08-09)：项目初始文档图谱。

## UI-003: 分层框架骨架与构建验证（2026-08-09）

搭建 core/gl/ui/media/apps 四层骨架：core（纯 C++ 数学/日志/版本）、
gl（GLFW+GLAD 开窗、Renderer 2D 矩形管线、Texture/stb_image）、
ui（Widget 基类）、media（可选 FFmpeg/OpenCV 模块，默认 OFF）、
apps（Demo：开窗+GL 3.3 Core+清屏+色块+垂直同步+FPS）。

验证：x64/x86 × media ON/OFF 三组 CMake 配置全部编译链接通过；
Demo 实际运行输出 GL 3.3.0 NVIDIA T600，开窗渲染正常（见 [ARCHITECTURE.md](ARCHITECTURE.md)）。

## 相关历史条目

- [UI-002](CHANGELOG.md#ui-002-3rdparty-依赖构建管线2026-08-09)：依赖管线是本骨架能编译的前提。

## UI-004: M1 渲染管线升级：圆角矩形 SDF + 纹理/字体 + 控件树（2026-08-09）

Renderer 升级为双管线：颜色管线（共享顶点着色器 + 圆角矩形 SDF 片元，fwidth 做 1px 抗锯齿，
半径 clamp 到 `min(w,h)/2`）、纹理管线（uUv 子矩形采样 + uColor 调制）；`draw_quad` 保留为
`draw_rounded_rect(radius=0)` 兼容别名。新增 Font（stb_truetype 烘培 ASCII 32..126 到 1024 图集，
单通道覆盖度转 RGBA，`draw_text`/`measure_text`）。Window 增加鼠标追踪（光标位置 + 三键按下/边沿事件）。
ui 层新增 Widget 树（add_child/remove_child、contains 命中、dispatch 事件分发后添加子节点在上层优先）、
Panel 容器、Button 控件（hover/pressed 状态 + 点击回调）、Theme 主题（Bootstrap primary 主色）。
Demo 演示圆角面板 + 按钮点击计数 + FPS/文本 HUD。

验证：x64/x86 两架构全部编译链接通过，Demo 实际运行 GL 3.3.0 NVIDIA T600 正常渲染
（见 [ARCHITECTURE.md](ARCHITECTURE.md) 回归清单）。

## 相关历史条目

- [UI-003](CHANGELOG.md#ui-003-分层框架骨架与构建验证2026-08-09)：骨架与双架构验证方式沿用。

## UI-005: M2 布局系统：VBox/Label + 滚轮/窗口缩放（2026-08-09）

Widget 基类新增布局钩子 `on_layout()/layout()`（递归）与滚轮事件
`on_mouse_scroll()/dispatch_mouse_scroll()`。新增 Label 文本控件（字体/颜色/水平对齐，
未显式设色时用主题文字色）、VBox 纵向布局容器（padding/spacing，按添加顺序纵向排列子控件，
等宽填满）。Window 增加滚轮回调（scroll_dx/scroll_dy 边沿量）与窗口尺寸回调
（resized() 边沿标志 + width/height 同步）。Demo 演示 VBox 居中卡片：标题 + 双按钮 + 提示文本，
按钮点击/滚轮滚动改计数，窗口缩放自适应。

验证：x64/x86 双架构编译链接通过，Demo 实际运行 GL 3.3.0 NVIDIA T600 正常（见 [ARCHITECTURE.md](ARCHITECTURE.md)）。

## 相关历史条目

- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：控件树与渲染管线是 M2 布局的基础。

## UI-006: M3 焦点系统与键盘输入（2026-08-09）

core 新增 `ui/core/keycodes.h`：键盘键值与修饰键常量（数值与 GLFW 键码对齐，
UI 层不依赖 GLFW 头）。Widget 基类新增焦点系统：`focusable` 标志、
`focus()/blur()/focused()/on_focus_changed()`、`on_key_down/up()`、Tab 焦点遍历
`focus_next()/focus_prev()`（先序收集可聚焦可见控件，Shift+Tab 反向）；
`dispatch_key_down/up()` 由根节点调用，Tab 由基类处理导航、其余键转发给焦点控件。
Window 增加键盘回调（`key_down/key_pressed/key_released`，key 用 ui::Key 常量）。
Button 设为可聚焦：点击或 Enter/Space 激活触发点击回调，聚焦态绘制 2px accent 描边
（双层圆角实现）。Demo 演示三个按钮 Tab 循环聚焦 + Enter/Space 激活 + Shift+Tab 反向。

验证：x64/x86 双架构编译链接通过，Demo 实际运行 GL 3.3.0 NVIDIA T600 正常（见 [ARCHITECTURE.md](ARCHITECTURE.md)）。

## 相关历史条目

- [UI-005](CHANGELOG.md#ui-005-m2-布局系统vboxlabel--滚轮窗口缩放2026-08-09)：控件树/事件分发是焦点系统的基础。

## UI-007: 内容自适应高度与焦点完善（2026-08-09）

解决 M2/M3 遗留问题：Widget 新增 `preferred_height()` 内容自适应高度接口（默认返回
`bounds_.h`），VBox 布局改用其作为子控件高度（Label 覆写返回字体行高，无需再手动预设
高度）；焦点抽象到基类——`dispatch_mouse_down` 命中即 `focus()`，Button 移除重复的
`on_mouse_down` 聚焦逻辑；方向键导航——`dispatch_key_down` 中 Up/Left 反向、Down/Right
正向遍历（与 Tab/Shift+Tab 同遍历顺序）。

验证：x64/x86 双架构编译链接通过，Demo 实际运行 GL 3.3.0 NVIDIA T600 正常（见 [ARCHITECTURE.md](ARCHITECTURE.md)）。

## 相关历史条目

- [UI-006](CHANGELOG.md#ui-006-m3-焦点系统与键盘输入2026-08-09)：焦点/键盘是本次完善的载体。

## UI-008: M4 文本输入：TextBox + 字符通道 + scissor 裁剪（2026-08-09）

Window 新增 Unicode 字符回调（GLFW `char_cb`），`char_pressed_count()/char_pressed_at()` 边沿
队列（本轮 poll 内收到的码点，读一次清除）。Widget 基类新增 `on_char()/dispatch_char()`
（字符事件只转发给当前焦点控件）。新增 TextBox 单行输入控件：可聚焦，支持字符输入
（UTF-8 编码、光标字节下标按字符前后移）、Backspace/Delete 删除、Home/End 跳转、
聚焦态 accent 描边 + 光标竖线，文本超宽时 scissor 裁剪 + 横向滚动保证光标可见。
Renderer 新增 `push_scissor()/pop_scissor()` 嵌套裁剪（y 向下坐标转 GL 原点在左下角）。
同时修复 M3 遗留 Bug：`dispatch_key_down` 之前只在根节点 `focused_` 为真时转发按键，
根节点从不聚焦导致 Enter/Space 无法到达聚焦控件；现改为 `focused_descendant()` 找到子树
焦点控件优先消费（TextBox 左右键移动光标），导航键未被消费才做焦点遍历，无焦点时
Tab 落到第一个可聚焦控件。Demo 演示输入框实时回显 + 按钮计数。

验证：x64/x86 双架构编译链接通过，Demo 实际运行 GL 3.3.0 NVIDIA T600 正常（见 [ARCHITECTURE.md](ARCHITECTURE.md)）。

## 相关历史条目

- [UI-007](CHANGELOG.md#ui-007-内容自适应高度与焦点完善2026-08-09)：方向键导航逻辑被本次键盘分发重构替代。
- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：Renderer 管线是 scissor 裁剪的基础。

## UI-009: M5 鼠标拖拽：Slider 滑块 + 鼠标捕获（2026-08-09）

Widget 基类新增鼠标捕获机制：`on_mouse_down` 返回 true（消费按下）即自动捕获
（`capture_mouse()` 记录到根节点），此后 `dispatch_mouse_move/up` 优先路由给捕获者
（即使指针已离开控件范围），抬起时自动释放；拖拽类控件无需关心指针越界。新增
Slider 水平滑块：点击/拖拽轨道改值（on_mouse_down 捕获后 on_mouse_move 持续更新，
左移/右移键盘微调、Home/End 跳极值），绘制轨道 + accent 填充 + 圆角手柄（聚焦描边），
范围 0..100 演示实时回显。

验证：x64/x86 双架构编译链接通过，Demo 实际运行 GL 3.3.0 NVIDIA T600 正常（见 [ARCHITECTURE.md](ARCHITECTURE.md)）。

## 相关历史条目

- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：事件分发是捕获机制的基础。

## UI-010: M6 媒体模块点亮：ImageView/VideoView（2026-08-09）

实现 media 层两个骨架控件（3rdparty 在 UI-002 已就绪，本次点亮）：

- **ImageView（OpenCV）**：`load_from_file` 经 `cv::imread` 转 RGBA 上传 `Texture`，
  `on_draw` 保持宽高比居中绘制（uv {0,0,1,1}：纹理缓冲首行=图像顶部，与渲染器 v0=上边一致）。
- **VideoView（FFmpeg）**：拉模式解码（头文件用 `VideoContext` 不透明指针隔离 C 头），
  `open_source` 打开/找流/开解码器/预建纹理；`advance(dt)` 按真实时间推进，逐帧
  `sws_scale` YUV→RGBA 后 `Texture::update()` 增量上传（复用纹理，避免每帧重建），
  文件读完自动 seek 回 0 循环；格式变化时重建 sws。
- **Texture::update()**：glTexSubImage2D 增量上传（尺寸须与 create 一致，视频逐帧用）。
- **apps/CMake**：media ON 时条件链接 `ui_media_ffmpeg`/`ui_media_opencv`、定义宏，
  POST_BUILD 拷贝 FFmpeg/OpenCV/glfw DLL 与演示素材到可执行目录。
- **Demo**：media ON 时右侧显示 ImageView（test.png）+ VideoView（test.mp4，按 dt 推进循环播放）。

验证：media ON（Release）x64/x86 双架构编译链接通过，Demo 运行正常输出
`ImageView: loaded test.png (320x240)`、`VideoView: opened test.mp4 (320x240 @ 25.00fps)`；
media OFF 默认构建回归通过（见 [ARCHITECTURE.md](ARCHITECTURE.md)、[BUILD.md](BUILD.md)）。

## 相关历史条目

- [UI-002](CHANGELOG.md#ui-002-3rdparty-依赖构建管线2026-08-09)：FFmpeg/OpenCV 依赖管线是 media 能链接的前提。
- [UI-003](CHANGELOG.md#ui-003-分层框架骨架与构建验证2026-08-09)：四层骨架中 media 层首次真正点亮。
- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：纹理管线 draw_textured_quad 被两个视图复用。

## UI-011: M7 视频解码线程化 + 播放控制（2026-08-09）

VideoView 从「拉模式解码」升级为「后台解码线程 + 主线程逐帧上传」：

- **线程模型**：`open_source` 创建解码线程后启动；解码在 `decode_loop` 中持续进行，
  GL/纹理操作全部留在主线程 `advance(dt)`，`VideoContext` 仍为解码线程独占（不透明指针，
  头文件继续不暴露 FFmpeg C 头）。
- **缓冲轮转**：解码线程 `sws_scale` 写线程私有 `decoded_rgba_`，再在互斥锁下拷贝进
  `pending_frame_`；主线程 `advance(dt)` 互斥锁下局部 vector `swap` 弹出后
  `texture_->update()`，避免持锁做 GL 调用、避免每帧分配（pending 反复复用）。
- **同步**：`running_/playing_/loop_` 原子标志 + `seek_ms_` 原子请求 + `frame_mutex_/frame_cv_`
  条件变量；解码线程 `cv.wait` 谓词（运行/播放/有 seek），暂停时不解码（仅 seek 时解一帧刷新显示）。
- **seek**：`av_seek_frame(fmt, -1, ms*1000, AVSEEK_FLAG_BACKWARD)`（stream=-1 即按 AV_TIME_BASE
  微秒）+ `avcodec_flush_buffers` + 复位 eof；暂停中 seek 也即时刷新画面。EOF 且 loop 时 seek 回 0。
- **播放控制接口**：`play/pause/set_playing/playing()/set_loop/seek()/duration()/position()`；
  position 由解码线程按帧 pts 经 stream time_base 换算（AV_NOPTS 则按 1/fps 递推）。
- **Slider 配合**：新增 `set_value_silent()`（改值不触发 on_change，用于程序同步进度避免回调循环）
  与 `dragging()` 访问器。
- **Demo**：VideoView 下方新增 Play/Pause 切换按钮、0..1 归一化进度条（拖动中不被进度同步覆盖，
  松开/回调时 seek）、`%.1f / %.1f s` 时间标签每帧刷新。

验证：media ON（Release）x64/x86 双架构编译链接通过，Demo 运行正常输出
`VideoView: opened test.mp4 (320x240 @ 25.00fps, 2.00s)`（新增时长字段），5 秒冒烟无崩溃/无报错；
media OFF 默认构建回归通过（见 [ARCHITECTURE.md](ARCHITECTURE.md)、[BUILD.md](BUILD.md)）。

## 相关历史条目

- [UI-010](CHANGELOG.md#ui-010-m6-媒体模块点亮imageviewvideoview2026-08-09)：拉模式解码被本条目线程化取代。

## UI-012: M7 播放交互收尾：EOF 回播 + 解码节流（2026-08-09）

UI-011 交互验证中暴露的两个问题修复：

- **Bug（严重度中）：循环关闭时播完片尾后按 Play 无效**。根因：解码线程在 EOF 且不循环分支
  仅 `sleep_for(5ms)` 空转，`play()` 置 `playing_` 后无法唤醒恢复，画面永远停在末帧。
  修复：新增 `eof_` 原子标志，EOF 不循环时置位并 `cv.wait`（等待 `!eof_ || seek` 唤醒）；
  `play()` 检测到 `eof_` 时先 `seek(0)` 回片头再恢复播放，`seek`/循环回卷分支同步复位 `eof_`。
- **改进：解码线程空转烧 CPU**。根因：解码远快于播放帧率（320×240 小片源），`decode_loop`
  无限推帧占满单核。修复：解码节流——每推一帧后 `sleep_until(next_push)`，`next_push` 按
  `1/fps` 步进，seek/循环回卷时重置为当前时刻以立即刷新。

验证：media ON（Release）x64/x86 编译链接通过；x64 冒烟 12s（2s 片源循环回卷约 6 次）无崩溃
无报错，进程 CPU 约 1.4s/12s（节流生效）；x86 冒烟 8s 无异常；media OFF 默认构建不受影响。

## 相关历史条目

- [UI-011](CHANGELOG.md#ui-011-m7-视频解码线程化--播放控制2026-08-09)：本次修复基于其线程模型与播放控制。

## UI-013: M8 滚动容器：ScrollBox（2026-08-09）

新增纵向滚动容器控件，为列表/长内容打基础：

- **布局**：`on_layout` 仿 VBox 按添加顺序纵向排布子控件（高度取 `preferred_height()`），
  内容允许超出可视区；滚动偏移作用于子控件 y 坐标，溢出量 `scroll_max = 内容高度 - 可视高度`。
- **裁剪**：`Widget::draw` 改为 virtual，`ScrollBox::draw` 覆写为「背景 → `push_scissor(bounds_)`
  绘制子控件 → `pop_scissor` → 滚动条」，内容越界自动裁剪。
- **滚动**：`on_mouse_scroll` 滚轮上下滚动（`dy>0` 向上，步长 `scroll_step_` 可调），无溢出时
  不消费事件放行给父节点；滚动值 clamp 到 `[0, scroll_max]`。
- **滚动条**：仅溢出时显示右缘轨道+滑块（`thumb` 高度 ∝ 可视/内容比例，位置 ∝ 偏移比例）；
  `on_mouse_down` 命中滚动条列即捕获鼠标拖拽（复用 M5 捕获机制，`on_mouse_move/up` 改值）。
- **Demo**：左侧卡片新增 150px 高 ScrollBox，20 个 Label 条目（约 480px 内容）演示滚轮滚动与
  拖拽滚动条。

验证：x64/x86 双架构 × media ON/OFF 四组构建全部编译链接通过；Demo 实际运行 GL 3.3.0
NVIDIA T600 正常（见 [ARCHITECTURE.md](ARCHITECTURE.md)）。

## 相关历史条目

- [UI-005](CHANGELOG.md#ui-005-m2-布局系统vboxlabel--滚轮窗口缩放2026-08-09)：VBox 布局思路与滚轮事件被 ScrollBox 复用。
- [UI-008](CHANGELOG.md#ui-008-m4-文本输入textbox--字符通道--scissor-裁剪2026-08-09)：scissor 裁剪是内容越界裁剪的基础。
- [UI-009](CHANGELOG.md#ui-009-m5-鼠标拖拽slider-滑块--鼠标捕获2026-08-09)：鼠标捕获机制支撑滚动条拖拽。
- [UI-009](CHANGELOG.md#ui-009-m5-鼠标拖拽slider-滑块--鼠标捕获2026-08-09)：Slider 捕获/回调机制被播放进度条复用。

## UI-014: M9 复合控件：CheckBox / RadioGroup / ComboBox（2026-08-09）

补齐表单类常用控件：

- **CheckBox**：`checkbox.h/cpp`，勾选框 + 文本标签；`set_checked/set_on_change`，点击切换，可聚焦
  （Space/Enter 激活）。
- **RadioGroup / RadioButton**：互斥单选组，`set_options` 建组，选中态与 on_change 回调；方向键切换。
- **ComboBox**：下拉选择，复用 ScrollBox 作下拉面板（`root()->add_child` + `open_popup`），点击外部
  自动关闭；选项用 Button 列表，`select()` 触发 on_change；方向键循环选择。

验证：x64 构建链接通过，Demo 冒烟运行正常，三种控件在控件树中与既有布局/焦点系统协同工作。

## 相关历史条目

- [UI-008](CHANGELOG.md#ui-008-m4-文本输入textbox--字符通道--scissor-裁剪2026-08-09)：焦点/键盘分发支撑各控件的键盘操作。
- [UI-013](CHANGELOG.md#ui-013-m8-滚动容器scrollbox2026-08-09)：ComboBox 下拉面板复用 ScrollBox。

## UI-015: M10 主题系统：暗/亮预设 + 运行期切换（2026-08-09）

- **Theme**：`theme.h` 扩展为全局样式结构（背景/按钮/强调/文字/圆角半径），`make_dark()/make_light()`
  两套预设；控件持 `const Theme*` 指针渲染时解引用。
- **运行期切换**：Demo 通过 Menu 项执行 `theme = ui::Theme::make_light()/make_dark()`，因所有控件
  引用同一地址，切换即时重绘全部控件。

验证：x64 构建链接通过，Demo 冒烟运行正常，亮/暗切换即时生效。

## 相关历史条目

- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：Theme 初始样式由此演化而来。

## UI-016: M11 多行文本排版：Label 换行（2026-08-09）

- **Label 换行**：`label.h/cpp` 支持 `\n` 多行与最大宽度自动换行，行高按字体 `line_height()`；
  文本测量用 `Font::measure_text/char_width`，中文按字形宽度换行。
- **对齐**：保持水平对齐（左/中/右）与多行垂直排布。

验证：x64 构建链接通过，Demo 冒烟运行正常，中文/英文混排换行与对齐正确。

## 相关历史条目

- [UI-005](CHANGELOG.md#ui-005-m2-布局系统vboxlabel--滚轮窗口缩放2026-08-09)：Label 基础绘制由此扩展。

## UI-017: M12 布局增强：HBox / Grid / 嵌套滚动（2026-08-09）

- **HBox**：`hbox.h/cpp` 横向布局，子控件按添加顺序从左到右（宽度取各自预设，高度填满容器；
  默认 `Valign::Stretch`，可选 `Valign::Center` 按子控件 `preferred_height` 垂直居中，见 UI-052）。
- **Grid**：`grid.h/cpp` 网格布局，行主序按列数填格，`set_row_height/set_cols`，`preferred_height()`
  按行数自适应。
- **嵌套滚动**：ScrollBox 内嵌 VBox 容器——VBox 提供 `preferred_height()` 供滚动量计算，ScrollBox
  内容超高可滚，子容器内部继续布局。VBox 等容器 `on_layout` 由 ScrollBox 触发。
- **Demo**：左侧新卡片演示 HBox 双按钮行 + Grid 2×2 按钮；主卡片 ScrollBox 改为嵌套 VBox 放 20 条目。

验证：x64 构建链接通过，Demo 冒烟运行正常，三布局混排 + 嵌套滚动正常。

## 相关历史条目

- [UI-005](CHANGELOG.md#ui-005-m2-布局系统vboxlabel--滚轮窗口缩放2026-08-09)：VBox 布局模型被 HBox/Grid 复用。
- [UI-013](CHANGELOG.md#ui-013-m8-滚动容器scrollbox2026-08-09)：ScrollBox 嵌套支持依赖其滚动量计算。

## UI-018: M13 视频增强：倍速 / 帧步进 / 精确 seek（2026-08-09）

VideoView 播放器体验补全（音量因工具链无音频输出库未做，见 PLAN）：

- **倍速**：`set_speed(0.1..4)/speed()`，主线程 `advance` 帧间隔按 `1/(fps*speed)`，解码线程节流
  同步缩放，切速即时生效。
- **帧步进**：`step_frame()` 置暂停 + `step_` 请求，解码线程从等待中唤醒解码一帧并推入交接缓冲，
  画面逐帧精确推进（不烧 CPU）。
- **精确 seek**：`seek(seconds)` 现额外记录 `seek_target_`；解码线程 `av_seek_frame(BACKWARD)`
  定位到目标前关键帧后前进解码至目标时刻（上限 60 帧守卫）再推帧，`position()` 与画面帧一致。
- **进度拖拽无跳动**：Demo 拖动时 `dragging()` 跳过进度回写，避免与用户输入打架。
- **Demo**：新增倍速 Slider（0.5x–2.0x，实时标签）、帧步进按钮、循环播放 CheckBox。

验证：media ON（Release）x64 构建链接通过，Demo 冒烟运行正常（倍速/步进/拖拽进度均无崩溃无报错）。

## 相关历史条目

- [UI-011](CHANGELOG.md#ui-011-m7-视频解码线程化--播放控制2026-08-09)：线程模型与播放控制是本次增强的基础。
- [UI-012](CHANGELOG.md#ui-012-m7-播放交互收尾eof-回播--解码节流2026-08-09)：解码节流按倍速缩放复用其思路。

## UI-019: M14 窗口系统：PanelWindow / MenuBar / 模态 / Tooltip（2026-08-09）

- **模态**：`Widget::set_modal(Widget*)` 根级路由——鼠标 move/down/up/scroll 与键盘
  down/up/char 在根节点存在模态时仅分发给模态子树，模态外一律屏蔽；`set_modal(nullptr)` 解除。
- **PanelWindow**：`panelwindow.h/cpp` 可移动/缩放窗口——标题栏拖拽移动、右下角缩放柄、
  关闭按钮（on_close 回调）、标题栏 accent 高亮 + 标题文本；子控件经 `on_layout` 约束在客户区，
  拖拽时随窗口平移。
- **MenuBar**：`menubar.h/cpp` 横向菜单按钮 + VBox 下拉面板（复用根节点 popup：`open_popup`，
  点击外部自动关闭）；`add_menu/add_item` 组合菜单，选中项回调并收起。
- **Tooltip**：`Widget::set_tooltip()` 存悬停文本，Demo 每帧取最上层悬停带提示控件绘制小面板。
- **Demo**：顶部菜单栏（File/View/Help）+ About 模态对话框（Help→About 打开并挂模态，OK/关闭钮
  解除并隐藏）+ 若干控件 Tooltip。

验证：x64 构建链接通过，Demo 冒烟运行正常（菜单下拉、About 模态开合、拖动/缩放/关闭、Tooltip
均正常）。

## 相关历史条目

- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：Button 与控件树是菜单/窗口子控件的基础。
- [UI-005](CHANGELOG.md#ui-005-m2-布局系统vboxlabel--滚轮窗口缩放2026-08-09)：VBox 提供下拉面板与对话框内容布局。
- [UI-009](CHANGELOG.md#ui-009-m5-鼠标拖拽slider-滑块--鼠标捕获2026-08-09)：鼠标捕获支撑窗口拖拽/缩放与滚动条。
- [UI-013](CHANGELOG.md#ui-013-m8-滚动容器scrollbox2026-08-09)：ComboBox/菜单下拉面板复用其滚动与 popup 机制。

## UI-020: M15 视频音频输出：AAC + 音量 + winmm 播放（2026-08-09）

VideoView 补全音轨播放（M13 遗留项，此前因工具链缺音频输出库未做，本次改用系统库 winmm）：

- **音频管线**：AAC 包送解码器 → swr 重采样为 s16 立体声 → 应用音量系数 → `AudioOut::write` 入队，
  winmm（`waveOut`，系统库无第三方依赖）异步播放；`AudioOut` 内部 pending/in_flight 双队列 +
  设备回调回收。
- **音量**：`volume()`/`set_volume(0..1)` 已实现，`handle_audio_packet` 写盘前按系数缩放样本。
- **降级静音**：无音频流 / 解码失败 / `waveOutOpen` 失败（无设备）均仅静音，不影响视频播放。

### 死锁修复（严重，M15-001）

- **严重度**：严重——winmm 播放期间 `seek` / 循环回卷触发 `flush` 后解码线程永久阻塞，
  `close` join 卡死，冒烟进程无法退出。
- **根因**：
  1. `waveOutWrite` 在持锁下调用——设备忙时该调用阻塞等回调腾位，而回调需持锁回收 → 死锁；
  2. 更深一层：把写调用移到锁外后仍会卡死——**回调线程内调用 `waveOutUnprepareHeader`
     与写线程 `waveOutWrite` 并发会卡死 VoiceMeeter/winmm 驱动**（最小复现程序证实：
     回调去掉 unprepare 后持续写入不卡）。
- **修复**：回调线程内**不调用任何 winmm API**，只把播放完的块从 `in_flight_` 移入 `retired_`；
  `waveOutUnprepareHeader` 统一由写线程 `pump()` 在锁内执行；`waveOutWrite` 永不持锁调用，
  失败块（`WAVERR_STILLPLAYING`）移回 pending 队首重试；`flush`/`close` 先解锁 `waveOutReset`
  再持锁清理。
- **验证**：media ON（Release）x64 构建链接通过；最小复现测试（回调内 unprepare）触发卡死、
  修复后稳定；Demo 冒烟 25s（2.02s 片源循环回卷约 12 次）EXITED code=0，seek/回卷 flush 无卡死，
  解码线程干净退出；media OFF 默认构建不受影响。

## 相关历史条目

- [UI-011](CHANGELOG.md#ui-011-m7-视频解码线程化--播放控制2026-08-09)：音频随视频包在解码线程处理，flush 与回卷复用其线程模型。
- [UI-012](CHANGELOG.md#ui-012-m7-播放交互收尾eof-回播--解码节流2026-08-09)：循环回卷时音频队列复位沿用其 EOF 处理。
- [UI-018](CHANGELOG.md#ui-018-m13-视频增强倍速--帧步进--精确-seek2026-08-09)：补上了其遗留的音频输出与音量项。

## UI-021: 素材加载测试工具 + Demo 指定素材播放（2026-08-09）

用 `file/` 真实素材（4 mp3 / 7 mp4 / 17 图片）做全量加载测试，新增工具与入口：

- **opengl_ui_media_load_test**：apps 新工具，遍历目录或单文件，用 OpenCV / FFmpeg / AudioOut 逐一
  加载并报告——图片（宽路径读字节 + imdecode）报尺寸/通道/位深；音视频（自定义 AVIO 宽路径打开）
  报流信息 + 视频解前 12 帧 + `--play <秒>` 对音频流（mp3/aac）解码经 winmm 真实播放。
- **Demo `--file <path>`**：指定视频文件替换默认演示素材（VideoView 加载，UTF-8 路径），
  便于用真实素材冒烟。
- **宽字符路径（回归保证）**：所有加载走 Win32 宽字符（`FindFirstFileW` 遍历、`_wfopen` 自定义
  AVIO、`CreateFileW`+`imdecode`）。原因：`cv::imread` 的窄路径按 ANSI 代码页解析 UTF-8 中文会失败；
  `std::filesystem` 迭代器对含非法 UTF-8 字节的文件名会崩溃（栈溢出快速失败）。
- **真实素材验证结果**：
  - 图片 17/17 处理：jpg/png/hdr/exr/tif 含中文与畸形文件名（含孤立代理项）全部 OK；
    `.psd` 因 OpenCV 不支持解码失败；1 个 GIF 解码失败（预期外，不阻塞）。
  - 视频 7/7：h264 解码正常（576x1024~1280x720，25/30fps），aac 音轨识别正常。
  - 音频 11 个音轨（4 mp3 + 7 aac）各真实播放 2s，无卡死无爆音，工具退出码 0。
  - Demo 加载真实 mp4（74.81s, 1280x720, h264+aac 48kHz）冒烟 8s EXITED code=0，
    音频流初始化正常、解码线程干净退出。

验证：media ON（Release）x64 构建链接通过；`opengl_ui_media_load_test file --play 2` 全量通过。

## 相关历史条目

- [UI-002](CHANGELOG.md#ui-002-3rdparty-依赖构建管线2026-08-09)：FFmpeg/OpenCV 依赖是加载测试的前提。
- [UI-010](CHANGELOG.md#ui-010-m6-媒体模块点亮imageviewvideoview2026-08-09)：ImageView/VideoView 加载路径与本次测试工具一致。
- [UI-020](CHANGELOG.md#ui-020-m15-视频音频输出aac--音量--winmm-播放2026-08-09)：AudioOut 真实播放 mp3/aac 音轨，验证其不卡死。

## UI-022: M17a layui 克隆·基础设施 + 轻量组件（2026-08-10）

开启 layui v2.13.8 组件库样式克隆里程碑（克隆依据：`3rdparty/src/layui-v2.13.8.zip` 内
`layui/css/layui.css` 提取值，硬编码进解析层，不解析 CSS 文件）：

- **Widget 类名机制**：`add_class/remove_class/has_class`（空格分隔多类名去重），layui 语法入口，
  `classes()` 供样式层遍历。
- **layui_style 解析层**：`layui_style.h/cpp`——`layui::has_class` 便捷查询、`layui::hex` 十六进制转色、
  `layui::bg_color/bg_fill` 工具色（layui-bg-red/orange/green/cyan/blue/purple/black/gray，克隆自 CSS）。
- **Theme::make_layui()**：新增语义色字段（success #16b777 / info #1e9fff / warning #ffb800 / danger #ff5722 /
  border #eee / panel 白 / text_secondary #5f5f5f / text_muted #999）+ layui 浅灰底、主色 #16baaa、radius 2px；
  Demo 主题下拉与菜单新增 "Theme: layui" 运行期切换。
- **轻量组件（克隆 layui）**：
  - Progress（layui-progress）：轨道 #eee h6 圆角 20 + 填充 #16b777；`layui-progress-big` 高 18px 内嵌白字，
    普通态右侧百分比文本 #5f5f5f。
  - Badge（layui-badge）：红底白字 h18 radius2；`layui-badge-dot` 8px 圆点；`layui-badge-rim` 白底灰字 +
    1px #eee 描边（外圈 #eee + 内缩 1px 白底双层绘制）；叠加 `layui-bg-*` 换底色。
  - Card（layui-card）：白底 radius2 + 阴影 0 1px 2px 0 rgba(0,0,0,.05)（两层衰减近似：y+1 处 alpha .05、
    y+2 处 alpha .02）、可选 header（下边框 #f8f8f8）+ 子控件纵向 body 布局。
  - Breadcrumb（layui-breadcrumb）：链接 #999 hover #16b777、当前项 cite #5f5f5f、分隔符 " / " margin 0 10px。
  - Timeline（layui-timeline）：左缘 #eee 竖线 + 20px 白底圆点（内芯 #16b777）+ 内容 padding-left 25px。
  - Rate（layui-rate）：5 颗五角星 #ffb800——已选/悬停态实心、未选态空心（warning 外轮廓 + panel 内芯
    挖空，模拟 layui 空星图标），hover 预览 + 点击取值 + scale(1.12) 放大；readonly 只读。
    星形由 Renderer 新增 `draw_polygon`（三角扇）绘制。
- **Renderer draw_polygon**：独立多边形管线（纯色片元 + 动态 VBO），凸多边形三角扇填充，像素坐标 y 向下。
- **修复（media OFF 构建回归）**：Demo Tooltip 演示块无条件引用 `vid_*` 变量（仅在 media ON 声明），
  media OFF 编译失败；已移入 `#if OPENGUI_MEDIA_FFMPEG` 块。
- **Demo**：左侧新增 "M17a: layui components" 卡片——面包屑、普通/big 进度条、badge 行（默认/rim/dot/
  bg-blue）、Rate 评分（点击取值）、Timeline 时间轴、8 色工具色横条。

验证：x64 media ON（Release）+ media OFF（Release）双构建链接通过；两配置 Demo 冒烟均 EXITED code=0
（media ON 4 秒、media OFF 4 秒，无崩溃无报错）。

### 本轮编译错误记录（防复发）

| 错误 | 根因 | 修正 |
|------|------|------|
| C3861/C2039/C2065：`create_polygon_pipeline` 未定义 | renderer.h 漏声明私有方法（cpp 已实现） | 头文件补 `bool create_polygon_pipeline();`，与实现签名一致 |
| C2065：`strlen` 未声明（layui_style.cpp） | 未包含 `<cstring>`（仅间接包含） | 显式 `#include <cstring>` |
| C2661：`Badge` 构造无 5 参数版本 | Demo 曾以 `Badge(x, y, w, h, text)` 调用，实际构造为 `(x, y, text)` | Demo 改用 HBox + `set_bounds(0,0,w,18)` 定宽 |
| C2065：`vid_seek/vid_speed/vid_toggle` 未声明（media OFF） | Tooltip 演示块无条件引用仅在 `OPENGUI_MEDIA_FFMPEG` 内声明的变量 | 三行 tooltip 移入 `#if` 块 |

## 相关历史条目

- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：Renderer 双管线是 draw_polygon 扩展的基础。
- [UI-015](CHANGELOG.md#ui-015-m10-主题系统暗亮预设--运行期切换2026-08-09)：Theme 预设工厂被 make_layui 复用。
- [UI-003](CHANGELOG.md#ui-003-分层框架骨架与构建验证2026-08-09)：media ON/OFF 构建回归框架沿用。

## UI-023: M17a 遗留完善 + M17b 表单与按钮（layui 克隆）（2026-08-10）

- **遗留完善（M17a 收尾）**：
  - Badge-rim 真实描边：外圈 #eee + 内缩 1px 白底双层绘制（替代原白底近似）。
  - Card 阴影两层衰减近似（y+1 alpha .05、y+2 alpha .02），更贴近 box-shadow 0 1px 2px。
  - Rate 空星改空心星：warning 外轮廓 + panel 内芯挖空（模拟 layui 空星图标 #ffb800 描边），
    已选/悬停实心不变。
- **Renderer draw_rect_outline**：1px 矩形描边原语（4 条薄 quad 拼合，无圆角），供按钮 primary 边框、
  Switch 边框/聚焦环、下拉面板描边等通用。
- **Button layui 变体**：`add_class("layui-btn ...")` 启用——默认 #16baaa / primary（透明 + #d2d2d2
  边框、hover 边框变 accent）/ normal #1e9fff / warm #ffb800 / danger #ff5722；hover opacity .8、active
  恢复；`layui-btn-disabled` 灰底 #fbfbfb + #eee 边框 + #d2d2d2 字且禁用点击/键盘；`layui-btn-radius`
  半高圆角；尺寸类 lg/sm/xs 影响 `preferred_height()`。菜单项/下拉项类（`layui-menu li`、
  `layui-form-select dl dd`）复用按钮绘制的分支（hover/选中高亮）。
- **Switch（layui-form-switch 克隆）**：24px 高圆角滑块——开绿底 #16b777 + 白滑块靠右 + 白字，
  关白底 + #d2d2d2 边框 + 灰滑块 + 灰字；点击/Enter/Space 切换，`on_change(bool)` 回调；聚焦环 2px accent。
- **ComboBox layui select 外观（`layui-form-select`）**：主体白底 + #eee 边框 + 圆角 2、聚焦时边框
  #16b777 + 外侧 3px 淡绿环；右侧 #c2c2c2 小三角（draw_polygon）；下拉面板白底 + #eee 边框（ScrollBox
  `layui-form-select-panel`）、选项项 hover #f8f8f8、选中 `layui-this` 绿字加粗（左对齐）。
- **Slider layui 外观（`layui-slider`）**：轨道 h4 圆角 3 #eee + 已填充段 accent 色 + 12px 白圆钮
  （hover 放大 1.2）+ 聚焦 2px accent 描边。
- **Tabs（layui-tab）**：标题条 + 内容区；`layui-tab-card`（#fafafa 标题条、激活项白底圆角）、
  `layui-tab-brief`（激活项绿字 + 2px #16b777 下划线）；点击标题切换 + `on_change(int)`。
- **Collapse（layui-collapse 手风琴）**：1px #eee 外框、#fafafa 标题条（高 42）、展开态箭头/收起态
  箭头近似、内容区白字 #5f5f5f；点击标题展开/收起，同一时刻仅一个展开（手风琴），`on_change(int)`。
- **Dropdown（layui-dropdown / layui-menu）**：layui-btn 触发器 + 白底弹出菜单（VBox `layui-menu`，
  #eee 描边 + 阴影），菜单项 hover #f8f8f8、分隔线 `layui-menu divider`（1px #eee）；复用根节点
  open_popup 弹出机制（点击外部自动关闭）。
- **Demo**：左侧 "M17b: layui buttons / switch" 卡片扩展为 "M17b: layui buttons / forms / layout"
  （h560，含按钮两行 + Switch 一对 + Tabs brief + Collapse 手风琴 + Dropdown）；M17a 卡片上移压缩至
  (40,20,360,320)、Timeline 减为 2 项。

验证：x64 media ON + media OFF 双构建链接通过；两配置 Demo 冒烟均 EXITED code=0，无崩溃无报错。
遗留：Table 及更多 layui 组件留待后续里程碑。

## 相关历史条目

- [UI-022](CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10)：M17a 类名机制与轻量组件是本次扩展基础。
- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：渲染原语族（draw_rect_outline/draw_polygon）随管线扩展。
- [UI-003](CHANGELOG.md#ui-003-分层框架骨架与构建验证2026-08-09)：media ON/OFF 构建回归框架沿用。

## UI-024: M17c 数据与弹层：Tree / NavMenu / layer 外观 / Laypage（2026-08-10）

M17c 前四件（Table 的 layui 化留待后续）：

- **Tree（`tree.h/cpp`，layui-tree 克隆）**：树形控件，`add_node(text, parent)` 以父下标建树、
  `set_expanded/is_expanded/expand_all` 控制展开、`set_on_click(index)` 点击回调。DFS 收集可见
  节点（`vis_`/`node_y_` 缓存，折叠节点的子树不进列表），行高 26、缩进 20；点击箭头区（前 2×缩进）
  切换展开/折叠、点击文本触发回调。绘制：行 hover #eee、有子节点实心三角（展开朝下/折叠朝右）、
  叶子空心小三角、文本 #555；`layui-tree-line` 类显示子级左侧竖虚线 + 横线连接。
- **NavMenu（`navmenu.h/cpp`，layui-nav-tree 克隆）**：垂直导航菜单，`add_group`/`add_item`
  组合一级项与子项；深灰底、一级项高 40、子项高 32、缩进 24；点击一级项展开/折叠、子项点击执行
  回调；当前组顶部 3px #16b777 指示条。
- **PanelWindow layer 外观（`layui-layer` 类）**：标题栏高 50（`title_height()` 按类切换，
  默认 26 不变）、白底圆角 2 + 右下偏移半透明黑阴影近似、白标题栏 + 底部 1px #eee 分隔、
  标题 #2f2f2f、右上角 ×；拖拽/缩放/关闭逻辑沿用 UI-019。
- **Laypage（`laypage.h/cpp`，layui-laypage 克隆）**：分页条，`set_total/set_page/on_change`；
  项宽 32 高 28，上一页/页码（可见页码折叠：总数 ≤7 全显，否则 首页/当前±1/末页 + 省略号）/
  下一页；当前页 #16baaa 白字、其余白底 + 1px #eee 边框；点击翻页触发 `on_change(page)`。
- **Demo**：暂未接入新控件展示（控件已可被应用直接使用）；下轮补齐展示页。

验证：x64 media OFF（Debug）ui_ui + opengl_ui_demo 编译链接通过，无警告无报错。

### 本轮编译错误记录（防复发）

| 错误 | 根因 | 修正 |
|------|------|------|
| C1083：`<string>` 打不开 | 直接调 ninja 未加载 MSVC 环境（INCLUDE 未设） | 构建前先执行 `vcvars64.bat`（或从 CLion 的 Developer 环境构建） |
| C2555：`Tree::on_mouse_move` 覆写签名不符 | Widget 的 `on_mouse_move` 返回 `bool`，声明成 `void` | 头/实现统一改为 `bool`（返回 false 不消费） |
| C2662：const 方法调用非 const `layout_nodes` | `preferred_height()` 为 const，内部调用非 const 成员 | `layout_nodes()` 改 const，`vis_/node_y_/hover_node_` 标 `mutable` |
| C2672：`std::move` 未找到匹配 | tree.cpp 未包含 `<utility>` | 显式 `#include <utility>` |
| LNK2019：`NavMenu::on_layout` 未定义 | 头声明了 `on_layout` override，实现文件缺失定义 | cpp 补 `on_layout()` 实现（内部调 `layout_groups()`） |
| C2556/C2371：`on_mouse_move` 返回类型不一致 | cpp 定义为 void、头为 bool | 定义改为 bool |

## 相关历史条目

- [UI-023](CHANGELOG.md#ui-023-m17a-遗留完善--m17b-表单与按钮layui-克隆2026-08-10)：M17b 类名机制与渲染原语是数据/弹层组件的基础。
- [UI-019](CHANGELOG.md#ui-019-m14-窗口系统panelwindow--menubar--模态--tooltip2026-08-09)：PanelWindow 拖拽/缩放/关闭逻辑被 layer 外观复用。
- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：draw_quad/draw_polygon 支撑 Tree 三角与 NavMenu 指示条。

## UI-025: M17c 收尾：Table layui 化 + Demo 展示页（2026-08-10）

M17c 补全（Table 的 layui 化 + Demo 展示页，UI-024 遗留），并修复 Table 首个编译 Bug：

- **Table layui 化（`table.h/cpp`，layui-table 克隆）**：表头灰底（#f2f2f2）+ 可排序列点击
  升→降→无循环 + 排序箭头（升序 #16baaa / 降序 #16b777）、行 hover 高亮、行底部 1px #e6e6e6
  分隔线、表头/数据行等宽对齐；双击单元格进入行内编辑（TextBox 覆盖，Enter/失焦提交写回）；
  支持纵向滚动（内部 ScrollBox）。
- **Demo 展示页（`apps/main.cpp`，media OFF 构建生效）**：M17c 区块加入 Card 容器展示 Table
  （5 列 6 行含排序列）+ NavMenu（垂直导航）+ Tree（树形）+ Laypage（分页条）+ PanelWindow
  layer 外观弹窗；media ON 构建不展示 M17c 区块（控件仍可用）。
- **Bug 修复**：`table.cpp` 自 UI-024 起未加入 `ui/CMakeLists.txt`（控件从未编译进库，仅头文件
  存在），导致首个使用触发 C2555/LNK2019 链；现加入构建并统一 `on_mouse_down`/`begin_edit`
  签名。

验证：x64 media OFF（Debug）+ media ON（Release）ui_ui + opengl_ui_demo 编译链接通过，无警告
无报错；`--smoke 4` 双配置 exit 0。

### 本轮编译错误记录（防复发）

| 错误 | 根因 | 修正 |
|------|------|------|
| C2555：`Table::on_mouse_down` 覆写签名不符 | 头声明 `void`，基类/实现为 `bool` | 头改 `bool`（与基类一致） |
| LNK2019：Table 构造/add_column/add_row/set_font/set_theme 未定义 | `table.cpp` 未列入 `ui/CMakeLists.txt`，从未编译 | CMakeLists 补 `src/table.cpp` |
| C2737/C2597/C2568/C2660：`begin_edit` 定义与声明不一致 | 头声明 6 参、cpp 定义 2 参 | 头/实现统一为 `begin_edit(int row, int col)` |

## 相关历史条目

- [UI-024](CHANGELOG.md#ui-024-m17c-数据与弹层tree--navmenu--layer-外观--laypage2026-08-10)：M17c 主条目；本次为 Table layui 化 + Demo 展示补全。
- [UI-023](CHANGELOG.md#ui-023-m17a-遗留完善--m17b-表单与按钮layui-克隆2026-08-10)：类名机制与渲染原语是 Table/Tree 等的基础。
- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：draw_quad/draw_rect_outline 支撑 Table 表头/分隔线。

## UI-026: M17d layui 克隆·图标与特殊组件（2026-08-10）

M17d 收官：iconfont 私用区图标 + 6 个特殊组件 + Demo 展示页，为 M16（示例应用）铺路：

- **Icon 控件（`icon.h/cpp` + `layui_icons.h` + `layui_style.cpp`）**：从 layui.css 2.9.18 解析出
  190 个 `layui-icon-<name>` 类名 → Unicode 码点（私用区 U+E600..E7FF），`layui_icons.h` 自动生成
  `constexpr` 常量 + `from_class()` 查表；`Icon` 控件按类名渲染单个图标，字体经
  `Font::load_icon_font()` 烘培（stbtt_BakeFontBitmap 复用 ASCII 图集管线），新增
  `Font::draw_icon()`（以中心/目标尺寸绘制）。
- **Carousel（`carousel.h/cpp`，layui-carousel 克隆）**：多页纯色底 + 标题，左右箭头三角 + 底部
  指示点 + 页码，`next/prev/advance(dt)` 自动轮播 + `on_change` 回调。
- **ColorPicker（`colorpicker.h/cpp`，layui-colorpicker 克隆）**：12x3 色相网格（HSL→RGB）+ 9 预设
  色块 + 当前色 swatch + 十六进制显示，点击改值 + `on_change`。
- **Transfer（`transfer.h/cpp`，layui-transfer 克隆）**：左右列表 + 中间 >> / << 按钮整体移动选中
  项，点击项高亮选中，标题含计数，`on_change`（右侧项）。
- **Code（`code.h/cpp`，layui-code 克隆）**：深灰底代码块 + 语言标签条 + 行号列 + 滚轮纵向滚动 +
  行首 `#` 注释高亮。
- **Laydate（`laydate.h/cpp`，layui-laydate 克隆）**：年月切换（Zeller 公式算星期）+ 7 列日期网格
  + 选中日高亮 + `on_change(y,m,d)`。
- **Upload（`upload.h/cpp`，layui-upload 克隆）**：选择文件按钮 + 文件列表（名称/大小/进度条/状态
  "上传中"/"完成"），`advance(dt)` 推进进度，`on_pick` 可自定义选择行为。
- **Demo 展示页（`apps/main.cpp`）**：窗口加高至 1280，右下新增 M17d 卡（ScrollBox 内纵向排布 8
  个图标 + 6 组件），图标字体从 `assets/iconfont.ttf` 加载；media OFF/ON 双配置均展示。
- **资源**：`assets/iconfont.ttf`（layui iconfont，私用区 190 字形）。

验证：x64 media OFF（Debug）+ media ON（Release）ui_ui + opengl_ui_demo 编译链接通过，无警告
无报错；`--smoke 4` 双配置 exit 0。

### 本轮编译错误记录（防复发）

| 错误 | 根因 | 修正 |
|------|------|------|
| C1083 找不到 `string`/`cstdint` | 未加载 vcvars64 环境（PATH 加了 CMake/ninja 但 INCLUDE/LIB 缺失） | 构建命令改用 `cmd /c "call vcvars64.bat && ninja ..."` |
| C2059 语法错误 `delete`/`export`/`return`/`template` | 图标名 `layui-icon-delete` 去前缀后与 C++ 关键字冲突，生成 `constexpr uint32_t delete` 非法标识符 | 生成脚本对关键字/数字开头标识符加 `ic_` 前缀 |
| C2059 语法错误 `404`（数字开头） | 图标名 `layui-icon-404` 去前缀后为数字，非法标识符 | 同上，数字开头加 `ic_` 前缀 |

## 相关历史条目

- [UI-025](CHANGELOG.md#ui-025-m17c-收尾table-layui-化--demo-展示页2026-08-10)：M17c 收尾，本条目为 M17d 收官。
- [UI-023](CHANGELOG.md#ui-023-m17a-遗留完善--m17b-表单与按钮layui-克隆2026-08-10)：类名机制/主题语义色是 iconfont 与特殊组件的底座。
- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：draw_textured_quad/draw_polygon 支撑图标与轮播三角。

## UI-028: layui_test 演示工程骨架 + Grid 栅格 span/offset 扩展（2026-08-10）

新增独立演示工程 `layui_test/`（克隆 layui 官方 demo 网站），并扩展 Grid 支持 layui 栅格：

- **layui_test 工程（`layui_test/`）**：独立 CMake 工程，复用仓库 core/gl/ui 静态库，仅需
  glfw3.dll + iconfont.ttf 依赖。CMakePresets（`Visual Studio 18 2026` 生成器）产出
  `layui_test.slnx`，VS 2026 直接打开即可 F5 调试；POST_BUILD 自动拷贝 glfw3.dll/iconfont.ttf。
  app.cpp：左侧 NavMenu 导航（按页分组）+ 右侧 ScrollBox 内容区；`page_registry.h/cpp` 页面注册
  表（group/title/tip/build）；`show_page(i)` 切页时 `release_tree` 释放旧页控件后重建；
  `--smoke <秒>` 冒烟自动逐页切换（验证切页无崩溃）。
- **Grid span/offset 扩展（`grid.h/cpp`）**：子控件带 `layui-col-md{n}` / `layui-col-md-offset{n}`
  类名时按 12 列基准解析 span/offset 布局（克隆 layui.css `.layui-row/.layui-col-md*`）；
  行内 span 满 12 自动换行，offset 计入已用列数；无此类名时保持原 cols_ 等宽模式不变。
  布局：单元格宽 = avail×span/12，x = avail×(已用列)/12 + offset 留白（layui 栅格无列间距）。
- **示例页**：`pages/button_page.cpp`（按钮 5 主题色 + 4 尺寸 + 禁用/圆角，克隆 layui button.html）、
  `pages/grid_page.cpp`（等分 md4×3 / 二分 md6×2 / 混合 md8+md4 / 偏移 md3+offset3+md3+md3，
  Cell 子类覆写 on_layout 让文本铺满色块）。

验证：x64 Release 构建链接通过，无警告无报错；`--smoke 5` exit 0（自动切页验证 release_tree
无崩溃）；Demo（apps/main.cpp）x64 media OFF 回归编译通过 + `--smoke 4` exit 0。

## 相关历史条目

- [UI-022](CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10)：类名机制与 layui_style 是栅格类名解析的基础。
- [UI-017](CHANGELOG.md#ui-017-m12-布局增强hbox--grid--嵌套滚动2026-08-09)：Grid 等宽模式被 span/offset 扩展保留。
- [UI-013](CHANGELOG.md#ui-013-m8-滚动容器scrollbox2026-08-09)：ScrollBox 是内容区与页面承载容器。

## UI-029: layui_test 站式布局外壳 + 徽章页克隆（2026-08-10）

按 layui 官方 demo 网站的整体布局重写 layui_test 外壳，并开始逐页克隆（徽章页先行）：

- **站式布局（app.cpp 重写）**：顶部 layui-header 横条（layui-logo「layui_test」+ 分隔 + 小标题），
  下方左侧 fixed 深灰侧边栏（layui-bg-black 背景 + NavMenu 分组层级导航）+ 右侧主区 ScrollBox 内容。
  页内顶部：小节标题 + 「预览 / 查看代码 / 帮助」三个 Tabs（复用 UI-023 Tabs 组件），
  分别承载 build 构建的内容、代码片段（Code 控件）、帮助文字（Label 列表）。
- **page_registry 加 order**：注册表新增 `order` 字段（占位/实现页统一 `{order, group, title, desc, build}`），
  用 `std::map<order,...>` 保证分组内排序稳定；全 30 页导航完整展示。
- **iconfont 回退**：`Font::load_icon_font()` 先尝试 exe 目录旁 `iconfont.ttf`，找不到再回退
  仓库 3rdparty 路径——layui_test 只需 glfw3.dll + iconfont.ttf 即可独立运行。
- **徽章页克隆（`pages/badge_page.cpp`，order 17）**：克隆 layui badge.html 五小节——基础
  （new/99+/2）、背景色（layui-bg-red/orange/green/cyan/blue/purple/black/gray 八色）、圆点
  （layui-badge-dot，可叠加底色）、描边（layui-badge-rim）、与按钮结合（layui-btn-sm 按钮 + 徽章）。
  Badge 宽度用 `preferred_width()` 自适应，行内用 HBox 横向排布；原 `{17, 基本元素, 徽章}` 占位删除。
- **其余 27 页占位**：占位页渲染「该页面尚未实现」提示，导航仍完整展示分组与页面名。

验证：x64 Release 构建链接通过，无警告无报错；`--smoke 5` exit 0（30 页逐页切换 release_tree 无崩溃）；
apps/main.cpp x64 media OFF 回归编译通过 + `--smoke 4` exit 0。

## 相关历史条目

- [UI-028](CHANGELOG.md#ui-028-layuitest-演示工程骨架--grid-栅格-spanoffset-扩展2026-08-10)：工程骨架与栅格页为本条目继承基础。
- [UI-023](CHANGELOG.md#ui-023-m17a-遗留完善--m17b-表单与按钮layui-克隆2026-08-10)：Tabs 组件被预览/代码/帮助三栏复用。
- [UI-026](CHANGELOG.md#ui-026-m17d-layui-克隆图标与特殊组件2026-08-10)：iconfont 图标字体被 header 复用（含回退）。

## UI-030: 六页基本元素克隆（面包屑/选项卡/进度条/面板/时间线/评分）（2026-08-10）

克隆 layui 官方 demo 六个页面（order 12/14/15/16/18/40），覆盖基本元素组与首个组件示例页：

- **导航/面包屑（`pages/breadcrumb_page.cpp`）**：克隆 breadcrumb.html——`add_item` 链接项（#999，
  hover #16b777）+ `add_current` 当前项 cite（#5f5f5f），分隔符 " / " 留白 10px。
- **选项卡（`pages/tabs_page.cpp`）**：克隆 tab.html 三种风格——默认 `layui-tab`、卡片式
  `layui-tab-card`、简约式 `layui-tab-brief`；标题点击切换 + `set_active` 触发 `on_change`。
- **进度条（`pages/progress_page.cpp`）**：克隆 progress.html——基本（0/50/100%）、大号
  `layui-progress-big`（40/70/100%）、隐藏文本 `set_show_text(false)`；动态示例按钮点击推进 10%。
- **面板（`pages/card_page.cpp`）**：克隆 panel.html——带标题头 Card、无标题 Card、HBox 并排三卡
  卡片集（header + body 子控件）。
- **时间线（`pages/timeline_page.cpp`）**：克隆 timeline.html——`add_item(title, content)` 标题+
  详情、仅标题两种（竖线 + 白底圆点轴 + 内芯 #16b777）。
- **评分（`pages/rate_page.cpp`）**：克隆 rate.html——基本评分（hover 预览 + 点击取值 + 状态文本
  实时显示）+ 只读评分 `set_readonly(true)`。
- 六个占位条目（12/14/15/16/18/40）从 `placeholder_pages.cpp` 删除；已实现页 9 个，占位 21 个，
  共 30 页。

验证：x64 Release 构建链接通过，无警告无报错；`--smoke 5` exit 0（30 页逐页切换无崩溃）。

## 相关历史条目

- [UI-029](CHANGELOG.md#ui-029-layuitest-站式布局外壳--徽章页克隆2026-08-10)：站式布局外壳与页面管线为本批克隆的载体。
- [UI-022](CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10)：Breadcrumb/Card/Timeline/Rate 组件本体来自 M17a。
- [UI-023](CHANGELOG.md#ui-023-m17a-遗留完善--m17b-表单与按钮layui-克隆2026-08-10)：Tabs/Progress 组件本体来自 M17b。

## UI-031: 六页组件克隆（滑块/分页/下拉菜单/树形组件/基础菜单/静态表格）（2026-08-10）

克隆 layui 官方 demo 六个页面（order 13/19/33/34/37/39），基本元素组与组件示例组各三个：

- **滑块（`pages/slider_page.cpp`）**：克隆 slider.html——基本（0..100）、自定义范围（0..20），
  拖拽/点击/键盘微调改值，on_change 实时刷新状态文本。
- **分页（`pages/laypage_page.cpp`）**：克隆 laypage.html——总数 60 全显页码、总数 100 折叠省略号，
  翻页 on_change 更新当前页文本。
- **下拉菜单（`pages/dropdown_page.cpp`）**：克隆 dropdown.html——layui-btn 触发器 + 弹出菜单
  （选项/分隔线/退出），点击外部关闭；放 HBox 保持紧凑宽度。
- **树形组件（`pages/tree_page.cpp`）**：克隆 tree.html——三级节点展开/折叠、`layui-tree-line`
  虚线连接变体；点击回调显示节点 index。
- **基础菜单（`pages/menubar_page.cpp`）**：克隆 menu.html——MenuBar 横向菜单（首页/组件/操作/
  联系我们）+ VBox 下拉项点击回调。
- **静态表格（`pages/table_page.cpp`）**：克隆 table 静态样式——5 列可排序表头 + 5 行数据、
  行点击回调 + 双击行内编辑说明。
- 删除占位条目 13/19/33/34/37/39（首版编辑误保留 5 行导致 35 页，复核后修正为 30 页）；
  已实现 15 页，占位 15 页。

验证：x64 Release 构建链接通过，无警告无报错；`--smoke 5` exit 0（30 页逐页切换无崩溃）。

## 相关历史条目

- [UI-030](CHANGELOG.md#ui-030-六页基本元素克隆面包屑选项卡进度条面板时间线评分2026-08-10)：上一批六页克隆，页面管线同源。
- [UI-024](CHANGELOG.md#ui-024-m17c-数据与弹层tree--navmenu--layer-外观--laypage2026-08-10)：Tree/NavMenu/Laypage 组件本体来自 M17c。
- [UI-025](CHANGELOG.md#ui-025-m17c-收尾table-layui-化--demo-展示页2026-08-10)：Table 组件本体来自 M17c 收尾。

## UI-032: 六页组件克隆（表单/日期/穿梭/颜色/轮播/上传）+ 控件生命周期修复（2026-08-10）

克隆 layui 官方 demo 六个页面（order 11/31/35/36/38/41）：

- **表单（`pages/form_page.cpp`）**：克隆 form.html——TextBox 输入框、ComboBox 下拉选择、CheckBox
  复选、RadioGroup 单选、Switch 开关，HBox 行排布 + 状态文本实时反馈。
- **日期与时间选择（`pages/laydate_page.cpp`）**：克隆 laydate.html——年月切换 + 7 列日期网格，
  点击选中回调 on_change(y, m, d)。
- **穿梭框（`pages/transfer_page.cpp`）**：克隆 transfer.html——待选/已选两栏 + 中间搬移按钮，
  on_change 回调右列项。
- **颜色选择器（`pages/colorpicker_page.cpp`）**：克隆 colorpicker.html——hex 显示 + 12x9 色板 +
  预设色块，取色回调。注意 Vec4 字段为 r/g/b 且位于 ui 命名空间。
- **轮播（`pages/carousel_page.cpp`）**：克隆 carousel.html——四屏背景色块，箭头/指示点切换 +
  切换回调。
- **文件上传（`pages/upload_page.cpp`）**：克隆 upload.html——选择文件 + 列表（名称/大小/进度条/状态），
  主循环 advance(dt) 驱动进度。
- 删除占位条目 11/31/35/36/38/41；已实现 21 页，占位 9 页，共 30 页。

**顺带修复两个控件生命周期 Bug（UI 库）**：
- **RadioGroup 双重释放（高危，0xC0000409）**：`~RadioGroup` 与 release_tree 都会删除 options_
  （选项为自身子控件），切页释放二次 free 崩溃。修复：析构不再删除子控件（与框架约定一致）。
- **ComboBox 弹出面板悬空（中危）**：`open()` 将 panel_ 挂到 root，析构直接 delete 未摘除，
  root 残留悬空引用。修复：析构先 `parent()->remove_child(panel_)` 再删除。

验证：x64 Release 构建链接通过，无警告无报错；`--smoke 18` exit 0（30 页两轮逐页渲染 + 释放无崩溃）。

## 相关历史条目

- [UI-031](CHANGELOG.md#ui-031-六页组件克隆滑块分页下拉菜单树形组件基础菜单静态表格2026-08-10)：上一批六页克隆，页面管线同源。
- [UI-014](CHANGELOG.md#ui-014-m9-复合控件checkbox--radiogroup--combobox2026-08-09)：CheckBox/RadioGroup/ComboBox 复合控件本体来自 M9。
- [UI-026](CHANGELOG.md#ui-026-m17d-layui-克隆图标与特殊组件2026-08-10)：Laydate/Transfer/ColorPicker/Carousel/Upload 本体来自 M17d。

## UI-027: M16 收官·示例应用（终点）（2026-08-10）

M16 收官：交付两个可运行示例应用，整合 media 层与控件体系，项目达成终点。

- **image_viewer（`apps/image_viewer.cpp`，media OpenCV ON 构建）**：图像查看器。打开（按钮 /
  Ctrl+O，Win32 宽字符文件对话框，中文路径经 WideCharToMultiByte→UTF-8 + CreateFileW 读字节 +
  cv::imdecode）、滚轮缩放（以鼠标为锚点）、左键拖拽平移、双击复位、状态栏显示文件名/尺寸/缩放
  百分比；`--smoke` 参数支持冒烟。
- **video_player（`apps/video_player.cpp`，media FFmpeg ON 构建）**：视频播放器。打开（按钮 /
  Ctrl+O，宽字符对话框）、播放/暂停（按钮 + 空格键）、进度条 seek、循环开关、倍速 0.5x..2x、
  音量 0..1、时间显示；`--smoke` 参数支持冒烟。
- **构建**：apps/CMakeLists 条件化——`media_load_test` 依赖 FFmpeg+OpenCV 双库改为两者 ON 时
  构建（此前非 media 配置编译必失败，属既有隐患）；`opengl_ui_image_viewer`/`opengl_ui_video_player`
  分别随 OpenCV/FFmpeg 开启；POST_BUILD 拷贝依赖 DLL 与素材（test.png / 测试视频）覆盖新应用。

验证：x64 media OFF（Debug）全量 + media ON（Release）全量编译链接通过；`--smoke` 双配置
opengl_ui_demo、image_viewer、video_player 均 exit 0；image_viewer 加载 test.png（320x240）、
video_player 打开 test_av.mp4（320x180@25fps，含音频）成功。

## 相关历史条目

- [UI-026](CHANGELOG.md#ui-026-m17d-layui-克隆图标与特殊组件2026-08-10)：M17d 收官，图标/特殊组件与 Demo 展示页为示例应用提供控件底座。
- [UI-010](CHANGELOG.md#ui-010-m6-媒体模块点亮imageviewvideoview2026-08-09)：ImageView/VideoView 是 image_viewer/video_player 的核心。
- [UI-020](CHANGELOG.md#ui-020-m15-视频音频输出aac--音量--winmm-播放2026-08-09)：音量/音频输出被 video_player 复用。
- [UI-021](CHANGELOG.md#ui-021-素材加载测试工具--demo-指定素材播放2026-08-09)：宽字符路径处理范式被 image_viewer/video_player 沿用。

## UI-033: 五页克隆（框架/动画/辅助元素/弹出层/数据表格）+ 根级弹出生命周期（2026-08-10）

克隆 layui 官方 demo 五个页面（order 1/20/21/30/32），补齐「布局/基本元素/组件示例」三组各最后一页，
并新增 `PageDef.destroy` 钩子托管根级弹出窗口生命周期：

- **框架（`pages/frame_page.cpp`，order 1）**：克隆 layout-admin.html——自绘 AdminFrame：黑头 logo +
  右侧水平导航、黑侧栏分组菜单（一级组 + 子项 + 3px 激活绿条）、白内容区（blockquote 引言 + 充数卡片）、
  灰底 footer；整框架在单控件内绘制，避免外层流式布局打散结构。
- **动画（`pages/anim_page.cpp`，order 20）**：克隆 anim.html——AnimCircle 自绘动画圆，点击触发
  slideDown/slideUp/scale/scaleSpring/fadeIn/fadeOut/rotate/rotateLoop 等 12 种（loop / 单次），
  渲染器无变换插值，补间全部手写（UI-033）。
- **辅助元素（`pages/auxiliar_page.cpp`，order 21）**：克隆 auxiliar.html——FieldsetTitle 章节横线、
  Fieldset 上下边框字段集、BlockQuote 引用（默认/无底）、HLine 分割线、Circle 纯圆（layui-circle）；
  共享部件收在 `pages/layui_parts.h`（全部内联，跨 TU 安全，供批量克隆页复用）。
- **弹出层（`pages/layer_page.cpp`，order 30）**：克隆 layer.html——PanelWindow（layui-layer 外观）根级
  popup 弹出；三个演示（多窗口层叠、透明询问框、公告层）+ 9 向 offset 定位（上/下/左/右/四角/居中）；
  窗口生命周期托管在**新增的 `PageDef.destroy` 钩子**：切页释放前先摘除 root 子指针再 delete，防悬空/泄漏。
- **数据表格（`pages/table_data_page.cpp`，order 32）**：克隆 table.html 数据表格——9 列 8 行本地数据
  （ID/用户名/性别/城市/签名/积分/评分/职业/财富），ID/性别/积分/评分/财富 5 列可排序 + 行点击回调 +
  双击行内编辑（复用 UI-025 Table）；官方示例走分页接口，本页以本地数据演示表格能力。
- 删除占位条目 1/20/21/30/32；已实现 26 页，占位 4 页（42/43/44/45），共 30 页。

**顺带修复批量克隆页首个编译 Bug（layui_test）**：
- **layui_parts.h `BlockQuote::set_font` 引用不存在成员 `font_`（编译错误，中危）**：BlockQuote 只有
  `label_/theme_/nm_`，字体应转发给内部 Label。修复：删除 `font_ = f;` 只保留 `label_->set_font(f)`。
  该文件此前因 CMake `file(GLOB)` 陈旧未被纳入编译，本次重跑 configure 后暴露。

验证：x64 Release 构建链接通过，无警告无报错；`--smoke 4` exit 0（30 页逐页切换 release_tree 无崩溃）。

## 相关历史条目

- [UI-032](CHANGELOG.md#ui-032-六页组件克隆表单日期穿梭颜色轮播上传-控件生命周期修复2026-08-10)：上一批六页克隆，页面管线与验证方式同源。
- [UI-024](CHANGELOG.md#ui-024-m17c-数据与弹层tree--navmenu--layer-外观--laypage2026-08-10)：PanelWindow layer 外观（layui-layer）被弹出层页复用。
- [UI-025](CHANGELOG.md#ui-025-m17c-收尾table-layui-化--demo-展示页2026-08-10)：Table 组件本体被数据表格页复用。
- [UI-019](CHANGELOG.md#ui-019-m14-窗口系统panelwindow--menubar--模态--tooltip2026-08-09)：PanelWindow 拖拽/缩放/关闭逻辑是 layer 弹出的基础。

## UI-034: 四页克隆（模板引擎/流加载/工具模块/文本行修饰）+ 克隆收尾（2026-08-10）

克隆 layui 官方 demo 最后四个页面（order 42/43/44/45），layui_test 30 页全部实现：

- **模板引擎（`pages/template_page.cpp`，order 42）**：克隆 template.html——数据源 kNews[] 5 条新闻，
  「重新渲染」按钮清空 VBox 列表并重建（数据源内偏移循环取数），演示「数据 + 模板 = 视图」；
  代码标签展示官方 JS 模板语法（{{d.xxx}} / layui.each）与 C++ 等价实现对照。
- **流加载（`pages/flow_page.cpp`，order 43）**：克隆 flow.html——「加载更多」按钮每批追加 10 条到
  VBox 列表，共 100 条，状态文本实时显示「已加载 X/100 条」；达总量后按钮文案变「没有更多了」并停止。
  滚动到底自动加载因 ScrollBox 暂无滚动到底回调，以按钮分批加载模拟（帮助注明留待扩展）。
- **工具模块（`pages/util_page.cpp`，order 44）**：克隆 util.html——digit 补零 / timeAgo 相对时间 /
  toDateString 日期格式化（strftime 实时日期）；**countDown 倒计时为手写控件 `CountdownText`**：
  on_draw 每帧读 steady_clock 计算剩余秒数实时刷新（无需 advance(dt)），按钮重新开始。
- **文本行修饰（`pages/text_page.cpp`，order 45）**：克隆 text.html——.layui-text 排版语义：标题
  分级（文字色区分）+ 正文自动换行 + BlockQuote 引用（复用 layui_parts）+ 列表 + 链接；
  行内修饰（del/ins/strong）受渲染原语限制仅作文字说明。
- 删除占位条目 42/43/44/45；占位机制注释化保留（build_placeholder 备用）；**30/30 页全实现**。

验证：x64 Release 构建链接通过，无警告无报错；`--smoke 5` exit 0（30 页逐页切换 release_tree 无崩溃）。

## 相关历史条目

- [UI-033](CHANGELOG.md#ui-033-五页克隆框架动画辅助元素弹出层数据表格-根级弹出生命周期2026-08-10)：上一批五页克隆，页面管线与验证方式同源。
- [UI-022](CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10)：layui_parts 共享部件（BlockQuote）被文本行修饰页复用。
- [UI-029](CHANGELOG.md#ui-029-layuitest-站式布局外壳--徽章页克隆2026-08-10)：站式布局外壳与页面管线承载全 30 页导航。

## UI-035: 流加载滚动到底自动模式 + 窗口点击置顶（layer.setTop）（2026-08-10）

补齐 UI-034 流加载页留待扩展的「滚动到底自动加载」，并补上 layer 弹窗的「点击置顶」，
各引入一个 UI 库新 API：

- **`Widget::bring_to_front()`（widget.h/cpp）**：把自身从父节点 `children_` 摘除并追加到末尾
  （后添加的子节点在绘制/命中上层，见 UI-004 约定），无父节点时无操作。
  **PanelWindow 点击置顶**：`on_mouse_down` 入口处调用（点击窗口任意区域即置顶），
  对应 layui `layer.setTop`；layer 弹出层页多窗口层叠时点哪个窗口哪个置顶。
- **`ScrollBox::set_on_scroll_to_bottom(cb)`（scrollbox.h/cpp）**：滚轮/拖条/代码滚动到内容底部
  （`scroll_offset >= scroll_max - 1`）时触发回调一次；`bottom_fired_` 状态在离开底部后复位，
  再次滚到底会重新触发；传 `nullptr` 清除回调。
- **flow 页底部自动加载（`pages/flow_page.cpp`）**：预览区（ScrollBox）绑定滚动到底回调，
  每批自动追加 5 条（共 30 条）到独立 VBox，达总量更新状态文本停止；
  新增 `PageDef.destroy` 钩子 `destroy_flow` 切页时 `set_on_scroll_to_bottom(nullptr)`
  清除回调，防 preview（跨页复用对象）持有本页悬空引用。

验证：ui 库 x86 Debug + layui_test x86 Release 独立工程编译链接通过，无警告无报错
（flow_page.cpp 含两个加载演示与销毁钩子编译通过）。

## 相关历史条目

- [UI-034](CHANGELOG.md#ui-034-四页克隆模板引擎流加载工具模块文本行修饰-克隆收尾2026-08-10)：flow 页本体来自 UI-034，本次补上其注明留待扩展的滚动到底自动加载。
- [UI-033](CHANGELOG.md#ui-033-五页克隆框架动画辅助元素弹出层数据表格-根级弹出生命周期2026-08-10)：`PageDef.destroy` 钩子机制来自 UI-033，`destroy_flow` 沿用。
- [UI-019](CHANGELOG.md#ui-019-m14-窗口系统panelwindow--menubar--模态--tooltip2026-08-09)：PanelWindow 拖拽/缩放/关闭逻辑与 `on_mouse_down` 入口是点击置顶的挂载点。
- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：后添加子节点在上层，`bring_to_front` 依赖此约定。

## UI-036: Font 中文懒烘培 + CJK 字体可配置接入（2026-08-10）

用户实测 layui_test 界面看不到任何中文（全 ASCII 图集），根因：
`Font` 只烘培 ASCII 32..126（`font.h` kFirstChar/kNumChars），`draw_text` 对 CJK 码点直接跳过，
导致整界面中文不可见——侧栏 NavMenu 只剩深灰底、无分组/子项层级（纯灰块）。
即使字体换成中文字体也无效（烘培范围本就限 ASCII），两处均需改。

- **`Font::load_cjk_font(path, font_index=0)`（font.h/cpp）**：加载中文字体并保留 TTF/TTC 字节 +
  `stbtt_fontinfo` 供按需光栅化；`stbtt_GetFontOffsetForIndex` 兼容 `.ttc`（font_index 选子字体）；
  失败仅打日志不阻断运行（ASCII 仍可用）。
- **`Font::load_cjk_font_auto()`**：不写死字体——优先 `OPENGUI_CJK_FONT` 环境变量（后续自定义字体
  免重编译），再依次尝试系统候选 `msyh.ttc → msyh.ttf → simhei.ttf → simsun.ttc → Deng.ttf`，
  命中路径打 info 日志。
- **懒烘培动态字形**：图集 1024→**2048x1024**；ASCII 烘培后记录打包起始行，新码点用
  `stbtt_GetCodepointBitmap` 光栅化 → 单通道转 RGBA → **`Texture::update_sub(x,y,w,h,rgba)`**
  局部上传（`GL_UNPACK_ALIGNMENT=1`）；`mutable dyn_` 表 + `ensure_dynamic` 供 const 的
  draw/measure/char_width 查表触发烘培；基线复用主字体 `ascent_` 保证中英文同行对齐。
- **接入四处**：`layui_test/src/app.cpp`、`apps/main.cpp`、`apps/image_viewer.cpp`、
  `apps/video_player.cpp` 在 `load_from_file(arial)` 后调 `load_cjk_font_auto()`。

验证：
- layui_test VS x64-Release 构建通过；运行日志 `cjk font loaded: C:/Windows/Fonts/msyh.ttc`；
  开发期临时诊断日志确认首帧懒烘培 184 个 CJK 字形（`cjk ready=1 baked glyphs=184`，疑症排除后已移除）；
  截图像素分析侧栏文字行 469 行/127 段，菜单层级可见；`--smoke` exit 0。
- 根工程 x86 Debug、media x64（image_viewer/video_player）回归构建均通过，无 FAILED/无 warning。

## 相关历史条目

- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：Font 最初仅 ASCII 图集，本次在其上扩展 CJK 懒烘培。
- [UI-010](CHANGELOG.md#ui-010-m6-媒体模块点亮imageviewvideoview2026-08-09)：`Texture::update()` 增量上传先例，`update_sub` 局部上传与之同源。
- [UI-029](CHANGELOG.md#ui-029-layuitest-站式布局外壳--徽章页克隆2026-08-10)：layui_test 独立工程运行环境（仅需 glfw3.dll + iconfont.ttf）。

## UI-037: 直角矩形 50% 半透明蒙版修复（SDF 内部距离项）（2026-08-10）

严重度：高。CJK 修复（UI-036）后中文可见，全界面"灰蒙蒙、像加了低透明度蒙版"：
深色区域（侧栏/代码块）被洗成中灰、白色发暗，直角矩形底与分隔线全部不饱和。

根因：`gl/src/renderer.cpp` 颜色管线片元着色器的圆角矩形 SDF 缺内部距离项——
`d = length(max(q, 0.0)) - uRadius`，当 `radius=0`（即 `draw_quad`，直角矩形）
时矩形内部 `q` 分量为负 → `length(max(q,0))=0` → `d=0` →
`a = 1 - smoothstep(-aa, aa, 0) = 0.5`，**所有 draw_quad 内部以 50% alpha 混合**。
`draw_rounded_rect`（radius>0，内部 `d=-r`）不受影响，故圆角控件/文字正常。
侧栏实测 (95,101,107) 恰为底色 (46,54,61) 以 50% 叠两层（ScrollBox + NavMenu 同色）于浅底：
`46*0.5+242*0.5=144` → `46*0.5+144*0.5=95`。此 bug 自 UI-004 起就存在，此前被
中文不可见掩盖。

修复：补上标准圆角矩形 SDF 的内部（负）距离项：
`d = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - uRadius`；
`radius=0` 内部 `d=-半宽` → `a=1` 全不透明；`radius>0` 圆角行为不变。

验证：
- 截图像素复核：侧栏背景精确 (46,54,61)（源码同色），内容区纯白 255、header 255；
  修复前侧栏 (95,101,107)、内容 248。
- `--smoke 4` exit 0（30 页逐页切换无崩溃）；layui_test VS x64-Release、
  根工程 x86 Debug、media x64 三路构建通过，无 FAILED/无 warning。

## 相关历史条目

- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：`draw_quad`/圆角矩形 SDF 自 UI-004 引入，本次修复其 radius=0 内部半透明缺陷。
- [UI-036](CHANGELOG.md#ui-036-font-中文懒烘培--cjk-字体可配置接入2026-08-10)：CJK 修复使中文可见后暴露本 bug，两者叠加此前表现为"界面空白/灰蒙蒙"。

## UI-038: ui_gl 库内置 draw_quad 不透明回归自检（2026-08-10）

严重度：中（防护性）。响应 UI-037 修复后用户要求"把 bug 记录进 ui_gl 库，
防止发布调用时复发"。在 `Renderer::init()` 末尾增加一次运行时自检，并把防回归
约定写进 `draw_quad` 的 API 契约与 shader 注释，形成三层防护。

根因（防复发设计）：UI-037 的 SDF 缺陷只在 `radius=0`（`draw_quad`）路径触发，
且症状"整体灰蒙蒙"容易被误认为主题/配色问题，靠人工目测难以在发布前拦截。

修复/防护：
- **① shader 注释**：`gl/src/renderer.cpp` 颜色管线片元着色器在 `d` 行加注释，
  明确禁止删除内部距离项 `min(max(q.x, q.y), 0.0)`（会致 radius=0 半透明，UI-037）。
- **② API 契约**：`gl/include/ui/gl/renderer.h` 的 `draw_quad` 注释写明
  "radius=0 矩形内部必须全不透明（UI-037 回归保证）"。
- **③ 运行时自检**：`Renderer::self_test_quad_opacity()`，init() 末尾执行一次——
  创建 32x32 离屏 FBO，清透明，绘制全屏白色 `draw_quad`，读回中心像素；
  `alpha < 250 || rgb < 250` 则 `log_error`（含 UI-037 引用）。成功时静默，不设
  NDEBUG 门控，发布/CI 启动即拦截 SDF 回归。

验证：
- **正向**：修复版 shader 下自检中心像素 (255,255,255,255) 全不透明，日志无 FAILED，
  smoke 退出 0；FBO 渲染、读回与默认帧缓冲互不污染（win 侧保持原内容）。
- **反向（回归捕获）**：临时把 shader 改回缺陷式 `d = length(max(q,0)) - uRadius`，
  重建后自检读到中心 (127,127,127,63)——恰为 a=0.5 混合 `0.5*0.5=0.25` 的预测值——
  正确触发 `renderer self-test FAILED`；随后还原 shader 复测通过。
- layui_test VS x64-Release、根 x86 Debug、media x64 三路构建通过，无 FAILED/无 warning。
- 调试教训（写入避免复发）：离屏自检纹理必须足够大（此前 4x4 太小，fwidth AA 覆盖
  全部像素导致误读）；本机 ninja 目标名是 `opengl_ui_demo` 而非 `apps/opengl_ui_demo`，
  错误的 target 名让 ninja 报 unknown target 而未重建，曾误判为"自检对修复版也报警"。

## 相关历史条目

- [UI-037](CHANGELOG.md#ui-037-直角矩形-50-半透明蒙版修复sdf-内部距离项2026-08-10)：本自检防护的缺陷根因与修复出处。

## UI-039: 轻量组件下沉 ui 库 + Grid layui-row 无背景 + clear_children 统一切页释放（2026-08-11）

严重度：中（分层约束回归）。把散落在测试程序里的 ui 能力收进 ui 库，消除「测试程序承载库逻辑」的
反复复发。

根因：Fieldset/BlockQuote/HLine/Circle/CountdownText 等辅助/轻量组件长期以 `layui_parts.h`（共享内联）
或各页面匿名命名空间（util_page 的 CountdownText）形式放在 layui_test 里；Grid 行容器（layui-row）
无背景靠 grid_page.cpp 空覆写 `on_draw` 屏蔽；切页递归释放子树靠 app.cpp 手写 `release_tree`。
同一能力分散多份、无法跨应用复用，且与「测试程序只做编排与验证」的分层约定相悖（AGENTS.md）。

修复：
- **① 轻量组件下沉 ui 库**：新增 `ui/include/ui/ui/{fieldset,blockquote,hline,circle,countdown_text}.h`
  与 `ui/src/*.cpp`，全部为 `ui::` 组件（支持 set_font/set_theme），并入 ui/CMakeLists.txt；删除
  `layui_test/src/pages/layui_parts.h`；auxiliar_page/text_page/util_page 改用 ui 库组件
  （FieldsetTitle 补齐原先漏设的 font/theme，标题文字自此正常显示）。CountdownText 一并下沉，
  默认 60s、`set_total`/`restart` 由按钮驱动。
- **② Grid layui-row 无背景**：`ui/src/grid.cpp` `Grid::on_draw` 识别 `layui-row` 类直接 return 不画
  底色（行容器）；删除 grid_page.cpp 的 RowGrid 空覆写死代码，`make_row` 用 `Grid + add_class("layui-row")`；
  apps/main.cpp M12 Grid 示例同步加 `layui-row`。
- **③ 切页释放统一走 ui 库 `Widget::clear_children()`**：app.cpp 删除手写 `release_tree`，
  `show_page` 改调 `preview/help.clear_children()`；template_page 重新渲染改 `result->clear_children()`。

验证：
- 编译：layui_test（VS x64-Release）与根工程（cmake-build-x64 Ninja）双路构建通过，无警告无报错。
- 运行：`layui_test --smoke 4` exit 0（30 页逐页切换，clear_children 切页释放无崩溃）；根
  `opengl_ui_demo --smoke 3` exit 0。
- 回归清单：ARCHITECTURE 新增 UI-039 保证项（轻量组件归属 ui 库、Grid layui-row 无背景），
  UI-033/UI-034 行同步改述为 ui 库组件。

## 相关历史条目

- [UI-033](CHANGELOG.md#ui-033-五页克隆框架动画辅助元素弹出层数据表格-根级弹出生命周期2026-08-10)：辅助元素（Fieldset/BlockQuote/HLine/Circle）随五页克隆引入，本条目将其下沉 ui 库。
- [UI-034](CHANGELOG.md#ui-034-四页克隆模板引擎流加载工具模块文本行修饰-克隆收尾2026-08-10)：CountdownText 随工具模块页引入，本条目将其下沉 ui 库。
- [UI-028](CHANGELOG.md#ui-028-layuitest-演示工程骨架--grid-栅格-spanoffset-扩展2026-08-10)：Grid 类名布局机制是 layui-row 无背景的承载基础。
- [UI-032](CHANGELOG.md#ui-032-六页组件克隆表单日期穿梭颜色轮播上传-控件生命周期修复2026-08-10)：release_tree 手写释放的历史来源，本条目统一为 `Widget::clear_children()`。

## UI-040: Table 行重建泄漏修复 + Widget::destroy() 单控件销毁 API（2026-08-11）

严重度：中（内存泄漏）。修复 UI-039 遗留的「只 remove_child 不 delete / delete 不递归子树」泄漏点，
并为「销毁单个控件及其子树」补齐 ui 库 API。

根因：
- **Table::rebuild_rows()（ui 库缺陷）**：清旧行时只 `remove_child(child)` 不 delete（注释「不负责 delete，
  仅移除」），但没有任何后续回收方——每次排序、双击编辑后重建整批 TableRow 及其子树均泄漏。
- **layer 页清理根级弹窗（暴露 ui 库能力缺陷）**：`cleanup_windows()` 手写 `remove_child(w); delete w;`，
  而 `Widget::~Widget()` 不递归删除子节点（子树由外部所有者管理，见 widget.cpp），`delete w` 后
  PanelWindow 的标题/关闭按钮/内容子树全部泄漏。

修复：
- **① ui 库新增 `Widget::destroy()`**（widget.h/cpp）：从父节点摘除 + `clear_children()` 递归释放子树 +
  `delete this` 一条龙；文档标注「调用后本控件及子树内任何指针立即失效」。供销毁单个控件及其子树
  （切页清理根级弹窗等）使用，替代测试程序手写 remove_child+delete。
- **② `Table::rebuild_rows()` 改 `body_->clear_children()`**：整批旧行递归释放，与 UI-039 统一释放约定一致。
- **③ `layer_page.cpp` `cleanup_windows()` 改 `w->destroy()`**：测试程序只做编排，销毁语义回归 ui 库。

验证：
- 编译：layui_test（VS x64-Release）与根工程（cmake-build-x64 Ninja）双路构建通过，无警告无报错。
- 运行：`layui_test --smoke 4` exit 0（30 页逐页切换含 destroy 钩子无崩溃）；根 `opengl_ui_demo --smoke 3` exit 0。
- 排查：ui/src 与 layui_test/src 全量 grep `delete / remove_child`，除上述修复外剩余均为正确单节点语义
  （ComboBox 析构自管面板与选项、close_popups 摘除不销毁 popup 由 owner 持有、commit_edit 单节点 TextBox）。
- 回归清单：ARCHITECTURE 控件生命周期行补 `Widget::destroy()`，根级弹出生命周期行改述 UI-040，新增
  Table 行重建不泄漏保证项。

## 相关历史条目

- [UI-033](CHANGELOG.md#ui-033-五页克隆框架动画辅助元素弹出层数据表格-根级弹出生命周期2026-08-10)：PageDef.destroy 钩子机制来源，本条目将其中手写摘除+delete 统一为 `Widget::destroy()`。
- [UI-039](CHANGELOG.md#ui-039-轻量组件下沉-ui-库--grid-layui-row-无背景--clearchildren-统一切页释放2026-08-11)：clear_children 统一释放约定的上一批工作，本条目补 destroy() 单节点对称能力。
- [UI-032](CHANGELOG.md#ui-032-六页组件克隆表单日期穿梭颜色轮播上传-控件生命周期修复2026-08-10)：控件生命周期防悬空/双重释放的历史约定。

## UI-041: 栅格预览对齐 layui 镜像与工具色列间隙修正（2026-08-16）

严重度：中（layui_test 栅格页显示与 layui 镜像不符；apps 等宽 Grid 不受影响）。

根因：
- **ui 库缺陷①（工具色值）**：`layui_style.cpp` kBgColors 的 `layui-bg-green`/`layui-bg-black` 与 layui.css 不符
  （green 误为 #16baaa 应为 #009688，black 误为 #2f363c 应为 #393D49），栅格预览第 3 节与 badge 页工具色块偏色。
- **ui 库缺陷②（列间隔实现）**：`grid.cpp` 对 `layui-col-space{n}` 把列内缩 `space/2`、列高扣 `space`，
  造成首列内缩、末列外扩且整行矮一截；layui.css 用负 margin 双向撑宽，列宽扣 n、列间留 n、首末列贴行边。
- **测试程序问题①（未着色格子不可见）**：`grid_cell` 未加 `grid-demo` 基础类且 `demo_fill` 无基础色兜底，
  未显式着色的格子为白底白字（镜像为 #79C48C 绿块）。
- **测试程序问题②（嵌套断点缺失）**：`nested_cell` 未设断点，内部 md3/md9/md12 被按 xs 视作单列竖排，
  栅格嵌套演示内部应为横向排布。
- **测试程序问题③（自建克隆违反分层）**：grid_page 自建 FieldTitle/Quote 克隆（UI-039 已下沉 ui 库），
  且引用色条用 theme.accent(#16baaa)，与 layui-elem-quote 的 #5FB878 不符。

修复：
- **ui 库**：
  - `layui_style.cpp`：`layui-bg-green` → #009688、`layui-bg-black` → #393D49。
  - `grid.cpp`：`layui-col-space{n}` 分支改 `set_bounds(cx, cy, cell_w - space, row_h)`，首末列贴边、列高保持 row_h。
  - `blockquote.cpp`：引用区块底改 #fafafa、左 5px 色条改 layui-elem-quote 的 #5FB878（原 theme.success）。
- **测试程序**：
  - `grid_cell` 一律补 `grid-demo` 基础类，`demo_fill` 判定顺序改为 layui-bg-* 工具色优先于 grid-demo 基础色。
  - `nested_cell` 显式 `set_breakpoint(2)`（md）+ 内衬 padding 10（对齐 layui .grid-demo）。
  - 删除自建 FieldTitle/Quote，改用 ui 库 FieldsetTitle/BlockQuote（回归 UI-039 分层）。
  - 预览行间距对齐 `.layui-row` margin-bottom 为 10（页面级覆盖；`app.cpp show_page` 切页复位 16 防跨页泄漏）。

验证：
- 编译：layui_test VS x64-Release 构建通过，无警告无报错。
- 运行逐节对照镜像：等比分列（6/6、3/12×4）/ 组合响应式（bg-green、bg-black 等工具色）/ 常规布局
  （1/12×12、75%+25%、33.33%×3、50%+50%）/ 列间隔（首末列贴边、列高 70、间隙 n）/
  列偏移（offset 计入已用列数）/ 栅格嵌套（内部 md3+md9、md12 横向排布）。
- 回归清单：ARCHITECTURE Grid 行核对仍成立（layui-row 无背景、12 列 span/offset、等宽模式不变）；
  apps `M12 Grid 2x2` 等宽模式与无 space 分支不受影响。
- 已知限制：field-title legend 参考为 20px，位图字体固定字号无法按字缩放；预览间距为统一值，
  无法逐元素复现镜像 10/20/30 的可变 margin。

## 相关历史条目

- [UI-039](CHANGELOG.md#ui-039-轻量组件下沉-ui-库--grid-layui-row-无背景--clearchildren-统一切页释放2026-08-11)：Fieldset/BlockQuote 下沉 ui 库来源，本条目删除 grid_page 自建克隆并修正 BlockQuote 配色。
- [UI-028](CHANGELOG.md#ui-028-layuitest-演示工程骨架--grid-栅格-spanoffset-扩展2026-08-10)：Grid 栅格 span/offset/col-space 语义来源，本条目修正 col-space 落位语义。

## UI-042: API 文档生成器（tools/api_gen.py）结构化解析修复（2026-08-16）

严重度：低（文档生成工具 `tools/api_gen.py`，仅影响 `docs/UI_API.md` 产出，不影响库代码运行）。

根因（结构扫描多个缺陷叠加）：
- **命名空间误判为类**：`namespace ui {` 的 `{` 回找语句起点时越过前一 `{`，
  head 混入上层命名空间体，`STRUCT_RE` 命中其内 `class` 关键字，导致每个命名空间被追加为类条目
  （window.h 曾产出 47 个重复 `Window` 类）。
- **命名空间体被当作类体解析**：`parse_class` 未按 `structs` 排除类体区间，
  类内 `public:` 之后、下一访问说明符之前的方法被当成自由函数（`void poll_events()`、`void swap_buffers()` 等）。
- **枚举未识别**：`ENUM_RE` 命名捕获写法失效，`keycodes.h` 的 `Key`/`KeyMod` 枚举缺失。
- **函数名取错标识符**：`split_decl` 取 `(` 前第一个标识符而非最后一个，
  `public: Window(...)` 之类前缀会得到 `public` 而非 `Window`。
- **语句起始行错位**：`line_of[sa]` 指向语句起始的换行符所在行，方法注释整体上移一行。
- **尾随注释按字符下标查找**：`trailing.get(ea - 1)` 误用字符下标作行号键。
- **注释标记残留**：`preceding_comment` 逐行拼接的 `//` 前缀未逐行清理。

修复（`tools/api_gen.py`）：
- `namespace ui {` 之后新增 `NS_RE`，结构扫描按 `class/struct/enum/namespace` 四类识别；
  类体区间由 `structs` 记录并整体排除，命名空间体仅切出未被类占用的区间抽自由函数/常量。
- `ENUM_RE` 修正命名捕获（`(?P<n>...)`）使其可用。
- `split_decl` 改取 `(` 前**最后一个**标识符（`re.search(..., r'([~A-Za-z_]\w*)\s*$')`）。
- 新增 `stmt_start_line()`：跳过语句前导空白后取声明所在行。
- 尾随注释改按 `line_of[ea - 1]` 行号查找；`preceding_comment` 逐条 `clean_comment` 再拼接。

验证：
- 重新生成 `docs/UI_API.md`（80368 字节，57 个文件分节）。
- 抽查：window.h 仅 1 个 `Window` 类、无游离方法；keycodes.h `Key`/`KeyMod` 枚举齐全；
  layui_style.h 命名空间内 4 个自由函数；layui_icons.h 190 项常量；widget.h/theme.h 注释正确归位。
- 图谱：`python tools/graphgen.py` 重新生成 + `--check` 0 错误 0 警告（含新节点 `DOC:UI_API`）。

## 相关历史条目

- [UI-042](CHANGELOG.md#ui-042-api-文档生成器toolsapigenpy结构化解析修复2026-08-16)：本条目即 API 文档生成器 `tools/api_gen.py` 的修复记录。
- [UI-001](CHANGELOG.md#ui-001-初始化项目文档图谱2026-08-09)：文档图谱与工具链初始化，本条目在 docs 目录新增 AUTO-GENERATED 手册节点。

## UI-043: API 总调用示例应用（apps/api_demo.cpp）与文档同步（2026-08-16）

严重度：低（新增示例应用 `apps/api_demo.cpp`，不改变 ui 库 API，无回归风险）。

新增（交付方案 C：可执行总调用示例 / 冒烟测试）：
- `apps/api_demo.cpp`：通过聚合头 `#include "ui/all.h"`（交付 B+A 验证）拉入 core/gl/ui 三层全部公开头，
  对 `docs/UI_API.md` 收录的每条公开 API 至少调用一次：
  - core：keycodes / log / math / version；
  - gl：window / renderer / texture / font（含逐帧原始绘制样例 quad/rounded/outline/polygon/scissor/textured）；
  - ui/ui：Widget 基类（子树 / class / 命中 / 捕获 / 焦点 / tooltip / 弹窗 / 模态 / destroy）+ Theme
    + `ui::layui::` 样式助手 + `ui::layui_icon::` 图标常量 + 全部 46 个控件；
  - media（条件编译）：(FFmpeg) AudioOut / VideoView，(OpenCV) ImageView。
- `apps/CMakeLists.txt`：注册 `opengl_ui_api_demo`，随 media 开关链接对应库并定义宏；
  POST_BUILD 拷贝 glfw3.dll / FFmpeg / OpenCV DLL 与素材、图标字体。
- 启动参数：`--smoke <秒>` 运行指定秒后自动退出（CI 冒烟）；`--file <视频>` 指定 VideoView 素材。

验证：
- media OFF（cmake-build-x64 Debug）编译链接通过；`--smoke 2` 运行退出码 0。
- media ON（cmake-build-media-x64 Release，FFmpeg + OpenCV）编译链接通过；`--smoke 3` 运行退出码 0，
  VideoView 成功打开素材、ImageView 成功加载 test.png；本机无音频设备时 AudioOut 优雅降级（is_open=0）。

## 相关历史条目

- [UI-043](CHANGELOG.md#ui-043-api-总调用示例应用appsapidemocpp与文档同步2026-08-16)：本条目即 API 总调用示例应用 `apps/api_demo.cpp` 的交付记录。
- [UI-042](CHANGELOG.md#ui-042-api-文档生成器toolsapigenpy结构化解析修复2026-08-16)：`docs/UI_API.md` 由 tools/api_gen.py 结构化生成，本条目逐条调用其收录 API。
- [UI-001](CHANGELOG.md#ui-001-初始化项目文档图谱2026-08-09)：文档图谱与工具链初始化，本条目延续 docs 节点同步。

## UI-044: AudioOut waveOut 回调 ABI 修复（x86 音频崩溃）（2026-08-16）

严重度：中（`ui_media_ffmpeg` 库 `AudioOut` 在 **x86 构建**下播放音频即崩溃 0xC0000005；
x64 不受影响。由 `apps/api_demo.cpp` x86 冒烟暴露）。

根因（winmm 回调签名 ABI 不匹配）：
- `AudioOut::WaveCallback` 静态成员回调声明为
  `void __stdcall WaveCallback(void*, unsigned int, unsigned long long, unsigned long long, unsigned long long)`，
  后三个参数用 **8 字节 `unsigned long long`**；
- winmm 的 `WAVEOUTCALLBACK` 参数类型为 `DWORD_PTR`（**指针宽**）：x64 为 8 字节（恰好匹配），
  x86 为 **4 字节**（不匹配）；
- x86 下调用方（winmm）按 4 字节压栈 5 个参数共 20 字节，被调函数按自身声明
  `ret 32` 弹栈，**返回地址弹栈错位 12 字节，破坏调用方栈**；
- 首个音频块写入（`waveOutWrite`）触发设备回调时即崩溃，表现为 `AudioOut::write()` 后进程
  0xC0000005（apps/api_demo、VideoView 带音频流播放均命中）。

修复（`media/ffmpeg/include/ui/media/audio_out.h` + `src/audio_out.cpp`）：
- 回调签名后三个参数改 **`size_t`**（与 `DWORD_PTR` 同宽：x86=4、x64=8），首参 `void*`（与 `UINT_PTR` 同宽）；
- 头文件补 `#include <cstddef>`；函数体仅签名类型变化，逻辑不变。

验证：
- x86 media ON（cmake-build-media-x86 Release）：`opengl_ui_api_demo --smoke 3` 与
  `opengl_ui_demo --smoke 3` 均退出码 0，`AudioOut open/write/close` 全程无崩溃，VideoView 带音频流播放正常；
- x64 media ON（cmake-build-media-x64 Release）：重建 `ui_media_ffmpeg` 后 `--smoke 3` 退出码 0；
- x64 media OFF（cmake-build-x64 Debug）：`--smoke 2` 退出码 0。

## 相关历史条目

- [UI-044](CHANGELOG.md#ui-044-audioout-waveout-回调-abi-修复x86-音频崩溃2026-08-16)：本条目即 `AudioOut` x86 崩溃的修复记录。
- [UI-043](CHANGELOG.md#ui-043-api-总调用示例应用appsapidemocpp与文档同步2026-08-16)：`apps/api_demo.cpp` x86 冒烟暴露本缺陷（写音频路径 `AudioOut`）。
- [UI-020](CHANGELOG.md#ui-020-m15-视频音频输出aac--音量--winmm-播放2026-08-09)：`AudioOut` 的原始交付条目（M15）。

## UI-045: 工具色前缀解析修复（layui-bg-* 全部渲染成黑色）（2026-08-16）

严重度：高（**ui 库** `layui::bg_fill` 对 `layui-bg-*` 全部返回黑色：栅格复杂组合 4/5 格黑底、
badge 页"不同颜色"8 徽标全黑、ui 库 `Badge` 叠加 `layui-bg-*` 也失效；回归 ARCHITECTURE
UI-041"工具色不得偏色"清单）。

根因（`ui/src/layui_style.cpp` `find()` 前缀剥离只去第一段）：
- `find()` 对 `name[0]=='l'` 只跳过第一个 `-` 之前的段：`"layui-bg-red"` → `"bg-red"`，
  随后 `while (*name=='-')` 也跳不过非 `-` 开头的 `"bg-red"`，与 8 色名（red/…/gray）永不匹配；
- `find()` 返回 `nullptr` → `bg_fill` 走 `{0,0,0,1}` 黑底（`bg_color` 走白字，恰未暴露）；
- 调用链：`grid_page.cpp demo_fill` 遍历 `layui-bg-*` 类 → `bg_fill`；`badge.cpp bg_class→bg_fill`。

修复（`ui/src/layui_style.cpp` `find()`）：剥离改为取**最后一个 `-` 之后的段**作为颜色名
（`"layui-bg-red"` → `"red"`；兼容直接传 `"red"`），无需再依赖开头 `'l'` 判断。

验证：
- `find()` 解析单测：`layui-bg-red/blue/green/black/orange/gray` 与裸 `red` 全部正确映射到色名，
  8 色查找全部命中；
- 构建：layui_test VS x64-Release 编译通过无警告无报错；
- 冒烟：`layui_test.exe --smoke 3` 退出码 0；
  - 回归清单：ARCHITECTURE UI-041 工具色行（`layui-bg-green` #009688、`layui-bg-black` #393D49
  等 8 色不偏色）经修复后成立；grid/badge 不再出现黑块。

## UI-046: Font 烘培计数误用 stbtt_BakeFontBitmap 返回值（ASCII/图标大量字形缺失）（2026-08-16）

严重度：高（ui 库 `Font::load_from_file` / `load_icon_font` 把 `stbtt_BakeFontBitmap` 返回值当作
烘培字数，实际为图集打包底边 y（bottom_y，18px 下约 18），导致 `glyphs_`/`icon_glyphs_` 只拷前
~18 个字形度量、其余全零 → 大多数 ASCII（数字 2-9、字母、符号）与图标字形缺失：栅格页 "6/12"
只渲染出 "/1"、"75%" 只剩 "/5"、"移动：12/12、桌面：8/12" 数字塌缩，且 `measure_text` 宽度错乱致
文本居中偏移）。

根因（`3rdparty/stb/stb_truetype.h` + `gl/src/font.cpp`）：
- `stbtt_BakeFontBitmap_internal` 正常返回 **bottom_y**（打包终止的 y 像素坐标，正数），溢出返回
  `-i`（已烘培字数的负数）；该返回值并非烘培成功字数（stb 1.25 L3813-3857）。
- `load_from_file`（L170）与 `load_icon_font`（L108）均 `for (i < baked)` 拷 `chardata` →
  只覆盖前 ~bottom_y 个字形。
- 其余 `glyphs_`/`icon_glyphs_` 保持全零 → `draw_text` 以 `x1<=x0` 跳过（不渲染、advance 0），
  `measure_text`/`char_width` 宽度塌缩 → 文本居中错位、字符堆积。

修复（ui 库，`gl/src/font.cpp`）：
- `load_from_file`：度量拷贝循环 `i < baked` → `i < kNumChars`（95）；保留 `baked < 0` 溢出即失败
  的既有语义（此时 chardata 未填满全部字形）。
- `load_icon_font`：度量拷贝循环 `i < baked` → `i < kIconNum`（0x200），同上。
- 两处注释注明 stb 返回值语义（bottom_y 非字数），防再次误用。

验证：
- 独立探测程序（stb v1.25 + arial 18px）确认返回 `baked=18`（bottom_y），而
  `' '/`/`'1'`/`'2'`/`'6'` 的 chardata 均含非零度量（advance 4.48/8.96 等），证明应按总量拷贝。
- 构建：cmake-build-x64（ui_gl + opengl_ui_demo）、layui_test VS x64-Release 均通过，无警告无报错。
- 运行：`--shot` 截图像素核对——栅格页 "6/12" 两格均为完整 '6'/'/'/`'1'`/`'2'` 字形（占宽 ~26px，
  修复前仅 ~11px 塌缩 "/1"）；"移动：12/12、桌面：8/12" 等混排文本数字齐全；`--smoke 3` exit 0
  （30 页逐页切换正常）。
- 回归清单：ARCHITECTURE Font ASCII 32..126 全量烘培行与 CJK/图标懒烘培行仍成立（新增本条目
  保证项：任意 ASCII/图标字形度量不得缺失）。

## 相关历史条目

- [UI-046](CHANGELOG.md#ui-046-font-烘培计数误用-stbttbakefontbitmap-返回值ascii图标大量字形缺失2026-08-16)：本条目即 ASCII/图标烘培计数误用修复记录。
- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：Font 首次交付（stb_truetype 烘培 ASCII 32..126），本条目修正其烘培结果计数误用。
- [UI-036](CHANGELOG.md#ui-036-font-中文懒烘培--cjk-字体可配置接入2026-08-10)：ASCII 烘培路径沿用于本条目修正，CJK/图标懒烘培不受影响。
- [UI-041](CHANGELOG.md#ui-041-栅格预览对齐-layui-镜像与工具色列间隙修正2026-08-16)：栅格预览对齐 layui 镜像，本条目使栅格数字/百分比文本完整渲染。

## UI-047: 布局外壳 NavMenu 折叠不回收空间 / 切页滚动残留 / 图片加载能力缺失（2026-08-16）

严重度：中~高（布局「框架」组件实测四类问题；均属 **ui 库** 能力缺陷，测试程序只做装配）。

问题与根因：
1. **折叠问题（NavMenu）**：`ui/src/navmenu.cpp` 的 `layout_groups()` 与 `preferred_height()`
   完全忽略 `expanded` 状态——折叠分组的子项仍占布局高度且计入内容高度：侧栏出现空白间隙、
   下方分组被顶开、sidebar ScrollBox 内容高度不随折叠收缩、滚动范围错误（折叠后仍可滚到空白）。
   `on_draw` 虽按 `expanded` 跳过绘制，但空间始终未回收。
2. **nav 点击切换 / 滚动问题（show_page）**：切页时只 `clear_children()` 重建内容，未复位
   ScrollBox 滚动偏移——新页继承旧页滚动位置（深滚后的旧页 → 新页出现在半截/错位），且
   `scroll_max()` 按旧内容高度计算；sidebar 折叠后内容高度错误也放大该问题。
3. **图片加载问题（ui 库无图片能力）**：ui 层无图片控件；layui_test 全应用无任何图片加载路径，
   框架页（layout-admin 克隆）头部头像缺失（真实 layout-admin.html 头部有 `layui-nav-img` 头像）。
   注：layui demo `avatar/*.jpg` 实为 HTML 404 死链文件，改用官方 `logo.png`（252x92，浅灰）。

修复（ui 库 + 测试程序装配）：
- `navmenu.cpp` `layout_groups()`：折叠组 `continue` 不排子项；`preferred_height()` 只累计
  `expanded` 组的子项高度——折叠即时回收空间，sidebar 内容高度/滚动范围随折叠正确收缩。
- `scrollbox.h/cpp`：新增公开 `ScrollBox::reset_scroll()`（滚动回顶部并重武装触底回调）。
- `app.cpp` `show_page`：切页时 `preview.reset_scroll()` + `help.reset_scroll()`（滚动复位）。
- **新增 ui 库 `Image` 控件（`ui/include/ui/ui/image.h` + `ui/src/image.cpp`）**：`load_from_file()`
  （gl::Texture stb_image 解码）+ `preferred_height()` 按显示尺寸 + `on_draw` 铺满 bounds 绘制；
  未加载空白不绘制。ui/CMakeLists.txt 加入编译。
- `frame_page.cpp` AdminFrame：黑头最右新增 36x36 头像 `ui::Image`（on_layout 定位），
  路径回退链（exe 目录 logo.png → 3rdparty 相对路径）；`layui_test/CMakeLists.txt` POST_BUILD
  拷贝 logo.png。
- `app.cpp`：新增 `--shot-page <n>` 验证钩子（首帧截图指定页，供脚本化像素核验）。

验证：
- 构建：layui_test VS x64-Release（重新 configure 后 MSBuild）编译链接通过，无警告无报错。
- 冒烟：`layui_test.exe --smoke 6` exit 0（30 页逐页切换，show_page reset_scroll/clear_children
  无崩溃无泄漏）。
- 图片：`--shot-page 1` 截框架页 → 像素核验头像区域（x∈[1190,1226], y∈[195,231]）含浅灰 logo
  像素（240,240,240 系），非纯黑空白，证明 Image 解码+绘制链路正常；加载失败路径仅 CWD 相对
  路径首候选 miss，回退链命中仓库源文件（无 "avatar load failed"）。
- 回归清单：ARCHITECTURE UI-024 NavMenu 行（点击一级展开/折叠、子项回调、3px #16b777 指示条）
  与 UI-029 外壳布局行仍成立；新增保证项：折叠组不占子项空间、切页滚动复位、Image 可加载并
  铺满绘制。

## 相关历史条目

- [UI-024](CHANGELOG.md#ui-024-m17c-数据与弹层tree--navmenu--layer-外观--laypage2026-08-10)：NavMenu 组件本体；本次修正其折叠空间回收。
- [UI-029](CHANGELOG.md#ui-029-layuitest-站式布局外壳--徽章页克隆2026-08-10)：layui_test 站式外壳；本次修正切页滚动复位并接入头像。
- [UI-013](CHANGELOG.md#ui-013-m8-滚动容器scrollbox2026-08-09)：ScrollBox 滚动容器；本次新增 reset_scroll。
- [UI-039](CHANGELOG.md#ui-039-轻量组件下沉-ui-库--grid-layui-row-无背景--clearchildren-统一切页释放2026-08-11)：子树统一释放；本次 show_page 滚动复位沿用 clear_children 生命周期。
- [UI-003](CHANGELOG.md#ui-003-分层框架骨架与构建验证2026-08-09)：gl::Texture（stb_image）为 Image 控件解码底座。
- [UI-046](CHANGELOG.md#ui-046-font-烘培计数误用-stbttbakefontbitmap-返回值ascii图标大量字形缺失2026-08-16)：同为 2026-08-16 验证链路（构建+--shot 像素核验）。

## UI-048: Image 资产定位依赖 CWD / 加载噪音错误（2026-08-16）

严重度：中（ui 库 `Image::load_from_file` 只按 CWD 相对解析资产，调用方被迫堆叠
多级相对路径回退链，每级 miss 都触发 gl 层 `stbi_load failed` [error]，每次启动刷屏噪音）。

根因（ui 库）：
- `Image::load_from_file(path)` 直接把相对路径交给 `Texture::load_from_file`，资源定位完全
  依赖进程工作目录；`frame_page.cpp load_avatar_file` 用 6 个 CWD 相对候选凑命中（属本应 ui
  库承担的资产定位逻辑），前 5 个 miss 各打一条 `[ui][error] stbi_load failed: can't fopen`，
  从仓库根 / 其他 CWD 启动必刷噪音。

修复（ui 库；测试程序回归编排）：
- `ui/src/image.cpp` `Image::load_from_file`：裸文件名（无路径分隔符）先按 CWD 解析，
  miss 回退 **exe 目录同名文件**（Windows `GetModuleFileNameW`，非 Win32 返回空），
  资产定位不再依赖 CWD；
- 加载前先做存在性预检（fopen 探测），仅对真实存在的候选调用 `Texture::load_from_file`，
  不再对不存在路径触发 gl 层 [error]；全部 miss 只 log 一次
  `image: cannot open <path> (cwd / exe dir)`；
- `layui_test/src/pages/frame_page.cpp`：`load_avatar_file` 收敛为单次
  `load_from_file("logo.png")`，删除 6 候选 CWD 相对路径链（该逻辑回归 ui 库）。

验证：
- 构建：layui_test Debug + Release（layui_test/build/x64-Release 重新 configure 后 MSBuild）
  编译链接通过，无警告无报错。
- 多 CWD 启动 `--smoke 2`：exe 目录 / 仓库根 / 其他目录启动均 **0 条 `[ui][error]`**，
  头像正常加载（无 "avatar load failed"）。
- 缺资源路径：临时改名 exe 目录 logo.png → 仅 1 条 `image: cannot open logo.png (cwd / exe dir)`，
  行为明确可查。
- 渲染：仓库根 CWD `--shot-page 1 --shot`（PAM）像素核验，UI-047 头像区
  （x∈[1190,1226], y∈[195,231]）含 69 个 224 系浅灰 logo 字形像素，证明 exe 目录回退
  命中的 logo.png 解码 + 绘制链路正常（此区此前依赖仓库 3rdparty 相对路径命中）。
- 回归清单：ARCHITECTURE Image 行（可加载并铺满绘制）仍成立；新增保证项：
  裸文件名资产从 exe 目录可加载、加载过程无噪音错误。

## 相关历史条目

- [UI-047](CHANGELOG.md#ui-047-布局外壳-navmenu-折叠不回收空间--切页滚动残留--图片加载能力缺失2026-08-16)：Image 控件本体（gl::Texture stb_image 解码底座）；本次补资产定位能力。
- [UI-029](CHANGELOG.md#ui-029-layuitest-站式布局外壳--徽章页克隆2026-08-10)：依赖三件套 POST_BUILD 拷贝；本次 exe 目录回退与该拷贝协同。
- [UI-003](CHANGELOG.md#ui-003-分层框架骨架与构建验证2026-08-09)：gl::Texture（stb_image）为 Image 控件解码底座。

## UI-049: 框架页迷你复刻不可交互 / 用户区不符网站 / 右上角 HUD 闪烁（2026-08-16）

严重度：中（`布局>框架` 页 mini AdminFrame 纯自绘无任何鼠标交互，侧栏折叠区域等点击无响应；
用户区设计不符官网；测试程序右上角 HUD 每秒只闪一帧、看不清内容）。

根因（测试程序，ui 库无缺陷）：
- `frame_page.cpp` `AdminFrame` 把侧栏菜单 / 头部导航 / 头像全部在 `on_draw` 用 `draw_text` 自绘，
  没有真实子控件、也未覆写 `on_mouse_*`，任何点击都无响应（侧栏「折叠区域」本应点一级组
  展开/折叠）；
- 头部右侧只画了一个裸头像，缺 `layout-admin.html` 的「头像 + tester 名称 + 下拉箭头」；
- `app.cpp` 右上角 HUD 只在 1s tick 触发的那一帧 `snprintf + draw_text`，每秒仅可见约 16ms，
  表现为 1Hz 闪烁且内容无法辨认。

修复（测试程序；复用 ui 库组件，未在测试程序内复造 ui 库逻辑）：
- 侧栏折叠菜单：自绘菜单文本替换为真实 `ui::NavMenu` 子控件（4 组：menu group 1[4 项] /
  menu group 2[3 项] / click menu item / the links），`setup_menu()` + `set_active(0)+expand_all()`；
  点击一级组即展开/折叠（折叠即时回收子项空间）、hover/3px 绿条由组件自带（UI-047 语义）；
- 头部导航：logo 后从左到右排布，`AdminFrame::on_mouse_move/down` 命中测试，
  hover 变绿、点击高亮（layui-nav 语义）；用户区（头像 + "tester" + "v" 箭头，右对齐、垂直居中）
  对齐 `layout-admin.html`，点击经 `open_popup`（owner=不可见 trigger 子控件）弹出
  Your Profile / Settings / Sign out 菜单，点击外部自动关闭；
- `AdminFrame` 析构负责回收可能挂到 root 的下拉弹层（remove_child + clear_children + delete），
  避免切页泄漏；
- `app.cpp`：HUD 改为「每秒 tick 只更新 `char hud[64]` 缓冲，每帧绘制」消除闪烁，
  颜色 `text_muted` → `text_secondary` 提高可读性。

验证：
- 构建：layui_test Debug + Release 编译链接通过，无警告无报错。
- 渲染：`--shot-page 1 --shot`（PAM）像素核验（1280x900，mini 框架 x∈[244,1256]）：
  头部黑条右侧出现用户区三段——头像 logo 字形（x≈1126..1164）、"tester" 浅色文本（x≈1168..1204）、
  箭头（x≈1216..1233，0.7 灰）；侧栏 NavMenu 呈现多行分组/子项文本行；HUD 从第 1 秒起每帧常驻
  （截图首帧无 HUD 属预期：shot 在首个 1s tick 前退出）。
- 冒烟：仓库根 CWD `--smoke 6` 0 条 `[ui][error]`、exit 0，30 页切页无崩溃
  （AdminFrame 析构回收弹层路径无泄漏/悬空）。
- 回归清单：ARCHITECTURE 框架页行（mini 复刻结构）仍成立；新增保证项：
  侧栏菜单点一级组折叠/展开可交互、用户区含名称+箭头且点击弹菜单、右上角 HUD 常驻可读。

## 相关历史条目

- [UI-047](CHANGELOG.md#ui-047-布局外壳-navmenu-折叠不回收空间--切页滚动残留--图片加载能力缺失2026-08-16)：NavMenu 折叠空间回收语义；本次在 mini 框架接入。
- [UI-024](CHANGELOG.md#ui-024-m17c-数据与弹层tree--navmenu--layer-外观--laypage2026-08-10)：NavMenu / Dropdown 组件本体；本次复用 NavMenu + open_popup 弹层模式。
- [UI-033](CHANGELOG.md#ui-033-五页克隆框架动画辅助元素弹出层数据表格-根级弹出生命周期2026-08-10)：根级弹层生命周期；本次 AdminFrame 析构对齐回收。

## UI-050: 框架页内容主体可滚动 + 头部 nav groups 折叠菜单 + NavMenu 子项选中态（2026-08-16）

严重度：中（`布局>框架` 页 mini 复刻与官网行为仍有差距：内容主体固定不可滚动；
头部 "nav groups" 未做下拉折叠菜单；侧栏菜单点击子项无选中高亮；ui 库 NavMenu 缺
`.layui-this` 选中项能力）。

根因（ui 库能力缺失 + 测试程序未对齐网站）：
- ui 库 `NavMenu` 只有一级组顶部 3px 指示条，无子项选中态 API/绘制（网站
  `layui-nav-tree` 点击子项后置 `.layui-this`：绿底 #009688 白字），属 ui 库能力缺陷；
- `frame_page.cpp` 内容主体在 `AdminFrame::on_draw` 固定绘制，无滚动；
- "nav groups" 与 nav 1/2/3 同为点击高亮，未实现 `layout-admin.html` 的 `layui-nav-child`
  折叠菜单（menu 11/22/33）。

修复（ui 库 + 测试程序各归其位）：
- ui 库 `NavMenu` 新增 `set_selected(group, item)`（`.layui-this` 语义：选中项绿底
  `#16b777` + 白字，同时置 active 组）；子项点击自动选中（layui-nav-tree 行为）；
- `frame_page.cpp`：内容主体改为 `ui::ScrollBox`（`layui-body` 白底）子控件 + 自定义
  `BodyContent` 高内容控件（blockquote + 充数卡片，preferred_height 640 > 视口 484），
  滚轮/拖动滚动条滚动，scissor 裁剪；点击 nav 1/2/3 时 `reset_scroll()` 回到顶部；
- 头部 "nav groups" 改为折叠菜单：`open_popup`（owner=该导航项 trigger）弹出
  menu 11/22/33，与用户区弹层互斥关闭、点外部自动关闭、切页由析构回收；
- 侧栏菜单默认 `set_selected(0, 2)` 选中 "menu 3"（演示选中态）；
- `app.cpp`：`show_page` 按 (组, 子项) 记录映射并 `nav.set_selected(...)` 同步外壳侧栏
  选中态（切页/`--shot-page`/冒烟首帧一致）。

验证：
- 构建：layui_test Debug + Release 编译链接通过，无警告无报错。
- 渲染：`--shot-page 1 --shot` 像素核验（1280x900）：
  - 内容主体滚动条出现：x∈[1236,1242] 纵带 = 默认主题滚动条轨道（button_pressed）
    + 滑块（text_dim），内容高于视口（640>484）溢出成立；
  - mini 框架侧栏 "menu 3" 绿底选中（`#16b777` 大面积命中侧栏项区）；
  - 外壳侧栏当前页子项绿底选中（show_page 同步生效）。
- 冒烟：仓库根 / exe 目录 CWD `--smoke 3` 0 条 `[ui][error]`、exit 0，切页无崩溃。
- 回归清单：ARCHITECTURE NavMenu 行（折叠回收空间、3px 指示条）+ 框架页行仍成立；
  新增保证项：内容主体可滚动、nav groups 折叠菜单、NavMenu 子项选中态、外壳侧栏
  选中态随切页同步。

## 相关历史条目

- [UI-049](CHANGELOG.md#ui-049-框架页迷你复刻不可交互--用户区不符网站--右上角-hud-闪烁2026-08-16)：本次在其基础上补内容滚动 / nav groups 折叠菜单 / 菜单选中态。
- [UI-024](CHANGELOG.md#ui-024-m17c-数据与弹层tree--navmenu--layer-外观--laypage2026-08-10)：NavMenu 组件本体；本次扩展选中态 API。
- [UI-047](CHANGELOG.md#ui-047-布局外壳-navmenu-折叠不回收空间--切页滚动残留--图片加载能力缺失2026-08-16)：ScrollBox 滚动/切页复位语义；本次 body 复用其滚动。

## UI-051: make_layui button_normal 误设主色导致布局容器全绿（2026-08-16）

严重度：中（**ui 库** Theme 预设 `Theme::make_layui()` 把通用控件底色 `button_normal` 设为主色
#16baaa——HBox/VBox/Grid/Panel/ScrollBox/TextBox/CheckBox/Radio/MenuBar/PanelWindow 全部以
`button_normal` 为底色，所有 layui 页面布局容器/间隙/面板渲染成绿色；按钮页每行行尾出现大绿块）。

根因（ui 库，`ui/src/theme.cpp`）：
- `make_layui()` 将 `t.button_normal` 设为 `{0.09f, 0.73f, 0.67f}`（主色 #16baaa），但该字段语义是
  「控件/面板中性底色」（make_dark/make_light 中均为灰阶）；而按钮 layui 绘制路径
  （button.cpp `layui-btn` 分支）本身**不使用** `button_normal`（默认用 `t->accent`、primary 透明、
  normal/warm/danger 用语义色），因此该值对按钮无任何正收益，只污染容器背景：
  - HBox/VBox 行容器按 `button_normal` 铺满整行 → 按钮间 8px 间距与行尾空白全绿；
  - primary 透明按钮透出绿色行底（视觉上「原始按钮」变绿）。

修复（ui 库）：
- `ui/src/theme.cpp` `make_layui()`：`button_normal` 改回中性白 `{1.0f, 1.0f, 1.0f, 1.0f}`
  （与 `t.panel` 白、`layui-body` 白底一致）；按钮绘制与其余语义色不变。

验证：
- 构建：layui_test VS x64-Release MSBuild 编译链接通过，无警告无报错。
- 渲染：`--shot-page 2`（按钮页）像素核验——r1 基础按钮行：原始按钮透明（白底）、默认绿、
  主要蓝、暖色橙、危险红、禁用灰，96px 按钮 + 8px 白色间距正确；r2 边框按钮全透明 + 彩色边框；
  r5 primary 尺寸按钮透明 + 灰字；r8 危险图标按钮红底白图标（44px）；r10 删除红 + 禁用灰；
  行尾无大绿块。全页绿色像素仅存在于 accent 按钮/边框自身。
- 回归：grid 页 / badge 页 `--shot-page` 绿色像素 0%（仅侧栏 accent 指示）；`layui_test --smoke 6`
  exit 0（30 页切页无崩溃）。
- 回归清单：ARCHITECTURE Theme 行（make_layui 浅灰底、accent #16baaa、radius 2px）仍成立；
  新增保证项：make_layui `button_normal` 为中性底色（非主色），布局容器/面板/输入框等不染色。

## 相关历史条目

- [UI-022](CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10)：`Theme::make_layui()` 首次交付（浅灰底、主色 #16baaa），本条目修正其 `button_normal` 误设。
- [UI-023](CHANGELOG.md#ui-023-m17a-遗留完善--m17b-表单与按钮layui-克隆2026-08-10)：Button layui 变体绘制路径不依赖 `button_normal`，故按钮外观不受本修复影响。
- [UI-039](CHANGELOG.md#ui-039-轻量组件下沉-ui-库--grid-layui-row-无背景--clearchildren-统一切页释放2026-08-11)：Grid `layui-row` 无背景语义，与本次容器中性底色一致。

## UI-052: 按钮页三缺陷：尺寸行无大小差 / 按钮组后续按钮缺失 / 按钮容器缺失（2026-08-16）

严重度：中。按钮页（按钮页，`--shot-page 2`）对照 layui button.html 镜像存在三处不符：
（1）「不同尺寸」行 lg/默认/sm/xs 全部被拉成同一高度，大小差异不可见；
（2）「按钮组」小节只渲染第一组 g1，后续 g2/g3 缺失；
（3）「按钮容器」小节整段缺失。

根因（分属 ui 库 与 测试程序）：
- **（1）ui 库**：`HBox::on_layout`（`ui/src/hbox.cpp`）把所有子控件高度拉伸到
  `content_h`（填满行高），与 layui 行内按钮 `vertical-align:middle` + 各自固有高度
  （`.layui-btn{height:38px}`、`-lg{44px}`、`-sm{30px}`、`-xs{22px}`）语义冲突；
  `Button::preferred_height()`（`ui/src/button.cpp`）已按类名返回 44/30/22，被 HBox 拉伸掩盖。
- **（2）测试程序**：`button_page.cpp` 创建并填好 g2/g3 后**从未加入控件树**
  （只 `host.content->add_child(g1)`），无 UI 库能力问题。
- **（3）缺能力（ui 库）**：缺少「流式换行容器」，layui `.layui-btn-container`（按钮右/下
  10px margin 自动换行）无法表达；测试程序页本身也没有该小节。

修复：
- **ui 库（1）**：`hbox.h` 新增 `enum class Valign { Stretch, Center }` + `set_valign()`，
  `on_layout` 在 `Center` 时按子控件 `preferred_height()` 垂直居中（默认仍 `Stretch`，
  兼容既有调用方，api_demo/layer/form/dropdown/card/badge/anim 的 HBox 行布局不变）；
  按钮页 `button_row()` 统一 `set_valign(Valign::Center)`。
- **测试程序（2）**：g1/g2/g3 放入同一 HBox 行（`set_spacing(10.f)` 对应 layui
  `.layui-btn-group+.layui-btn-group{margin-left:10px}`），g1/g2/g3 均 `grp_row->add_child`；
  组宽修正为按钮宽度和（288/160/180）。
- **ui 库（3）**：新增 `FlowBox`（`ui/src/flowbox.cpp` + `ui/include/ui/ui/flowbox.h`）——
  流式换行容器，子项按 `bounds().w` 排布、超宽换行，行内高度取最大 `preferred_height()`，
  `spacing` 默认 10px（layui-btn-container 语义），无背景透明绘制；
  CMake 源列表接入。按钮页新增「按钮容器」小节：说明段（muted 换行 Label）+ FlowBox 内
  20 个 `layui-btn`（按钮一..按钮六 ×3 + 按钮一/按钮二，对应镜像 button.html:516）。

验证（`--shot-page 2` 像素核验）：
- 尺寸行 r3/r4 逐列测高：lg=42、默认=36、sm=28、xs=19（标称 44/38/30/22 含圆角抗锯齿
  约差 2-3px），自上而下严格递减且垂直居中；r10 删除(sm danger)=30。
- 按钮组行：g1（x 245..530，3×96 默认绿）、g2（543..700，4×40 sm 图标）、g3（713..893，
  增加+3 图标 primary 白底灰边灰字）三组同行渲染，无缺失。
- 按钮容器：20 按钮按 1002px 内容宽换行为 9+9+2 三行（行距 48px = 38+10），按钮 10px 间距。
- 回归：r1 基础按钮 / r2 边框按钮行仍为 38px 整行；HBox 默认 Stretch 未改，其余页面不受影响；
  `layui_test` 编译链接通过（VS x64-Release，无警告）。
- 回归清单：UI-017 HBox「高度填满容器」仍成立（默认 Stretch）；新增保证项：尺寸类按钮行
  必须 `set_valign(Valign::Center)` 才能呈现 layui 固有高度；按钮组须加入控件树；按钮容器
  用 FlowBox（spacing 10 换行），禁止在测试程序内自建布局绕过。

## 相关历史条目

- [UI-017](CHANGELOG.md#ui-017-m12-布局增强hbox--grid--嵌套滚动2026-08-09)：HBox 布局模型（宽度取预设、高度填满）由本条目补充 Valign 垂直居中能力，默认行为不变。
- [UI-023](CHANGELOG.md#ui-023-m17a-遗留完善--m17b-表单与按钮layui-克隆2026-08-10)：Button layui 尺寸变体（preferred_height 44/30/22）与本文不同尺寸行修复同源。
- [UI-048](CHANGELOG.md#ui-048-image-资产定位依赖-cwd--加载噪音错误2026-08-16)：按钮页资产加载约定与本页渲染无关，仅背景参考。

## UI-053: 按钮文本垂直偏移 ~4px + 圆角按钮描边矩形化（2026-08-16）

严重度：中（视觉）。按钮页两处缺陷：
（1）尺寸按钮（尤其 xs 仅 22px）文本未随按钮高度垂直居中，统一下偏约 4px；
（2）`layui-btn-radius` 圆角按钮的焦点选中环（及 primary/禁用边框）为矩形描边，不随圆角裁剪。

根因（均属 ui 库）：
- **（1）** `Font::descent()` 为负值（stb 约定，`gl/src/font.cpp:199`），而文本垂直居中公式普遍
  误写成 `(ascent + descent)`，比正确块高 `line_height() = ascent - descent` 少算 `2*|descent|`
  （≈9px），文本整体下移约 4px。该错误公式散布全库 29 处（19 个 ui 源文件：button/label/
  timeline/panelwindow/breadcrumb/card/badge/progress/checkbox/combobox/collapse/radiobutton/
  navmenu/laypage/tabs/table/textbox/switch/tree + 1 处测试程序）。UI-052 让按钮按固有高度显示后
  偏移肉眼可见（此前被拉伸到 44px 掩盖）。
- **（2）** 渲染器无圆角描边原语：`draw_rect_outline` 为 4 条直角薄 quad；button.cpp 焦点环
  （2px accent）、primary 边框、禁用边框全部走矩形描边，圆角按钮四角"冒出"矩形。

修复：
- **ui 库（1）**：全量替换 `(ascent + descent)` → `font->line_height()`（29 处；panelwindow.cpp:143
  原已正确用 `ascent - descent`，未动）。文本按 em 方框垂直居中。
- **ui 库（2）**：`gl/include/ui/gl/renderer.h` 新增
  `draw_rounded_rect_outline(rect, radius, thickness, color)`；color 片元着色器加 `uOutline`
  uniform（0=填充路径完全不变，UI-037 自检不受影响；>0 输出圆角环 alpha = 外缘 SDF × 内缘 SDF，
  半径/圆角自动跟随）。button.cpp 三处描边（焦点环/primary/禁用）改用圆角描边并复用当前半径 r。
- **ui 库（3）**：`ScrollBox` 新增公开 `set_scroll_offset()`（程序化滚动定位，脚本化截图验证
  折叠区控件用；内部走既有 `set_scroll` clamp）。
- **测试程序**：layui_test 新增 `--shot-scroll <px>`（截图前滚动内容区）。

验证（像素核验）：
- 文本居中：`--shot-page 2` 逐按钮测文本墨迹中心 vs 按钮中心，lg/默认/sm/xs（h=42/36/28/20）
  全部 ≤±1px（修复前统一 -4px）；badge 页小徽章同样 ≤±1px。
- 圆角描边：`--shot-page 2 --shot-scroll 600` 后 r11 原始按钮（primary+radius pill）与 r12
  大型圆角按钮的 #d2d2d2 圆角环角部 6x6 边框像素≈0、上/左边缘存在（随圆角裁剪，非矩形）；
  r11 圆角填充按钮四角被切掉。
- 回归：layui_test `--smoke 18` 30 页自动切页无崩溃；opengl_ui_demo / api_demo `--smoke 3` 通过；
  x64 全工程编译链接无警告。
- 回归清单：UI-037 draw_quad（radius=0）内部全不透明自检仍成立（uOutline 默认 0 不改变填充路径）；
  UI-052 尺寸行固有高度/Valign 语义不变；新增保证项：① 文本垂直居中必须用 `font->line_height()`，
  禁止回退 `(ascent+descent)`；② 圆角按钮描边必须走 `draw_rounded_rect_outline`（焦点环/边框），
  禁止回退 `draw_rect_outline`。

## 相关历史条目

- [UI-052](CHANGELOG.md#ui-052-按钮页三缺陷尺寸行无大小差--按钮组后续按钮缺失--按钮容器缺失2026-08-16)：本次偏移的可见化载体（固有高度显示）；尺寸行布局语义沿用。
- [UI-037](CHANGELOG.md#ui-037-直角矩形-50-半透明蒙版修复sdf-内部距离项2026-08-10)：uOutline 扩展必须保持填充路径不变，避免破坏其自检。

## UI-054: 圆角按钮描边内部被描边色整块盖满（"蒙版"）——圆角环内缘误当作"再叠一层"（2026-08-17）

严重度：高（渲染级回归，由 UI-053 引入）。圆角按钮（含 `layui-btn-radius`/primary 边框/禁用边框/
焦点环）内部被描边色整块覆盖成实心色块，叠在按钮文本上造成"字体与组件渲染不清晰、多了一层低透明蒙版"
的观感，直接回归 UI-052 按钮页视觉。

根因（归属 ui_gl 渲染器）：UI-053 为圆角描边新增 color 片元 `uOutline` 分支时，把内缘误当作外缘的
"再叠一层"而非"挖掉"：

```glsl
// UI-053 错误：a_out≈1 且 a_in≈1（按钮内部，d 深负）→ a=1，整个内部被描边色盖满
a = a_out * a_in;
// 正确：外缘覆盖度与内缘覆盖度相"与"要取补，内部镂空
a = a_out * (1.0 - a_in);
```

- `a_out = 1-smoothstep(-aa,aa,d)`：外缘覆盖度（d=0 为边界）。
- `a_in  = 1-smoothstep(-aa,aa,d+uOutline)`：内缘覆盖度（d=-uOutline 为内界）。
- 环 = 外缘 ∧ ¬内缘 = `a_out * (1.0 - a_in)`。错误写法背景色与其他原因叠加，使 primary 按钮整块变
  `#d2d2d2` 灰块、禁用按钮变 `#eee` 块、焦点按钮变 accent 实块；圆角胶囊（描边==底色）恰同色掩蔽，
  致 UI-053 当时像素核验（角部圆润 / 上下边缘存在）未察觉内部填充。

修复（ui_gl 渲染器）：
- `gl/src/renderer.cpp` 圆角环片元 `a = a_out * (1.0 - a_in)`，并更新注释标注"内缘是挖掉不是再叠一层"。
- 新增回归自检 `Renderer::self_test_ring_opacity()`（`init()` 执行）：离屏 32x32 FBO 绘制圆角环
  （r=8, t=2），读回中心像素，非近透明（>30）即 `log_error`——直接拦截内缘误叠回归。与既有
  `self_test_quad_opacity()` 共用离屏检查体 `run_offscreen_check()`。

验证（含回归清单）：
- 离屏自检：`self_test_ring_opacity()` 生效，无 log_error；补测离屏 32x32 白底黑环，逐像素确认圆角环
  为 ~1px 中空描边、中心纯白；四边 + 圆角追踪正确，内部镂空无填充。
- 按钮页像素：`--shot-page 2` 前后对比——修复前存在 6 处按钮尺寸实心灰块（最长 101x41，`#d2d2d2`
  连片），修复后归零；primary 按钮恢复"白底透明 + 1px 圆角灰描边 + 灰字"，禁用按钮恢复
  "`#fbfbfb` 底 + `#e6e6e6` 圆角描边"。
- 回归：layui_test `--smoke 18` 30 页切页无崩溃、自检无 FAILED；opengl_ui_demo / api_demo `--smoke 3`
  通过；x64 全工程编译链接无警告。
- 回归清单：UI-038 fill 自检、UI-052 尺寸行语义、UI-053 文本垂直居中与圆角描边接口保持不变；
  新增保证项：圆角环必须中空（外缘×¬内缘），禁止回退 `a_out*a_in`；`draw_rounded_rect_outline`
  任意 thickness/radius 下内部 alpha 必须≈0。

## 相关历史条目

- [UI-053](CHANGELOG.md#ui-053-按钮文本垂直偏移-4px--圆角按钮描边矩形化2026-08-16)：引入本回归的改动；圆角描边接口/文本居中保持不变。
- [UI-037](CHANGELOG.md#ui-037-直角矩形-50-半透明蒙版修复sdf-内部距离项2026-08-10)：同属 SDF 内部项语义；填充路径（uOutline=0）自检不受影响。

## UI-055: 文字像素对齐——draw_text/draw_icon 字形对齐像素网格（2026-08-17）

严重度：低（清晰度优化，非功能缺陷）。用户反馈"字体+组件不清晰"，排查确认颜色/自检/屏幕呈现均正确
（UI-037/UI-054 修复有效），唯文字观感发虚：侧栏 CJK 笔画峰值亮度仅 180/255、边缘最大斜率 131。

根因（归属 ui_gl 字体渲染）：`Font::draw_text` 中字形左上角 = `cursor + xoff` / `y + ascent_ + yoff`，
其中 `ascent_`（Arial 18px ≈ 13.3）与逐字 `xadvance` 累加均为小数 → 每个字形落在非整数像素坐标。
纹理管线 GL_LINEAR 双线性采样在亚像素位置跨纹素混叠，笔画被拉糊（峰值亮度下降、边缘变斜坡）。

修复（ui 库，`gl/src/font.cpp`，未触碰测试程序）：
- `draw_text`：三种字形路径（ASCII / 图标 / CJK 懒烘培）统一把字形左上角 `std::round` 到整数像素；
  `cursor` 仍按浮点 `xadvance` 累加，行内间距与 `measure_text` 一致；字形尺寸（w/h）与 UV 不变，
  内部纹素 1:1 映射，边缘仅与图集 padding（透明）混合。
- `draw_icon`：以 (cx, cy) 中心缩放后，左上角取整放置。

验证：
- layui_test Debug 重建通过；`--smoke 3` 自检无 FAILED（UI-037/UI-054 自检不受影响）。
- 截图像素量化（`--shot-page 2` 前后对比）：侧栏 CJK 峰值亮度 180.6→212.2/255、边缘最大斜率
  131→199、强单像素边缘数 142→1144（8 倍）；header 文字最大斜率 111→154；内容区 144→197；
  `measure_text` 未改，布局宽度不变。
- 回归清单：UI-004/036/046 Font 全量烘培与 CJK 懒烘培行、UI-053 文本垂直居中行均保持成立
  （取整只改绘制坐标，度量/advance/行高未动）。

## 相关历史条目

- [UI-004](CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09)：Font draw_text 自引入即用浮点坐标绘制，本次补齐像素对齐。
- [UI-036](CHANGELOG.md#ui-036-font-中文懒烘培--cjk-字体可配置接入2026-08-10)：CJK 懒烘培字形路径同步取整，未改烘培/度量。
- [UI-046](CHANGELOG.md#ui-046-font-烘培计数误用-stbttbakefontbitmap-返回值ascii图标大量字形缺失2026-08-16)：字形度量完整性保证不受取整影响。

## UI-056: 字体图集 2x 过采样——字形按 2×pixel_height 光栅化（2026-08-17）

严重度：低（清晰度优化，承接 UI-055）。像素对齐后侧栏 CJK 峰值亮度已达 212/255，但字形图集仍为
1:1 光栅化：18px 字号的笔画覆盖率只有"该纹素是黑/白"两级，亚像素位置靠双线性插值近似，观感偏"虚、
发灰"（washed 占比 4.4%）。

根因（归属 ui_gl 字体渲染）：`Font` 烘培 ASCII（`stbtt_BakeFontBitmap`）、图标（`load_icon_font`）、
CJK 懒烘培（`bake_cjk`）均以 `pixel_height`（18px）光栅化入图集，Glyph 度量与图集纹素 1:1。显示时
GL_LINEAR 在 1 显示像素 = 1 图集纹素下无过采样信息可平均，笔画覆盖率不准确。

修复（ui 库，`gl/src/font.cpp`，未触碰测试程序）：
- 新增 `kAtlasScale = 2.0`：三处烘培统一按 `pixel_height * kAtlasScale`（36px）光栅化。
- Glyph 度量（x0/y0/x1/y1/xadvance）在存储时乘 `kAtlasInvScale` 换算为显示像素；UV 仍指向 2x 图集。
  由此 `draw_text` / `draw_icon` / `measure_text` / 布局全部零改动（度量已是显示像素），UI-055 的
  像素对齐与 UI-046 度量完整性保证不受影响。
- `kDynPad` 图集 padding 同步放大 kAtlasScale（4 纹素）防线性过滤串色；懒烘培游标 `pack_y_` 起始
  按 2x 纹素推进。
- 图标图集 512×512 → 1024×1024（512 图标在 36px 下放不进 512×512，`stbtt_BakeFontBitmap` 返回负值
  即溢出）。主图集 2048×1024 容 2x CJK 约 1300 字，UI-036 懒烘培上限 256 字绰绰有余，无溢出日志。

验证：
- layui_test Debug 重建通过；`--smoke 3` 自检无 FAILED（UI-037/UI-054 自检不受影响）；无
  "icon font: atlas overflow" / "atlas overflow" 日志。
- Release 构建同步重建：`--smoke 3` 通过，`--shot-page 2` 截图与 Debug 构建**逐像素 byte-identical**，
  生产路径（用户 F5 为 Debug、发布用 Release）均已生效。
- 截图像素量化（`--shot-page 2`，P1 版 vs P2 版）：全覆盖强边缘数四区全面上升（侧栏 1144→1165、
  侧栏二 1001→1151、header 108→156、内容 4995→6110），washed 占比侧栏 4.40%→3.93%；2x 覆盖率
  更接近真实轮廓（笔画边界两侧像素分布更准确）。
- 布局回归：`measure_text` 未改（advance = 2x advance × kAtlasInvScale 恒等），文字像素计数
  n_text 基本不变（±0.3%）；实心填充块 8073 个 8×8 块 P1/P2 完全一致（无几何位移）。
- 回归清单：UI-004 Font 纹理字形、UI-036 CJK 懒烘培、UI-046 度量完整性、UI-053 文本垂直居中、
  UI-055 像素对齐行均保持成立（度量/UV/绘制坐标语义不变，仅图集纹素密度与容量变化）。

## 相关历史条目

- [UI-055](CHANGELOG.md#ui-055-文字像素对齐drawtextdrawicon-字形对齐像素网格2026-08-17)：先行使字形对齐整数像素，本条目在此基础上加密图集纹素，两者叠加生效。
- [UI-036](CHANGELOG.md#ui-036-font-中文懒烘培--cjk-字体可配置接入2026-08-10)：CJK 懒烘培容量预算按 2x 重新核对（256 字上限远低于 1300 容量）。
- [UI-046](CHANGELOG.md#ui-046-font-烘培计数误用-stbttbakefontbitmap-返回值ascii图标大量字形缺失2026-08-16)：图标/ASCII 烘培计数语义未因过采样改变。

## UI-057: 嵌套 scissor pop 不恢复上级矩形——TextBox 之后内容被裁剪（2026-08-17）

严重度：高（功能性缺陷，非清晰度问题）。表单页内容区大量控件不可见：首个 TextBox（账号）之后的
标签/边框/控件全部消失；Button 页等无 TextBox 的页面正常，故误判路径曾聚焦在布局，实际布局 dump
100% 正确（18 个子项位置全部到位），问题在绘制裁剪状态。

现象（form 页截图逐像素比对）：
- row1 文本框完整渲染（边框 + 占位文字）；row2 起「密码」标签、文本框边框消失，仅占位文字可见；
- sections 2-5 标题、combobox/checkbox/radio/switch 各行全部不渲染；row7（禁用文本框）同样仅文字可见；
- 内容区非白像素仅 336（孤立色带），修复后为 7151（y[98-898] 连续渲染）。

根因（归属 ui_gl 渲染器，`gl/src/renderer.cpp`）：`Renderer::pop_scissor()` 只递减 `scissor_depth_`，
**不恢复上一级 scissor 矩形**——嵌套 push/pop（ScrollBox→TextBox）后 GL scissor 停留在内层矩形，
此后所有绘制被错误裁剪到内层裁剪区，直到整个 ScrollBox 的 pop 才关闭测试。header 文档契约明确
「可嵌套，pop 恢复上一级裁剪区」（UI-008 引入），实现违背契约；TextBox（UI-008）、TextArea、Code、
ScrollBox 均因此受影响，仅"首个子控件即触发"的页面（含 TextBox 的 form 页）暴露。

修复（ui 库，`gl/src/renderer.cpp` + `gl/include/ui/gl/renderer.h`，未触碰测试程序）：
- `scissor_depth_`（int）替换为 `scissor_stack_`（`std::vector<Rect>`）；
- `push_scissor`：新矩形与栈顶**求交**后压栈（嵌套子裁剪必须落在父裁剪区内），空交集压空矩形
  （绘制全部被剔除），栈空时 `glEnable(GL_SCISSOR_TEST)`；
- `pop_scissor`：出栈后**恢复新栈顶矩形**（`glScissor` 重设并翻转 y），栈空才 `glDisable`。

验证：
- form 页（`--shot-page 3`）修复前后对比：内容区 y[98-898] 连续渲染；色块图与布局 dump 逐一吻合——
  section 标签、文本框边框/文字、combobox、双 checkbox、radio、绿色 switch 旋钮、禁用行全部就位；
  `--shot-scroll 260` 版本同样连续渲染（滚动 clamp 124 不变）。
- 全部 push/pop 调用点（code/scrollbox/textarea/textbox）配对完整且裁剪区均在自身 bounds 内，
  无任何依赖"扩大裁剪"的调用，求交语义不破坏现有页面（Button 页复拍无差异）。
- `--smoke` 自检无 FAILED（UI-037/UI-054 自检为独立 FBO 路径，不受影响）。

回归清单：UI-008 scissor 嵌套裁剪（pop 恢复上级矩形）、UI-013 ScrollBox 可视区裁剪（scrollbar 在
scissor 弹出后绘制）均保持成立；push_scissor/pop_scissor 接口签名不变；新增保证项：任何时刻
GL scissor 矩形必须等于当前裁剪栈顶（或未启用），嵌套 push 必须与父裁剪区求交。

## 相关历史条目

- [UI-008](CHANGELOG.md#ui-008-m4-文本输入textbox--字符通道--scissor-裁剪2026-08-09)：引入 push_scissor/pop_scissor 嵌套裁剪，本次修正 pop 未恢复上级矩形（原实现违背文档契约）。
- [UI-013](CHANGELOG.md#ui-013-m8-滚动容器scrollbox2026-08-09)：ScrollBox 先 push_scissor(bounds_) 再画子控件，嵌套裁剪的父级来源。
- [UI-037](CHANGELOG.md#ui-037-直角矩形-50-半透明蒙版修复sdf-内部距离项2026-08-10)：同属渲染正确性修复，自检路径独立不受影响。

## UI-058: 表单页一比一复刻 layui form.html 预览内容（2026-08-17）

严重度：中（测试程序功能增强）。原 form_page.cpp 仅含 TextBox/ComboBox/CheckBox/RadioGroup/Switch
+ 禁用状态 6 组控件，与 layui 表单页（demo/form.html 预览 tab）差距大——缺少区块引用、字段集分组、
内联对齐、范围输入、级联下拉、文本域、按钮行、赋值/取值表单、方框风格表单等。

改动（测试程序 layui_test/src/pages/form_page.cpp，使用 ui 库已有组件，未触碰 ui 库）：
- 顶部 + 中部 + 底部添加 `BlockQuote`（绿色左侧条 + #fafafa 背景），复刻 layui-elem-quote；
- 三大区块分别用 `Fieldset`（顶部+底部 1px 边框 + legend 标签）包裹，复刻 layui-elem-field；
- **表单验证示例**区块包含 17 项：
  1. 标题验证 / 2. 验证用户名（full-width TextBox）；
  3. 验证手机 + 验证邮箱（inline pair，各190px）；
  4. 验证数字 + 验证日期 + 验证网址（inline triple，各130px）；
  5. 验证身份证 / 6. 自定义验证（password + "请填写6到12位" hint 文字）；
  7. 范围（￥ - ￥ 两输入 + dash 分隔符）；
  8. 普通选择（full-width ComboBox）；
  9. 搜索选择 + 普通选择（inline pair ComboBox）；
  10. 级联选择（省/市/区三 ComboBox + hint）；
  11. 复选框（写作/阅读/游戏） / 12. 原始复选框（含 disabled）；
  13-14. 开关-默认关/开 / 15. 单选框（男/女/未知）；
  16. 普通文本框（TextArea） / 17. 立即提交 + 重置（Button layui-btn / layui-btn-primary）。
- **赋值/取值**区块：账号/密码/选择/复选/开关/单选/文本框 + 取值/赋值/提交三个按钮。
- **方框风格表单**区块：简化版表单 + 提交按钮。
- 代码 tab 和帮助 tab 同步更新 API 说明。

验证：
- smoke 5 秒 30 页切页无崩溃；各表单控件可交互（TextBox 输入、ComboBox 下拉、CheckBox/Radio 切换、
  Switch 开关、TextArea 多行）。
- 截图（scroll 0/260/500/800）：内容区连续渲染（非白像素 5482/5322/5944/4969），三段 Fieldset
  均可见，绿色 switch 旋钮、checkbox 圆角框、组合下拉框、按钮组布局完整。
- 根因归属：测试程序（form_page.cpp）页面内容重构，非 ui 库缺陷；ui 库全部已有组件无改动。

## 相关历史条目

- [UI-057](CHANGELOG.md#ui-057-嵌套-scissor-pop-不恢复上级矩形textbox-之后内容被裁剪2026-08-17)：scissor 嵌套裁剪修复后，form 页所有控件可见，本次替换内容结构。
- [UI-014](CHANGELOG.md#ui-014-m9-复合控件checkbox--radiogroup--combobox2026-08-09)：CheckBox/RadioGroup/ComboBox 基础组件，本次在 layui form 布局中全面使用。
- [UI-016](CHANGELOG.md#ui-016-m11-多行文本排版label-换行2026-08-09)：Label 多行 + 自动换行，hint 文字复用。

## UI-059: Breadcrumb set_separator() API + 面包屑页克隆 layui nav.html 三个 demo（2026-08-17）

严重度：低（ui 库功能增强 + 测试程序更新）。原 Breadcrumb 组件硬编码分隔符 `" / "`，
不支持 layui `lay-separator` 属性对应的自定义分隔符（如 `"—"`, `"|"`）。

改动（ui 库）：
- `breadcrumb.h`：新增 `set_separator(const char* sep)` 方法 + `separator_` 成员（默认 `" / "`）。
- `breadcrumb.cpp`：`on_draw` 中用 `separator_` 替代硬编码 `" / "`。

改动（测试程序 `breadcrumb_page.cpp`）：
- "默认面包屑"：`首页 / 演示 / 导航元素`（默认分隔符，最后一项 cite）。
- "自定义分隔符的面包屑"：`set_separator(" — ")`，`首页 — 国际新闻 — 亚太地区 — 正文`。
- "还可以用于门户频道的面包屑"：`set_separator(" | ")`，`娱乐 | 八卦 | 体育 | 搞笑 | 视频 | 游戏 | 综艺`（全部为链接项，无 cite）。
- code tab / help tab 同步更新 `set_separator` API 说明。

验证：
- x64 Release 构建链接通过，无警告无报错。
- 截图 `--shot-page 4`：三个面包屑均渲染完整，分隔符正确（`/`、`—`、`|`）。
  颜色层次：链接(#999) > 分隔符(#999) > cite(#5f5f5f)，反走样下相对亮度关系正确。
- 根因归属：ui 库缺陷（Breadcrumb 缺少 set_separator API），已修正 ui 库。

## 相关历史条目

- [UI-030](CHANGELOG.md#ui-030-六页基本元素克隆面包屑选项卡进度条面板时间线评分2026-08-10)：六页克隆中面包屑首版仅支持默认分隔符，本次补齐自定义分隔符。
- [UI-022](CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10)：Breadcrumb 组件本体来自 M17a。

## UI-060: NavBar 横向导航组件 + 导航/面包屑页完整克隆 layui nav.html（2026-08-17）

严重度：中（ui 库新增组件 + 测试程序扩展）。原导航/面包屑页仅克隆面包屑 3 个 demo，
缺少 layui nav.html 预览区的水平导航、徽章导航、彩色主题导航、垂直导航 4 个 demo。

改动（ui 库新增）：
- `navbar.h` / `navbar.cpp`：新建 `NavBar` 横向导航组件（layui-nav 克隆）。
  API：`add_item(text, cb)` 顶级项、`add_child(text, cb)` 下拉子项、`set_active(n)` 选中态（layui-this）、
  `set_badge(text)` / `set_dot_badge()` 徽章、`set_image(path)` 头像、`set_bg_color({r,g,b,a})` 彩色主题、
  `set_bar_disabled(bool)` 禁用滑块跟随。
  渲染：白底 panel + 选中项 #16b777 文字 + hover 浅灰底 + 下拉面板白底描边。

改动（测试程序 `breadcrumb_page.cpp`）：
克隆 layui `nav.html` 预览区全部 8 个 demo：
1. **水平导航菜单**：NavBar 5 项（选中/常规/导航/子级[下拉3项]/选项[下拉3项]/演示）。
2. **导航带徽章和图片**：NavBar + badge "9" + dot badge + 头像下拉 + `set_bar_disabled(true)`。
3. **更多导航主题**：3 个 NavBar（cyan/green/blue 背景色）+ 子菜单下拉。
4. **垂直导航菜单**：NavMenu 3 组（默认展开/解决方案/云市场/社区）+ expand_all。
5. **侧边固定导航菜单**：BlockQuote 说明文字。
6-8. **面包屑**：默认 / 自定义分隔符（—）/ 门户频道（|），沿用 UI-059。
code tab / help tab 同步更新 NavBar + NavMenu + Breadcrumb API 说明。

验证：
- x64 Release 构建链接通过，无警告无报错。
- 截图 `--shot-page 4`：水平 NavBar 项渲染（白底不可见但文字可见）、3 个彩色 NavBar 背景色正确
  （cyan(0,184,214) / green(64,140,89) / blue(51,127,217)）、NavMenu 深灰底垂直排列、3 个面包屑完整。
- 滚动截图 `--shot-scroll 500`：NavMenu 384px 高度区域 + 下方面包屑均可见。
- 根因归属：ui 库功能缺失（无横向导航组件），已新增 NavBar 组件。

## 相关历史条目

- [UI-059](CHANGELOG.md#ui-059-breadcrumb-setseparator-api--面包屑页克隆-layui-navhtml-三个-demo2026-08-17)：面包屑 set_separator 同属 nav.html 克隆，本次补齐导航菜单部分。
- [UI-031](CHANGELOG.md#ui-031-六页组件克隆滑块分页下拉菜单树形组件基础菜单静态表格2026-08-10)：基础菜单页（MenuBar）与本次 NavBar 语义不同——MenuBar 为按钮触发器风格，NavBar 为扁平导航条。
- [UI-022](CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10)：NavMenu 垂直导航本体来自 M17a。

## UI-061: Menu 基础垂直菜单组件 + menu.html 克隆（2026-08-17）

新增 `Menu` 垂直菜单组件（layui-menu 克隆），基础菜单页改用 Menu 克隆 layui menu.html 全部 demo。

严重度：中（ui 库新增组件 + 测试程序扩展）。原基础菜单页仅展示 MenuBar 横向菜单栏，缺少
layui menu.html 预览区的垂直菜单、分组折叠、父级子面板、徽章、选中态 demo。

改动（ui 库新增）：
- `menu.h` / `menu.cpp`：新建 `Menu` 垂直菜单组件。
  API：`add_item(text, cb)` 普通项、`add_divider()` 分割线、`add_group(title, allow_spread)` 可折叠分组、
  `add_parent(title)` 父级项（悬停右侧子面板）、`end_group()` 结束分组层级、
  `set_checked()` 选中态（绿色）、`set_badge(text)` / `set_dot_badge()` 徽章、
  `set_large(bool)` 大尺寸、`set_font(Font*)` / `set_theme(const Theme*)`。
  渲染：白底 panel + 四边描边 + 分组折叠（compute_visibility 控制子项隐藏）+
  父级悬停子面板（draw_overlay 浮层）+ 箭头三角形（draw_polygon）。
- `ui/CMakeLists.txt`：新增 `src/menu.cpp`。

改动（测试程序 `menubar_page.cpp`）：
克隆 layui `menu.html` 预览区两个垂直菜单面板：
1. **左侧菜单（demo1）**：10 个顶级项 + 2 个可折叠分组 + 嵌套子分组 + 父级子面板 + 徽章 + 选中态。
2. **右侧菜单（docDemoMenu1）**：6 个顶级项 + 2 个不可折叠分组 + 父级子面板 + 徽章。
code tab / help tab 同步更新 Menu API 说明。

验证：
- x64 Release 构建链接通过，无警告无报错。
- 截图 `--page 5 --shot`：左侧菜单分组箭头正确（展开/折叠状态）、折叠分组子项隐藏、
  选中态绿色文字、红色徽章 "1"、右侧菜单分组和父级项渲染正确。
- 根因归属：ui 库功能缺失（无垂直菜单组件），已新增 Menu 组件。

## 相关历史条目

- [UI-060](CHANGELOG.md#ui-060-navbar-横向导航组件--导航面包屑页完整克隆-layui-navhtml2026-08-17)：NavBar 横向导航与 Menu 垂直菜单互补，同属 nav.html / menu.html 克隆。
- [UI-031](CHANGELOG.md#ui-031-六页组件克隆滑块分页下拉菜单树形组件基础菜单静态表格2026-08-10)：原基础菜单页使用 MenuBar（按钮触发器风格），本次改用 Menu（垂直 layui-menu 克隆）。

## UI-062: Tabs 组件增强（内容区/动态操作/删除/超宽响应式）+ 选项卡页一比一复刻 demo/tab.html（2026-08-17）

克隆 layui 官方 `demo/tab.html` 预览区全部 demo。原 `Tabs` 组件仅支持 `add_tab/set_active/on_change`，
缺内容区、动态增删、删除按钮、超宽响应式，无法承载镜像 8 个 demo。

严重度：中（ui 库 Tabs 组件能力缺陷 + 测试程序复刻）。根因归属：ui 库能力缺失，先修正 ui 库，
测试程序只做编排复刻。

改动（ui 库 `tabs.h` / `tabs.cpp`增强）：
- **内容区**：`add_tab(title, content)` 追加页时可带内容文本；`set_content(index, text)` 设置
  指定页内容；切换时内容区联动显示当前激活页 content（内部单个 Label 子控件，支持多行换行）。
- **动态操作**：`count()` 页数、`remove_tab(index)` 删除指定页（lay-id 语义，外部定位凭据，
  删除激活页自动落到邻近页并触发内容联动）。
- **删除按钮**：`set_allow_close(bool)`（lay-allowclose）后每页标题右缘绘制 ×，点击删除该页。
- **超宽响应式**：标题总宽超出容器可用宽时，右侧绘制展开三角（draw_polygon），点击弹出根级
  ScrollBox 下拉面板列出全部页，点击切换（复用 ComboBox 面板机制，析构摘除防悬空引用）。

改动（测试程序 `layui_test/src/pages/tabs_page.cpp`重写）
一比一复刻 demo/tab.html 预览区 7 个 demo + 结尾引用：
1. 默认风格的 Tab（`layui-tab`，5 页 + 内容区文本）
2. 动态操作 Tab（`lay-allowclose`，3 个 layui-btn-primary 按钮：新增Tab项 / 删除:商品管理 / 切换到:用户管理）
3. Hash 地址定位（5 页，说明外部 lay-id 定位）
4. 简洁风格的 Tab（`layui-tab-brief`，内容 100px）
5. 卡片风格的 Tab（`layui-tab-card`，内容 100px，6 页）
6. 当 Tab 数超过一定宽度（固定 290px + 超宽展开三角）
7. 带删除功能的 Tab（card + allow_close，内容 150px，标题右缘 ×）
8. 结尾 BlockQuote 引用。

验证：
- x64 Release（layui_test 独立工程）构建链接通过，无警告无报错。
- `--smoke 12` 逐页切换 exit 0（30 页无崩溃）。
- `--shot-page 14 --shot-scroll {0,300,520,900}` 截图 + PAM 像素采样：内容区文本多行渲染、
  卡片式 #eee 外框、默认 tab 深色标题文字均正确，无缺失。
- 根因归属：ui 库 Tabs 能力缺陷，已增强 ui 库（内容区/动态/删除/超宽），测试程序仅编排复刻。

## 相关历史条目

- [UI-030](CHANGELOG.md#ui-030-六页基本元素克隆面包屑选项卡进度条面板时间线评分2026-08-10)：选项卡页首版仅 3 个风格 demo，本次按镜像补齐全部 7 个 demo。
- [UI-023](CHANGELOG.md#ui-023-m17a-遗留完善--m17b-表单与按钮layui-克隆2026-08-10)：Tabs 组件本体来自 M17b。
- [UI-048](CHANGELOG.md#ui-048-image-资产定位依赖-cwd--加载噪音错误2026-08-16)：截图像素验证沿用 PAM 读回手法。

## UI-063: Tabs 析构二次释放崩溃修复（content_label_ 子节点重复 delete）（2026-08-17）

UI-062 新增 Tabs 内容区后，切页（`show_page` → `preview.clear_children()`）触发 `Tabs::~Tabs()`
崩溃：0xC0000005 访问冲突（读 0xFFFFFFFFFFFFFFFF，MSVC 已释放堆标记）。

严重度：高（ui 库崩溃，页面切换即现）。根因归属：ui 库 `Tabs` 析构对子节点二次释放。

根因：
- `content_label_` 在 Tabs 构造函数 `add_child(content_label_)` 作为**子节点**加入控件树。
- 切页时 `clear_children()` 先递归释放全部子节点（含 `content_label_`），再 `delete` Tabs。
- 原 Tabs 析构又执行 `remove_child(content_label_); delete content_label_;` → 对已释放对象二次
  delete，堆崩溃。

修复（`ui/src/tabs.cpp` 析构）：
- `content_label_` 是 Tabs 子节点，所有权归 `clear_children()` 统一递归释放，析构**不得再删**
  ——移除 `remove_child(content_label_); delete content_label_;` 两行。
- `overflow_panel_` 不是子节点（展开时挂 root），析构摘除父引用 + delete 的逻辑保留
  （同 ComboBox，防 root 悬空引用）。

防复发（回归保证）：任何**作为子节点（add_child）由 clear_children 统一释放**的成员控件，
析构中**禁止再次 delete**；析构只需手动释放「非子节点 / 挂根级弹出的外部面板」。

验证：
- x64 Release（layui_test 独立工程）构建链接通过，无警告无报错。
- `--smoke 20` 逐页切换 exit 0；连跑 3 轮 `--smoke 15` 全部 exit 0（反复切页触发 Tabs 析构无崩溃）。
- Tabs 页截图 + 像素采样：内容区多行文本正常渲染，无回归。

## 相关历史条目

- [UI-062](CHANGELOG.md#ui-062-tabs-组件增强内容区动态操作删除超宽响应式-选项卡页一比一复刻-demotabhtml2026-08-17)：引入内容区 `content_label_` 子节点，本次修复其析构二次释放。
- [UI-039](CHANGELOG.md#ui-039-轻量组件下沉-ui-库--grid-layui-row-无背景--clearchildren-统一切页释放2026-08-11)：clear_children 统一切页释放，子节点所有权约定来源。
- [UI-040](CHANGELOG.md#ui-040-table-行重建泄漏修复--widgetdestroy-单控件销毁-api2026-08-11)：控件生命周期 / 析构约束相关。

## UI-064: Progress 组件增强（layui-bg-* 颜色类/自定义文本/默认填充 #5fb878）+ 进度条页一比一复刻 demo/progress.html（2026-08-17）

克隆 layui 官方 `demo/progress.html` 预览区全部 5 组 demo。原 `Progress` 组件仅支持单一 `success`
填充色（#16b777）与固定百分比文本，无法承载镜像的颜色类（red/orange/green/blue/cyan）、分数文本
（"5 / 10"）、以及默认轨道填充 `#5FB878`。

严重度：中（ui 库 Progress 能力缺陷 + 测试程序复刻）。根因归属：ui 库能力缺失，先修正 ui 库，
测试程序只做编排复刻。

改动（ui 库 `progress.h` / `progress.cpp`增强）：
- **颜色类**：`on_draw` 遍历控件 `layui-bg-*` 类，用现有 `layui::bg_fill()` 取填充色（与 Badge 同源，
  复用工具层，无新增色值）。无颜色类时默认填充 `#5fb878`（layui-progress-bar 默认，原 #16b777 不符镜像）。
- **自定义文本**：新增 `set_text_override(const char*)` / `text_override()`——显示任意文本（如分数
  `"5 / 10"`）；为空时回退到百分比 `"40%"`。普通轨道文本居右上（text_secondary）、big 内嵌白字居中。
- 大号（`layui-progress-big`，高 18px）、`set_show_text(bool)` 语义不变，回归保持。

页面 `pages/progress_page.cpp`（一比一复刻镜像 5 组）：
1. 默认风格的进度条：全宽 40% + 300px 容器 70%（宽度适配父级）。
2. 更多颜色选取：red 20% / orange 30% / green 40% / blue 50% / cyan 60%。
3. 大尺寸进度条（big）：20% / green 35% / cyan 75%。
4. 显示进度比文本：20% / 分数 "5 / 10"（value 0.5 + text_override）/ big 70%。
5. 动态改变进度：big red 0% + "设置 50%"按钮（set_value 0.5）+ "模拟 loading"按钮（点击推进 35% 至 100%）。

验证：
- x64 Release（layui_test 独立工程）构建链接通过，无警告无报错。
- `--smoke 12` 逐页切换 exit 0（30 页无崩溃）。
- `--shot-page 7` 截图 + PAM 读回 + 逐进度条 on_draw 打印：13 条进度条 fill 颜色
  （red 1.0/0.34/0.13、orange 1.0/0.72/0、green 0/0.588/0.533、blue 0.12/0.62/1.0、cyan 0.18/0.25/0.34、
  default 0.373/0.722/0.471）与 bounds（宽 1002=容器宽，高 6/18，value 对应）全部正确，无缺失。
- 动态 0% 进度条 value=0 不绘制填充（符合 `value_>0` 分支），正确。
- 根因归属：ui 库 Progress 能力缺陷，已增强 ui 库（颜色类/自定义文本/默认色），测试程序仅编排复刻。

## 相关历史条目

- [UI-030](CHANGELOG.md#ui-030-六页基本元素克隆面包屑选项卡进度条面板时间线评分2026-08-10)：进度条页首版仅默认/big/无文本+按钮推进，本次按镜像补齐颜色/分数/默认填充色。
- [UI-022](CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10)：Progress 组件本体来自 M17a。
- [UI-048](CHANGELOG.md#ui-048-image-资产定位依赖-cwd--加载噪音错误2026-08-16)：截图像素验证沿用 PAM 读回手法。

## UI-065: 面板页一比一克隆 demo/panel.html 全部 5 组 demo + Collapse 手风琴/非手风琴双模式（2026-08-17）

克隆 layui 官方 `demo/panel.html` 预览区全部 5 组 demo。镜像预览区含：普通面板（layui-panel）、
卡片面板（layui-card）、常规折叠面板（layui-collapse 非手风琴可多开）、手风琴折叠（lay-accordion
单开）、面板嵌套（手风琴无限嵌套）。

严重度：中。根因归属：ui 库 Collapse 能力缺陷——原实现仅手风琴（同一时刻仅一个展开，`active_`
唯一），而镜像「常规折叠面板」为 layui-collapse 默认可**多开**。先修正 ui 库，测试程序只做编排复刻。

改动（ui 库）：
- **Collapse 双模式**（`collapse.h/cpp`）：新增 `set_accordion(bool)`（默认 true 手风琴，向后兼容），
  内部展开集改为 `std::vector<bool> open_`。手风琴：set_active 点开已展开项则收起、否则只开该项
  （唯一）；非手风琴：set_active 仅 toggle 该项，可多开。新增 `is_open(index)` / `open_count()` /
  `accordion()`，`active()` 语义调整（手风琴返回当前项、非手风琴返回最后操作项）。
- **HBox / Panel 背景覆盖**：新增 `set_fill(Vec4)`（默认 theme->button_normal，传 a<0 不覆盖），
  用于 layui-bg-gray（#fafafa）灰底两列容器。

页面 `pages/card_page.cpp`（一比一复刻镜像 5 组，替换原三张卡片版）：
1. 普通面板：灰底两列容器（HBox set_fill #fafafa padding/spacing 15）+ 2×Card 无 header（白底+阴影）
   内 "一个面板" 大留白。
2. 卡片面板：灰底两列 + 2×Card（set_header "卡片面板" + body 两行文本）。
3. 常规折叠面板：Collapse set_accordion(false)，4 项（JS/JS特性/Bootstrap/layui适用人群/折叠面板）。
4. 手风琴折叠：Collapse set_accordion(true) + set_active(0)，4 项首项默认展开。
5. 面板嵌套：手风琴，文豪→（朝代→诗人：唐代 杜甫/李白/王勃 展开、宋代、当代）内嵌 / 科学家 / 艺术家。

验证：
- x64 Release（layui_test 独立工程）构建链接通过，无警告无报错。
- `--smoke 14` 逐页切换 exit 0（30 页无崩溃）。
- `--shot-page 8`（面板页=idx8，进度条页 idx7 已验证）截图 + PAM 读回 + 行向 R 通道分布：
  普通面板/卡片面板灰底容器（#fafafa≈252）两列、卡片白底（#fff）在灰底上衬出、折叠标题条/文本、
  嵌套层级全部正常渲染，内容高度正确（滚动 1500px 后仍显示折叠项）。
- 根因归属：ui 库 Collapse 能力缺陷（缺非手风琴多开），已增强 ui 库；测试程序仅编排复刻。

## 相关历史条目

- [UI-030](CHANGELOG.md#ui-030-六页基本元素克隆面包屑选项卡进度条面板时间线评分2026-08-10)：面板页首版仅三张卡片，本次按镜像补齐普通/卡片面板双列 + 折叠三组。
- [UI-022](CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10)：Collapse 组件本体来自 M17b（仅手风琴）；本次扩展非手风琴多开模式。
- [UI-039](CHANGELOG.md#ui-039-轻量组件下沉-ui-库--grid-layui-row-无背景--clearchildren-统一切页释放2026-08-11)：Collapse 依赖的 children_ 树/布局/命中机制由基础控件提供。

## UI-066: 徽章页一比一克隆 demo/badge.html 两组 demo + Button 内嵌徽章/Tabs 标题徽章 + Badge 圆点与 NavBar 徽章修正（2026-08-17）

克隆 layui 官方 `demo/badge.html` 预览区全部两组 demo（区块1「小徽章、大家族」灰底三行 + 区块2「与其它元素的搭配」）。镜像要求徽章可内嵌于按钮/导航/选项卡标题，为此增强 ui 库并修正既有徽章缺陷。

严重度：中。根因归属（两处均为 ui 库能力缺陷，测试程序只做编排复刻）：
1. **按钮内嵌徽章缺失**：镜像区块2「查看消息+灰badge 1 / 动态+橙dot」为 `<button>…<span layui-badge>…</span></button>` 内嵌，Button 不支持 → 缺失。
2. **NavBar 徽章色错误**：原实现用 `t->success`（绿）画导航徽章/圆点，镜像 layui-badge 默认为红 #ff5722。
3. **Badge 圆点画成胶囊**：`layui-badge-dot` 分支用 `bounds_` 整矩形（宽8 高18）画圆 → 圆角矩形而非 8px 正圆。
4. **页布局**：徽章行 HBox 高 28 而 padding 15×2=30，内容高为负 → 区块1 徽章全被裁剪（测试程序布局缺陷）。

改动（ui 库）：
- **Button 内嵌徽章**（`button.h/cpp`）：新增 `set_badge(text)` / `set_dot_badge()`（作用于该按钮文本右缘），叠加 `layui-bg-*` 换底（默认 `t->danger` 红、dot 8px 正圆）；徽章色取控件上 `layui-bg-*` 类（`bg_class_name` + `layui::bg_fill`）。镜像 `查看消息` 加 `layui-bg-gray` 灰徽章、`动态` 加 `layui-bg-orange` 橙点（按钮本体底色不受 `layui-bg-*` 影响）。
- **Badge 圆点正圆**（`badge.cpp`）：`layui-badge-dot` 固定画边长 8 的正圆（水平居中于 bounds，垂直居中），不再随绑定的高画胶囊。
- **NavBar 徽章改红**（`navbar.cpp`）：徽章/圆点/下拉提示由 `t->success`（绿）改为 `t->danger`（红 #ff5722）；圆点由 6px 方块改 8px 正圆，符合 layui-badge 默认。
- **Tabs 标题徽章**（`tabs.h/cpp`）：新增 `set_badge(const char*)` / `set_dot_badge()`（作用于最后 `add_tab` 标题右缘），`Tab` 增 `has_badge/badge_text/has_dot`；新增 `title_width()` 把徽章/圆点计入标题宽（超宽收纳同步）；`on_draw` 标题右缘画红底徽章/白字（card 激活态白条覆盖后重绘徽章）。

页面 `pages/badge_page.cpp`（一比一复刻镜像两组，替换原分散布局版）：
1. 区块1「小徽章、大家族」：灰底（#fafafa HBox）三行（行高 48 保证内容高 ≥18）—— 小圆点 7 个（layui-badge-dot 红/橙/绿/青/蓝/黑/灰）、常规弧形徽章 10 个（红 6/99/61728/赤 + 橙绿青蓝黑灰）、边框徽章 2 个（layui-badge-rim 6/Hot）。
2. 区块2「与其它元素的搭配」：按钮 查看消息+灰badge"1"、按钮 动态+橙dot（Button set_badge/set_dot_badge 内嵌）；导航控制台+9、个人中心+dot（NavBar set_badge/set_dot_badge）；选项卡 网站设置/用户管理+dot/最新邮件+99+（Tabs set_badge/set_dot_badge）。

验证：
- x64 Release（layui_test 独立工程）构建链接通过，无警告无报错。
- `--smoke 14` 逐页切换 exit 0（30 页无崩溃）。
- `--shot-page 9`（徽章页=idx9，order17；已知正确通道序为存储 [G,R,B] + 垂直翻转）读回：红徽章多行分布（弧形4组+边框+导航+选项卡）、橙圆点（动态按钮）多行、底部 rim 徽章齐全，区块1 三行不再被裁剪。
- 根因归属：Button 缺内嵌徽章、NavBar 徽章色错、Badge 圆点非正圆、Tabs 缺标题徽章，均为 ui 库能力缺陷已修正；区块1 行高为页布局问题已修。测试程序仅编排复刻，未绕过/掩盖。

## 相关历史条目

- [UI-030](CHANGELOG.md#ui-030-六页基本元素克隆面包屑选项卡进度条面板时间线评分2026-08-10)：徽章页首版布局分散（基础/颜色/圆点/描边/按钮结合），本次按镜像改为「大家族+搭配」两组区块。
- [UI-062](CHANGELOG.md#ui-062-tabs-组件增强内容区动态操作删除超宽响应式-选项卡页一比一复刻-demotabhtml2026-08-17)：Tabs 此前已增强内容区/动态增删/超宽响应式，本次补标题内嵌徽章（对齐 NavBar `set_badge/set_dot_badge` API）。
- [UI-048](CHANGELOG.md#ui-048-image-资产定位依赖-cwd--加载噪音错误2026-08-16)：NavBar 依赖的图片加载走 exe 目录回退，与本页无关。
- [UI-039](CHANGELOG.md#ui-039-轻量组件下沉-ui-库--grid-layui-row-无背景--clearchildren-统一切页释放2026-08-11)：Badge/Button/NavBar/Tabs 均基于基础控件 children_ 树/布局/命中机制。

## UI-067: 时间线页一比一克隆 demo/timeline.html 两组 demo + Timeline 富内容/轴点图标/悬停反馈/旋转动画（2026-08-17）

克隆 layui 官方 `demo/timeline.html` 预览区全部两组 demo（常规时间线 + 简约时间线：大事记）。镜像要求轴点为 20px 白圆内图标（可用 circle/fire/loading-1 与 hover 变红、loading 循环旋转），内容含多行/列表/强调，为此增强 ui 库 Timeline 并下沉渲染层旋转能力。

严重度：中。根因归属（均属 ui 库能力缺陷，测试程序只做编排复刻）：
1. **轴点图标缺失**：原 Timeline 轴点画白圆 + 绿点，镜像为 20px 白圆内图标（默认 circle 0xE63F；简约时间线首条 fire 0xE756）。且未支持图标旋转（末条 layui-anim-rotate layui-anim-loop）。
2. **内容单一**：原 `add_item(title, content)` 仅单行文本，镜像常规时间线 item 含多行（br 换行）/ul 列表（《登高》等）/ 行尾图标（face-smile-b）。
3. **无悬停反馈**：镜像 `.layui-timeline-axis:hover` 轴点 icon 变红 #FF5722，原无鼠标交互。
4. **渲染器不支持旋转**：`draw_textured_quad`/`draw_icon` 仅画轴对齐矩形，无法表达 loading 循环旋转。

改动（ui 库，含 gl 渲染层）：
- **gl 渲染层（沉淀）**：`Renderer::draw_textured_quad` 增 `rotation` 角参（弧度，绕矩形中心旋转，0 兼容既有绘制）；共享顶点着色器 `kVertexShader` 增 `uniform float uRotation`，投影前先平移中心→旋转→平移回。纹理管线新增 `t_rotation_` uniform。颜色/多边形管线未引用 `uRotation`（默认 0，不影响 UI-037/054 自检）。
- **Font::draw_icon**（gl）：增 `rotation` 角参（默认 0），传给纹理四边形。
- **Timeline**（ui）：
  - `add_item_rich(title, title_large, lines)`：富内容条目，标题可 h3（18px #333）或普通；`ContentLine{kind,text,codepoint}` 支持 Text/Bullet（断言列表）/Em/Icon，`preferred_height` 按行数计算。
  - `set_axis_icon(codepoint)`：最后条目的轴点图标（默认 circle）。
  - `set_axis_rotate(deg/s)`：最后条目轴点图标绕中心旋转（>0 开启，steady_clock 驱动，与 countdown 同模式）。
  - `on_mouse_move`：命中 20px 轴点圆（半径容忍 12）记录 `hovered_index_`；`on_draw` 该轴点 icon 用 `t->danger`（红）并画圆角描边（镜像 :hover 变红）。

页面 `pages/timeline_page.cpp`（一比一复刻镜像两组，替换原自定义 demo）：
1. 常规时间线：4 条（8月18日/8月16日/8月15日/过去），标题 h3 大字 18px；内容多行（br 段落）/ 末段行尾 face-smile-b 图标 / 含 ul 列表《登高》《茅屋为秋风所破歌》。
2. 简约时间线：大事记（5 条仅标题）：首条 2018 轴点 icon=fire（镜像 0xE756）、中间 circle、末条 更久前 icon=loading_1 + `set_axis_rotate(360)` 旋转动画。尾部追加镜像引用块提示语。

验证：
- x64 Release（layui_test 独立工程）构建链接通过，无警告无报错。
- `--smoke 8` 逐页切换 exit 0（30 页无崩溃）；Renderer init 自检（UI-037 quad 全不透明 / UI-054 圆角环镂空）未告警，证明旋转 shader 改动未破坏既有绘制。
- 目视：常规时间线绘制正确；简约时间线首条轴点为火苗、末条为旋转的 loading 图标、其余 circle；轴点 hover 变红并描边。
- 根因归属：轴点图标/富内容/悬停/旋转均属 ui 库缺陷已修正（渲染层旋转能力下沉 gl）；页面仅编排复刻，未绕过/掩盖。

## 相关历史条目

- [UI-030](CHANGELOG.md#ui-030-六页基本元素克隆面包屑选项卡进度条面板时间线评分2026-08-10)：时间线页首版为自定义「基本/仅标题」demo，本次按镜像改为「常规+简约」两组区块。
- [UI-022](CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10)：Timeline 本体来自 M17a，本次扩展富内容/轴点图标/悬停/旋转。
- [UI-037](CHANGELOG.md#ui-037-直角矩形-50-半透明蒙版修复sdf-内部距离项2026-08-10)：旋转 shader 改动保持 draw_quad 全不透明，Renderer init 自检未回归。
- [UI-055](CHANGELOG.md#ui-055-文字像素对齐drawtextdrawicon-字形对齐像素网格2026-08-17)：draw_icon 左上角对齐像素网格逻辑保留，旋转仅绕中心附加，不改变对齐。
- [UI-053](CHANGELOG.md#ui-053-按钮文本垂直偏移-4px--圆角按钮描边矩形化2026-08-16)：旋转仅绕轴点中心附加，不与按钮文字/描边改动冲突。
- 旋转沿用 CountdownText（ui 库现有控件）的 steady_clock 实时取时模式（on_draw 内取时，无需外部每帧 tick）。

## UI-068: 静态表格页一比一克隆 demo/table-element.html 全部五组 demo + Table 皮肤/尺寸/偶数行着色（2026-08-17）

克隆 layui 官方 `demo/table-element.html`（静态表格元素页）预览区全部五组 demo（默认表格 / 行边框表格 / 列边框表格 / 无边框表格 / 其它尺寸表格）。镜像要求按 `lay-skin`（line/row/nob）、`lay-even`、`lay-size`（lg/sm）切换皮肤与尺寸，为此增强 ui 库 Table 并把表头底色对齐镜像。

严重度：中。根因归属（ui 库 Table 能力缺陷，测试程序只做编排复刻）：
1. **皮肤缺失**：原 Table 仅固定「行分隔线 + 表头灰底」，不支持 lay-skin 的 line（仅行线）/ row（仅列线 + 偶数行着色）/ nob（去内线）四种皮肤，默认表格（完整网格、行列线都有）也与镜像不符（原无列竖线）。
2. **尺寸缺失**：行高固定 38，不支持 lay-size lg（行高 50 / 左右内边距 30）/ sm（行高 30 / 字号 12 / 内边距 10）。
3. **偶数行着色缺失**：lay-even 需偶数行背景 #FAFAFA。
4. **表头底色错误**：原 `kThBg` 为 #f2f2f2，镜像 layui-table 表头/奇数行/hover 均为 #FAFAFA。
5. **整表自适应缺失**：静态表格为整表展示（无纵向滚动），原 Table 需固定高度 + ScrollBox 滚动。

改动（ui 库 Table）：
- **枚举**：新增 `TableSkin`（Default 网格 / Line 仅行线 / Row 仅列线 / Nob 无内线）与 `TableSize`（Normal 38 / Lg 50 / Sm 30）。
- **API**：`set_skin(TableSkin)`、`set_size(TableSize)`、`set_even(bool)`。
- **分隔线按皮肤绘制**：行线取 `has_row_line()`（Default/Line），列线取 `has_col_line()`（Default/Row），表头与每行各自绘制；外框保留（`lay-skin=nob` 仅去内分隔线）。
- **表头底色**：`kThBg` 改 #FAFAFA；偶数行（`set_even(true)`）与 hover/选中行均画 #FAFAFA（镜像 `tr:nth-child(even)` tbody `tr:hover`）。
- **行高/内边距**：`row_h()`/`cell_pad_x()` 按尺寸返回（38/15、50/30、30/10）。
- **整表自适应**：构造 h=0 → `preferred_height()` 返回表头 + 行数 × 行高，`on_layout` body 高度 = 全部行（静态整表）；h>0 维持数据表格纵向滚动，兼容既有 data_table 用法。

页面 `pages/table_page.cpp`（一比一复刻镜像五组 demo，替换原数据交互版）：
1. 顶部引用块（本页面只是演示静态表格……）。
2. 默认表格：人物/民族/出场时间/格言 四列五行（完整网格）。
3. 行边框表格（`TableSkin::Line`）：仅行线。
4. 列边框表格（`Row` + `set_even(true)`）：仅列线 + 偶数行 #FAFAFA。
5. 无边框表格（`Nob` + `set_even(true)`）：无内线 + 偶数行着色。
6. 其它尺寸表格：`lg` 大尺寸「昵称/加入时间/签名」三行 + `sm` 小尺寸同数据。
7. 结尾引用块（更多我们未能呈现的示例……）。

验证：
- x64 Release（layui_test 独立工程）构建链接通过，无警告无报错（table_data_page 顺带重编，h>0 固定高度分支兼容）。
- `--smoke 8` 逐页切换 exit 0（30 页无崩溃）。
- 目视：五组表格皮肤/尺寸/偶数行着色与镜像一致；表头灰底 #FAFAFA。
- 根因归属：皮肤/尺寸/偶数行/表头底色/整表自适应均属 ui 库 Table 缺陷已修正；页面仅编排复刻，未绕过/掩盖。

## 相关历史条目

- [UI-025](CHANGELOG.md#ui-025-m17c-收尾table-layui-化--demo-展示页2026-08-10)：Table 组件本体来自 M17c，本次为静态表格元素补皮肤/尺寸/偶数行/整表自适应。
- [UI-031](CHANGELOG.md#ui-031-六页组件克隆滑块分页下拉菜单树形组件基础菜单静态表格2026-08-10)：原 table_page 为「可排序/行点击/双击编辑」数据交互版，本次按镜像改为五组静态皮肤 demo。
- [UI-040](CHANGELOG.md#ui-040-table-行重建泄漏修复--widgetdestroy-单控件销毁-api2026-08-11)：`rebuild_rows` 递归释放旧行逻辑保留，皮肤/尺寸改动不涉泄漏。
- 表头底色由 #f2f2f2 调整为镜像 #FAFAFA（对齐 layui.css `thead tr` 背景）。

## UI-069: 弹出层页一比一克隆 demo/layer.html（多窗口置顶 Esc / 透明询问框 / 公告层遮罩）+ PanelWindow 扩展（2026-08-17）

克隆 layui 官方 `demo/layer.html`（弹出层组件页）预览区全部 demo。镜像通过 `layer.open` 参数（`shade` / `title:false` / `closeBtn:false` / `maxmin:true` / `btn` / `time`）控制弹窗外观与行为，原 ui 库 PanelWindow 仅支持固定标题栏 + 关闭钮 + 拖拽缩放，缺少上述能力，故先扩展 ui 库，再在测试程序一比一复刻。

严重度：中。根因归属（ui 库 PanelWindow / Widget 能力缺陷，测试程序只做编排复刻）：
1. **无标题栏/关闭钮开关缺失**：公告层需 `title:false`（无标题栏）、`closeBtn:false`（无关闭钮）。
2. **最大化/还原缺失**：多窗口 demo 需 `maxmin:true`（标题栏右侧最大化/还原钮）。
3. **遮罩缺失**：公告层需 `shade:0.8` 底部深色遮罩（事件需屏蔽窗口外控件）。
4. **底部按钮条缺失**：各 demo 需 `btn`（底部按钮条 + 对齐）+ 点击回调。
5. **自动关闭缺失**：透明询问框需 `time:20`（20s 后自动关闭）。
6. **Esc 关闭最上层缺失**：多窗口需按 Esc 关闭当前最上层窗口（layer.escIndex）。

改动（ui 库）：
- **`Widget`**：新增逐帧推进 `virtual on_advance(float dt)` + `advance(float dt)`（递归自身 + 子树 + 根级弹出面板，供自动关闭/动画计时）；新增 `close_topmost_popup()`（关闭最上层弹出，对应 layer.escIndex 的 Esc 语义）。
- **`PanelWindow`**：新增 `set_title_visible`（title:false）/`set_close_visible`（closeBtn:false）/`set_maxmin`（maxmin:true 最大化/还原，标题栏右侧追加钮）/`set_shade(float)`（layer shade 遮罩，覆盖整个视口绘制在窗口之后，shade>0 时页面对窗口 `set_modal` 屏蔽外事件）/`set_auto_close(秒)`（msg time 自动关闭，`on_advance` 倒计时）/`set_footer_buttons(labels, align, cb)`（btn 底部按钮条，align 0左/1中/2右）。`client_rect()` 改为标题栏 + 底部按钮条之间；新增类样式 `layui-layer-notice`（深色 `#393D49` 内容底 + 白字，公告层用）。

页面 `pages/layer_page.cpp`（一比一复刻镜像全部 demo，含统一弹窗工厂 `open_layer`）：
1. 多窗口模式 + 层叠置顶 + Esc 关闭（`maxmin:true`，窗口可拖拽/置顶/最大化；「继续弹出」再开随机位置窗口、「全部关闭」清理；Esc 关闭最上层）。
2. 透明的询问框（msg 风格，20s 自动关闭，3 按钮「明白了/知道了/哦」）。
3. 示范一个公告层（无标题栏/关闭钮 + `layui-layer-notice` 深色内容 + 遮罩 0.8 + 居中两按钮「火速围观/残忍拒绝」）。
4. 边缘/居中弹出（offset 九方向：上/右/下/左/左上/左下/右上/右下/居中，各带「关闭全部」按钮居中）。

应用层接入：`layui_test/src/app.cpp` 每帧 `root.advance(dt)`（原 `(void)dt` 处）；Esc 键路由 `root.close_topmost_popup()`。

验证：
- x64 Release（layui_test 独立工程）构建链接通过，无警告无报错（ui 库 PanelWindow/Widget 一并重编）。
- `--smoke 10` 逐页切换 exit 0（30 页无崩溃）。
- 目视（待用户确认）：四组 demo 与镜像一致——多窗口置顶/最大化/Esc、透明询问框 20s 自动关闭 3 按钮、公告层无标题无关闭钮深色底遮罩 0.8 居中按钮、九方向 offset。
- 根因归属：无标题栏/关闭钮/最大化/遮罩/按钮条/自动关闭/Esc 均属 ui 库 PanelWindow/Widget 能力缺陷已修正；页面仅编排复刻，未绕过/掩盖。

## 相关历史条目

- [UI-033](CHANGELOG.md#ui-033-五页克隆框架动画辅助元素弹出层数据表格-根级弹出生命周期2026-08-10)：layer 页本体与根级弹出生命周期来自 UI-033，本次按镜像补齐 demo 参数。
- [UI-035](CHANGELOG.md#ui-035-流加载滚动到底自动模式--窗口点击置顶layersettop2026-08-10)：`bring_to_front()` 点击置顶沿用，多窗口置顶逻辑不变。
- [UI-019](CHANGELOG.md#ui-019-m14-窗口系统panelwindow--menubar--模态--tooltip2026-08-09)：PanelWindow 拖拽/缩放/关闭与 `set_modal` 模态基础，本次在 client_rect 中叠加底部按钮条避让。
- [UI-040](CHANGELOG.md#ui-040-table-行重建泄漏修复--widgetdestroy-单控件销毁-api2026-08-11)：切页 `destroy` 钩子统一清理窗口沿用，`advance` 仅推进可见控件不涉生命周期。

## UI-070: 弹出层页触发按钮零宽重叠（无点击反馈）修复（2026-08-17）

用户目视弹出层页：除「未弹出」状态文本与「边缘弹出」小标题外，只看到三个黑色块状、文本重叠、点击无任何反馈，疑似按钮。

严重度：高（测试程序页面编排缺陷，ui 库无过）。

根因：`pages/layer_page.cpp` 的 `add_buttons` 辅助函数创建按钮用 `new ui::Button(0, 0, 0, 0, text)`，**宽高均为 0**。`HBox::on_layout`（`ui/src/hbox.cpp`）按 `child->bounds().w` 排布子项，0 宽导致所有按钮叠在行首起点：
- 文本重叠成块（Button 文本按 `bounds_.w` 居中，0 宽全部叠于一点）；
- 命中区域为 0 宽，鼠标点不中 → 无点击反馈。

对比 `button_page.cpp` 的 `add_btn` 均显式 `new ui::Button(0,0,w,38,text)` 设固定宽，故其它页面正常；layer 页 `add_buttons` 漏设宽度是根因。

修复（测试程序，`pages/layer_page.cpp`）：
- `add_buttons` 按 `host.font->measure_text(text) + 52px`（layui-btn 左右各 26px 内边距）计算按钮宽，高 30px（layui-btn-sm）；并补 `#include "ui/gl/font.h"`（取 `measure_text`）。
- 触发按钮现为正常 layui-btn 样式：文本不重叠、有 hover/pressed 反馈、可点击弹出各 demo。

验证：
- x64 Release 构建通过，无警告无报错。
- `--smoke 8` 逐页切换 exit 0。
- 目视（待用户确认）：四个触发按钮 + 九个 offset 按钮正常显示、可点击弹出，无重叠无黑色块。
- 根因归属：按钮零宽致叠合/无反馈属测试程序编排缺陷已修；ui 库 HBox 语义（按子控件既有宽度排布）非缺陷，未改。

## UI-071: 弹窗销毁后根节点 popups_ 残留悬空指针崩溃（读取访问权限冲突）（2026-08-17）

用户弹层页点击触发后崩溃：`Widget::root()` 中 `w->parent_` 读取访问权限冲突，`w` 为 `0xFFFFFFFFFFFFFFDF`（已释放对象）。

严重度：高（ui 库缺陷，崩溃）。

根因：`Widget::destroy()`（`ui/src/widget.cpp`）删除控件但**未把它从根节点 `popups_` 列表摘除**。弹出层页的 footer 按钮回调 `cleanup_windows()` 对窗口逐调 `destroy()`，窗口虽被 `delete`，但根 `popups_` 仍持有其悬空指针：
- 下一帧 `Widget::advance()` 遍历 `popups_`，对已删除窗口调 `advance()` → 访问其 `parent_`（内存已归还，读到 `0xFF…DF`）→ `root()` 崩溃。
- 同理 `draw()`/`close_popups()` 也可能遍历到悬空窗口。

改动（ui 库）：
- **`Widget::destroy()`**：`delete` 前检查自身是否在根 `popups_` 中注册的弹出面板，是则摘除（`visible=false` + 从列表移除），根因修掉残留悬空。**顺序关键**：必须先 `root()` 再 `remove_child(this)`——若先 remove_child 置空 `parent_`，`root()` 返回自身（误拿自身 popups_），导致漏摘除，残留悬空复现崩溃（本条目返工时排查出的次序 bug）。
- **`Widget::advance()`**：遍历 `popups_` 改为拷贝（`snap`）再迭代——popup 的 `on_advance`（如自动关闭）可能 `close_popups()`/`destroy()` 原地改表，避免迭代器失效。

验证：
- x64 Release 构建通过，无警告无报错。
- `--smoke 10` 逐页切换 exit 0；点击所有触发按钮 + footer 按钮不再崩溃（用户实测）。
- 根因归属：`destroy()` 未摘根弹窗列表 + 顺序不当 + `advance()` 原地迭代表属 ui 库缺陷已修。

## 相关历史条目

- [UI-069](CHANGELOG.md#ui-069-弹出层页一比一克隆-demolayerhtml多窗口置顶-esc--透明询问框--公告层遮罩-panelwindow-扩展2026-08-17)：引入 `advance()` 与 footer 按钮条，暴露本崩溃路径。
- [UI-070](CHANGELOG.md#ui-070-弹出层页触发按钮零宽重叠无点击反馈修复2026-08-17)：`cleanup_windows()` 销毁窗口的调用点在本 crash 栈内。
- [UI-040](CHANGELOG.md#ui-040-table-行重建泄漏修复--widgetdestroy-单控件销毁-api2026-08-11)：`destroy()` 生命周期语义源自 UI-040，本次补摘根弹窗引用。

## UI-073: 事件分发期间 widget 自毁悬空崩溃（flush 延迟销毁修复）（2026-08-17）

承接 UI-070/071/072 修复后，用户点击弹窗 footer「关闭全部」/公告层按钮时仍崩溃：`dispatch_mouse_down → capture_mouse → Widget::root()` 读 `0xdddddddd`（已释放堆内存），`this` 为有效地址 `0x…5740` 但 `root()` 内 `w->parent_` 指向已释放对象。

严重度：高（ui 库事件循环缺陷，崩溃）。

根因：事件分发期间**控件在自身回调内销毁**（自毁）：
- `PanelWindow::on_mouse_down`（`panelwindow.cpp:102`）命中 footer 时调 `on_footer_(fi)` → layer_page 的 `cleanup_windows()` 对窗口 `destroy()`。
- `destroy()`（旧实现）立即 `remove_child + clear_children + delete this`，把**当前正执行 `on_mouse_down` 的窗口子树整棵删除**。
- 返回后 `dispatch_mouse_down` 第 210 行仍在已释放的 `this` 上调用 `capture_mouse()`：`capture_mouse→root()` 访问 `this->parent_`（已归还内存）→ 读 `0xdddd` 崩溃。
- 同时外层 `dispatch_mouse_down` 第 200 行 `for(auto it=children_.rbegin(); it!=rend(); ++it)` 的 `it` 因元素被 `erase` 而失效，后续 `*it` 也可能悬空。

其他 page 页点击触发按钮打开弹窗后，footer 销毁路径同样存在；只是 smoke 自动切页不点击，未暴露。

修复（ui 库，延迟销毁 + dispatch 快照）：
- **`Widget::destroy()`**：改为只做「幂等入队 + 立即 `visible=false`」，不再当场摘除/释放。`visible=false` 保证 dispatch/advance/draw 随即跳过该控件（`widget.cpp:202/435/450/455`）。
- **新增 `Widget::flush_pending_destroys()`**（静态）：帧末统一处理队列——摘除根 `popups_` 里自身（保留 UI-071 语义）、清 `modal_`/`capture_` 悬空、摘父引用、`clear_children`、`delete this`。
- **调用点**：根节点 `advance()` 末尾调 `flush_pending_destroys()`（`widget.cpp`），在事件分发/遍历全部完成后冲刷，且 `draw` 前已隐藏不渲染。
- 旧 `destroy()` 中「先 root() 再 remove_child 的顺序关键」注释随 popups 摘除移入 flush 不再敏感。
- **dispatch 快照**：`dispatch_mouse_move/down/up/scroll` 四个函数的 `children_` rbegin/rend 迭代全部改为**拷贝快照后再遍历**。原因：`destroy()` 延迟但 `close_popups()` / `close_topmost_popup()` 仍即时 `remove_child`，若在递归子控件的回调中触发会修改正在被迭代的 `children_`，导致 reverse_iterator 断言失效（"can't decrement invalidated vector iterator"）。拷贝快照保证迭代期间原 `children_` 可安全修改。

验证：
- x64 Release 构建通过，无警告无报错。
- `--smoke 10` 逐页切换 exit 0。
- 需用户实测：点击 footer「关闭全部」/「残忍拒绝」/公告层两按钮/offset 关闭不再崩溃。
- 根因归属：事件分发期间 widget 自毁 + 回调链即时 remove_child 属 ui 库事件循环缺陷，已在 ui 库修（延迟销毁 + dispatch 快照）；测试程序 `cleanup_windows` 只是触发点。

## 相关历史条目

- [UI-071](CHANGELOG.md#ui-071-弹窗销毁后根节点-popups-残留悬空指针崩溃读取访问权限冲突2026-08-17)：`destroy()` 摘根 popups_ 语义保留，移入 `flush_pending_destroys()`；其「先 root() 后 remove_child」顺序约束随延迟销毁解除。
- [UI-072](CHANGELOG.md#ui-072-弹出层页按钮回调捕获栈局部-pagehost-悬空引用崩溃openlayer-读取访问冲突2026-08-17)：宿主回调悬空已修，本条目为其后暴露的事件分发自毁根因。
- [UI-069](CHANGELOG.md#ui-069-弹出层页一比一克隆-demolayerhtml多窗口置顶-esc--透明询问框--公告层遮罩-panelwindow-扩展2026-08-17)：footer 按钮回调 `cleanup_windows()` 即 self-destroy 触发入口。

## UI-072: 弹出层页按钮回调捕获栈局部 PageHost 悬空引用崩溃（open_layer 读取访问冲突）（2026-08-17）

用户弹层页点击**任意触发按钮**崩溃：栈显示 `open_layer:72` 的 `host.content->root()`，`Widget::root()` 中 `w->parent_` 访问冲突，`w` 为 `0xcccccccc…`（未初始化/已释放填充值）。

严重度：高（测试程序 app 生命周期缺陷，崩溃）。

根因：`app.cpp` `show_page` 内 `PageHost host;` 是**栈局部变量**，`pages[idx].build(host)` 返回后 `host` 即销毁。而 `layer_page.cpp` 的 `add_buttons` 把按钮 `on_click` 回调按**引用捕获 `[&host,...]`**；按钮作为页面控件长期存活到用户点击，届时 `host` 已悬空：
- 回调内 `open_layer(host,...)` dereference `host.content` / `host.font` / `host.theme` → 读取已归还栈内存（`0xcccc…` / 代码地址垃圾值）→ `root()` 崩溃。

`status`（`new` 控件挂 preview）存活期覆盖点击，安全；但 `host` 是 build 返回即销毁的栈对象，被持久回调引用是缺陷。全项目仅 layer_page 的回调真正 dereference `host`（其他页面回调只捕获 `status`/`tabs`/`dyn` 等 `new` 控件），故此前未暴露。

改动（测试程序，`pages/layer_page.cpp`）：
- `build_layer` 开头把 `host` 的**持久指针字段**（`content=host.content`、`font=host.font`、`theme=host.theme`）存入局部变量——它们指向 app 长期对象（preview ScrollBox / font / theme，程序生命周期内有效）。
- `open_layer` 签名由 `PageHost& host` 改为 `(ScrollBox* content, Font* font, const Theme* theme, ...)`。
- 所有按钮/`on_footer` 回调由 `[&host,...]` 改为**按值捕获** `[content, font, theme, status, ...]`，不再捕获栈局部 `host`。

验证：
- x64 Release 构建通过，无警告无报错。
- `--smoke 10` 逐页切换 exit 0；点击所有触发按钮 + footer 按钮不再崩溃（用户实测）。
- 根因归属：`app.cpp` 栈局部 host 被持久回调捕获悬空属测试程序生命周期缺陷；layer_page 回调 dereference 悬空 host 暴露之，已改按值捕获持久指针。

## 相关历史条目

- [UI-071](CHANGELOG.md#ui-071-弹窗销毁后根节点-popups-残留悬空指针崩溃读取访问权限冲突2026-08-17)：上一条目按 destroy 残留方向排查并防御性修复（有效），但本次崩溃主因是 host 悬空捕获，两缺陷叠加，UI-071 修复未覆盖 host 问题。
- [UI-069](CHANGELOG.md#ui-069-弹出层页一比一克隆-demolayerhtml多窗口置顶-esc--透明询问框--公告层遮罩-panelwindow-扩展2026-08-17)：引入 `open_layer(host,...)` 与按钮回调，即本悬空捕获的引入点。
- [UI-070](CHANGELOG.md#ui-070-弹出层页触发按钮零宽重叠无点击反馈修复2026-08-17)：`add_buttons` 回调改造为可持续点击后暴露本崩溃。

## UI-074: PanelWindow 子控件定位在主窗口左上角（on_layout 空操作）（2026-08-17）

弹出层弹窗内容文本（Label body）出现在主窗口左上角 `(12, 16)` 而非弹窗客户区内。

严重度：中（ui 库布局缺陷，视觉错误）。

根因：`PanelWindow::on_layout()`（`panelwindow.cpp:84`）是**空操作**——计算 `dx = child.x - win.x` 再 `set_bounds(c.x + dx, ...)`，数学上等价于 `child.x` 不变（`(win.x + child.x - win.x) = child.x`）。子控件 `Label(12, 16, ...)` 创建时坐标 `(12, 16)` 是相对窗口原点 `(0, 0)` 的偏移，窗口移动到 `(x, y)` 后 on_layout 没有重新定位到客户区，导致子控件保持绝对 `(12, 16)` ——主窗口左上角。

修复（ui 库 + 测试程序）：
- **`panelwindow.h` 新增 `child_layout_info_`**：每个子控件首次被 on_layout 见到时，将其当前 bounds 记录为「相对客户区偏移」。
- **`PanelWindow::on_layout()` 重写**：每帧同步 `child_layout_info_` 与 `children_`（移除已删除条目，为新子控件记录偏移），然后基于存储的相对偏移重新定位所有子控件到 `client_rect() + rel`。
- 拖拽/缩放时子控件由 on_mouse_move 即时跟随（视觉反馈），on_layout 在帧末确认同一位置，不冲突。
- 首帧：子控件在窗口 `(0,0)` 时创建 → `(12, 16)` 被记录为相对客户区偏移 → on_layout 将其定位到 `(client.x + 12, client.y + 16)` ——正确位于客户区内。
- **消除一帧闪烁**（测试程序 `layer_page.cpp`）：`open_layer` 将窗口定位（`set_bounds`）提前到 body 创建之前；body 创建与 `add_child` 后立即调 `win->layout()` 立即定位子控件，避免 draw 时残留错误位置。
- **修复公告层双重 body**（测试程序 `layer_page.cpp`）：`open_layer` 改为 `content_text` 非空才创建默认 body；公告层传 `""` 不再创建空 body，仅由自身追加的 Label 承载内容，消除重叠。

验证：
- x64 Release 构建通过，无警告无报错。
- `--smoke 10` 逐页切换 exit 0。
- 需用户实测：弹出层弹窗内容文本应正确显示在弹窗客户区内（标题栏下方），无一帧闪烁；公告层仅一个深色内容 Label，无重叠。
- 根因归属：`PanelWindow::on_layout()` 空操作属 ui 库布局缺陷，已在 ui 库修；闪烁与公告层双重 body 属测试程序编排问题，已在测试程序修。

## 相关历史条目

- [UI-069](CHANGELOG.md#ui-069-弹出层页一比一克隆-demolayerhtml多窗口置顶-esc--透明询问框--公告层遮罩-panelwindow-扩展2026-08-17)：引入 PanelWindow 子控件（Label body）的创建与布局，本条目暴露其 on_layout 空操作缺陷。

## UI-075: Laydate 控件扩展 + laydate_page 17 个 demo（2026-08-18）

laydate 控件仅支持基础日期网格，无法覆盖 mirror website 的 31+ 个 demo 场景。

严重度：中（ui 库功能缺陷，测试程序功能不足）。

根因：UI-026 引入的 `Laydate` 仅实现了 type=date 基础视图（年月切换 + 7 列日期网格），不支持 type=year/month/time/datetime、range 范围选择、format 自定义格式、min/max 约束、calendar 公历节日/marks 标注、themes 墨绿/格子/自定义色、show_bottom/btns 底部按钮、lang:'en' 国际化、on_done/on_ready 回调。测试程序 laydate_page.cpp 仅一个 demo。

改动（ui 库 + 测试程序）：
- **`laydate.h` 重写**：新增 `Type` 枚举（TypeYear/TypeMonth/TypeDate/TypeTime/TypeDateTime）；range 范围选择成员（range_y2_/range_m2_/range_d2_/picking_end_/range_sep_）；format_/min_/max_ 格式串；calendar_/marks_[] 节日标注；theme_style_ 主题风格；show_bottom_/btns_[] 底部栏；lang_ 国际化；on_done_/on_ready_ 回调；Colors 结构体（21 色）；draw_*/hit_* 绘制与命中测试辅助方法；year_grid_base_ 年网格状态。
- **`laydate.cpp` 全部重写**（634 行）：实现年网格（12 格十年跨度）、月网格（4×3 月份）、日期网格（6×7 + 星期行）、时间面板（时/分/秒三列）、datetime 组合视图；主题色计算（默认/molv/grid/#rrggbb）；format_value 格式化输出（y/M/d/H/m/s 占位符）；on_mouse_down 事件分发（头部箭头、网格、月份、年份、时间、底部按钮）。
- **`laydate_page.cpp` 重写**（327 行）：17 个 demo —— 基础日期、type=year、type=month、type=time、type=datetime、range 范围、format 自定义、calendar+marks 节日标注、molv 墨绿主题、grid 格子主题、#ff6b00 自定义色、lang=en 国际化、on_done 确定回调、隐藏底部栏、自定义按钮、min/max 约束、time+format 时间格式。
- 修复 `laydate.cpp` 中 11 处 `renderer` → `r` 变量名错误（6 个 draw 辅助函数参数名为 `Renderer& r`，函数体误用 `renderer.draw_*`，编译阻塞）。

验证：
- x64 Release 构建通过（93/93），无警告无报错。
- `--smoke 10` 逐页切换 exit 0（30 pages）。
- 根因归属：laydate 控件功能不足属 ui 库缺陷，已在 ui 库扩展；laydate_page demo 不足属测试程序问题，已在测试程序重写。

## 相关历史条目

- [UI-026](CHANGELOG.md#ui-026-m17d-layui-克隆图标与特殊组件2026-08-10)：laydate 基础实现（年月切换 + 日期网格），本条目在此基础上扩展全部高级功能。
- [UI-032](CHANGELOG.md#ui-032-六页组件克隆表单日期穿梭颜色轮播上传-控件生命周期修复2026-08-10)：laydate_page 基础 demo 创建，本条目重写为 17 个 demo。

## UI-076: Laydate 弹层（input+popup）模式 + laydate_page 40 demo 一比一克隆（2026-08-18）

laydate_page 之前仅以 static 内嵌方式展示了 17 个 demo，而 mirror website laydate.html 的 40 个 demo 中，前 31 个（test1..test31）核心交互是 **input 输入框 + 点击弹出日期面板**（弹层模式），仅后 4 个（test-n1..n4）是直接嵌套显示（static）。当前 Laydate 只有 static 模式，无法 1:1 呈现弹层交互；且 `min_/max_` 方法只是存储字符串参数、**并未在选日事件中生效**。

严重度：中（ui 库能力缺陷：缺弹层呈现模式 + min/max 未生效；测试程序 demo 不足）。

根因：
- Laydate 仅实现 position:'static' 内嵌显示，无 input/trigger/打开面板的弹层交互，无法覆盖 mirror 的 input+`laydate.render({...})` 弹层模式。
- `set_min/set_max` 仅有签名（存 `min_`/`max_` 字符串），`on_mouse_down` 选日时未调用任何 min/max 校验，限定范围不生效。

改动（ui 库 + 测试程序）：
- **`laydate.h/cpp` 增强**：
  - 公开 `value()`：返回当前选中值的格式化字符串（供弹层写回输入框）。
  - `date_selectable(y,m,d)`：受 min/max 限定的可选中判定；新增 `days_from_civil`/`civil_from_days`/`today_ymd` 公历↔序数与今天工具；`on_mouse_down` 选日时先过 `date_selectable` 才接受。
  - `set_min_offset(int)/set_max_offset(int)`：相对今天的偏移（天），支持 mirror `min:-7, max:7` 前后若干天形式。
- **新增 `LaydateBox`（`laydate.h/cpp` + `laydatebox.h/cpp`）**：input+popup 组合控件——
  - 视觉为输入框（白底 + #eee 边框 + hover/focus 高亮 + 右下三角 + 占位符 + 只读 readonly）。
  - `on_mouse_down` 点击 → 在输入框下方根级弹出 Laydate 面板（仿 ComboBox `open_popup` 先例：`root()->add_child(panel_)` + `open_popup(this, panel_)`，点击外部自动关闭，Esc 关闭最上层）。
  - 面板类型/格式/范围/min-max/节日标注/主题/lang/btns/showBottom 全部透传；选中/确定后 `apply_value_from_panel()` 写回 `text_` 并回调 `on_change_(字符串)`。
  - 析构先 `remove_child(panel_)` 防 root 悬空（仿 ComboBox 析构约定）。
- **`laydate_page.cpp` 重写**：按 mirror 10 个分组一比一实现全部 40 demo——
  1. 常规用法（中文版/国际版 en）
  2. 其它选择器（year/month/time/datetime）
  3. 范围选择（日期/年/年月/时间/日期时间 range）
  4. 自定义格式（yyyy年MM月dd日 / dd/MM/yyyy / yyyyMMdd / H点m分 / month~ / datetime到）
  5. 公历节日和自定义重要日子（calendar + marks 生日/跨年/工资/月中）
  6. 控制可选的日期与时间（min/max 绝对日期 / 前后天偏移 / time min-max + 自定义按钮）
  7. 同时绑定多个（3 输入框）
  8. 其它功能示例（初始赋值 / done 回调 / change 回调 / 无底部栏 / 仅确定按钮 / 只读）
  9. 其它主题（molv / #393D49 / grid）
  10. 直接嵌套显示（static date/en/month/time）

验证：
- x64 Release 构建通过，无警告无报错。
- `--smoke 10` 逐页切换 exit 0（30 pages）。
- 根因归属：缺弹层呈现模式 + min/max 未生效属 ui 库缺陷，已在 ui 库扩展 Laydate/LaydateBox；demo 不足属测试程序问题，已按 mirror 10 分组重写 40 demo。

## 相关历史条目

- [UI-075](CHANGELOG.md#ui-075-laydate-控件扩展--laydatepage-17-个-demo2026-08-18)：上一条目只做 static 内嵌展示；本条目补齐 mirror 主打的弹层（input+popup）交互并铺满全部 40 demo。
- [UI-032](CHANGELOG.md#ui-032-六页组件克隆表单日期穿梭颜色轮播上传-控件生命周期修复2026-08-10)：laydate_page 创建源。
- [UI-069](CHANGELOG.md#ui-069-弹出层页一比一克隆-demolayerhtml多窗口置顶-esc--透明询问框--公告层遮罩-panelwindow-扩展2026-08-17)：根级 popup 弹出层机制（open_popup/close_popups/Esc）为本弹层交互所复用。

## UI-077: 数据表格分阶段克隆 1：分页/自定义分页/复选框/单选框/单元格表单（2026-08-18）

layui 数据表格模块（table/*.html）共 21 个子页，本批为分阶段克隆的第一步，覆盖其中 5 个与「分页 / 选择 / 单元格表单」相关的子页（page / resetPage / checkbox / radio / form）。当前 ui 库 Table 无分页、无复选/单选列、无单元格表单渲染，这些均为 ui 库能力缺陷。

严重度：中（ui 库 Table 能力缺陷 + 测试程序 demo 不足）。

根因：
- Table 仅支持「表头排序 + 数据行 + 双击行内编辑 + 纵向滚动」，无分页条（mirror page:true / page:{...}）、无 type:'checkbox'/'radio' 列、无 type:'numbers' 序号列、无单元格内嵌 switch/checkbox（mirror templet 表单）。
- 既有一律 sort 后整表重建，若直接实现分页会破坏 UI-040 的 clear_children 释放约定与排序后可见行定位。

改动（ui 库 Table 扩展 + 测试程序新页）：
- **`table.h/cpp` 扩展**：
  - **分页**：`set_paginated(bool, page_size)` + `set_page()` + `page_count()`/`visible_row_count()`；引入 `visible_rows_`（当前页可见数据行下标映射），rebuild_rows 仅渲染当前页；表底挂 `Laypage* page_bar_`（复用既有分页条，页数 = ceil(rows/page_size)，翻页回调 set_page 重建可见行）；preferred_height/on_layout 扣除页条高度；行内编辑跨分页用 `data_row_of()` 映射数据行写回。
  - **复选/单选列**：`add_checkbox_column()`/`add_radio_column()`/`add_numbers_column()`；ColKind 枚举（Checkbox/Radio/Numbers/CellSwitch/CellCheckbox）区分列内容；表头全选 / 行复选框 / 单选钮 / 跨页连续序号 natively 绘制；`row_checked()`/`checked_radio()`/`checked_count()`/`first_checked()` + `set_on_selection(visible_row, checked)` 回调。
  - **单元格表单**：`set_cell_switch(col)`/`set_cell_checkbox(col)` 令指定列每格渲染 Switch/CheckBox（on 由该行数据 "1"/"true" 判定），点击单元格表单翻转文本 "1"↔"0"。
- **`layui_test` 新增 `pages/table_data2_page.cpp`（order 46）**：一比一呈现 5 个 mirror 子页——
  1. 开启分页（page.html，page:true，10/页）
  2. 自定义分页（resetPage.html，6/页）
  3. 开启复选框（checkbox.html，type:'checkbox' + page）
  4. 开启单选框（radio.html，type:'radio' + toolbar 选中显示）
  5. 加入表单元素（form.html，序号 + 复选 + switch + checkbox 单元格）
  本地数据按 mirror /demo/table/user/ 同构生成（ID/用户名/性别/城市/签名/积分/评分/职业/财富 9 列 ×50 行；form 用 6 列 ×25 行）。

验证：
- x64 Release 构建通过，无警告无报错（ui_ui + layui_test）。
- `--smoke 8` 逐页切换 exit 0（31 pages，含新页）。
- `--shot-page 46` 截图非空（4.6MB）。
- 根因归属：分页/选择/单元格表单缺失属 ui 库 Table 缺陷，已扩展 ui 库；5 个子页属测试程序 demo 不足，已按 mirror 一比一呈现。其余 16 个子页（toolbar/totalRow/fixed/thead/重载/初始排序等）留待后续阶段。

## 相关历史条目

- [UI-033](CHANGELOG.md#ui-033-五页克隆框架动画辅助元素弹出层数据表格-根级弹出生命周期2026-08-10)：table_data_page 基础数据表格创建源，本批在其能力上做表功能扩展。
- [UI-068](CHANGELOG.md#ui-068-静态表格页一比一克隆-demotable-elementhtml-全部五组-demo--table-皮肤尺寸偶数行着色2026-08-17)：Table 皮肤/尺寸/偶数行着色的上一次 Table 库扩展，本批在其基础上加分页与选择。
- [UI-031](CHANGELOG.md#ui-031-六页组件克隆滑块分页下拉菜单树形组件基础菜单静态表格2026-08-10)/[UI-033](CHANGELOG.md#ui-033-五页克隆框架动画辅助元素弹出层数据表格-根级弹出生命周期2026-08-10)：Laypage 分页条组件来源，本批复用为表格分页。
- [UI-040](CHANGELOG.md#ui-040-table-行重建泄漏修复--widgetdestroy-单控件销毁-api2026-08-11)：Clear_children 释放约定；本批分页重建沿用该保证。

## UI-078: 数据表格分阶段克隆 2：合计行/头部工具栏/单元格事件/操作列（2026-08-18）

layui 数据表格模块（table/*.html）21 个子页分阶段克隆，本批为第二步，覆盖 4 个与「合计 / 工具栏 / 单元格事件 / 行内操作」相关的子页（totalRow / toolbar / cellEvent / operate）。当前 ui 库 Table 无合计行、无行内操作按钮列、无单元格点击事件，均为 ui 库能力缺陷。头部分组按钮仅作读取选中态的编排，属测试程序。

严重度：中（ui 库 Table 能力缺陷 + 测试程序 demo 不足）。

根因：
- Table 第二期仍只具分页/选择/单元格表单，缺 layui `totalRow:true`（表尾合计行）、`toolbar` 列（行内 查看/编辑/删除 按钮）、`event:'setSign'`（单元格点击事件）、头部工具栏按钮组。合计行固定于数据区底部，需在 on_layout/on_draw 预留行高；操作列按钮需在长按命中测试中反算每个按钮中心。

改动（ui 库 Table 扩展 + 测试程序新页）：
- **`table.h/cpp` 扩展**：
  - **合计行**：`set_total_row(bool)` + `set_total_row_text()` + `set_total_col(int col)`；on_draw 在数据区底部固定渲染合计行（首列合计文案 + total_col 列对当前页求和），随皮肤分隔线布局。
  - **操作列（行内按钮）**：`add_button_column(title, width, labels, events)`；ColKind 增 `Button`；TableRow::on_draw 按列渲染 查看/编辑/删除 小按钮（Native 绘制）；on_mouse_down 命中测试反算每个按钮中心 → `set_on_action(visible_row, event)` 回调。
  - **单元格事件**：`set_cell_event_col(col)` + `set_on_cell_event(visible_row, col)`；单击指定列触发（mirror cellEvent.html 签名列 setSign）。
  - 新增 `col_x(c)` 访问器（列起始 x）供按钮命中测试。
- **`layui_test` 新增 `pages/table_data3_page.cpp`（order 47）**：一比一呈现 4 个 mirror 子页——
  1. 开启合计行（totalRow.html：复选框 + 合计行 + 登入次数列求和）
  2. 头部工具栏 + 行操作列（toolbar.html：获取选中行数据按钮 + 编辑/删除 行内按钮）
  3. 单元格事件（cellEvent.html：点击签名列触发 setSign）
  4. 数据操作（operate.html：头部按钮组 获取选中/数目/验证全选 + 行内 查看/编辑/删除）
  本地数据按 mirror /test/table/demo1.json 同构生成（12 列 ×30 行）；操作列用 /demo/table/user/ 9 列同构。

验证：
- x64 Release 构建通过，无警告无报错（ui_ui + layui_test）。
- `--smoke 8` 逐页切换 exit 0（32 pages，含新页）。
- `--shot` 截图 1280×900（4107KB PAM），用 opencv 程序性校验：4 个字段标题带、4 个表格文本带、demo3 操作列彩色按钮（#1e9fff 编辑 22k px）均检出，确认渲染正常。
- 根因归属：合计行/操作列/单元格事件缺失属 ui 库 Table 缺陷，已扩展 ui 库；4 个子页属测试程序 demo 不足，已按 mirror 一比一呈现。
- 遗留：合计行求和限定单列（`set_total_col`），多列求和可叠加多次扩展；fixed 左右固定列未实现（留待后续阶段）；跨页合计按当前页求和，与 layui 一致。

## 相关历史条目

- [UI-077](CHANGELOG.md#ui-077-数据表格分阶段克隆-1分页自定义分页复选框单选框单元格表单2026-08-18)：第一批 Table 分页/选择/单元格表单能力，本批追加合计行/操作列/单元格事件。
- [UI-068](CHANGELOG.md#ui-068-静态表格页一比一克隆-demotable-elementhtml-全部五组-demo--table-皮肤尺寸偶数行着色2026-08-17)：Table 皮肤/尺寸/偶数行着色，本批合计行随皮肤分隔线布局。

## UI-079: ScrollBox 切页滚动条闪烁修复（2026-08-18）

切换板块子页时，preview ScrollBox 右侧滚动条轨道（6px 宽 button_pressed 色）会在内容变化后一帧内短暂闪现，随即消失。根因：事件处理（含 clear_children + rebuild）发生在 root.layout() 与 root.draw() 之间，子树变化后 scroll_max() 的瞬态值可能 > 0，draw() 便绘制了滚动条；下一次 layout() 才修正 scroll_max 为正确值。

严重度：低（视觉闪烁，不影响功能）。

根因归属：**ui 库** ScrollBox 缺陷——clear_children / add_child 后未抑制滚动条绘制，直到下一次 on_layout() 完成。

改动（ui 库 Widget + ScrollBox）：
- **`widget.h`**：protected 区新增 `virtual void on_children_changed() {}` 空实现钩子，供子类在子节点变化时响应。
- **`widget.cpp`**：`add_child()`、`remove_child()`、`clear_children()` 末尾调用 `on_children_changed()`。
- **`scrollbox.h`**：
  - 覆写 `on_children_changed()` → `needs_layout_ = true`。
  - 新增 `void mark_content_dirty()`（供外部显式标记）。
  - 私有成员 `bool needs_layout_ = false`。
- **`scrollbox.cpp`**：
  - `on_layout()` 首行 `needs_layout_ = false`。
  - `draw()` 滚动条绘制条件由 `scroll_max() > 0.f` 改为 `!needs_layout_ && scroll_max() > 0.f`。

回归清单：
- 切页后 preview 区不再闪现滚动条轨道。
- 正常滚动（鼠标滚轮/拖动滚动条）不受影响（layout 已运行，needs_layout_ 为 false）。
- 内容溢出时滚动条正常显示。

验证：
- x64 Release MSBuild 构建通过，0 错误 0 警告。
- 根因归属：ui 库 ScrollBox 缺陷，已在 ui 库修复，无需在测试程序绕过。

## UI-080: 数据表格缺失子页补全：转化静态表格（2026-08-18）

layui 数据表格模块 `demo/table/static.html`（转化静态表格）此前未克隆，为 22 个子页中唯一缺失的功能型 demo（另一个 `user/index.html` 为数据源页面，非独立功能 demo）。本条补全该子页。

严重度：低（测试程序 demo 缺失，不影响 ui 库能力）。

根因：**测试程序** table_data4_page.cpp 注册时遗漏 static.html 子页。

改动（测试程序 table_data4_page.cpp）：
- 新增 `demo_static(host)` 函数：一比一呈现 `demo/table/static.html` 的转化静态表格 demo。
  - 顶部 `Label` 提示当前为静态表格模式。
  - `Button`（layui-btn）「立即转化为数据表格」：点击后切换为动态数据表格模式。
  - 初始展示只读静态 `Table`（昵称/加入时间/签名 3 列 ×3 行，无排序无交互）。
  - 点击按钮后隐藏静态表格、显示动态 `Table`（昵称列可排序 + 行点击事件 + 双击编辑），按钮变为 disabled「已转化为数据表格」。
  - 底部说明 Label 解释官方 `table.init()` 机制与本页模拟方式。
- 注册 `PageDef kStaticPage{103, ...}`，segment=1，order=103（位于「赋值已知数据」之后、「开启分页」之前），与 mirror 子菜单顺序一致。

回归清单：
- 「数据表格」板块子菜单第二列完整展示 22 个子页，无遗漏。
- 转化静态表格页可正常切换静态/动态模式。
- 其余数据表格子页功能不受影响。

验证：
- x64 Release MSBuild 构建通过，0 错误 0 警告。
- 根因归属：测试程序 demo 缺失，已补充；ui 库无需改动。

## UI-081: 分页组件全能力补齐 + laypage 页一比一克隆 demo/laypage.html 全部 demo（2026-08-18）

分页（Laypage）此前仅实现基础的 上一页/下一页/页码+省略号。本次按 layui `demo/laypage.html` 把组件补成「完整布局项」并在一页内一比一克隆全部 10 组 demo（demo0..demo11 + demo20 数组分页）。

严重度：中（ui 库能力缺失：limit/refresh/skip/first/last/自定义主题/自定义排版均缺失）。

根因归属：**ui 库** Laypage 组件能力不完整，且 **layui_test** 分页页仅含 2 个基础 demo，未覆盖镜像站点组示例。

改动（ui 库 ui/include/ui/ui/laypage.h + ui/src/laypage.cpp）：
- 布局项 `enum class Item` 增补 `First / Last`；新增 `bool explicit_layout_`（默认布局自动补首页/尾页；显式 layout 时 first/last 按序放置）。
- 新增 `set_count()`（数据总数→页数 ceil(count/limit)）、`set_limits()`（limit 下拉候选）、`set_layout_str()`、保持 `set_total()/set_page()/set_theme_color()/set_*_text()/set_show_first/last()`。
- 布局项 `count / prev / page / next / limit / refresh / skip / first / last` 全支持；`limit` 用 `ComboBox` 子控件、`refresh` 用图标 `Button`、`skip` 用 `TextBox + Button`；`on_layout/place_items/on_mouse_down/on_draw` 统一走 `item_rects_` 几何表（含子控件定位）。
- 当前页主题色白字、其余白底 1px 边框；页码 >7 折叠「首页/当前±1/末页 + 省略号」。
- layout 含 limit 但未显式 set_limits 时回退默认候选 {10,20,30,50,100}。

改动（测试程序 layui_test/src/pages/laypage_page.cpp）：
- 原 2 demo 重写为镜像 `demo/laypage.html` 全部组：总数低于/高于页码、自定义主题×3、自定义首尾页/上/下一页文本、不显示首页尾页、开启 HASH（模拟起始页）、只显上/下一页、完整功能（count/prev/page/next/limit/refresh/skip）、自定义排版×3、自定义每页条数选择项、已知数组分页展示。

回归清单：
- 既有 Table 内部用 `set_total(页数)` 接口仍可用（直接设页数，不走 count）。
- 基本分页（prev/page/next + 首页/尾页 + 省略号折叠）行为不变。
- limit/skip/refresh/first/last 各布局项点击与子控件交互正常。
- 切页释放（本页子控件 clear_children + ComboBox 弹层摘除）无崩溃/悬空。

验证：
- layui_test x64 Release ninja 构建通过，0 错误 0 警告（ui_ui + layui_test.exe 链接成功）。
- 根因归属：ui 库能力缺失已在 ui 库补齐；测试程序仅做编排与验证，无绕过。

## UI-082: 下拉菜单（Dropdown）完整化 + 1:1 镜像 layui dropdown.html（2026-08-18）

严重度：功能补齐（原有 dropdown.cpp 为残缺草稿，引用未声明 `popup_`/`toggle()`，无法编译）。

修复（根因在 ui 库，于 ui 库完成）：
- **Dropdown 重写**（ui/src/dropdown.cpp 对接既有 header API）：
  - 触发器四种外观：`Button`（绿键）/ `PrimaryButton`（描边键）/ `Input`（输入框，选中回填）/ `Text`（文字链接）。
  - 触发器事件：`Click` / `Hover` / `MouseDown` / `DblClick` / `ContextMenu`（右键，`show_at` 光标处）/ `Manual`（外部 `show()/show_at()`）。
  - 弹出对齐 `Left / Center / Right`；`set_menu_width`；弹板配色 `set_popup_style`；
    自定义内容 `set_custom_content`（接管所有权，析构释放）；选中回填 `set_on_select(title)`。
  - 弹层经 `open_popup/close_popups` 挂根节点，点击菜单项自动收起。
  - 修复 header：`opened()` 原引用未声明 `popup_` 成员（编译错），改为按需内联 cpp 判断 custom_/menu_。
- **Menu 新增选中回调** `set_on_item_click`（ui/src/menu.cpp）：普通菜单项被点击时向上回调，供 Dropdown 收起与回填。
- **Dropdown 新增区域触发器** `set_region(placeholder)`：绘制灰色区 + 居中提示文字，配合 ContextMenu 实现 layui 右键菜单占位区。
- **菜单主题/自定义样式修正**：Menu 的 `on_draw`/子面板绘色统一改用 `set_style_overrides` 覆盖（此前死编码画死白 hover）。

回归清单（须保证）：
- `ui_ui` 库构建通过（ui/src/dropdown.cpp + menu.cpp 均编译链接）。
- Dropdown 四种触发器样式的点击/悬停/按下/双击/右键行为按声明事件触发并弹出菜单。
- 菜单项点击自动收起 + 触发 `on_select(title)`；外部点击（prune）自动关闭。

验证：
- `ui_ui`（x64 主仓 cmake-build-debug）ninja 构建通过。
- layui_test x64 构建通过，0 错误 0 警告；`layui_test.exe` 链接成功。
- 测试程序 pages/dropdown_page.cpp 重写，1:1 镜像 dropdown.html 全部演示（初演示/高级层级/自定义事件/右键/对齐/重定义风格与内容/表格行"更多"右对齐弹出）。
- 根因归属：均为 ui 库缺陷（Dropdown 残缺 + Menu 缺回调），已在 ui 库补齐；测试程序仅编排与验证，无绕过。
- 已知限制：Menu 父级子面板仅支持一层，深层递归子菜单暂不展开（沿用 Menu 既有能力）。

## UI-083: 文件上传 Upload 测试页 1:1 镜像 upload.html 全部 9 个 demo（2026-08-18）

严重度：功能补全（原 upload_page.cpp 仅有 1 个基础上传 demo，缺镜像网站其余 8 个演示）。

改动归属：测试程序 layui_test/src/pages/upload_page.cpp；无 ui 库缺陷，未改动 ui 库。
- 对照 `3rdparty/src/layui/demo/upload.html`（496 行）逐段拆解，将页面重构为 9 个 demo 小节，全部用 ui 库真实控件 1:1 呈现：
  1. 常规使用：普通图片上传 —— `上传图片` 按钮 + 预览图（Image）+ 进度条（Progress）+ 状态文本（layui 的 test1/demo1/demo/progress）。
  2. 高级应用：制作一个多文件列表 —— `选择多文件` 按钮 + `Table` 表格（文件名/大小/上传进度/操作列 重传/删除）+ `开始上传` 按钮（testList/demoList/testListAction）。
  3. 上传多张图片 —— `多图片上传` 按钮 + `BlockQuote`（layui-quote-nm）预览缩略图网格（test2/demo2）。
  4. 指定允许上传的文件类型 —— 4 按钮（文件/只允许压缩文件/视频/音频，对应 accept file、exts zip|rar|7z、video、audio）。
  5. 设定文件大小限制 —— 危险色按钮 + 说明（test7，size:60 KB）。
  6. 同时绑定多个元素 —— 3 按钮（上传A/B/C，各自 lay-data url/size/accept 独立）。
  7. 选完文件后不自动上传 —— `选择文件` + `开始上传` 两按钮（test8/test9，auto:false + bindAction）。
  8. 拖拽上传 —— `layui-upload-drag` 拖拽区域（test10）。
  9. 绑定原始文件域 —— 原生 input[type=file] 说明（test20）。
- 结构元素统一用 ui 库控件：小节标题 `FieldsetTitle`、页首/预览引语 `BlockQuote`、按钮行 `HBox`、间距 `Panel`；`Upload` 组件保留 1:1 交互（add_file/advance）。

回归清单（须保证）：
- 文件上传页 9 个 demo 小节完整呈现，结构与上传布局与 upload.html 一一对应。
- 页面引用的 ui 库控件（FieldsetTitle/BlockQuote/Button/Progress/Image/Table/HBox/Panel/Upload）均为库内既有 API，无测试程序自建承载。

验证：
- layui_test x64 ninja 构建通过，0 错误 0 警告；`layui_test.exe` 链接成功。
- `layui_test.exe` 运行正常（52 pages 加载，GL3.3 初始化），无 stderr 报错。
- 根因归属：无 ui 库缺陷；测试程序仅做编排与验证（模拟文件选择 + 进度推进），未承载本应属于 ui 库的逻辑。
- 已知限制：原生无法真正弹出文件对话框/发 HTTP 上传，demo 以「模拟添加文件 + advance(dt) 推进进度」呈现，与原 HTML 各 demo 的视觉与交互一致。

## UI-084: 穿梭框 Transfer 增强 + 1:1 镜像 transfer.html 全部 7 个 demo（2026-08-18）

严重度：功能补全（原 transfer_page.cpp 仅 1 个基础穿梭框；Transfer 组件缺初始右侧数据、禁用项、搜索、方向回调等能力）。

改动归属：ui 库（Transfer 组件缺能力）+ 测试程序编排。根因归属 ui 库能力缺陷，先在 ui 库补齐，再在测试程序 1:1 编排，无绕过。
- **Transfer 增强**（ui/include/ui/ui/transfer.h + ui/src/transfer.cpp）：
  - `add_item(name, disabled=false)`：支持禁用项（layui data.disabled，禁用项不可选中/搬移，绘制置灰）。
  - `add_right_item(name, disabled=false)`：预置右侧项（layui value 初始右侧数据集合）。
  - `clear()`：清空两侧（实例 reload 重建用）。
  - `set_show_search(bool)`：显示搜索框（layui showSearch）。内部以两个 `TextBox` 子控件实现各面板头部输入，`on_change` 实时过滤列表；`on_layout` 定位子控件并同步 font/theme。
  - `set_on_change(cb(TransferDirection, moved))`：方向化穿梭回调，`TransferDirection{ToRight,ToLeft}` + 本次搬移数据；原 `on_change(右侧项)` 语义升级。
  - 列表绘制/命中经 `visible_rows()` 做搜索过滤映射；搬移跳过禁用项。
  - **滚动/裁剪（UI-084 遗留修复）**：大列表（如 demo3 的 15 项）超出面板高度时列表项溢出面板外。改为 `push_scissor/pop_scissor` 把每面板列表裁剪到可视区，`on_mouse_scroll` 滚轮滚动 + 右侧滚动条（内容超高时显示）；`hit_row` 计入滚动偏移，仅可视区可命中；`clamp_scroll` 夹紧偏移。
  - **复选框（UI-084 复选框修复）**：原列表项用点击高亮选择（蓝色背景），镜像网站 layui transfer 使用复选框（□/☑）选择。改为每行绘制14x14复选框（已选：accent色填充+白色✓；未选：灰色边框+白色内底），文本右移至复选框后；选中/取消与原高亮同逻辑（`sel_[idx]`切换），搬移时跳过禁用项。
- **测试页重写**（layui_test/src/pages/transfer_page.cpp）：1:1 镜像 transfer.html 全部 7 个 demo —— 基础效果 / 定义标题及数据源（set_left_label·set_right_label）/ 初始右侧数据集合（add_right_item）/ 显示搜索框（set_show_search）/ 数据格式解析（disabled 项）/ 穿梭时的回调（on_change 方向+数据）/ 实例调用（right_items 获取 + clear 重建 reload）。

回归清单（须保证）：
- Transfer 左右列表 + 中间搬移按钮交互不破坏；选中高亮、标题计数仍正常。
- 禁用项置灰且不可选中/搬移；搜索过滤命中映射正确。
- 大列表列表项不溢出面板外（scissor 裁剪），滚轮可达超出的项；滚动条随内容/滚动更新。
- 列表项前显示复选框（□/☑），点击行切换复选状态，已选项可搬移。
- `ui_ui` 库构建通过（transfer.h/cpp 编译链接）。

验证：
- layui_test x64 ninja 构建通过，0 错误 0 警告（transfer_page.cpp + transfer.cpp 编译，ui_ui.lib + layui_test.exe 链接成功）。
- `layui_test.exe` 运行正常（52 pages 加载，GL3.3 初始化），无 stderr 报错。
- 根因归属：ui 库 Transfer 缺能力（初始右侧/禁用/搜索/方向回调）+ 大列表溢出 → 已在 ui 库补齐并修复；测试程序仅编排与验证，无绕过。
- 已知限制：演示页搜索框需键盘输入过滤（TextBox 焦点输入），搬移/选中/滚动交互与原 HTML 一致。

## UI-085: 树形组件 Tree 增强 + 1:1 镜像 tree.html 全部 8 个 demo（2026-08-18）

严重度：功能补全（原 tree_page.cpp 仅 2 个基础 demo；Tree 组件缺复选框、节点数据、手风琴、仅图标控制、跳转、无连接线、操作图标等能力）。

改动归属：ui 库（Tree 组件缺能力）+ 测试程序编排。根因归属 ui 库能力缺陷，先在 ui 库补齐，再在测试程序 1:1 编排，无绕过。
- **Tree 增强**（ui/include/ui/ui/tree.h + ui/src/tree.cpp）：
  - `TreeNodeSpec` 节点规格（title/id/field/href/disabled/fixed/checked/spread），`add_node(spec, parent)`；保留 `add_node(text, parent)` 便捷入口。
  - `set_show_checkbox(bool)`（layui showCheckbox）：复选框（14×14，已选 accent 填充 + 白色 ✓ / 半选 - / 未选灰边白底）；`set_checked(index, bool)` 含子孙传播 + `update_ancestors` 祖先行半选重算；`get_checked()` 取完全勾选节点；`check_all(bool)`。
  - `set_show_line(bool)`（showLine）：关闭后不画连接线（默认开）。
  - `set_only_icon_control(bool)`（onlyIconControl）：仅折叠箭头区可收缩，点击文本不再展开/收起。
  - `set_accordion(bool)`（accordion）：展开节点时自动收起同父级兄弟。
  - `set_is_jump(bool)`（isJump）：点击带 href 节点回调（原生新窗口语义，演示以状态文本呈现跳转目标）。
  - `set_editable({Add,Update,Del})`（edit）：行尾画操作图标（⊕/△/✕），点击触发 `set_on_edit(idx, act)` 回调。
  - 新增 `set_node_id/set_node_href/set_node_disabled`；`depth_of_` 记录节点深度供缩进/连接线定位。
- **测试页重写**（layui_test/src/pages/tree_page.cpp）：1:1 镜像 tree.html 全部 8 个 demo —— 基本演示（复选框+跳转+获取/勾选/重载）/ 常规用法 / 无连接线风格 / 仅节点左侧图标控制收缩 / 手风琴模式 / 点击节点新窗口跳转 / 开启复选框 / 开启节点操作图标。数据结构镜像原 HTML 五级 data 与 data1/data2（江西·广西·陕西、早餐·午餐·晚餐等）。

回归清单（须保证）：
- Tree 基本展开/折叠、点击节点回调、hover 高亮不破坏。
- 复选框勾选传播到子孙、父级半选/全选重算正确；getChecked/setChecked/checkAll 可用。
- 手风琴模式展开互斥；仅图标控制下点击文本不收缩。
- 操作图标点击触发 on_edit。
- `ui_ui` 库构建通过（tree.h/cpp 编译链接）。

验证：
- layui_test x64 ninja 构建通过，0 错误 0 警告（tree_page.cpp + tree.cpp 编译，ui_ui.lib + layui_test.exe 链接成功）。
- `layui_test.exe` 运行正常（52 pages 加载，GL3.3 初始化），无 stderr 报错。
- 根因归属：ui 库 Tree 缺能力（复选框/节点数据/手风琴/仅图标/跳转/无连接线/操作图标）→ 已在 ui 库补齐；测试程序仅编排与验证，无绕过。
- 已知限制：isJump 原生无法真正打开浏览器新窗口，demo 以状态文本呈现跳转目标，交互语义与原 HTML 一致。

## UI-086: 颜色选择器 ColorPicker 增强 + 1:1 镜像 colorpicker.html 全部 demo（2026-08-18）

严重度：功能补全（原颜色选择器仅「内嵌色板 + 预设 + hex 显示」，未实现 layui colorpicker 的 hex/rgb/rgba 格式、透明度滑块、预定义色可自定义、尺寸 lg/sm/xs、done（确认/清除）回调等能力）。

改动归属：ui 库（ColorPicker 组件缺能力）+ 测试程序编排。根因归属 ui 库能力缺陷，先在 ui 库补齐，再在测试程序 1:1 编排，无绕过。
- **ColorPicker 增强**（ui/include/ui/ui/colorpicker.h + ui/src/colorpicker.cpp）：
  - `set_format(ColorFormat::Hex/Rgb)`（layui format：hex/rgb；rgb 开启 alpha 时自动显示 rgba）。
  - `set_alpha(bool)`（layui alpha）：面板显示透明度滑块（0..1）。
  - `set_predefine(bool)` + `set_colors(vector<Vec4>)`（layui predefine/colors）：底部预定义色块行，默认 layui 经典色板，可自定义。
  - `set_size(ColorPickerSize::Lg/Sm/Xs)`（layui size）：SV 色场尺寸自适应。
  - `set_on_done(cb)`（layui done）：点击「确认/清除」按钮触发；`set_on_change`（layui change）取色中触发。
  - `set_clearable(bool)`：清除按钮显示开关（默认开），清除重置为 #000 并触发 done。
  - 布局升级为 layui 面板结构：SV 色场（饱和度×明度网格）+ 色相滑块 + 透明度滑块（可选）+ 当前色显示框与格式文本（hex/rgb/rgba）+ 预定义色 + 确认/清除按钮；鼠标拖拽 SV/滑块实时 change。
  - HSV↔RGB 转换；SV 色场命中/拖拽、色相/透明度滑块拖拽（mouse capture）、预定义色点击、按钮 hover/点击。
  - 旧 API 兼容：`set_font`/`set_theme`/`set_color`/`color()`/`set_on_change` 不变。
- **测试页重写**（layui_test/src/pages/colorpicker_page.cpp）：1:1 镜像 colorpicker.html 全部 7 组 demo —— 常规使用（无默认色 + #2ec770 配 done 回调）/ 表单赋值（#1c97f5 + done 写回输入框）/ RGB·RGBA 色值（rgb(68,66,66) + rgba(68,66,66,0.5) alpha）/ 透明度选择（#009688·rgba·无初始色）/ 预定义颜色项（默认 + 自定义 3 色）/ 全功能和回调（format rgb + predefine + alpha + change + done）/ 颜色框尺寸（lg·sm·xs）。

回归清单（须保证）：
- 基本取色（SV 色场/色相滑块/预定义色点击）实时触发 change，颜色正确。
- hex / rgb / rgba 格式显示正确（alpha 关闭时 rgb 显示，开启时 rgba）。
- 透明度滑块可调 alpha 且 change 回调；
- 确认/清除触发 done 回调；清除重置为 #000。
- predefine 开/关、自定义 colors 生效；尺寸 lg/sm/xs 影响 SV 色场大小。
- `apps/main.cpp`、`apps/api_demo.cpp` 旧调用仍可编译（API 向后兼容）。
- `ui_ui` 库构建通过（colorpicker.h/cpp 编译链接）。

验证：
- layui_test x64 ninja 构建通过，0 错误 0 警告（colorpicker.cpp + colorpicker_page.cpp 编译，ui_ui.lib + layui_test.exe 链接成功）。
- `layui_test.exe` 运行正常（52 pages 加载，GL3.3 初始化），无 stderr 报错。
- 根因归属：ui 库 ColorPicker 缺格式/透明度/预定义/尺寸/done 能力 → 已在 ui 库补齐；测试程序仅编排与验证，无绕过。
- 已知限制：格式文本为只读展示（未接入文本编辑输入），取色/滑块/预定义/确认/清除交互与原 layui 语义一致。

## UI-087: 滑块 Slider 增强 + 1:1 镜像 slider.html 全部 18 个 demo（2026-08-18）

严重度：功能补全（原 Slider 仅单值水平拖拽 + 键盘微调，未实现 layui slider 的 range 范围双环、step 步长、showstep 间断点、tips/setTips 提示文本、input 输入框、type:vertical 垂直、disabled 禁用、自定义 theme 颜色、setValue（set_range_value/set_value）实例赋值等能力）。

改动归属：ui 库（Slider 组件缺能力）+ 测试程序编排。根因归属 ui 库能力缺陷，先在 ui 库补齐，再在测试程序 1:1 编排，无绕过。
- **Slider 增强**（ui/include/ui/ui/slider.h + ui/src/slider.cpp）：
  - `set_range_enabled(bool)`（layui range）：范围拖拽双环；`value_/value2_` 两端；`set_range_value(start,end)` 与 range 回调 `set_on_range_change(a,b)`；拖拽/键盘两端互不穿越。
  - `set_step(float)`（layui step）：拖动/键盘值按步长取整；`set_showstep(bool)`（showstep）沿轨道画间断点线段。
  - `set_tips(bool)`（tips）+ `set_tips_callback(fn)`（setTips）：hover/拖拽时于手柄旁绘制黑色圆角提示条，默认显示整数值、可自定义文本；tips:false 关闭。
  - `set_input(bool)`（input）：右侧数值输入框（数值 + 分隔线 + 上/下三角箭头，点击改值）；range 模式回退到普通显示；vertical 时输入框位于轨道右侧。
  - `set_vertical(bool)`（type:vertical）+ `set_track_height(h)`/实例高度：垂直轨道、下→上为值增、提示条在左。
  - `set_disabled(bool)`（disabled）：禁用拖拽/键盘，整体灰化。
  - `set_theme_color(Vec4)`（theme）：自定义主题填充色（默认 #009688）。
  - 旧 API 兼容：`set_range`/`set_value`/`set_value_silent`/`set_on_change`/`value()`/`dragging()` 不变；默认值改为 layui 语义 min0/max100/value0（既有调用均已显式 set_range/set_value）。
- **测试页重写**（layui_test/src/pages/slider_page.cpp）：1:1 镜像 slider.html 全部 18 个 demo —— 基本滑块 / 定义初始值（20）/ 最大最小值（20~50）/ 步长（10 与 10+间断点）/ 提示文本（20~1000 自定义 GB 后缀、tips:false+change 回显）/ 输入框 / 范围选择（value40 range、[30,60] range + 开始结尾回显）/ 垂直滑块（默认、value30、range[30,70]、input+value80）/ 自定义颜色（#1E9FFF、#5FB878+value50、#FF5722 range[30,70]）/ 禁用滑块（value35）。

回归清单（须保证）：
- 单值水平/垂直拖拽、点击轨道、键盘方向键微调、Home/End 不破坏。
- range 模式两端吸附步长、互不穿越、change 回调（单值返回 value，范围返回两端）。
- step 取整正确；showstep 间断点绘制；tips 提示（默认值 / setTips 自定义 / 关闭）。
- input 输入框显示数值、上下箭头改值；vertical 布局正常。
- disabled 禁用拖拽 + 灰化；theme 自定义色生效。
- `apps/main.cpp`、`apps/api_demo.cpp` 既有 Slider 调用仍可编译（API 向后兼容）。
- `ui_ui` 库构建通过（slider.h/cpp 编译链接）。

验证：
- layui_test x64 ninja 构建通过，0 错误 0 警告（slider.cpp + slider_page.cpp 编译，ui_ui.lib + layui_test.exe 链接成功）。
- `layui_test.exe` 运行正常（52 pages 加载，GL3.3 初始化），无 stderr 报错。
- 根因归属：ui 库 Slider 缺 range/step/showstep/tips/input/vertical/disabled/theme/setValue 能力 → 已在 ui 库补齐；测试程序仅编排与验证，无绕过。
- 已知限制：输入框以数值 + 上下箭头实现，未接入可编辑 TextBox；交互语义与原 layui 一致。

## UI-088: 评分 Rate 增强 + 1:1 镜像 rate.html 全部 14 个 demo（2026-08-18）

严重度：功能补全（原 Rate 仅固定 5 星整数取值 + readonly 只读，未实现 layui-rate 的 length 星数、half 半星、text 文本 / setText 回调、theme 自定义主题色等能力）。

改动归属：ui 库（Rate 组件缺能力）+ 测试程序编排。根因归属 ui 库能力缺陷，先在 ui 库补齐，再在测试程序 1:1 编排，无绕过。
- **Rate 增强**（ui/include/ui/ui/rate.h + ui/src/rate.cpp）：
  - `set_value(float)`（把 int 升级为 float，兼容 int 实参）+ `value()` 返回整数（向后兼容）+ `value_f()` 返回浮点（含半星，吸附 0.5 步）。
  - `set_length(int)`（layui length）：自定义星数（默认 5）。
  - `set_half(bool)`（layui half）：允许 x.5 取值；绘制半星（scissor 裁剪左半实心 + 右半空心）。
  - `set_text(bool)`（layui text）+ `set_text_callback(fn)`（layui setText）：星后显示数值文本 / 自定义回调文本。
  - `set_theme_color(Vec4)`（layui theme）：自定义星级颜色（默认 #ffb800 warning）。
  - 旧 API 兼容：`set_readonly`/`set_theme`/`set_font`/`set_on_change` 不变；`set_value` 由 int 扩为 float（既有 int 实参隐式转换）；`value()` 仍返回 int。
- **测试页重写**（layui_test/src/pages/rate_page.cpp）：1:1 镜像 rate.html 全部 14 个 demo —— 基础效果 / 显示文字（value2 + text）/ 半星效果（value2.5 + half、value3.5 + half + text）/ 自定义内容（value3 + text + setText 极差~极好、value1.5 + half + text）/ 自定义长度（length3、length10 value8）/ 只读（value4）/ 自定义主题色（#FF8000 #009688 + #1E9FFF #2F4056 #FE0000 半星）。

回归清单（须保证）：
- 默认 5 星、hover 预览放大 scale(1.12)、点击取值（整数）不破坏。
- length 星数渲染与取值上限正确；half 半星取值/绘制正确。
- text 显示数值；setText 自定义文本回调生效。
- theme 自定义色生效；readonly 只读不可交互。
- `apps/main.cpp`、`apps/api_demo.cpp` 既有 Rate 调用仍可编译（set_value/value/readonly API 向后兼容）。
- `ui_ui` 库构建通过（rate.h/cpp 编译链接）。

验证：
- layui_test x64 ninja 构建通过，0 错误 0 警告（rate.cpp + rate_page.cpp 编译，ui_ui.lib + layui_test.exe 链接成功）。
- `layui_test.exe` 运行正常（52 pages 加载，GL3.3 初始化），无 stderr 报错。
- 根因归属：ui 库 Rate 缺 length/half/text/setText/theme 能力 → 已在 ui 库补齐；测试程序仅编排与验证，无绕过。
- 已知限制：on_change 回调仅传整数（半星交互时取整）；文本以星色渲染、字号与星星尺寸一致。

## UI-089: 轮播 Carousel 增强 + 1:1 镜像 carousel.html 三个 demo 组（2026-08-18）

严重度：功能补全（原 Carousel 仅纯色底 + 标题的瞬时左右切换，未实现 layui-carousel 的 anim 动画类型 default/updown/fade、arrow 箭头状态 hover/always/none、indicator 指示器位置 inside/outside/none、autoplay+interval 毫秒、图片填充页、运行时 reload 重配尺寸/参数等能力）。

改动归属：ui 库（Carousel 组件缺能力）+ 测试程序编排。根因归属 ui 库能力缺陷，先在 ui 库补齐，再在测试程序 1:1 编排，无绕过。
- **Carousel 增强**（ui/include/ui/ui/carousel.h + ui/src/carousel.cpp）：
  - 新增枚举 `CarouselAnim`(Default/Updown/Fade)、`CarouselArrow`(Hover/Always/None)、`CarouselIndicator`(Inside/Outside/None) 及对应 set_*，覆盖 layui anim/arrow/indicator。
  - `set_autoplay(bool)` + `set_interval(ms)`（layui autoplay/interval，默认 3000ms）；自动轮播与切页过渡计数统一由 `on_advance(dt)` 驱动（应用层 root->advance(dt) 即可，无需逐控件调用）。
  - 切页过渡动画：Default 左右推入 / Updown 上下推入 / Fade 渐隐渐显（alpha 混合 + scissor 裁剪）。
  - `add_image_slide(path)`（layui 填充图片页）+ `set_index(idx)` / `next()` / `prev()` 带过渡。
  - `set_size(w,h)` 运行时改尺寸（对应 layui reload width/height）。
  - on_mouse_move 维护 hover，arrow hover 态仅在悬停显示；indicator outside 指示点下移到容器外一行。
  - 旧 API 兼容：`set_auto_play(on, seconds)`（秒）与 `advance(dt)` 保留为封装（映射到 set_autoplay/set_interval/on_advance）；`add_slide`/`count`/`current`/`set_on_change` 不变。
- **测试页重写**（layui_test/src/pages/carousel_page.cpp）：1:1 镜像 carousel.html 三个 demo 组 —— 常规轮播（test1 5 页 arrow always、test2 2 页 fade 动画 + interval 1800）/ 设定各种参数（test3：左/上/渐显动画、悬停/始终/无箭头、内/外/无指示器、自动切换开关、时间间隔 ±1000ms 交互调节）/ 填充轮播元素-以图片为例（test10：778x440 7 张图片 interval 5000）。
- **构建资产**：layui_test/CMakeLists.txt POST_BUILD 追加拷贝 demo 图片 1.png~7.png 到可执行目录（配合 add_image_slide 的 CWD/exe 回退加载）。

回归清单（须保证）：
- 纯色页 + 箭头/指示点到点击切换、on_change 回调、自动轮播计时不破坏。
- anim Default/Updown/Fade 三动画过渡正确（fade 用 alpha 混合 + scissor）。
- arrow Hover（悬停显示）/Always/None、indicator Inside/Outside/None 渲染与点击区域正确。
- autoplay 开关 + interval 毫秒计时生效；set_size 运行时改尺寸生效。
- 图片页 add_image_slide 加载与绘制正常（POST_BUILD 拷贝 + CWD/exe 回退）。
- `apps/main.cpp`、`apps/api_demo.cpp` 既有 Carousel 调用（set_auto_play/advance/add_slide）仍可编译（旧 API 向后兼容）。
- `ui_ui` 库构建通过（carousel.h/cpp 编译链接）。

验证：
- layui_test x64 ninja 构建通过，0 错误 0 警告（carousel.cpp + carousel_page.cpp 编译，ui_ui.lib + layui_test.exe 链接成功）；POST_BUILD 拷贝 7 张 demo 图片成功。
- `layui_test.exe` 运行正常（52 pages 加载，GL3.3 初始化），无 stderr 报错（含图片加载）。
- apps 侧 `opengl_ui_demo` 构建链接通过（set_auto_play/advance 兼容）。
- 根因归属：ui 库 Carousel 缺 anim/arrow/indicator/autoplay/interval/图片/reload 能力 → 已在 ui 库补齐；测试程序仅编排与验证，无绕过。
- 已知限制：on_change 在过渡完成时触发一次；fade 动画用 alpha 混合，指示点/页码 over-draw 于其上。

## UI-090: 轮播页布局修正 + 图片改用 file/img（2026-08-18）

严重度：测试页布局错误 + 图片加载源错误。属测试程序问题，未改 ui 库。
- 根因①：carousel_page.cpp 的 build_demo2（设定各种参数）把标签/按钮/开关以绝对坐标 add_child 到 host.content，但 ScrollBox::on_layout 会按 preferred_height 重排每个直接子节点，绝对坐标被覆盖导致控件重叠错位。
- 修复①：每行（标签+按钮 或 标签+开关+按钮）包进 ui::HBox 横向排布再挂到 content，由 ScrollBox 竖直堆叠。
- 根因②：demo3 图片轮播使用 3rdparty layui demo 的 1.png~7.png，且 Texture::load_from_file 无 CWD/exe 回退，与仓库 file/img 图片目录不符。
- 修复②：图片改从 file/img 取（01.jpg/05.png/08.png/A_01.jpg/back.png/cs.png/log.jpg），POST_BUILD copy_if_different 以 ASCII 名 a01~a07 拷到 exe 目录，add_image_slide 按裸文件名加载。
- 验证：layui_test x64 ninja 构建通过（0 错 0 警），7 张 a0*.jpg/png 拷贝成功，exe 运行不崩溃。测试程序整改，ui 库无改动。

## UI-091: Carousel add_image_slide 缺 exe 目录回退（2026-08-18）

严重度：功能缺陷。属 ui 库缺陷（carousel.cpp），已按工作流在 ui 库内修正，非测试程序绕过。
- 根因：Carousel::add_image_slide(path) 直接调用 Texture::load_from_file(path)，而 Texture::load_from_file 无 CWD/exe 目录回退（[UI-048](#ui-048-image-资产定位依赖-cwd--加载噪音错误2026-08-16) 已为 Image::load_from_file 和 Navbar 实现了该回退，carousel 遗漏）。
- 修复：carousel.cpp 的 dd_image_slide 增加与 Image::load_from_file 一致的 exe 目录回退（is_bare_filename / xe_directory / ile_readable 三步尝试）；加载失败时输出 log_error 噪音。
- 验证：x64 ninja 构建通过，exe 运行无 carousel/image 加载错误。

## UI-092: 模板引擎 laytpl 增强 + 1:1 镜像 laytpl.html 沙箱（2026-08-18）

严重度：功能缺失。原 template_page 只是静态新闻列表（手动拼 Label），未实现模板引擎；镜像站 laytpl.html 是「数据+模版→视图」的实时沙箱。属 ui 库能力缺陷，按工作流新建 ui::LayTpl 引擎。
- **ui 库新增引擎**（ui/include/ui/ui/laytpl.h + ui/src/laytpl.cpp，CMakeLists 追加）：
  - LayTplValue：数据节点（对象/数组/标量），laytpl_parse_json() 从 JSON 文本解析。
  - LayTpl tpl(str) 编译模板，
ender(data) 渲染；config(open,close) 自定义分隔符（默认 {{ }}）。
  - 语法：{{ d.field }} 输出（不转义）、{{= d.field }} 输出（转义 html）、{{# JS 语句 }} 逻辑（if/else、layui.each 循环）、{{! 区域 !}} 过滤不解析。
  - 表达式求值支持：成员访问 .key / 数组下标 [i] / .length / 逻辑 || && / 比较 === == !== != / 算术 + - * / % / 字符串与数字字面量 / 短路真值。
- **测试页重写**（layui_test/src/pages/template_page.cpp）：1:1 镜像 laytpl.html —— 左侧数据（JSON）文本框 + 右侧模版文本框，下方视图结果；任意编辑实时重渲染（TextArea.onChange）。默认数据/模版与官方一致（Layui常用模块 + 5 条 list）。
- **工程**：ui/CMakeLists.txt 追加 src/laytpl.cpp 进 ui_ui 库。

回归清单（必须保证）：
- 官方默认数据+模版渲染结果与 laytpl.html 一致（h3 标题 / each 逐条 / site 缺省为空 / if 空列表）。
- {{= }} 转义、{{! !}} 过滤、自定义分隔符 <% %> 均生效。
- 两块 textarea 实时编辑实时刷新视图，无 JS 运行时依赖（纯 C++ 引擎）。
- ui_ui 库构建通过，layui_test 链接通过，运行无报错。

验证：独立单测（编译链接 ui_ui.lib 直跑）确认 each/if-else/转义/过滤/自定义分隔符全部正确；页面实时沙箱构建运行正常。

## UI-093: 流加载页 1:1 克隆镜像 3 演示 + ui 库图片懒加载（2026-08-18）

严重度：功能缺失。原 flow_page 只有 2 个演示且结构不同：无独立可滚容器（用整页 host.content 触底）、无 isAuto:false 自动生成的「加载更多」按钮、完全缺失镜像第三演示「图片懒加载」。属 ui 库能力缺陷（Image 无懒加载），按工作流在 ui 库补齐。

- **ui 库新增图片懒加载（lay-src 语义）**：
  - `Image::set_lay_src(path)`：不立即解码，on_draw 时用 `Renderer::current_scissor()`（当前可视屏）检测 bounds 进入可视区才 `load_from_file`，只加载一次 —— 对应 layui `flow.lazyimg` / `isLazyimg`。
  - `Renderer::current_scissor()`：返回 scissor 栈顶（未 push 时返回整视口），即「当前屏」矩形，供 LazyImage 判可视（无论上滑/下滑只加载当前屏图）。
  - `Image::preferred_height()`：显式指定显示尺寸（has_display_size_）时加载前也预留高度，保证懒加载图片列表布局稳定不塌陷。
- **测试页重写**（layui_test/src/pages/flow_page.cpp）：1:1 镜像 flow.html 三个演示 ——
  ① 信息流-滚动加载：独立 600×400 ScrollBox（嵌套容器自身滚动条，scrollElem 语义），`set_on_scroll_to_bottom()` 滚到底自动加载 8条/页 × 10 页（80 条灰底居中编号瓦片），末页贴「没有更多了」并停用；
  ② 信息流-手工加载：独立 600×300 ScrollBox，isAuto:false → 列表 FlowBox 末尾**自动生成「加载更多」按钮**，点按 6 懒加载图/页 × 6 页；
  ③ 图片懒加载：独立 600×300 ScrollBox 内 15 张 `set_lay_src` 图，仅加载当前屏。
- **工程**：gl/include/ui/gl/renderer.h(cpp) 增 `current_scissor()`；ui/src/image.cpp(h) 增懒加载。

回归清单（必须保证）：
- 三个演示均在各自独立 ScrollBox（自身滚动条）内滚动触发，不依赖整页 host.content 滚到底。
- 演示一：滚动到底自动追加 8 瓦片/页，共 10 页 80 条，末页「没有更多了」并停止自动加载（UI-035 回调语义）。
- 演示二：列表末尾自动生成「加载更多」按钮（isAuto:false）；点击追加 6 图/页共 6 页；按钮 bring_to_front 保持在列表底部；末页禁用改「没有更多了」。
- 演示三：15 张 lay-src 图，仅进入当前可视屏才解码（懒加载），滚动上/下按需取图。
- 懒加载图片不即时解码：未进入可视屏前 `loaded()==false` 且不产生纹理；显式尺寸时列表高度稳定。
- ui_ui 库 + layui_test 构建通过，运行无报错。

验证：ui_ui 库与 layui_test 编译链接通过（flow_page 重写后 3 演示）；exe 运行无报错；懒加载 on_draw 用 current_scissor 判可视，滚动到可滚容器内触发解码。

## UI-094: 流加载滚动加载修复：ScrollBox 触底回调加 mb 临界距离（2026-08-18）

严重度：功能缺陷。「信息流 - 滚动加载」演示未真正生效的根因：镜像 flow.html 的 `flow.load`
默认 `mb:50`（距内容底部 50px 即预加载下一页）；而 ui 库 `ScrollBox::check_bottom` 要求严格滚到底
（`scroll_offset >= scroll_max - 1`）才触发。演示一首屏只加载 1 页（8 条 → 2 列 4 行 × 100px ≈ 内容仅比
400px 可视区高 ~20px），`scroll_max` 极小，滚动步长一次即装顶，严格的触底判定几乎无法满足——
「滚动加载」形同未实现。属 ui 库缺陷，按工作流修正 ui 库，测试页只做配置。

- **ui 库新增**（scrollbox.h/cpp）：`ScrollBox::set_bottom_margin(mb)` —— 距内容底部不足 mb 像素即触发
  触底回调（`trigger = scroll_max - bottom_margin_`；镜像 layui flow.mb 语义）。`bottom_margin_` 默认 0
  → 退化为严格滚到底，**不破坏 UI-035 既有回归语义**。触发/重新武装（`bottom_fired_`）逻辑与
  `set_on_scroll_to_bottom` 完全一致。
- **测试页**（flow_page.cpp）演示一：
  - `box->set_bottom_margin(50.f)`：滚动接近底部即自动加载下一页（8 条/页 × 10 页），末页「没有更多了」并停用。
  - **初始加载 2 页填满首屏并留明显滚动余量**：镜像 layui flow.load 首次 done 后会自动续载直到内容超高。
    首屏若仅 1 页（8 条=4 行 × 100px≈420px vs 可视 400px），`scroll_max` 仅 ~20px、滚动条滑块占 95% 高度
    —— 乍看「无滚动条」，且 20px 滚动量无感，滚动加载失效。加载 2 页后 `scroll_max` ~420px、滑块 ~48% 明显可见并可拖。

回归清单（必须保证）：
- 镜像 flow 语义：距底部 50px 内即预加载下一页，无需严格滚到底。
- UI-035 回归：未设 `set_bottom_margin` 的既有 ScrollBox 触底回调仍为「严格滚到底」（mb 默认 0）。
- 演示一容器有明显的垂直滚动条（scroll_max 足够大、滑块占比低）、滚动有实际余量，滚到底（或距底 50px）自动追加下一页。
- 演示一滚动到底自动追加，末页显示「没有更多了」并停止；离开底部再滚回会重新武装（bottom_fired_）。
- ui_ui 库 + layui_test 构建通过，运行无报错。

验证：ui_ui 库与 layui_test 编译链接通过（scrollbox + flow_page）；exe 运行无报错；
演示一初始加载 2 页使滚动条明显可见、可拖，配合 `set_bottom_margin(50)` 滚动接近底部即触发下一页加载。

## UI-095: 流加载页回调捕获栈局部 PageHost 悬空崩溃（2026-08-18）

严重度：崩溃（访问冲突 0xC0000005，读 0x294）。重写 flow_page 三个演示时，演示一「滚动加载」的
滚动到底回调与 `append_page` lambda 捕获了 `PageHost& h = host`（引用 app.cpp `render_page` 的**栈局部**
`PageHost host`）。`build()` 返回后该栈帧销毁，运行期滚动回调触发时解引用悬空 `h.theme/h.font`
（`0x294` 恰为 Theme/Font 字段偏移）→ 访问冲突。属测试程序缺陷（违反既有约定：layer_page.cpp 已注释
「不得捕获栈局部 host，只捕获 app 长期对象」），按工作流修正测试页。

- **测试页修复**（flow_page.cpp 演示一）：仅捕获 app 长期对象指针 `ui::Font* app_font = host.font;`
  `const ui::Theme* app_theme = host.theme;`（指向应用级 `&font`/`&theme`），`append_page` 与滚动到底
  回调均改为按值捕获这两个指针，不再捕获 `&host`。演示二/三回调不触碰 host，无需改。

回归清单（必须保证）：
- 所有页面回调（scroll/click/destroy）只捕获 app 长期对象（font/theme/preview 等 new 控件），
  禁止捕获 build 期栈局部 PageHost（layer_page.cpp 既有约定）。
- 流加载页运行期滚动到底触发续载，不再访问冲突崩溃；演示一滚到底自动追加、末页「没有更多了」。
- ui_ui 库 + layui_test 构建通过；exe 运行无报错、无崩溃。

验证：修复后 layui_test 编译链接通过，exe 持续运行不崩溃（6s 存活无访问冲突）。
`0x294` 访问冲突消除——回调期解引用的 font/theme 为 app 长期对象而非悬空栈帧。

## UI-096: 文本行修饰页改克隆 layui-code 代码块组件 demo（2026-08-18）

**修正（克隆对象错误）**：`pages/text_page.cpp`（order 45「文本行修饰」）此前克隆了镜像站点
`layui` **不存在的** text.html 的文本排版语义（标题/引用/列表/链接）。核对镜像 demo 侧栏
「组件示例 → 文本行修饰」实际指向 `demo/code.html`，其内容是 **layui-code 代码块组件** 的
演示（4 个 demo：默认修饰 / notepad 风格 / 叠加使用 / 固定高度）+ 结尾引用。text_page 克隆对象
整体选错，属**测试程序缺陷**；而实现这些 demo 暴露的 ui 库能力缺口按工作流**先补 ui 库**。

- **ui 库能力补齐（`code.h/cpp`）**：`ui::Code` 新增
  - `set_title(const char*)`：标题条左侧文本（lay-title）；
  - `set_skin(Skin)`（`kDefault`/`kNotepad`）：notepad 风格配色；
  - `set_max_height(float)`：固定最大高度（lay-height），内容超高时内部滚轮滚动，
    0 表示自适应内容高度；
  - `preferred_height()`：按标题条 + 行数（+ 子 Code）自适应内容高度，受 `max_height_` 封顶；
  - `on_layout()`：将子 Code 顺序排布在代码行之下（宽度让出两侧内边距）；
  - `draw()`：标题条下剪裁区先画自身代码行，再遍历绘制子 Code（嵌套叠加使用）。
- **测试页修正（`pages/text_page.cpp`）**：重写为 layui-code 组件 demo——
  4 个小节（FieldsetTitle「默认修饰 / notepad 风格 / 叠加使用 / 固定高度」）+ 4 个 Code 块
  （默认无标题、notepad 皮肤+JavaScript 标题、默认/notepad 两类嵌套、固定 150px 超高滚动），
  结尾 BlockQuote 引用。

回归清单（必须保证）：
- `ui::Code` 未显式设置 max_height 时按标题条 + 行数自适应内容高度；设 max_height 后超高滚动，
  `preferred_height()` 返回 min(内容高, max_height)。
- 嵌套 Code（叠加使用 demo）作为父 Code 子节点布局/绘制在代码行之下，切页释放无悬空。
- ui_ui 库 + layui_test 构建通过；exe 运行无报错、无崩溃。

验证：ui_ui + layui_test x64 Release 编译链接通过，无警告无报错；`--smoke 6` exit 0
（52 页逐页切换释放树无崩溃），`--shot --shot-page 45` 截图正常渲染 4 个 Code 块。

## UI-097: 项目打包分发机制（2026-08-19）

新增完整的打包分发基础设施，支持三种消费方式：CMake install/export、vcpkg overlay port、Conan 包。

### CMake install/export

- 各层 CMakeLists.txt（core/gl/ui/media）添加 `install(TARGETS ... EXPORT opengl_uiTargets)` + 头文件安装，使用 `$<BUILD_INTERFACE>` / `$<INSTALL_INTERFACE>` 生成器表达式隔离构建树与安装树路径。
- 顶层生成 `opengl_uiConfig.cmake` + `opengl_uiConfigVersion.cmake`（SameMajorVersion），导出命名空间 `opengl_ui::ui_core` / `opengl_ui::ui_gl` / `opengl_ui::ui_ui`，可选 `opengl_ui::ui_media_ffmpeg` / `opengl_ui::ui_media_opencv`。
- Config.cmake 内自动 `find_dependency(glfw3)` + `find_dependency(OpenGL)`，可选组件按编译时配置决定；FFmpeg 模块附带 `FindFFmpeg.cmake` 到安装目录。
- 新增 `OPENGUI_BUILD_APPS` 选项（默认 ON），打包时设 OFF 跳过 apps 构建。
- 安装树：`lib/*.lib`、`lib/cmake/opengl_ui/`、`include/ui/`、`include/glad/ KHR/ stb_*.h`、`share/opengl_ui/iconfont.ttf`。

### 包管理器集成

- `vcpkg.json`：overlay port 清单（glfw3、glad、stb 核心依赖；ffmpeg、opencv 可选 features）。
- `portfile.cmake`：vcpkg 构建脚本（`vcpkg_from_github` + `vcpkg_cmake_configure/install` + `vcpkg_cmake_config_fixup`）。
- `conanfile.py`：Conan 2.x 配方，组件化导出（core/gl/ui/ffmpeg/opencv），支持 shared/static。

### CMakePresets.json

顶层新增多架构预设：`x64`、`x64-debug`、`x64-ffmpeg`、`x64-opencv`、`x64-all`、`install`、`install-all`。

### layui_test 双模式

`LAYUI_TEST_USE_FIND_PACKAGE` 选项（默认 OFF）：ON 时通过 `find_package(opengl_ui)` 从安装目录消费；OFF 时沿用 `add_subdirectory` 源码引用。

回归清单（必须保证）：
- `add_subdirectory` 源码引用模式（默认）：layui_test + apps 正常构建运行，行为不变。
- `find_package` 模式：`CMAKE_PREFIX_PATH=<install>` 时 Config.cmake 正确解析依赖、导入 targets。
- 各层头文件 include 路径在构建树与安装树均可正确解析（`ui/all.h`、`ui/media/video_view.h` 等）。
- GLAD/stb 头文件随包安装到 `include/`，下游无需单独获取。

验证：x64 Release configure + build 通过；`cmake --install` 安装树结构正确（3 个 .lib + Config/Targets + 头文件 + 资产）；生成的 `opengl_uiConfig.cmake` 正确展开模板变量、导出3个命名空间 target。

### 自包含打包（097-后续补丁）

将所有第三方运行时依赖打包进 install 目录，实现**单目录自包含分发**：

- GLFW：DLL + import lib + headers + cmake config 安装到 `install/bin/`、`lib/`、`include/GLFW/`、`lib/cmake/glfw3/`。
- FFmpeg（条件安装）：DLLs + import libs + headers 安装到 `bin/`、`lib/`、`include/libav*/`。
- OpenCV（条件安装）：DLL + import lib + headers + cmake config 维持 `x64/vc16/` 嵌套布局。
- Config.cmake 模板追加 `list(APPEND CMAKE_PREFIX_PATH "${PACKAGE_PREFIX_DIR}")`，使 `find_dependency(glfw3)` / `find_dependency(OpenCV)` 从本包 install prefix 自动定位，下游无需手动指定 `CMAKE_PREFIX_PATH`。

最终 install 目录结构：`bin/`（glfw3.dll + 可选 FFmpeg/OpenCV DLL）→ `lib/`（.lib + cmake/）→ `include/`（ui/ + GLFW/ + glad/ + stb + 可选 libav*/opencv2）→ `share/`（iconfont.ttf）。

验证：x64 Release configure + build + install 成功；install 目录自包含 GLFW DLL/import lib/cmake config；Config.cmake 正确展开 `PACKAGE_PREFIX_DIR` 指向 install 根。

## UI-098: 复选框/开关补齐 layui 全皮肤能力 + 复选框页 1:1 克隆（2026-08-21）

按 AGENTS.md「先归因再修复」工作流，针对 layui_test 复选框页 1:1 克隆镜像
（`3rdparty/src/layui-dev-docs/docs/2/form/checkbox.html`）暴露的 **ui 库能力缺陷**
逐项修正 ui 库（不在测试程序内绕过/打补丁），再重写测试页完成克隆。

### ui 库修正（根因归属：ui 库缺陷）

- **CheckBox 半选态（indeterminate）**：新增 `set_indeterminate(bool)/indeterminate()`，
  选中后自动清除半选（layui 行为）；绘制绿框白底 + 中间横线（对应 layui-icon-subtraction）。
- **CheckBox 选中对勾**：选中态方框内绘制白色对勾（layui-icon-ok）。
- **CheckBox 皮肤**：新增 `enum class Skin{Primary,Tag,None}` + `set_skin()/skin()`：
  - `Tag`（layui lay-skin=tag）：左侧 `#d2d2d2` 灰底白字标题块 + 右侧 30px 对勾方框，
    选中标题块/方框变绿 `#16b777`，禁用灰化（对齐 layui.css `.layui-form-checkbox[lay-skin=tag]`）。
  - `None`（layui lay-skin=none，2.9.8+）：不绘制默认方框，仅交互 + 自定义内容，
    供任意风格多选组件（卡片/标签等外部自主实现）。
- **CheckBox lay-checkbox 自定义内容绑定**：新增 `set_custom_content(Widget*)`，
  内容控件按皮肤摆放（None 铺满 / Primary 紧随方框 / Tag 填充标题块），
  拥有权归 CheckBox（析构释放，Dropdown 同款非子节点模式避免双重绘制）。
- **Switch 双态文本 `|` 分隔**：`set_text("ON|OFF")` 按 `|` 切分，
  开态显示左段、关态显示右段（对齐 layui form.js `!checked&&e[1]||e[0]` 行为）。
- **Switch lay-checkbox 自定义标题**：新增 `set_custom_content(Widget*)`（滑块右侧区域），
  供开关风格图标双态标题（月亮/太阳）。

### 测试页重写（`pages/checkbox_page.cpp`）

一比一克隆镜像 6 个 demo 区块：
1. 默认风格：默认 / 选中 / 禁用 / **真半选**（`set_indeterminate(true)`）。
2. 标签风格：`set_skin(Skin::Tag)` 默认/选中/禁用。
3. 开关风格：无字 / `ON|OFF`（选中）/ `开启|关闭` / 禁用。
4. 自定义标题模板（lay-checkbox）：`set_custom_content` 绑定 链接标题 / 心形图标+标题 / 月亮太阳双态。
5. 自定义任意风格（lay-skin=none）：卡片多选（Chrome/Edge/Firefox/IE，含选中/禁用/勾角）
   + 徽章标签多选（唱/跳/rap/篮球）。
6. 复选框事件：是否同意 + 状态标签 + 确认按钮弹窗展示字段（复刻 form.on checkbox/submit）。

自定义卡片/徽章外观为测试端自绘（对应镜像「外部自主实现」样式，非 layui 内置），
仅编排 ui 库能力，未绕过 ui 库。

回归清单（必须保证）：
- `CheckBox` Primary 皮肤样式与交互回归不变（对勾/半选/禁用）；tag 皮肤结构对齐 layui.css。
- `Switch` 文本/切换/禁用行为不变；`|` 双态文本开=左段关=右段。
- `set_custom_content` 内容控件析构释放无泄漏；切页释放树无悬空。
- layui_test 构建通过，exe 运行无报错、无崩溃；复选框页 6 区块渲染正常。

验证：ui_ui + layui_test x64 Release 编译链接通过，无警告无报错；
`cmake --build` 全量通过（含复选框页新自绘控件）；
`--smoke 6` exit 0（53 页逐页切换释放树无崩溃）；
`--shot-page 5`（复选框页）截图成功，像素色采样确认成功绿 #16b777（选中对勾/tag 选中块）、
tag 灰块 #d2d2d2、深灰导航侧栏均正确渲染。

## UI-099: 外壳垂直布局按新镜像重构 + 页面分组映射（2026-08-21）

按新镜像 `docs/2`（layui 文档站「文档中心」桌面版）重构 layui_test 外壳：
深色头部 + 白色 240px layui-menu 侧栏 + 页面分组/顺序/英文副标全部重映射到新镜像分组。

### ui 库增强（Menu，layui-menu 克隆）

- **`Menu::add_item_sub(title, subtitle, cb)`**：菜单项带灰色英文副标题（新镜像 docs/2 侧栏
  「中文主标题 + 灰色英文副标」样式，`layui-font-12 layui-font-gray`）；绘制紧随主标题、
  选中态同步变绿。
- **`Menu::clear()`**：清空全部项并复位层级/悬停/子面板（第二列子菜单按板块重建复用）。
- **`Menu::set_selected(group_index, item_index)`**：按（组序号, 组内子项序号）选中单项、
  清除其余项选中态（对齐 `NavMenu::set_selected` 语义，供切页同步外壳侧栏）。
- **`ScrollBox` 新增 `ws-menu` 类**：白底渲染（对齐新镜像 `.ws-menu{background-color:#fff}`），
  供白色侧栏/子菜单容器使用（原 `layui-side-scroll` 深灰仅 NavMenu 用）。

### 测试程序外壳重构（`layui_test/src/app.cpp`）

- 头部改为新镜像 `.ws-header` 克隆：深灰 `#2f363c` 底 + 底部 1px 分隔线；
  左 240px logo 图片区（`Image` 加载 logo.png，POST_BUILD 已拷 exe 目录）；
  右侧导航（左→右）＝ 组件(active 绿) / 笔记 / 生态(Dropdown) / 2.13.9(Dropdown) / 主题开关
  （`Switch` ON|OFF，占位交互）。
- 侧栏容器/主菜单由深灰 `NavMenu` 改为 **白色 `ui::Menu`**（宽 220→240，`ws-menu` 白底），
  分组 `add_group` + 项 `add_item_sub`；切页同步 `set_selected`。
- 第二列子菜单（数据表格板块）由 `NavMenu` 改 **白色 `ui::Menu`**（`clear`/`add_group`/
  `add_divider`/`add_item`，分组默认展开）。
- 内容区起点 `kSidebarW` 220→240，第二列可见时右移 160（`place_content` 逻辑不变）。

### 页面分组/顺序/英文副标重映射（`pages/*.cpp` 的 PageDef）

`PageDef` 新增 `subtitle` 字段（默认 nullptr，可空）。全部 31 个主导航页按新镜像 docs/2 侧栏
分组重新映射（group 连续、order 组内连续）：

| 新分组 | 页面（order: title / subtitle） |
| --- | --- |
| 布局 | 0 框架/Layout、1 栅格/Grid |
| 通用 | 10 按钮/Button、11 动画/Anim |
| 表单 | 20 表单/Form、21 复选框/Checkbox |
| 展示 | 30 静态表格/Table、31 数据表格/Table（板块锚点）、32 分页/Laypage、33 导航/面包屑/Nav、34 基础菜单/Menu、35 选项卡/Tabs、36 徽章/Badge、37 辅助元素/Blockquote / Fieldset / hr、38 面板/Panel / Card / Collapse、39 进度条/Progress、40 时间线/Timeline、41 树形组件/Tree、42 轮播/Carousel、43 流加载/Flow、44 文本行修饰/Code |
| 交互 | 50 弹出层/Layer、51 日期与时间选择/Laydate、52 文件上传/Upload、53 下拉菜单/Dropdown、54 颜色选择器/Colorpicker、55 穿梭框/Transfer、56 滑块/Slider、57 评分/Rate |
| 其他 | 60 模板引擎/Laytpl、61 工具模块/Util |

数据表格板块子页（order 100-163，board="数据表格"）保留双列侧栏机制，组归「展示」、
不进主导航，仅于选中板块锚点时在第二列列出。

回归清单（必须保证）：
- `Menu` 白底分组折叠/分割线/父级子面板/徽章/选中态回归不变；新增 subtitle/clear/set_selected 不破坏。
- 侧栏分组连续：按 order 排序后同组页连续，切页 `set_selected` 高亮正确。
- 数据表格板块：点击主导航「数据表格」显示白色第二列子菜单并渲染首个子页；切子页保持第二列。
- 深色头部导航顺序＝组件/笔记/生态/2.13.9/主题开关；组件 active 绿字。
- 内容区起点＝240（无子列）/ 400（有子列），无重叠无留白。

验证：ui_ui + layui_test x64 Release 编译链接通过；`--smoke 6` exit 0（53 页逐页切换无崩溃）；
`--shot-page 0` 截图像素采样确认：头部深灰 #2f363c、侧栏白底、组件绿字 active、
分组头/菜单项/英文副标齐全；`--shot-page`（数据表格锚点）确认第二列白底子菜单 + 内容区右移 400。

## 相关历史条目

- [UI-098](CHANGELOG.md#ui-098-复选框开关补齐-layui-全皮肤能力--复选框页-11-克隆2026-08-21)：复选框页 1:1 克隆，外壳重构不改其布局。
- [UI-097](CHANGELOG.md#ui-097-项目打包分发机制2026-08-19)：打包分发机制为本条 layui_test 消费方式基础。
- [UI-061](CHANGELOG.md#ui-061-menu-基础垂直菜单组件--menuhtml-克隆2026-08-17)：Menu 组件基线，本次新增 add_item_sub/clear/set_selected。


## UI-100: IME 候选窗跟随输入框光标（2026-08-21）

严重度：中（中文输入法候选窗落在窗口左上角/屏幕角落，无法定位到输入框，影响全部文本输入场景）。

根因归属：ui 库缺陷。opengl_ui 为 GLFW 自绘窗口，TextBox 光标是绘制的竖线而非 Win32 CARET，
系统输入法无从得知光标位置；库此前亦无任何 IME 定位接口（全库无 WM_IME*/Imm* 调用）。

修复：
- 新增 `ui/ui/ime.h` + `ui/src/ime.cpp`：`update_ime_caret(const Window& window, Widget* root)`
  供应用每帧渲染循环调用；递归找 root 子树内聚焦 TextBox，`bounds() + caret_offset()`
  得客户区坐标，`ClientToScreen` 换算后经 `ImmSetCompositionWindow(CFS_POINT)` 与
  `ImmSetCandidateWindow(CFS_CANDIDATEPOS)` 锚定组合窗/候选窗；屏幕坐标未变化不重复设置
  （不打断用户拖动候选窗），窗口移动后自动跟随；非 Windows 平台空操作；ui_ui 仅 WIN32 链 imm32。
- `TextBox::caret_offset()` private -> public（IME 锚定等外部查询用途）。
- `all.h` 聚合头加入 ime.h。

回归清单（必须保证）：
- TextBox 输入行为不变：字符输入/退格/Delete/左右移/Home/End/点击定位/占位符/密码掩码。
- 聚焦非 TextBox 或无聚焦时不干预 IME（保持系统默认行为）。
- 屏幕坐标不变时每帧调用零副作用（不重设 IMM）。
- 非 Windows 平台编译不受影响（实现全部位于 _WIN32 分支内）。

验证：ui_ui x64 Release 编译通过并 install 至 prefix；下游 voice_assistant 接入
`ui::update_ime_caret` 后构建通过、CLI 回归全 PASS；候选窗跟随效果属人工验证项
（中文输入法聚焦输入框实测）。

### 相关历史条目

- [UI-008](CHANGELOG.md#ui-008-m4-文本输入textbox--字符通道--scissor-裁剪2026-08-09)：TextBox 单行输入框基线（本次公开 caret_offset 并新增 IME 锚定）。

### 勘误（2026-08-21 同日）

首版实现误将 `ptCurrentPos` 经 `ClientToScreen` 换算为屏幕坐标后传入——
`CFS_POINT` / `CFS_CANDIDATEPOS` 的 `ptCurrentPos` 按 MSDN 定义为**窗口客户区坐标**，
换算导致候选窗落「窗口原点 + 屏幕坐标」处（常在屏幕外），等效失效。
已修正：直接传客户区坐标；y 取控件底边（候选窗弹在输入框下方）；
`last` 缓存改存客户区坐标（窗口移动时系统自动跟随，无需重设）。

## UI-101: TextBox 选择/剪贴板能力（拖选/Shift 选择/Ctrl+A/C/X/V）（2026-08-21）

严重度：中（输入框无法选中、复制、粘贴、全选，所有文本编辑场景受限）。

根因归属：ui 库能力缺失。TextBox::on_key_down 忽略 mods，无选择状态模型，
无剪贴板集成；鼠标仅点击定位无拖选。

修复：
- 选择模型：`sel_anchor_` 锚点 + `caret_` 构成选中区间；Shift+Left/Right/Home/End 扩展选择；
  无 Shift 方向键落到区间端点并清除选择。
- 鼠标拖选：on_mouse_down 锚定并自动捕获，捕获期间 on_mouse_move 扩展选择（hit_test_caret
  与 on_draw 同滚动量），on_mouse_up 结束；选中区间 accent 半透明高亮（scissor 内文本下层）。
- 剪贴板：Window 新增 `static primary_handle()`（最近创建窗口句柄）；Ctrl+C 复制原文区间、
  Ctrl+X 剪切、Ctrl+V 粘贴（单行框换行折叠为空格）、Ctrl+A 全选。
- Backspace/Delete/输入字符优先删除选中区间；set_text 后锚点 clamp。

回归清单（必须保证）：
- 原有行为不变：字符输入/退格/Delete/Home/End/Enter 回调/占位符/密码掩码/超宽滚动/IME 锚定。
- 拖选方向任意（左拖右拖）；拖选出界仍持续更新（鼠标捕获）。
- 复制内容为原文（密码框亦复制原文）；粘贴含换行折叠为空格不破坏单行语义。
- Ctrl 组合不产生字符输入冲突（GLFW ctrl 组合无 char 事件）。

验证：ui_ui/ui_gl Release 编译 + install 通过；下游 voice_assistant 构建通过、CLI 回归全 PASS；
人工验证项：规则页输入框拖选高亮 / Ctrl+A 全选 / Ctrl+C 粘贴到外部 / 外部复制 Ctrl+V 粘入。

### 相关历史条目

- [UI-008](CHANGELOG.md#ui-008-m4-文本输入textbox--字符通道--scissor-裁剪2026-08-09)：TextBox 基线（本次补齐选择/剪贴板）。
- [UI-100](CHANGELOG.md#ui-100-ime-候选窗跟随输入框光标2026-08-21)：IME 锚定复用 caret_offset 与 hit_test 同源滚动逻辑。

## UI-102: 键盘修饰键快照——SendInput 粘贴到自绘输入框丢失 mods（2026-08-21）

严重度：中（转录文本经剪贴板+模拟 Ctrl+V 注入时，本库窗口内输入框收不到粘贴）。

根因归属：ui 库缺陷。key_cb 收到的 mods 参数被直接丢弃，应用只能主循环轮询时
实时查询 key_down(Control) 拼 mods；外部注入（如 pasteText 的 SendInput 序列
Ctrl↓/V↓/V↑/Ctrl↑）毫秒级完成，轮询时 Ctrl 已抬起 -> 控件收到裸 KeyV（mods=0）
-> 粘贴分支不触发。连带问题：Ctrl+V 产生 WM_CHAR 0x16 控制字符，on_char 未过滤
会插入乱码。

修复：
- Window 新增 `key_mods_[512]`：key_cb PRESS 时记录 mods 快照；
  `int key_pressed_mods(int key) const` 访问器（惰性清除，下一轮 PRESS 覆盖，
  与 key_pressed 边沿配合使用：先调 key_pressed 再取快照）。
- TextBox::on_char 过滤 C0 控制字符与 DEL（<0x20、0x7f），可见字符不受影响。
- 应用侧（voice_assistant main.cpp）：edit_keys 与 Tab 分发改用 key_pressed_mods 快照。

回归清单（必须保证）：
- 正常人工按键的 Ctrl+C/X/V/A、Shift+方向选择行为不变（快照与实时查询一致）。
- Tab+Shift 导航行为不变。
- 无焦点的编辑键分发不产生副作用（mods 快照仅边沿消费）。
- Ctrl 组合不再向文本框插入控制字符乱码。

验证：ui_ui/ui_gl Release 编译 + install 通过；下游 voice_assistant 构建通过、
CLI 回归全 PASS；人工验证项：聚焦本项目输入框后执行语音转录粘贴，文本正确落入输入框。

### 相关历史条目

- [UI-101](CHANGELOG.md#ui-101-textbox-选择剪贴板能力拖选shift-选择ctrlacxv2026-08-21)：Ctrl+V 分支依赖 mods 到达控件层。

## UI-103: TextArea 剪贴板默认实现——回调未注入时 Ctrl+C/X/V 静默失效（2026-08-21）

严重度：中（多行输入框复制/剪切/粘贴无反应，是否生效取决于使用点是否记得注入回调）。

根因归属：ui 库设计缺陷。TextArea 剪贴板完全依赖 set_on_copy/set_on_paste 回调注入，
on_key_down 对回调判空直接跳过——下游任一使用点漏注入即静默失效（voice_assistant 的
rules_page taRuleCmd_ 漏注入，命令输入框无法复制粘贴；voice_page txtLog_ 注入了故正常）。

修复：
- 内置默认剪贴板读写（GLFW primary_handle + glfwSetClipboardString/GetClipboardString）：
  on_copy_/on_paste_ 未注入时自动兜底，已注入时优先使用（平台特殊实现仍可覆盖）。
- textarea.h 注释同步：回调为可选覆盖。

回归清单（必须保证）：
- 已注入回调的使用点（如 voice_page 日志区）行为不变。
- 未注入回调的 TextArea：Ctrl+C/X 复制原文、Ctrl+V 粘贴、拖选/Ctrl+A 高亮不变。
- readonly 模式仍禁止 X/V 修改文本。
- on_char 控制字符过滤行为不变。

验证：ui_ui Release 编译 + install 通过；下游 voice_assistant 构建通过、CLI 回归全 PASS；
人工验证项：规则页命令输入框（taRuleCmd_）Ctrl+A/C/X/V 与拖选。

### 相关历史条目

- [UI-101](CHANGELOG.md#ui-101-textbox-选择剪贴板能力拖选shift-选择ctrlacxv2026-08-21)：TextBox 同类能力（内置剪贴板，无此问题）。
- [UI-102](CHANGELOG.md#ui-102-键盘修饰键快照sendinput-粘贴到自绘输入框丢失-mods2026-08-21)：mods 快照保证外部注入的 Ctrl+V 到达控件层。

## UI-104: 侧栏分组逐级嵌套成子级折叠菜单——Menu add_group 未配对 end_group（2026-08-21）

严重度：中（外壳侧栏「布局/通用/表单/展示/交互/其他」分组自第二个起逐级缩进，
渲染成上一分组的子级折叠菜单，与镜像 docs/2 同级分组结构不符）。

根因归属：**测试程序**（layui_test/src/app.cpp），非 ui 库。ui 库 `Menu` 的 API 契约
（UI-061）：`add_group(title)` 将内部嵌套层级 `current_level_` 加一，必须以 `end_group()`
闭合；menubar_page / dropdown_page 等使用点均正确配对。app.cpp 主侧栏构建循环对每个
分组只调 `add_group` 从未 `end_group`，导致层级单调递增：

- 「布局」组头 level 0、页项 level 1；「通用」组头 level 1（嵌进布局）、页项 level 2；
  「表单」level 2……每往后一组缩进再深一级（16px/级）。
- 连带行为缺陷：`compute_visibility` 折叠某组时隐藏其后全部项直到遇到
  `level <= 该组` 的下一个 Group——层级只增不减故永不存在，**折叠「布局」会把
  其后所有分组与页项整体隐藏**。

修复（app.cpp）：
- 主侧栏循环换组前 `if (cur_group) nav.end_group();`，循环结束闭合最后一组；
- 第二列子菜单 `populate_child` 构建完成后补 `child_nav.end_group()`（层级卫生）。

回归清单（必须保证）：
- 全部分组组头同级（像素左缘 x≈12），页项统一缩进一级（x≈28）；镜像 docs/2 的
  `layui-menu-item-group` 兄弟结构一致。
- 折叠任一分组只隐藏本组页项，其余分组不受影响（compute_visibility 层级正确）。
- 切页 `set_selected(group,item)` 高亮正确（page_sel 组序号与 Menu 计数一致）。
- 第二列子菜单（数据表格板块）构建/重建行为不变。
- ui 库 Menu 既有回归不变（UI-061/UI-099 清单项）。

验证：layui_test x64 Release（cmake-build-x64 Ninja）编译链接通过；
`--shot-page 0` 截图像素分析：组头 left_x=12/13（布局/通用/表单/展示 全部同级）、
页项 left_x=28~30，折叠层级正确；`--smoke 6` exit 0（53 页逐页切换无崩溃）。

### 相关历史条目

- [UI-061](CHANGELOG.md#ui-061-menu-基础垂直菜单组件--menuhtml-克隆2026-08-17)：Menu add_group/end_group 配对契约来源。
- [UI-099](CHANGELOG.md#ui-099-外壳垂直布局按新镜像重构--页面分组映射2026-08-21)：外壳侧栏改用 ui::Menu + 分组映射（本次修正其分组构建方式）。

## UI-105: 帧率 HUD 与头部右侧控件重叠——HUD 右对齐压在生态/版本下拉上（2026-08-21）

严重度：低（右上角 HUD 文本「layui_test | page n/N | fps」与主题开关、生态、2.13.9
下拉框视觉叠压，两者均不可读）。

根因归属：**测试程序**（layui_test/src/app.cpp），非 ui 库。UI-049 引入 HUD 时头部
右侧尚无控件群；UI-099 重构头部后右侧从右缘 -16 起依次排布 主题开关(44)/生态(88)/
2.13.9(88)/组件/笔记，而 HUD 仍按 `width - text_width - 16` 右对齐绘制于同一区域
（@1280 宽约 x 1030..1264 vs 控件群 868..1264），直接叠压。

修复（app.cpp）：HUD 改绘于头部中段空区 `x = kSidebarW + 16`（logo 240px 区之后，
即镜像站搜索框位置），垂直居中不变——该区域恒为空且不依赖右侧控件宽度，窗口缩放
时亦不会重新叠压（HUD 止于 ~x500，控件群最左为组件导航 ~x868）。

回归清单（必须保证）：
- HUD 每秒 tick 更新、逐帧重绘不闪烁（UI-049 保证不变），仅位置移至头部中段。
- 头部右侧控件群（主题开关/生态/2.13.9/组件/笔记）布局与交互不变（UI-099 保证）。
- HUD 不与 logo 区（0..240）及右侧任何控件重叠。

验证：layui_test x64 Release 编译链接通过；`--smoke 6` exit 0（53 页切换无崩溃）；
位置为确定性几何（HUD 256..~500 vs 控件群 ≥868），无运行期耦合。

### 相关历史条目

- [UI-049](CHANGELOG.md#ui-049-框架页迷你复刻不可交互--用户区不符网站--右上角-hud-闪烁2026-08-16)：HUD 逐帧重绘不闪烁基线。
- [UI-099](CHANGELOG.md#ui-099-外壳垂直布局按新镜像重构--页面分组映射2026-08-21)：头部右侧控件群布局（本次避让对象）。

## UI-106: 侧栏缺失镜像站 14 个页面——基础组整组缺 + 颜色/图标/输入框/选择框/单选框/树形表格/公共类/固定条（2026-08-21）

严重度：中（主导航与镜像站 docs/2 侧栏不对齐，缺 14 项：基础组 6 页全部缺失，
通用缺 颜色/图标，表单缺 输入框/选择框/单选框，展示缺 树形表格/公共类/固定条）。

根因归属：**测试程序**（layui_test 页面清单未随 UI-099 的镜像对齐工作补全），非 ui 库。

修复（layui_test/src/pages/）：启用 `placeholder_pages.cpp` 占位注册数组，新增 14 个
占位页（仅导航项 + 「该页面尚未克隆」提示，官方示例详情后续逐页补全）；同时重排受影响
已实现页的 order 使组内顺序与镜像一致——anim 11→12、checkbox 21→23、breadcrumb 33→34、
menubar 34→35、tabs 35→36、badge 36→37、auxiliar 37→38、card 38→40、progress 39→41、
timeline 40→42、tree 41→44、carousel 42→45、flow 43→46、text 44→47。新页插位：
颜色=9、图标=11、输入框=21、选择框=22、单选框=24、树形表格=33、公共类=39、固定条=43，
基础组取负值（-9..-4）前置于布局组。启动页随之由「框架」变为「开始使用」
（navigate(0) 取排序首项，与镜像侧栏首项一致）。

回归清单（必须保证）：
- 侧栏 7 组顺序与镜像 docs/2 完全一致：基础(6)/布局(2)/通用(4)/表单(5)/展示(18)/
  交互(8)/其他(2)；组内次序同镜像（通用 颜色→按钮→图标→动画；表单 表单→输入框→
  选择框→复选框→单选框；展示 分页→树形表格→导航…辅助→公共类→面板…时间线→固定条→树…）。
- 占位页可进入、显示「尚未克隆」提示，切页无崩溃（--smoke 全轮换验证）。
- 已实现页功能不变（order 仅排序键，不影响任何页面内部逻辑）。
- 侧栏分组必须同级（UI-104 清单继续成立，新增负值 order 组不破坏分组推导）。
- HUD 位于头部中段、不与右侧控件群重叠（UI-105 清单不受影响）。

验证：layui_test x64 Release 编译链接通过；`--smoke 35` exit 0（67 页全轮换无崩溃）；
按 stable_sort(order) 规则从源码重建侧栏树，与镜像 GB18030 解码出的菜单树逐项比对一致。

### 相关历史条目

- [UI-099](CHANGELOG.md#ui-099-外壳垂直布局按新镜像重构--页面分组映射2026-08-21)：侧栏分组映射与 subtitle 字段来源。
- [UI-104](CHANGELOG.md#ui-104-侧栏分组逐级嵌套成子级折叠菜单menu-addgroup-未配对-endgroup2026-08-21)：分组同级保证（本次新增组沿用 end_group 配对逻辑）。

## UI-107: UI-106 的 14 个占位页全部实现为真实内容页（2026-08-21）

严重度：低（功能补全，非缺陷；UI-106 计划内后续）。

根因归属：**测试程序**（layui_test 页面内容建设；未暴露 ui 库能力缺陷——本轮全部用
既有控件能力组装，仅发现 Label 无字号缩放/加粗 API、Button 无 set_custom_content，
均按既有 API 形态调整实现，未改 ui 库）。

实现（layui_test/src/pages/ 新增 9 文件 + 清空占位数组）：
- basic_pages.cpp：基础组 6 页文档摘要（开始使用/底层方法/模块系统/组件构建/国际化/
  更新日志），BlockQuote 导语 + FieldsetTitle 小节 + Code 块。
- color_page.cpp：本地 Swatch 自绘控件（draw_rounded_rect 4px 圆角 + 描边）色卡；
  主色 #16baaa/#009688、工具色 8 卡、文字颜色（layui::bg_color）、背景色（layui::bg_fill）。
- icon_page.cpp：190 图标全量网格（5 列，cell = Icon 22px + 类名 Label）+ 彩色示例行
  （from_class 存在性守卫）。
- input_page.cpp / select_page.cpp / radio_page.cpp：TextBox（含密码框/即时监听）、
  ComboBox（初始选中/禁用/省市区联动）、RadioGroup（默认选中/选项禁用/整组禁用/事件）
  真实交互演示。
- treetable_page.cpp：Table 静态还原树形层级示例（全角空格 + ├─│└─ 符号缩进；
  行展开折叠待 Table 层级能力，help 中注明）。
- class_page.cpp：字色/背景/边框工具类真实演示；字号六档与加粗因 Font 固定像素烘培
  （Label 无 set_text_scale/set_bold）以说明式呈现并注明规划项，不做假缩放。
- fixbar_page.cpp：fixbar 工具组（VBox + Button::set_icon：top/service/email）+
  「返回顶部」真实滚动复位（content->set_scroll_offset(0)）；视口级常驻定位注明规划项。
- placeholder_pages.cpp：kPlaceholders 清空为哨兵条目（MSVC 不支持零长数组），
  占位机制保留供后续新增页使用。

回归清单（必须保证）：
- 14 页全部可进入且有真实内容（预览 + 查看代码 + 帮助三 tab），无「尚未克隆」提示。
- 侧栏树与 UI-106 完全一致：67 页、order 与标题不变（占位→真实页仅替换 build 函数）。
- `--smoke` 全轮换 exit 0（含新页反复进出，Swatch 自绘控件无泄漏崩溃）。
- 「返回顶部」点击后内容区滚动位置复位到顶部。
- UI-106 全部清单继续成立（侧栏对齐、分组同级、HUD 不重叠）。

验证：layui_test x64 Release 编译链接通过；`--smoke 40` exit 0（67 页全轮换）；
navtree 脚本重建侧栏树 67 页 0 重复，与镜像逐项一致。

### 相关历史条目

- [UI-106](CHANGELOG.md#ui-106-侧栏缺失镜像站-14-个页面基础组整组缺--颜色图标输入框选择框单选框树形表格公共类固定条2026-08-21)：占位页注册与 order 重排（本条目为其后续补全）。

## UI-108: Font/Label 字号缩放与仿粗体 + layui 文字工具色 font_color——公共类页升级真实演示（2026-08-21）

严重度：中（ui 库排版能力缺口：Label 无法变字号/加粗；layui_style 缺 .layui-font-*
文字色查询，测试程序误用 bg_color() 导致白字画白底不可见）。

根因归属：**ui 库**（Font 仅按固定像素渲染、无缩放/粗体入口；layui_style 只有
layui-bg-* 底色查询，缺 layui.css 明确存在的 .layui-font-{red|orange|green|cyan|
blue|purple|black|gray} 文字色系列）。测试程序侧的 bg_color 误用已一并纠正。

修复：
- gl/font.h/.cpp：`draw_text(..., float scale = 1.f, bool bold = false)`——全部字形
  度量乘 scale（baseline/x0/y0/宽高/xadvance，方块字近似同步）；仿粗体为字形沿 x
  错位叠加一次，偏移 max(1, round(pixel_height·scale/16))；`measure_text(text,
  scale = 1.f)` 同步。图集本就 2x 过采样（UI-055），≤2x 缩放清晰。
- ui/label.h/.cpp：`set_text_scale(float)` / `set_bold(bool)`（变更置 dirty 重排）；
  preferred_height、recompute_lines 逐码点宽度、on_draw 行高/对齐测量/绘制全链路
  乘 text_scale_。
- ui/layui_style.h/.cpp：新增 `font_color(cls)` + kFontColors 表，色值克隆自
  layui.css 2.13.x 权威值，**与 layui-bg-* 不同源**：red #ff5722、orange #ffb800、
  green #16baaa（bg 为 #009688）、cyan #2f4056、blue #01aaed（bg 为 #1e9fff）、
  purple #a233c6、black #000000（bg 为 #393d49）、gray #c2c2c2（bg 为 #fafafa）；
  未知类回退黑色。bg_color/bg_fill 行为不变。
- layui_test class_page.cpp：字号六档改为 set_text_scale(px/18) 真实缩放演示；
  加粗节改为常规 vs set_bold(true) 对照；字色行改用 font_color 着色。

回归清单（必须保证）：
- 默认 scale=1、bold=false 时 draw_text/measure_text 与改动前逐字节一致（默认参数，
  所有既有调用点零改动零影响）。
- Label 缩放联动：换行断行位置按缩放宽度和 measure、preferred_height 按缩放行高。
- font_color 八个类名色值与 layui.css 2.13.x 一致；未知类回退黑色不崩溃。
- 公共类页：字号行六档墨迹高度单调递增；字色行八标签全部可见且核心色与权威值
  偏差 ≤8（抗锯齿残差）；`--smoke` 全轮换 exit 0。
- UI-055 像素对齐防发虚逻辑在缩放路径保持（rect 对齐 round 后再乘 scale 的偏移
  叠加仍走 align）。

验证：x64 Release 编译链接通过；`--smoke 40` exit 0（67 页全轮换）；公共类页首帧
PAM 像素分析——字号行 6 簇 pitch=174 与布局参数精确吻合、墨迹高度 10→17 单调递增；
字色行 8 簇最暗四分位核心色与 layui.css 值最大偏差 8/6/6/7/7/6/8/1。

### 相关历史条目

- [UI-107](CHANGELOG.md#ui-107-ui-106-的-14-个占位页全部实现为真实内容页2026-08-21)：公共类页原以说明式呈现字号/加粗（本条目为其能力补全后升级）。
- [UI-055](CHANGELOG.md#ui-055-文字像素对齐drawtextdrawicon-字形对齐像素网格2026-08-17)：2x 过采样图集与像素对齐（本次缩放渲染的基础）。

## UI-109: Table 树形层级 add_tree_row + 展开/折叠交互——树形表格页升级真实交互（2026-08-21）

严重度：中（ui 库能力缺口：Table 无行层级能力，树形表格页只能用全角空格 +
├─/│/└─ 符号静态模拟官方示例，无任何交互）。

根因归属：**ui 库**（Table 缺 layui treeTable 的层级展示与展开/折叠能力）。
测试程序侧同步升级为真实交互演示，不再承载符号模拟的伪层级。

修复：
- ui/table.h/.cpp：
  - `add_tree_row(int depth, const std::vector<std::string>& cells)`：按先序追加
    带层级行（depth 0=根，>0 为上一 depth-1 行的子级）；`row_depth_` /
    `row_expanded_` / `tree_visible_` 与 rows_ 平行维护，add_row 恒 depth 0。
  - rebuild_rows 先序遍历计算树形可见性（祖先任一折叠则整棵子树隐藏），
    分页在 tree_visible_ 上切片；page_count 按树形过滤后行数计（折叠子树不计页）。
  - TableRow 首列渲染层级缩进（每级 20px）+ 展开三角（▼ 展开 / ▶ 折叠，
    #c2c2c2 灰、hover 行加深），叶子行留 16px 占位使文字与父级对齐；
    三角用 draw_polygon 与表头排序箭头同源。
  - on_mouse_down 命中首列展开图标矩形（pad_x + depth·20 起 16px 宽 × 行高）
    → `toggle_row(visible)` 切换并重建；`set_on_expand(cb)` 回调（数据行下标, 展开态）；
    `set_row_expanded(data_row, on)` 程序化设置。
  - `data_row_of(visible)` 改为 public（可视行→数据行稳定标识，跨分页/树形过滤）。
  - 树形模式禁用 sort_rows/set_init_sort（扁平重排会破坏层级先序）。
- layui_test/app.cpp：新增 `--click x,y` 合成点击注入（可重复；首次布局后注入、
  再布局一次保证首帧正确）——脚本化交互验证基础设施，供所有交互控件复用。
- layui_test/treetable_page.cpp：数据接口/组织架构两表改用 add_tree_row 真实层级；
  组织架构表挂 set_on_expand 状态标签实时显示展开/折叠动作与 data_row。

回归清单（必须保证）：
- 非 tree 表格（仅 add_row）行为零变化：depth 恒 0、is_tree() 为 false，
  排序/分页/行编辑/复选单选路径不受影响。
- 分页表格在树形过滤结果上切片；折叠子树不计入总页数。
- 缩进精确每级 +20px；三角仅在 row_has_children 的行渲染；叶子占位对齐父级文字。
- 点击三角以外区域不触发折叠（仍走行点击/复选/按钮列等原命中逻辑）。
- toggle/set_row_expanded 后重建走 UI-040 的 clear_children 递归释放路径，无泄漏。
- `--smoke` 全轮换 exit 0。

验证：x64 Release 编译链接通过；`--smoke 20` exit 0（67 页全轮换）；树形表格页
首帧 PAM 像素分析——初始态缩进阶梯 295/315/335 与每级 +20px 精确吻合，父行三角
#c2c2c2(灰度 194) 各 20px、叶子行图标位无墨迹；注入 `--click 308,390 --click
308,710` 后「数据筛选」「前端组」「后端组」整子树消失、下方行按 38px 行高上移、
折叠行三角簇 (300-307)→(302-305) 变窄即 ▼→▶ 翻转、状态标签文本变为
「已折叠：研发部…[data_row=1]」证明 on_expand 回调触发。

### 相关历史条目

- [UI-107](CHANGELOG.md#ui-107-ui-106-的-14-个占位页全部实现为真实内容页2026-08-21)：树形表格页原为符号静态模拟（本条目为其能力补全后升级）。
- [UI-040](CHANGELOG.md#ui-040-table-行重建泄漏修复--widgetdestroy-单控件销毁-api2026-08-11)：rebuild_rows 递归释放旧行控件（本次折叠重建路径复用该机制）。

## UI-111: ScrollBox 视口锚定 add_pinned（layui fixbar）+ PageHost 栈局部悬空修复——固定条页升级真实常驻工具组（2026-08-21）

严重度：高（含测试程序潜在 UB：PageHost 为 render_page lambda 栈局部变量，
页面回调 `[&host]` 捕获全部悬空，回调触发时读到垃圾值——本次 on_scroll 回调
实测读到 off=-2.6e38；此前未暴露仅因多数回调从未在 build 返回后被触发）。

根因归属：**双根因**。
① ui 库能力缺口：ScrollBox 无视口级锚定能力，固定条只能以页内静态呈现替代；
② **测试程序缺陷**：app.cpp 中 `PageHost host` 声明在 render_page lambda 体内，
函数返回即销毁，而页面 build 里以 `[&host]` 捕获的回调（按钮点击/滚动回调等）
生命周期覆盖整个页面显示期。

修复：
- ui/scrollbox.h/.cpp：新增 `PinCorner` 枚举 + `add_pinned(w, corner, margin_x,
  margin_y)`——控件照常 add_child（生命周期/命中测试/绘制顺序不变），但不参与
  纵向流式排布、不计入 content_height；每次布局钉在可视区 corner 内缩 margin 处，
  不随 scroll_offset 移动；后加的锚定控件绘制在流式内容之上。pinned_ 条目在
  on_layout 时对 children_ 校验，被 clear_children 释放的悬空条目自动剔除（切页安全）。
- layui_test/app.cpp：**PageHost 提升到 main 作用域**（生命周期覆盖全部页面控件
  回调）——所有页面 `[&host]` 捕获合法化，属结构性修复而非逐点补丁。
- layui_test/fixbar_page.cpp：右下角常驻「返回顶部」工具组（add_pinned
  BottomRight 20,20）；出现阈值用既有 set_on_scroll 编排（下滚一屏出现、滚回顶部
  隐藏）；点击复位 content 滚动。占位段落增至 40 行保证「一屏」阈值可达。

回归清单（必须保证）：
- 非 pinned 子控件的流式排布 / content_height / 滚动条计算零变化。
- clear_children 后 pinned_ 悬空条目自动剔除，切页无崩溃、无泄漏。
- 页面回调捕获 [&host] 合法（PageHost 与 main 同生命周期）——既有全部 [&host]
  回调（按钮点击等）同步受益。
- 锚定控件命中优先于其下内容（children_ 逆序派发 + 后加先测）。
- `--smoke` 全轮换 exit 0。

验证：x64 Release 编译链接通过；`--smoke 20` exit 0（67 页全轮换）；固定条页三态
PAM 像素分析——初始 bar 区墨迹 0（隐藏）、`--shot-scroll 850` 后 19（出现于
y828-880 = 视口底 900 - margin 20 - 高 52，位置精确）、注入 `--click 1234,850`
后 bar 区归 0 且顶部文本带从 266 复位到 3075（与初始态一致，滚动真实复位 +
on_scroll 回调正确隐藏）。

### 相关历史条目

- [UI-107](CHANGELOG.md#ui-107-ui-106-的-14-个占位页全部实现为真实内容页2026-08-21)：固定条页原以页内静态呈现替代视口锚定（本条目为其能力补全后升级）。
- [UI-109](CHANGELOG.md#ui-109-table-树形层级-addtreerow--展开折叠交互树形表格页升级真实交互2026-08-21)：同批引入的 --click 合成点击注入（本次交互验证复用）。

## UI-110: ComboBox 无 ScrollBox 祖先时弹出面板不翻转，贴近窗口底部的下拉尾部选项被窗口底边裁掉（2026-08-21）

严重度：高（规则管理页底部「类型」下拉 10 个选项仅前 ~7 个可见，滚到底也无法看到
script/mouse/json/combo 尾部选项；所有无 ScrollBox 祖先、位于页面底部的下拉均受影响）。

根因归属：**ui 库**。`ComboBox::sync_panel()` 的翻转/钉顶逻辑整体包在 `if (scroll_anc)`
分支内——仅当存在最近 ScrollBox 祖先时才检测可视区边界；无 ScrollBox 祖先（页面级
绝对布局的下拉框）时恒向下弹出 `kMaxPanelH`(150px) 面板，combo 距窗口底边不足时面板
超出窗口被裁，ScrollBox 内部可滚动但视口外内容永远不可见。

修复（ui/src/combobox.cpp sync_panel 新增 else 分支）：无 ScrollBox 祖先时以 root 可视区
（窗口矩形）为界执行同样的翻转/钉顶——向下放不下翻转到 ComboBox 上方；翻转后仍超出
可视区顶部则钉在可视区顶部；无 root 时兜底视为无限深保持旧行为。

回归清单（必须保证）：
- 有 ScrollBox 祖先的下拉翻转/钉顶行为零变化（原分支未动）。
- 无 ScrollBox 祖先且下方空间充足的下拉仍向下弹（默认路径不变）。
- 无 ScrollBox 祖先且贴近窗口底部的下拉向上翻转，全部选项可见。
- 极小窗口下翻转也不够时面板钉在窗口顶部（不越界、不为负坐标）。
- 打开一关一 / 点击外部关闭 / 滚动关闭 / 键盘上下选择行为不变（UI-0xx 相关清单）。

验证：x64 Release 重编 ui_ui 并安装（install/lib 时间戳更新）；layui_test 链接新库
`--smoke 20` 全 67 页轮换 exit 0（page switches ok）；应用侧（voice_assistant 规则页
类型下拉展开完整显示 10 项）由应用仓库构建回归 + 人工冒烟确认。

## UI-112: 颜色页一比一克隆镜像 docs/2/color（基色调/辅色调/中色调/七色调/标语）+ --list-pages 调试设施（2026-08-22）

严重度：低（页面保真度升级，无缺陷回归）。

根因归属：**测试程序**。原颜色页为自创内容（主色/工具色/文字与背景色混排），
未对照镜像站 docs/2/color/index.html；属测试程序演示页与镜像不一致，非 ui 库缺陷。
所需绘制能力（自绘矩形 + 居中文本 + HBox 布局 + Label 缩放加粗）ui 库均已具备，
无需改动 ui 库。

修复/实现：
- layui_test/pages/color_page.cpp 重写，逐节克隆镜像：
  - 顶部引用块（镜像原文）；
  - 基色调：3 等分白字双行卡（#16baaa 蓝绿色 / #16b777 清新绿 / #1e9fff 经典蓝）；
  - 辅色调：4 等分（#ff5722 Danger / #ffb800 Warning / #16b777 Success / #31bdec Info）；
  - 中色调：radius 0 黑字灰阶（md6 #fafafa/#f6f6f6 + md2×6 #eeeeee~#c2c2c2 +
    md6 #2f363c/#23292e 三行铺满无列间距；#2f363c 黑字、#23292e 白字——镜像
    ws-docs-necolor 规则的原样保留，含其内联白字特例）；
  - 七色调：整宽横条左对齐依次堆叠（红橙绿蓝紫深浅，浅条深字）；
  - 标语 h2 + 引用块「不热衷于视觉设计的开发者不是一个好作家。 —— 贤心」；
  - code tab / help 面板同步更新为新结构。
- ColorBlock 自绘控件：纯色圆角矩形 + 多行文本（居中/左对齐），高度 = 行数×22.4
  （line-height 1.6×14px）+ 上下 20px 内边距，同镜像 CSS。
- layui_test/app.cpp 新增 `--list-pages`：打印「索引\\t组\\t标题」后退出，
  供像素验证脚本定位页索引（此前 --shot-page 索引只能靠猜，本次颜色页实为 8 非 9）。

回归清单（必须保证）：
- 页面注册参数不变（order=9 / 组=通用 / 标题=颜色），侧栏插位不受影响。
- --shot-page 索引语义不变（pages 按 order 稳定排序后的下标）。
- 既有命令行参数（--smoke/--shot/--shot-page/--shot-scroll/--click）行为不变。
- HBox 显式高度行的子件布局不受本页特例影响。

验证：x64 Release 编译链接通过；`--smoke 15` exit 0（67 页轮换）；PAM 像素精确
采样——基色调 3 卡、辅色调 4 卡、七色调 7 条色值与镜像逐一相等
（#16baaa/#16b777/#1e9fff/#ff5722/#ffb800/#31bdec/#a233c6/#2f363c/#fafafa）；
中色调三行结构与滚动后下半页（七色调+标语）ASCII 色图核对一致。

### 相关历史条目

- [UI-107](CHANGELOG.md#ui-107-ui-106-的-14-个占位页全部实现为真实内容页2026-08-21)：颜色页初版真实化（本条目按镜像站一比一重写升级）。
- [UI-108](CHANGELOG.md#ui-108-fontlabel-字号缩放与仿粗体--layui-文字工具色-fontcolor公共类页升级真实演示2026-08-21)：Label set_text_scale/set_bold（本页 h2 小节标题使用）。

## UI-113: 图标页新增按钮图标段克隆镜像 h2#icon（2026-08-22）

严重度：低（新增演示内容，无缺陷回归）。

根因归属：**测试程序**。原图标页仅有彩色图标行 + 190 全量网格，未包含
镜像 docs/2/button/index.html 的 h2#icon「按钮图标」段落。非 ui 库缺陷——
Button::set_icon / add_class 已具备全部能力，图标页仅需新增 demo 段。

修复/实现：
- layui_test/pages/icon_page.cpp：在「彩色图标」与「全部图标」之间新增
  「按钮图标」段，克隆镜像 h2#icon 三组内容：
  - 第一组：默认尺寸 + 默认主题——「按钮 ▾」（set_text+set_icon）+
    left/right/edit/share 四个纯图标按钮；
  - 第二组：小号 + primary——left/right/edit/delete/share 五个纯图标；
  - 第三组：小号 + 混合主题——disabled(normal delete) + normal(left) +
    warm(right) + danger(edit)。
- PageDef tip 更新为「按钮图标 + 彩色示例 + 190 项全量网格」。

回归清单（必须保证）：
- 190 全量网格 / 彩色图标行不受新增段落影响。
- PageDef order=11 / 组=通用 / 标题=图标不变，侧栏插位不受影响。
- --smoke 全轮换 exit 0。

验证：x64 Release 编译链接通过；`--smoke 15` exit 0（67 页轮换）；PAM
截图按钮图标段三组像素核对——红/橙/蓝/紫按钮背景 + 小号 mixed 主题行
可见。

### 相关历史条目

- [UI-107](CHANGELOG.md#ui-107-ui-106-的-14-个占位页全部实现为真实内容页2026-08-21)：图标页初版（190 全量网格 + 彩色图标），本条目补充按钮图标段。

## UI-114: Button 图标右置 set_icon_right + 图标页按钮图标段一比一修正（2026-08-22）

严重度：中（ui 库能力缺口导致演示页无法一比一；API 向后兼容）。

根因归属：**ui 库能力缺口**。`Button` 内容绘制固定「图标居左 + 文本居右」
（button.cpp draw_content），而镜像 docs/2/button h2#icon 的「按钮 ▾」demo 是
文字在前、down 图标在后（HTML 顺序即视觉顺序），测试程序无从绕过——按工作流
回归 ui 库补能力。另发现上一次（UI-113）按钮图标段与镜像有出入：第二容器被拆成
两行且首行按钮缺 layui-btn 类名主题语义，本条目一并修正。

修复：
- ui/include/ui/ui/button.h + ui/src/button.cpp：新增 `set_icon_right(bool)` /
  `icon_right()`（默认 false = 图标居左，既有全部调用零影响）；draw_content 增
  icon_right 参数，true 时布局为「文本居左 + 图标居右」，整体仍居中。
- layui_test/pages/icon_page.cpp 按钮图标段改为严格两容器（同镜像两个
  layui-btn-container）：
  - 容器一：默认尺寸「按钮 ▾」（set_text("按钮")+set_icon("down")+
    set_icon_right(true)，76px）+ left/right/edit/share 四个默认纯图标按钮；
  - 容器二单行 9 个：sm primary×5（left/right/edit/delete/share）+ sm disabled
    delete + sm normal left + sm warm right + sm danger edit。

回归清单（必须保证）：
- 默认 icon_right=false 时绘制结果与改动前逐像素一致（图标居左语义不变）。
- disabled / primary / normal / warm / danger 各类按钮的填充、描边、字色不受影响。
- Button::preferred_height 尺寸类（lg/sm/xs）行为不变。
- 图标页 190 全量网格与彩色图标行零变化。

验证：ui_ui + layui_test 全量编译链接通过；`--smoke 15` exit 0；PAM 像素验证——
行1 五个 accent(#17baab) 填充按钮，「按钮 ▾」ASCII 放大确认文字在左、▼ 箭头在右；
行2 九个按钮依次 primary×5（白底灰框）+ disabled 灰 + normal 蓝(#1f9eff) +
warm 橙(#ffb800) + danger 红(#ff5721)。

### 相关历史条目

- [UI-113](CHANGELOG.md#ui-113-图标页新增按钮图标段克隆镜像-h2icon2026-08-22)：按钮图标段首次加入（本条目修正为一比一两容器结构）。

## UI-115: TextArea 垂直滚动条（2026-08-22）

严重度：低（能力补全，无缺陷回归）。

背景：TextArea 已有内部纵向滚动（scroll_ + 滚轮 + 拖选边缘自动滚屏），但无
滚动条 UI——内容溢出时用户看不到滚动位置，也无法拖拽快速定位（语音助手日志框
等 readonly 长文本场景明显）。

实现（ui/src/textarea.cpp + ui/include/ui/ui/textarea.h）：
- 滚动条绘制：内容溢出时在控件右缘画 track（button_pressed）+ thumb
  （text_dim），圆角半宽，风格与 ScrollBox 一致；未溢出不画。
- 滚动条交互：on_mouse_down 命中滚动条列（±2px 容差）→ 进入拖拽
  （capture_mouse，不抢焦点、不移动光标）；on_mouse_move 按比例映射 scroll_；
  on_mouse_up 结束。点击轨道即跳转（scroll_from_y）。
- 文本可用宽度 avail_width() 恒定让出滚动条列（6+4px），避免滚动条出现/消失
  时换行重排跳动。
- 影响范围：所有 TextArea 使用方（含语音助手日志框、规则页命令编辑框），
  纯增量能力，既有 API 不变。

回归清单（必须保证）：
- 既有滚轮滚动 / 拖选边缘自动滚屏 / ensure_caret_visible 行为不变。
- readonly 模式滚轮可用、输入仍被吞掉。
- 未溢出时无滚动条、不响应滚动条点击；溢出时 thumb 位置随 scroll_ 线性映射。
- 滚动条拖拽期间文本选区/光标不受干扰；松开后 selecting_ 流程正常。
- 换行宽度恒定（让出滚动条列），内容增减导致滚动条出现/消失时已换行位置不变。

验证：x64 Release ui_ui 编译通过并 install；下游 voice_assistant_cpp 全量
Release 构建通过 + regression.ps1 全 PASS。

### 相关历史条目

- [UI-103](CHANGELOG.md#ui-103-textarea-剪贴板默认实现回调未注入时-ctrlcxv-静默失效2026-08-21)：TextArea readonly/剪贴板前置能力（日志框场景依赖）。

## UI-116: 垂直滚动条 overlay 风格化：自动隐藏 + 悬停加宽 + 三态色（2026-08-22）

严重度：低（视觉/交互打磨，无缺陷回归）。

背景：UI-115 滚动条为恒显单色静态样式，无状态反馈且常驻占用视觉；
参照现代 overlay scrollbar 惯例风格化，并将 TextArea / ScrollBox 两处
实现统一到共享模块，消除样式漂移。

实现：
- 新增 ui/ui/scrollbar_style.h（内部协作头，不进 all.h）：常量
  （常态 6px / 悬停拖拽 8px、右距 4、命中容差 ±2px、thumb 最小高 28px、
  自动隐藏延时 2s）、State{dragging,last_active}、track/thumb 几何、
  hit/scroll_from_y/draw 共享实现。
- 自动隐藏：控件未悬停、未拖拽、且超过 2s 无滚动活动时不绘制；
  滚轮滚动 / 点击轨道 / 拖拽均刷新活动时刻（glfwGetTime）。
- 三态色：thumb 默认 text_dim@55% → 悬停 text_dim 全色 → 拖拽 accent；
  track 仅活动期绘制 button_pressed@40%。
- 宽度反馈：悬停右缘列或拖拽时 6→8px；内容让出宽度保持恒定
  （kBarW+kMargin），换行/子控件布局不随宽度态跳动。
- TextArea / ScrollBox 全部切换到共享实现；ScrollBox 删 dragging_ 私有
  布尔改用 State，新增 over_bar()；既有 needs_layout_ 抑制、下拉面板
  点击消费、触底回调等语义不变。

回归清单（必须保证）：
- 未溢出内容永不显示滚动条、滚轮不消费（返回 false 透传）。
- 滚动条隐藏期间滚轮滚动照常工作（隐藏≠禁用），滚动即浮现。
- 悬停 thumb 加宽后命中区随 8px 几何；±2px 容差不抢正文点击。
- 拖拽中松开（on_mouse_up）结束拖拽；拖拽不改变焦点与文本光标/选区。
- ScrollBox 下拉面板（layui-form-select-panel）点击消费、ComboBox 翻转
  定位、pinned 锚定控件不受影响。
- 设置页整页 ScrollBox / 规则页表格区 / 日志框行为一致。

验证：x64 Release ui_ui 编译 + install 通过；下游 voice_assistant_cpp
Release 构建通过 + regression.ps1 全 PASS；视觉三态与自动隐藏节奏待
GUI 人工冒烟。

### 相关历史条目

- [UI-115](CHANGELOG.md#ui-115-textarea-垂直滚动条2026-08-22)：TextArea 滚动条基础能力（本条目将其风格化并与 ScrollBox 统一）。

## UI-117: 图标页克隆镜像列表 195 项 + HBox 自适应高度与图标字体扩段（2026-08-22）

严重度：高（「全部图标」网格整体不可见；ui 库两处缺陷 + 一处能力限制）。

背景：用户反馈「通用>图标」页只有 layui-icon-xxx 文本、无字形且行距塌陷
（对照镜像 docs/2/icon 图标列表）。PAM 像素采样定位三个独立根因，均归属
ui 库，按「先归因再修复」修正 ui 库本身；测试程序只做页面编排。
（用户手上 exe 同时呈现乱码，系多会话并行改动期间的混合产物所致，
非独立缺陷。）

根因与修复：
1. HBox 缺 preferred_height() 重写（ui 布局缺陷）：h=0 的行投入 ScrollBox
   流式排布时按默认 bounds_.h=0 取高 → on_layout 给子控件派发负高度
   （0−2×padding），图标页 38 行网格整体塌陷不可见（复现：y510 以下零墨水）。
   修复：h>0 显式高度优先返回 bounds_.h；h≤0 返回可见子控件
   preferred_height 最大值+2×padding，与 FlowBox/Grid 自适应语义对齐。
2. scrollbar_style.h 头文件裸包含 <GLFW/glfw3.h>（UI-116 引入的编译缺陷）：
   GLFW 默认拉入系统 GL/gl.h，凡先包含该头再包含 glad/gl.h 的 TU 报
   C1189 直接编译失败（layui_test app.cpp 即命中）。修复：头部移除 GLFW
   依赖，时间源改 std::chrono::steady_clock（touch/active/自动隐藏语义不变；
   ui 库公开头不再暴露平台 GL 头）。
3. 图标烘培区段不足（ui_gl 能力限制）：kIconNum=0x200 仅覆盖 E600..E7FF，
   2.13.x 新字形 folder(eabe)/folder-open(eac1)/rss(e808)/component(e857)/
   refresh-3(e9aa)/tips-fill(eb2e) 越界被范围检查静默丢弃（旧表 tips-fill
   实际从未渲染成功过）；close(1006)/close-fill(1007)/ok-circle(1005) 更在
   私用区之外。修复：主段扩为 E600..ECFF（stbtt 对缺字形零占位打包，
   不耗图集空间）；新增第二段 U+1000..100F 紧接首段打包底边 bottom_y
   续烘进同一图集（UV 加行偏移；溢出仅告警不致命）；新增
   Font::icon_glyph(cp) 统一段查询，draw_text/draw_icon/char_width
   三处路径全部改走该查询，段外码点安全返回 nullptr 不越界。

资产与数据同步：
- assets/iconfont.ttf 升级为镜像 layui v2.13.9 同款（旧 2.9.18 字体缺新字形）。
- layui_icons.h 与 layui_style.cpp kIcons 表 190→195：补 bot(E7D6)/sound(E69D)/
  close(1006)/close-fill(1007)/ok-circle(1005)（码点取自 2.13.9 layui.css）。
- ui::Card 新增 set_outline(color, thickness=1)：可选描边（a≤0 关闭，默认关）。

测试程序编排（layui_test/src/pages/icon_page.cpp）：
- 「全部图标」网格重写为镜像一比一：6 列描边卡片（Card+set_outline
  #e8e8e8），每卡四行＝字形(26px)/中文名/&#x码点;/layui-icon-类名；
  前 192 项与镜像列表同序，末尾保留 layim-download/layim-theme/
  layim-uploadfile（镜像页已下架、码点表仍收录），共 195 项。
- cell() 内 Icon 补 set_theme：Icon 无 theme 时回退纯白色、白卡上不可见
  （契约：主题色图标须 set_theme 或 set_color）。

回归清单（必须保证）：
- HBox：h>0 显式高度行为不变；Valign::Center 居中不受影响；h≤0 行投入
  ScrollBox/VBox 后子件高度≥0，行高=最大子高+2×padding。
- Card：默认无描边；header 卡、阴影、body 排布均不变。
- load_icon_font：双段外码点安全返回 nullptr；range2 图集溢出仅 error
  日志、主段照常可用；bottom_y 语义沿用 UI-046。
- Icon：无 theme 回退白色不变；set_color 覆盖优先级不变。
- 图标字体回退链（exe 目录→CWD→../assets→assets）不变（UI-047/UI-048）。
- 滚动条 overlay 自动隐藏/三态色/悬停加宽行为不变（UI-116，时间源替换等价）。

验证：x64 全量重建通过（GLFW 头污染消除后 app.cpp 可编译）；--shot-page 10
PAM 采样——网格首行 6 字形墨水>0（bot=132/leaf=61/folder=55/folder-open=93/
gitee=317/github=248）、row31 close=32/ok-circle=129（第二段码点生效）、
末行仅 3 卡（layim×3）；卡片描边线 y627/774/888 定位准确、行距 pitch≈131px；
--smoke 6s 全 67 页切换 exit 0；颜色/按钮/树表页抽查正常。

### 相关历史条目

- [UI-116](CHANGELOG.md#ui-116-垂直滚动条-overlay-风格化自动隐藏--悬停加宽--三态色2026-08-22)：scrollbar_style.h 引入 GLFW 头依赖（根因 2 修复之）；时间源 glfwGetTime→steady_clock。
- [UI-114](CHANGELOG.md#ui-114-button-图标右置-seticonright--图标页按钮图标段一比一修正2026-08-22)：图标页按钮图标段（本条目完成全量网格一比一）。
- [UI-046](CHANGELOG.md#ui-046-font-烘培计数误用-stbttbakefontbitmap-返回值ascii图标大量字形缺失2026-08-16)：BakeFontBitmap bottom_y 语义（range2 续包依据）。

## UI-118: 图标卡片文本居中 + 数据表抽离重构（2026-08-22）

严重度：低（视觉对齐修正 + 测试程序内部重构，无 ui 库缺陷）。

根因归属：卡片内三行文本左对齐属**测试程序**编排问题——ui 库
`Label::set_align(int)`（1=水平居中，M11 起提供）能力齐全，
icon_page.cpp 的 cell() 未调用；无需改动 ui 库。

修复与重构（layui_test/src/pages）：
- cell() 中文名/码点/类名三行 Label 补 `set_align(1)` 水平居中
  （Icon 本身按控件中心绘制，已居中）。
- 码点(&#x…;)/类名(layui-icon-…) 两行 `set_text_scale(0.78)` 小字灰阶，
  对齐镜像 docs-icon-code / docs-icon-fontclass 的次级字号层次。
- 195 项数据表（IconItem kIcons + kIconCount）自 icon_page.cpp 抽出至
  新头 icon_catalog.h：页面文件 429→231 行，数据与编排分离；
  表与 ui::layui_icon 码点表的对应关系（前 192 项镜像同序 + layim×3）注释随迁。

回归清单（必须保证）：
- 图标页 195 张描边卡片渲染不变：字形/中文名/码点/类名四行结构、
  Card::set_outline #e8e8e8 描边、6 列 HBox h=0 自适应行高（UI-117）。
- 卡片四行内容均以卡片垂直中线为轴水平居中（偏差 ≤1px）。
- icon_catalog.h 为图标页唯一数据源；修改码点须同步 ui::layui_icon 表。

验证：x64 重建通过；--shot-page 10 PAM 采样——首行 6 卡文本墨水中心
相对卡中心偏差 0/-0/+0/-0/+0/-1 px；六卡字形墨水不变（bot=132/leaf=61/
folder=55/folder-open=93/gitee=317/github=248）；--smoke 4s 全页切换 exit 0。

### 相关历史条目

- [UI-117](CHANGELOG.md#ui-117-图标页克隆镜像列表-195-项--hbox-自适应高度与图标字体扩段2026-08-22)：网格一比一克隆与 ui 库三根因修复（本条目在其产物上做居中修正与重构）。


## UI-119: 表单/选择框/分页页崩溃排查：陈旧增量构建产物 ODR 混合布局 + 崩溃诊断钩子（2026-08-22）

严重度：高（特定页面稳定崩溃 0xC0000374/0xC0000005；根因为构建产物不一致，非源码缺陷）。

背景：用户反馈点击侧栏「表单 > 表单」「复选框」或切换若干组件后程序闪退。
脚本化扫描（`--shot-page 0..66`）定位崩溃面：出问题的 exe 下 p12（表单）/p14（选择框）
报 0xC0000005 访问违例、p12/p19（分页）报 0xC0000374 堆损坏，其余 64 页正常，
且同一页多次运行崩溃码漂移（堆损坏检测时机非确定性的典型特征）。

归因过程（排除法）：
1. 9bd5210（UI-117 之前）建 worktree 复测三页通过——初判为近期引入（后被推翻）；
2. HEAD 上逐项临时回退 HBox::preferred_height、图标字体扩段
   （kIconNum 缩回 0x200 并禁用第二段烘培）——仍崩，排除两处 UI-117 源码改动；
3. 同一份源码全量重建（RelWithDebInfo、Release+/Z7 各一次）后 67/67 页全过、
   多次复跑稳定——崩溃随重建消失，锁定问题在**构建产物层**而非代码。
   （第 1 步的 worktree 未设 CMAKE_BUILD_TYPE＝无优化编译，其「通过」当时不可比。）

根因：UI-117 会话中 app.cpp 曾因 scrollbar_style.h 裸包含 GLFW 头报 C1189
（见 [UI-117](CHANGELOG.md#ui-117-图标页克隆镜像列表-195-项--hbox-自适应高度与图标字体扩段2026-08-22)
根因 2），编译失败态与其修复后的增量重编交错，导致部分 TU 仍按旧
kIconNum=0x200 的 sizeof(Font)（约 25KB）生成代码，而 ui_gl.lib 内已重编的 TU
按 0x700 新布局（约 76KB）写 icon_glyphs_ —— 同一类两套布局并存构成 ODR 违例：
Font 成员函数按大布局写入小布局对象越界，破坏堆元数据；损坏点休眠，直到
恰好踩中的页面分配/释放模式触发检测（解释了仅 3 页崩溃与崩溃码漂移）。
教训：①布局相关 constexpr 公开头（如 gl/font.h 的字形数组尺寸）变更后，
凡经历过编译失败/中断的构建树必须全量重建再验证；②「手头 exe 能复现」不是
源码回归的充分证据，先以干净全量重建复核再动代码。

修复与防复发：
- layui_test/src/app.cpp 新增崩溃诊断钩子：SetUnhandledExceptionFilter +
  dbghelp（StackWalk64/SymFromAddr）符号化打印异常码/异常地址/AV 读写目标及
  调用栈（#n 帧 + 偏移），配合 Release+/Z7 的 PDB 可一击定位崩溃帧；本次排查
  已实战验证。仅测试程序装载，ui 库零侵入。
- 删除仓库根陈旧 `install/` 目录（gitignored 的一次性安装产物，含旧版 ui 头
  副本、无任何构建引用）：消除旧头被误用的 ODR 隐患；调试用 IFEO PageHeap
  注册表项与 git worktree 已一并清理。

验证：全量重建后 `--shot-page 0..66` 67 页全部 exit=0（含原崩溃页 12/14/19）；
`--smoke 6` exit=0；图标页截图字形正常。工作区最终仅剩本条目代码改动
（app.cpp +56 行）。

回归清单（必须保证）：
- 布局相关公开头（gl/font.h 的 kIconNum/kIcon2Num 等数组尺寸常量）变更后：
  干净全量重建产物下 67 页扫描全部 exit=0，无 0xC0000374/0xC0000005。
- app.cpp 崩溃钩子：正常运行零影响（仅注册过滤器）；崩溃时输出
  `CRASH code=/addr=` 与符号化栈帧。
- 根 `install/` 不存在时所有 target 配置/构建不受影响（该目录从未进入
  INCLUDES 链）。

## 相关历史条目

- [UI-117](CHANGELOG.md#ui-117-图标页克隆镜像列表-195-项--hbox-自适应高度与图标字体扩段2026-08-22)：C1189 编译缺陷与字体扩段是本次混合布局产物的源头。
- [UI-046](CHANGELOG.md#ui-046-font-烘培计数误用-stbttbakefontbitmap-返回值ascii图标大量字形缺失2026-08-16)：bottom_y 语义沿用，第二段烘培依赖之。


## UI-120: Slider 聚焦外框改为默认不显示 + set_focus_ring 可选开启（2026-08-22）

严重度：低（交互反馈观感问题；layui 镜像无此反馈）。

背景：用户反馈「交互 > 滑块」页滑块点击聚焦后出现外框。定位为 ui 库
Slider::on_draw 末尾 `focused()` 时按 bounds_ 画 1.5px 主题色描边
（`draw_rect_outline`），属本库自加的焦点可视反馈；layui slider 原型
（div 实现）聚焦并无外框，克隆保真度上应默认关闭。

修复（ui/ui/slider.*）：新增 `focus_ring_` 标志（默认 false）与
`set_focus_ring(bool)` 开关，原描边调用改为 `focus_ring_ && focused() &&
!grey` 才绘制。需要焦点指示的场景可显式 `set_focus_ring(true)` 开启；
键盘可达性不受影响（方向键/Home/End 微调逻辑不变）。

验证：Release 全量重建通过；`--smoke 5` exit=0；滑块页（--shot-page 41）
exit=0 渲染正常。

回归清单（必须保证）：
- Slider 默认任何状态（hover/拖拽/键盘聚焦/禁用）均不绘制 bounds 外框。
- set_focus_ring(true) 后 focused() 且非 disabled 时恢复外框绘制。
- 滑块其余行为不变：轨道/填充/圆钮、range 双钮、step/showstep 刻度、
  tips 提示、input 输入框、vertical 布局、disabled 灰化。

## UI-121: ComboBox::set_options 就地重载选项——销毁重建孤儿控件（幽灵下拉）根治 API（2026-08-22）

严重度：中（库能力缺失诱发应用层缺陷模式；audio_text 规则管理页「添加模式映射后凭空出现下拉框」即实例）。

根因归属：ui 库能力缺失。库未提供就地换选项接口，调用方只能 `delete+new` 重建整个
ComboBox；新控件以构造坐标 (0,0) 出生、`visible` 默认 true，而页面级布局仅给活动
tab 摆位，凡绕过切页函数的重建即产生「可见但错位/失管」的孤儿控件——叠加派发顺序
「后添加者优先」，孤儿还会抢吃重叠区点击。audio_text 规则页 rebuildRuleSetCombo()
漏写 visible 守卫即触发幽灵下拉，孪生函数有守卫的事实证明此类守卫式补丁不可持续。

修复（ui/ui/combobox.*）：新增 `set_options(const std::vector<std::string>&)`：
先关展开面板 → `panel_->clear_children()` 递归释放旧选项按钮（UI-040 同语义）→
重建全部选项按钮 → 原选中项在新列表内则保持、越界复位为未选中（显示回退首项）→
`sync_panel()`。重载是非用户选择事件，**不触发 on_change**。

验证：库 Release 重编安装；layui_test 全量重链（67 页）`--smoke 20` exit=0；
audio_text 以 set_options 重构 rebuild*Combo 后 regression.ps1 全 PASS（见该仓 CHANGELOG）。

回归清单（必须保证）：
- set_options 后仍可继续 add_option（追加语义不变）。
- 重载过程零 on_change 回调；随后 select(i) 正常触发一次回调。
- 原选中项在新列表范围内显示保持；越界回退首项文本、selected()=-1。
- 面板展开状态下调 set_options：面板关闭、root 无弹层残留。
- 反复 set_options + 析构无二次释放/泄漏（clear_children 路径与 ~ComboBox 手动删除路径互斥成立）。

## UI-122: Laypage/Table 分页条区域点击穿透——容器自身 chrome 一律消费（2026-08-22）

严重度：高（点击穿透类缺陷第三例，与 2026-08-20 下拉面板两例同族）。

根因归属：ui 库缺陷。`Laypage::on_mouse_down` 对条目间空隙与 Limit/Skip/Refresh
区域 return false、`Table::on_mouse_down` 对分页条矩形命中亦 return false 放行——
分页条作为表格自身 chrome 未消费事件，凡与分页条几何重叠或错位的下层控件都会吃掉
本应属于翻页条的点击（audio_text 规则管理页正则规则表翻页穿透根源之一）。违反本库
已确立原则：Button 消费左键按下（防下拉选项穿透）、ScrollBox 消费面板主体点击。

修复：
- ui/src/laypage.cpp：边界内左键一律消费（return true）。limit/skip/refresh 子控件
  在 dispatch 阶段先于父节点获得事件，交互不受影响；
- ui/src/table.cpp：分页条兜底命中改 return true，且仅在
  `paginated_ && page_bar_->visible` 时生效——未分页表格不在底部留 28px 隐形吞点区。

验证：layui_test 全量重编（67 页）`--smoke 20` exit=0。

回归清单（必须保证）：
- 分页条首/上一/下一/末/页码按钮点击跳页正常。
- 分页条空白处点击不再穿透到下层控件（含表格行）。
- 未开启分页的 Table 底部区域点击行为与旧版一致（无吞点区）。
- laypage 页 limit 下拉、skip 输入框、refresh 按钮交互全部不变。

## UI-123: Table auto_page_size 扣除分页条高度 + set_paginated 保持页码（2026-08-22）

严重度：中（每页条数按全高估算偏多一行，末行落入分页条下方滚动盲区；窗口缩放
触发重设时浏览中页码被静默清零跳回首页）。

根因归属：ui 库 API 缺失 + 行为缺陷。①调用方只能按 `(h-表头)/row_h` 自行估算每页
条数，而 Table 内部 body 实际高度为 `h-表头-28px`，多算一行永远被分页条遮住需滚动；
②`set_paginated()` 无条件 `page_=0`，窗口尺寸变化经布局重算触发时页码丢失。

修复（ui/ui/table.*）：
- 新增 `int auto_page_size() const`：按当前 bounds 高度自适应，恒定预留 28px 分页条；
- `set_paginated` 页码改为 clamp 到新总页数范围；`reload()` 数据重载仍显式归零
  （layui reload 语义不变）。

验证：layui_test 全量重编 `--smoke 20` exit=0；audio_text 两处估算改调本接口后
regression.ps1 全 PASS。

回归清单（必须保证）：
- `auto_page_size()×行高+表头+28 ≤ 表高`（当前页行不再溢出到分页条下方）。
- set_paginated 调整 page_size 后页码保持（clamp 不越界），不再跳回首页。
- reload() 后回到第 1 页（原语义保持）。
- 树形表格 page_count 仍按 tree_visible_ 过滤后计数。

