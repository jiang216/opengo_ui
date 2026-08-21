# 一键构建全部依赖（x64 + x86）：GLFW → FFmpeg → OpenCV
param([ValidateSet("x64","x86","all")]$Arch = "all")
. "$PSScriptRoot\env.ps1"
$arches = if ($Arch -eq "all") { @("x64","x86") } else { @($Arch) }
foreach ($a in $arches) {
    & "$PSScriptRoot\build-glfw.ps1" -Arch $a
    & "$PSScriptRoot\build-ffmpeg.ps1" -Arch $a
    & "$PSScriptRoot\build-opencv.ps1" -Arch $a
}
Write-Host "[build-all] 全部完成"
