# opengui MCP 注入脚本 —— 把 opengl_ui 知识服务注册进 opencode / Claude Code / .mcp.json
# 零依赖：不建 venv / 不 pip install；写入前自检 `python server.py --selfcheck`。
# 用法：
#   .\register-mcp.ps1 -Mode status                          # 查看（默认，安全）
#   .\register-mcp.ps1 -Mode register -Install E:\opengl_ui\install
#   .\register-mcp.ps1 -Mode unregister
# 开关：-Targets opencode,claude,mcpjson | -Python python | -ProjectDir <dir>
#       -ConfigRoot <dir>（测试用，覆盖用户主目录）| -WriteNotes
[CmdletBinding()]
param(
    [ValidateSet("register", "unregister", "status")][string]$Mode = "status",
    [string[]]$Targets = @("opencode", "claude", "mcpjson"),
    [string]$Install = "",
    [string]$Python = "python",
    [string]$ProjectDir = (Get-Location).Path,
    [string]$ConfigRoot = $env:USERPROFILE,
    [switch]$WriteNotes,
    [switch]$SkipCheck
)

$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# 工具函数
# ---------------------------------------------------------------------------
function Write-Ok   { Write-Host "  [OK] "   -NoNewline -ForegroundColor Green;  Write-Host $args }
function Write-Info { Write-Host "  [..] "   -NoNewline -ForegroundColor Cyan;   Write-Host $args }
function Write-Warn { Write-Host "  [WARN] " -NoNewline -ForegroundColor Yellow; Write-Host $args }

function Read-JsonObject([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path)) { return [ordered]@{} }
    $raw = [System.IO.File]::ReadAllText($Path)
    if (-not $raw.Trim()) { return [ordered]@{} }
    try { return (ConvertFrom-Json -InputObject $raw -AsHashtable) } catch { return [ordered]@{} }
}

function Merge-Hashtable($Base, $Override) {
    if ($null -eq $Override) { return $Base }
    if ($Override -isnot [System.Collections.IDictionary]) { return $Override }
    if ($null -eq $Base -or $Base -isnot [System.Collections.IDictionary]) {
        $h = [ordered]@{}
        foreach ($k in $Override.Keys) { $h[$k] = $Override[$k] }
        return $h
    }
    foreach ($k in @($Override.Keys)) {
        if ($Base.Contains($k)) {
            if ($Base[$k] -is [System.Collections.IDictionary] -and $Override[$k] -is [System.Collections.IDictionary]) {
                $Base[$k] = Merge-Hashtable $Base[$k] $Override[$k]
            } else { $Base[$k] = $Override[$k] }
        } else { $Base[$k] = $Override[$k] }
    }
    return $Base
}

function Get-Value([object]$Root, [string[]]$Steps) {
    $cur = $Root
    foreach ($s in $Steps) {
        if ($null -eq $cur -or $cur -isnot [System.Collections.IDictionary] -or -not $cur.Contains($s)) {
            return $null
        }
        $cur = $cur[$s]
    }
    if ($cur -is [System.Collections.IDictionary] -and $cur.Count -eq 0) { return $null }
    return $cur
}

function Remove-KeyPath([object]$Root, [string[]]$Steps) {
    $cur = $Root
    $chain = @()
    foreach ($s in $Steps) {
        if ($null -eq $cur -or $cur -isnot [System.Collections.IDictionary] -or -not $cur.Contains($s)) {
            return
        }
        $chain += , @($cur, $s)
        $cur = $cur[$s]
    }
    for ($i = $chain.Count - 1; $i -ge 0; $i--) {
        $parent = $chain[$i][0]; $key = $chain[$i][1]
        if (-not $parent.Contains($key)) { continue }
        $parent.Remove($key)
        if ($parent.Count -gt 0) { return }
    }
}

