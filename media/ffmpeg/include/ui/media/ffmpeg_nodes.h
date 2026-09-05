#pragma once

// FFmpeg 节点注册入口（UI-191）：把 ffmpeg-source / ffmpeg / winmm 三类节点注册进全局
// 媒体注册表（media/core）。仅链接 ui_media_ffmpeg 时调用一次即可；幂等（重复注册被忽略）。
// 头文件不暴露任何 FFmpeg/C 类型——实现完全隔离在 media/ffmpeg/src/ffmpeg_nodes.cpp。

namespace ui {
namespace media {

// 注册 FFmpeg 实现节点（可在任意线程/任意调用次数安全执行）
void register_ffmpeg_nodes();

}  // namespace media
}  // namespace ui