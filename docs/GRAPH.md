# 知识图谱索引（AUTO-GENERATED）

> 由 `tools/graphgen.py` 生成，**请勿手改**。重新生成：`python tools/graphgen.py`。
> 规范数据见 `graph.json`（节点自带 `backlinks` 数组，模型无需全库 grep 即可读反链）。

## 统计

- 节点: 163（BUG 155 / DOC 6 / META 2）
- 边: 359
- lint: 0 错误 / 0 警告

## 反链索引

### BUG:UI-001 · UI-001: 初始化项目文档图谱（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-001-初始化项目文档图谱2026-08-09`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:README` (checklist) — 回归清单#文档图谱与 CHANGELOG 条目可被 `python tools\graphgen.py` 正常生成与校验

### BUG:UI-002 · UI-002: 3rdparty 依赖构建管线（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-002-3rdparty-依赖构建管线2026-08-09`
- 反链(6):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:BUILD` (checklist) — 回归清单#FFmpeg 环境注入依赖 `MSYS2_PATH_TYPE=inherit`（MSYS2 profile 默认 minimal 会丢弃继承的 Windows PATH，导致 cl 不可见）
  - `DOC:BUILD` (checklist) — 回归清单#OpenCV 安装为嵌套布局 `install/{arch}/x64/vc16/{bin,lib}`；CMake 直接 `find_package(OpenCV CONFIG PATHS .../x64/vc16/lib)` 绕开对 MSVC 194x 失效的 WinPack 版本检测
  - `DOC:BUILD` (checklist) — 回归清单#GLFW 经 CMake 安装，`find_package(glfw3 CONFIG PATHS install/{arch}/lib/cmake/glfw3)`，目标名 `glfw`
  - `DOC:BUILD` (checklist) — 回归清单#`cmake/FindFFmpeg.cmake` 同时搜 `bin`/`lib` 两处 import lib；SHARED IMPORTED 目标需同时设 `IMPORTED_LOCATION`（DLL）与 `IMPORTED_IMPLIB`（.lib）
  - `DOC:README` (checklist) — 回归清单#`scripts/build/*.ps1` 双架构 x64/x86 一键构建全部 3rdparty 依赖并安装到 `3rdparty/install/{x64,x86}`

### BUG:UI-003 · UI-003: 分层框架骨架与构建验证（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-003-分层框架骨架与构建验证2026-08-09`
- 反链(10):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#core 层不得 include GL/OpenCV/FFmpeg 头文件（纯 C++，保证核心可移植、可单测）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#media 层仅反向依赖 ui/ui_gl/ui_core，且默认 OFF（纯 UI 构建不依赖媒体库）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Demo 冒烟验证：GL 3.3 Core 上下文 + 清屏 + 色块矩形 + 垂直同步 + FPS 循环，运行正常
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Renderer 像素坐标 y 向下、(0,0) 左上角；draw_quad 经 uRect 变换单位矩形
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#x64/x86 双架构 + media ON/OFF 编译回归
  - `DOC:BUILD` (checklist) — 回归清单#顶层 CMake 对 MSVC 全局加 `/utf-8`（源码含 UTF-8 中文注释，否则 C2447/C4819）
  - `DOC:BUILD` (checklist) — 回归清单#MSVC 下 GLAD `gl.c` 以 `LANGUAGE CXX` 编译（-TP），避免 C 代码告警
  - `DOC:README` (checklist) — 回归清单#顶层 CMake：C++17、media 开关默认 OFF、按架构推导 `OPENGUI_3RDPARTY` 前缀
  - `DOC:README` (checklist) — 回归清单#Demo（apps/opengl_ui_demo）开窗渲染可运行，media ON/OFF 双配置均可编译

### BUG:UI-004 · UI-004: M1 渲染管线升级：圆角矩形 SDF + 纹理/字体 + 控件树（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-004-m1-渲染管线升级圆角矩形-sdf--纹理字体--控件树2026-08-09`
- 反链(5):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#圆角矩形：SDF + fwidth 抗锯齿，半径 clamp 到 `min(w,h)/2`；draw_quad 为 radius=0 别名，内部必须全不透明（SDF 含内部距离项 `min(max(q.x,q.y),0)`，缺则 radius=0 内部 50% 半透明造成全界面灰蒙蒙）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#纹理管线：uUv 子矩形采样 + uColor 调制；uv 用 Rect{x,y,w,h} 承载 {u0,v0,u1,v1}
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Font：stb_truetype 烘培 ASCII 32..126 图集，单通道覆盖度转 RGBA 白字
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Widget 树：后添加子节点在上层优先命中；dispatch 事件命中后冒泡停止

### BUG:UI-005 · UI-005: M2 布局系统：VBox/Label + 滚轮/窗口缩放（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-005-m2-布局系统vboxlabel--滚轮窗口缩放2026-08-09`
- 反链(4):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Widget 布局：on_layout()/layout() 递归；VBox 按添加顺序纵向排布（padding/spacing），子控件等宽填满
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Label：字体/颜色/水平对齐；未显式设色时回退主题文字色
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Window：滚轮量 scroll_dx/dy 与 resized() 均为边沿触发（读一次清除）

### BUG:UI-006 · UI-006: M3 焦点系统与键盘输入（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-006-m3-焦点系统与键盘输入2026-08-09`
- 反链(4):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#焦点：focus()/blur()/focused()，整棵树唯一焦点；Tab 遍历 focus_next/prev（先序收集可聚焦可见控件，Shift 反向）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#键盘：dispatch_key_down/up 由根节点调用，Tab 由基类导航、其余转发给焦点控件；keycodes.h 键值对齐 GLFW
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Button 可聚焦：点击或 Enter/Space 激活触发 on_click，聚焦态 2px accent 描边

### BUG:UI-007 · UI-007: 内容自适应高度与焦点完善（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-007-内容自适应高度与焦点完善2026-08-09`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#内容自适应高度：preferred_height() 默认返回 bounds_.h；VBox 以之为子控件高度，Label 返回字体行高
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#焦点抽象：dispatch_mouse_down 命中即 focus()（子类无需自行聚焦）

### BUG:UI-008 · UI-008: M4 文本输入：TextBox + 字符通道 + scissor 裁剪（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-008-m4-文本输入textbox--字符通道--scissor-裁剪2026-08-09`
- 反链(5):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#键盘分发：dispatch_key_down 先让焦点控件消费（focused_descendant 定位），导航键未被消费才遍历；无焦点时 Tab 落到第一个
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#字符输入：Window char 队列边沿触发；dispatch_char 只转发焦点控件；on_char 默认不消费
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#TextBox 单行输入：字符/Backspace/Delete/左右移/Home/End；聚焦光标 + 超宽 scissor 裁剪横向滚动
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Renderer scissor：push_scissor/pop_scissor 可嵌套，pop 恢复上一级矩形（内部维护栈：push 与栈顶求交、pop 重设新栈顶并翻转 y，栈空才关闭 GL_SCISSOR_TEST）；禁止仅递减深度不恢复矩形

### BUG:UI-009 · UI-009: M5 鼠标拖拽：Slider 滑块 + 鼠标捕获（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-009-m5-鼠标拖拽slider-滑块--鼠标捕获2026-08-09`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#鼠标捕获：on_mouse_down 返回 true 即捕获，move/up 越界仍路由给捕获者，up 自动释放
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Slider 滑块：拖拽/点击改值 + 左右键微调 + Home/End 极值；轨道 + 填充 + 聚焦手柄

### BUG:UI-010 · UI-010: M6 媒体模块点亮：ImageView/VideoView（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-010-m6-媒体模块点亮imageviewvideoview2026-08-09`
- 反链(7):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#media 层依赖 ui_ui/ui_gl/ui_core 且默认 OFF；ImageView(OpenCV)/VideoView(FFmpeg) 载入素材→Texture
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Texture::update() 增量上传（尺寸须与 create 一致，视频逐帧复用纹理）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#VideoView 头文件不暴露 FFmpeg C 头（VideoContext 不透明指针隔离）；GL/纹理只在主线程
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#media ON 需 Release 配置（OpenCV 仅发布库，Debug 会 debug_build_guard 命名空间错位导致 LNK2019）
  - `DOC:BUILD` (checklist) — 回归清单#media 开启：`-DOPENGUI_MEDIA_FFMPEG=ON -DOPENGUI_MEDIA_OPENCV=ON`，且 **必须 `-DCMAKE_BUILD_TYPE=Release`**（OpenCV 仅发布库，Debug 下 `debug_build_guard` 命名空间错位 → LNK2019）
  - `DOC:BUILD` (checklist) — 回归清单#media ON 时 apps POST_BUILD 自动拷贝 glfw3.dll、FFmpeg `bin/*.dll`、OpenCV `*/vc16/bin/opencv_world*.dll` 及 `assets/test.{png,mp4}` 到可执行目录

### BUG:UI-011 · UI-011: M7 视频解码线程化 + 播放控制（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-011-m7-视频解码线程化--播放控制2026-08-09`
- 反链(5):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#VideoView 头文件不暴露 FFmpeg C 头（VideoContext 不透明指针隔离）；GL/纹理只在主线程
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#VideoView 线程化：解码线程 + 原子播放/循环/seek 标志 + 条件变量；主线程 advance(dt) swap 出帧后纹理上传；close 先 join 再释放
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#VideoView 播放控制：play/pause/set_loop/seek/position/duration；暂停不解码，暂停中 seek 刷新一帧；EOF 且 loop 回 0
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Slider 程序同步：set_value_silent() 改值不触发 on_change（避免回调循环）；dragging() 供外部跳过同步

### BUG:UI-012 · UI-012: M7 播放交互收尾：EOF 回播 + 解码节流（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-012-m7-播放交互收尾eof-回播--解码节流2026-08-09`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#VideoView 播完回播：EOF 不循环时置 eof_ 等待，play() 检测到先 seek(0) 再播；解码按 1/fps sleep_until 节流（seek 后立即刷新）

### BUG:UI-013 · UI-013: M8 滚动容器：ScrollBox（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-013-m8-滚动容器scrollbox2026-08-09`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Widget::draw 为 virtual（容器可覆写插入 scissor 等绘制阶段）；on_draw 只画自身背景
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ScrollBox 滚动容器：内容超高可滚，scissor 裁剪可视区；滚轮滚动无溢出不消费；右缘轨道+滑块指示，拖动改值；布局让出滚动条列；子节点变化（add/remove/clear）标记重布局，抑制切页后滚动条闪现

### BUG:UI-014 · UI-014: M9 复合控件：CheckBox / RadioGroup / ComboBox（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-014-m9-复合控件checkbox--radiogroup--combobox2026-08-09`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#CheckBox / RadioGroup / ComboBox 复合控件：Tab 聚焦 + 键盘选择 + 点击/回调；ComboBox 下拉复用 ScrollBox，点击外部关闭

### BUG:UI-015 · UI-015: M10 主题系统：暗/亮预设 + 运行期切换（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-015-m10-主题系统暗亮预设--运行期切换2026-08-09`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Theme 暗/亮预设 + 运行期切换：控件持 `const Theme*` 渲染时解引用，切换同一地址即时重绘

### BUG:UI-016 · UI-016: M11 多行文本排版：Label 换行（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-016-m11-多行文本排版label-换行2026-08-09`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Label 多行：`\n` 换行 + 最大宽度自动换行（中文按字形宽度），水平对齐保持

### BUG:UI-017 · UI-017: M12 布局增强：HBox / Grid / 嵌套滚动（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-017-m12-布局增强hbox--grid--嵌套滚动2026-08-09`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#HBox 横向 / Grid 网格布局：子控件按序排布，Grid 按列数换行 + `preferred_height()` 自适应；ScrollBox 内可嵌套 VBox 等容器。HBox `Valign` 垂直对齐（默认 Stretch 填满 / Center 按 preferred_height 居中，layui 尺寸按钮行用）；FlowBox 流式换行容器（layui-btn-container 语义，spacing 10 自动换行）；HBox h≤0 时 `preferred_height()`=可见子高最大值+2×padding（h>0 显式优先），ScrollBox/VBox 流式排布不再塌陷

### BUG:UI-018 · UI-018: M13 视频增强：倍速 / 帧步进 / 精确 seek（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-018-m13-视频增强倍速--帧步进--精确-seek2026-08-09`
- 反链(4):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#VideoView 倍速 `set_speed(0.1..4)`：主线程 `advance` 与解码节流按 `1/(fps*speed)` 缩放
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#VideoView 帧步进 `step_frame()`：暂停中精确解码一帧推入交接缓冲，不烧 CPU
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#VideoView 精确 seek：BACKWARD 到关键帧后前进解码至目标时刻再推帧，`position()` 与画面一致

### BUG:UI-019 · UI-019: M14 窗口系统：PanelWindow / MenuBar / 模态 / Tooltip（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-019-m14-窗口系统panelwindow--menubar--模态--tooltip2026-08-09`
- 反链(5):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#根级模态 `set_modal()`：鼠标/键盘事件仅路由给模态子树，模态外屏蔽；`set_modal(nullptr)` 解除
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#PanelWindow：标题栏拖拽移动 + 右下角缩放 + 关闭回调；子控件约束在客户区并随窗口平移；点击任意区域置顶（`bring_to_front()` 把自身移到父节点子控件末尾，对应 layer.setTop）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#MenuBar：横向菜单 + VBox 下拉面板（popup 机制），点击外部关闭，选中项回调
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Tooltip：`Widget::set_tooltip()` 存悬停文本，应用层在鼠标旁绘制小面板

### BUG:UI-020 · UI-020: M15 视频音频输出：AAC + 音量 + winmm 播放（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-020-m15-视频音频输出aac--音量--winmm-播放2026-08-09`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#VideoView 音频输出：AAC 解码 → swr 重采样 s16 立体声 → 音量系数 → winmm（系统库）播放；无音频流 / 解码或设备失败仅静音不影响视频
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#AudioOut（winmm）无死锁：waveOutUnprepareHeader 不在回调线程调用（回调只把播放完块移入 retired_，unprepare 由写线程 pump 统一执行）、waveOutWrite 永不持锁；seek / 循环回卷 flush 不卡死

### BUG:UI-021 · UI-021: 素材加载测试工具 + Demo 指定素材播放（2026-08-09）

- 路径: `docs/CHANGELOG.md#ui-021-素材加载测试工具--demo-指定素材播放2026-08-09`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#素材加载测试 `opengl_ui_media_load_test`：加载一律走宽字符路径（FindFirstFileW / _wfopen 自定义 AVIO / CreateFileW+imdecode），支持中文与畸形 UTF-16 文件名；`--play <秒>` 音频真实播放

### BUG:UI-022 · UI-022: M17a layui 克隆·基础设施 + 轻量组件（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-022-m17a-layui-克隆基础设施--轻量组件2026-08-10`
- 反链(6):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Widget 类名机制：`add_class/remove_class/has_class`（空格分隔多类名去重），`classes()` 供样式层遍历
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_style 解析层：`layui::has_class` 便捷查询、`layui::hex` 十六进制转色、`layui::bg_color/bg_fill` 工具色（8 个 layui-bg-*）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Theme 语义色扩展：success/info/warning/danger/border/panel/text_secondary/text_muted + `Theme::make_layui()`（浅灰底、accent #16baaa、radius 2px）；`button_normal` 为中性底色（白，非主色）——HBox/VBox/Grid/Panel/ScrollBox/TextBox/CheckBox/Radio/MenuBar/PanelWindow 等容器/控件底不染色
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Renderer `draw_polygon`：凸多边形三角扇填充（独立 flat 管线 + 动态 VBO），像素坐标 y 向下
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#轻量组件：Progress（layui-progress / layui-progress-big + layui-bg-* 色类填充、默认填充 #5fb878、`set_text_override` 自定义文本如分数"5 / 10"）、Badge（默认/rim 描边/dot 正圆 + layui-bg-* 叠加、`preferred_width()` 自适应）、Card（header+body 布局 + 两层阴影近似）、Breadcrumb（链接/当前项/分隔符 + `set_separator()` 自定义）、Timeline（左缘 #eee 竖线 + 20px 白底圆轴（内置 icon，`set_axis_icon` 换、`set_axis_rotate` 旋转动画、hover 变红描边）+ 富内容 `add_item_rich`（多行/列表/强调/图标，标题可 h3 大字））、Rate（5 星、已选实心/未选空心、hover 预览放大、点击取值、readonly）

### BUG:UI-023 · UI-023: M17a 遗留完善 + M17b 表单与按钮（layui 克隆）（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-023-m17a-遗留完善--m17b-表单与按钮layui-克隆2026-08-10`
- 反链(9):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Renderer `draw_rect_outline`：矩形描边原语（4 条薄 quad，无圆角）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Button layui 变体：layui-btn 默认/normal/warm/danger/primary + disabled（禁点击键盘）+ radius + lg/sm/xs（preferred_height）；hover opacity .8 / active 恢复；`set_badge`/`set_dot_badge` 内嵌徽章/圆点（文本右缘，叠加 layui-bg-* 换底）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Switch（layui-form-switch）：开绿底白滑块 / 关白底灰滑块，点击/Enter/Space 切换 + on_change(bool)
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ComboBox `layui-form-select`：白底 #eee 边框圆角2 + 右三角 + 聚焦绿框淡环；面板白底描边、dd 项 hover #f8f8f8、选中 layui-this 绿字；有 ScrollBox 祖先时随滚动实时翻转/钉顶，无则按窗口可视区定位（底部下拉不被窗口底边裁切）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Slider `layui-slider`：h4 #eee 轨道 + accent 填充 + 12px 白圆钮（hover 放大 1.2）+ 聚焦描边
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Tabs（layui-tab）：标题条 + 内容区，card（#fafafa 标题条 / 激活白底）/ brief（激活绿字 + 2px 绿下划线）变体，点击切换 + on_change(int)；`add_tab(title, content)` 每页可带内容区文本（切换联动）、`set_content/set_allow_close ×` 删除、`remove_tab/count` 动态操作（lay-id 语义）、标题超宽右缘展开三角弹出下拉面板；`title_width()` 把徽章/圆点计入标题宽，`set_badge/set_dot_badge` 标题右缘内嵌红徽章
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Collapse（layui-collapse）：1px #eee 外框 + #fafafa 标题条 + `set_accordion(true)` 手风琴单开 / `(false)` 非手风琴多开（open_ 集合）+ 内容区白字；`is_open/open_count/active` 查询；可作另一 Collapse 子控件无限嵌套
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Dropdown（layui-dropdown / layui-menu）：layui-btn 触发器 + 白底弹出菜单（hover #f8f8f8、分隔线 #eee），点击外部自动关闭

### BUG:UI-024 · UI-024: M17c 数据与弹层：Tree / NavMenu / layer 外观 / Laypage（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-024-m17c-数据与弹层tree--navmenu--layer-外观--laypage2026-08-10`
- 反链(5):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Tree（layui-tree）：`add_node(text,parent)` 建树 + `set_expanded/expand_all` + 点击回调；DFS 可见列表缓存（折叠子树隐藏）；点击箭头区切换展开、文本触发回调；`layui-tree-line` 虚线连接
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#NavMenu（layui-nav-tree）：`add_group/add_item` 组合一级/子项；深灰底 + 一级项高 40 + 子项高 32；点击一级展开/折叠、子项回调；当前组顶部 3px #16b777 指示条；`set_selected(group,item)` 选中子项（`.layui-this` 绿底 #16b777 白字，点击子项自动选中）；折叠组即时回收子项空间（不占布局/内容高度）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#PanelWindow layer 外观（`layui-layer`）：标题栏高 50（`title_height()` 按类切换，默认 26 不变）、白底圆角 2 + 阴影近似、白标题栏 + 1px #eee 分隔、右上角 ×；拖拽/缩放/关闭逻辑沿用 UI-019；UI-069 扩展：`set_title_visible`（title:false 无标题栏）/`set_close_visible`（closeBtn:false）/`set_maxmin`（maxmin:true 最大化还原）/`set_shade(alpha)`（layer shade 遮罩覆盖视口）/`set_auto_close(秒)`（msg time 自动关闭，需 `root->advance(dt)`）/`set_footer_buttons(labels,align,cb)` 底部按钮条 + `layui-layer-notice` 深色 `#393D49` 内容底
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Laypage（layui-laypage）：`set_total`（页数）/`set_count`（数据总数→页数 ceil(count/limit)）/`set_limit/set_limits`（limit 下拉候选）/`set_layout_str`（count/prev/page/next/limit/refresh/skip/first/last 全布局项，limit 用 ComboBox、refresh 用图标 Button、skip 用 TextBox+确定）/`set_theme_color`/`set_*_text`/`set_show_first/last`/`set_on_change/set_on_jump/set_on_refresh`；项高 28；可见页码折叠（≤7 全显，否则首页/当前±1/末页 + 省略号）；当前页主题色白字、其余白底 #eee 边框

