// font.cpp 内含 stb_truetype 唯一实现
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/gl/texture.h"
#include "ui/core/log.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace ui {
namespace {

constexpr int kAtlasW = 2048;
constexpr int kAtlasH = 1024;
constexpr int kIconAtlasW = 1024;
constexpr int kIconAtlasH = 1024;
constexpr int kDynPad = 2;  // 懒烘培字形之间的间隔像素（防线性过滤串色；显示像素，烘培按 kAtlasScale 放大）

// 字体图集过采样倍数（UI-055 P2）：字形按 kAtlasScale×pixel_height 光栅化入图集，
// Glyph 度量换算成显示像素存储（乘 kAtlasInvScale），UV 仍指向 2x 图集；显示时
// GL_LINEAR 缩小采样 = 每显示像素平均 2x2 纹素，笔画更实、边缘更平滑。draw_text/
// measure_text/布局均无需改动（度量已是显示像素）。
constexpr float kAtlasScale = 2.0f;
constexpr float kAtlasInvScale = 1.0f / kAtlasScale;

}  // namespace

unsigned Font::decode_utf8(const char*& in) {
    const unsigned char c = (unsigned char)*in;
    if (c == 0) return 0;
    if (c < 0x80) {
        ++in;
        return c;
    }
    if ((c & 0xE0) == 0xC0) {  // 2 字节
        const unsigned c2 = (unsigned char)in[1];
        if ((c2 & 0xC0) != 0x80) { ++in; return c; }
        const unsigned cp = ((c & 0x1F) << 6) | (c2 & 0x3F);
        in += 2;
        return cp;
    }
    if ((c & 0xF0) == 0xE0) {  // 3 字节
        const unsigned c2 = (unsigned char)in[1], c3 = (unsigned char)in[2];
        if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) { ++in; return c; }
        const unsigned cp = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
        in += 3;
        return cp;
    }
    if ((c & 0xF8) == 0xF0) {  // 4 字节
        const unsigned c2 = (unsigned char)in[1], c3 = (unsigned char)in[2], c4 = (unsigned char)in[3];
        if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 || (c4 & 0xC0) != 0x80) { ++in; return c; }
        const unsigned cp = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
        in += 4;
        return cp;
    }
    ++in;
    return c;
}

Font::~Font() {
    delete atlas_;
    delete icon_atlas_;
    delete static_cast<stbtt_fontinfo*>(cjk_info_);
}

bool Font::load_icon_font(const char* path, float pixel_height) {
    std::vector<unsigned char> ttf;
    FILE* f = fopen(path, "rb");
    if (!f) {
        log_error("icon font: cannot open %s", path ? path : "");
        return false;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size > 0) {
        ttf.resize((size_t)size);
        if (fread(ttf.data(), 1, (size_t)size, f) != (size_t)size) {
            fclose(f);
            log_error("icon font: read failed %s", path ? path : "");
            return false;
        }
    }
    fclose(f);

    // 烘培 U+E600..E7FF 私用区（kAtlasScale 过采样）
    std::vector<unsigned char> bitmap((size_t)kIconAtlasW * kIconAtlasH);
    stbtt_bakedchar cdata[kIconNum];
    const int baked = stbtt_BakeFontBitmap(ttf.data(), 0, pixel_height * kAtlasScale, bitmap.data(),
                                           kIconAtlasW, kIconAtlasH, kIconFirst, kIconNum, cdata);
    if (baked < 0) {
        log_error("icon font: atlas overflow, need larger atlas");
        return false;
    }

    std::vector<unsigned char> rgba((size_t)kIconAtlasW * kIconAtlasH * 4);
    for (size_t i = 0; i < bitmap.size(); ++i) {
        rgba[i * 4 + 0] = 255;
        rgba[i * 4 + 1] = 255;
        rgba[i * 4 + 2] = 255;
        rgba[i * 4 + 3] = bitmap[i];
    }

    delete icon_atlas_;
    icon_atlas_ = new Texture;
    icon_atlas_->create(kIconAtlasW, kIconAtlasH, rgba.data());

    // stbtt_BakeFontBitmap 返回值为图集打包底边 y（bottom_y），非烘培字数；
    // 成功（>0）时 chardata 已填满全部 kIconNum 字形，须按总量拷贝度量（UI-046）
    const float ipw = 1.0f / kIconAtlasW, iph = 1.0f / kIconAtlasH;
    for (int i = 0; i < kIconNum; ++i) {
        Glyph& g = icon_glyphs_[i];
        const stbtt_bakedchar& b = cdata[i];
        // 烘培坐标为 2x 图集纹素，度量换算为显示像素（UV 仍指 2x 图集）
        g.x0 = b.xoff * kAtlasInvScale;
        g.y0 = b.yoff * kAtlasInvScale;
        g.x1 = (b.xoff + (b.x1 - b.x0)) * kAtlasInvScale;
        g.y1 = (b.yoff + (b.y1 - b.y0)) * kAtlasInvScale;
        g.u0 = b.x0 * ipw;
        g.v0 = b.y0 * iph;
        g.u1 = b.x1 * ipw;
        g.v1 = b.y1 * iph;
        g.xadvance = b.xadvance * kAtlasInvScale;
    }
    return true;
}

