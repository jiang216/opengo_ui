# -*- coding: utf-8 -*-
"""
opengl_ui MCP 知识服务（零依赖 stdlib 实现，Python 3.9+，无需 pip/venv/网络）。

用途：给 opencode / Claude Code / 任意 MCP 客户端提供 opengl_ui 的
「查询 + 代码生成 + 脚手架」知识层，防止智能体忘记库功能与调用细节。

数据源：与 server.py 同目录的生成产物（由 tools/api_gen.py 同源生成，禁手改）：
    catalog.json      解析全部公共头 + 人工组件元数据（组件/分类/方法签名/示例/notes）
    templates.json    可改装 C++ 用法模板 + 工程脚手架
    constraints.json  回归约束（带 CHANGELOG UI-### 溯源）
    CHANGELOG.md      供 get_changelog 提取条目（可选，缺失时降级）
    ARCHITECTURE.md   供 opengui_get_regression 上下文（可选）

协议：MCP（JSON-RPC 2.0 over stdio，协议版本 2025-06-18）。
帧传输：newline-delimited JSON 或可选 Content-Length: 头 + 空行（兼容两种客户端）。
能力：tools / resources / prompts。未知方法返回 -32601；非法参数返回 -32602。

运行：
    python server.py            # stdio 服务（MCP 客户端调用）
    python server.py --selfcheck   # 内部回环自检（exit 0/1），发布注入前校验用
    python server.py --data <dir>  # 覆盖数据目录（默认 = 本文件所在目录）
"""
import json
import os
import re
import sys

PROTOCOL_VERSION = "2025-06-18"
SERVER_NAME = "opengui"
SERVER_VERSION = "0.1.0"

# JSON-RPC 错误码
PARSE_ERROR = -32700
INVALID_REQUEST = -32600
METHOD_NOT_FOUND = -32601
INVALID_PARAMS = -32602
INTERNAL_ERROR = -32603


def _log(*parts):
    sys.stderr.write("opengui: " + " ".join(str(p) for p in parts) + "\n")
    sys.stderr.flush()


class DataStore:
    """延迟加载单次的数据仓库。"""

    def __init__(self, data_dir):
        self.dir = data_dir
        self._catalog = None
        self._templates = None
        self._constraints = None
        self._changelog = None
        self._index = None

    # ---- 原始数据 ----
    def _load(self, name, default):
        path = os.path.join(self.dir, name)
        try:
            with open(path, encoding="utf-8") as f:
                return json.load(f)
        except (OSError, ValueError) as e:
            _log("load {} failed: {}".format(name, e))
            return default

    @property
    def catalog(self):
        if self._catalog is None:
            self._catalog = self._load("catalog.json", {})
        return self._catalog

    @property
    def templates(self):
        if self._templates is None:
            self._templates = self._load("templates.json", {"templates": {}, "scaffolds": {}})
        return self._templates

    @property
    def constraints(self):
        if self._constraints is None:
            self._constraints = self._load("constraints.json", {"constraints": []})
        return self._constraints

    @property
    def changelog(self):
        if self._changelog is None:
            path = os.path.join(self.dir, "CHANGELOG.md")
            try:
                with open(path, encoding="utf-8") as f:
                    self._changelog = f.read()
            except OSError:
                self._changelog = ""
        return self._changelog

    # ---- 便捷索引 ----
    def components_by_name(self):
        if self._index is None:
            idx = {}
            for h in self.catalog.get("headers", []):
                for c in h.get("classes", []):
                    if c.get("name"):
                        idx[c["name"]] = c
            self._index = idx
        return self._index

    def header_of(self, header):
        for h in self.catalog.get("headers", []):
            if h.get("header") == header:
                return h
        return None

    def changelog_entry(self, ui_id):
        m = re.search(
            r"^## {}\s*[:::（(]?\s*.*?$".format(re.escape(ui_id)),
            self.changelog, re.M | re.S)
        if not m:
            return ""
        body = self.changelog[m.start():]
        nxt = re.search(r"^## ", body, re.M | re.S)
        return body if not nxt else body[:nxt.start()]


# --------------------------------------------------------------------------
# 工具实现
# --------------------------------------------------------------------------

