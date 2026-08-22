#pragma once

#include "ui/core/math.h"

#include <chrono>
#include <unordered_map>
#include <vector>

namespace ui {
class Renderer;
class Texture;

// 位图字体：stb_truetype 静态烘培 ASCII 32..126 + 可选图标区（U+E600..ECFF、U+1000..100F）
// 到基础图集；非 ASCII/图标码点（CJK 等）经 load_cjk_font 载入的中文字体按需懒烘培
// 到同一图集的剩余区（glTexSubImage2D 局部上传），支持自定义字体路径/环境变量。
class Font {
public:
    Font() = default;
    ~Font();

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    // 从 .ttf 加载并烘培图集；pixel_height 为像素字号
    bool load_from_file(const char* path, float pixel_height = 32.f);

    // 加载中文字体（懒烘培源）：保留 TTF/TTC 字节，首次遇到图集外码点按需光栅化。
    // .ttc 集合用 font_index 选取子字体；失败返回 false（ASCII 仍可用）。
    bool load_cjk_font(const char* path, int font_index = 0);

    // 自动探测 CJK 字体：OPENGUI_CJK_FONT 环境变量（自定义字体）→ 系统候选
    // msyh.ttc / msyh.ttf / simhei.ttf / simsun.ttc / Deng.ttf；命中打日志。
    bool load_cjk_font_auto();

    // 加载图标字体（M17d）：烘培 U+E600..E7FF 私用区 + U+1000..100F（layui 2.13.x
    // close/ok-circle 等非私用区码点）到第二图集（layui iconfont.ttf）
    bool load_icon_font(const char* path, float pixel_height);

    // (x, y) 为文本左上角（像素坐标，y 向下）；UTF-8 输入，私用区码点走图标图集。
    // scale 为相对烘培像素号的缩放倍率（图集 2x 过采样，≤2x 仍清晰）；bold 为仿粗体
    // （字形沿 x 错位叠加一次，偏移随字号缩放）。
    void draw_text(Renderer& renderer, const char* text, float x, float y,
                   const Vec4& color, float scale = 1.f, bool bold = false) const;

    float measure_text(const char* text, float scale = 1.f) const;
    // 单字符宽度：ASCII 用字形 advance；私用区图标用图标 advance；CJK 懒烘培后取真实 advance，
    // 仍缺失时按方块字近似为字号
    float char_width(unsigned int codepoint) const;

    bool icon_loaded() const { return icon_atlas_ != nullptr; }
    // 强制烘焙一段 UTF-8 文本的全部 CJK 字形（UI-125）：不受逐帧配额限制，
    // 供启动预热等场景一次性消除首绘卡顿；重复码点/图集内字形自动跳过
    void prebake_text(const char* utf8);
    // 以 (cx, cy) 为中心绘制单个图标，目标尺寸 size（像素）；rotation 绕中心旋转（弧度，0 不转）
    void draw_icon(Renderer& renderer, unsigned codepoint, float cx, float cy, float size,
                   const Vec4& color, float rotation = 0.f) const;

    float ascent() const { return ascent_; }
    float descent() const { return descent_; }
    float line_height() const { return ascent_ - descent_; }
    float pixel_height() const { return pixel_height_; }
    bool cjk_ready() const { return cjk_ready_; }

private:
    static constexpr int kFirstChar = 32;
    static constexpr int kNumChars = 95;  // 32..126
    static constexpr unsigned kIconFirst = 0xE600;
    static constexpr int kIconNum = 0x700;  // E600..ECFF（2.13.x 新增字形散布至 EB2E）
    static constexpr unsigned kIcon2First = 0x1000;
    static constexpr int kIcon2Num = 0x10;  // 1000..100F（2.13.x 新增图标段）

    struct Glyph {
        float x0 = 0, y0 = 0, x1 = 0, y1 = 0;  // 相对基线（像素，y 向下）
        float u0 = 0, v0 = 0, u1 = 0, v1 = 0;  // 图集 UV
        float xadvance = 0;
    };

    // UTF-8 解码下一个码点；*in 指向字节，返回解码后的码点，*in 前进到下一字符（返回 0 表示结束/非法）
    static unsigned decode_utf8(const char*& in);

    // 懒烘培 CJK 字形：查 dyn_ → 命中返回；cjk 字体未载入返回 nullptr；否则光栅化打包进图集
    const Glyph* ensure_dynamic(unsigned int cp) const;
    void bake_cjk(unsigned int cp) const;

    // 图标字形查询：按码点落在 E600..E7FF / 1000..100F 哪一段返回对应字形，段外返回 nullptr
    const Glyph* icon_glyph(unsigned int cp) const;

    Texture* atlas_ = nullptr;
    Texture* icon_atlas_ = nullptr;
    Glyph glyphs_[kNumChars] = {};
    Glyph icon_glyphs_[kIconNum] = {};
    Glyph icon2_glyphs_[kIcon2Num] = {};
    float ascent_ = 0;
    float descent_ = 0;
    float pixel_height_ = 32.f;

    // ---- CJK 懒烘培状态（draw/measure 为 const，故均 mutable）----
    std::vector<unsigned char> cjk_ttf_;
    void* cjk_info_ = nullptr;  // stbtt_fontinfo*（stb_truetype 头不进公开头文件）
    bool cjk_ready_ = false;
    mutable std::unordered_map<unsigned int, Glyph> dyn_;
    mutable int pack_x_ = 0;
    mutable int pack_y_ = 0;
    mutable int pack_row_h_ = 0;
    mutable bool atlas_full_logged_ = false;

    // ---- 逐帧烘焙配额（UI-125）：单帧大量新字形逐字光栅化+纹理上传会造成
    // 可感知卡顿/闪烁；按时间窗口限量烘焙，超配额本帧回退方块字宽近似 ----
    static constexpr int kBakeBudgetPerWindow = 12;
    static constexpr double kBakeWindowSec = 0.016;
    mutable int bake_budget_ = kBakeBudgetPerWindow;
    mutable std::chrono::steady_clock::time_point bake_window_;
};

}  // namespace ui
