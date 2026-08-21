param([Parameter(Mandatory)][ValidateSet("x64","x86")]$Arch)
. "$PSScriptRoot\env.ps1"
$ffArch = if ($Arch -eq "x64") { "x86_64" } else { "x86" }
$vcArch = if ($Arch -eq "x64") { "x64" } else { "x86" }
$prefix = Get-Prefix $Arch
$srcMsys = Win2Msys (Join-Path $SRC "ffmpeg-5.1.9")
$prefixMsys = Win2Msys $prefix
$buildMsys = Win2Msys (Join-Path $BUILD "ffmpeg-$Arch")
$flags = "--toolchain=msvc --arch=$ffArch --prefix=$prefixMsys " +
  "--enable-shared --disable-static --disable-programs --disable-doc --disable-debug " +
  "--disable-avdevice --disable-avfilter --disable-postproc --disable-network --enable-small --enable-x86asm"
Write-Host "[ffmpeg] load MSVC env ($vcArch)"
New-Item -ItemType Directory -Force -Path (Join-Path $BUILD "ffmpeg-$Arch") | Out-Null
$dumpCmd = Join-Path $env:TEMP "opengui-vcvars-$vcArch.cmd"
[System.IO.File]::WriteAllLines($dumpCmd, @(
  "@echo off",
  "call `"$VCVARSALL`" $vcArch >nul 2>&1",
  "if errorlevel 1 exit /b 1",
  "set",
  "exit /b 0"
))
$envDump = & $dumpCmd
Remove-Item -Force $dumpCmd -ErrorAction SilentlyContinue
if ($LASTEXITCODE -ne 0) { throw "vcvarsall 失败 ($vcArch)" }
foreach ($line in $envDump) {
    $idx = $line.IndexOf("=")
    if ($idx -gt 0) {
        [Environment]::SetEnvironmentVariable($line.Substring(0, $idx), $line.Substring($idx + 1), "Process")
    }
}
Write-Host "[ffmpeg] configure+build+install $Arch (MSVC toolchain)"
# 让 MSYS2 profile 继承 Windows PATH（含 MSVC），否则 cl.exe 不可见
[Environment]::SetEnvironmentVariable("MSYS2_PATH_TYPE", "inherit", "Process")
$bashCmd = "set -o pipefail; cd $buildMsys && $srcMsys/configure $flags 2>&1 | tee configure.log && make -j8 2>&1 | tee make.log && make install 2>&1 | tee install.log"
& "$MSYS2\usr\bin\bash.exe" --login -c $bashCmd
if ($LASTEXITCODE -ne 0) { throw "ffmpeg build failed ($Arch) — 见 3rdparty/build/ffmpeg-$Arch/*.log" }
Write-Host "[ffmpeg] OK $Arch"