### BUG:UI-025 · UI-025: M17c 收尾：Table layui 化 + Demo 展示页（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-025-m17c-收尾table-layui-化--demo-展示页2026-08-10`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table（layui-table）：`add_column(title,width,sortable)` + `add_row/clear_rows` + `set_on_sort/set_on_row_click` + `set_skin(TableSkin)`（Default 网格 / Line 仅行线 / Row 仅列线 / Nob 无内线）+ `set_size(TableSize)`（Normal 38 / Lg 50 / Sm 30）+ `set_even(bool)` 偶数行 #FAFAFA；表头灰底 #FAFAFA + 排序箭头（升 #16baaa / 降 #16b777）+ 行 hover 高亮 + 分隔线按皮肤 + 双击行内编辑；h=0 整表自适应 / h>0 纵向滚动；转化静态表格子页（静动态切换）

### BUG:UI-026 · UI-026: M17d layui 克隆·图标与特殊组件（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-026-m17d-layui-克隆图标与特殊组件2026-08-10`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Icon（layui-icon）：`layui_icons.h` 自动生成 195 个类名→码点常量（主区 U+E600..ECFF + 2.13.x 非私用区 U+1005..1007，非法 C++ 标识符加 `ic_` 前缀）+ `from_class()` 查表；`Font::load_icon_font()` 双段烘培同图集 + `draw_icon()` 中心绘制；`Icon` 控件按类名渲染（无 theme/set_color 时回退白色）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#特殊组件：Carousel（轮播：箭头/指示点/页码/自动播放 advance(dt)）、ColorPicker（12x3 色相网格 + 预设 + hex 显示）、Transfer（穿梭框：列表 + 移动按钮）、Code（代码块：深灰/notepad 皮肤 + 标题条 title/lang + 行号 + 超高滚动 + 嵌套）、Upload（文件列表 + 进度条模拟）

### BUG:UI-027 · UI-027: M16 收官·示例应用（终点）（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-027-m16-收官示例应用终点2026-08-10`
- 反链(3):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#image_viewer 示例应用：打开（Win32 宽字符对话框 + imdecode）/ 滚轮缩放（鼠标锚点）/ 拖拽平移 / 双击复位 / 状态栏；`--smoke` 冒烟
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#video_player 示例应用：打开（宽字符对话框）/ 播放/暂停（按钮 + 空格）/ 进度 seek / 循环 / 倍速 0.5x..2x / 音量 0..1；`--smoke` 冒烟
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#构建：`media_load_test` 依赖 FFmpeg+OpenCV 双库仅双 ON 时构建；image_viewer/video_player 分别随对应 media 开关构建；POST_BUILD 覆盖新应用 DLL/素材拷贝

### BUG:UI-028 · UI-028: layui_test 演示工程骨架 + Grid 栅格 span/offset 扩展（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-028-layuitest-演示工程骨架--grid-栅格-spanoffset-扩展2026-08-10`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Grid layui 栅格（`layui-col-md{n}` / `layui-col-md-offset{n}`）：12 列基准 span/offset 布局，行内 span 满 12 自动换行、offset 计入已用列数；无此类名时保持等宽模式
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test 页面管线：`page_registry` 注册表带 order 排序（std::map）；`show_page` 切页 `clear_children()` 释放旧页重建 + `preview/help.reset_scroll()` 滚动复位（UI-039 起由 ui 库 `Widget::clear_children` 统一递归释放，替代原应用侧 release_tree）；`--smoke <秒>` 逐页切换验证切页无崩溃、`--shot-page <n>` 首帧截图指定页；全 30 页可切换

### BUG:UI-029 · UI-029: layui_test 站式布局外壳 + 徽章页克隆（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-029-layuitest-站式布局外壳--徽章页克隆2026-08-10`
- 反链(6):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test 站式布局：顶部 layui-header 横条 + 左侧 fixed 深灰 NavMenu（layui-bg-black，分组层级）+ 右侧 ScrollBox 内容区；页内「预览/查看代码/帮助」Tabs 三栏承载 build/code/help
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test 外壳垂直布局（新镜像 docs/2 重构）：顶部深灰 `.ws-header`（#2f363c + logo 图片 240px 区 + 组件/笔记/生态(Dropdown)/2.13.9(Dropdown)/主题开关）+ 左侧白色 240px `ui::Menu` 侧栏（`ws-menu` 白底，分组 + 中文主标题 + 灰色英文副标 `add_item_sub`）+ 白色第二列子菜单（数据表格板块，`clear/add_group/add_divider/add_item`）；内容区起点 240（有子列 400）；侧栏宽 220→240
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test 页面管线：`page_registry` 注册表带 order 排序（std::map）；`show_page` 切页 `clear_children()` 释放旧页重建 + `preview/help.reset_scroll()` 滚动复位（UI-039 起由 ui 库 `Widget::clear_children` 统一递归释放，替代原应用侧 release_tree）；`--smoke <秒>` 逐页切换验证切页无崩溃、`--shot-page <n>` 首帧截图指定页；全 30 页可切换
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Icon 字体回退：`Font::load_icon_font()` 优先 exe 目录 iconfont.ttf，缺失回退 3rdparty（layui_test 仅需 glfw3.dll + iconfont.ttf + logo.png 独立运行；POST_BUILD 拷贝三件套）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#徽章页克隆 layui badge.html：区块1「小徽章、大家族」灰底三行——小圆点 layui-badge-dot 正圆 / 常规弧形 layui-badge / 描边 layui-badge-rim；区块2「与其它元素的搭配」——按钮内嵌徽章（`Button::set_badge/set_dot_badge` 叠加 layui-bg-*）、导航徽章（`NavBar::set_badge/set_dot_badge`）、选项卡标题徽章（`Tabs::set_badge/set_dot_badge`）；Badge 宽用 preferred_width() 自适应，HBox 横排

### BUG:UI-030 · UI-030: 六页基本元素克隆（面包屑/选项卡/进度条/面板/时间线/评分）（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-030-六页基本元素克隆面包屑选项卡进度条面板时间线评分2026-08-10`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#六页克隆（面包屑/选项卡/进度条/面板/时间线/评分）：Breadcrumb 链接项+当前项、Tabs 默认/card/brief 点击切换、Progress 默认/big/无文本+按钮推进、Card header+body 与 HBox 卡片集、Timeline 标题+详情、Rate 基本/只读+状态文本

### BUG:UI-031 · UI-031: 六页组件克隆（滑块/分页/下拉菜单/树形组件/基础菜单/静态表格）（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-031-六页组件克隆滑块分页下拉菜单树形组件基础菜单静态表格2026-08-10`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#六页克隆（滑块/分页/下拉菜单/树形组件/基础菜单/静态表格）：Slider 范围+键盘微调+on_change 状态文本、Laypage 折叠省略号+翻页回调、Dropdown 触发器+分隔线、Tree 三级节点+layui-tree-line+点击回调、MenuBar 横向菜单+下拉项、Table 可排序表头+行点击+双击编辑

### BUG:UI-032 · UI-032: 六页组件克隆（表单/日期/穿梭/颜色/轮播/上传）+ 控件生命周期修复（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-032-六页组件克隆表单日期穿梭颜色轮播上传-控件生命周期修复2026-08-10`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#六页克隆（表单/日期/穿梭/颜色/轮播/上传）：TextBox/ComboBox/CheckBox/RadioGroup/Switch 表单行、Transfer 两栏搬移+右列回调、ColorPicker 色板+预设+hex、Carousel 箭头/指示点、Upload 添加文件+列表
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#控件生命周期：子树释放统一走 ui 库 `Widget::clear_children()`（递归 delete 整个子树）；销毁单个控件及其子树用 `Widget::destroy()`（摘除父引用 + 递归释放子树 + delete 自身，调用后指针失效）；RadioGroup 选项为自身子控件不得在析构重复删除；ComboBox 面板挂 root 时析构先 remove_child 再删（防悬空/双重释放）；作为子节点（add_child）由 clear_children 统一释放的成员控件析构中禁止再次 delete，析构只手动释放「非子节点/挂根级弹出的外部面板」（曾致 Tabs 内容区二次释放崩溃）

### BUG:UI-033 · UI-033: 五页克隆（框架/动画/辅助元素/弹出层/数据表格）+ 根级弹出生命周期（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-033-五页克隆框架动画辅助元素弹出层数据表格-根级弹出生命周期2026-08-10`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#五页克隆（框架/动画/辅助元素/弹出层/数据表格）：AdminFrame 自绘管理布局（黑头 logo + 左导航 + 用户区[头像+"tester"+箭头] + 侧栏 `ui::NavMenu` 折叠菜单，可交互：导航 hover/点击高亮、`nav groups` 折叠菜单[menu 11/22/33]、用户区 open_popup 下拉、点一级组折叠/展开、点子项选中态、内容主体 `ScrollBox(layui-body)` 可滚动）、AnimCircle 手写补间动画 12 种、Fieldset/BlockQuote/HLine/Circle 辅助元素（ui 库组件，UI-039 下沉）、layer 弹出（PanelWindow 根级 popup + 多窗口层叠 + 9 向 offset）、Table 本地数据 9 列 8 行（5 列可排序 + 行点击 + 双击编辑）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#根级弹出生命周期：页面级弹出窗口由 PageDef.destroy 钩子统一清理（`Widget::destroy()` 摘除父引用 + 递归释放子树 + delete 自身，UI-040 起替代手写 remove_child+delete）；切页释放旧页控件不得悬空引用已清理的弹窗

### BUG:UI-034 · UI-034: 四页克隆（模板引擎/流加载/工具模块/文本行修饰）+ 克隆收尾（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-034-四页克隆模板引擎流加载工具模块文本行修饰-克隆收尾2026-08-10`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#四页克隆（模板引擎/流加载/工具模块/文本行修饰）：template 数据渲染+重新渲染（`clear_children()` 重建列表）、flow 分批加载 100 条（VBox 运行时增子控件）、util digit/timeAgo/toDateString/CountdownText 实时倒计时（ui 库组件，on_draw 读 steady_clock）、文本行修饰=layui-code 代码块组件 demo（默认/notepad/叠加/固定高度）；30/30 页全实现

### BUG:UI-035 · UI-035: 流加载滚动到底自动模式 + 窗口点击置顶（layer.setTop）（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-035-流加载滚动到底自动模式--窗口点击置顶layersettop2026-08-10`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ScrollBox 滚动到底回调：`set_on_scroll_to_bottom(cb)` 滚到底触发一次（离开底部再滚回重新武装）；传 nullptr 清除（切页清理防悬空引用）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#PanelWindow：标题栏拖拽移动 + 右下角缩放 + 关闭回调；子控件约束在客户区并随窗口平移；点击任意区域置顶（`bring_to_front()` 把自身移到父节点子控件末尾，对应 layer.setTop）

### BUG:UI-036 · UI-036: Font 中文懒烘培 + CJK 字体可配置接入（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-036-font-中文懒烘培--cjk-字体可配置接入2026-08-10`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Font CJK 懒烘培：`load_cjk_font(path, index)` 载入中文字体（`.ttc` 兼容），`load_cjk_font_auto()` 探测（env `OPENGUI_CJK_FONT` 自定义 → msyh/simhei/simsun/Deng 系统候选）；非 ASCII/图标码点按需光栅化并经 `Texture::update_sub` 局部上传到 2048x1024 图集
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Texture::update_sub(x,y,w,h,rgba) 局部区域上传（动态字体图集按需烘培用，`GL_UNPACK_ALIGNMENT=1`）

### BUG:UI-037 · UI-037: 直角矩形 50% 半透明蒙版修复（SDF 内部距离项）（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-037-直角矩形-50-半透明蒙版修复sdf-内部距离项2026-08-10`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#圆角矩形：SDF + fwidth 抗锯齿，半径 clamp 到 `min(w,h)/2`；draw_quad 为 radius=0 别名，内部必须全不透明（SDF 含内部距离项 `min(max(q.x,q.y),0)`，缺则 radius=0 内部 50% 半透明造成全界面灰蒙蒙）

### BUG:UI-038 · UI-038: ui_gl 库内置 draw_quad 不透明回归自检（2026-08-10）

- 路径: `docs/CHANGELOG.md#ui-038-uigl-库内置-drawquad-不透明回归自检2026-08-10`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Renderer init() 回归自检 `self_test_quad_opacity()`：离屏 32x32 FBO 绘制白色 draw_quad 并读回中心像素，alpha/rgb < 250 即 log_error（拦截 SDF 缺内部距离项回归，发布前可见）；shader 注释与 draw_quad API 契约同标注禁删该距离项

### BUG:UI-039 · UI-039: 轻量组件下沉 ui 库 + Grid layui-row 无背景 + clear_children 统一切页释放（2026-08-11）

- 路径: `docs/CHANGELOG.md#ui-039-轻量组件下沉-ui-库--grid-layui-row-无背景--clearchildren-统一切页释放2026-08-11`
- 反链(7):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test 页面管线：`page_registry` 注册表带 order 排序（std::map）；`show_page` 切页 `clear_children()` 释放旧页重建 + `preview/help.reset_scroll()` 滚动复位（UI-039 起由 ui 库 `Widget::clear_children` 统一递归释放，替代原应用侧 release_tree）；`--smoke <秒>` 逐页切换验证切页无崩溃、`--shot-page <n>` 首帧截图指定页；全 30 页可切换
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#控件生命周期：子树释放统一走 ui 库 `Widget::clear_children()`（递归 delete 整个子树）；销毁单个控件及其子树用 `Widget::destroy()`（摘除父引用 + 递归释放子树 + delete 自身，调用后指针失效）；RadioGroup 选项为自身子控件不得在析构重复删除；ComboBox 面板挂 root 时析构先 remove_child 再删（防悬空/双重释放）；作为子节点（add_child）由 clear_children 统一释放的成员控件析构中禁止再次 delete，析构只手动释放「非子节点/挂根级弹出的外部面板」（曾致 Tabs 内容区二次释放崩溃）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#五页克隆（框架/动画/辅助元素/弹出层/数据表格）：AdminFrame 自绘管理布局（黑头 logo + 左导航 + 用户区[头像+"tester"+箭头] + 侧栏 `ui::NavMenu` 折叠菜单，可交互：导航 hover/点击高亮、`nav groups` 折叠菜单[menu 11/22/33]、用户区 open_popup 下拉、点一级组折叠/展开、点子项选中态、内容主体 `ScrollBox(layui-body)` 可滚动）、AnimCircle 手写补间动画 12 种、Fieldset/BlockQuote/HLine/Circle 辅助元素（ui 库组件，UI-039 下沉）、layer 弹出（PanelWindow 根级 popup + 多窗口层叠 + 9 向 offset）、Table 本地数据 9 列 8 行（5 列可排序 + 行点击 + 双击编辑）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#四页克隆（模板引擎/流加载/工具模块/文本行修饰）：template 数据渲染+重新渲染（`clear_children()` 重建列表）、flow 分批加载 100 条（VBox 运行时增子控件）、util digit/timeAgo/toDateString/CountdownText 实时倒计时（ui 库组件，on_draw 读 steady_clock）、文本行修饰=layui-code 代码块组件 demo（默认/notepad/叠加/固定高度）；30/30 页全实现
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#轻量组件下沉 ui 库：Fieldset/FieldsetTitle（字段集）、BlockQuote（引用区块）、HLine（分割线）、Circle（圆徽标）、CountdownText（实时倒计时）为 ui 库组件（`ui/include/ui/ui/{fieldset,blockquote,hline,circle,countdown_text}.h` + 对应 cpp），测试程序不得再自建承载
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Grid 行容器无背景：Grid 带 `layui-row` 类即不画底色（行容器），列/单元格等其余用法保持等宽模式默认底色；apps 与 layui_test 的行容器统一用该类

### BUG:UI-040 · UI-040: Table 行重建泄漏修复 + Widget::destroy() 单控件销毁 API（2026-08-11）

- 路径: `docs/CHANGELOG.md#ui-040-table-行重建泄漏修复--widgetdestroy-单控件销毁-api2026-08-11`
- 反链(4):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#控件生命周期：子树释放统一走 ui 库 `Widget::clear_children()`（递归 delete 整个子树）；销毁单个控件及其子树用 `Widget::destroy()`（摘除父引用 + 递归释放子树 + delete 自身，调用后指针失效）；RadioGroup 选项为自身子控件不得在析构重复删除；ComboBox 面板挂 root 时析构先 remove_child 再删（防悬空/双重释放）；作为子节点（add_child）由 clear_children 统一释放的成员控件析构中禁止再次 delete，析构只手动释放「非子节点/挂根级弹出的外部面板」（曾致 Tabs 内容区二次释放崩溃）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#根级弹出生命周期：页面级弹出窗口由 PageDef.destroy 钩子统一清理（`Widget::destroy()` 摘除父引用 + 递归释放子树 + delete 自身，UI-040 起替代手写 remove_child+delete）；切页释放旧页控件不得悬空引用已清理的弹窗
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table 行重建不泄漏：`rebuild_rows()` 清旧行用 `Widget::clear_children()` 递归释放整批 TableRow（不得只 remove_child 不 delete，否则排序/双击编辑累积泄漏）

### BUG:UI-041 · UI-041: 栅格预览对齐 layui 镜像与工具色列间隙修正（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-041-栅格预览对齐-layui-镜像与工具色列间隙修正2026-08-16`
- 反链(5):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui 工具色对齐 layui.css：`layui-bg-green` #009688、`layui-bg-black` #393D49 等 8 色背景/文字色值按 layui.css 复刻，grid/badge 等工具色块不得偏色（`layui::bg_color/bg_fill` 需正确剥离 `layui-bg-` 前缀，取末段色名）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Grid 列间隔 `layui-col-space{n}`：列宽扣 n、列间留 n px 间隙、首末列贴行边，列高保持 row_h（layui 负 margin 双向撑宽语义）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#BlockQuote（layui-elem-quote）：#fafafa 底 + 左侧 5px #5FB878 色条（不得用 theme.success/accent）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#栅格演示色块：未显式着色的格子用 `grid-demo` 基础色 #79C48C（白字可见），`layui-bg-*` 工具色优先于基础色；嵌套栅格需显式断点（如 md）才按预期横向排布

### BUG:UI-042 · UI-042: API 文档生成器（tools/api_gen.py）结构化解析修复（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-042-api-文档生成器toolsapigenpy结构化解析修复2026-08-16`
- 反链(1):
  - `META:CHANGELOG` (ref) — 相关历史条目

### BUG:UI-043 · UI-043: API 总调用示例应用（apps/api_demo.cpp）与文档同步（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-043-api-总调用示例应用appsapidemocpp与文档同步2026-08-16`
- 反链(1):
  - `META:CHANGELOG` (ref) — 相关历史条目

### BUG:UI-044 · UI-044: AudioOut waveOut 回调 ABI 修复（x86 音频崩溃）（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-044-audioout-waveout-回调-abi-修复x86-音频崩溃2026-08-16`
- 反链(1):
  - `META:CHANGELOG` (ref) — 相关历史条目

### BUG:UI-045 · UI-045: 工具色前缀解析修复（layui-bg-* 全部渲染成黑色）（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-045-工具色前缀解析修复layui-bg--全部渲染成黑色2026-08-16`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui 工具色对齐 layui.css：`layui-bg-green` #009688、`layui-bg-black` #393D49 等 8 色背景/文字色值按 layui.css 复刻，grid/badge 等工具色块不得偏色（`layui::bg_color/bg_fill` 需正确剥离 `layui-bg-` 前缀，取末段色名）

### BUG:UI-046 · UI-046: Font 烘培计数误用 stbtt_BakeFontBitmap 返回值（ASCII/图标大量字形缺失）（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-046-font-烘培计数误用-stbttbakefontbitmap-返回值ascii图标大量字形缺失2026-08-16`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Font ASCII/图标全量烘培：`stbtt_BakeFontBitmap` 返回值为图集打包底边 y（bottom_y），非烘培字数；`glyphs_`/`icon_glyphs_` 必须按 `kNumChars`/`kIconNum` 全量拷贝字形度量，任意 ASCII（32..126）与图标（U+E600..ECFF 主段、U+1000..100F 辅段）字形在 `draw_text`/`measure_text` 中不得缺失、advance 不得为 0（曾致 "6/12" 只渲染 "/1"）

