# OpenGL UI

C++ OpenGL UI 框架（CMake 多模块工程）。面向网站的产品介绍 / 落地文案见 [INTRO.md](INTRO.md)，里程碑计划见 [PLAN.md](PLAN.md)，四层架构见 [ARCHITECTURE.md](ARCHITECTURE.md)，依赖构建见 [BUILD.md](BUILD.md)。

## 必须保证的功能（回归清单）

| 功能 | 来源 |
|------|------|
| 文档图谱与 CHANGELOG 条目可被 `python tools\graphgen.py` 正常生成与校验 | UI-001 |
| `scripts/build/*.ps1` 双架构 x64/x86 一键构建全部 3rdparty 依赖并安装到 `3rdparty/install/{x64,x86}` | UI-002 |
| 顶层 CMake：C++17、media 开关默认 OFF、按架构推导 `OPENGUI_3RDPARTY` 前缀 | UI-003 |
| Demo（apps/opengl_ui_demo）开窗渲染可运行，media ON/OFF 双配置均可编译；左侧列用 `ScrollBox`+`VBox` 流式堆叠：起始 y 让出内容区原点下方（标题条+菜单栏），整列可滚动、随窗口高度同步 | UI-003 · [UI-174](CHANGELOG.md#ui-174-修复-opengluidemo-左侧列卡片与顶栏重叠--无滚动appsmaincpp2026-09-01) |
| opengui MCP 知识服务随 `install/` 发布（`share/opengl_ui/mcp/`），`scripts/publish/register-mcp.ps1` 可一键注入 opencode / Claude Code / `.mcp.json` | [UI-208](CHANGELOG.md#ui-208-opengui-mcp-知识服务零依赖-stdlib-服务--apigen-同源目录--发布期一键注入-idecli-智能体toolsmcpserverpy--toolsapigenpy--toolsmcpdatapy--scriptspublishregister-mcpps12026-09-21) |

## 智能体 MCP 知识服务（opengui）

写 opengl_ui 代码的智能体（opencode / Claude Code 等）可经 MCP 查证「组件 / 模板 / 回归约束」，
**禁止凭记忆猜 API**。随包发布与注入：

```powershell
# install 后（install/share/opengl_ui/ 下），注入 opencode + Claude Code + 当前项目 .mcp.json
pwsh install/share/opengl_ui/register-mcp.ps1 -Mode register
pwsh install/share/opengl_ui/register-mcp.ps1 -Mode status     # 查看
pwsh install/share/opengl_ui/register-mcp.ps1 -Mode unregister # 卸载（保留既有配置键）
```

- 数据同源生成：`tools/api_gen.py` 输出 `docs/mcp/{catalog,templates,constraints}.json`（禁手改）+ 人工事实源 `tools/mcp_data.py`；`--check` 一并校验。
- 服务零第三方依赖（纯 stdlib，`pip install` / venv / 外网均不需要）；`--selfcheck` 内置回环自检。
- 消费方指引见 `scripts/publish/OPENGUI_AGENT.md` 与工具内 `opengui_help`。细节与回归要求见 [CHANGELOG UI-208](CHANGELOG.md#ui-208-opengui-mcp-知识服务零依赖-stdlib-服务--apigen-同源目录--发布期一键注入-idecli-智能体toolsmcpserverpy--toolsapigenpy--toolsmcpdatapy--scriptspublishregister-mcpps12026-09-21)。

## 示例应用（M16 收官）

两个可运行应用随对应 media 开关构建，POST_BUILD 自动拷贝依赖 DLL 与素材，开箱可运行：

- **image_viewer（`OPENGUI_MEDIA_OPENCV=ON`）**：图像查看器。按钮 / Ctrl+O 打开图片（Win32
  宽字符文件对话框，支持中文路径），滚轮缩放（以鼠标为锚点），左键拖拽平移，双击复位，
  状态栏显示文件名 / 尺寸 / 缩放。用法：`opengl_ui_image_viewer [图片路径] [--smoke <秒>]`。
- **video_player（`OPENGUI_MEDIA_FFMPEG=ON`）**：视频播放器。按钮 / Ctrl+O 打开视频，播放/暂停
  （按钮或空格键），进度条 seek，循环开关，倍速 0.5x..2x，音量 0..1。用法：
  `opengl_ui_video_player [视频路径] [--smoke <秒>]`。

构建与冒烟示例（x64，详见 [BUILD.md](BUILD.md)）：

```text
cmake -S . -B build-media -DCMAKE_BUILD_TYPE=Release -DOPENGUI_MEDIA_FFMPEG=ON -DOPENGUI_MEDIA_OPENCV=ON
cmake --build build-media --target opengl_ui_image_viewer opengl_ui_video_player
build-media/apps/opengl_ui_image_viewer.exe --smoke 2
build-media/apps/opengl_ui_video_player.exe --smoke 3
```
