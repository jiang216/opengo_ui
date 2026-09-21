# publish.ps1 —— opengl_ui 发布包装：打包 zip 并（可选）注入 MCP 知识服务。
# 委托 scripts/publish/register-mcp.ps1 做 MCP 注入；install 目录默认取
# <仓库根>/install/combo 相邻的 install/（可用 -Install 覆盖）。
param(
    [string]$Install = "",
    [string]$Python  = "python",
    [ValidateSet("register", "unregister", "status")][string]$MCPMode = "status",
    [string[]]$Targets = @("opencode", "claude", "mcpjson"),
    [switch]$WriteNotes,
    [switch]$SkipCheck
)

$ErrorActionPreference = "Stop"

# 默认 install 目录：本仓库根下的 install/
if (-not $Install) {
    $repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
    $Install = Join-Path $repo "install"
}
if (-not (Test-Path -LiteralPath $Install)) {
    Write-Warning "install 目录不存在：$Install（可用 -Install 指定）"
    exit 1
}

& "$PSScriptRoot/register-mcp.ps1" `
    -Install  $Install `
    -Python   $Python `
    -Mode     $MCPMode `
    -Targets  $Targets `
    -WriteNotes:$WriteNotes `
    -SkipCheck:$SkipCheck

exit $LASTEXITCODE