### BUG:UI-047 · UI-047: 布局外壳 NavMenu 折叠不回收空间 / 切页滚动残留 / 图片加载能力缺失（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-047-布局外壳-navmenu-折叠不回收空间--切页滚动残留--图片加载能力缺失2026-08-16`
- 反链(6):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#NavMenu（layui-nav-tree）：`add_group/add_item` 组合一级/子项；深灰底 + 一级项高 40 + 子项高 32；点击一级展开/折叠、子项回调；当前组顶部 3px #16b777 指示条；`set_selected(group,item)` 选中子项（`.layui-this` 绿底 #16b777 白字，点击子项自动选中）；折叠组即时回收子项空间（不占布局/内容高度）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Image 图片控件（layui `<img>`）：`load_from_file()`（gl::Texture stb_image 解码 png/jpg/bmp/webp）+ `preferred_height()` 按显示尺寸 + `on_draw` 铺满 bounds 纹理绘制；未加载空白不绘制；**裸文件名资产定位不依赖 CWD**（先 CWD、miss 回退 exe 目录同名文件，POST_BUILD 三件套即开箱可用），加载无噪音错误（全部 miss 仅 log 一次）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test 页面管线：`page_registry` 注册表带 order 排序（std::map）；`show_page` 切页 `clear_children()` 释放旧页重建 + `preview/help.reset_scroll()` 滚动复位（UI-039 起由 ui 库 `Widget::clear_children` 统一递归释放，替代原应用侧 release_tree）；`--smoke <秒>` 逐页切换验证切页无崩溃、`--shot-page <n>` 首帧截图指定页；全 30 页可切换
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Icon 字体回退：`Font::load_icon_font()` 优先 exe 目录 iconfont.ttf，缺失回退 3rdparty（layui_test 仅需 glfw3.dll + iconfont.ttf + logo.png 独立运行；POST_BUILD 拷贝三件套）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#五页克隆（框架/动画/辅助元素/弹出层/数据表格）：AdminFrame 自绘管理布局（黑头 logo + 左导航 + 用户区[头像+"tester"+箭头] + 侧栏 `ui::NavMenu` 折叠菜单，可交互：导航 hover/点击高亮、`nav groups` 折叠菜单[menu 11/22/33]、用户区 open_popup 下拉、点一级组折叠/展开、点子项选中态、内容主体 `ScrollBox(layui-body)` 可滚动）、AnimCircle 手写补间动画 12 种、Fieldset/BlockQuote/HLine/Circle 辅助元素（ui 库组件，UI-039 下沉）、layer 弹出（PanelWindow 根级 popup + 多窗口层叠 + 9 向 offset）、Table 本地数据 9 列 8 行（5 列可排序 + 行点击 + 双击编辑）

### BUG:UI-048 · UI-048: Image 资产定位依赖 CWD / 加载噪音错误（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-048-image-资产定位依赖-cwd--加载噪音错误2026-08-16`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Image 图片控件（layui `<img>`）：`load_from_file()`（gl::Texture stb_image 解码 png/jpg/bmp/webp）+ `preferred_height()` 按显示尺寸 + `on_draw` 铺满 bounds 纹理绘制；未加载空白不绘制；**裸文件名资产定位不依赖 CWD**（先 CWD、miss 回退 exe 目录同名文件，POST_BUILD 三件套即开箱可用），加载无噪音错误（全部 miss 仅 log 一次）

### BUG:UI-049 · UI-049: 框架页迷你复刻不可交互 / 用户区不符网站 / 右上角 HUD 闪烁（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-049-框架页迷你复刻不可交互--用户区不符网站--右上角-hud-闪烁2026-08-16`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#五页克隆（框架/动画/辅助元素/弹出层/数据表格）：AdminFrame 自绘管理布局（黑头 logo + 左导航 + 用户区[头像+"tester"+箭头] + 侧栏 `ui::NavMenu` 折叠菜单，可交互：导航 hover/点击高亮、`nav groups` 折叠菜单[menu 11/22/33]、用户区 open_popup 下拉、点一级组折叠/展开、点子项选中态、内容主体 `ScrollBox(layui-body)` 可滚动）、AnimCircle 手写补间动画 12 种、Fieldset/BlockQuote/HLine/Circle 辅助元素（ui 库组件，UI-039 下沉）、layer 弹出（PanelWindow 根级 popup + 多窗口层叠 + 9 向 offset）、Table 本地数据 9 列 8 行（5 列可排序 + 行点击 + 双击编辑）

### BUG:UI-050 · UI-050: 框架页内容主体可滚动 + 头部 nav groups 折叠菜单 + NavMenu 子项选中态（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-050-框架页内容主体可滚动--头部-nav-groups-折叠菜单--navmenu-子项选中态2026-08-16`
- 反链(2):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#NavMenu（layui-nav-tree）：`add_group/add_item` 组合一级/子项；深灰底 + 一级项高 40 + 子项高 32；点击一级展开/折叠、子项回调；当前组顶部 3px #16b777 指示条；`set_selected(group,item)` 选中子项（`.layui-this` 绿底 #16b777 白字，点击子项自动选中）；折叠组即时回收子项空间（不占布局/内容高度）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#五页克隆（框架/动画/辅助元素/弹出层/数据表格）：AdminFrame 自绘管理布局（黑头 logo + 左导航 + 用户区[头像+"tester"+箭头] + 侧栏 `ui::NavMenu` 折叠菜单，可交互：导航 hover/点击高亮、`nav groups` 折叠菜单[menu 11/22/33]、用户区 open_popup 下拉、点一级组折叠/展开、点子项选中态、内容主体 `ScrollBox(layui-body)` 可滚动）、AnimCircle 手写补间动画 12 种、Fieldset/BlockQuote/HLine/Circle 辅助元素（ui 库组件，UI-039 下沉）、layer 弹出（PanelWindow 根级 popup + 多窗口层叠 + 9 向 offset）、Table 本地数据 9 列 8 行（5 列可排序 + 行点击 + 双击编辑）

### BUG:UI-051 · UI-051: make_layui button_normal 误设主色导致布局容器全绿（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-051-makelayui-buttonnormal-误设主色导致布局容器全绿2026-08-16`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Theme 语义色扩展：success/info/warning/danger/border/panel/text_secondary/text_muted + `Theme::make_layui()`（浅灰底、accent #16baaa、radius 2px）；`button_normal` 为中性底色（白，非主色）——HBox/VBox/Grid/Panel/ScrollBox/TextBox/CheckBox/Radio/MenuBar/PanelWindow 等容器/控件底不染色

### BUG:UI-052 · UI-052: 按钮页三缺陷：尺寸行无大小差 / 按钮组后续按钮缺失 / 按钮容器缺失（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-052-按钮页三缺陷尺寸行无大小差--按钮组后续按钮缺失--按钮容器缺失2026-08-16`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#HBox 横向 / Grid 网格布局：子控件按序排布，Grid 按列数换行 + `preferred_height()` 自适应；ScrollBox 内可嵌套 VBox 等容器。HBox `Valign` 垂直对齐（默认 Stretch 填满 / Center 按 preferred_height 居中，layui 尺寸按钮行用）；FlowBox 流式换行容器（layui-btn-container 语义，spacing 10 自动换行）；HBox h≤0 时 `preferred_height()`=可见子高最大值+2×padding（h>0 显式优先），ScrollBox/VBox 流式排布不再塌陷

### BUG:UI-053 · UI-053: 按钮文本垂直偏移 ~4px + 圆角按钮描边矩形化（2026-08-16）

- 路径: `docs/CHANGELOG.md#ui-053-按钮文本垂直偏移-4px--圆角按钮描边矩形化2026-08-16`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#圆角描边：`draw_rounded_rect_outline(rect, radius, thickness, color)` 输出圆角环（color 片元 `uOutline`>0 时 alpha = 外缘覆盖度 × ¬内缘覆盖度，即 `a_out*(1-a_in)`，必须中空，禁止回退 `a_out*a_in` 致内部被描边色整块盖满；半径/圆角自动跟随；默认 0 走填充路径，UI-037 自检不受影响）；init() 有 `self_test_ring_opacity()` 自检读中心像素验证内部镂空；圆角按钮的焦点环 / primary / 禁用边框必须走它，禁止回退矩形 `draw_rect_outline`

### BUG:UI-054 · UI-054: 圆角按钮描边内部被描边色整块盖满（"蒙版"）——圆角环内缘误当作"再叠一层"（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-054-圆角按钮描边内部被描边色整块盖满蒙版圆角环内缘误当作再叠一层2026-08-17`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#圆角描边：`draw_rounded_rect_outline(rect, radius, thickness, color)` 输出圆角环（color 片元 `uOutline`>0 时 alpha = 外缘覆盖度 × ¬内缘覆盖度，即 `a_out*(1-a_in)`，必须中空，禁止回退 `a_out*a_in` 致内部被描边色整块盖满；半径/圆角自动跟随；默认 0 走填充路径，UI-037 自检不受影响）；init() 有 `self_test_ring_opacity()` 自检读中心像素验证内部镂空；圆角按钮的焦点环 / primary / 禁用边框必须走它，禁止回退矩形 `draw_rect_outline`

### BUG:UI-055 · UI-055: 文字像素对齐——draw_text/draw_icon 字形对齐像素网格（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-055-文字像素对齐drawtextdrawicon-字形对齐像素网格2026-08-17`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#文字像素对齐：`Font::draw_text`/`draw_icon` 字形左上角必须 `std::round` 对齐整数像素（`y+ascent_` 与 `xadvance` 累加均为小数，非整数坐标下 GL_LINEAR 双线性采样跨纹素混叠致文字发虚）；cursor 仍按浮点 advance 累加、尺寸/UV/`measure_text` 不变（行内间距与布局宽度不受影响）

### BUG:UI-056 · UI-056: 字体图集 2x 过采样——字形按 2×pixel_height 光栅化（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-056-字体图集-2x-过采样字形按-2pixelheight-光栅化2026-08-17`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#字体图集 2x 过采样：ASCII/图标/CJK 烘培统一按 `pixel_height × kAtlasScale`（=2）光栅化入图集，Glyph 度量（x0/y0/x1/y1/xadvance）乘 `kAtlasInvScale` 换算为显示像素存储、UV 指 2x 图集——`draw_text`/`measure_text`/布局零改动，UI-055 像素对齐与 UI-046 度量完整性不因过采样失效；图集 padding（kDynPad）按 kAtlasScale 放大防线性过滤串色；图标图集 1024×1024、主图集 2048×1024（2x CJK 约 1300 字容量）

### BUG:UI-057 · UI-057: 嵌套 scissor pop 不恢复上级矩形——TextBox 之后内容被裁剪（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-057-嵌套-scissor-pop-不恢复上级矩形textbox-之后内容被裁剪2026-08-17`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Renderer scissor：push_scissor/pop_scissor 可嵌套，pop 恢复上一级矩形（内部维护栈：push 与栈顶求交、pop 重设新栈顶并翻转 y，栈空才关闭 GL_SCISSOR_TEST）；禁止仅递减深度不恢复矩形

### BUG:UI-058 · UI-058: 表单页一比一复刻 layui form.html 预览内容（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-058-表单页一比一复刻-layui-formhtml-预览内容2026-08-17`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#表单页一比一复刻 layui form.html 预览：三大 Fieldset 分组（表单验证示例 / 赋值取值 / 方框风格） + BlockQuote 引用块 + 17 项表单元素（full-width / inline pair / inline triple / 密码+hint / 范围(￥-￥) / ComboBox / 级联三下拉 / CheckBox / 原始CheckBox(disabled) / Switch 关/开 / RadioGroup / TextArea / 提交+重置按钮）；Fieldset 内子控件宽度 = Fieldset.w − 24px，label 120px / inline-label 80-90px / inline-input 190px

### BUG:UI-059 · UI-059: Breadcrumb set_separator() API + 面包屑页克隆 layui nav.html 三个 demo（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-059-breadcrumb-setseparator-api--面包屑页克隆-layui-navhtml-三个-demo2026-08-17`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#轻量组件：Progress（layui-progress / layui-progress-big + layui-bg-* 色类填充、默认填充 #5fb878、`set_text_override` 自定义文本如分数"5 / 10"）、Badge（默认/rim 描边/dot 正圆 + layui-bg-* 叠加、`preferred_width()` 自适应）、Card（header+body 布局 + 两层阴影近似）、Breadcrumb（链接/当前项/分隔符 + `set_separator()` 自定义）、Timeline（左缘 #eee 竖线 + 20px 白底圆轴（内置 icon，`set_axis_icon` 换、`set_axis_rotate` 旋转动画、hover 变红描边）+ 富内容 `add_item_rich`（多行/列表/强调/图标，标题可 h3 大字））、Rate（5 星、已选实心/未选空心、hover 预览放大、点击取值、readonly）

### BUG:UI-060 · UI-060: NavBar 横向导航组件 + 导航/面包屑页完整克隆 layui nav.html（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-060-navbar-横向导航组件--导航面包屑页完整克隆-layui-navhtml2026-08-17`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#NavBar（layui-nav）：横向导航 + 下拉子菜单 + 徽章/头像 + 彩色主题；`add_item/add_child` 组合顶级/子项、`set_active` 选中态（#16b777）、`set_badge/set_dot_badge` 徽章/圆点（红 #ff5722 正圆）、`set_bg_color` 彩色背景、`set_bar_disabled` 禁用滑块跟随

### BUG:UI-061 · UI-061: Menu 基础垂直菜单组件 + menu.html 克隆（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-061-menu-基础垂直菜单组件--menuhtml-克隆2026-08-17`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Menu（基础垂直菜单）：白底 panel + 四边描边 + `compute_visibility` 分组折叠隐藏子项 + `draw_overlay` 父级悬停右侧子面板 + `draw_polygon` 箭头三角形；`add_item/add_item_sub(title,subtitle)/add_divider/add_group/add_parent/end_group` 嵌套层级 + `set_checked/set_selected(group,item)` 选中态 + `set_badge/set_dot_badge` 徽章 + `set_large` 大尺寸 + `clear()` 清空重建

### BUG:UI-062 · UI-062: Tabs 组件增强（内容区/动态操作/删除/超宽响应式）+ 选项卡页一比一复刻 demo/tab.html（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-062-tabs-组件增强内容区动态操作删除超宽响应式-选项卡页一比一复刻-demotabhtml2026-08-17`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Tabs（layui-tab）：标题条 + 内容区，card（#fafafa 标题条 / 激活白底）/ brief（激活绿字 + 2px 绿下划线）变体，点击切换 + on_change(int)；`add_tab(title, content)` 每页可带内容区文本（切换联动）、`set_content/set_allow_close ×` 删除、`remove_tab/count` 动态操作（lay-id 语义）、标题超宽右缘展开三角弹出下拉面板；`title_width()` 把徽章/圆点计入标题宽，`set_badge/set_dot_badge` 标题右缘内嵌红徽章

### BUG:UI-063 · UI-063: Tabs 析构二次释放崩溃修复（content_label_ 子节点重复 delete）（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-063-tabs-析构二次释放崩溃修复contentlabel-子节点重复-delete2026-08-17`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#控件生命周期：子树释放统一走 ui 库 `Widget::clear_children()`（递归 delete 整个子树）；销毁单个控件及其子树用 `Widget::destroy()`（摘除父引用 + 递归释放子树 + delete 自身，调用后指针失效）；RadioGroup 选项为自身子控件不得在析构重复删除；ComboBox 面板挂 root 时析构先 remove_child 再删（防悬空/双重释放）；作为子节点（add_child）由 clear_children 统一释放的成员控件析构中禁止再次 delete，析构只手动释放「非子节点/挂根级弹出的外部面板」（曾致 Tabs 内容区二次释放崩溃）

### BUG:UI-064 · UI-064: Progress 组件增强（layui-bg-* 颜色类/自定义文本/默认填充 #5fb878）+ 进度条页一比一复刻 demo/progress.html（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-064-progress-组件增强layui-bg--颜色类自定义文本默认填充-5fb878-进度条页一比一复刻-demoprogresshtml2026-08-17`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#轻量组件：Progress（layui-progress / layui-progress-big + layui-bg-* 色类填充、默认填充 #5fb878、`set_text_override` 自定义文本如分数"5 / 10"）、Badge（默认/rim 描边/dot 正圆 + layui-bg-* 叠加、`preferred_width()` 自适应）、Card（header+body 布局 + 两层阴影近似）、Breadcrumb（链接/当前项/分隔符 + `set_separator()` 自定义）、Timeline（左缘 #eee 竖线 + 20px 白底圆轴（内置 icon，`set_axis_icon` 换、`set_axis_rotate` 旋转动画、hover 变红描边）+ 富内容 `add_item_rich`（多行/列表/强调/图标，标题可 h3 大字））、Rate（5 星、已选实心/未选空心、hover 预览放大、点击取值、readonly）

### BUG:UI-065 · UI-065: 面板页一比一克隆 demo/panel.html 全部 5 组 demo + Collapse 手风琴/非手风琴双模式（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-065-面板页一比一克隆-demopanelhtml-全部-5-组-demo--collapse-手风琴非手风琴双模式2026-08-17`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Collapse（layui-collapse）：1px #eee 外框 + #fafafa 标题条 + `set_accordion(true)` 手风琴单开 / `(false)` 非手风琴多开（open_ 集合）+ 内容区白字；`is_open/open_count/active` 查询；可作另一 Collapse 子控件无限嵌套

### BUG:UI-066 · UI-066: 徽章页一比一克隆 demo/badge.html 两组 demo + Button 内嵌徽章/Tabs 标题徽章 + Badge 圆点与 NavBar 徽章修正（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-066-徽章页一比一克隆-demobadgehtml-两组-demo--button-内嵌徽章tabs-标题徽章--badge-圆点与-navbar-徽章修正2026-08-17`
- 反链(5):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#轻量组件：Progress（layui-progress / layui-progress-big + layui-bg-* 色类填充、默认填充 #5fb878、`set_text_override` 自定义文本如分数"5 / 10"）、Badge（默认/rim 描边/dot 正圆 + layui-bg-* 叠加、`preferred_width()` 自适应）、Card（header+body 布局 + 两层阴影近似）、Breadcrumb（链接/当前项/分隔符 + `set_separator()` 自定义）、Timeline（左缘 #eee 竖线 + 20px 白底圆轴（内置 icon，`set_axis_icon` 换、`set_axis_rotate` 旋转动画、hover 变红描边）+ 富内容 `add_item_rich`（多行/列表/强调/图标，标题可 h3 大字））、Rate（5 星、已选实心/未选空心、hover 预览放大、点击取值、readonly）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Button layui 变体：layui-btn 默认/normal/warm/danger/primary + disabled（禁点击键盘）+ radius + lg/sm/xs（preferred_height）；hover opacity .8 / active 恢复；`set_badge`/`set_dot_badge` 内嵌徽章/圆点（文本右缘，叠加 layui-bg-* 换底）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Tabs（layui-tab）：标题条 + 内容区，card（#fafafa 标题条 / 激活白底）/ brief（激活绿字 + 2px 绿下划线）变体，点击切换 + on_change(int)；`add_tab(title, content)` 每页可带内容区文本（切换联动）、`set_content/set_allow_close ×` 删除、`remove_tab/count` 动态操作（lay-id 语义）、标题超宽右缘展开三角弹出下拉面板；`title_width()` 把徽章/圆点计入标题宽，`set_badge/set_dot_badge` 标题右缘内嵌红徽章
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#NavBar（layui-nav）：横向导航 + 下拉子菜单 + 徽章/头像 + 彩色主题；`add_item/add_child` 组合顶级/子项、`set_active` 选中态（#16b777）、`set_badge/set_dot_badge` 徽章/圆点（红 #ff5722 正圆）、`set_bg_color` 彩色背景、`set_bar_disabled` 禁用滑块跟随
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#徽章页克隆 layui badge.html：区块1「小徽章、大家族」灰底三行——小圆点 layui-badge-dot 正圆 / 常规弧形 layui-badge / 描边 layui-badge-rim；区块2「与其它元素的搭配」——按钮内嵌徽章（`Button::set_badge/set_dot_badge` 叠加 layui-bg-*）、导航徽章（`NavBar::set_badge/set_dot_badge`）、选项卡标题徽章（`Tabs::set_badge/set_dot_badge`）；Badge 宽用 preferred_width() 自适应，HBox 横排

### BUG:UI-067 · UI-067: 时间线页一比一克隆 demo/timeline.html 两组 demo + Timeline 富内容/轴点图标/悬停反馈/旋转动画（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-067-时间线页一比一克隆-demotimelinehtml-两组-demo--timeline-富内容轴点图标悬停反馈旋转动画2026-08-17`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#轻量组件：Progress（layui-progress / layui-progress-big + layui-bg-* 色类填充、默认填充 #5fb878、`set_text_override` 自定义文本如分数"5 / 10"）、Badge（默认/rim 描边/dot 正圆 + layui-bg-* 叠加、`preferred_width()` 自适应）、Card（header+body 布局 + 两层阴影近似）、Breadcrumb（链接/当前项/分隔符 + `set_separator()` 自定义）、Timeline（左缘 #eee 竖线 + 20px 白底圆轴（内置 icon，`set_axis_icon` 换、`set_axis_rotate` 旋转动画、hover 变红描边）+ 富内容 `add_item_rich`（多行/列表/强调/图标，标题可 h3 大字））、Rate（5 星、已选实心/未选空心、hover 预览放大、点击取值、readonly）

