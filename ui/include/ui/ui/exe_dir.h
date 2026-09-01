#pragma once

#include <string>

namespace ui {

// 可执行文件所在目录（Windows，UTF-8 编码，不含尾斜杠）；非 Win32 / 取不到时返回空串。
// 集中实现于 ui/src（image.cpp），供资产定位（如 iconfont.ttf / logo.png / 图片）按 exe 目录回退。
std::string exe_directory();

}  // namespace ui