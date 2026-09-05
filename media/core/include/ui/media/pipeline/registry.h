#pragma once

// 媒体节点注册表（UI-191）：name -> 工厂。运行期按名组装管线，实现"节点可更换"。
// media/core 自带合成源（test-pattern）；media/ffmpeg 静态注册 ffmpeg 实现。
// 同名重复注册：告警并忽略后者（防覆盖导致行为二义）。

#include "ui/media/pipeline/node.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace ui {
namespace media {

using SourceFactory = std::function<MediaSource*()>;
using DecoderFactory = std::function<VideoDecoder*()>;
using AudioFactory = std::function<AudioSink*()>;

class NodeRegistry {
public:
    void register_source(const char* name, SourceFactory f);
    void register_decoder(const char* name, DecoderFactory f);
    void register_audio(const char* name, AudioFactory f);

    MediaSource* make_source(const char* name) const;    // 未注册返回 nullptr
    VideoDecoder* make_decoder(const char* name) const;
    AudioSink* make_audio(const char* name) const;

    bool has_source(const char* name) const;
    bool has_decoder(const char* name) const;
    bool has_audio(const char* name) const;

    std::vector<std::string> source_names() const;
    std::vector<std::string> decoder_names() const;
    std::vector<std::string> audio_names() const;

private:
    std::map<std::string, SourceFactory> sources_;
    std::map<std::string, DecoderFactory> decoders_;
    std::map<std::string, AudioFactory> audios_;
};

// 全局注册表（进程单例）
NodeRegistry& node_registry();

// 注册 media/core 自带节点（test-pattern / null）。MediaPipeline 构造时自动调用（幂等），
// 显式暴露以防静态链接裁剪未引用 TU 导致合成源缺失。
void register_core_nodes();

}  // namespace media
}  // namespace ui