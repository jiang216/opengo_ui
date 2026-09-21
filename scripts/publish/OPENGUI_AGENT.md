# opengui MCP —— opengl_ui 智能体知识服务（随 install/ 发布）

本目录是 **opengl_ui MCP 知识服务**（零依赖 stdlib 实现，无 pip/venv/网络）：

```
mcp/
  server.py             MCP stdio 服务（tools/resources/prompts，协议 2025-06-18）
  catalog.json          组件/头文件/方法签名/示例目录（tools/api_gen.py 同源生成）
  templates.json        可改装用法模板 + 工程脚手架（tools/mcp_data.py 维护）
  constraints.json      回归约束（带 CHANGELOG UI-### 溯源）
  CHANGELOG.md          历史条目（opengui_get_changelog 检索用）
  ARCHITECTURE.md       架构与回归清单（检索用）
register-mcp.ps1        一键注入下列客户端配置
publish.ps1             包装入口（委托 register-mcp.ps1）
OPENGUI_AGENT.md        给「消费方智能体」的指引（本文件）
```

## 给其他智能体 / 人类的一句话

写 opengl_ui 代码前，先通过 `opengui_*` MCP 工具查证组件、模板与回归约束，
**禁止凭记忆猜 API**。典型闭环：

1. `opengui_get_template("window-skeleton")` —— 推荐入口骨架（`WindowContainer`）。
2. `opengui_get_component("<组件>")` —— 真实方法签名 + layui 类 + 示例。
3. `opengui_get_regression()` —— 易错点铁律（文本居中/释放/标脏/media 等，带 UI-### 溯源）。
4. 初始化工程 → `opengui_get_scaffold("app" | "app-media")` + `opengui-build-app` prompt。

详见服务内 `opengui_help` 工具（完整流程 + 铁律 + 工具表）。

## 注入客户端

```powershell
# 列出当前状态（安装目录自动从脚本位置推断）
pwsh share/opengl_ui/register-mcp.ps1 -Mode status

# 注入 opencode + Claude Code + 当前项目 .mcp.json
pwsh share/opengl_ui/register-mcp.ps1 -Mode register

# 卸载注入（保留原有配置键）
pwsh share/opengl_ui/register-mcp.ps1 -Mode unregister
```

常用开关：`-Targets opencode,claude,mcpjson` / `-Install <install目录>` /
`-Python python3` / `-ProjectDir <消费项目>` / `-WriteNotes`（把指引追加进消费项目
AGENTS.md）。

- 只读文本知识层，**不做屏幕自动化/OCR/鼠标操作**，与仓库「无 ai_ocr_tools 类 GUI
  工具」原则一致。
- 配置按目标深合并写入（保留其它既有 mcpServers），改前自动备份 `.bak`。