bool Font::load_from_file(const char* path, float pixel_height) {
    std::vector<unsigned char> ttf;
    FILE* f = fopen(path, "rb");
    if (!f) {
        log_error("font: cannot open %s", path ? path : "");
        return false;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size > 0) {
        ttf.resize((size_t)size);
        if (fread(ttf.data(), 1, (size_t)size, f) != (size_t)size) {
            fclose(f);
            log_error("font: read failed %s", path ? path : "");
            return false;
        }
    }
    fclose(f);

    // 1) 烘培 32..126 到单通道位图（kAtlasScale 过采样）
    std::vector<unsigned char> bitmap((size_t)kAtlasW * kAtlasH);
    stbtt_bakedchar cdata[kNumChars];
    const int baked = stbtt_BakeFontBitmap(ttf.data(), 0, pixel_height * kAtlasScale, bitmap.data(),
                                           kAtlasW, kAtlasH, kFirstChar, kNumChars, cdata);
    if (baked < 0) {
        log_error("font: atlas overflow, need larger atlas");
        return false;
    }

    // 2) 单通道覆盖值 -> RGBA（白字 + 覆盖度作 alpha，供纹理管线调制颜色）
    std::vector<unsigned char> rgba((size_t)kAtlasW * kAtlasH * 4);
    for (size_t i = 0; i < bitmap.size(); ++i) {
        rgba[i * 4 + 0] = 255;
        rgba[i * 4 + 1] = 255;
        rgba[i * 4 + 2] = 255;
        rgba[i * 4 + 3] = bitmap[i];
    }

    delete atlas_;
    atlas_ = new Texture;
    atlas_->create(kAtlasW, kAtlasH, rgba.data());

    // 3) 字形度量 + 记录 ASCII 烘培占用区（懒烘培游标从其下开始）
    // stbtt_BakeFontBitmap 返回值为图集打包底边 y（bottom_y，18px 下约 18），非烘培字数；
    // 成功（>0）时 chardata 已填满全部 kNumChars 字形，须按总量拷贝度量（UI-046）。
    // 烘培坐标为 2x 图集纹素，度量换算为显示像素（UV 仍指 2x 图集）。
    const float ipw = 1.0f / kAtlasW, iph = 1.0f / kAtlasH;
    int ascii_max_x = 0, ascii_max_y = 0;
    for (int i = 0; i < kNumChars; ++i) {
        Glyph& g = glyphs_[i];
        const stbtt_bakedchar& b = cdata[i];
        g.x0 = b.xoff * kAtlasInvScale;
        g.y0 = b.yoff * kAtlasInvScale;
        g.x1 = (b.xoff + (b.x1 - b.x0)) * kAtlasInvScale;
        g.y1 = (b.yoff + (b.y1 - b.y0)) * kAtlasInvScale;
        g.u0 = b.x0 * ipw;
        g.v0 = b.y0 * iph;
        g.u1 = b.x1 * ipw;
        g.v1 = b.y1 * iph;
        g.xadvance = b.xadvance * kAtlasInvScale;
        ascii_max_x = std::max(ascii_max_x, (int)b.x1);
        ascii_max_y = std::max(ascii_max_y, (int)b.y1);
    }
    pack_x_ = 0;
    pack_y_ = ascii_max_y + (int)(kDynPad * kAtlasScale);

    // 4) 行高：经 stbtt_GetFontVMetrics 获取 ascent/descent 并缩放
    stbtt_fontinfo info;
    if (stbtt_InitFont(&info, ttf.data(), stbtt_GetFontOffsetForIndex(ttf.data(), 0))) {
        int asc = 0, desc = 0, gap = 0;
        stbtt_GetFontVMetrics(&info, &asc, &desc, &gap);
        const float scale = stbtt_ScaleForPixelHeight(&info, pixel_height);
        ascent_ = asc * scale;
        descent_ = desc * scale;
    } else {
        const Glyph& H = glyphs_['H' - kFirstChar];
        const Glyph& p = glyphs_['p' - kFirstChar];
        ascent_ = -H.y0;
        descent_ = p.y1;
    }
    pixel_height_ = pixel_height;
    return true;
}

