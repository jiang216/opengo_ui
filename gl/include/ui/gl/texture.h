#pragma once

namespace ui {

// 2D 纹理（stb_image 解码 + GL 纹理上传）
class Texture {
public:
    Texture() = default;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    bool load_from_file(const char* path);
    void create(int width, int height, const void* rgba);
    // 更新已创建纹理的内容（尺寸须与 create 一致，视频逐帧上传用）
    void update(const void* rgba);
    // 局部更新纹理矩形区域（动态字体图集按需烘培用）；区域须在纹理边界内
    void update_sub(int x, int y, int w, int h, const void* rgba);
    void bind(unsigned int unit = 0) const;

    unsigned int id() const { return id_; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    void destroy();
    unsigned int id_ = 0;
    int width_ = 0;
    int height_ = 0;
};

}  // namespace ui