### BUG:UI-068 · UI-068: 静态表格页一比一克隆 demo/table-element.html 全部五组 demo + Table 皮肤/尺寸/偶数行着色（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-068-静态表格页一比一克隆-demotable-elementhtml-全部五组-demo--table-皮肤尺寸偶数行着色2026-08-17`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table（layui-table）：`add_column(title,width,sortable)` + `add_row/clear_rows` + `set_on_sort/set_on_row_click` + `set_skin(TableSkin)`（Default 网格 / Line 仅行线 / Row 仅列线 / Nob 无内线）+ `set_size(TableSize)`（Normal 38 / Lg 50 / Sm 30）+ `set_even(bool)` 偶数行 #FAFAFA；表头灰底 #FAFAFA + 排序箭头（升 #16baaa / 降 #16b777）+ 行 hover 高亮 + 分隔线按皮肤 + 双击行内编辑；h=0 整表自适应 / h>0 纵向滚动；转化静态表格子页（静动态切换）

### BUG:UI-069 · UI-069: 弹出层页一比一克隆 demo/layer.html（多窗口置顶 Esc / 透明询问框 / 公告层遮罩）+ PanelWindow 扩展（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-069-弹出层页一比一克隆-demolayerhtml多窗口置顶-esc--透明询问框--公告层遮罩-panelwindow-扩展2026-08-17`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#PanelWindow layer 外观（`layui-layer`）：标题栏高 50（`title_height()` 按类切换，默认 26 不变）、白底圆角 2 + 阴影近似、白标题栏 + 1px #eee 分隔、右上角 ×；拖拽/缩放/关闭逻辑沿用 UI-019；UI-069 扩展：`set_title_visible`（title:false 无标题栏）/`set_close_visible`（closeBtn:false）/`set_maxmin`（maxmin:true 最大化还原）/`set_shade(alpha)`（layer shade 遮罩覆盖视口）/`set_auto_close(秒)`（msg time 自动关闭，需 `root->advance(dt)`）/`set_footer_buttons(labels,align,cb)` 底部按钮条 + `layui-layer-notice` 深色 `#393D49` 内容底

### BUG:UI-070 · UI-070: 弹出层页触发按钮零宽重叠（无点击反馈）修复（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-070-弹出层页触发按钮零宽重叠无点击反馈修复2026-08-17`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#PanelWindow layer 外观（`layui-layer`）：标题栏高 50（`title_height()` 按类切换，默认 26 不变）、白底圆角 2 + 阴影近似、白标题栏 + 1px #eee 分隔、右上角 ×；拖拽/缩放/关闭逻辑沿用 UI-019；UI-069 扩展：`set_title_visible`（title:false 无标题栏）/`set_close_visible`（closeBtn:false）/`set_maxmin`（maxmin:true 最大化还原）/`set_shade(alpha)`（layer shade 遮罩覆盖视口）/`set_auto_close(秒)`（msg time 自动关闭，需 `root->advance(dt)`）/`set_footer_buttons(labels,align,cb)` 底部按钮条 + `layui-layer-notice` 深色 `#393D49` 内容底

### BUG:UI-071 · UI-071: 弹窗销毁后根节点 popups_ 残留悬空指针崩溃（读取访问权限冲突）（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-071-弹窗销毁后根节点-popups-残留悬空指针崩溃读取访问权限冲突2026-08-17`
- 反链(3):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#PanelWindow layer 外观（`layui-layer`）：标题栏高 50（`title_height()` 按类切换，默认 26 不变）、白底圆角 2 + 阴影近似、白标题栏 + 1px #eee 分隔、右上角 ×；拖拽/缩放/关闭逻辑沿用 UI-019；UI-069 扩展：`set_title_visible`（title:false 无标题栏）/`set_close_visible`（closeBtn:false）/`set_maxmin`（maxmin:true 最大化还原）/`set_shade(alpha)`（layer shade 遮罩覆盖视口）/`set_auto_close(秒)`（msg time 自动关闭，需 `root->advance(dt)`）/`set_footer_buttons(labels,align,cb)` 底部按钮条 + `layui-layer-notice` 深色 `#393D49` 内容底
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#控件生命周期：子树释放统一走 ui 库 `Widget::clear_children()`（递归 delete 整个子树）；销毁单个控件及其子树用 `Widget::destroy()`（摘除父引用 + 递归释放子树 + delete 自身，调用后指针失效）；RadioGroup 选项为自身子控件不得在析构重复删除；ComboBox 面板挂 root 时析构先 remove_child 再删（防悬空/双重释放）；作为子节点（add_child）由 clear_children 统一释放的成员控件析构中禁止再次 delete，析构只手动释放「非子节点/挂根级弹出的外部面板」（曾致 Tabs 内容区二次释放崩溃）

### BUG:UI-072 · UI-072: 弹出层页按钮回调捕获栈局部 PageHost 悬空引用崩溃（open_layer 读取访问冲突）（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-072-弹出层页按钮回调捕获栈局部-pagehost-悬空引用崩溃openlayer-读取访问冲突2026-08-17`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#PanelWindow layer 外观（`layui-layer`）：标题栏高 50（`title_height()` 按类切换，默认 26 不变）、白底圆角 2 + 阴影近似、白标题栏 + 1px #eee 分隔、右上角 ×；拖拽/缩放/关闭逻辑沿用 UI-019；UI-069 扩展：`set_title_visible`（title:false 无标题栏）/`set_close_visible`（closeBtn:false）/`set_maxmin`（maxmin:true 最大化还原）/`set_shade(alpha)`（layer shade 遮罩覆盖视口）/`set_auto_close(秒)`（msg time 自动关闭，需 `root->advance(dt)`）/`set_footer_buttons(labels,align,cb)` 底部按钮条 + `layui-layer-notice` 深色 `#393D49` 内容底

### BUG:UI-073 · UI-073: 事件分发期间 widget 自毁悬空崩溃（flush 延迟销毁修复）（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-073-事件分发期间-widget-自毁悬空崩溃flush-延迟销毁修复2026-08-17`
- 反链(2):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#焦点抽象：dispatch_mouse_down 命中即 focus()（子类无需自行聚焦）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#PanelWindow layer 外观（`layui-layer`）：标题栏高 50（`title_height()` 按类切换，默认 26 不变）、白底圆角 2 + 阴影近似、白标题栏 + 1px #eee 分隔、右上角 ×；拖拽/缩放/关闭逻辑沿用 UI-019；UI-069 扩展：`set_title_visible`（title:false 无标题栏）/`set_close_visible`（closeBtn:false）/`set_maxmin`（maxmin:true 最大化还原）/`set_shade(alpha)`（layer shade 遮罩覆盖视口）/`set_auto_close(秒)`（msg time 自动关闭，需 `root->advance(dt)`）/`set_footer_buttons(labels,align,cb)` 底部按钮条 + `layui-layer-notice` 深色 `#393D49` 内容底

### BUG:UI-074 · UI-074: PanelWindow 子控件定位在主窗口左上角（on_layout 空操作）（2026-08-17）

- 路径: `docs/CHANGELOG.md#ui-074-panelwindow-子控件定位在主窗口左上角onlayout-空操作2026-08-17`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#PanelWindow layer 外观（`layui-layer`）：标题栏高 50（`title_height()` 按类切换，默认 26 不变）、白底圆角 2 + 阴影近似、白标题栏 + 1px #eee 分隔、右上角 ×；拖拽/缩放/关闭逻辑沿用 UI-019；UI-069 扩展：`set_title_visible`（title:false 无标题栏）/`set_close_visible`（closeBtn:false）/`set_maxmin`（maxmin:true 最大化还原）/`set_shade(alpha)`（layer shade 遮罩覆盖视口）/`set_auto_close(秒)`（msg time 自动关闭，需 `root->advance(dt)`）/`set_footer_buttons(labels,align,cb)` 底部按钮条 + `layui-layer-notice` 深色 `#393D49` 内容底

### BUG:UI-075 · UI-075: Laydate 控件扩展 + laydate_page 17 个 demo（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-075-laydate-控件扩展--laydatepage-17-个-demo2026-08-18`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Laydate 扩展（日期/时间选择器全类型）：Type 枚举（Year/Month/Date/Time/DateTime）、range 范围选择、format 自定义格式串（y/M/d/H/m/s 占位符）、min/max 约束、calendar 公历节日 + marks[] 自定义标注、theme_style 主题（默认/molv/grid/#rrggbb）、show_bottom/btns[] 底部栏、lang:'en' 国际化、on_done/on_change/on_ready 三回调；Colors 结构体 21 色 + draw_*/hit_* 绘制与命中测试辅助；`value()` 写回字符串 + `date_selectable` min/max 真实限选（含 `set_min_offset/set_max_offset` 相对今天偏移）；laydate_page 17 个 demo

### BUG:UI-076 · UI-076: Laydate 弹层（input+popup）模式 + laydate_page 40 demo 一比一克隆（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-076-laydate-弹层inputpopup模式--laydatepage-40-demo-一比一克隆2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#LaydateBox（laydate 弹层模式，input+popup）：视觉 = 输入框（白底/边框/占位/三角/只读）+ 点击下方根级弹出 Laydate 面板（仿 ComboBox `open_popup` 先例，外部点击/Esc 自动关闭）；面板类型/range/format/min-max/节日标注/主题/lang/btns/showBottom 全透传；选中/确定写回 `text_` + `on_change` 回调；对应 mirror laydate.html test1..test31 弹层交互；laydate_page 10 分组 40 demo 一目了

### BUG:UI-077 · UI-077: 数据表格分阶段克隆 1：分页/自定义分页/复选框/单选框/单元格表单（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-077-数据表格分阶段克隆-1分页自定义分页复选框单选框单元格表单2026-08-18`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table 分页/选择/单元格表单（数据表格分阶段克隆 1）：`set_paginated(on,page_size)` 表底挂 Laypage 分页条（页数 = ceil(rows/page_size)，`visible_rows_` 可见行映射 + `data_row_of()` 定位数据行）；`add_checkbox_column()`/`add_radio_column()`/`add_numbers_column()`（ColKind 枚举）+ `set_on_selection(visible_row,checked)` + `row_checked`/`checked_radio`/`checked_count`/`first_checked` 查询；`set_cell_switch(col)`/`set_cell_checkbox(col)` 单元格内嵌 Switch/CheckBox（on = 行文本 "1"/"true"）；表头全选/逐行勾选/跨页连续序号、复选/单选/表单原生绘制 + on_mouse_down 命中切换；对应 mirror table/page、resetPage、checkbox、radio、form 五个子页一比一（table_data2_page order 46）

### BUG:UI-078 · UI-078: 数据表格分阶段克隆 2：合计行/头部工具栏/单元格事件/操作列（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-078-数据表格分阶段克隆-2合计行头部工具栏单元格事件操作列2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table 合计/工具栏/操作（数据表格分阶段克隆 2）：`set_total_row(bool)`+`set_total_col(col)`+`set_total_row_text()` 表尾合计行（首列文案 + total_col 对当前页求和，随皮肤分隔线，on_draw 固定于数据区底部）；`add_button_column(title,width,labels,events)` 行内操作按钮列（ColKind::Button，查看/编辑/删除 原生绘制，on_mouse_down 反算按钮中心命中 → `set_on_action(visible_row,event)`）；`set_cell_event_col(col)`+`set_on_cell_event(visible_row,col)` 单元格点击事件（mirror cellEvent.html 签名列）；对应 mirror table/totalRow、toolbar、cellEvent、operate 四个子页一比一（table_data3_page order 47）

### BUG:UI-079 · UI-079: ScrollBox 切页滚动条闪烁修复（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-079-scrollbox-切页滚动条闪烁修复2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ScrollBox 滚动容器：内容超高可滚，scissor 裁剪可视区；滚轮滚动无溢出不消费；右缘轨道+滑块指示，拖动改值；布局让出滚动条列；子节点变化（add/remove/clear）标记重布局，抑制切页后滚动条闪现

### BUG:UI-080 · UI-080: 数据表格缺失子页补全：转化静态表格（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-080-数据表格缺失子页补全转化静态表格2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table（layui-table）：`add_column(title,width,sortable)` + `add_row/clear_rows` + `set_on_sort/set_on_row_click` + `set_skin(TableSkin)`（Default 网格 / Line 仅行线 / Row 仅列线 / Nob 无内线）+ `set_size(TableSize)`（Normal 38 / Lg 50 / Sm 30）+ `set_even(bool)` 偶数行 #FAFAFA；表头灰底 #FAFAFA + 排序箭头（升 #16baaa / 降 #16b777）+ 行 hover 高亮 + 分隔线按皮肤 + 双击行内编辑；h=0 整表自适应 / h>0 纵向滚动；转化静态表格子页（静动态切换）

### BUG:UI-081 · UI-081: 分页组件全能力补齐 + laypage 页一比一克隆 demo/laypage.html 全部 demo（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-081-分页组件全能力补齐--laypage-页一比一克隆-demolaypagehtml-全部-demo2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Laypage（layui-laypage）：`set_total`（页数）/`set_count`（数据总数→页数 ceil(count/limit)）/`set_limit/set_limits`（limit 下拉候选）/`set_layout_str`（count/prev/page/next/limit/refresh/skip/first/last 全布局项，limit 用 ComboBox、refresh 用图标 Button、skip 用 TextBox+确定）/`set_theme_color`/`set_*_text`/`set_show_first/last`/`set_on_change/set_on_jump/set_on_refresh`；项高 28；可见页码折叠（≤7 全显，否则首页/当前±1/末页 + 省略号）；当前页主题色白字、其余白底 #eee 边框

### BUG:UI-082 · UI-082: 下拉菜单（Dropdown）完整化 + 1:1 镜像 layui dropdown.html（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-082-下拉菜单dropdown完整化--11-镜像-layui-dropdownhtml2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Dropdown 下拉菜单完整化（layui dropdown.html 1:1）：四种触发器外观（Button/PrimaryButton/Input/Text）+ 事件（Click/Hover/MouseDown/DblClick/ContextMenu/Manual）+ 对齐（Left/Center/Right）+ 层级/分组/分隔线 + `set_popup_style` 重定义配色 + `set_custom_content` 自定义内容 + `set_region` 右键占位区 + 菜单项自动收起回调 `set_on_select(title)`；Menu 增 `set_on_item_click` 选中回调

### BUG:UI-083 · UI-083: 文件上传 Upload 测试页 1:1 镜像 upload.html 全部 9 个 demo（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-083-文件上传-upload-测试页-11-镜像-uploadhtml-全部-9-个-demo2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Upload 文件上传测试页 1:1 镜像 upload.html 全部 9 个 demo：普通图片上传 / 多文件列表（Table 文件名·大小·进度·操作）/ 多图片预览（BlockQuote）/ 指定文件类型（accept·exts·video·audio）/ 大小限制 / 多元素绑定 / 选完不自动上传 / 拖拽上传（upload-drag）/ 原始文件域；结构用 FieldsetTitle + BlockQuote + HBox + Panel + Progress + Image + Table 组合

### BUG:UI-084 · UI-084: 穿梭框 Transfer 增强 + 1:1 镜像 transfer.html 全部 7 个 demo（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-084-穿梭框-transfer-增强--11-镜像-transferhtml-全部-7-个-demo2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Transfer 穿梭框增强并 1:1 镜像 transfer.html 全部 7 个 demo：初始右侧数据 `add_right_item` / 禁用项 `add_item(name, disabled)` / 显示搜索框 `set_show_search`（TextBox 子控件过滤）/ 方向化穿梭回调 `set_on_change(TransferDirection, moved)` / `clear()` 实例重载 / 复选框选择（□/☑，accent色填充+白色✓）；基础效果 / 定义标题及数据源 / 初始右侧 / 搜索 / 数据格式 / 穿梭回调 / 实例调用

### BUG:UI-085 · UI-085: 树形组件 Tree 增强 + 1:1 镜像 tree.html 全部 8 个 demo（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-085-树形组件-tree-增强--11-镜像-treehtml-全部-8-个-demo2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Tree 树形组件增强并 1:1 镜像 tree.html 全部 8 个 demo：节点数据 `TreeNodeSpec`（title/id/field/href/disabled/fixed/checked/spread）/ 复选框 `set_show_checkbox`+`set_checked`（子孙传播+父级半选）+`get_checked`+`check_all` / 无连接线 `set_show_line(false)` / 仅图标控制 `set_only_icon_control` / 手风琴 `set_accordion` / 跳转 `set_is_jump` / 操作图标 `set_editable`+`set_on_edit`；基本演示 / 常规用法 / 无连接线 / 仅图标控制 / 手风琴 / 跳转 / 复选框 / 操作图标

### BUG:UI-086 · UI-086: 颜色选择器 ColorPicker 增强 + 1:1 镜像 colorpicker.html 全部 demo（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-086-颜色选择器-colorpicker-增强--11-镜像-colorpickerhtml-全部-demo2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ColorPicker 颜色选择器增强并 1:1 镜像 colorpicker.html 全部 7 组 demo：格式 `set_format(Hex/Rgb)`（alpha 时自动 rgba）/ 透明度滑块 `set_alpha` / 预定义色 `set_predefine`+`set_colors` / 尺寸 `set_size(Lg/Sm/Xs)` / done 回调 `set_on_done`（确认/清除按钮）+ change 回调 / 清除 `set_clearable`；layout 升级为 SV 色场 + 色相/透明度滑块 + 格式文本 + 预定义色 + 确认/清除；常规使用 / 表单赋值 / RGB·RGBA / 透明度 / 预定义 / 全功能回调 / 尺寸

### BUG:UI-087 · UI-087: 滑块 Slider 增强 + 1:1 镜像 slider.html 全部 18 个 demo（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-087-滑块-slider-增强--11-镜像-sliderhtml-全部-18-个-demo2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Slider 滑块增强并 1:1 镜像 slider.html 全部 18 个 demo：range 范围双环 `set_range_enabled`+`set_range_value`+`set_on_range_change` / 步长 `set_step` / 间断点 `set_showstep` / 提示 `set_tips`+`set_tips_callback`（setTips）/ 输入框 `set_input`（数值+上下箭头）/ 垂直 `set_vertical` / 禁用 `set_disabled` / 自定义主题色 `set_theme_color`；基本 / 初始值 / 最大最小值 / 步长 / 提示文本 / 输入框 / 范围选择 / 垂直滑块 / 自定义颜色 / 禁用滑块

### BUG:UI-088 · UI-088: 评分 Rate 增强 + 1:1 镜像 rate.html 全部 14 个 demo（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-088-评分-rate-增强--11-镜像-ratehtml-全部-14-个-demo2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Rate 评分增强并 1:1 镜像 rate.html 全部 14 个 demo：星数 `set_length` / 半星 `set_half` / 文本 `set_text`+`set_text_callback`（setText）/ 自定义主题色 `set_theme_color` / 浮点取值 `set_value(float)`+`value_f()`；基础 / 显示文字 / 半星效果 / 自定义内容 / 自定义长度 / 只读 / 自定义主题色

### BUG:UI-089 · UI-089: 轮播 Carousel 增强 + 1:1 镜像 carousel.html 三个 demo 组（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-089-轮播-carousel-增强--11-镜像-carouselhtml-三个-demo-组2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Carousel 轮播增强并 1:1 镜像 carousel.html 三个 demo 组：动画 `set_anim(Default/Updown/Fade)` / 箭头 `set_arrow(Hover/Always/None)` / 指示器 `set_indicator(Inside/Outside/None)` / 自动轮播 `set_autoplay`+`set_interval(毫秒)` / 图片页 `add_image_slide`（含 exe 目录回退） / 运行时 `set_size`；常规轮播 / 设定各种参数（交互，HBox 行）/ 填充图片（file/img 图片 build 拷贝）

### BUG:UI-090 · UI-090: 轮播页布局修正 + 图片改用 file/img（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-090-轮播页布局修正--图片改用-fileimg2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#轮播页布局修正（控制行包 HBox 行）+ 图片改取 file/img（build 拷到 exe 目录）