bool Font::load_cjk_font(const char* path, int font_index) {
    std::vector<unsigned char> ttf;
    FILE* f = fopen(path, "rb");
    if (!f) {
        log_error("cjk font: cannot open %s", path ? path : "");
        return false;
    }
    fseek(f, 0, SEEK_END);
    const long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size > 0) {
        ttf.resize((size_t)size);
        if (fread(ttf.data(), 1, (size_t)size, f) != (size_t)size) {
            fclose(f);
            log_error("cjk font: read failed %s", path ? path : "");
            return false;
        }
    }
    fclose(f);
    if (ttf.empty()) {
        log_error("cjk font: empty file %s", path ? path : "");
        return false;
    }

    if (!cjk_info_) cjk_info_ = new stbtt_fontinfo;
    const int offset = stbtt_GetFontOffsetForIndex(ttf.data(), font_index);
    if (offset < 0 || !stbtt_InitFont(static_cast<stbtt_fontinfo*>(cjk_info_), ttf.data(), offset)) {
        log_error("cjk font: init failed %s (index %d)", path ? path : "", font_index);
        cjk_ttf_.clear();
        cjk_ready_ = false;
        return false;
    }
    cjk_ttf_ = std::move(ttf);
    cjk_ready_ = true;
    log_info("cjk font loaded: %s", path ? path : "");
    return true;
}

bool Font::load_cjk_font_auto() {
    const char* custom = getenv("OPENGUI_CJK_FONT");
    if (custom && custom[0]) {
        if (load_cjk_font(custom)) return true;
        log_error("cjk font: OPENGUI_CJK_FONT '%s' failed, falling back to system fonts", custom);
    }
    static const char* kCandidates[] = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyh.ttf",
        "C:/Windows/Fonts/simhei.ttf",
        "C:/Windows/Fonts/simsun.ttc",
        "C:/Windows/Fonts/Deng.ttf",
    };
    for (const char* p : kCandidates) {
        if (load_cjk_font(p)) return true;
    }
    log_error("cjk font: no fallback found (set OPENGUI_CJK_FONT to use a custom font)");
    return false;
}

const Font::Glyph* Font::ensure_dynamic(unsigned int cp) const {
    auto it = dyn_.find(cp);
    if (it != dyn_.end()) return &it->second;
    if (!cjk_ready_) return nullptr;
    bake_cjk(cp);
    it = dyn_.find(cp);
    return it != dyn_.end() ? &it->second : nullptr;
}