def _fmt_methods(cls, limit=None):
    ms = cls.get("methods") or []
    if limit and len(ms) > limit:
        ms = ms[:limit]
    if not ms:
        return ""
    lines = []
    for m in ms:
        doc = re.sub(r"\s+", " ", (m.get("doc") or "")).strip()
        lines.append("- `{}({})`{}".format(m["name"], m.get("sig", ""),
                                            "  -- " + doc if doc else ""))
    return "\n".join(lines)


def _component_card(store, comp):
    """组件 → 单页 markdown（名字//头文件/语法类/摘要/示例/方法签名）。"""
    cls = store.components_by_name().get(comp["name"]) or {}
    methods = _fmt_methods(cls)
    classes = "、".join("`" + c + "`" for c in comp.get("layui_classes", [])) or "—"
    out = [
        "## `{}`".format(comp["name"]),
        "",
        "- 头文件：`#include <{}>`".format(comp["header"]),
        "- 分类：{}".format(comp.get("category", "")),
        "- layui 类：{}".format(classes),
        "",
        comp.get("summary", ""),
    ]
    if methods:
        out += ["", "### 方法（完整签名见头文件）", ""] + methods.split("\n")
    ex = comp.get("example") or ""
    if ex:
        out += ["", "### 用法示例", "", ex]
    return "\n".join(out)


def opengui_help():
    return """## opengui 知识服务 — opengl_ui 使用指南

本服务让智能体在写 opengl_ui 代码前先查证组件/模板/约束，避免「凭记忆猜 API」。

### 典型调用流程
1. `opengui_get_template("window-skeleton")` —— 拿推荐入口骨架（WindowContainer）。
2. `opengui_list_components(category=...)` 或 `opengui_get_component("<组件名>")` —— 查组件与真实方法签名/示例。
3. 写复杂页面 → `opengui_get_template("table-page" | "form-page" | "layout" | ...)`。
4. 初始化整个工程 → `opengui_get_scaffold("app")`（或 app-media）→ `opengui-build-app` prompt。
5. 涉及易错点 → `opengui_get_regression("<关键词>")` 查看回归约束（带 CHANGELOG UI-### 溯源）。

### 铁律
- **入口**：优先 `ui::WindowContainer` + `Scene::run()`/`run_for()`，别手写五段主循环
  （否则必须自带 `root.advance(dt)` 冲刷 destroy 队列、`set_chrome()` 首帧前，见 C003）。
- **文本居中**：用 `Font::line_height()`（= ascent−descent），禁止 (ascent+descent)（C001）。
- **asset**：裸文件名加载会自动回退 exe 目录（`Image::load_from_file`），别堆 CWD 路径链（C015）。
- **media**：默认构建关闭；开了必须 Release（C009）；`VideoView` 每帧 `advance(dt)`。
- **释放**：容器子节点可为栈对象；堆对象用 `clear_children()` 或 `destroy()`，别对栈对象 delete（C004）。
- **freestyle 状态写自报标脏**：事件里直接改状态必须 `mark_dirty()`（C006/C012）。

### 可用工具
- opengui_help —— 本页
- opengui_list_components(category?) —— 组件清单
- opengui_get_component(name) —— 组件详情（方法签名+示例）
- opengui_get_template(id) —— 用法模板（可改装）
- opengui_get_scaffold(id) —— 工程脚手架（app / app-media）
- opengui_search(query) —— 关键词检索组件/方法/模板/约束
- opengui_find_class(layui-*) —— 按 layui CSS 类反向定位组件
- opengui_get_regression(keyword?) —— 回归约束（关键证词 C001..C015）
- opengui_get_changelog(UI-###) —— CHANGELOG 条目原文

### 资源 / 提示词
- resources：`opengui://catalog`、`opengui://component/<name>`、`opengui://template/<id>`、`opengui://changelog/<UI-###>`
- prompts：`opengui-build-app(app_name, install_dir?)` —— 生成完整应用工程任务
"""


