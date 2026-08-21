#include "ui/core/log.h"

#include <cstdarg>
#include <cstdio>
#include <ctime>

namespace ui {
namespace detail {
static void log_print(const char* level, const char* fmt, va_list args) {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &t);
    char ts[32];
    std::strftime(ts, sizeof(ts), "%H:%M:%S", &tm);
    std::fprintf(stderr, "[%s][ui][%s] ", ts, level);
    std::vfprintf(stderr, fmt, args);
    std::fprintf(stderr, "\n");
    std::fflush(stderr);  // 重定向/管道下也实时输出，避免进程终止时丢失尾部日志
}
}  // namespace detail

void log_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    detail::log_print("info", fmt, args);
    va_end(args);
}

void log_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    detail::log_print("error", fmt, args);
    va_end(args);
}
}  // namespace ui
