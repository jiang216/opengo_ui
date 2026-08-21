# 里程碑计划（Roadmap）

本文件为项目里程碑计划与完成判定的唯一事实源。已完成条目详情见 [CHANGELOG.md](CHANGELOG.md)，
回归保证见 [ARCHITECTURE.md](ARCHITECTURE.md)「必须保证的功能（回归清单）」。

## 目标与终点

交付一个可用的自研 C++ OpenGL UI 框架（core/gl/ui/media/apps 四层）+ 两个可运行示例应用
（图像查看器、视频播放器），全部核心控件同时支持鼠标与键盘操作。

**终点（完成判定）**：M15 交付后全部里程碑标记完成，两个示例应用开箱可运行，
回归清单全绿（[ARCHITECTURE.md](ARCHITECTURE.md)），`python tools/graphgen.py --check` 0 错误 0 警告。

## 里程碑总览

| 里程碑 | 内容 | 交付条目 | 状态 |
|--------|------|----------|------|
| M0 | 项目文档图谱 + 3rdparty 依赖管线 + 四层骨架 | UI-001 ~ UI-003 | 完成 |
| M1 | 渲染管线（圆角 SDF / 纹理 / 字体）+ 控件树 + Button | UI-004 | 完成 |
| M2 | 布局系统：VBox / Label + 滚轮 / 窗口缩放 | UI-005 | 完成 |
| M3 | 焦点系统 + 键盘输入 | UI-006 | 完成 |
| M4 | 文本输入：TextBox + 字符通道 + scissor | UI-008 | 完成 |
| M5 | 鼠标拖拽：Slider + 鼠标捕获 | UI-009 | 完成 |
| M6 | 媒体模块点亮：ImageView / VideoView | UI-010 | 完成 |
| M7 | 视频解码线程化 + 播放控制 + 交互收尾 | UI-011 ~ UI-012 | 完成 |
| M8 | 滚动容器：ScrollBox | UI-013 | 完成 |
| M9 | 复合控件：CheckBox / Radio / ComboBox | UI-014 | 完成 |
| M10 | 主题系统：暗色 / 亮色切换与样式定制 | UI-015 | 完成 |
| M11 | 多行文本排版 | UI-016 | 完成 |
| M12 | 布局增强：HBox / Grid / 嵌套滚动 | UI-017 | 完成 |
| M13 | 视频增强：倍速 / 帧步进 / 精确 seek | UI-018 | 完成 |
| M14 | 窗口系统：弹窗 / 模态框 / 菜单栏 | UI-019 | 完成 |
| M15 | 视频音频输出：AAC 解码 + 音量 + winmm 播放 | UI-020 | 完成 |
| M17a | layui 克隆·基础设施 + 轻量组件（Progress/Badge/Card/Breadcrumb/Timeline/Rate） | UI-022 | 完成 |
| M17b | layui 克隆·表单与按钮（Button 变体/Switch/select 外观/Tabs/Collapse/Dropdown 已完成） | UI-023 | 完成 |
| M17c | layui 克隆·数据与弹层（Table/Tree/Menu/layer/Laypage） | UI-024、UI-025 | 完成 |
| M17d | layui 克隆·图标与特殊组件（iconfont/特殊组件/Demo 展示页）+ 收官 | UI-026 | 完成 |
| M16 | 收官：示例应用（图像查看器 + 视频播放器） | UI-027 | 完成（终点） |
| M18 | layui_test 克隆工程：demo 网站逐页克隆（已实现 30/30 页，全部完成） | UI-028 ~ UI-034 | 完成 |

## 已完成里程碑（M0–M15）

