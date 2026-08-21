#include "../page_registry.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/countdown_text.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

#include <cstdio>
#include <ctime>

namespace {

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

void add_line(PageHost& host, const char* text, const ui::Vec4* color = nullptr) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(color ? *color : host.theme->text);
    host.content->add_child(label);
}

void build_util(PageHost& host) {
    add_section(host, "工具模块（layui.util）——数字补零 / 相对时间 / 日期格式化 / 倒计时");

    add_section(host, "util.digit：数字补零");
    add_line(host, "digit(1) = 01，digit(10) = 10，digit(99) = 99，digit(100) = 100", &host.theme->text_secondary);

    add_section(host, "util.timeAgo：相对时间");
    add_line(host, "刚刚（< 1 分钟）· 5 分钟前 · 3 小时前 · 昨天 · 3 天前 · 1 周前 · 1 个月前 · 1 年前",
             &host.theme->text_secondary);

    add_section(host, "util.toDateString：日期格式化");
    {
        std::time_t t = std::time(nullptr);
        std::tm tm{};
#ifdef _MSC_VER
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        char buf[64];
        strftime(buf, sizeof(buf), "toDateString(t) = %Y-%m-%d", &tm);
        add_line(host, buf, &host.theme->text_secondary);
        strftime(buf, sizeof(buf), "toDateString(t, 'yyyy-MM-dd HH:mm:ss') = %Y-%m-%d %H:%M:%S", &tm);
        add_line(host, buf, &host.theme->text_secondary);
    }

    add_section(host, "util.countDown：倒计时（实时刷新）");
    ui::CountdownText* cd = new ui::CountdownText(0, 0, 800, 0);
    cd->set_font(host.font);
    cd->set_theme(host.theme);
    host.content->add_child(cd);
    {
        ui::Button* restart = new ui::Button(0, 0, 180.f, 34.f, "重新开始倒计时");
        restart->set_font(host.font);
        restart->set_theme(host.theme);
        restart->add_class("layui-btn layui-btn-sm");
        restart->set_on_click([cd]() { cd->restart(); });
        host.content->add_child(restart);
    }

    add_section(host, "说明");
    ui::Label* note = new ui::Label(0, 0, 800, 0,
                                    "util 为纯函数工具集：本页展示前四项；另有 util.fixbar 固定按钮条（跳转顶部/二维码）"
                                    "与 util.event 自定义事件，均与页面渲染无直接关系，未展示。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);

    if (host.code) {
        host.code->set_lang("JavaScript");
        host.code->set_code(
            "// 克隆 layui util.html：工具模块\n"
            "util.digit(num);                 // 数字补零（<10 -> 0X）\n"
            "util.timeAgo(timestamp);         // 相对时间（刚刚/xx 前/昨天/…）\n"
            "util.toDateString(t, 'yyyy-MM-dd HH:mm:ss');  // 日期格式化\n"
            "util.countDown({time: 60, end: function(){ /* 时间到 */ }});\n"
            "\n"
            "// C++ 侧等价实现（pages/util_page.cpp）：\n"
            "// 补零/相对时间/格式化由 std::strftime 与静态文本演示\n"
            "// 倒计时为 ui 库 CountdownText：on_draw 每帧读 steady_clock 计算剩余");
    }
    if (host.help) {
        const char* lines[] = {
            "工具模块（layui.util）API",
            "  digit(num)：数字补零，两位数内左补 0",
            "  timeAgo(time)：输出「刚刚 / 5 分钟前 / 昨天 / 3 天前…」",
            "  toDateString(time, fmt)：按格式模板格式化时间",
            "  countDown({time, end})：倒计时，到点回调",
            "  本页倒计时为 ui 库 CountdownText：on_draw 每帧按 steady_clock 实时刷新剩余",
        };
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            label->set_font(host.font);
            label->set_theme(host.theme);
            label->set_color(host.theme->text);
            host.help->add_child(label);
        }
    }
}

}  // namespace

static void build(PageHost& host) {
    build_util(host);
}

static const PageDef kUtilPage{61, "其他", "工具模块", "工具模块：digit/timeAgo/倒计时", build, nullptr, false, nullptr, 0, "Util"};

static const bool kRegistered = []() {
    page::register_page(kUtilPage);
    return true;
}();
