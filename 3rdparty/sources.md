# 第三方源码 / 环境清单（SOURCES MANIFEST）

本文件记录框架依赖的源码与构建环境来源，保证可复现。改动需同步更新。

## 源码（3rdparty/src/）

| 库 | 版本 | 来源 | 校验 |
|----|------|------|------|
| GLFW | 3.4 | `https://github.com/glfw/glfw` tag `3.4`（git clone --depth 1） | git tag 3.4 |
| FFmpeg | n5.1.9 | `https://ffmpeg.org/releases/ffmpeg-5.1.9.tar.bz2` | configure 存在（已解压至 `3rdparty/src/ffmpeg-5.1.9/`） |
| OpenCV | 4.5.2 | `https://downloads.sourceforge.net/project/opencvlibrary/4.5.2/OpenCV 4.5.2.zip` | SHA256 `87A402D9EE7D1F6E62B2C31F3F420F56B3039EDEE9C3D0055F6F0214B147E4D3` |
| GLAD | 2.0.8（glad2） | `python -m pip install glad2`（TUNA pypi 镜像），生成 GL 3.3 core loader | 产物已 vendor 至 `3rdparty/glad/` |
| stb | master | `https://cdn.jsdelivr.net/gh/nothings/stb@master/stb_image.h`、`stb_truetype.h` | 已 vendor 至 `3rdparty/stb/` |
| layui | v2.13.8 | `https://github.com/layui/layui` tag `v2.13.8`（整站镜像） | `3rdparty/src/layui/` |
| layui-dev-docs | v2.13.9 | layui 开发文档站点镜像（含 docs + layui dist） | `3rdparty/src/layui-dev-docs/` |

## 构建环境

| 组件 | 版本 | 说明 |
|------|------|------|
| MSYS2 | 20260611 | 安装器来自 `https://repo.msys2.org/distrib/x86_64/`（走 TUNA 镜像），已装至 `C:\msys64` |
| MSYS2 包 | base-devel / nasm 2.16.03 / diffutils / p7zip | pacman，镜像配置为 TUNA（`/etc/pacman.d/mirrorlist.*`） |
| Python | 3.12.8（anaconda） | 用于 glad2 生成器 |
| 编译器 | MSVC 14.51（VS 2026 Enterprise） | 主工具链，最终以 VS 2026 生成 `.slnx` 编译调试 |
| 回退 | MinGW-w64 `F:\ProgramFiles\mingw64` | 仅当 MSVC 链路不可用时启用 |
