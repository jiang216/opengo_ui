# package.ps1 —— opengl_ui 发布包（zip）生成。
# 把 install/ 目录（自包含分发树：bin/include/lib/share）打包为
# dist/opengl_ui-<arch>-all-release.zip（dist/ 与产物已 gitignore），供下载方「解压即用」。
#
# 默认流程：校验 install/ 完整性 → 直接打包。可选：
#   -Rebuild       先用 -BuildDir（默认 build/install-all）cmake 重建并 install，再打包
#   -InjectMCP     打包前注入 opengui MCP 知识服务（委托 publish.ps1，写 install/share/opengl_ui/mcp）
#   -MCPMode       注入模式 register|status|none（默认 none；-InjectMCP 等价 register）
#   -Install/-Out/-BuildDir/-Cmake 可覆盖默认路径（-Out 默认 dist/opengl_ui-<arch>-all-release.zip）
#
# 用法：
#   .\package.ps1                       # 打包现有 install/ → dist/
#   .\package.ps1 -Rebuild              # 重建 install/ 后再打包
#   .\package.ps1 -Rebuild -InjectMCP   # 重建 + 注入 MCP + 打包
param(
    [string]$Install = "",
    [string]$Out = "",
    [ValidateSet("x64", "x86")][string]$Arch = "x64",
    [string]$BuildDir = "",
    [switch]$Rebuild,
    [switch]$InjectMCP,
    [ValidateSet("none", "status", "register")][string]$MCPMode = "none",
    [string]$Cmake = "",
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$PSNativeCommandArgumentPassing = "Standard"

$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
if (-not $Install)  { $Install  = Join-Path $repo "install" }
if (-not $Out)      { $Out      = Join-Path $repo "dist\opengl_ui-$Arch-all-release.zip" }
if (-not $BuildDir) { $BuildDir = Join-Path $repo "build\install-all" }

function Write-Step($m) { Write-Host "==> $m" -ForegroundColor Cyan }
function Write-Ok($m)   { Write-Host "  [OK] $m" -ForegroundColor Green }
function Write-Warn2($m){ Write-Host "  [WARN] $m" -ForegroundColor Yellow }

function Resolve-Cmake([string]$explicit) {
    if ($explicit) { return $explicit }
    $envCmake = Join-Path $repo "scripts\build\env.ps1"
    if (Test-Path -LiteralPath $envCmake) {
        # env.ps1 里的 $CMAKE 是团队 CLion 路径，优先采用（存在时）
        $c = & {
            . $envCmake
            $CMAKE
        }
        if ($c -and (Test-Path -LiteralPath $c)) { return $c }
    }
    $candidates = @(
        "C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe",
        "C:\Program Files\Microsoft Visual Studio\17\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    )
    foreach ($c in $candidates) { if (Test-Path -LiteralPath $c) { return $c } }
    $cmd = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    throw "未找到 cmake（可用 -Cmake 指定）"
}

# ---------------------------------------------------------------------------
# 1) 可选：重建 + install 刷新 install/
# ---------------------------------------------------------------------------
if ($Rebuild) {
    if (-not (Test-Path -LiteralPath (Join-Path $BuildDir "CMakeCache.txt"))) {
        throw "构建树不存在：$BuildDir（请先按 docs/BUILD.md 用 install-all 预设 configure，或改用已存在的 -BuildDir）"
    }
    $cmakeExe = Resolve-Cmake $Cmake
    Write-Step "重建 build/install-all（Release）"
    & $cmakeExe --build $BuildDir --config Release
    if ($LASTEXITCODE -ne 0) { throw "构建失败（exit $LASTEXITCODE）" }
    Write-Step "cmake --install → $Install"
    & $cmakeExe --install $BuildDir --config Release
    if ($LASTEXITCODE -ne 0) { throw "安装失败（exit $LASTEXITCODE）" }
    Write-Ok "install/ 已刷新"
}

if (-not (Test-Path -LiteralPath $Install)) {
    throw "install 目录不存在：$Install（先构建并 install，或加 -Rebuild）"
}

# ---------------------------------------------------------------------------
# 2) 可选：注入 MCP 知识服务
# ---------------------------------------------------------------------------
$effMCP = if ($InjectMCP) { "register" } else { $MCPMode }
if ($effMCP -ne "none") {
    Write-Step "MCP 知识服务注入（$effMCP）"
    & (Join-Path $PSScriptRoot "publish.ps1") -Install $Install -MCPMode $effMCP -Targets opencode,claude,mcpjson
    if ($LASTEXITCODE -ne 0) { throw "MCP 注入失败（exit $LASTEXITCODE）" }
}

# ---------------------------------------------------------------------------
# 3) 完整性校验：install 必须是自包含分发树
# ---------------------------------------------------------------------------
Write-Step "校验 install/ 结构"
$problems = @()
foreach ($d in @("bin", "include", "lib", "share")) {
    if (-not (Test-Path -LiteralPath (Join-Path $Install $d))) { $problems += "缺少目录 $d/" }
}
if (-not (Get-ChildItem (Join-Path $Install "lib") -Filter *.lib -ErrorAction SilentlyContinue)) {
    $problems += "lib/ 无 .lib 静态/导入库"
}
if (-not (Get-ChildItem (Join-Path $Install "bin") -Filter glfw3.dll -ErrorAction SilentlyContinue)) {
    $problems += "bin/ 缺少 glfw3.dll"
}
if (-not (Test-Path -LiteralPath (Join-Path $Install "include\ui\all.h"))) {
    $problems += "include/ui/all.h 缺失（聚合头）"
}
if (-not (Test-Path -LiteralPath (Join-Path $Install "lib\cmake\opengl_ui\opengl_uiConfig.cmake"))) {
    $problems += "lib/cmake/opengl_ui/opengl_uiConfig.cmake 缺失"
}
if ($problems.Count -gt 0) {
    foreach ($p in $problems) { Write-Warn2 $p }
    throw "install/ 不完整，打包中止（请 -Rebuild 或修正后重试）"
}
Write-Ok "结构完整（bin/ include/ lib/ share/）"

# ---------------------------------------------------------------------------
# 4) 打包
# ---------------------------------------------------------------------------
if ((Test-Path -LiteralPath $Out) -and -not $Force) {
    Write-Warn2 "目标已存在，将覆盖：$Out"
    Remove-Item -LiteralPath $Out -Force
}
$outDir = Split-Path -Parent $Out
if ($outDir -and -not (Test-Path -LiteralPath $outDir)) {
    New-Item -ItemType Directory -Path $outDir -Force | Out-Null
}
Write-Step "打包 → $Out"
Compress-Archive -Path (Join-Path $Install "*") -DestinationPath $Out -CompressionLevel Optimal -Force

Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = [System.IO.Compression.ZipFile]::OpenRead($Out)
$count = $zip.Entries.Count
$zip.Dispose()
$size = (Get-Item -LiteralPath $Out).Length
Write-Ok ("发布包已生成：{0}（{1:N2} MB，{2} 条目）" -f $Out, ($size / 1MB), $count)