### BUG:UI-091 · UI-091: Carousel add_image_slide 缺 exe 目录回退（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-091-carousel-addimageslide-缺-exe-目录回退2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Carousel 轮播增强并 1:1 镜像 carousel.html 三个 demo 组：动画 `set_anim(Default/Updown/Fade)` / 箭头 `set_arrow(Hover/Always/None)` / 指示器 `set_indicator(Inside/Outside/None)` / 自动轮播 `set_autoplay`+`set_interval(毫秒)` / 图片页 `add_image_slide`（含 exe 目录回退） / 运行时 `set_size`；常规轮播 / 设定各种参数（交互，HBox 行）/ 填充图片（file/img 图片 build 拷贝）

### BUG:UI-092 · UI-092: 模板引擎 laytpl 增强 + 1:1 镜像 laytpl.html 沙箱（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-092-模板引擎-laytpl-增强--11-镜像-laytplhtml-沙箱2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#模板引擎 laytpl 重写：新增 ui::LayTpl 引擎（`{{ d.field }}`/`{{= }}` 转义/`{{# }}` 逻辑 if·each/`{{! !}}` 过滤/自定义分隔符），template_page 1:1 镜像 laytpl.html 实时沙箱（数据+模版文本框 → 视图实时重渲染）

### BUG:UI-093 · UI-093: 流加载页 1:1 克隆镜像 3 演示 + ui 库图片懒加载（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-093-流加载页-11-克隆镜像-3-演示--ui-库图片懒加载2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#图片懒加载（lay-src）：`Image::set_lay_src()` on_draw 用 `Renderer::current_scissor()`（当前可视屏）判进入可视区才解码，只加载一次；显式尺寸时加载前也预留高度（列表布局不塌陷）

### BUG:UI-094 · UI-094: 流加载滚动加载修复：ScrollBox 触底回调加 mb 临界距离（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-094-流加载滚动加载修复scrollbox-触底回调加-mb-临界距离2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ScrollBox 触底临界距离：`set_bottom_margin(mb)` 距底部不足 mb 即触发触底回调（镜像 layui flow.mb 语义）；mb 默认 0 = 严格滚到底，不破坏 UI-035

### BUG:UI-095 · UI-095: 流加载页回调捕获栈局部 PageHost 悬空崩溃（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-095-流加载页回调捕获栈局部-pagehost-悬空崩溃2026-08-18`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#页面回调生命周期：PageHost 为 **main 作用域长命对象**（UI-110 结构性修复），页面 build 里 `[&host]` 捕获合法；历史上 host 曾是 render_page lambda 栈局部变量、回调触发即悬空（UI-095 首次暴露，当时以"回调只捕获长期对象"规避）

### BUG:UI-096 · UI-096: 文本行修饰页改克隆 layui-code 代码块组件 demo（2026-08-18）

- 路径: `docs/CHANGELOG.md#ui-096-文本行修饰页改克隆-layui-code-代码块组件-demo2026-08-18`
- 反链(2):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#特殊组件：Carousel（轮播：箭头/指示点/页码/自动播放 advance(dt)）、ColorPicker（12x3 色相网格 + 预设 + hex 显示）、Transfer（穿梭框：列表 + 移动按钮）、Code（代码块：深灰/notepad 皮肤 + 标题条 title/lang + 行号 + 超高滚动 + 嵌套）、Upload（文件列表 + 进度条模拟）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#四页克隆（模板引擎/流加载/工具模块/文本行修饰）：template 数据渲染+重新渲染（`clear_children()` 重建列表）、flow 分批加载 100 条（VBox 运行时增子控件）、util digit/timeAgo/toDateString/CountdownText 实时倒计时（ui 库组件，on_draw 读 steady_clock）、文本行修饰=layui-code 代码块组件 demo（默认/notepad/叠加/固定高度）；30/30 页全实现

### BUG:UI-097 · UI-097: 项目打包分发机制（2026-08-19）

- 路径: `docs/CHANGELOG.md#ui-097-项目打包分发机制2026-08-19`
- 反链(1):
  - `META:CHANGELOG` (ref) — 相关历史条目

### BUG:UI-098 · UI-098: 复选框/开关补齐 layui 全皮肤能力 + 复选框页 1:1 克隆（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-098-复选框开关补齐-layui-全皮肤能力--复选框页-11-克隆2026-08-21`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#CheckBox/Switch layui 全皮肤：CheckBox Primary 对勾 + `set_indeterminate` 半选（绿框白底横线）+ `set_skin(Tag)`（灰块+右侧 30px 方框）+ `set_skin(None)` 无默认样式 + `set_custom_content` lay-checkbox 绑定；Switch 双态文本（ON 竖线 OFF 分隔，开=左段关=右段）+ `set_custom_content` 开关自定义标题；复选框页 6 区块 1:1 克隆 checkbox.html

### BUG:UI-099 · UI-099: 外壳垂直布局按新镜像重构 + 页面分组映射（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-099-外壳垂直布局按新镜像重构--页面分组映射2026-08-21`
- 反链(2):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test 外壳垂直布局（新镜像 docs/2 重构）：顶部深灰 `.ws-header`（#2f363c + logo 图片 240px 区 + 组件/笔记/生态(Dropdown)/2.13.9(Dropdown)/主题开关）+ 左侧白色 240px `ui::Menu` 侧栏（`ws-menu` 白底，分组 + 中文主标题 + 灰色英文副标 `add_item_sub`）+ 白色第二列子菜单（数据表格板块，`clear/add_group/add_divider/add_item`）；内容区起点 240（有子列 400）；侧栏宽 220→240
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Menu（基础垂直菜单）：白底 panel + 四边描边 + `compute_visibility` 分组折叠隐藏子项 + `draw_overlay` 父级悬停右侧子面板 + `draw_polygon` 箭头三角形；`add_item/add_item_sub(title,subtitle)/add_divider/add_group/add_parent/end_group` 嵌套层级 + `set_checked/set_selected(group,item)` 选中态 + `set_badge/set_dot_badge` 徽章 + `set_large` 大尺寸 + `clear()` 清空重建

### BUG:UI-100 · UI-100: IME 候选窗跟随输入框光标（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-100-ime-候选窗跟随输入框光标2026-08-21`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#IME 候选窗跟随：`update_ime_caret(window, root)` 每帧锚定聚焦 TextBox 光标（ImmSetCompositionWindow/CandidateWindow，屏幕坐标不变不重设，非 Windows 空操作）；TextBox::caret_offset 公开

### BUG:UI-101 · UI-101: TextBox 选择/剪贴板能力（拖选/Shift 选择/Ctrl+A/C/X/V）（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-101-textbox-选择剪贴板能力拖选shift-选择ctrlacxv2026-08-21`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#TextBox 选择/剪贴板：Shift+方向/Home/End 与鼠标拖选（accent 高亮）、Ctrl+A/C/X/V 全选/复制/剪切/粘贴（Window::primary_handle 剪贴板句柄）、Backspace/Delete/输入优先删选中

### BUG:UI-102 · UI-102: 键盘修饰键快照——SendInput 粘贴到自绘输入框丢失 mods（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-102-键盘修饰键快照sendinput-粘贴到自绘输入框丢失-mods2026-08-21`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#键盘 mods 快照：key_cb PRESS 时记录修饰键，`key_pressed_mods(key)` 供应用在边沿消费时取用（SendInput 快速序列实时查询丢 mods）；TextBox::on_char 过滤 C0/DEL 控制字符

### BUG:UI-103 · UI-103: TextArea 剪贴板默认实现——回调未注入时 Ctrl+C/X/V 静默失效（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-103-textarea-剪贴板默认实现回调未注入时-ctrlcxv-静默失效2026-08-21`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#TextArea 剪贴板兜底：未注入 on_copy_/on_paste_ 时内置 GLFW 剪贴板读写（primary_handle），已注入时优先覆盖；消除「漏注入即静默失效」

### BUG:UI-104 · UI-104: 侧栏分组逐级嵌套成子级折叠菜单——Menu add_group 未配对 end_group（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-104-侧栏分组逐级嵌套成子级折叠菜单menu-addgroup-未配对-endgroup2026-08-21`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test 侧栏分组同级：主侧栏/第二列构建时 `Menu::add_group` 必须与 `end_group()` 配对闭合（换组前闭合上一组），分组组头全部同级、页项缩进一级；折叠任一分组只隐藏本组页项——漏配对会使后续分组逐级嵌套成子级折叠菜单，且折叠首组隐藏其后全部项

### BUG:UI-105 · UI-105: 帧率 HUD 与头部右侧控件重叠——HUD 右对齐压在生态/版本下拉上（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-105-帧率-hud-与头部右侧控件重叠hud-右对齐压在生态版本下拉上2026-08-21`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test 帧率 HUD 绘于头部中段空区（`x = kSidebarW + 16`，logo 区之后），不与头部右侧控件群（主题开关/生态/2.13.9 下拉/组件/笔记）重叠；HUD 每秒 tick、逐帧重绘不闪烁不变——禁止改回 `width - text_width` 右对齐（会压上下拉框）

### BUG:UI-106 · UI-106: 侧栏缺失镜像站 14 个页面——基础组整组缺 + 颜色/图标/输入框/选择框/单选框/树形表格/公共类/固定条（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-106-侧栏缺失镜像站-14-个页面基础组整组缺--颜色图标输入框选择框单选框树形表格公共类固定条2026-08-21`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test 侧栏与镜像 docs/2 菜单树对齐：7 组顺序 基础(6)/布局(2)/通用(4)/表单(5)/展示(18)/交互(8)/其他(2)，组内次序同镜像（通用 颜色→按钮→图标→动画；表单 表单→输入框→选择框→复选框→单选框；展示含 树形表格/公共类/固定条 插位）；未实现页走 `placeholder_pages.cpp` 占位注册（导航项 + 「尚未克隆」提示），新增页先在此数组登记并按镜像插位排 order——禁止绕过占位机制硬编码导航项

### BUG:UI-107 · UI-107: UI-106 的 14 个占位页全部实现为真实内容页（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-107-ui-106-的-14-个占位页全部实现为真实内容页2026-08-21`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui_test UI-106 的 14 个占位页全部为真实内容页：基础组 6 页文档摘要（basic_pages）、颜色页（UI-112 起一比一克隆镜像色彩页）、图标页（[UI-113](CHANGELOG.md#ui-113-图标页新增按钮图标段克隆镜像-h2icon2026-08-22) 新增按钮图标段克隆镜像 h2#icon）、输入框/选择框/单选框真实交互演示、树形表格静态还原、公共类工具类演示（字号缩放/加粗因 Font 固定像素烘培以说明呈现，禁止做假缩放）、固定条（返回顶部真实复位 content 滚动）；占位数组保留哨兵机制供后续新增页

### BUG:UI-108 · UI-108: Font/Label 字号缩放与仿粗体 + layui 文字工具色 font_color——公共类页升级真实演示（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-108-fontlabel-字号缩放与仿粗体--layui-文字工具色-fontcolor公共类页升级真实演示2026-08-21`
- 反链(2):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Font 缩放与仿粗体：`Font::draw_text(..., scale=1.f, bold=false)` 全度量乘 scale（≤2x 清晰，图集 2x 过采样）+ 仿粗体 x 错位叠加；`Label::set_text_scale/set_bold` 行高/换行/测量全链路联动；默认参数下既有调用零影响——禁止在测试程序内自建多份 Font 实例或手动错位双绘实现加粗
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#layui 文字工具色查询：`ui::layui::font_color("layui-font-*")` 返回 .layui-font-{red orange green cyan blue purple black gray} 文字色（克隆 layui.css 2.13.x：green=#16baaa/blue=#01aaed/black=#000/gray=#c2c2c2 与 bg-\* 不同源），未知类回退黑；bg_color/bg_fill 行为不变——字色行禁用 bg_color()（其 text 字段是底色上的白前景，白底面板上不可见）

### BUG:UI-109 · UI-109: Table 树形层级 add_tree_row + 展开/折叠交互——树形表格页升级真实交互（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-109-table-树形层级-addtreerow--展开折叠交互树形表格页升级真实交互2026-08-21`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table 树形层级（treeTable）：`add_tree_row(depth, cells)` 先序追加层级行，首列每级缩进 20px + 展开三角（▼/▶，仅 row_has_children 行渲染），点击切换整棵子树显隐；分页在树形过滤结果上切片、折叠子树不计页；`set_on_expand` 回调、`data_row_of` public——非 tree 表格行为零变化；树形模式禁排序（扁平重排破坏先序）；layui_test `--click x,y` 合成点击注入供脚本化交互验证

### BUG:UI-110 · UI-110: ComboBox 无 ScrollBox 祖先时弹出面板不翻转，贴近窗口底部的下拉尾部选项被窗口底边裁掉（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-110-combobox-无-scrollbox-祖先时弹出面板不翻转贴近窗口底部的下拉尾部选项被窗口底边裁掉2026-08-21`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ComboBox `layui-form-select`：白底 #eee 边框圆角2 + 右三角 + 聚焦绿框淡环；面板白底描边、dd 项 hover #f8f8f8、选中 layui-this 绿字；有 ScrollBox 祖先时随滚动实时翻转/钉顶，无则按窗口可视区定位（底部下拉不被窗口底边裁切）

### BUG:UI-111 · UI-111: ScrollBox 视口锚定 add_pinned（layui fixbar）+ PageHost 栈局部悬空修复——固定条页升级真实常驻工具组（2026-08-21）

- 路径: `docs/CHANGELOG.md#ui-111-scrollbox-视口锚定-addpinnedlayui-fixbar-pagehost-栈局部悬空修复固定条页升级真实常驻工具组2026-08-21`
- 反链(2):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ScrollBox 视口锚定：`add_pinned(w, corner, mx, my)` 控件照常入子树但不参与流式排布/不计内容高度，每次布局钉在可视区角落不随滚动移动；pinned_ 悬空条目 on_layout 自动剔除（clear_children 切页安全）；后加先绘先命中（layui fixbar）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#页面回调生命周期：PageHost 为 **main 作用域长命对象**（UI-110 结构性修复），页面 build 里 `[&host]` 捕获合法；历史上 host 曾是 render_page lambda 栈局部变量、回调触发即悬空（UI-095 首次暴露，当时以"回调只捕获长期对象"规避）

### BUG:UI-112 · UI-112: 颜色页一比一克隆镜像 docs/2/color（基色调/辅色调/中色调/七色调/标语）+ --list-pages 调试设施（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-112-颜色页一比一克隆镜像-docs2color基色调辅色调中色调七色调标语---list-pages-调试设施2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#颜色页一比一克隆镜像 docs/2/color：基色调 3 卡 / 辅色调 4 卡（白字双行 hex+名，radius 3）/ 中色调灰阶梯 radius 0 黑字（md6+md2×6+md6 三行无间距，#2f363c 黑字/#23292e 白字同镜像怪癖）/ 七色调整宽横条左对齐堆叠 / 标语引用块；色值全部精确采样验证；ColorBlock 自绘（HBox 行须传显式高度——Stretch 模式子高=行高-2×padding，行高 0 则子件不可见）

### BUG:UI-113 · UI-113: 图标页新增按钮图标段克隆镜像 h2#icon（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-113-图标页新增按钮图标段克隆镜像-h2icon2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#图标页按钮图标段：克隆镜像 h2#icon 两容器（UI-114 一比一修正）——容器一「按钮 ▾」(`set_icon_right` 文字左图标右) + left/right/edit/share 纯图标 / 容器二单行 9 个 sm：primary×5 + disabled + normal/warm/danger；Button::set_icon + add_class 能力复用，190 全量网格不受影响

### BUG:UI-114 · UI-114: Button 图标右置 set_icon_right + 图标页按钮图标段一比一修正（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-114-button-图标右置-seticonright--图标页按钮图标段一比一修正2026-08-22`
- 反链(2):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#图标页按钮图标段：克隆镜像 h2#icon 两容器（UI-114 一比一修正）——容器一「按钮 ▾」(`set_icon_right` 文字左图标右) + left/right/edit/share 纯图标 / 容器二单行 9 个 sm：primary×5 + disabled + normal/warm/danger；Button::set_icon + add_class 能力复用，190 全量网格不受影响
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Button 图标右置：`set_icon_right(true)` 内容布局切为「文本居左 + 图标居右」（整体仍居中），默认 false 图标居左零影响；镜像「文字 <i 图标>」顺序 demo 必备

### BUG:UI-115 · UI-115: TextArea 垂直滚动条（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-115-textarea-垂直滚动条2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#TextArea 垂直滚动条：内容溢出时右缘绘制 track/thumb，点击轨道跳转 + 拖拽映射 scroll_；avail_width 恒定让出滚动条列防换行跳动

### BUG:UI-116 · UI-116: 垂直滚动条 overlay 风格化：自动隐藏 + 悬停加宽 + 三态色（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-116-垂直滚动条-overlay-风格化自动隐藏--悬停加宽--三态色2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#垂直滚动条 overlay 风格化（TextArea+ScrollBox 共享 scrollbar_style.h）：自动隐藏（悬停/拖拽/2s 内滚过才显示）、悬停右缘列 6→8px 加宽、thumb 三态色（默认半透明→悬停全色→拖拽 accent）；隐藏期间滚轮照常滚动

### BUG:UI-117 · UI-117: 图标页克隆镜像列表 195 项 + HBox 自适应高度与图标字体扩段（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-117-图标页克隆镜像列表-195-项--hbox-自适应高度与图标字体扩段2026-08-22`
- 反链(2):
  - `META:CHANGELOG` (ref) — 相关历史条目
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#图标页全量网格一比一克隆镜像 docs/2/icon 列表：6 列描边卡片（Card::set_outline #e8e8e8）×每卡四行（字形/中文名/&#x码点;/类名，均水平居中，后两行 0.78 缩放小字；数据源 icon_catalog.h），192 项同序 + layim×3 末尾保留 = 195；依赖 HBox h≤0 自适应行高、Font 双段图标烘培（E600..ECFF + 1000..100F）、iconfont.ttf 2.13.9

### BUG:UI-118 · UI-118: 图标卡片文本居中 + 数据表抽离重构（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-118-图标卡片文本居中--数据表抽离重构2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#图标页全量网格一比一克隆镜像 docs/2/icon 列表：6 列描边卡片（Card::set_outline #e8e8e8）×每卡四行（字形/中文名/&#x码点;/类名，均水平居中，后两行 0.78 缩放小字；数据源 icon_catalog.h），192 项同序 + layim×3 末尾保留 = 195；依赖 HBox h≤0 自适应行高、Font 双段图标烘培（E600..ECFF + 1000..100F）、iconfont.ttf 2.13.9

### BUG:UI-119 · UI-119: 表单/选择框/分页页崩溃排查：陈旧增量构建产物 ODR 混合布局 + 崩溃诊断钩子（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-119-表单选择框分页页崩溃排查陈旧增量构建产物-odr-混合布局--崩溃诊断钩子2026-08-22`
- 反链(2):
  - `DOC:BUILD` (checklist) — 回归清单#布局相关 constexpr 公开头（如 gl/font.h 字形数组尺寸）变更后，凡经历过编译失败/中断的构建树必须全量重建再验证；增量混合产物会触发 ODR 违例型堆崩溃（症状：仅个别页 0xC0000374/0xC0000005 且崩溃码漂移）
  - `DOC:BUILD` (checklist) — 回归清单#layui_test 崩溃时自动打印符号化调用栈（SetUnhandledExceptionFilter + dbghelp）；定位崩溃帧需 Release+/Z7 生成 PDB

### BUG:UI-120 · UI-120: Slider 聚焦外框改为默认不显示 + set_focus_ring 可选开启（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-120-slider-聚焦外框改为默认不显示--setfocusring-可选开启2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Slider 聚焦外框默认不显示：任何状态（hover/拖拽/键盘聚焦/禁用）均不绘制 bounds 外框；`set_focus_ring(true)` 后 focused() 且非 disabled 才恢复外框；滑块其余行为（轨道/填充/圆钮/range 双钮/step/showstep/tips/input/vertical/disabled）不变

### BUG:UI-121 · UI-121: ComboBox::set_options 就地重载选项——销毁重建孤儿控件（幽灵下拉）根治 API（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-121-comboboxsetoptions-就地重载选项销毁重建孤儿控件幽灵下拉根治-api2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ComboBox `set_options()`：就地重载选项（先关面板→递归清旧选项按钮→重建→越界钳制 selected_），不销毁控件本体；配合应用层「首次才 new、后续复用实例」消灭销毁重建遗留的孤儿控件（幽灵下拉）；原 add_option 单项追加行为不变

### BUG:UI-122 · UI-122: Laypage/Table 分页条区域点击穿透——容器自身 chrome 一律消费（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-122-laypagetable-分页条区域点击穿透容器自身-chrome-一律消费2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Laypage/Table 分页条自身 chrome（按钮/文本/留白）鼠标按下一律消费，点击不再穿透到下层控件；表格内容区与翻页回调行为不变