function Write-JsonFileAtomic([string]$Path, $Obj) {
    $json = ConvertTo-Json -InputObject $Obj -Depth 100
    $dir = Split-Path -Parent $Path
    if ($dir -and -not (Test-Path -LiteralPath $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }
    $exists = Test-Path -LiteralPath $Path
    if ($exists) {
        $cur = [System.IO.File]::ReadAllText($Path)
        if ($cur.Trim() -ne $json.Trim() -and -not (Test-Path -LiteralPath "$Path.bak")) {
            Copy-Item -LiteralPath $Path -Destination "$Path.bak" -Force
        }
    }
    $enc = New-Object System.Text.UTF8Encoding($false)   # 无 BOM
    [System.IO.File]::WriteAllText($Path, $json + [Environment]::NewLine, $enc)
    Write-Info "$(if ($exists) { 'updated' } else { 'created' }) $Path"
}

function Test-SelfCheck([string]$Exe, [string]$Server, [string]$DataDir) {
    if ($SkipCheck) { return $true }
    Write-Info "selfcheck: & $Exe $Server --selfcheck --data $DataDir"
    $env:PYTHONIOENCODING = "utf-8"
    & $Exe $Server --selfcheck --data $DataDir
    if ($LASTEXITCODE -ne 0) {
        Write-Warn "opengui 服务自检失败（exit=$LASTEXITCODE），中止写入。"
        return $false
    }
    return $true
}

function Resolve-InstallDir {
    if ($Install) {
        if (-not (Test-Path -LiteralPath $Install)) { throw "install 目录不存在: $Install" }
        return (Resolve-Path -LiteralPath $Install).Path
    }
    # 已安装布局：<inst>/share/opengl_ui/<本脚本>.ps1，且 mcp/server.py 存在
    $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    if ((Split-Path -Leaf $scriptDir) -eq "opengl_ui" -and
        (Test-Path -LiteralPath (Join-Path $scriptDir "mcp\server.py"))) {
        return (Split-Path -Parent (Split-Path -Parent $scriptDir))
    }
    return $null
}

# ---------------------------------------------------------------------------
# 三个目标的 注册 / 卸载 / 状态
# ---------------------------------------------------------------------------
function OpenCode-ConfigPath([string]$cfgRoot) { return Join-Path $cfgRoot ".config\opencode\opencode.json" }
function OpenCode-Steps  { return @("mcp", "opengui") }
function OpenCode-Entry([string]$exe, [string]$server) {
    return [ordered]@{ type = "local"; command = @($exe, $server); enabled = $true }
}

function Claude-ConfigPath([string]$cfgRoot) { return Join-Path $cfgRoot ".claude.json" }
function Claude-Steps([string]$projDir) {
    return @("projects", $projDir.Replace("\", "/"), "mcpServers", "opengui")
}
function Claude-Entry([string]$exe, [string]$server) {
    return [ordered]@{ type = "stdio"; command = $exe; args = @($server) }
}

function McpJson-ConfigPath([string]$projDir) { return Join-Path $projDir ".mcp.json" }
function McpJson-Steps { return @("mcpServers", "opengui") }
function McpJson-Entry([string]$exe, [string]$server) {
    return [ordered]@{ type = "stdio"; command = $exe; args = @($server) }
}

function Register-Config {
    param([string]$Target, [string]$CfgPath, [string[]]$Steps, [object]$Entry, [string]$Label)
    $obj = Read-JsonObject $CfgPath
    # 沿 Steps 的父级建链
    $cur = $obj
    for ($i = 0; $i -lt $Steps.Count - 1; $i++) {
        $k = $Steps[$i]
        if (-not $cur.Contains($k) -or $cur[$k] -isnot [System.Collections.IDictionary]) {
            $cur[$k] = [ordered]@{}
        }
        $cur = $cur[$k]
    }
    $cur[$Steps[-1]] = $Entry
    Write-JsonFileAtomic $CfgPath $obj
    Write-Ok "${Label}: opengui 已注册 → $CfgPath"
}

function Unregister-Config {
    param([string]$Target, [string]$CfgPath, [string[]]$Steps, [string]$Label)
    if (-not (Test-Path -LiteralPath $CfgPath)) { Write-Info "${Label}: 无配置，跳过"; return }
    $obj = Read-JsonObject $CfgPath
    Remove-KeyPath $obj $Steps
    Write-JsonFileAtomic $CfgPath $obj
    Write-Ok "${Label}: opengui 已移除（保留其它配置）→ $CfgPath"
}

function Status-Config {
    param([string]$Target, [string]$CfgPath, [string[]]$Steps, [string]$Label)
    if (-not (Test-Path -LiteralPath $CfgPath)) {
        Write-Info "${Label}: 未配置（无配置文件 $CfgPath）"; return
    }
    $obj = Read-JsonObject $CfgPath
    $leaf = Get-Value $obj $Steps
    if ($null -eq $leaf) { Write-Info "${Label}: 未配置（$CfgPath 中无 opengui）" }
    else {
        $cmd = ""
        if ($leaf -is [System.Collections.IDictionary] -and $leaf.Contains("command")) {
            $cmd = [string]($leaf["command"])
        }
        Write-Ok "${Label}: 已配置 → $CfgPath"
        Write-Info "      command: $cmd"
    }
}

# ---------------------------------------------------------------------------
# 主流程
# ---------------------------------------------------------------------------
foreach ($t in $Targets) {
    if ($t.ToLowerInvariant().Trim() -notin @("opencode", "claude", "mcpjson")) {
        throw "未知注入目标: $t（可选 opencode / claude / mcpjson）"
    }
}

$inst = Resolve-InstallDir
if (-not $inst) {
    throw "无法确定 opengl_ui install 目录：请传 -Install <install 目录>，或从已安装副本 <install>/share/opengl_ui/ 下运行本脚本。"
}
$mcpDir  = Join-Path $inst "share\opengl_ui\mcp"
$server  = Join-Path $mcpDir "server.py"
if (-not (Test-Path -LiteralPath $server)) {
    throw "未找到 MCP 服务: $server（install 目录是否缺 mcp 资产？）"
}

Write-Host ""
Write-Host "== opengui MCP ($Mode) =="
Write-Host "install  = $inst"
Write-Host "server   = $server"

if ($Mode -eq "status") {
    foreach ($t in $Targets) {
        switch ($t.ToLowerInvariant()) {
            "opencode" { Status-Config $t (OpenCode-ConfigPath $ConfigRoot) (OpenCode-Steps)     "opencode" }
            "claude"   { Status-Config $t (Claude-ConfigPath   $ConfigRoot) (Claude-Steps $ProjectDir) "claude" }
            "mcpjson"  { Status-Config $t (McpJson-ConfigPath  $ProjectDir) (McpJson-Steps)    ".mcp.json" }
        }
    }
    Write-Host ""
    Write-Warn "python = $Python（可用 -Python 覆盖）"
    Write-Host ""
    exit 0
}

if (-not (Test-SelfCheck $Python $server $mcpDir)) { exit 1 }

foreach ($t in $Targets) {
    switch ($t.ToLowerInvariant()) {
        "opencode" {
            if ($Mode -eq "register") {
                Register-Config   "opencode" (OpenCode-ConfigPath $ConfigRoot) (OpenCode-Steps) (OpenCode-Entry $Python $server) "opencode"
            } else {
                Unregister-Config "opencode" (OpenCode-ConfigPath $ConfigRoot) (OpenCode-Steps)   "opencode"
            }
        }
        "claude" {
            if ($Mode -eq "register") {
                Register-Config   "claude" (Claude-ConfigPath $ConfigRoot) (Claude-Steps $ProjectDir) (Claude-Entry $Python $server) "claude"
            } else {
                Unregister-Config "claude" (Claude-ConfigPath $ConfigRoot) (Claude-Steps $ProjectDir) "claude"
            }
        }
        "mcpjson" {
            if ($Mode -eq "register") {
                Register-Config   "mcpjson" (McpJson-ConfigPath $ProjectDir) (McpJson-Steps) (McpJson-Entry $Python $server) ".mcp.json"
            } else {
                Unregister-Config "mcpjson" (McpJson-ConfigPath $ProjectDir) (McpJson-Steps) ".mcp.json"
            }
        }
    }
}

if ($Mode -eq "register" -and $WriteNotes) {
    $note = Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) "OPENGUI_AGENT.md"
    $projAgents = Join-Path $ProjectDir "AGENTS.md"
    if (Test-Path -LiteralPath $note) {
        $marker = "opengui MCP 知识服务（OPENGUI_AGENT.md 指引）"
        $already = $false
        if (Test-Path -LiteralPath $projAgents) {
            $already = [bool](Select-String -LiteralPath $projAgents -Pattern ([regex]::Escape($marker)) -Quiet)
        }
        if (-not $already) {
            $block = @(
                "",
                "## $marker",
                "",
                "写 opengl_ui 代码前先调用 `opengui_*` MCP 工具查证组件/模板/回归约束，禁止凭记忆猜 API。",
                "完整指引见：$note",
                ""
            )
            Add-Content -LiteralPath $projAgents -Value $block -Encoding utf8
            Write-Ok "消费指引已追加到 $projAgents"
        } else {
            Write-Info "消费指引已存在，跳过写入 $projAgents"
        }
    }
}

Write-Host ""
Write-Host "== 完成。目标 = $($Targets -join ', ') =="
Write-Host ("消费方智能体指引：" + (Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) "OPENGUI_AGENT.md"))
Write-Host ""