#pragma once

#include "ui/ui/widget.h"

#include <string>

namespace ui {
class Renderer;
class Texture;

// 图片控件（M17e，layui <img> 克隆）：显示本地图片文件（stb_image 解码），
// 绘制为整个控件矩形（铺满 bounds）。加载失败时空白不绘制。
// 用于头像/插图等静态图片展示；动态纹理（视频帧等）请直接用 ui::Texture + 自定义绘制。
//
// 图片懒加载（lay-src 语义，UI-093）：调 set_lay_src() 后不立即解码，而是在每次
// on_draw 时用 Renderer::current_scissor()（即当前可视屏）检测自身 bounds 是否进入可视区，
// 首次进入才真正 load_from_file。适合信息流等「仅加载当前屏图片」的大图列表。
class Image : public Widget {
public:
    Image(float x, float y, float w, float h);
    ~Image() override;

    // 加载图片文件（png/jpg/bmp/webp 等），成功返回 true；
    // 构造时未显式给显示尺寸（w/h <= 0）则用图片原始尺寸作为内容高度。
    // 裸文件名（无路径分隔符）先按 CWD 解析，miss 时回退 exe 目录同名文件
    //（资产定位不依赖工作目录，UI-048）；全部 miss 返回 false 并 log 一次。
    bool load_from_file(const char* path);

    // 设置懒加载源（lay-src 语义）：仅在图片进入当前可视屏（Renderer::current_scissor）
    // 时真正 load_from_file。便于信息流/图片懒加载按需取图；进入可视区即加载一次。
    void set_lay_src(const char* path);

    bool loaded() const { return loaded_; }

    // 内容自适应高度：返回显示高度（未加载且未显式指定显示尺寸返回 0；
    // 懒加载图片构造时显式给 w/h，则加载前也预留该空间保证列表布局稳定——UI-093）
    float preferred_height() const override;

    void on_draw(Renderer& renderer) override;

private:
    Texture* texture_ = nullptr;
    bool loaded_ = false;
    float display_w_ = 0.f;
    float display_h_ = 0.f;
    bool has_display_size_ = false;  // 构造时是否显式指定了显示尺寸（懒加载预留布局用）
    std::string lay_src_;  // 懒加载源（lay-src），仅在进入可视屏后解码
};

}  // namespace ui
