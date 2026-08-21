# graphgen 安装文档

graphgen-watch 插件 / 文档图谱生成器的分发包安装指南。本包**自包含**：只需整体拷走即可在任何项目安装。

> 配置字段详细参考见同目录 `README.md`；本文只讲安装。

## 1. 包内容（文件清单）

| 文件 | 说明 | 必装 |
|------|------|------|
| `graphgen.py` | 引擎（纯 stdlib，无第三方依赖，核心分发物） | 是 |
| `graphgen.config.template.json` | 配置模板（`{{NAME}}` 占位由安装替换为项目名） | 是 |
| `plugin-graphgen-watch.ts` | opencode 插件：文件事件自动重生成 + `graphgen_query` 工具 | 可选 |
| `skill-graphgen.md` | opencode 技能（教智能体查反链/跑生成器） | 可选 |
| `AGENTS-snippet.md` | 追加到目标项目 `AGENTS.md` 的工作流片段（含 `<!-- graphgen:installed -->` marker） | 可选 |
| `bootstrap/` | 全局自安装引导器（skill + plugin + install 脚本），装到接收机 `~/.config/opencode/` 一次，之后任意项目拷贝本包即自动安装 | 可选 |
| `INSTALL.md` | 本文档 | — |
| `README.md` | 包概述 + 配置字段参考 | — |

## 2. 前置要求

- **Python 3.10+**（仅用标准库 json/re/pathlib）
- 目标项目已有 `docs/` 目录，以及一份条目文件（默认 `docs/CHANGELOG.md`，格式见模板 `entryFiles.heading`）
- opencode 增强（skill/plugin）**可选**，不装也能用命令行
- Windows 下建议先 `$env:PYTHONIOENCODING='utf-8'` 防中文乱码

## 3. 安装

### 方式 A：一键 `--init`（推荐）

本包已含引擎，可直接从包目录引导，无需打开 openfbx 仓库：

```powershell
python <包目录>\graphgen.py --init <目标项目目录> [--name 项目名]
```

示例（包解压到 `D:\graphgen-dist`，安装到 `D:\code\myapp`）：

```powershell
python D:\graphgen-dist\graphgen.py --init D:\code\myapp --name myapp
```

`--init` 自动完成（全部**幂等**，可重复执行；已存在文件一律跳过）：

| 目标产物 | 说明 |
|----------|------|
| `<目标>/tools/graphgen.py` | 引擎副本 |
| `<目标>/tools/graphgen.config.json` | 配置实例化（`{{NAME}}` → 项目名） |
| `<目标>/.opencode/skills/graphgen/SKILL.md` | 技能（可选） |
| `<目标>/.opencode/plugins/graphgen-watch.ts` | 插件（可选） |
| `<目标>/.opencode/package.json` | `@opencode-ai/plugin` 依赖（仅当缺省时写入） |
| `<目标>/AGENTS.md` | 无则**自动创建**；已有则幂等追加工作流片段（marker 防重复）。内容含「中文汇报 / 改动工作流（改动前读模块文档与条目日志、bug 修正追加错误日志防重复）/ 图谱同步」 |
| `<目标>/tools/graphgen-dist/` | README + INSTALL 参考副本 |

### 方式 C：一次性全局自安装（推荐接收方，零指令）

在本机 `~/.config/opencode/` 装一次「全局引导器」，之后**任意项目**只要拷入 `graphgen-dist/`，
用 opencode 打开即可**自动安装**，无需任何命令或指令。

```powershell
# Windows
pwsh -File <包目录>\bootstrap\install-bootstrap.ps1
# macOS / Linux
sh <包目录>/bootstrap/install-bootstrap.sh
```

引导器 = 一个全局**技能**（`skills/graphgen-bootstrap/SKILL.md`，教智能体发现 `graphgen-dist/` 后按 INSTALL 自助安装）
+ 一个全局**插件**（`plugins/graphgen-bootstrap.ts`，启动时检测「有 `graphgen-dist/` 且未安装」→ 自动执行 `--init`，幂等）。

装好后的使用方式：