def opengui_list_components(store, category=None):
    comps = store.catalog.get("components", [])
    if category:
        comps = [c for c in comps if c.get("category") == category]
    comps = sorted(comps, key=lambda c: (c.get("category", ""), c["name"].lower()))
    if not comps:
        cats = "、".join(store.catalog.get("categories", {}).keys())
        return "分类「{}」无组件。可用分类：{}".format(category, cats)
    out = ["## 组件清单{}".format("（分类：" + category + "）" if category else "")]
    cur = None
    for c in comps:
        cat = c.get("category", "")
        if cat != cur:
            cur = cat
            out += ["", "### " + store.catalog.get("categories", {}).get(cat, cat), ""]
        lay = "".join("`" + x + "` " for x in c.get("layui_classes", []))
        out.append("- **{}** —— {}（{} {}/{}）{}".format(
            c["name"], c.get("summary", ""), c.get("header", ""),
            c.get("category", ""), lay.strip()))
    out += ["", "分类总览："] + [
        "- {}：{}".format(k, v) for k, v in store.catalog.get("categories", {}).items()]
    return "\n".join(out)


def opengui_get_component(store, name):
    comps = store.catalog.get("components", [])
    comp = next((c for c in comps if c["name"].lower() == name.lower()), None)
    if not comp:
        hint = [c["name"] for c in comps if name.lower() in c["name"].lower()][:8]
        if hint:
            return "未找到组件「{}」，相近：{}".format(name, "、".join(hint))
        return "未找到组件「{}」。可用：opengui_list_components 或 opengui_find_class / opengui_search。".format(name)
    return _component_card(store, comp)


def opengui_get_template(store, tid):
    tpl = store.templates.get("templates", {}).get(tid)
    if not tpl:
        avail = "、".join(store.templates.get("templates", {}).keys())
        return "未找到模板「{}」。可用：{}".format(tid, avail)
    uses = "、".join("`" + u + "`" for u in tpl.get("uses", []))
    return "## 模板：{}（{}）\n\n使用组件：{}\n\n```cpp\n{}\n```".format(
        tpl.get("title", tid), tpl.get("category", ""), uses, tpl.get("code", ""))


def opengui_get_scaffold(store, sid):
    sc = store.templates.get("scaffolds", {}).get(sid)
    if not sc:
        avail = "、".join(store.templates.get("scaffolds", {}).keys())
        return "未找到脚手架「{}」。可用：{}".format(sid, avail)
    out = ["## 脚手架：{}（{}）".format(sid, sc.get("title", ""))]
    for path, content in (sc.get("files") or {}).items():
        out += ["", "### `{}`".format(path), "", "```", content, "```"]
    return "\n".join(out)


def opengui_search(store, query):
    q = query.lower() if query else ""
    hits = []
    if not q:
        return "opengui_search 需要关键词（组件名/方法名/layui 类/keyword）。"

    for c in store.catalog.get("components", []):
        blobs = [c["name"].lower(), (c.get("summary") or "").lower()] + \
                [x.lower() for x in c.get("layui_classes", [])]
        if any(q in b for b in blobs) or (q in (c.get("header") or "").lower()):
            hits.append("组件 `{}`（{}）：{}".format(
                c["name"], c.get("header", ""), c.get("summary", "")))

    for comp in store.components_by_name().values():
        for m in comp.get("methods") or []:
            if q in m["name"].lower() or q in (m.get("doc") or "").lower():
                hits.append("方法 `{}::{}({})`：{}".format(
                    comp["name"], m["name"], m.get("sig", ""),
                    re.sub(r"\s+", " ", m.get("doc") or "")))

    for tpl_id, t in store.templates.get("templates", {}).items():
        if q in tpl_id.lower() or q in (t.get("title") or "").lower():
            hits.append("模板 `{}`：{}".format(tpl_id, t.get("title", "")))
    for sc_id, sc in store.templates.get("scaffolds", {}).items():
        if q in sc_id.lower():
            hits.append("脚手架 `{}`：{}".format(sc_id, sc.get("title", "")))

    for con in store.constraints.get("constraints", []):
        if q in con.get("id", "").lower() or q in (con.get("rule") or "").lower() \
                or q in " ".join(con.get("ui_ids", [])).lower() or q in (con.get("danger") or "").lower():
            hits.append("约束 `{}`（{}）：{}".format(
                con.get("id", ""), ",".join(con.get("ui_ids", [])),
                (con.get("rule") or "")[:120]))

    if not hits:
        return "未命中「{}」。试试组件名、方法名、layui 类（如 layui-btn-lg）或约束关键词（line_height）。".format(query)
    return "## 检索「{}」（{} 条命中）\n\n{}".format(query, len(hits), "\n".join(hits[:40]))


