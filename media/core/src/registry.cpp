#include "ui/media/pipeline/registry.h"

#include "ui/core/log.h"

#include <utility>

namespace ui {
namespace media {

namespace {
NodeRegistry* g_registry = nullptr;
}

void NodeRegistry::register_source(const char* name, SourceFactory f) {
    if (!name || !*name || !f) return;
    if (sources_.count(name)) {
        log_error("media: duplicate source node '%s' ignored", name);
        return;
    }
    sources_[name] = std::move(f);
}

void NodeRegistry::register_decoder(const char* name, DecoderFactory f) {
    if (!name || !*name || !f) return;
    if (decoders_.count(name)) {
        log_error("media: duplicate decoder node '%s' ignored", name);
        return;
    }
    decoders_[name] = std::move(f);
}

void NodeRegistry::register_audio(const char* name, AudioFactory f) {
    if (!name || !*name || !f) return;
    if (audios_.count(name)) {
        log_error("media: duplicate audio node '%s' ignored", name);
        return;
    }
    audios_[name] = std::move(f);
}

MediaSource* NodeRegistry::make_source(const char* name) const {
    auto it = sources_.find(name ? name : "");
    return it == sources_.end() ? nullptr : it->second();
}

VideoDecoder* NodeRegistry::make_decoder(const char* name) const {
    auto it = decoders_.find(name ? name : "");
    return it == decoders_.end() ? nullptr : it->second();
}

AudioSink* NodeRegistry::make_audio(const char* name) const {
    auto it = audios_.find(name ? name : "");
    return it == audios_.end() ? nullptr : it->second();
}

bool NodeRegistry::has_source(const char* name) const { return sources_.count(name ? name : "") > 0; }
bool NodeRegistry::has_decoder(const char* name) const {
    return decoders_.count(name ? name : "") > 0;
}
bool NodeRegistry::has_audio(const char* name) const { return audios_.count(name ? name : "") > 0; }

std::vector<std::string> NodeRegistry::source_names() const {
    std::vector<std::string> v;
    for (const auto& kv : sources_) v.push_back(kv.first);
    return v;
}

std::vector<std::string> NodeRegistry::decoder_names() const {
    std::vector<std::string> v;
    for (const auto& kv : decoders_) v.push_back(kv.first);
    return v;
}

std::vector<std::string> NodeRegistry::audio_names() const {
    std::vector<std::string> v;
    for (const auto& kv : audios_) v.push_back(kv.first);
    return v;
}

NodeRegistry& node_registry() {
    if (!g_registry) g_registry = new NodeRegistry;
    return *g_registry;
}

}  // namespace media
}  // namespace ui