1. 把 `graphgen-dist/` 整个文件夹复制进目标项目（放根目录或 `tools/` 下均可）。
2. 用 opencode 打开该项目。**插件**会在启动时自动完成安装（日志见 `service=graphgen-bootstrap`）。
   若插件因环境（如 python 不在 PATH、权限）未成功，**智能体**会按引导技能读到该文件夹并自助安装。
3. 安装后项目根会生成/追加 `AGENTS.md`，内含：**中文汇报**、**改动工作流**（改动前读相关模块文档与 `docs/CHANGELOG.md` 条目/回归清单；修 Bug / 加功能必须追加条目到 CHANGELOG 防错误复发）、**图谱同步** 三段规则——后续智能体据此工作。
4. 首次配置 `tools/graphgen.config.json` 的 `prefixes`，跑 `python tools\graphgen.py --check` 验证。
5. 重启 opencode，graphgen 技能/插件正式生效。

> 注意：全局引导器只需每台接收机装一次；它不是通过 `--init` 复制进目标项目的。

### 方式 B：手动拷贝

```powershell
# 1. 引擎 + 配置模板（必需）
Copy-Item <包>\graphgen.py, <包>\graphgen.config.template.json <目标>\tools\
# 2. 模板改名并编辑（见 README「配置字段参考」）
Rename-Item <目标>\tools\graphgen.config.template.json graphgen.config.json

# 3. 可选增强：技能 + 插件 + 依赖声明
#    <目标>\.opencode\skills\graphgen\SKILL.md            ← 内容 = skill-graphgen.md
#    <目标>\.opencode\plugins\graphgen-watch.ts          ← 内容 = plugin-graphgen-watch.ts
#    <目标>\.opencode\package.json 加：
#    {"dependencies": {"@opencode-ai/plugin": "1.18.7"}}

# 4. 可选：把 AGENTS-snippet.md 内容追加到 <目标>\AGENTS.md

# 5. 首次生成（必需）
cd <目标>
$env:PYTHONIOENCODING='utf-8'
python tools\graphgen.py
```

## 4. 首次配置（最小）

编辑 `<目标>/tools/graphgen.config.json`：

1. `name`：项目名
2. `prefixes`：CHANGELOG 条目前缀表，如 `{"G": "Gizmo", "CAM": "Camera"}`（lint 会校验未知前缀，**必须填**）
3. `output` 三个产物路径 + `scan.ignore` **务必包含三个产物**（防自扫描循环）
4. 其它按需（`tableEdges` 等，见 README）

## 5. 安装后验证

```powershell
python tools\graphgen.py --check   # 期望 0 errors / 0 warnings
python tools\graphgen.py           # 生成 docs/graph.json / GRAPH.md / graph.mmd
```

opencode 内验证（装了插件后**重启 opencode** 使插件生效）：

- 工具 `graphgen_query`：`action=check`、`action=query node=BUG:xxx`（支持简称如 `g-004`）、`action=generate`
- 修改任一 `docs/*.md` → 约 1.5s 后自动重生成（观察日志 `service=graphgen-watch`）

## 6. 卸载

删除目标项目内：`tools/graphgen.py`、`tools/graphgen.config.json`、`tools/graphgen-dist/`、
`.opencode/skills/graphgen/`、`.opencode/plugins/graphgen-watch.ts`、
`docs/graph.json` / `docs/GRAPH.md` / `docs/graph.mmd`；
并从 `AGENTS.md` 移除 `## 文档图谱 graphgen（已安装）` 片段。

## 7. 常见问题

- **中文乱码**：先 `$env:PYTHONIOENCODING='utf-8'`。
- **`--check` 报「未知前缀」**：在 `prefixes` 补对应前缀，或修正 CHANGELOG 标题。
- **产物文件名冲突**：产物固定 `docs/GRAPH.md`（大写）。功能文档**勿用 `graph.md`** 命名（Windows 大小写不敏感文件系统上与产物同一文件）。
- **插件不生效**：确认 `.opencode/package.json` 含 `@opencode-ai/plugin`，并重启 opencode（自动 `bun install`）。
- **引擎不在包内？** 本包必须含 `graphgen.py`；若缺少，回到 openfbx 仓库执行 `Copy-Item tools\graphgen.py tools\graphgen-dist\graphgen.py` 重新打包。
