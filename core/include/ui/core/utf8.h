#pragma once

#include <cstddef>
#include <string>

namespace ui {
namespace utf8 {

// 下一个 UTF-8 字符起始下标（跳过后续续字节）。越界/回尾返回 s.size()。
// 统一 textbox/textarea/table 各自复制的 next_cp，防实现漂移。
inline size_t next_index(const std::string& s, size_t i) {
    if (i >= s.size()) return s.size();
    size_t j = i + 1;
    while (j < s.size() && ((unsigned char)s[j] & 0xC0) == 0x80) ++j;
    return j;
}

// 上一个 UTF-8 字符起始下标。i==0 返回 0。
inline size_t prev_index(const std::string& s, size_t i) {
    if (i == 0) return 0;
    size_t j = i - 1;
    while (j > 0 && ((unsigned char)s[j] & 0xC0) == 0x80) --j;
    return j;
}

// 解码 s[i] 起一个 UTF-8 码点（假定 i 为字符起始）。
inline unsigned int decode_cp(const std::string& s, size_t i) {
    const unsigned char c = (unsigned char)s[i];
    if (c < 0x80) return c;
    if ((c & 0xE0) == 0xC0) return ((c & 0x1F) << 6) | ((unsigned char)s[i + 1] & 0x3F);
    if ((c & 0xF0) == 0xE0)
        return ((c & 0x0F) << 12) | (((unsigned char)s[i + 1] & 0x3F) << 6) |
               ((unsigned char)s[i + 2] & 0x3F);
    return ((c & 0x07) << 18) | (((unsigned char)s[i + 1] & 0x3F) << 12) |
           (((unsigned char)s[i + 2] & 0x3F) << 6) | ((unsigned char)s[i + 3] & 0x3F);
}

}  // namespace utf8
}  // namespace ui