def opengui_find_class(store, cls):
    cls = (cls or "").strip().lower()
    if not cls.startswith("layui-"):
        return "参数应为 CSS 类，如 layui-btn-primary / layui-table / layui-col-md4。"
    hits = []
    for c in store.catalog.get("components", []):
        for lc in c.get("layui_classes", []):
            if cls in lc.lower():
                hits.append("`{}`（{}，头文件 `{}`）".format(
                    lc, c["name"], c.get("header", "")))
    if not hits:
        return "未找到匹配「{}」的组件。可用 opengui_search 查组件能力。".format(cls)
    return "## 类「{}」命中的组件\n\n{}".format(cls, "\n".join(hits))


def opengui_get_regression(store, keyword=None):
    cons = store.constraints.get("constraints", [])
    kw = (keyword or "").strip().lower()
    matched = list(cons)
    if kw:
        matched = [c for c in cons
                   if kw in c.get("id", "").lower() or kw in (c.get("rule") or "").lower()
                   or kw in " ".join(c.get("ui_ids", [])).lower()
                   or kw in (c.get("danger") or "").lower()]
    if not matched:
        return "未找到约束「{}」。可用关键词：line_height / advance / destroy / set_chrome / mark_dirty / clearance / font / scissor / Release / text_secondary。".format(kw)
    out = ["## 回归约束{}（{} 条）".format("「" + kw + "」" if kw else "", len(matched))]
    for c in matched:
        out += ["", "### {}（{}）".format(c.get("id", ""), "、".join(c.get("ui_ids", []))),
                c.get("rule", ""),
                "风险：{}".format(c.get("danger", ""))]
    return "\n".join(out)


def opengui_get_changelog(store, ui_id):
    ui_id = (ui_id or "").strip().upper()
    if not re.match(r"^UI-\d+$", ui_id):
        return "参数应为条目号，如 UI-053 / UI-161。"
    body = store.changelog_entry(ui_id)
    if not body:
        # CHANGELOG.md 缺失或条目不存在
        head = next((h for h in store.catalog.get("headers", [])), None)
        if store.changelog == "":
            return "CHANGELOG.md 未随服务部署（数据目录缺文件），无法提供条目原文。约束条目见 opengui_get_regression。"
        return "未找到条目 {}".format(ui_id)
    return body.strip()


# --------------------------------------------------------------------------
# 资源 / 提示词
# --------------------------------------------------------------------------

RESOURCES = [
    {"uri": "opengui://catalog", "name": "opengui 组件目录",
     "description": "全量组件/分类/头文件索引（JSON）", "mimeType": "application/json"},
    {"uri": "opengui://component/{name}", "name": "组件详情",
     "description": "单组件（方法签名+示例）", "mimeType": "text/markdown"},
    {"uri": "opengui://template/{id}", "name": "用法模板",
     "description": "可改装 C++ 用法模板", "mimeType": "text/markdown"},
    {"uri": "opengui://changelog/{UI-###}", "name": "CHANGELOG 条目",
     "description": "历史条目原文", "mimeType": "text/markdown"},
]

PROMPTS = [
    {"name": "opengui-build-app",
     "description": "生成一个 opengl_ui 应用工程（查证组件 → 建脚手架 → 组装骨架）",
     "arguments": [
         {"name": "app_name", "description": "应用名", "required": True},
         {"name": "install_dir", "description": "opengl_ui 的 install/ 目录（CMake find_package 用）", "required": False},
     ]},
]


