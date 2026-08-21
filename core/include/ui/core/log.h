#pragma once

namespace ui {
// 简洁日志：stderr 输出，带 [ui] 前缀。TODO(M1): 升级为可带 level/时间戳的 Logger
void log_info(const char* fmt, ...);
void log_error(const char* fmt, ...);
}