### BUG:UI-123 · UI-123: Table auto_page_size 扣除分页条高度 + set_paginated 保持页码（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-123-table-autopagesize-扣除分页条高度--setpaginated-保持页码2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table `auto_page_size()` 按可视高自动计算每页行数（扣除表头+分页条高度）；`set_paginated(true,n)` 总页数缩小时保持当前页（钳制不回跳第 1 页）；应用侧手算 pageSize 公式可整体替换

### BUG:UI-124 · UI-124: 表格行单击选中高亮（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-124-表格行单击选中高亮2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table 行单击选中高亮：单击数据行记录可见行号（`sel_row_` + 公开 `set_selected_row()`），选中行 #16baaa α0.18 叠加、hover #EFEFEF，与底色可区分（修复三分支同画 #FAFAFA 零反馈）；reload/翻页/排序自动清除；radio 模式语义不变

### BUG:UI-125 · UI-125: CJK 字形逐帧烘焙配额 + prebake_text 预热（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-125-cjk-字形逐帧烘焙配额--prebaketext-预热2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#CJK 字形逐帧烘焙配额：懒烘培按 16ms 窗口限量（默认 12 字形），超配额本帧回退方块字宽近似、后续帧渐进补齐，消除首次打开新页一帧内数十次光栅化+纹理上传的单帧卡顿/闪烁；`Font::prebake_text()` 不受限强制预热

### BUG:UI-126 · UI-126: Table 流加载支持——触底重检 rearm + 尾注 + on_load_more（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-126-table-流加载支持触底重检-rearm--尾注--onloadmore2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table 流加载（layui flow 滚动自动加载）：`set_on_load_more(cb)` 仅非分页模式生效（set_paginated 内同步挂/摘钩子防误触发）；rebuild_rows 尾部自动 `ScrollBox::rearm_bottom()` 挂起触底重检（UI-127 分帧）——触底→续灌→重建→重检直至填满一屏或耗尽断链，补齐「DOM 插入后再查一次」语义；`set_end_note()` 行尾只读尾注（不参与行号映射/命中）；`reset_scroll()` 回顶；分页模式与 UI-035/093/094 既有语义零变化

### BUG:UI-127 · UI-127: 流加载/表格首显闪烁——触底重检分帧 + prebake_visible_text 字形预热（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-127-流加载表格首显闪烁触底重检分帧--prebakevisibletext-字形预热2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#流加载/表格首显防闪烁：`ScrollBox::rearm_bottom()` 分帧（置挂起标记，on_layout 开头消费 check_bottom，每帧至多灌一批、回调重建行当帧布局归位无乱帧，打断同步递归级联单帧暴增）；`Table::prebake_visible_text()` 预热表头+可见行+尾注 CJK 字形（转发 prebake_text 跳过已烘焙码点），消除 UI-125 配额限流方块宽渐进补齐闪烁

### BUG:UI-128 · UI-128: emoji 渲染回退链——缺字 tofu 防护 + Segoe UI Emoji 单色轮廓烘焙（2026-08-22）

- 路径: `docs/CHANGELOG.md#ui-128-emoji-渲染回退链缺字-tofu-防护--segoe-ui-emoji-单色轮廓烘焙2026-08-22`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#emoji 渲染回退链：`load_emoji_font(_auto)` 载入 seguiemj.ttf 单色轮廓源（env `OPENGUI_EMOJI_FONT`）；bake_cjk 显式 FindGlyphIndex>0 回退链查字，两源皆无记 missing_ 防逐帧重试，杜绝 .notdef tofu 入图集；FE0F/ZWJ 零宽处理

### BUG:UI-129 · UI-129: Table 单元格表单切换回调——set_on_cell_form 点击翻转通知调用方（2026-08-23）

- 路径: `docs/CHANGELOG.md#ui-129-table-单元格表单切换回调setoncellform-点击翻转通知调用方2026-08-23`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table 单元格表单切换回调：`set_on_cell_form(cb)`（可见行号, 列号, 切换后选中态），点击 Switch/CheckBox 格翻转文本后触发（此前仅翻转不通知）；未注册回调行为不变；复选/单选/操作列/树形/分页/流加载路径零影响

### BUG:UI-130 · UI-130: 开关皮肤统一——draw_switch_skin 共享函数，表格行内开关与 Switch 同源渲染（2026-08-23）

- 路径: `docs/CHANGELOG.md#ui-130-开关皮肤统一drawswitchskin-共享函数表格行内开关与-switch-同源渲染2026-08-23`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#开关皮肤统一：`draw_switch_skin(renderer, rect, theme, checked, enabled, hovered, pressed)` 共享三态绘制（Switch 本体与 Table 单元格开关同源渲染，开=绿底白滑块靠右/关=白底灰边灰滑块靠左）；新增开关一律复用该皮肤，严禁自绘矩形/方块开关

### BUG:UI-131 · UI-131: draw_cell_form 参数错位修复——单元格开关恒走复选分支绘矩形；表单列表头居中（2026-08-23）

- 路径: `docs/CHANGELOG.md#ui-131-drawcellform-参数错位修复单元格开关恒走复选分支绘矩形表单列表头居中2026-08-23`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#draw_cell_form 形参与调用点语义一致（w=列宽、col=列号）：行内开关真实走圆角皮肤分支；表单列（开关/复选）表头标题水平居中与单元格控件对齐

### BUG:UI-132 · UI-132: Table preferred_height 高度语义修正——默认内容跟随，新增 set_fixed_height/set_max_content_height（2026-08-23）

- 路径: `docs/CHANGELOG.md#ui-132-table-preferredheight-高度语义修正默认内容跟随新增-setfixedheightsetmaxcontentheight2026-08-23`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table preferred_height 高度语义：默认内容跟随（表头+可见行×行高[+分页条] 每次重算，不再被首次分配的 bounds_.h 冻结——流式容器内 add_row 即时伸缩）；`set_fixed_height(h)` 显式锁定（固定视口+内部滚动）、`set_max_content_height(h)` 内容跟随上限钳制（流式容器防溢出）；手动 set_bounds 页面与分页/树形/流加载路径零变化

### BUG:UI-133 · UI-133: 模态拦截可见性防御——modal 控件隐藏后不再吞输入，修复僵尸模态全 UI 死锁（2026-08-23）

- 路径: `docs/CHANGELOG.md#ui-133-模态拦截可见性防御modal-控件隐藏后不再吞输入修复僵尸模态全-ui-死锁2026-08-23`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#模态拦截可见性防御：7 处 dispatch_* 模态分支统一 `modal_ && modal_->visible`——modal 控件隐藏（destroy 待释放/手动隐藏）即解除输入拦截，杜绝宿主未接 advance 时僵尸模态永久吞输入锁死全 UI；可见模态行为不变

### BUG:UI-134 · UI-134: ScrollBar 可命中区加宽 + Label scissor 裁剪防溢出（2026-08-27）

- 路径: `docs/CHANGELOG.md#ui-134-scrollbar-可命中区加宽--label-scissor-裁剪防溢出2026-08-27`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ScrollBar 可命中区加宽（kMargin 6px / kHitPad 4px，命中区总宽 20px）+ Label `on_draw()` scissor 裁剪防文本溢出 bounds 上叠相邻控件/滚动条

### BUG:UI-135 · UI-135: 帧级门控渲染完备性——draw 移除脏守卫全树绘制 + hover/pressed/focus/scroll 显式标脏（2026-08-27）

- 路径: `docs/CHANGELOG.md#ui-135-帧级门控渲染完备性draw-移除脏守卫全树绘制--hoverpressedfocusscroll-显式标脏2026-08-27`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#帧级门控渲染完备性：`Widget::draw()` 渲染帧内全树绘制、脏标记仅作帧级门控（删除子树脏守卫——begin_frame 每帧全屏清屏下只重绘脏控件会擦净干净控件）；hover/按下/抬起/焦点变更与 `ScrollBox::set_scroll`、滚动条 2s 自动隐藏均显式 `mark_dirty()`，长内容页滚轮/拖动平滑重绘

### BUG:UI-136 · UI-136: 帧级门控下 CJK 惰性烘焙自动续帧——待烘焙队列 + bake-epoch 全树重排（2026-08-27）

- 路径: `docs/CHANGELOG.md#ui-136-帧级门控下-cjk-惰性烘焙自动续帧待烘焙队列--bake-epoch-全树重排2026-08-27`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#CJK 惰性烘焙续帧：`Font` 记录「待烘焙码点集」+ 新字形上屏单调计数（bake-epoch），`Widget::advance()` 根分支在帧级门控之前查询 `any_pending_bake()/bake_epoch_sum()`，有待烘字形或 epoch 变化即 `mark_dirty_all()` 全树置脏续帧——首显后自动烘齐（≤12 字形/16ms 配额不变）并以真实宽度重排，排空后恢复空闲停帧，杜绝音频宿主首显文本无尽占位

### BUG:UI-137 · UI-137: 帧级门控下控件状态写统一自报标脏——开关/下拉/滑块事件内直改漏标点（2026-08-27）

- 路径: `docs/CHANGELOG.md#ui-137-帧级门控下控件状态写统一自报标脏开关下拉滑块事件内直改漏标点2026-08-27`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#控件状态写统一自报标脏（金芯）：帧级门控下事件回调内直接改成员状态必须显式 `mark_dirty()` 自报——`Switch::on_mouse_down/on_key_down` 统一走 `set_checked`（内部标脏）、`ComboBox` 开合 toggle 与键盘方向键改选项、`Slider::emit()` 均显式标脏，根治「开关/下拉需多次点击、键盘改值无重绘」

### BUG:UI-138 · UI-138: 帧级门控下 TextBox/TextArea 编辑与光标/选区/滚动路径漏标脏（2026-08-27）

- 路径: `docs/CHANGELOG.md#ui-138-帧级门控下-textboxtextarea-编辑与光标选区滚动路径漏标脏2026-08-27`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#文本编辑写统一自报标脏（金芯扩展）：帧级门控下 `TextBox` / `TextArea` 的每次编辑 / 光标移动 / 选区变更 / 滚动都必须显式 `mark_dirty()` 自报（输入/删除/方向键/Home/End/Ctrl+A/C/V/X、拖选、滚轮滚动、滚动条拖拽、`scroll_to_end`），根治「输入需滚动/移鼠标才显示、光标选区不随键更新、滚动需额外一拍刷新」

### BUG:UI-139 · UI-139: 帧级门控下 Menu 左侧导航 hover 高亮不跟随——on_mouse_move 更新 hover_ 却漏标脏（2026-08-27）

- 路径: `docs/CHANGELOG.md#ui-139-帧级门控下-menu-左侧导航-hover-高亮不跟随onmousemove-更新-hover-却漏标脏2026-08-27`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Menu 左侧导航 hover 高亮随跟随：`Menu::on_mouse_move` 重排 `hover_`/`active_hover_` 后必须比对旧值、目标变化即 `mark_dirty()`（基类 `dispatch_mouse_move` 只在控件 hover 边界翻转时标脏，菜单项间移动不标脏）——高亮框在菜单项间移动即时跟随，不需额外点击/切页；移出菜单高亮即时清除

### BUG:UI-140 · UI-140: ScrollBox/Code 覆写 draw 不清自身脏标记→dirty_children_ 常脏，短路 mark_dirty 上抛、容器内控件点击/悬停不刷新（2026-08-27）

- 路径: `docs/CHANGELOG.md#ui-140-scrollboxcode-覆写-draw-不清自身脏标记dirtychildren-常脏短路-markdirty-上抛容器内控件点击悬停不刷新2026-08-27`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ScrollBox/Code 覆写 draw 末尾必须清自身脏（`dirty_=false; dirty_children_=false;`，与基类 `Widget::draw` 一致）——否则 `dirty_children_` 常脏使 `mark_dirty()` 沿父链短路 `break`，事件内子控件标脏到不了 root、滚动/点击/悬停不即时刷新

### BUG:UI-141 · UI-141: Transfer 双侧列表滚轮改 scroll 漏 mark_dirty——同类漏标脏补全（2026-08-27）

- 路径: `docs/CHANGELOG.md#ui-141-transfer-双侧列表滚轮改-scroll-漏-markdirty同类漏标脏补全2026-08-27`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Transfer 双侧列表滚轮即时刷新：`Transfer::on_mouse_scroll`（滚动条目）直接改 `side->scroll` 后必须 `mark_dirty()`（与 `move/add_item/clear` 等 setter 一致），否则滚轮滚动不刷新

### BUG:UI-142 · UI-142: ScrollBox 固定头部 + 背景直角化——页面顶部留白固定且与左栏拼全宽横条（2026-08-27）

- 路径: `docs/CHANGELOG.md#ui-142-scrollbox-固定头部--背景直角化页面顶部留白固定且与左栏拼全宽横条2026-08-27`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ScrollBox 固定头部 `set_header_height(float)`：置顶一条固定不随滚动移动的头部带，内容区/滚动范围/裁减区以其为顶边内边距；默认背景改为直角 `draw_quad(bounds_, button_normal)`（消除面板圆角与左导航直角月牙空隙），`layui-round` class 可显式切回圆角

### BUG:UI-143 · UI-143: Table 新增下拉单元格列——表格行内下拉选择（2026-08-30）

- 路径: `docs/CHANGELOG.md#ui-143-table-新增下拉单元格列表格行内下拉选择2026-08-30`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Table 下拉单元格列：`set_cell_combobox(col)` 该列每格渲染文本 + 右侧下拉三角（#c2c2c2），`set_combobox_options(col,opts)` 指定选项、`set_on_cell_combobox(visible_row,col,index,text)` 选中回调（复用 ComboBox layui-form-select 皮肤与 popup 弹出；单击展开、选中即写回格文本）；对象池复用生命周期（on_change 回调栈内不 delete 触发者自身），文本/开关/复选/按钮/树形/分页路径零变化

### BUG:UI-144 · UI-144: ScrollBox 滚动条按下滑块不再立即跳转，点击轨道空白仍跳转（2026-08-28）

- 路径: `docs/CHANGELOG.md#ui-144-scrollbox-滚动条按下滑块不再立即跳转点击轨道空白仍跳转2026-08-28`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#ScrollBox 滚动条按下不跳：`on_mouse_down` 命中滑块仅 `dragging=true` 不再按下瞬间 `set_scroll`（消除按住滑块即下跳）；命中轨道空白（滑块上/下）仍 `set_scroll` 跳转原地

### BUG:UI-145 · UI-145: 全方位优化批次——渲染性能/内存生命周期/代码去重/构建与清理（2026-08-31）

- 路径: `docs/CHANGELOG.md#ui-145-全方位优化批次渲染性能内存生命周期代码去重构建与清理2026-08-31`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#优化批次契约（不改 API 语义）：Renderer 投影矩阵在 `set_viewport` 缓存、颜色/纹理管线状态合并（不得逐 draw 重算/重传投影）；字形旋转三角函数 CPU 预计算经 uniform 下传；事件分发子节点遍历用 ≤32 栈数组快照免堆分配（语义同 UI-073）；标准光标一次创建缓存复用（`StandardCursorCache`）；`Image::texture_` 为独占 `unique_ptr` 随控件释放；`Font::draw_text` 双载合并共享实现；UTF-8 解码统一 `core/ui/core/utf8.h`（textbox/textarea/table 收敛）；scissor/标准光标 helper 去重

### BUG:UI-146 · UI-146: 新增统一生命周期抽象 ui::Scene——生产者→分配者→消费者→清理重置（2026-09-01）

- 路径: `docs/CHANGELOG.md#ui-146-新增统一生命周期抽象-uiscene生产者分配者消费者清理重置2026-09-01`
- 反链(6):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Scene 生命周期四阶段：`ui::Scene(window, renderer)` 随窗口同步创建，构造广播 `ScenePhase::Ready`；`run()/run_for(秒)` 承载 poll+swap 完整循环与 `--smoke` 通用语义；`reset()/shutdown()` 随 `~Scene()` 自动收尾——禁止应用手写 poll/layout/dispatch/draw/swap 五段主循环
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Scene 全量发布-订阅：`subscribe(ScenePhase, fn)` 阶段向量广播（一阶段多观察者，Phase 含 Ready/Reset/Frame/PostDraw/Cleanup/Shutdown）、`on(phase, fn)` 便捷重载、`set_event_hook(SceneEvent, hook)` 逐事件钩子（返回 true=消费，含 MouseMove/Down/Up/Scroll/Key/Char/Drop/Resized）；`subscribe/on` 返回**订阅 token**（非 0），`unsubscribe(token)` 按 token 单独摘除单个观察者（不清其他 App 级订阅）；`event_key()/event_scroll_dx/y()` 取随路数据
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#页面级订阅生命周期（layui_test 展示 > 生命周期）：页面 build 内经 `host.scene` 挂的 PostDraw/Frame 等局部订阅，切页时必须在页面 `destroy` 钩子统一 `unsubscribe(token)`（在预览区 `clear_children` 之前）逐个摘除，**禁止** `clear_observers()` 清掉 App 级订阅，且不得持有已释放页面控件的闭包指针
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Scene 分配开关 `dispatch_mouse/scroll/nav_keys/text/resize`（默认全开）保留各应用迁移前按键语义；Escape 不在标准模板（属应用语义经 Key 钩子自处理）——迁移应用禁止为省事改而改原按键语义
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#Scene 清理重置只清库内自管对象（`flush_pending_destroys` + `close_popups`），**禁止 `clear_children()`**——Widget 容器默认不拥有子树，其子节点可为栈上对象（`root.add_child(&栈Label)`），对栈对象 `delete` 即堆损坏崩溃（0xC0000374）
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#多窗口独立 GL 上下文：每窗口 = 独立 `Scene` + `Window`/`Renderer`/`Font`/`root`；绘制某窗口前先 `window.make_current()`（`Window::make_current()`），禁止同上下文运行两个 `Renderer`（Renderer 实例级缓存 active_program_/scissor_stack_/proj_[16] 等 GL 全局状态会冲突）

### BUG:UI-147 · UI-147: 新增自绘系统风格标题条 TitleBar + 窗口居中——无边框窗口可拖动 / 关闭（2026-09-01）

- 路径: `docs/CHANGELOG.md#ui-147-新增自绘系统风格标题条-titlebar--窗口居中无边框窗口可拖动--关闭2026-09-01`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#无边框窗口框架：`GLFW_DECORATED=false` 下，`ui::TitleBar(w,h,title,win)` 提供自绘系统风格标题条（空白区 `start_drag/update_drag/end_drag` 拖动整窗、关闭钮触发 `on_close_`，win 为空仅绘制）；`Window::center_on_primary_monitor()` 在主显示器工作区内居中（窗口大于工作区贴左上）。主窗口 / Scene 子窗均经其拖动与关闭

### BUG:UI-148 · UI-148: TitleBar 补齐 最小化/最大化/关闭 三键 + 左上角应用图标；Menu 分组头箭头改用 iconfont + 可联动收起子菜单列（2026-09-01）

- 路径: `docs/CHANGELOG.md#ui-148-titlebar-补齐-最小化最大化关闭-三键--左上角应用图标menu-分组头箭头改用-iconfont--可联动收起子菜单列2026-09-01`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#标题条交互相通：`ui::TitleBar` 右侧最小化/最大化(还原)/关闭三键（`iconify()/maximize()/restore()` 依 `is_maximized()` 切换图标 `□`/`⧉`、关闭钮 `on_close_`）、`set_icon(font,cp)` 左上角应用图标；`ui::Menu` 分组头折叠箭头用 `Font::draw_icon`（`layui_icon::down/right`）并新增 `set_on_group_click` 供宿主收起/展开外部子菜单列

### BUG:UI-149 · UI-149: 修复主窗口自绘标题条被顶部 Header 遮住——Header 构造默认 y=0 与 TitleBar 重叠，首帧标题条不可见（2026-09-01）

- 路径: `docs/CHANGELOG.md#ui-149-修复主窗口自绘标题条被顶部-header-遮住header-构造默认-y0-与-titlebar-重叠首帧标题条不可见2026-09-01`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#自绘标题条布局回归项：无边框主窗口首帧（未发生 Resize）标题条即完整可见、不被顶部 Header（y=0..30 需让出）遮盖，Header 位于 y=kTitleBarH 起不重叠

### BUG:UI-150 · UI-150: 自绘 TitleBar 默认背景与下方 Header 同色导致"无缝融合、标题条不可区分"——改为独立更深灰（2026-09-01）

- 路径: `docs/CHANGELOG.md#ui-150-自绘-titlebar-默认背景与下方-header-同色导致无缝融合标题条不可区分改为独立更深灰2026-09-01`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#自绘 TitleBar 独立配色回归项：标题条默认背景 `#262A2F` 与紧邻下方页头/导航栏底色（`#2F363C`）明度可区分，标题条作为导航栏上方独立深色条可见，不与其无缝融合

