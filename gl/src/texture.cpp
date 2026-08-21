// texture.cpp 内含 stb_image 唯一实现，UI 层其他文件不含它
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "ui/gl/texture.h"
#include "ui/core/log.h"
#include <glad/gl.h>

namespace ui {

Texture::~Texture() { destroy(); }

void Texture::destroy() {
    if (id_) glDeleteTextures(1, &id_);
    id_ = 0;
}

bool Texture::load_from_file(const char* path) {
    int w = 0, h = 0, comp = 0;
    unsigned char* data = stbi_load(path, &w, &h, &comp, 4);
    if (!data) {
        log_error("stbi_load failed: %s (%s)", stbi_failure_reason(), path ? path : "");
        return false;
    }
    create(w, h, data);
    stbi_image_free(data);
    return true;
}

void Texture::create(int width, int height, const void* rgba) {
    destroy();
    width_ = width;
    height_ = height;
    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void Texture::update(const void* rgba) {
    if (!id_) return;
    glBindTexture(GL_TEXTURE_2D, id_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::update_sub(int x, int y, int w, int h, const void* rgba) {
    if (!id_ || w <= 0 || h <= 0) return;
    glBindTexture(GL_TEXTURE_2D, id_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id_);
}

}  // namespace ui
