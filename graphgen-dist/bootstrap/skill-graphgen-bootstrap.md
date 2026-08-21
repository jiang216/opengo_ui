---
name: graphgen-bootstrap
description: When the project contains a graphgen-dist/ folder (self-contained distribution of the graphgen markdown-docs knowledge-graph generator) but graphgen is not installed yet, auto-install it by reading the package and running its --init scaffold; also use when asked to install/use graphgen or the doc graph.
---

# graphgen 自安装引导技能

> 目的：当项目里出现 `graphgen-dist/`（graphgen 分发包）但尚未安装时，由你（智能体）读取该包并**自动完成安装**，生成 `tools/`、`.opencode/skills|plugins`、配置与文档图谱。
> 全局安装：本技能常驻 `~/.config/opencode/skills/graphgen-bootstrap/`，任何项目都可用。

## 何时使用

- 项目根目录存在 `graphgen-dist/`（或 `tools/graphgen-dist/`）文件夹，且 `tools/graphgen.py` / `.opencode/skills/graphgen/SKILL.md` 不存在（未安装）。
- 用户说「安装 graphgen」「把 graphgen-dist 用起来」「生成文档图谱」等。

## 安装步骤

1. **定位分发包**：`graphgen-dist/` 通常直接在项目根，也可能是 `tools/graphgen-dist/`。记下它的绝对路径。
2. **判断是否已装**：`tools/graphgen.py` 与 `.opencode/skills/graphgen/SKILL.md` 任一不存在即为未装；已装则跳到第 6 步验证。
3. **读安装文档**：阅读 `<dist>/INSTALL.md` 确认一键命令与前置要求（Python 3.10+，Windows 先设 `$env:PYTHONIOENCODING='utf-8'`）。
4. **一键安装**（在项目根目录执行）：

   ```powershell
   $env:PYTHONIOENCODING='utf-8'
   python <dist>\graphgen.py --init <项目根目录> [--name 项目名]
   ```

   `--name` 缺省用项目目录名。`--init` 幂等：已存在的文件一律跳过，可放心重复执行。它自动生成：
   `tools/graphgen.py`、`tools/graphgen.config.json`、`.opencode/skills/graphgen/SKILL.md`、
   `.opencode/plugins/graphgen-watch.ts`、`.opencode/package.json`、`AGENTS.md` 工作流片段（项目已有 AGENTS.md 时）、`tools/graphgen-dist/README+INSTALL` 参考副本。
5. **首次配置**：编辑 `tools/graphgen.config.json`：
   - `prefixes` 必须填项目 CHANGELOG 的条目前缀表（如 `{"AA": "Alpha"}`），lint 会校验未知前缀；
   - 若项目还没有 `docs/` 与条目文件，先创建（默认 `docs/CHANGELOG.md`，格式为 `## 前缀-编号 标题`）。
6. **验证**：
   ```powershell
   python tools\graphgen.py --check   # 期望 0 errors / 0 warnings
   python tools\graphgen.py           # 生成 docs/graph.json / GRAPH.md / graph.mmd
   ```
7. **提示重启**：安装后 opencode 需**重启**才能加载新的 graphgen 技能/插件（`.opencode/` 在启动时扫描）。重启后即可用 `graphgen_query` 工具与自动重生成。

## 注意事项

- 若 `--init` 因 python 不在 PATH 失败：提示用户装 Python 或用完整路径；也可退化为「方式 B 手动拷贝」（见 INSTALL.md）。
- `graphgen-dist/` 本身是**分发包**（含引擎副本），安装后可以留着作参考，也可删除（`--init` 已把引擎装到 `tools/`）。
- 安装完成后，日常使用/查反链参考已安装的 `graphgen` 技能（`.opencode/skills/graphgen/SKILL.md`）。