### BUG:UI-151 · UI-151: 修复主窗口顶部导航题条重叠的深层根因——Header 子项未随父 y 偏移（本库子控件 bounds 为绝对坐标）（2026-09-01）

- 路径: `docs/CHANGELOG.md#ui-151-修复主窗口顶部导航题条重叠的深层根因header-子项未随父-y-偏移本库子控件-bounds-为绝对坐标2026-09-01`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#顶部导航内容偏移回归项：Header 的 logo 图、导航文本、下拉、开关等子项须叠加父 `bounds_.y` 偏移（本库子控件 bounds 为绝对坐标），随 Header 外框整体下移到标题条下方，不与标题条重叠

### BUG:UI-153 · UI-153: 库级「顶部装帧内容区原点」——Scene::set_chrome/content_origin_y，其余控件统一绘制于标题栏下方 + 修复帧率 HUD 与标题条重叠（2026-09-01）

- 路径: `docs/CHANGELOG.md#ui-153-库级顶部装帧内容区原点scenesetchromecontentoriginy其余控件统一绘制于标题栏下方--修复帧率-hud-与标题条重叠2026-09-01`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#内容区原点回归项：注册 `Scene::set_chrome(titlebar, h)` 后，header/sidebar/tabs/content/帧率 HUD 的 y 统一以 `Scene::content_origin_y()` 为单一来源派生（承接 UI-149/151），其余控件绘制于标题条下方、帧率 HUD 垂直居中于 Header 不与标题条重叠

### BUG:UI-154 · UI-154: 子树变更不标脏使长命容器首帧不重排、新子控件在 (0,0) 闪一帧——基类 add_child/remove_child/clear_children 自报标脏（2026-09-01）

- 路径: `docs/CHANGELOG.md#ui-154-子树变更不标脏使长命容器首帧不重排新子控件在-00-闪一帧基类-addchildremovechildclearchildren-自报标脏2026-09-01`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#子树变更即重排（防 0,0 闪帧）：`Widget::add_child/remove_child/clear_children` 均自报 `mark_dirty()`——`layout()` 的帧级门控（`!dirty_ && !dirty_children_` 提前返回）下，长命容器（内容面板 ScrollBox/代码区/侧栏/VBox 等）被清空重建后下一帧必重排，新子控件同帧归位，不再残留在构造默认 `(0,0,0,0)` bounds 于左上角闪一帧；弹出面板（root 挂载、`sync_panel` 幂等重算）与 PanelWindow 既有行为不变

### BUG:UI-155 · UI-155: 新增窗口容器 WindowFrame——自带自绘标题栏 + 内容区相对坐标 + 通用头部槽（2026-09-01）

- 路径: `docs/CHANGELOG.md#ui-155-新增窗口容器-windowframe自带自绘标题栏--内容区相对坐标--通用头部槽2026-09-01`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#窗口容器 `WindowFrame`（UI-155）：自带自绘顶部标题栏（默认系统风格 `TitleBar`，标题栏高参数化默认 30）+ 标题栏下方铺满内容区；内容子控件以**容器相对坐标** `set_content_child(child,x,y,w,h)` 定位（相对内容区顶部），控件内部以相对矩形快照为唯一事实源、`on_layout` 重基准为绝对坐标（不二次叠加）；通用头部 `set_head(widget,height)` 可换任意 Widget 作头（`set_head_child` 头部相对坐标），内容区随 head_h 整体下移

### BUG:UI-156 · UI-156: 标脏三态闭环「标记/去标/重标」——库级 set_visible 重标 + finish_draw_dirty 统一去标 + 清理死码 clear_dirty（2026-09-01）

- 路径: `docs/CHANGELOG.md#ui-156-标脏三态闭环标记去标重标库级-setvisible-重标--finishdrawdirty-统一去标--清理死码-cleardirty2026-09-01`
- 反链(1):
  - `DOC:ARCHITECTURE` (checklist) — 回归清单#显隐变更经 `set_visible`（重标闭环）：库内所有 `Widget::visible=` 直接写点一律改走 `set_visible(bool)`（变化才 `mark_dirty()`），杜绝「隐藏且仍脏的子控件再显示时父容器 `dirty_children_` 已被前帧 draw 抹 false → `layout()` 门控提前返回跳过重排 → 新现身子控件停在旧/0,0 坐标成一帧闪」；帧末去标统一走 `Widget::finish_draw_dirty()`（`Widget::draw`/`Code::draw`/`ScrollBox::draw` 一致），检测到隐藏且仍脏的子节点时保留 `dirty_children_=true` 以维系重标传播（死码 `clear_dirty()` 已删）。同类重标缺口一并收敛：`ScrollBox::mark_content_dirty()` 置 `needs_layout_` 同时 `mark_dirty()`（`needs_layout_` 仅在受脏门控保护的 `on_layout()` 里消费，不重标则 root 保持 clean、内容永不重排/滚动条永久抑制）

### DOC:API_REFERENCE · API_REFERENCE

- 路径: `docs/distribution/API_REFERENCE.md`
- 反链: 无

### DOC:ARCHITECTURE · ARCHITECTURE

- 路径: `docs/ARCHITECTURE.md`
- 反链(16):
  - `META:CHANGELOG` (mention) — UI-003: 分层框架骨架与构建验证（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-004: M1 渲染管线升级：圆角矩形 SDF + 纹理/字体 + 控件树（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-005: M2 布局系统：VBox/Label + 滚轮/窗口缩放（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-006: M3 焦点系统与键盘输入（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-007: 内容自适应高度与焦点完善（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-008: M4 文本输入：TextBox + 字符通道 + scissor 裁剪（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-009: M5 鼠标拖拽：Slider 滑块 + 鼠标捕获（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-010: M6 媒体模块点亮：ImageView/VideoView（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-011: M7 视频解码线程化 + 播放控制（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-013: M8 滚动容器：ScrollBox（2026-08-09）
  - `DOC:PLAN` (mention) — 里程碑计划（Roadmap）
  - `DOC:PLAN` (mention) — 目标与终点
  - `DOC:PLAN` (mention) — 终点达成（M16 验收）
  - `DOC:PLAN` (mention) — 完成定义（DoD）
  - `DOC:README` (mention) — OpenGL UI
  - `META:CHANGELOG` (mention)

### DOC:BUILD · BUILD

- 路径: `docs/BUILD.md`
- 反链(5):
  - `META:CHANGELOG` (mention) — UI-002: 3rdparty 依赖构建管线（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-010: M6 媒体模块点亮：ImageView/VideoView（2026-08-09）
  - `META:CHANGELOG` (mention) — UI-011: M7 视频解码线程化 + 播放控制（2026-08-09）
  - `DOC:README` (mention) — OpenGL UI
  - `DOC:README` (mention) — 示例应用（M16 收官）

### DOC:PLAN · PLAN

- 路径: `docs/PLAN.md`
- 反链(1):
  - `DOC:README` (mention) — OpenGL UI

### DOC:README · README

- 路径: `docs/README.md`
- 反链: 无

### DOC:UI_API · UI_API

- 路径: `docs/UI_API.md`
- 反链(1):
  - `META:CHANGELOG` (mention)

### META:AGENTS · AGENTS

- 路径: `AGENTS.md`
- 反链: 无

### META:CHANGELOG · CHANGELOG

- 路径: `docs/CHANGELOG.md`
- 反链(24):
  - `META:CHANGELOG` (mention) — UI-100: IME 候选窗跟随输入框光标（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-101: TextBox 选择/剪贴板能力（拖选/Shift 选择/Ctrl+A/C/X/V）（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-102: 键盘修饰键快照——SendInput 粘贴到自绘输入框丢失 mods（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-103: TextArea 剪贴板默认实现——回调未注入时 Ctrl+C/X/V 静默失效（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-104: 侧栏分组逐级嵌套成子级折叠菜单——Menu add_group 未配对 end_group（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-105: 帧率 HUD 与头部右侧控件重叠——HUD 右对齐压在生态/版本下拉上（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-106: 侧栏缺失镜像站 14 个页面——基础组整组缺 + 颜色/图标/输入框/选择框/单选框/树形表格/公共类/固定条（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-107: UI-106 的 14 个占位页全部实现为真实内容页（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-108: Font/Label 字号缩放与仿粗体 + layui 文字工具色 font_color——公共类页升级真实演示（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-109: Table 树形层级 add_tree_row + 展开/折叠交互——树形表格页升级真实交互（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-111: ScrollBox 视口锚定 add_pinned（layui fixbar）+ PageHost 栈局部悬空修复——固定条页升级真实常驻工具组（2026-08-21）
  - `META:CHANGELOG` (mention) — UI-112: 颜色页一比一克隆镜像 docs/2/color（基色调/辅色调/中色调/七色调/标语）+ --list-pages 调试设施（2026-08-22）
  - `META:CHANGELOG` (mention) — UI-113: 图标页新增按钮图标段克隆镜像 h2#icon（2026-08-22）
  - `META:CHANGELOG` (mention) — UI-114: Button 图标右置 set_icon_right + 图标页按钮图标段一比一修正（2026-08-22）
  - `META:CHANGELOG` (mention) — UI-115: TextArea 垂直滚动条（2026-08-22）
  - `META:CHANGELOG` (mention) — UI-116: 垂直滚动条 overlay 风格化：自动隐藏 + 悬停加宽 + 三态色（2026-08-22）
  - `META:CHANGELOG` (mention) — UI-117: 图标页克隆镜像列表 195 项 + HBox 自适应高度与图标字体扩段（2026-08-22）
  - `META:CHANGELOG` (mention) — UI-118: 图标卡片文本居中 + 数据表抽离重构（2026-08-22）
  - `META:CHANGELOG` (mention) — UI-119: 表单/选择框/分页页崩溃排查：陈旧增量构建产物 ODR 混合布局 + 崩溃诊断钩子（2026-08-22）
  - `DOC:PLAN` (mention) — 里程碑计划（Roadmap）
  - `DOC:PLAN` (mention) — 完成定义（DoD）
  - `DOC:UI_API` (mention) — UI 库 API 手册
  - `META:AGENTS` (mention) — 文档图谱 graphgen（已安装）
  - `META:AGENTS` (mention)

## 全部节点