void Font::bake_cjk(unsigned int cp) const {
    if (atlas_full_logged_) return;
    auto* info = static_cast<stbtt_fontinfo*>(cjk_info_);
    // kAtlasScale 过采样：2x 光栅化入图集，度量换算成显示像素
    const float scale = stbtt_ScaleForPixelHeight(info, pixel_height_ * kAtlasScale);
    int w = 0, h = 0, xoff = 0, yoff = 0;
    unsigned char* bmp = stbtt_GetCodepointBitmap(info, scale, scale, (int)cp, &w, &h, &xoff, &yoff);
    if (!bmp) return;
    if (w <= 0 || h <= 0) {
        stbtt_FreeBitmap(bmp, info);
        return;
    }

    const int pad = (int)(kDynPad * kAtlasScale);
    const int gw = w + pad, gh = h + pad;
    if (pack_x_ + gw > kAtlasW) {
        pack_x_ = 0;
        pack_y_ += pack_row_h_;
        pack_row_h_ = 0;
    }
    if (pack_y_ + gh > kAtlasH) {
        log_error("cjk font: atlas full (%dx%d), stop baking new glyphs", kAtlasW, kAtlasH);
        atlas_full_logged_ = true;
        stbtt_FreeBitmap(bmp, info);
        return;
    }

    // 单通道覆盖度 -> RGBA 局部子矩形（每行 gw 宽，右侧 pad 透明）
    std::vector<unsigned char> rgba((size_t)gw * gh * 4);
    for (int y = 0; y < h; ++y) {
        const unsigned char* src = bmp + (size_t)y * w;
        unsigned char* dst = rgba.data() + (size_t)y * gw * 4;
        for (int x = 0; x < w; ++x) {
            dst[x * 4 + 0] = 255;
            dst[x * 4 + 1] = 255;
            dst[x * 4 + 2] = 255;
            dst[x * 4 + 3] = src[x];
        }
    }
    stbtt_FreeBitmap(bmp, info);
    atlas_->update_sub(pack_x_, pack_y_, gw, gh, rgba.data());

    Glyph g;
    g.x0 = (float)xoff * kAtlasInvScale;
    g.y0 = (float)yoff * kAtlasInvScale;
    g.x1 = (float)(xoff + w) * kAtlasInvScale;
    g.y1 = (float)(yoff + h) * kAtlasInvScale;
    g.u0 = (float)pack_x_ / kAtlasW;
    g.v0 = (float)pack_y_ / kAtlasH;
    g.u1 = (float)(pack_x_ + w) / kAtlasW;
    g.v1 = (float)(pack_y_ + h) / kAtlasH;
    int adv = 0, lsb = 0;
    stbtt_GetCodepointHMetrics(info, (int)cp, &adv, &lsb);
    g.xadvance = adv * scale * kAtlasInvScale;
    dyn_[cp] = g;

    pack_x_ += gw;
    pack_row_h_ = std::max(pack_row_h_, gh);
}

