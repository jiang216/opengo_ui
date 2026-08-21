# 公共构建环境（所有 build-*.ps1 共享）。用法：. "$PSScriptRoot\env.ps1"
$ErrorActionPreference = "Stop"
# Windows 模式下裸 token 里的 $var 不会展开，改为 Standard 模式统一展开
$PSNativeCommandArgumentPassing = "Standard"
$ROOT = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$SRC = Join-Path $ROOT "3rdparty\src"
$INSTALL = Join-Path $ROOT "3rdparty\install"
$BUILD = Join-Path $ROOT "3rdparty\build"
$CMAKE = "C:\Program Files\JetBrains\CLion 2026.1.3\bin\cmake\win\x64\bin\cmake.exe"
$VS = "C:\Program Files\Microsoft Visual Studio\18\Enterprise"
$VCVARSALL = Join-Path $VS "VC\Auxiliary\Build\vcvarsall.bat"
$MSYS2 = "C:\msys64"
$GENERATOR = "Visual Studio 18 2026"

function Get-Prefix($arch) { return (Join-Path $INSTALL $arch) }

function Win2Msys([string]$p) {
    $p = $p -replace "\\", "/"
    if ($p -match "^([A-Za-z]):/") { return "/" + $Matches[1].ToLower() + $p.Substring(2) }
    return $p
}