TOOLS = [
    {"name": "opengui_help", "description": "opengui 使用指南：推荐流程、铁律、工具总表",
     "inputSchema": {"type": "object", "properties": {"_": {"type": "string"}}}},
    {"name": "opengui_list_components",
     "description": "列出组件（可过滤分类）。category ∈ core/framework/layout/basic/form/navigation/data/container/media",
     "inputSchema": {"type": "object", "properties": {
         "category": {"type": "string", "description": "可选，分类过滤"}}}},
    {"name": "opengui_get_component",
     "description": "组件详情：真实方法签名 + layui 类 + 用法示例",
     "inputSchema": {"type": "object", "properties": {
         "name": {"type": "string", "description": "组件名，如 Button"}}, "required": ["name"]}},
    {"name": "opengui_get_template",
     "description": "取可改装用法模板，如 window-skeleton / hand-roll-loop / table-page / form-page / video-player / image-viewer / layer-popup / dropdown-menu / scroll-list / grid-page / multi-window",
     "inputSchema": {"type": "object", "properties": {
         "id": {"type": "string", "description": "模板 id"}}, "required": ["id"]}},
    {"name": "opengui_get_scaffold",
     "description": "取工程脚手架文件（app / app-media），含 CMakeLists.txt + main.cpp",
     "inputSchema": {"type": "object", "properties": {
         "id": {"type": "string", "description": "scaffold id"}}, "required": ["id"]}},
    {"name": "opengui_search",
     "description": "关键词检索：组件名/方法名/layui 类/模板/约束",
     "inputSchema": {"type": "object", "properties": {
         "query": {"type": "string"}}, "required": ["query"]}},
    {"name": "opengui_find_class",
     "description": "按 layui CSS 类反向定位组件（如 layui-btn-primary）",
     "inputSchema": {"type": "object", "properties": {
         "class": {"type": "string", "description": "layui-* 类名"}}, "required": ["class"]}},
    {"name": "opengui_get_regression",
     "description": "回归约束列表（带 CHANGELOG UI-### 溯源）；关键词过滤如 line_height、advance、scissor、media" ,
     "inputSchema": {"type": "object", "properties": {
         "keyword": {"type": "string", "description": "可选，过滤关键词"}}}},
    {"name": "opengui_get_changelog",
     "description": "取 CHANGELOG 条目原文（如 UI-053）",
     "inputSchema": {"type": "object", "properties": {
         "id": {"type": "string", "description": "UI-### 条目号"}}, "required": ["id"]}},
]


# --------------------------------------------------------------------------
# MCP handler
# --------------------------------------------------------------------------