- M0：UI-001（文档图谱）、UI-002（依赖管线）、UI-003（四层骨架）。
- M1：UI-004（渲染管线升级）。
- M2：UI-005（布局系统）。
- M3：UI-006（焦点系统与键盘输入）。
- M4：UI-008（文本输入）。
- M5：UI-009（Slider 与鼠标捕获）。
- M6：UI-010（媒体模块点亮）。
- M7：UI-011（视频解码线程化 + 播放控制）、UI-012（EOF 回播 + 解码节流）。
- M8：UI-013（滚动容器 ScrollBox）。
- M9：UI-014（复合控件 CheckBox / RadioGroup / ComboBox）。
- M10：UI-015（主题系统暗/亮预设 + 运行期切换）。
- M11：UI-016（Label 多行文本排版）。
- M12：UI-017（HBox / Grid / 嵌套滚动）。
- M13：UI-018（视频倍速 / 帧步进 / 精确 seek）。
- M14：UI-019（PanelWindow / MenuBar / 模态 / Tooltip）。
- M15：UI-020（视频音频输出：AAC + 音量 + winmm 播放）。
- M17a：UI-022（layui 克隆·基础设施：Widget 类名机制 + layui_style 解析层 + Theme::make_layui +
  轻量组件 Progress / Badge / Card / Breadcrumb / Timeline / Rate）。
- M17b：UI-023（layui 克隆·表单与按钮：Button 变体 + Switch + ComboBox select 外观 + Slider 外观 +
  Tabs / Collapse / Dropdown）。
- M17c（完成）：UI-024（layui 克隆·数据与弹层：Tree + NavMenu + PanelWindow layer 外观 + Laypage）、
  UI-025（Table layui 化 + Demo 展示页）。
- M17d（完成）：UI-026（layui 克隆·图标与特殊组件：iconfont 私用区 + Icon / Carousel / ColorPicker /
  Transfer / Code / Laydate / Upload + Demo 展示页）。
- M16（完成·终点）：UI-027（示例应用：image_viewer 打开/缩放/平移 + video_player 播放/进度/音量/倍速）。

## 已完成里程碑（M17b–M17d / M16）

- M17b：UI-023（layui 克隆·表单与按钮：Button 变体 + Switch + ComboBox select 外观 + Slider 外观 +
  Tabs / Collapse / Dropdown）。
- M17c：UI-024 + UI-025（layui 克隆·数据与弹层：Table / Tree / NavMenu / PanelWindow layer / Laypage）。
- M17d：UI-026（layui 克隆·图标与特殊组件：iconfont 私用区 + 6 特殊组件 + Demo 展示页）。
- M16（终点）：UI-027（示例应用 image_viewer + video_player）。全部里程碑完成，项目收官。

## 终点达成（M16 验收）

- image_viewer / video_player 随对应 media 开关构建，POST_BUILD 自动拷贝依赖 DLL 与素材，开箱可运行；
- 双配置（media OFF / ON）全量编译链接通过，`--smoke` 冒烟 exit 0；
- 回归清单全绿（[ARCHITECTURE.md](ARCHITECTURE.md)），`python tools/graphgen.py --check` 0 错误 0 警告；
- 文档（README / BUILD / ARCHITECTURE / PLAN / CHANGELOG）与现状一致。

## 完成定义（DoD）

每个里程碑交付必须同时满足：

1. 双架构 x64/x86 构建链接通过（涉及媒体时再叠加 media ON/OFF 四组）。
2. Demo 冒烟运行无崩溃、无报错。
3. [CHANGELOG.md](CHANGELOG.md) 追加条目；[ARCHITECTURE.md](ARCHITECTURE.md) 回归清单与
   本文件状态同步。
4. `python tools/graphgen.py --check` 0 错误 0 警告。

## 当前基线（2026-08-10）

- 全部里程碑完成：M0–M15（UI-001 ~ UI-020）+ M17a（UI-022）+ M17b（UI-023）+ M17c（UI-024/UI-025）
  + M17d（UI-026）+ M16 收官（UI-027）。项目达成终点。
- 回归清单全绿，`python tools/graphgen.py --check` 0 错误 0 警告。