void Font::draw_text(Renderer& renderer, const char* text, float x, float y,
                     const Vec4& color, float scale, bool bold) const {
    if ((!atlas_ && !icon_atlas_) || !text) return;
    const float baseline = y + ascent_ * scale;
    float cursor = x;
    // 仿粗体偏移：约字号的 1/16（18px → 1px），随 scale 同步
    const float bold_off = std::max(1.f, std::round(pixel_height_ * scale / 16.f));
    // 字形左上角对齐像素网格（round 到整数像素）：baseline = y + ascent_ 与
    // 逐字 xadvance 累加均为小数，落在非整数像素坐标时 GL_LINEAR 双线性采样会
    // 跨纹素混叠，造成文字发虚（UI-055）。对齐后每个字形内部 1:1 映射纹素，
    // 边缘仅与图集 padding（透明）混合，笔画更实；cursor 仍按浮点累加，行内
    // 间距与 measure_text 保持一致。
    const auto align = [](float v) { return std::round(v); };
    for (const char* p = text; *p;) {
        const unsigned cp = decode_utf8(p);
        if (cp == 0) break;
        if (cp >= kFirstChar && cp < kFirstChar + kNumChars) {
            const Glyph& g = glyphs_[cp - kFirstChar];
            if (g.x1 <= g.x0 || g.y1 <= g.y0) { cursor += g.xadvance * scale; continue; }
            const Rect rect{align(cursor + g.x0 * scale), align(baseline + g.y0 * scale),
                            (g.x1 - g.x0) * scale, (g.y1 - g.y0) * scale};
            const Rect uv{g.u0, g.v0, g.u1, g.v1};
            renderer.draw_textured_quad(rect, atlas_->id(), uv, color);
            if (bold)
                renderer.draw_textured_quad({rect.x + bold_off, rect.y, rect.w, rect.h},
                                            atlas_->id(), uv, color);
            cursor += g.xadvance * scale;
        } else if (cp >= kIconFirst && cp < kIconFirst + kIconNum && icon_atlas_) {
            const Glyph& g = icon_glyphs_[cp - kIconFirst];
            if (g.x1 <= g.x0 || g.y1 <= g.y0) { cursor += g.xadvance * scale; continue; }
            const Rect rect{align(cursor + g.x0 * scale), align(baseline + g.y0 * scale),
                            (g.x1 - g.x0) * scale, (g.y1 - g.y0) * scale};
            const Rect uv{g.u0, g.v0, g.u1, g.v1};
            renderer.draw_textured_quad(rect, icon_atlas_->id(), uv, color);
            if (bold)
                renderer.draw_textured_quad({rect.x + bold_off, rect.y, rect.w, rect.h},
                                            icon_atlas_->id(), uv, color);
            cursor += g.xadvance * scale;
        } else {
            // CJK 懒烘培：命中即按真实字形绘制，缺失退回方块字宽近似
            const Glyph* g = ensure_dynamic(cp);
            if (g && g->x1 > g->x0 && g->y1 > g->y0) {
                const Rect rect{align(cursor + g->x0 * scale), align(baseline + g->y0 * scale),
                                (g->x1 - g->x0) * scale, (g->y1 - g->y0) * scale};
                const Rect uv{g->u0, g->v0, g->u1, g->v1};
                renderer.draw_textured_quad(rect, atlas_->id(), uv, color);
                if (bold)
                    renderer.draw_textured_quad({rect.x + bold_off, rect.y, rect.w, rect.h},
                                                atlas_->id(), uv, color);
                cursor += g->xadvance * scale;
            } else {
                cursor += pixel_height_ * scale;  // 非图集字符（CJK 等）按方块字近似
            }
        }
    }
}

float Font::measure_text(const char* text, float scale) const {
    if (!text) return 0.f;
    float width = 0.f;
    for (const char* p = text; *p;) {
        const unsigned cp = decode_utf8(p);
        if (cp == 0) break;
        width += char_width(cp) * scale;
    }
    return width;
}

void Font::draw_icon(Renderer& renderer, unsigned codepoint, float cx, float cy, float size,
                     const Vec4& color, float rotation) const {
    if (!icon_atlas_ || codepoint < kIconFirst || codepoint >= kIconFirst + kIconNum) return;
    const Glyph& g = icon_glyphs_[codepoint - kIconFirst];
    if (g.x1 <= g.x0 || g.y1 <= g.y0) return;
    const float gw = g.x1 - g.x0, gh = g.y1 - g.y0;
    if (gw <= 0.f || gh <= 0.f) return;
    // 归一化字形框（glyph 在 em 方块内的比例），再按目标尺寸放置
    const float glyph_w = g.xadvance > 0.f ? g.xadvance : size;
    const float scale = size / glyph_w;
    const float w = gw * scale, h = gh * scale;
    // 左上角对齐像素网格（UI-055），避免中心小数坐标下 LINEAR 采样发虚
    const Rect rect{std::round(cx - w / 2.f), std::round(cy - h / 2.f), w, h};
    const Rect uv{g.u0, g.v0, g.u1, g.v1};
    renderer.draw_textured_quad(rect, icon_atlas_->id(), uv, color, 0.f, rotation);
}

float Font::char_width(unsigned int codepoint) const {
    if (codepoint >= kFirstChar && codepoint < kFirstChar + kNumChars)
        return glyphs_[codepoint - kFirstChar].xadvance;
    if (codepoint >= kIconFirst && codepoint < kIconFirst + kIconNum)
        return icon_glyphs_[codepoint - kIconFirst].xadvance > 0
                   ? icon_glyphs_[codepoint - kIconFirst].xadvance
                   : pixel_height_;
    const Glyph* g = ensure_dynamic(codepoint);
    if (g) return g->xadvance;
    return pixel_height_;  // 非图集字符按方块字近似
}

}  // namespace ui