class McpServer:
    def __init__(self, data_dir):
        self.store = DataStore(data_dir)

    # ---- 请求分发 ----
    def handle(self, msg):
        if not isinstance(msg, dict):
            return _err(None, INVALID_REQUEST, "request must be a JSON object")
        method = msg.get("method")
        rid = msg.get("id")
        params = msg.get("params") or {}
        if method is None:
            return _err(rid, INVALID_REQUEST, "missing method")
        if method == "initialize":
            proto = params.get("protocolVersion") or PROTOCOL_VERSION
            return _ok(rid, {
                "protocolVersion": PROTOCOL_VERSION if proto != "" else proto,
                "serverInfo": {"name": SERVER_NAME, "version": SERVER_VERSION},
                "capabilities": {
                    "tools": {"listChanged": False},
                    "resources": {"subscribe": False, "listChanged": False},
                    "prompts": {"listChanged": False},
                }})
        if method == "notifications/initialized":
            return None  # notification
        if method == "resources/list":
            return _ok(rid, {"resources": RESOURCES})
        if method == "resources/read":
            uri = params.get("uri", "")
            return self._read_resource(rid, uri)
        if method == "prompts/list":
            return _ok(rid, {"prompts": PROMPTS})
        if method == "prompts/get":
            return self._get_prompt(rid, params)
        if method == "tools/list":
            return _ok(rid, {"tools": TOOLS})
        if method == "tools/call":
            return self._call_tool(rid, params)
        if method == "ping":
            return _ok(rid, {})
        return _err(rid, METHOD_NOT_FOUND, "unknown method: " + method)

    # ---- 资源 ----
    def _read_resource(self, rid, uri):
        if uri == "opengui://catalog":
            text = json.dumps(self.store.catalog, ensure_ascii=False, indent=1)
            return _ok(rid, {"contents": [{"uri": uri, "mimeType": "application/json", "text": text}]})
        m = re.match(r"^opengui://component/(.+)$", uri)
        if m:
            text = opengui_get_component(self.store, m.group(1))
            return _ok(rid, {"contents": [{"uri": uri, "mimeType": "text/markdown", "text": text}]})
        m = re.match(r"^opengui://template/(.+)$", uri)
        if m:
            text = opengui_get_template(self.store, m.group(1))
            return _ok(rid, {"contents": [{"uri": uri, "mimeType": "text/markdown", "text": text}]})
        m = re.match(r"^opengui://changelog/(UI-\d+)$", uri, re.I)
        if m:
            text = opengui_get_changelog(self.store, m.group(1).upper())
            return _ok(rid, {"contents": [{"uri": uri, "mimeType": "text/markdown", "text": text}]})
        return _err(rid, METHOD_NOT_FOUND, "unknown resource: " + uri)

    # ---- prompts ----
    def _get_prompt(self, rid, params):
        name = params.get("name", "")
        args = params.get("arguments") or {}
        if name != "opengui-build-app":
            return _err(rid, INVALID_PARAMS, "unknown prompt: " + name)
        app = args.get("app_name", "<APP>")
        inst = args.get("install_dir", "<OPENGUI_INSTALL_DIR>")
        msgs = [
            {"role": "system",
             "content": {"type": "text",
                         "text": "你是 opengl_ui 应用生成助手。先查证组件与约束，再按脚手架生成工程，禁止凭空猜 API。"}},
            {"role": "user",
             "content": {"type": "text", "text": (
                 "为应用「{app}」：\n"
                 "1) opengui_get_scaffold(\"app\")（视频/图像→app-media）取工程文件；\n"
                 "2) opengui_get_template(\"window-skeleton\") 取推荐入口骨架；\n"
                 "3) 按页面需要用 opengui_get_component / opengui_get_template 补齐表格/表单/弹层等；\n"
                 "4) 落地前 opengui_get_regression() 逐一核对约束（文本居中/释放/标脏/media 等）。\n"
                 "app_name={app}，install_dir={inst}。"
             ).format(app=app, inst=inst)}},
        ]
        return _ok(rid, {"description": "opengl_ui 应用工程生成", "messages": msgs})

    # ---- tools ----
    def _call_tool(self, rid, params):
        name = params.get("name", "")
        args = params.get("arguments") or {}
        proto = {"_": ""}
        try:
            if name == "opengui_help":
                text = opengui_help()
            elif name == "opengui_list_components":
                text = opengui_list_components(self.store, (args or proto).get("category"))
            elif name == "opengui_get_component":
                text = opengui_get_component(self.store, (args or proto).get("name") or "")
            elif name == "opengui_get_template":
                text = opengui_get_template(self.store, (args or proto).get("id") or "")
            elif name == "opengui_get_scaffold":
                text = opengui_get_scaffold(self.store, (args or proto).get("id") or "")
            elif name == "opengui_search":
                text = opengui_search(self.store, (args or proto).get("query") or "")
            elif name == "opengui_find_class":
                text = opengui_find_class(self.store, (args or proto).get("class") or "")
            elif name == "opengui_get_regression":
                text = opengui_get_regression(self.store, (args or proto).get("keyword"))
            elif name == "opengui_get_changelog":
                text = opengui_get_changelog(self.store, (args or proto).get("id") or "")
            else:
                return _err(rid, INVALID_PARAMS, "unknown tool: " + name)
        except TypeError as e:
            return _err(rid, INVALID_PARAMS, "bad arguments for {}: {}".format(name, e))
        return _ok(rid, {"content": [{"type": "text", "text": text}]})

    # ---- 主循环 ----
    def run(self):
        while True:
            try:
                frame = read_frame()
            except (EOFError, KeyboardInterrupt):
                break
            if frame is None:
                break
            if not frame.strip():
                continue
            try:
                msg = json.loads(frame)
            except ValueError:
                write_json(_err(None, PARSE_ERROR, "invalid json"))
                continue
            try:
                resp = self.handle(msg)
            except Exception as e:  # 服务侧兜底防崩
                _log("internal error:", e)
                resp = _err(msg.get("id"), INTERNAL_ERROR, str(e))
            if resp is not None:
                write_json(resp)

    # ---- 自检 ----
    def selfcheck(self):
        fails = []
        def check(cond, label):
            print(("[PASS] " if cond else "[FAIL] ") + label)
            if not cond:
                fails.append(label)

        s = self.store
        comps = s.catalog.get("components", [])
        check(len(comps) > 50, "catalog 组件数 {}（要求 >50）".format(len(comps)))
        btn = next((c for c in comps if c["name"] == "Button"), None)
        check(btn is not None and any("layui-btn" in x for x in btn.get("layui_classes", [])),
              "get_component(Button) 含 layui-btn 语法类")
        check("window-skeleton" in s.templates.get("templates", {}),
              "template window-skeleton 存在")
        regs = opengui_get_regression(s, "line_height")
        check("UI-053" in regs, "regression(line_height) 命中 UI-053")
        search = opengui_search(s, "layui-btn-lg")
        check("Button" in search, "search(layui-btn-lg) 命中 Button")
        window_tpl = opengui_get_template(s, "window-skeleton")
        check("WindowContainer" in window_tpl, "template 含 WindowContainer")
        scaffold = opengui_get_scaffold(s, "app")
        check("CMakeLists.txt" in scaffold, "scaffold app 含 CMakeLists.txt")
        ch = opengui_get_changelog(s, "UI-053")
        check("UI-053" in ch or s.changelog == "", "changelog UI-053 可取")
        check(self.handle({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                           "params": {"protocolVersion": PROTOCOL_VERSION}}) is not None,
              "initialize 响应正常")
        print("\n{} fail(s)".format(len(fails)))
        return 0 if not fails else 1


