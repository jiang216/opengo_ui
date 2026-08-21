param([Parameter(Mandatory)][ValidateSet("x64","x86")]$Arch)
. "$PSScriptRoot\env.ps1"
$prefix = Get-Prefix $Arch
$src = Join-Path $SRC "opencv-4.5.2"
$buildDir = Join-Path $BUILD "opencv-$Arch"
$platform = if ($Arch -eq "x64") { "x64" } else { "Win32" }

Write-Host "[opencv] configure $Arch"
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
& $CMAKE -S $src -B $buildDir -G $GENERATOR -A $platform `
  "-DCMAKE_INSTALL_PREFIX=$prefix" `
  "-DCMAKE_BUILD_TYPE=Release" `
  "-DBUILD_LIST=world,core,imgproc,highgui,imgcodecs" `
  -DBUILD_opencv_world=ON `
  -DBUILD_SHARED_LIBS=ON `
  -DBUILD_opencv_apps=OFF `
  -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_EXAMPLES=OFF `
  -DBUILD_opencv_python2=OFF -DBUILD_opencv_python3=OFF -DBUILD_JAVA=OFF `
  -DBUILD_opencv_java_bindings_generator=OFF -DBUILD_opencv_python_bindings_generator=OFF `
  -DWITH_FFMPEG=OFF -DWITH_IPP=OFF -DWITH_OPENCL=OFF `
  -DOPENCV_INSTALL_BINARIES_PREFIX="" 2>&1 | Select-Object -Last 15
if ($LASTEXITCODE -ne 0) { throw "opencv configure failed ($Arch) — 见 $buildDir\CMakeCache.txt" }

Write-Host "[opencv] build+install $Arch"
& $CMAKE --build $buildDir --config Release --target install "--" "-m:10" 2>&1 | Select-Object -Last 20
if ($LASTEXITCODE -ne 0) { throw "opencv build failed ($Arch)" }
Write-Host "[opencv] OK $Arch"