| id | type | label | path |
|----|------|-------|------|
| `BUG:UI-001` | BUG | UI-001: 初始化项目文档图谱（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-002` | BUG | UI-002: 3rdparty 依赖构建管线（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-003` | BUG | UI-003: 分层框架骨架与构建验证（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-004` | BUG | UI-004: M1 渲染管线升级：圆角矩形 SDF + 纹理/字体 + 控件树（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-005` | BUG | UI-005: M2 布局系统：VBox/Label + 滚轮/窗口缩放（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-006` | BUG | UI-006: M3 焦点系统与键盘输入（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-007` | BUG | UI-007: 内容自适应高度与焦点完善（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-008` | BUG | UI-008: M4 文本输入：TextBox + 字符通道 + scissor 裁剪（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-009` | BUG | UI-009: M5 鼠标拖拽：Slider 滑块 + 鼠标捕获（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-010` | BUG | UI-010: M6 媒体模块点亮：ImageView/VideoView（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-011` | BUG | UI-011: M7 视频解码线程化 + 播放控制（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-012` | BUG | UI-012: M7 播放交互收尾：EOF 回播 + 解码节流（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-013` | BUG | UI-013: M8 滚动容器：ScrollBox（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-014` | BUG | UI-014: M9 复合控件：CheckBox / RadioGroup / ComboBox（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-015` | BUG | UI-015: M10 主题系统：暗/亮预设 + 运行期切换（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-016` | BUG | UI-016: M11 多行文本排版：Label 换行（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-017` | BUG | UI-017: M12 布局增强：HBox / Grid / 嵌套滚动（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-018` | BUG | UI-018: M13 视频增强：倍速 / 帧步进 / 精确 seek（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-019` | BUG | UI-019: M14 窗口系统：PanelWindow / MenuBar / 模态 / Tooltip（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-020` | BUG | UI-020: M15 视频音频输出：AAC + 音量 + winmm 播放（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-021` | BUG | UI-021: 素材加载测试工具 + Demo 指定素材播放（2026-08-09） | `docs/CHANGELOG.md` |
| `BUG:UI-022` | BUG | UI-022: M17a layui 克隆·基础设施 + 轻量组件（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-023` | BUG | UI-023: M17a 遗留完善 + M17b 表单与按钮（layui 克隆）（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-024` | BUG | UI-024: M17c 数据与弹层：Tree / NavMenu / layer 外观 / Laypage（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-025` | BUG | UI-025: M17c 收尾：Table layui 化 + Demo 展示页（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-026` | BUG | UI-026: M17d layui 克隆·图标与特殊组件（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-027` | BUG | UI-027: M16 收官·示例应用（终点）（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-028` | BUG | UI-028: layui_test 演示工程骨架 + Grid 栅格 span/offset 扩展（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-029` | BUG | UI-029: layui_test 站式布局外壳 + 徽章页克隆（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-030` | BUG | UI-030: 六页基本元素克隆（面包屑/选项卡/进度条/面板/时间线/评分）（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-031` | BUG | UI-031: 六页组件克隆（滑块/分页/下拉菜单/树形组件/基础菜单/静态表格）（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-032` | BUG | UI-032: 六页组件克隆（表单/日期/穿梭/颜色/轮播/上传）+ 控件生命周期修复（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-033` | BUG | UI-033: 五页克隆（框架/动画/辅助元素/弹出层/数据表格）+ 根级弹出生命周期（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-034` | BUG | UI-034: 四页克隆（模板引擎/流加载/工具模块/文本行修饰）+ 克隆收尾（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-035` | BUG | UI-035: 流加载滚动到底自动模式 + 窗口点击置顶（layer.setTop）（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-036` | BUG | UI-036: Font 中文懒烘培 + CJK 字体可配置接入（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-037` | BUG | UI-037: 直角矩形 50% 半透明蒙版修复（SDF 内部距离项）（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-038` | BUG | UI-038: ui_gl 库内置 draw_quad 不透明回归自检（2026-08-10） | `docs/CHANGELOG.md` |
| `BUG:UI-039` | BUG | UI-039: 轻量组件下沉 ui 库 + Grid layui-row 无背景 + clear_children 统一切页释放（2026-08-11） | `docs/CHANGELOG.md` |
| `BUG:UI-040` | BUG | UI-040: Table 行重建泄漏修复 + Widget::destroy() 单控件销毁 API（2026-08-11） | `docs/CHANGELOG.md` |
| `BUG:UI-041` | BUG | UI-041: 栅格预览对齐 layui 镜像与工具色列间隙修正（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-042` | BUG | UI-042: API 文档生成器（tools/api_gen.py）结构化解析修复（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-043` | BUG | UI-043: API 总调用示例应用（apps/api_demo.cpp）与文档同步（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-044` | BUG | UI-044: AudioOut waveOut 回调 ABI 修复（x86 音频崩溃）（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-045` | BUG | UI-045: 工具色前缀解析修复（layui-bg-* 全部渲染成黑色）（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-046` | BUG | UI-046: Font 烘培计数误用 stbtt_BakeFontBitmap 返回值（ASCII/图标大量字形缺失）（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-047` | BUG | UI-047: 布局外壳 NavMenu 折叠不回收空间 / 切页滚动残留 / 图片加载能力缺失（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-048` | BUG | UI-048: Image 资产定位依赖 CWD / 加载噪音错误（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-049` | BUG | UI-049: 框架页迷你复刻不可交互 / 用户区不符网站 / 右上角 HUD 闪烁（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-050` | BUG | UI-050: 框架页内容主体可滚动 + 头部 nav groups 折叠菜单 + NavMenu 子项选中态（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-051` | BUG | UI-051: make_layui button_normal 误设主色导致布局容器全绿（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-052` | BUG | UI-052: 按钮页三缺陷：尺寸行无大小差 / 按钮组后续按钮缺失 / 按钮容器缺失（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-053` | BUG | UI-053: 按钮文本垂直偏移 ~4px + 圆角按钮描边矩形化（2026-08-16） | `docs/CHANGELOG.md` |
| `BUG:UI-054` | BUG | UI-054: 圆角按钮描边内部被描边色整块盖满（"蒙版"）——圆角环内缘误当作"再叠一层"（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-055` | BUG | UI-055: 文字像素对齐——draw_text/draw_icon 字形对齐像素网格（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-056` | BUG | UI-056: 字体图集 2x 过采样——字形按 2×pixel_height 光栅化（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-057` | BUG | UI-057: 嵌套 scissor pop 不恢复上级矩形——TextBox 之后内容被裁剪（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-058` | BUG | UI-058: 表单页一比一复刻 layui form.html 预览内容（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-059` | BUG | UI-059: Breadcrumb set_separator() API + 面包屑页克隆 layui nav.html 三个 demo（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-060` | BUG | UI-060: NavBar 横向导航组件 + 导航/面包屑页完整克隆 layui nav.html（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-061` | BUG | UI-061: Menu 基础垂直菜单组件 + menu.html 克隆（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-062` | BUG | UI-062: Tabs 组件增强（内容区/动态操作/删除/超宽响应式）+ 选项卡页一比一复刻 demo/tab.html（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-063` | BUG | UI-063: Tabs 析构二次释放崩溃修复（content_label_ 子节点重复 delete）（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-064` | BUG | UI-064: Progress 组件增强（layui-bg-* 颜色类/自定义文本/默认填充 #5fb878）+ 进度条页一比一复刻 demo/progress.html（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-065` | BUG | UI-065: 面板页一比一克隆 demo/panel.html 全部 5 组 demo + Collapse 手风琴/非手风琴双模式（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-066` | BUG | UI-066: 徽章页一比一克隆 demo/badge.html 两组 demo + Button 内嵌徽章/Tabs 标题徽章 + Badge 圆点与 NavBar 徽章修正（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-067` | BUG | UI-067: 时间线页一比一克隆 demo/timeline.html 两组 demo + Timeline 富内容/轴点图标/悬停反馈/旋转动画（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-068` | BUG | UI-068: 静态表格页一比一克隆 demo/table-element.html 全部五组 demo + Table 皮肤/尺寸/偶数行着色（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-069` | BUG | UI-069: 弹出层页一比一克隆 demo/layer.html（多窗口置顶 Esc / 透明询问框 / 公告层遮罩）+ PanelWindow 扩展（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-070` | BUG | UI-070: 弹出层页触发按钮零宽重叠（无点击反馈）修复（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-071` | BUG | UI-071: 弹窗销毁后根节点 popups_ 残留悬空指针崩溃（读取访问权限冲突）（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-072` | BUG | UI-072: 弹出层页按钮回调捕获栈局部 PageHost 悬空引用崩溃（open_layer 读取访问冲突）（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-073` | BUG | UI-073: 事件分发期间 widget 自毁悬空崩溃（flush 延迟销毁修复）（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-074` | BUG | UI-074: PanelWindow 子控件定位在主窗口左上角（on_layout 空操作）（2026-08-17） | `docs/CHANGELOG.md` |
| `BUG:UI-075` | BUG | UI-075: Laydate 控件扩展 + laydate_page 17 个 demo（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-076` | BUG | UI-076: Laydate 弹层（input+popup）模式 + laydate_page 40 demo 一比一克隆（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-077` | BUG | UI-077: 数据表格分阶段克隆 1：分页/自定义分页/复选框/单选框/单元格表单（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-078` | BUG | UI-078: 数据表格分阶段克隆 2：合计行/头部工具栏/单元格事件/操作列（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-079` | BUG | UI-079: ScrollBox 切页滚动条闪烁修复（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-080` | BUG | UI-080: 数据表格缺失子页补全：转化静态表格（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-081` | BUG | UI-081: 分页组件全能力补齐 + laypage 页一比一克隆 demo/laypage.html 全部 demo（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-082` | BUG | UI-082: 下拉菜单（Dropdown）完整化 + 1:1 镜像 layui dropdown.html（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-083` | BUG | UI-083: 文件上传 Upload 测试页 1:1 镜像 upload.html 全部 9 个 demo（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-084` | BUG | UI-084: 穿梭框 Transfer 增强 + 1:1 镜像 transfer.html 全部 7 个 demo（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-085` | BUG | UI-085: 树形组件 Tree 增强 + 1:1 镜像 tree.html 全部 8 个 demo（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-086` | BUG | UI-086: 颜色选择器 ColorPicker 增强 + 1:1 镜像 colorpicker.html 全部 demo（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-087` | BUG | UI-087: 滑块 Slider 增强 + 1:1 镜像 slider.html 全部 18 个 demo（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-088` | BUG | UI-088: 评分 Rate 增强 + 1:1 镜像 rate.html 全部 14 个 demo（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-089` | BUG | UI-089: 轮播 Carousel 增强 + 1:1 镜像 carousel.html 三个 demo 组（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-090` | BUG | UI-090: 轮播页布局修正 + 图片改用 file/img（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-091` | BUG | UI-091: Carousel add_image_slide 缺 exe 目录回退（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-092` | BUG | UI-092: 模板引擎 laytpl 增强 + 1:1 镜像 laytpl.html 沙箱（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-093` | BUG | UI-093: 流加载页 1:1 克隆镜像 3 演示 + ui 库图片懒加载（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-094` | BUG | UI-094: 流加载滚动加载修复：ScrollBox 触底回调加 mb 临界距离（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-095` | BUG | UI-095: 流加载页回调捕获栈局部 PageHost 悬空崩溃（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-096` | BUG | UI-096: 文本行修饰页改克隆 layui-code 代码块组件 demo（2026-08-18） | `docs/CHANGELOG.md` |
| `BUG:UI-097` | BUG | UI-097: 项目打包分发机制（2026-08-19） | `docs/CHANGELOG.md` |
| `BUG:UI-098` | BUG | UI-098: 复选框/开关补齐 layui 全皮肤能力 + 复选框页 1:1 克隆（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-099` | BUG | UI-099: 外壳垂直布局按新镜像重构 + 页面分组映射（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-100` | BUG | UI-100: IME 候选窗跟随输入框光标（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-101` | BUG | UI-101: TextBox 选择/剪贴板能力（拖选/Shift 选择/Ctrl+A/C/X/V）（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-102` | BUG | UI-102: 键盘修饰键快照——SendInput 粘贴到自绘输入框丢失 mods（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-103` | BUG | UI-103: TextArea 剪贴板默认实现——回调未注入时 Ctrl+C/X/V 静默失效（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-104` | BUG | UI-104: 侧栏分组逐级嵌套成子级折叠菜单——Menu add_group 未配对 end_group（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-105` | BUG | UI-105: 帧率 HUD 与头部右侧控件重叠——HUD 右对齐压在生态/版本下拉上（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-106` | BUG | UI-106: 侧栏缺失镜像站 14 个页面——基础组整组缺 + 颜色/图标/输入框/选择框/单选框/树形表格/公共类/固定条（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-107` | BUG | UI-107: UI-106 的 14 个占位页全部实现为真实内容页（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-108` | BUG | UI-108: Font/Label 字号缩放与仿粗体 + layui 文字工具色 font_color——公共类页升级真实演示（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-109` | BUG | UI-109: Table 树形层级 add_tree_row + 展开/折叠交互——树形表格页升级真实交互（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-110` | BUG | UI-110: ComboBox 无 ScrollBox 祖先时弹出面板不翻转，贴近窗口底部的下拉尾部选项被窗口底边裁掉（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-111` | BUG | UI-111: ScrollBox 视口锚定 add_pinned（layui fixbar）+ PageHost 栈局部悬空修复——固定条页升级真实常驻工具组（2026-08-21） | `docs/CHANGELOG.md` |
| `BUG:UI-112` | BUG | UI-112: 颜色页一比一克隆镜像 docs/2/color（基色调/辅色调/中色调/七色调/标语）+ --list-pages 调试设施（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-113` | BUG | UI-113: 图标页新增按钮图标段克隆镜像 h2#icon（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-114` | BUG | UI-114: Button 图标右置 set_icon_right + 图标页按钮图标段一比一修正（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-115` | BUG | UI-115: TextArea 垂直滚动条（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-116` | BUG | UI-116: 垂直滚动条 overlay 风格化：自动隐藏 + 悬停加宽 + 三态色（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-117` | BUG | UI-117: 图标页克隆镜像列表 195 项 + HBox 自适应高度与图标字体扩段（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-118` | BUG | UI-118: 图标卡片文本居中 + 数据表抽离重构（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-119` | BUG | UI-119: 表单/选择框/分页页崩溃排查：陈旧增量构建产物 ODR 混合布局 + 崩溃诊断钩子（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-120` | BUG | UI-120: Slider 聚焦外框改为默认不显示 + set_focus_ring 可选开启（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-121` | BUG | UI-121: ComboBox::set_options 就地重载选项——销毁重建孤儿控件（幽灵下拉）根治 API（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-122` | BUG | UI-122: Laypage/Table 分页条区域点击穿透——容器自身 chrome 一律消费（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-123` | BUG | UI-123: Table auto_page_size 扣除分页条高度 + set_paginated 保持页码（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-124` | BUG | UI-124: 表格行单击选中高亮（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-125` | BUG | UI-125: CJK 字形逐帧烘焙配额 + prebake_text 预热（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-126` | BUG | UI-126: Table 流加载支持——触底重检 rearm + 尾注 + on_load_more（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-127` | BUG | UI-127: 流加载/表格首显闪烁——触底重检分帧 + prebake_visible_text 字形预热（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-128` | BUG | UI-128: emoji 渲染回退链——缺字 tofu 防护 + Segoe UI Emoji 单色轮廓烘焙（2026-08-22） | `docs/CHANGELOG.md` |
| `BUG:UI-129` | BUG | UI-129: Table 单元格表单切换回调——set_on_cell_form 点击翻转通知调用方（2026-08-23） | `docs/CHANGELOG.md` |
| `BUG:UI-130` | BUG | UI-130: 开关皮肤统一——draw_switch_skin 共享函数，表格行内开关与 Switch 同源渲染（2026-08-23） | `docs/CHANGELOG.md` |
| `BUG:UI-131` | BUG | UI-131: draw_cell_form 参数错位修复——单元格开关恒走复选分支绘矩形；表单列表头居中（2026-08-23） | `docs/CHANGELOG.md` |
| `BUG:UI-132` | BUG | UI-132: Table preferred_height 高度语义修正——默认内容跟随，新增 set_fixed_height/set_max_content_height（2026-08-23） | `docs/CHANGELOG.md` |
| `BUG:UI-133` | BUG | UI-133: 模态拦截可见性防御——modal 控件隐藏后不再吞输入，修复僵尸模态全 UI 死锁（2026-08-23） | `docs/CHANGELOG.md` |
| `BUG:UI-134` | BUG | UI-134: ScrollBar 可命中区加宽 + Label scissor 裁剪防溢出（2026-08-27） | `docs/CHANGELOG.md` |
| `BUG:UI-135` | BUG | UI-135: 帧级门控渲染完备性——draw 移除脏守卫全树绘制 + hover/pressed/focus/scroll 显式标脏（2026-08-27） | `docs/CHANGELOG.md` |
| `BUG:UI-136` | BUG | UI-136: 帧级门控下 CJK 惰性烘焙自动续帧——待烘焙队列 + bake-epoch 全树重排（2026-08-27） | `docs/CHANGELOG.md` |
| `BUG:UI-137` | BUG | UI-137: 帧级门控下控件状态写统一自报标脏——开关/下拉/滑块事件内直改漏标点（2026-08-27） | `docs/CHANGELOG.md` |
| `BUG:UI-138` | BUG | UI-138: 帧级门控下 TextBox/TextArea 编辑与光标/选区/滚动路径漏标脏（2026-08-27） | `docs/CHANGELOG.md` |
| `BUG:UI-139` | BUG | UI-139: 帧级门控下 Menu 左侧导航 hover 高亮不跟随——on_mouse_move 更新 hover_ 却漏标脏（2026-08-27） | `docs/CHANGELOG.md` |
| `BUG:UI-140` | BUG | UI-140: ScrollBox/Code 覆写 draw 不清自身脏标记→dirty_children_ 常脏，短路 mark_dirty 上抛、容器内控件点击/悬停不刷新（2026-08-27） | `docs/CHANGELOG.md` |
| `BUG:UI-141` | BUG | UI-141: Transfer 双侧列表滚轮改 scroll 漏 mark_dirty——同类漏标脏补全（2026-08-27） | `docs/CHANGELOG.md` |
| `BUG:UI-142` | BUG | UI-142: ScrollBox 固定头部 + 背景直角化——页面顶部留白固定且与左栏拼全宽横条（2026-08-27） | `docs/CHANGELOG.md` |
| `BUG:UI-143` | BUG | UI-143: Table 新增下拉单元格列——表格行内下拉选择（2026-08-30） | `docs/CHANGELOG.md` |
| `BUG:UI-144` | BUG | UI-144: ScrollBox 滚动条按下滑块不再立即跳转，点击轨道空白仍跳转（2026-08-28） | `docs/CHANGELOG.md` |
| `BUG:UI-145` | BUG | UI-145: 全方位优化批次——渲染性能/内存生命周期/代码去重/构建与清理（2026-08-31） | `docs/CHANGELOG.md` |
| `BUG:UI-146` | BUG | UI-146: 新增统一生命周期抽象 ui::Scene——生产者→分配者→消费者→清理重置（2026-09-01） | `docs/CHANGELOG.md` |
| `BUG:UI-147` | BUG | UI-147: 新增自绘系统风格标题条 TitleBar + 窗口居中——无边框窗口可拖动 / 关闭（2026-09-01） | `docs/CHANGELOG.md` |
| `BUG:UI-148` | BUG | UI-148: TitleBar 补齐 最小化/最大化/关闭 三键 + 左上角应用图标；Menu 分组头箭头改用 iconfont + 可联动收起子菜单列（2026-09-01） | `docs/CHANGELOG.md` |
| `BUG:UI-149` | BUG | UI-149: 修复主窗口自绘标题条被顶部 Header 遮住——Header 构造默认 y=0 与 TitleBar 重叠，首帧标题条不可见（2026-09-01） | `docs/CHANGELOG.md` |
| `BUG:UI-150` | BUG | UI-150: 自绘 TitleBar 默认背景与下方 Header 同色导致"无缝融合、标题条不可区分"——改为独立更深灰（2026-09-01） | `docs/CHANGELOG.md` |
| `BUG:UI-151` | BUG | UI-151: 修复主窗口顶部导航题条重叠的深层根因——Header 子项未随父 y 偏移（本库子控件 bounds 为绝对坐标）（2026-09-01） | `docs/CHANGELOG.md` |
| `BUG:UI-153` | BUG | UI-153: 库级「顶部装帧内容区原点」——Scene::set_chrome/content_origin_y，其余控件统一绘制于标题栏下方 + 修复帧率 HUD 与标题条重叠（2026-09-01） | `docs/CHANGELOG.md` |
| `BUG:UI-154` | BUG | UI-154: 子树变更不标脏使长命容器首帧不重排、新子控件在 (0,0) 闪一帧——基类 add_child/remove_child/clear_children 自报标脏（2026-09-01） | `docs/CHANGELOG.md` |
| `BUG:UI-155` | BUG | UI-155: 新增窗口容器 WindowFrame——自带自绘标题栏 + 内容区相对坐标 + 通用头部槽（2026-09-01） | `docs/CHANGELOG.md` |
| `BUG:UI-156` | BUG | UI-156: 标脏三态闭环「标记/去标/重标」——库级 set_visible 重标 + finish_draw_dirty 统一去标 + 清理死码 clear_dirty（2026-09-01） | `docs/CHANGELOG.md` |
| `DOC:API_REFERENCE` | DOC | API_REFERENCE | `docs/distribution/API_REFERENCE.md` |
| `DOC:ARCHITECTURE` | DOC | ARCHITECTURE | `docs/ARCHITECTURE.md` |
| `DOC:BUILD` | DOC | BUILD | `docs/BUILD.md` |
| `DOC:PLAN` | DOC | PLAN | `docs/PLAN.md` |
| `DOC:README` | DOC | README | `docs/README.md` |
| `DOC:UI_API` | DOC | UI_API | `docs/UI_API.md` |
| `META:AGENTS` | META | AGENTS | `AGENTS.md` |
| `META:CHANGELOG` | META | CHANGELOG | `docs/CHANGELOG.md` |

## 全部边

| from | to | kind |
|------|----|------|
| `DOC:ARCHITECTURE` | `BUG:UI-003` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-003` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-003` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-003` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-003` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-004` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-004` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-004` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-004` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-005` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-005` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-005` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-006` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-006` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-006` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-007` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-007` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-008` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-008` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-008` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-008` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-009` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-009` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-010` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-010` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-010` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-010` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-011` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-011` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-011` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-011` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-012` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-013` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-013` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-014` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-015` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-016` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-017` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-018` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-018` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-018` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-019` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-019` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-019` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-019` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-020` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-020` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-021` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-022` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-022` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-022` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-022` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-022` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-023` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-023` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-023` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-023` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-023` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-023` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-023` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-023` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-024` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-024` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-024` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-024` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-025` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-026` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-026` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-027` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-027` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-027` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-028` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-028` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-029` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-029` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-029` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-029` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-029` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-030` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-031` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-032` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-032` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-033` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-033` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-034` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-035` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-035` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-036` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-036` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-037` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-038` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-039` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-039` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-039` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-039` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-039` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-039` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-040` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-040` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-040` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-041` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-041` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-041` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-041` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-045` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-046` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-047` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-047` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-047` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-047` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-047` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-048` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-049` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-050` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-050` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-051` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-052` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-053` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-054` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-055` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-056` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-057` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-058` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-059` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-060` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-061` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-062` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-063` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-064` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-065` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-066` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-066` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-066` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-066` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-066` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-067` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-068` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-069` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-070` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-071` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-071` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-072` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-073` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-073` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-074` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-075` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-076` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-077` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-078` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-079` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-080` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-081` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-082` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-083` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-084` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-085` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-086` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-087` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-088` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-089` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-090` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-091` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-092` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-093` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-094` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-095` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-096` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-096` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-098` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-099` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-099` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-100` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-101` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-102` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-103` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-104` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-105` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-106` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-107` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-108` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-108` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-109` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-110` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-111` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-111` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-112` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-113` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-114` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-114` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-115` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-116` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-117` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-118` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-120` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-121` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-122` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-123` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-124` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-125` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-126` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-127` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-128` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-129` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-130` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-131` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-132` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-133` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-134` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-135` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-136` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-137` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-138` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-139` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-140` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-141` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-142` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-143` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-144` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-145` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-146` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-146` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-146` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-146` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-146` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-146` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-147` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-148` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-149` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-150` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-151` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-153` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-154` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-155` | checklist |
| `DOC:ARCHITECTURE` | `BUG:UI-156` | checklist |
| `DOC:BUILD` | `BUG:UI-002` | checklist |
| `DOC:BUILD` | `BUG:UI-002` | checklist |
| `DOC:BUILD` | `BUG:UI-002` | checklist |
| `DOC:BUILD` | `BUG:UI-002` | checklist |
| `DOC:BUILD` | `BUG:UI-003` | checklist |
| `DOC:BUILD` | `BUG:UI-003` | checklist |
| `DOC:BUILD` | `BUG:UI-010` | checklist |
| `DOC:BUILD` | `BUG:UI-010` | checklist |
| `DOC:BUILD` | `BUG:UI-119` | checklist |
| `DOC:BUILD` | `BUG:UI-119` | checklist |
| `DOC:PLAN` | `DOC:ARCHITECTURE` | mention |
| `DOC:PLAN` | `DOC:ARCHITECTURE` | mention |
| `DOC:PLAN` | `DOC:ARCHITECTURE` | mention |
| `DOC:PLAN` | `DOC:ARCHITECTURE` | mention |
| `DOC:PLAN` | `META:CHANGELOG` | mention |
| `DOC:PLAN` | `META:CHANGELOG` | mention |
| `DOC:README` | `BUG:UI-001` | checklist |
| `DOC:README` | `BUG:UI-002` | checklist |
| `DOC:README` | `BUG:UI-003` | checklist |
| `DOC:README` | `BUG:UI-003` | checklist |
| `DOC:README` | `DOC:ARCHITECTURE` | mention |
| `DOC:README` | `DOC:BUILD` | mention |
| `DOC:README` | `DOC:BUILD` | mention |
| `DOC:README` | `DOC:PLAN` | mention |
| `DOC:UI_API` | `META:CHANGELOG` | mention |
| `META:AGENTS` | `META:CHANGELOG` | mention |
| `META:AGENTS` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `BUG:UI-001` | ref |
| `META:CHANGELOG` | `BUG:UI-002` | ref |
| `META:CHANGELOG` | `BUG:UI-003` | ref |
| `META:CHANGELOG` | `BUG:UI-004` | ref |
| `META:CHANGELOG` | `BUG:UI-005` | ref |
| `META:CHANGELOG` | `BUG:UI-006` | ref |
| `META:CHANGELOG` | `BUG:UI-007` | ref |
| `META:CHANGELOG` | `BUG:UI-008` | ref |
| `META:CHANGELOG` | `BUG:UI-009` | ref |
| `META:CHANGELOG` | `BUG:UI-010` | ref |
| `META:CHANGELOG` | `BUG:UI-011` | ref |
| `META:CHANGELOG` | `BUG:UI-012` | ref |
| `META:CHANGELOG` | `BUG:UI-013` | ref |
| `META:CHANGELOG` | `BUG:UI-014` | ref |
| `META:CHANGELOG` | `BUG:UI-015` | ref |
| `META:CHANGELOG` | `BUG:UI-016` | ref |
| `META:CHANGELOG` | `BUG:UI-017` | ref |
| `META:CHANGELOG` | `BUG:UI-018` | ref |
| `META:CHANGELOG` | `BUG:UI-019` | ref |
| `META:CHANGELOG` | `BUG:UI-020` | ref |
| `META:CHANGELOG` | `BUG:UI-021` | ref |
| `META:CHANGELOG` | `BUG:UI-022` | ref |
| `META:CHANGELOG` | `BUG:UI-023` | ref |
| `META:CHANGELOG` | `BUG:UI-024` | ref |
| `META:CHANGELOG` | `BUG:UI-025` | ref |
| `META:CHANGELOG` | `BUG:UI-026` | ref |
| `META:CHANGELOG` | `BUG:UI-028` | ref |
| `META:CHANGELOG` | `BUG:UI-029` | ref |
| `META:CHANGELOG` | `BUG:UI-030` | ref |
| `META:CHANGELOG` | `BUG:UI-031` | ref |
| `META:CHANGELOG` | `BUG:UI-032` | ref |
| `META:CHANGELOG` | `BUG:UI-033` | ref |
| `META:CHANGELOG` | `BUG:UI-034` | ref |
| `META:CHANGELOG` | `BUG:UI-035` | ref |
| `META:CHANGELOG` | `BUG:UI-036` | ref |
| `META:CHANGELOG` | `BUG:UI-037` | ref |
| `META:CHANGELOG` | `BUG:UI-039` | ref |
| `META:CHANGELOG` | `BUG:UI-040` | ref |
| `META:CHANGELOG` | `BUG:UI-041` | ref |
| `META:CHANGELOG` | `BUG:UI-042` | ref |
| `META:CHANGELOG` | `BUG:UI-043` | ref |
| `META:CHANGELOG` | `BUG:UI-044` | ref |
| `META:CHANGELOG` | `BUG:UI-046` | ref |
| `META:CHANGELOG` | `BUG:UI-047` | ref |
| `META:CHANGELOG` | `BUG:UI-048` | ref |
| `META:CHANGELOG` | `BUG:UI-049` | ref |
| `META:CHANGELOG` | `BUG:UI-052` | ref |
| `META:CHANGELOG` | `BUG:UI-053` | ref |
| `META:CHANGELOG` | `BUG:UI-055` | ref |
| `META:CHANGELOG` | `BUG:UI-057` | ref |
| `META:CHANGELOG` | `BUG:UI-059` | ref |
| `META:CHANGELOG` | `BUG:UI-060` | ref |
| `META:CHANGELOG` | `BUG:UI-061` | ref |
| `META:CHANGELOG` | `BUG:UI-062` | ref |
| `META:CHANGELOG` | `BUG:UI-068` | ref |
| `META:CHANGELOG` | `BUG:UI-069` | ref |
| `META:CHANGELOG` | `BUG:UI-070` | ref |
| `META:CHANGELOG` | `BUG:UI-071` | ref |
| `META:CHANGELOG` | `BUG:UI-072` | ref |
| `META:CHANGELOG` | `BUG:UI-075` | ref |
| `META:CHANGELOG` | `BUG:UI-077` | ref |
| `META:CHANGELOG` | `BUG:UI-097` | ref |
| `META:CHANGELOG` | `BUG:UI-098` | ref |
| `META:CHANGELOG` | `BUG:UI-117` | ref |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:ARCHITECTURE` | mention |
| `META:CHANGELOG` | `DOC:BUILD` | mention |
| `META:CHANGELOG` | `DOC:BUILD` | mention |
| `META:CHANGELOG` | `DOC:BUILD` | mention |
| `META:CHANGELOG` | `DOC:UI_API` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
| `META:CHANGELOG` | `META:CHANGELOG` | mention |