# --------------------------------------------------------------------------
# JSON-RPC / 帧协议
# --------------------------------------------------------------------------

def _ok(rid, result):
    return {"jsonrpc": "2.0", "id": rid, "result": result}


def _err(rid, code, message):
    return {"jsonrpc": "2.0", "id": rid, "error": {"code": code, "message": message}}


def write_json(obj):
    try:
        sys.stdout.buffer.write(json.dumps(obj, ensure_ascii=False).encode("utf-8") + b"\n")
        sys.stdout.buffer.flush()
    except (BrokenPipeError, OSError):
        raise EOFError


def _readline():
    """从 stdin.buffer 读一行（去 \r\n），EOF 返回 None。"""
    out = bytearray()
    while True:
        b = sys.stdin.buffer.read(1)
        if not b:
            return None if not out else bytes(out)
        if b == b"\n":
            if out and out[-1] == 0x0d:
                del out[-1]
            return bytes(out)
        out += b


def _read_exact(n):
    data = b""
    while len(data) < n:
        chunk = sys.stdin.buffer.read(n - len(data))
        if not chunk:
            raise EOFError
        data += chunk
    return data


def read_frame():
    """读取一帧：兼容 newline-delimited JSON 与 Content-Length 头 + 空行。EOF 返回 None。"""
    while True:
        line = _readline()
        if line is None:
            return None
        if not line:
            continue  # 帧间空行
        if line.startswith(b"Content-Length:"):
            n = int(line.split(b":", 1)[1].strip())
            while True:
                h = _readline()
                if h is None:
                    return None
                if not h:
                    break
            return _read_exact(n)
        return line


def _default_data_dir():
    return os.path.dirname(os.path.abspath(__file__))


def main():
    args = sys.argv[1:]
    data_dir = _default_data_dir()
    selfcheck = False
    i = 0
    while i < len(args):
        if args[i] == "--data":
            i += 1
            data_dir = args[i]
        elif args[i] == "--selfcheck":
            selfcheck = True
        i += 1
    if not os.path.isdir(data_dir):
        _log("data dir not found:", data_dir)
        return 2
    mcp = McpServer(data_dir)
    if selfcheck:
        return mcp.selfcheck()
    try:
        mcp.run()
    except EOFError:
        return 0
    return 0


if __name__ == "__main__":
    sys.exit(main())