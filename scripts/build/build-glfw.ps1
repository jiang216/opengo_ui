param([Parameter(Mandatory)][ValidateSet("x64","x86")]$Arch)
. "$PSScriptRoot\env.ps1"
$vsArch = if ($Arch -eq "x64") { "x64" } else { "Win32" }
$prefix = Get-Prefix $Arch
Write-Host "[glfw] configure $Arch"
& $CMAKE -S (Join-Path $SRC "glfw") -B (Join-Path $BUILD "glfw-$Arch") -G $GENERATOR -A $vsArch `
  -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF `
  -DBUILD_SHARED_LIBS=ON "-DCMAKE_INSTALL_PREFIX=$prefix"
if ($LASTEXITCODE -ne 0) { throw "glfw configure failed ($Arch)" }
Write-Host "[glfw] build+install $Arch"
& $CMAKE --build (Join-Path $BUILD "glfw-$Arch") --config Release --target install
if ($LASTEXITCODE -ne 0) { throw "glfw build failed ($Arch)" }
Write-Host "[glfw] OK $Arch"
