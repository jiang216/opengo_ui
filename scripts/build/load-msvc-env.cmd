@echo off
rem Load MSVC env and dump as var list (stdout). Usage: load-msvc-env.cmd <arch: x64|x86>
call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" %1 >nul 2>&1
if errorlevel 1 exit /b 1
set
exit /b 0