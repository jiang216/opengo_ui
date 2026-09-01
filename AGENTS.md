## 文档图谱 graphgen（已安装）

> 由 `python tools/graphgen.py --init` 安装。产物 `docs/graph.json` / `docs/GRAPH.md` / `docs/graph.mmd` 为 **AUTO-GENERATED，禁手改**。

### 汇报语言

- 开发 / 改动 / 排查工作完成后，用**中文**汇报：做了什么、验证结果、遗留问题。
- 汇报必须写明根因归属：问题出现的原因在 **ui 库**（core/gl/ui/media）还是 **测试程序**（apps、layui_test）；若属 ui 库缺陷，须说明是否已按「改动工作流」强制规定修正 ui 库（禁止测试程序内绕过或掩盖）。

### 强制记忆（资源路径 / 参考库）

- `3rdparty/src/` 是第三方源码 + 参考素材的**镜像库**（来源清单见 `3rdparty/sources.md`）：
  - `3rdparty/src/layui/` = **layui 官方站点整站镜像**（layui.com 克隆），UI 组件的**参考资源库**：
    组件样式 / 图片 / 图标 / 示例素材按需取自 `static/images/layui/`（头像等用 `logo.png`、`logo-2.png`）、
    `avatar/`、`demo/`、`doc/`、`layer/`、`extend/`、`jquery/`、`mobile/` 等；
  - `3rdparty/src/layui-v2.13.8/layui/` = layui 2.13.8 发行包（dist）；
  - 其余镜像：glfw / ffmpeg / opencv 等源码同在此根下。
- 运行时图片等资产：build 期 POST_BUILD 拷贝 `logo.png` / `iconfont.ttf` / `glfw3.dll` 到 exe 目录，
  配合 ui 库 `Image::load_from_file` 的 **exe 目录回退**（[UI-048](docs/CHANGELOG.md#ui-048-image-资产定位依赖-cwd--加载噪音错误2026-08-16)）
  即可不依赖 CWD 加载；需新素材优先从上述镜像库取，**禁止**在测试程序内堆叠 CWD 相对路径回退链。

### 改动工作流（强制，防错误复发）

- **先归因再修复（强制）**：任何 Bug / 异常 / 行为不符，改动前必须先判定根因归属——
  - 属 **ui 库**（core/gl/ui/media）缺陷 → **修正 ui 库**，禁止在测试程序 / 示例应用
    （apps、layui_test）内绕过或打补丁掩盖；
  - 属 **测试程序**（apps、layui_test）问题 → 修正测试程序，但其唯一目的是**测试 / 使用 ui 库**：
    凡暴露的是 ui 库能力缺陷，一律回归到 ui 库修复；测试程序只做编排与验证，
    不得承载本应属于 ui 库的逻辑。
- **改动前必读**：先读与改动相关的 `docs/*.md` 功能文档 + `docs/CHANGELOG.md`（条目文件）相关条目及其「必须保证的功能（回归清单）」，逐项核对不得破坏。
- **改动后必更**：
  - 涉及已有功能 → 核对回归清单项是否仍成立；新增保证项 → 加入对应文档回归清单。
  - 修 Bug / 加功能 → 在 `docs/CHANGELOG.md` **追加**条目（严重度 / 根因 / 修复 / 验证），**防止错误重复出现**；功能文档只放引用 `[xx-###](CHANGELOG.md#...)`，不得内嵌详情。
- **改动后必提交（强制自提交）**：一项 Bug 修复 / 功能 / 加固工作**完成且验证通过后，自行 git 提交**，无需等用户显式说「提交」。每个逻辑变更（一个 CHANGELOG 编号 = 一个逻辑单位）独立 commit，信息与 CHANGELOG 编号对应（如 `UI-###: ...`）。提交前核查只暂存本次相关文件、不夹带无关改动、不提交密钥。

### 图谱同步（强制）

- 涉及 `docs/`、`AGENTS.md`、`.opencode/skills/*/SKILL.md`、`tools/graphgen.config.json` 的改动完成后：
  1. `$env:PYTHONIOENCODING='utf-8'`（Windows 防中文乱码）
  2. `python tools/graphgen.py` 重新生成图谱
  3. `python tools/graphgen.py --check` 必须 0 错误 0 警告
- 查反链优先读 `docs/graph.json`（每节点自带 `backlinks[]`），勿对 docs/ 全库 grep。
- 节点 id：`BUG:xx-###`（条目文件，如 CHANGELOG）/ `DOC:xxx`（docs/*.md）/ `SKILL:xxx`（技能）/ `META:xxx`（AGENTS、条目文件本体）/ `SRC:xxx`（源码映射表）。
- 边 kind：`ref`（相关历史条目引用）/ `checklist`（回归清单保证项，context 带原文）/ `mention`（正文链接）/ `index`（索引表）/ `owns`（源码归属）。
- 配置字段说明见本仓库 `tools/graphgen-dist/README.md`（模板：`tools/graphgen.config.json`）。

<!-- graphgen:installed -->
